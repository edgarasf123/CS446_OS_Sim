#ifndef _SIMULATION
#define _SIMULATION

#include "ConfigManager.h"
#include "ResourceIO.h"

#include <string>
#include <queue>
#include <deque>
#include <exception>
#include <cstdarg>
#include <fstream>
#include <chrono>

#include <pthread.h>
#include <atomic>

#define SIM_INTERRUPT_LOADER 0b00000001
#define SIM_INTERRUPT_SCHEDULER_RR 0b00000010


/**
 * @brief An exception class used by Simulation class.
 * @details Unlike exception provided by standard library,
 *          this exception class allows for the message to
 *          be formated using printf style.
 * 
 * @param format,... Structure of log output followed by arguments specified in structure.
 * 
 * @return Exception class
 */
class SimError : std::exception
{
    char msg[1024];
    
    public:
        /**
         * @brief Constructor for SimError exception class.
         * @details Takes format and varargs to construct a message string.
         * 
         * @param format,... Structure of log output followed by arguments specified in structure.
         */
        SimError( char const * format, ... )
            __attribute__ ((format(printf, 2, 3)));
        /**
         * @brief Returns exception message
         * @return Exception message.
         */
        char const * what() const throw ();
};

/**
 * @brief A parsed structure of metadata unit.
 * 
 */
struct SimEvent
{
    char code;
    std::string descriptor;
    long int cycles;
};
typedef std::deque<SimEvent> Application;

/**
 * @brief Scheduling enumeration.
 * 
 */
enum class SchedulingCode{
    RR, SRTF
};

/**
 * @brief Process state enumeration used in PCB structure.
 * 
 */
enum class ProcessState{
    START, READY, RUNNING, WAITING, EXIT
};

/**
 * @brief Holds information about process in simulation.
 * 
 */
struct PCB{
    std::atomic<ProcessState> state;
    unsigned int pid;
    Application eventQueue;
    bool eventInProgress;
    unsigned long eventTimeRemaining;
};

/**
 * @brief Used by scheduling (priority) queue to store process id's 
 * 
 */
struct Job
{
    unsigned int pid;
    int priority;
    bool operator<(const Job &rhs) const
    {
        return priority < rhs.priority;
    }
};


class Simulation
{
    public:
        unsigned int currentProcess;
        unsigned int processCounter;
        std::vector<PCB *> processes;
        std::priority_queue<Job> jobs;
        
        /**
         * @brief Constructor for Simulation.
         * @details Initializes simulation variables, including configuration values.
         * 
         * @param configFile Filename of simulation config file
         */
        Simulation( const std::string &configFile );

        /**
         * @brief Destructor for Simulation class.
         * @details Checks whether log file is open or not. If open, it closes it.
         * 
         */
        ~Simulation( );

        /**
         * @brief Runs the simulation from the eventQueue.
         * @details Runs the simulation from the eventQueue. As for assignment 1 specifications,
         *          it will only measure cycle times for each event.
         * 
         */
        void Run( );

        /**
         * @brief Runs simulation on a process
         * 
         * @param process Pointer to the process PCB
         */
        void RunProcess( unsigned int pid );

        /**
         * @brief Function used by simulation to log.
         * @details Depending on the config, the log will output to monitor, file, or both.
         *          It uses same format as printf, except it outputs to config specified
         *          location (monitor/file).
         *          
         * @param format,... Structure of log output followed by arguments specified in structure.
         */
        void Log( char const * format, ... )
            __attribute__ ((format(printf, 2, 3)));;

        /**
         * @brief Returns current simulation time.
         * @return Simulation time in seconds.
         */
        float simTime();

    private:
        SchedulingCode scheduling;

        std::unordered_map<std::string, std::string> configKeyValues;
        ConfigManager config;

        std::fstream  logFile;
        bool logToFile = false;
        bool logToMonitor = false;
        
        Application * currentApplication;
        std::vector<Application *> applications;
        bool osRunning = false;

        unsigned int memoryBlockCounter;
        unsigned int maxMemoryBlocks;

        std::chrono::high_resolution_clock::time_point simStartTime;

        ResourceHDD         *resHdd;
        ResourcePrinter     *resPrinter;
        ResourceMonitor     *resMonitor;
        ResourceKeyboard    *resKeyboard;
        ResourceMouse       *resMouse;
        ResourceSpeaker     *resSpeaker;

        pthread_mutex_t logMutex;
        pthread_mutex_t simMutex;
        std::atomic<bool> loaderFinished;
        std::atomic<unsigned short> simInterrupt;

        unsigned long GetRemainingTime( unsigned int processId );
        
        /**
         * @brief A threaded loader function, loads new processes into simulation
         *        ten times, every 100ms.
         * 
         * @param simPtr Pointer to simulation object.
         * @return NULL
         */
        static void * JobLoader( void * simPtr );
        
        /**
         * @brief A threaded scheduling function, sets SIM_INTERRUPT_SCHEDULER_RR
         *        flag every 50ms. 
         * 
         * @param simPtr Pointer to simulation object.
         * @return NULL
         */
        static void * SchedulerRR( void * simPtr );

        /**
         * @brief Does simulation work.
         * @details Basically while loop that checks time elapsed.
         * 
         * @param ms miliseconds to do work for.
         */
        void doWork( long int ms );

        /**
         * @brief Does simulation work for processes.
         * @details Similarly to doWork, but can be interrupted.
         * 
         * @param ms miliseconds to do work for.
         * 
         * @return remaining time in ms
         */
        long int doProcWork( long int ms );

        /**
         * @brief Resets simulation timer.
         */
        void simResetTimer();

        /**
         * @brief Reads the configuration file.
         * @details Reads configuration files an populates configKeyValues map.
         *          It doesn't check if individual configuration values are valid
         *          or not.
         * 
         * @param configFile Location of configuration file.
         */
        void ReadConfigFile( const std::string &configFile );

        /**
         * @brief Loads configuration into simulation.
         * 
         * @details Loads individual configuration values from configKeyValues map into
         *          simulation. It checks whether config values are valid or not. And 
         *          opens log file for writing if specified by the config.
         * 
         */
        void LoadConfig( );

        /**
         * @brief Loads meta-data into a queue.
         * @details Reads the meta-data from a file, specified by configuration, and loads it into a eventQueue.
         * 
         */
        void ReadMetaData( );

        /**
         * @brief Adds meta-data event to a eventQueue.
         * @details Adds meta-data event to a eventQueue. Other than adding the event into a queue, 
         *          it also checks if descriptor and cycles are valid.
         * 
         * @param code Event code.
         * @param descriptor Event descriptor.
         * @param cycles Event cycles.
         */
        void AddEvent( char code, std::string &descriptor, long int cycles );

        /**
         * @brief Processes processor event
         * @param event Event data.
         */
        void handleProc( unsigned int pid, const SimEvent &event  );

        /**
         * @brief Processes memory event
         * @param event Event data.
         */
        void handleMem( unsigned int pid, const SimEvent &event  );

        /**
         * @brief Processes IO event
         * @param event Event data.
         */
        void handleIO( unsigned int pid, const SimEvent &event  );
        
        /**
         * @brief Assigns memory and returns address
         * @param totMem amount of memory in kb
         * @return Memory address
         */
        unsigned int allocateMemory( int totMem );
};

#endif // _SIMULATION
