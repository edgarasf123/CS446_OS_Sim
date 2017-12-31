
#include "Simulation.h"
#include "helpers.h"

#include <vector>
#include <unordered_set>
#include <iostream>
#include <string>
#include <regex>
#include <pthread.h>
#include <climits>

using SimHelpers::strTrim;
using SimHelpers::strSplit;
using SimHelpers::strLower;

using std::vector;
using std::unordered_set;
using std::queue;
using std::fstream;
using std::ios;
using std::getline;
using std::string;
using std::cout;
using std::flush;
using std::regex;
using std::regex_search;
using std::smatch;

SimError::SimError( const char * format, ...)
{
    va_list args;
    va_start( args, format );
    vsnprintf( msg, sizeof msg, format, args );
    va_end ( args );
}

char const * SimError::what() const throw () { return msg; }

Simulation::Simulation( const string &configFile )
{
    processes.resize(4096);
    pthread_mutex_init(&simMutex, NULL);
    pthread_mutex_init(&logMutex, NULL);

    memoryBlockCounter = 0;

    config.AddOption( "Version/Phase",                  ConfigType::Double );
    config.AddOption( "File Path",                      ConfigType::String );
    config.AddOption( "Processor cycle time (msec)",    ConfigType::Int    );
    config.AddOption( "Monitor display time (msec)",    ConfigType::Int    );
    config.AddOption( "Hard drive cycle time (msec)",   ConfigType::Int    );
    config.AddOption( "Printer cycle time (msec)",      ConfigType::Int    );
    config.AddOption( "Keyboard cycle time (msec)",     ConfigType::Int    );
    config.AddOption( "Mouse cycle time (msec)",        ConfigType::Int    );
    config.AddOption( "Speaker cycle time (msec)",      ConfigType::Int    );
    config.AddOption( "Memory cycle time (msec)",       ConfigType::Int    );
    config.AddOption( "Log",                            ConfigType::String );
    config.AddOption( "Log File Path",                  ConfigType::String );
    config.AddOption( "Printer quantity",               ConfigType::Int    );
    config.AddOption( "Hard drive quantity",            ConfigType::Int    );
    config.AddOption( "Speaker quantity",               ConfigType::Int    );
    config.AddOption( "Quantum Number (msec)",          ConfigType::Int    );
    config.AddOption( "Memory block size (kbytes)",     ConfigType::Int    );
    config.AddOption( "System memory (kbytes)",         ConfigType::Int    );
    config.AddOption( "System memory (Mbytes)",         ConfigType::Int    );
    config.AddOption( "System memory (Gbytes)",         ConfigType::Int    );
    config.AddOption( "CPU Scheduling Code",            ConfigType::String );

    // Default values
    config.SetInt( "Mouse cycle time (msec)", 1 );
    config.SetInt( "Speaker cycle time (msec)", 1 );
    config.SetInt( "System memory (Mbytes)", 0 );
    config.SetInt( "Speaker quantity", 1 );
    config.SetInt( "Hard drive quantity", 1 );
    config.SetInt( "System memory (Gbytes)", 0 );

	ReadConfigFile( configFile );
	LoadConfig( );
    ReadMetaData( );

    //LogConfig( );
}
Simulation::~Simulation( )
{
    if ( logFile.is_open() )
        logFile.close();

    if(resHdd)      delete resHdd;
    if(resPrinter)  delete resPrinter;
    if(resMonitor)  delete resMonitor;
    if(resKeyboard) delete resKeyboard;
    if(resMouse)    delete resMouse;
    if(resSpeaker)  delete resSpeaker;
}

void Simulation::Log( char const * format, ... )
{
    pthread_mutex_lock(&logMutex);

    char msg[1024];

    va_list args;
    va_start( args, format );
    vsnprintf( msg, sizeof msg, format, args );
    va_end ( args );

    if( logToMonitor )
        cout << msg << flush;
    
    if( logToFile )
        logFile << msg << flush;

    pthread_mutex_unlock(&logMutex);
}

