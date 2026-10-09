#ifndef CACHE_H
#define CACHE_H

#include <vector>
#include <string>
#include "Way.h"
#include "AccessStats.h"
#include "PrefetchUnit.h"

struct CacheConfig{
    int blockSize;
    int associativity;
    int numSets;
    int prefN = 0;
    int prefM = 0;
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
        bool access(unsigned long address, char op, bool isPrefetch = false);
        void recordWriteback();
        void recordPrefetch(); 

        void printContents(const char*) const;
        bool hasPrefetchUnit() const;
        void printPrefetchContents() const;

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

        PrefetchUnit prefetch;

        AccessStats stats;

        void recordRead(bool hit);
        void recordWrite(bool hit);

        int findMatch(unsigned int setIndex, unsigned long tag) const; 
        int pickVictim(unsigned int setIndex) const;
        void updateLru(unsigned int setIndex, unsigned int accessedWay);
        void issuePrefetches(const std::vector<unsigned long>& prefetches);
    
};

#endif