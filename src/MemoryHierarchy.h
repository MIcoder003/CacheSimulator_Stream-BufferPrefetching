#ifndef MEMORYHIERARCHY_H
#define MEMORYHIERARCHY_H

#include <vector>
#include "Cache.h"
#include "AccessStats.h"


class MemoryHierarchy{
    public:
        int numLevels;
        MemoryHierarchy(const std::vector<CacheConfig>& configs);
        ~MemoryHierarchy();

        bool access(unsigned long address, char op);
        const AccessStats& getLevelStats(int level) const;
        void printAllStats() const;

    private:
    std::vector<Cache*> levels;

};

#endif