void Simulation::ReadConfigFile( const string &configFile )
{
    const string configHeader = "Start Simulator Configuration File";
    const string configFooter = "End Simulator Configuration File";
    
    
    fstream  fl;
    fl.open( configFile, ios::in );
    if(!fl.is_open())    
        throw SimError( "Unable to open config file: %s", configFile.c_str() );

    string line;
    // Skip to config header
    while(getline(fl, line))
    {
        if(strTrim(line) == configHeader);
            break;
    }
    if( line != configHeader )
        throw SimError( "Config header is missing!" );
    
    // Read the config
    string key;
    string val;
    
    regex rConfigLine(R"(^\s*([\S\t ]*?)\s*:\s*([\S\t ]+?)\s*$)");
    regex rEmptyLine(R"(^\s*$)");
    
    while( getline(fl, line) )
    {
        line = strTrim(line);
        if( line == configFooter )
            break;

        smatch sm;        
        if( !regex_search(line, sm, rConfigLine) )
        {
            if( !regex_search(line, sm, rEmptyLine) )
                throw SimError( "Unable to parse config line: %s", line.c_str() );
            continue;
        }

        key = sm.str(1);
        val = sm.str(2);

        configKeyValues.insert({key, val});
    }
    fl.close();

    if( line != configFooter )
        throw SimError( "Config footer is missing!" );
}

void Simulation::LoadConfig( )
{
    // Load all config options
    for ( auto it = configKeyValues.begin(); it != configKeyValues.end(); ++it ){
        config.SetStr(it->first, it->second);

        if( it->first == "System memory (Mbytes)" )
            config.SetInt( "System memory (kbytes)", (long int)config.GetInt("System memory (Mbytes)") * 10e3 );
        else if( it->first == "System memory (Gbytes)" )
            config.SetInt( "System memory (kbytes)", (long int)config.GetInt("System memory (Gbytes)") * 10e6 );
    }

    // Check if all config options are initialized
    vector<string> labels;
    config.GetOptionLabels( labels );
    for( auto it = labels.begin(); it != labels.end(); it++ )
    {
        if( !config.OptionInitialized( *it ) )
            throw SimError( "\"%s\" config option is not initialized", (*it).c_str() );
    }

    // Check if times are at least 1
    if( config.GetInt( "Processor cycle time (msec)" ) < 1 )
        throw SimError( "Processor cycle time (msec) must be at least 1." );
    if( config.GetInt( "Monitor display time (msec)" ) < 1 )
        throw SimError( "Monitor display time (msec) must be at least 1." );
    if( config.GetInt( "Hard drive cycle time (msec)" ) < 1 )
        throw SimError( "Hard drive cycle time (msec) must be at least 1." );
    if( config.GetInt( "Printer cycle time (msec)" ) < 1 )
        throw SimError( "Printer cycle time (msec) must be at least 1." );
    if( config.GetInt( "Keyboard cycle time (msec)" ) < 1 )
        throw SimError( "Keyboard cycle time (msec) must be at least 1." );
    if( config.GetInt( "Mouse cycle time (msec)" ) < 1 )
        throw SimError( "Mouse cycle time (msec) must be at least 1." );
    if( config.GetInt( "Speaker cycle time (msec)" ) < 1 )
        throw SimError( "Speaker cycle time (msec) must be at least 1." );
    if( config.GetInt( "Memory cycle time (msec)" ) < 1 )
        throw SimError( "Memory cycle time (msec) must be at least 1." );
    if( config.GetInt( "System memory (kbytes)" ) < 1 )
        throw SimError( "System memory must be at least 1 kbytes." );
    if( config.GetInt( "Memory block size (kbytes)" ) < 1 )
        throw SimError( "Memory block size must be at least 1 kbytes." );

    // Set scheduling algorithm
    string s_scheduling = config.GetStr("CPU Scheduling Code");
    if( s_scheduling == "RR" )
        scheduling = SchedulingCode::RR;
    else if( s_scheduling == "STR" ) // Shortest Time Remaining
        scheduling = SchedulingCode::SRTF;
    else if( s_scheduling == "SRT" ) // Shortest Remaining Time
        scheduling = SchedulingCode::SRTF;
    else if( s_scheduling == "SRTF" ) // Shortest Remaining Time First
        scheduling = SchedulingCode::SRTF;
    else
        throw SimError( "\"%s\" is an invalid scheduling code. Possible scheduling codes are RR and SRTF.", s_scheduling.c_str() );

    // Calculate max of memory blocks
    maxMemoryBlocks = config.GetInt( "System memory (kbytes)" ) / config.GetInt( "Memory block size (kbytes)" );

    // Initialize logs
    string log = strLower( config.GetStr("Log") );
    if( log == "log to both" )
    {
        logToFile = true;
        logToMonitor = true;
    }
    else if( log == "log to file" )
    {
        logToFile = true;
        logToMonitor = false;

    }
    else if( log == "log to monitor" )
    {
        logToFile = false;
        logToMonitor = true;
    }
    else
    {
        throw SimError( "Log config option is invalid: %s", config.GetStr("Log").c_str() );
    }
    
    if( logToFile )
    {
        string logFilePath = config.GetStr("Log File Path");

        logFile.open( logFilePath, ios::out );
        if( !logFile.is_open() )    
            throw SimError( "Unable to open log file: %s", logFilePath.c_str() );
    }

    //Initialize resources

    config.AddOption( "Printer quantity",               ConfigType::Int    );
    config.AddOption( "Hard drive quantity",            ConfigType::Int    );

    resPrinter  = new ResourcePrinter(  this, config.GetInt( "Printer quantity" ),      config.GetInt( "Printer cycle time (msec)" ) );
    resHdd      = new ResourceHDD(      this, config.GetInt( "Hard drive quantity" ),   config.GetInt( "Hard drive cycle time (msec)" ) );
    resSpeaker  = new ResourceSpeaker(  this, config.GetInt( "Speaker quantity" ),      config.GetInt( "Speaker cycle time (msec)" ) );
    resMonitor  = new ResourceMonitor(  this, config.GetInt( "Monitor display time (msec)" ) );
    resKeyboard = new ResourceKeyboard( this, config.GetInt( "Keyboard cycle time (msec)" ) );
    resMouse    = new ResourceMouse(    this, config.GetInt( "Mouse cycle time (msec)" ) );
}

