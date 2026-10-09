CC = g++
CFLAGS = -Wall -g -Isrc

SRC = src/Way.cpp src/Cache.cpp src/MemoryHierarchy.cpp src/SimulationResults.cpp src/PrefetchUnit.cpp

all: sim

sim: src/sim.cc $(SRC)
	$(CC) $(CFLAGS) src/sim.cc $(SRC) -o sim