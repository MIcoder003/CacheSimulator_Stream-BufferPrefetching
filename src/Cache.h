#ifndef CACHE_H
#define CACHE_H

#include <vector>
#include <string>
#include "Way.h"
#include "AccessStats.h"

struct CacheConfig{
    int blockSize;
    int associativity;
    int numSets;
    std::string name;
};

struct AddressParts{
    unsigned long tag;
    unsigned long index;
    unsigned int offset; 
};

class Cache {
    public:
        Cache(const CacheConfig&, Cache*);
        bool access(unsigned long address, char op);
        void recordWriteback();
        void recordPrefetch(); 

        const AccessStats& getStats() const;
    
    private:
        int size; 
        int blockSize;
        int associativity; 
        int numSets;

        Cache* nextLevel;

        unsigned int offsetBits;
        unsigned int indexBits; 

        std::vector<std::vector<Way>> sets; 

        AddressParts addressResolution(unsigned long address) const;
        AccessStats stats;

        void recordRead(bool hit);
        void recordWrite(bool hit);

        int findMatch(unsigned int setIndex, unsigned long tag) const; 
        int pickVictim(unsigned int setIndex) const;
        void updateLru(unsigned int setIndex, unsigned int accessedWay);
    
};

#endif