void Simulation::ReadMetaData( )
{
    const string mdFile = config.GetStr("File Path");
    const string mdHeader = "Start Program Meta-Data Code:";
    const string mdFooter = "End Program Meta-Data Code.";
    
    fstream  fl;
    fl.open( mdFile, ios::in );
    if(!fl.is_open())    
        throw SimError( "Unable to open meta-data file: %s", mdFile.c_str() );
    
    string line;

    // Extract lines between meta-data header and footer
    string mdStr = "";
        
    while(getline(fl, line))
    {
        line = strTrim(line);
        if(line == mdHeader);
            break;
    }
    if( line != mdHeader )
        throw SimError( "Meta-Data header is missing!" );
    
    
    while(getline(fl, line))
    {
        line = strTrim(line);
        if(line == mdFooter)
            break;
        mdStr += line;
    }
    fl.close();

    if( line != mdFooter )
        throw SimError( "Meta-Data footer is missing!" );
    
    if( mdStr.back() != '.' )
        throw SimError( "Meta-Data is missing period at the end of events!" );
    mdStr.pop_back();
    
    // Read meta-data events and populate queue
    vector<string> tokens;
    strSplit( mdStr, ';', tokens );
    
    regex rEvent(R"(^\s*([A-Z])\s*\(\s*([a-z\s]*)\s*\)\s*(\d+)\s*$)");
    smatch sm;

    currentApplication = NULL;
    osRunning = false;

    string eventDescriptor;
    for( auto it = tokens.begin(); it != tokens.end(); it++)
    {
        char eventCode;
        long int eventCycles;

        if( !regex_search(*it, sm, rEvent) )
            throw SimError("Unable to parse following event: %s", (*it).c_str());
        eventCode = sm.str(1)[0];
        eventDescriptor = sm.str(2);
        eventCycles = strtol(sm.str(3).c_str(), NULL, 10);

        AddEvent( eventCode, eventDescriptor, eventCycles );

    }
    if(currentApplication)
        throw SimError( "Missing meta-data to end last process." );
    if(osRunning)
        throw SimError( "Missing meta-data to end OS." );
}

