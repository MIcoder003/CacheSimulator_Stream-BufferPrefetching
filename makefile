CC = g++
CFLAGS = -Wall -g -Isrc

SRC = src/Way.cpp src/Cache.cpp src/MemoryHierarchy.cpp src/SimulationResults.cpp src/PrefetchUnit.cpp

all: sim

sim: src/sim.cc $(SRC)
	$(CC) $(CFLAGS) src/sim.cc $(SRC) -o sim

tests: test_way test_cache test_two_levels test_mh

test_way: tests/test_way.cpp src/Way.cpp
	$(CC) $(CFLAGS) tests/test_way.cpp src/Way.cpp -o test_way

test_cache: tests/test_cache.cpp src/Cache.cpp src/Way.cpp
	$(CC) $(CFLAGS) tests/test_cache.cpp src/Cache.cpp src/Way.cpp -o test_cache

test_two_levels: tests/test_two_levels.cpp src/Cache.cpp src/Way.cpp
	$(CC) $(CFLAGS) tests/test_two_levels.cpp src/Cache.cpp src/Way.cpp -o test_two_levels

test_mh: tests/test_memory_hierarchy.cpp $(SRC)
	$(CC) $(CFLAGS) tests/test_memory_hierarchy.cpp $(SRC) -o test_mh

clean:
	rm -f sim test_way test_cache test_two_levels test_mh