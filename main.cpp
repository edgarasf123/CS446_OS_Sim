#include "Simulation.h"

#include <cstdlib>
#include <iostream>
#include <cstdarg>
#include <exception>

using std::cout;
using std::endl;
using std::max;
using std::min;

/**
 * @brief Outputs error and halts the program.
 * @details Takes format and varargs to construct a message string.
 *          It uses same format as printf to generate output.
 *          
 * @param format,... Structure of error output followed by arguments specified in structure.
 * 
 */
void program_error( char const * format, ... )
    __attribute__ ((format(printf, 1, 2)));

void program_error( char const * format, ... )
{	
	va_list args;
	va_start (args, format);
	vfprintf (stderr, format, args);
	va_end (args);
	fprintf(stderr, "\n");

    exit(1);
}

/**
 * @brief Main function, initializes Simulation and runs it.
 */
int main( int argc, char *argv[] )
{
    try
    {
        if(argc < 2)
            throw std::invalid_argument( "Supply configuration file as first argument!" );
        
        char * configFile = argv[1];

        Simulation s(configFile);
        s.Run();
    }
    catch(const SimError& e)
    {
        program_error("Simulation error: %s", e.what());
    }
    catch(const std::exception& e)
    {
        program_error("Error: %s", e.what());
    }
    
    return 0;
}
