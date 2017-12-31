#include "ResourceIO.h"
#include "Simulation.h"

#include <chrono>
#include <stdexcept>
#include <string>
#include <cstring>

void *ResourceIO::doWork( void *p )
{
    ResIOThreadParams *params = (ResIOThreadParams *)p;

    Simulation *sim = params->sim;
    unsigned long time = params->time;
    unsigned int pid = params->pid;
    char *device_str = params->deviceStr;

    auto t_end = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds( time );
    while(std::chrono::high_resolution_clock::now() < t_end);

    if(sim->processes[pid]->state == ProcessState::WAITING)
        sim->processes[pid]->state = ProcessState::READY;

    sim->Log( "%lf - Process %d: end %s\n", sim->simTime(), pid, device_str );

    delete params;
    return NULL;
}

void ResourceIO::run( unsigned int cycles, unsigned int pid, const char * device_str )
{
    ResIOThreadParams *params = new ResIOThreadParams();

    params->sim = sim;
    params->time = cycleTime * cycles;
    params->pid = pid;
    strncpy ( params->deviceStr, device_str, 32 );

    sim->Log( "%lf - Process %d: start %s\n", sim->simTime(), pid, device_str );

    pthread_t ioThread;
    int rc = pthread_create(&ioThread, NULL, doWork, params);
    if( rc )
        throw std::runtime_error( "Unable to create IO thread, error code (" + std::to_string(rc) +")." );
    //pthread_join(ioThread, NULL);
}


////////////////////////////////////////////////////////////////////////////////

IOResourceSemaphore::IOResourceSemaphore( unsigned int count ):
	deviceCount(count),
	deviceIndex(0)
{
	sem_init(&s, 0, count);
    pthread_mutex_init(&update_mutex, NULL);	
}
IOResourceSemaphore::~IOResourceSemaphore()
{
	sem_destroy(&s);
    pthread_mutex_destroy(&update_mutex);	
}

////////////////////////////////////////////////////////////////////////////////

IOResourceMutex::IOResourceMutex()
{
    pthread_mutex_init(&m, NULL);	
}
IOResourceMutex::~IOResourceMutex()
{
    pthread_mutex_destroy(&m);	
}

////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////

ResourceHDD::ResourceHDD(Simulation *sim, unsigned int count, unsigned int cycleTime) :
	IOResourceSemaphore( count )
{
	ResourceIO::cycleTime = cycleTime,
	ResourceIO::sim = sim;
}

bool ResourceHDD::run( unsigned int cycles, ResIOState ioState, unsigned int pid )
{
    if(sem_trywait(&s) == 0){
        // Semaphore retrieved
    	pthread_mutex_lock(&update_mutex);
        int dev_id = deviceIndex;
        deviceIndex = (deviceIndex+1)%deviceCount;
        pthread_mutex_unlock(&update_mutex);

        char device_str[32];
        sprintf (device_str, "hard drive %s on HDD %d", ioState == INPUT ? "input" : "output", dev_id);
        ResourceIO::run( cycles, pid, device_str );

        sem_post(&s);
        return true;
    }
    return false;
}
////////////////////////////////////////////////////////////////////////////////

ResourcePrinter::ResourcePrinter(Simulation *sim, unsigned int count, unsigned int cycleTime) :
	IOResourceSemaphore( count )
{
	ResourceIO::cycleTime = cycleTime,
	ResourceIO::sim = sim;
}

bool ResourcePrinter::run( unsigned int cycles, ResIOState ioState, unsigned int pid )
{
    if(sem_trywait(&s) == 0){
        // Semaphore retrieved
        pthread_mutex_lock(&update_mutex);
        int dev_id = deviceIndex;
        deviceIndex = (deviceIndex+1)%deviceCount;
        pthread_mutex_unlock(&update_mutex);

        char device_str[32];
        sprintf (device_str, "printer output on PRNTR %d", dev_id);
        ResourceIO::run( cycles, pid, device_str );

        sem_post(&s);
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
ResourceSpeaker::ResourceSpeaker(Simulation *sim, unsigned int count, unsigned int cycleTime) :
    IOResourceSemaphore( count )
{
    ResourceIO::cycleTime = cycleTime,
    ResourceIO::sim = sim;
}
bool ResourceSpeaker::run( unsigned int cycles, ResIOState ioState, unsigned int pid )
{
    if(sem_trywait(&s) == 0){
        // Semaphore retrieved
        pthread_mutex_lock(&update_mutex);
        int dev_id = deviceIndex;
        deviceIndex = (deviceIndex+1)%deviceCount;
        pthread_mutex_unlock(&update_mutex);

        char device_str[32];
        sprintf (device_str, "speaker output on SPKR %d", dev_id);
        ResourceIO::run( cycles, pid, device_str );

        sem_post(&s);
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
ResourceMonitor::ResourceMonitor(Simulation *sim, unsigned int cycleTime) :
	IOResourceMutex()
{
	ResourceIO::cycleTime = cycleTime,
	ResourceIO::sim = sim;
}
bool ResourceMonitor::run( unsigned int cycles, ResIOState ioState, unsigned int pid )
{
    if(pthread_mutex_trylock(&m) == 0)
    {
        // Mutex retrieved
        ResourceIO::run( cycles, pid, "monitor output" );
        pthread_mutex_unlock(&m);
        return true;
    }
    return false;
}
////////////////////////////////////////////////////////////////////////////////
ResourceKeyboard::ResourceKeyboard(Simulation *sim, unsigned int cycleTime) :
	IOResourceMutex()
{
	ResourceIO::cycleTime = cycleTime,
	ResourceIO::sim = sim;
}
bool ResourceKeyboard::run( unsigned int cycles, ResIOState ioState, unsigned int pid )
{
    if(pthread_mutex_trylock(&m) == 0)
    {
        // Mutex retrieved
        ResourceIO::run( cycles, pid, "keyboard input" );
        pthread_mutex_unlock(&m);
        return true;
    }
    return false;
}
////////////////////////////////////////////////////////////////////////////////

ResourceMouse::ResourceMouse(Simulation *sim, unsigned int cycleTime) :
	IOResourceMutex()
{
	ResourceIO::cycleTime = cycleTime,
	ResourceIO::sim = sim;
}
bool ResourceMouse::run( unsigned int cycles, ResIOState ioState, unsigned int pid )
{
    if(pthread_mutex_trylock(&m) == 0)
    {
        // Mutex retrieved
        ResourceIO::run( cycles, pid, "mouse input" );
        pthread_mutex_unlock(&m);
        return true;
    }
    return false;
}