void Simulation::AddEvent( char code, string &descriptor, long int cycles )
{
    // Check if event is valid
    unordered_set<string> validDescriptors;
    switch(code)
    {
        case 'S': validDescriptors = { "start", "end" }; break;
        case 'A': validDescriptors = { "start", "end" }; break;
        case 'P': validDescriptors = { "run" }; break;
        case 'I': validDescriptors = { "hard drive", "keyboard", "mouse" }; break;
        case 'O': validDescriptors = { "hard drive", "monitor", "speaker", "printer" }; break;
        case 'M': validDescriptors = { "block", "allocate" }; break;
        default:
            throw SimError( "%c(%s)%ld Unknown event code for meta-data event.", code, descriptor.c_str(), cycles );
    }
    if( validDescriptors.find(descriptor) == validDescriptors.end() )
        throw SimError( "%c(%s)%ld Invalid descriptor for meta-data event.", code, descriptor.c_str(), cycles );
    if( cycles < 0 )
        throw SimError( "%c(%s)%ld Invalid cycles for meta-data event.", code, descriptor.c_str(), cycles );
    SimEvent event = {code, descriptor, cycles};

    // Process the event
    switch(code)
    {
        case 'S': 
            if( event.descriptor == "start" && osRunning )
                throw SimError( "%c(%s)%ld Attempt to start OS while OS its already running!", event.code, event.descriptor.c_str(), event.cycles );
            else if( event.descriptor == "end" && !osRunning )
                throw SimError( "%c(%s)%ld Attempt to stop OS while OS its already stopped!", event.code, event.descriptor.c_str(), event.cycles );
            osRunning = event.descriptor == "start";
            break;
        case 'A': validDescriptors = { "start", "end" }; 
            if( !osRunning )
                throw SimError( "%c(%s)%ld Attempt to %s application without OS!", event.code, event.descriptor.c_str(), event.cycles, event.descriptor.c_str() );
            if( event.descriptor == "start" )
            {
                if(currentApplication)
                    throw SimError( "%c(%s)%ld Attempt to start new application within running application!", event.code, event.descriptor.c_str(), event.cycles );
                
                currentApplication = new Application();
            }
            else if( event.descriptor == "end" )
            {
                if(!currentApplication)
                    throw SimError( "%c(%s)%ld Attempt to stop non-existing application!", event.code, event.descriptor.c_str(), event.cycles );
                applications.push_back(currentApplication);
                currentApplication = NULL;
            }
            break;
        case 'P':
        case 'I':
        case 'O':
        case 'M':
            if( !currentApplication )
                throw SimError( "%c(%s)%ld Attempt to execute outside of application.", event.code, event.descriptor.c_str(), event.cycles );
            currentApplication->push_back(event);
            break;
        default:
            throw SimError( "%c(%s)%ld Unknown event code for meta-data event.", code, descriptor.c_str(), cycles );
    }
}

void Simulation::doWork( long int ms )
{
    auto t_end = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds( ms );
    while(std::chrono::high_resolution_clock::now() < t_end);
}
long int Simulation::doProcWork( long int ms )
{
    auto t_end = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds( ms );
    while(std::chrono::high_resolution_clock::now() < t_end)
    {
        if(simInterrupt)
        {
            long int timeRemaining = std::chrono::duration_cast<std::chrono::milliseconds>(t_end-std::chrono::high_resolution_clock::now()).count();
            return timeRemaining > 0 ? timeRemaining : 0;
        }
    }
    return 0;
}

float Simulation::simTime()
{
    auto simCurrentTime = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(simCurrentTime - simStartTime).count() / 1e6;
}

void Simulation::simResetTimer()
{
    simStartTime = std::chrono::high_resolution_clock::now();
}

void Simulation::handleProc( unsigned int pid, const SimEvent &event )
{
    PCB *process = processes[pid];
    long int event_time;
    if(process->eventInProgress){
        event_time = process->eventTimeRemaining;
    }else{
        event_time = event.cycles * config.GetInt( "Processor cycle time (msec)" );
        Log( "%lf - Process %d: start processing action\n", simTime(), pid);
    }
    

    long int timeRemaining = doProcWork(event_time);

    if(simInterrupt){
        process->eventInProgress = true;
        process->eventTimeRemaining = timeRemaining;

        Log( "%lf - Process %d: interrupt processing action\n", simTime() , pid);
    }else{
        Log( "%lf - Process %d: end processing action\n", simTime(), pid );
        process->eventInProgress = false;
        process->eventQueue.pop_front();
    }
    process->state = ProcessState::READY;
}

