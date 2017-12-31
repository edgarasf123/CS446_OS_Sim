CC = g++
DEBUG = -g
CFLAGS = -Wall -c -std=c++11 $(DEBUG)
LFLAGS = -Wall -pthread -std=c++11 $(DEBUG)
OBJS = Sim05

all: clean $(OBJS)

Sim05 : main.o Simulation.o ConfigManager.o ResourceIO.o
	$(CC) $(LFLAGS) main.o Simulation.o ConfigManager.o ResourceIO.o -o Sim05

main.o : main.cpp
	$(CC) $(CFLAGS) main.cpp

Simulation.o : Simulation.cpp Simulation.h helpers.h ConfigManager.h ResourceIO.h
	$(CC) $(CFLAGS) Simulation.cpp

ConfigManager.o : ConfigManager.cpp ConfigManager.h
	$(CC) $(CFLAGS) ConfigManager.cpp

ResourceIO.o : ResourceIO.cpp ResourceIO.h
	$(CC) $(CFLAGS) ResourceIO.cpp

clean:
	rm -f *.o $(OBJS)
    
