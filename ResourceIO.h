#ifndef _IO_RESOURCE
#define _IO_RESOURCE

#include <pthread.h>
#include <semaphore.h>

class Simulation;

enum ResIOState { INPUT, OUTPUT };
struct ResIOThreadParams
{
	Simulation *sim;
	unsigned long time;
	unsigned int pid;
	char deviceStr[32];
};


/**
 * @brief Generic resource class shared by all resources.
 * 
 */
class ResourceIO
{
	protected:
		Simulation *sim;
		unsigned int cycleTime;
        /**
         * @brief pthread function to do simulation work.
         * 
         * @param arg pointer to long int variable.
         * @return always returns NULL.
         */
        static void *doWork( void *ms );

        /**
         * @brief Runs resource for given cycles
         * 
         * @param int Number of cycles to run.
         */
    	void run( unsigned int cycles, unsigned int pid, const char * device_str );
	private:
    public:
    	
    	/**
    	 * @brief Public pure virtual function, used by Simulation to run cycles on resource.
    	 * 
    	 * @param int Number of cycles to run.
    	 * @param ioState Whether resource is used as INPUT or OUTPUT.
	 	 * @param pid The process which inquiries IO resource.
		 * 
		 * @return Returns true if resource assigned.
    	 */
    	virtual bool run( unsigned int cycles, ResIOState ioState, unsigned int pid ) = 0;
};


////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Resource implementation of semaphores.
 * 
 */
class IOResourceSemaphore : public ResourceIO
{
private:
protected:
	sem_t s;
	pthread_mutex_t update_mutex;
	unsigned int deviceCount;
	unsigned int deviceIndex;
public:
	/**
	 * @brief Constructor for IOResourceSemaphore.
	 * @param int Number of devices available in resource.
	 */
	IOResourceSemaphore( unsigned int count );
	~IOResourceSemaphore();
};


/**
 * @brief Resource implementation of mutex.
 * 
 */
class IOResourceMutex : public ResourceIO
{
protected:
	pthread_mutex_t m;
public:
	IOResourceMutex();
	~IOResourceMutex();
};




////////////////////////////////////////////////////////////////////////////////

/**
 * @brief HDD resource class.
 * 
 */
class ResourceHDD final : public IOResourceSemaphore
{
public:
	/**
	 * @brief Constructor of ResourceHDD.
	 * 
	 * @param sim Pointer to Simulation instance.
	 * @param count Number of devices.
	 * @param cycleTime Time in ms that 1 cycle takes.
	 */
	ResourceHDD(Simulation *sim, unsigned int count, unsigned int cycleTime);
	/**
	 * @brief Implementation of virtual function of ResourceIO. Runs resource for given cycles.
	 * 
	 * @param int Number of cycles to run.
	 * @param ioState Whether resource is used as INPUT or OUTPUT.
	 * 
	 * @return Returns true if resource assigned.
	 */
	bool run( unsigned int cycles, ResIOState ioState, unsigned int pid );
};

/**
 * @brief Printer resource class.
 * 
 */
class ResourcePrinter final : public IOResourceSemaphore
{
public:
	/**
	 * @brief Constructor of ResourcePrinter.
	 * 
	 * @param sim Pointer to Simulation instance.
	 * @param count Number of devices.
	 * @param cycleTime Time in ms that 1 cycle takes.
	 */
	ResourcePrinter(Simulation *sim, unsigned int count, unsigned int cycleTime);
	/**
	 * @brief Implementation of virtual function of ResourceIO. Runs resource for given cycles.
	 * 
	 * @param int Number of cycles to run.
	 * @param ioState Whether resource is used as INPUT or OUTPUT.
	 * @param pid The process which inquiries IO resource.
	 * 
	 * @return Returns true if resource assigned.
	 */
	bool run( unsigned int cycles, ResIOState ioState, unsigned int pid );
};

/**
 * @brief Speaker resource class.
 * 
 */
class ResourceSpeaker final : public IOResourceSemaphore
{
public:
	/**
	 * @brief Constructor of ResourceSpeaker.
	 * 
	 * @param sim Pointer to Simulation instance.
	 * @param count Number of devices.
	 * @param cycleTime Time in ms that 1 cycle takes.
	 */
	ResourceSpeaker(Simulation *sim, unsigned int count, unsigned int cycleTime);
	/**
	 * @brief Implementation of virtual function of ResourceIO. Runs resource for given cycles.
	 * 
	 * @param int Number of cycles to run.
	 * @param ioState Whether resource is used as INPUT or OUTPUT.
	 * @param pid The process which inquiries IO resource.
	 * 
	 * @return Returns true if resource assigned.
	 */
	bool run( unsigned int cycles, ResIOState ioState, unsigned int pid );
};

/**
 * @brief Monitor resource class.
 * 
 */
class ResourceMonitor final : public IOResourceMutex
{
public:
	/**
	 * @brief Constructor of ResourceMonitor.
	 * 
	 * @param sim Pointer to Simulation instance.
	 * @param cycleTime Time in ms that 1 cycle takes.
	 */
	ResourceMonitor(Simulation *sim, unsigned int cycleTime);
	/**
	 * @brief Implementation of virtual function of ResourceIO. Runs resource for given cycles.
	 * 
	 * @param int Number of cycles to run.
	 * @param ioState Whether resource is used as INPUT or OUTPUT.
	 * @param pid The process which inquiries IO resource.
	 * 
	 * @return Returns true if resource assigned.
	 */
	bool run( unsigned int cycles, ResIOState ioState, unsigned int pid );
};

/**
 * @brief Keyboard resource class.
 * 
 */
class ResourceKeyboard final : public IOResourceMutex
{
public:
	/**
	 * @brief Constructor of ResourceKeyboard.
	 * 
	 * @param sim Pointer to Simulation instance.
	 * @param cycleTime Time in ms that 1 cycle takes.
	 */
	ResourceKeyboard(Simulation *sim, unsigned int cycleTime);
	/**
	 * @brief Implementation of virtual function of ResourceIO. Runs resource for given cycles.
	 * 
	 * @param int Number of cycles to run.
	 * @param ioState Whether resource is used as INPUT or OUTPUT.
	 * @param pid The process which inquiries IO resource.
	 * 
	 * @return Returns true if resource assigned.
	 */
	bool run( unsigned int cycles, ResIOState ioState, unsigned int pid );
};

/**
 * @brief Mouse resource class.
 * 
 */
class ResourceMouse final : public IOResourceMutex
{
public:
	/**
	 * @brief Constructor of ResourceMouse.
	 * 
	 * @param sim Pointer to Simulation instance.
	 * @param cycleTime Time in ms that 1 cycle takes.
	 */
	ResourceMouse(Simulation *sim, unsigned int cycleTime);
	/**
	 * @brief Implementation of virtual function of ResourceIO. Runs resource for given cycles.
	 * 
	 * @param int Number of cycles to run.
	 * @param ioState Whether resource is used as INPUT or OUTPUT.
	 * @param pid The process which inquiries IO resource.
	 * 
	 * @return Returns true if resource assigned.
	 */
	bool run( unsigned int cycles, ResIOState ioState, unsigned int pid );
};


#endif // _IO_RESOURCE