void Simulation::handleMem( unsigned int pid, const SimEvent &event )
{
    PCB *process = processes[pid];
    long int event_time;

    if(process->eventInProgress){
        event_time = process->eventTimeRemaining;
    }else{
        event_time = event.cycles * config.GetInt( "Memory cycle time (msec)" );
    }

    long int timeRemaining;

    if( event.descriptor == "allocate" )
    {
        if(!process->eventInProgress)
            Log( "%lf - Process %d: allocating memory\n", simTime(), pid );

        timeRemaining = doProcWork(event_time);

        if(!simInterrupt){
            unsigned int newMemory = allocateMemory( 1 );
            Log( "%lf - Process %d: memory allocated at 0x%08x\n", simTime(), pid, newMemory );
        }
    }
    else if( event.descriptor == "block" )
    {
        if(!process->eventInProgress)
            Log( "%lf - Process %d: start memory blocking\n", simTime(), pid );

        timeRemaining = doProcWork(event_time);

        if(!simInterrupt)
            Log( "%lf - Process %d: end memory blocking\n", simTime(), pid );
    }


    if(simInterrupt){
        Log( "%lf - Process %d: interrupt processing action\n", simTime() , pid);
        process->eventInProgress = true;
        process->eventTimeRemaining = timeRemaining;
    }else{
        process->eventInProgress = false;
        process->eventQueue.pop_front();
    }
    process->state = ProcessState::READY;
}

void Simulation::handleIO( unsigned int pid, const SimEvent &event )
{
    PCB *process = processes[pid];
    if(process->eventInProgress){
        process->eventInProgress = false;
        process->eventQueue.pop_front();
        process->state = ProcessState::READY;
    }else{
        ResourceIO *resource = NULL; 

        if( event.descriptor == "hard drive" )
            resource = resHdd;
        else if( event.descriptor == "monitor" )
            resource = resMonitor;
        else if( event.descriptor == "printer" )
            resource = resPrinter;
        else if( event.descriptor == "keyboard" )
            resource = resKeyboard;
        else if( event.descriptor == "mouse" )
            resource = resMouse;
        else if( event.descriptor == "speaker" )
            resource = resSpeaker;

        bool resource_retrieved = false;
        ResIOState resource_io = event.code == 'I' ? INPUT : OUTPUT;
        while(!resource_retrieved && !simInterrupt)
            resource_retrieved = resource->run( event.cycles, resource_io, pid );

        if(resource_retrieved){
            process->eventInProgress = true;
            process->state = ProcessState::WAITING; 
        }
    }
}

unsigned long Simulation::GetRemainingTime( unsigned int pid )
{
    unsigned long remaining_time = 0;
    PCB * process = processes[pid];
    for(auto it = process->eventQueue.begin(); it != process->eventQueue.end(); ++it) {
        // I wasn't sure how its bein calculated, so made 2 version. 

        ////////////////////////////////////////////////////////////////////////
        // Version 1 : Acts as SJF from project 4, 1 task is 1 time unit 
        
        ++remaining_time;

        ////////////////////////////////////////////////////////////////////////
        // Version 2 : Computes remaining time in ms of P and M tasks, doesn't
        //             include IO cause that doesn't effect processing time.
        /*
        if(it == process->eventQueue.begin() && process->eventInProgress){
            remaining_time += process->eventTimeRemaining;
        }else{
            switch(event.code)
            {
                case 'P': remaining_time += event.cycles * config.GetInt( "Processor cycle time (msec)" ); break;
                case 'M': remaining_time += event.cycles * config.GetInt( "Memory cycle time (msec)" ); break;
            }
        }
        */
    }

    return remaining_time;
}

void * Simulation::JobLoader( void * simPtr )
{
    Simulation *sim = (Simulation *)simPtr;
    std::vector<Application *> *applications = &(sim->applications);
    for( size_t i = 0; i<10; ++i ){
        if( i != 0 ) // Wait 100ms
            sim->doWork(100);
        
        sim->simInterrupt |= SIM_INTERRUPT_LOADER;
        pthread_mutex_lock(&(sim->simMutex));

        // Load applications
        for(auto it = applications->begin(); it != applications->end(); ++it) {
            unsigned int newPid = sim->processCounter++;

            sim->Log( "%lf - OS: preparing process %u\n", sim->simTime(), newPid );
            
            PCB * newProcess = new PCB();
            newProcess->state = ProcessState::START;
            newProcess->pid = newPid;
            newProcess->eventQueue = **it;
            newProcess->eventInProgress = false;
            newProcess->eventTimeRemaining = 0;
            
            if( newPid >= sim->processes.size() ){
                sim->processes.resize(sim->processes.size()*2);
            }
            sim->processes[newPid] = newProcess;
            
            int process_priority = 0;
            if(sim->scheduling == SchedulingCode::SRTF)
                process_priority = sim->GetRemainingTime(newPid);

            sim->jobs.push(Job{newPid, process_priority});
        }

        sim->simInterrupt &= ~SIM_INTERRUPT_LOADER;
        pthread_mutex_unlock(&(sim->simMutex));
    }

    sim->loaderFinished = true;
    return NULL;
}

void * Simulation::SchedulerRR( void * simPtr )
{
    Simulation *sim = (Simulation *)simPtr;
    while(true){
        sim->doWork(50);
        sim->simInterrupt |= SIM_INTERRUPT_SCHEDULER_RR;
    }

    return NULL;
}

void Simulation::Run()
{
    Simulation::simResetTimer();
    Log( "%lf - Simulator program starting\n", simTime() );

    // Prepare the simulation for execution
    processCounter = 0;
    loaderFinished = false;
    simInterrupt = 0;

    pthread_t schedulerThread;
    pthread_t loaderThread;
    int rc;
    rc = pthread_create(&loaderThread, NULL, Simulation::JobLoader, this);
    if( rc ) throw SimError( "Unable to create loader thread, error code (%d).", rc );

    // Initiate round robin thread, if scheduling is set
    if(scheduling == SchedulingCode::RR)
    {
        int rc = pthread_create(&schedulerThread, NULL, Simulation::SchedulerRR, this);
        if( rc ) throw SimError( "Unable to create loader thread, error code (%d).", rc );
    }    


    // Execute the simulation
    while(!loaderFinished || !jobs.empty())
    {
        pthread_mutex_lock(&simMutex);
        while(!jobs.empty() && !(simInterrupt & SIM_INTERRUPT_LOADER))
        {   
            // Pop next process from scheduling queue
            Job job = jobs.top();
            jobs.pop();

            unsigned int pid = job.pid;
            PCB *process = processes[pid];
            currentProcess = pid;
            ProcessState state = process->state;

            // If the process is newly created, set it to ready
            if(state == ProcessState::START)
                process->state = ProcessState::READY;

            // Start of process execution
            if(state == ProcessState::READY)
                RunProcess(pid);
            state = process->state;
            // End of process execution

            // Clear RR flag, since process is already been interrupted
            simInterrupt &= !SIM_INTERRUPT_SCHEDULER_RR;


            // Readd the process into scheduling queue
            if(state != ProcessState::EXIT){
                int process_priority = 0;
                if(scheduling == SchedulingCode::SRTF)
                    process_priority = -GetRemainingTime(pid);
                jobs.push(Job{pid, process_priority});
            }
        }
        pthread_mutex_unlock(&simMutex);
        while(simInterrupt & SIM_INTERRUPT_LOADER);
    }
    pthread_join(loaderThread, NULL);
    Log( "%lf - Simulator program ending\n", simTime() );
}

void Simulation::RunProcess( unsigned int pid )
{
    Log( "%lf - OS: starting process %d\n", simTime(), pid );

    PCB *process = processes[pid];
    process->state = ProcessState::RUNNING;
    while (!process->eventQueue.empty())
    {
        SimEvent event = process->eventQueue.front();
        
        switch(event.code)
        {
            case 'P':
                handleProc( pid, event );
                break;
            case 'M':
                handleMem( pid, event );
                break;
            case 'I':
            case 'O':
                handleIO( pid, event );
                break;
            default:
                continue;
        }
        if(simInterrupt || process->state == ProcessState::WAITING)
            break;
    }
    
    if(process->eventQueue.empty()){
        // Remove Process
        Log( "%lf - Process %d completed\n", 
            simTime(), 
            pid );
        processes[pid]->state = ProcessState::EXIT;
    }
}

unsigned int Simulation::allocateMemory( int totMem )
{
    unsigned int blockSize = config.GetInt( "Memory block size (kbytes)" );
    unsigned int requiredBlocks = (totMem) / blockSize;
    if( blockSize * requiredBlocks < (unsigned int)totMem)
        ++requiredBlocks;

    if( memoryBlockCounter + requiredBlocks >= maxMemoryBlocks)
        memoryBlockCounter = 0;

    unsigned int address = memoryBlockCounter * blockSize;
    memoryBlockCounter += requiredBlocks;

    return address;
}
