#include <iostream>
#include "MemoryHierarchy.h"


// Constructor 
MemoryHierarchy::MemoryHierarchy(const std::vector<CacheConfig>& configs){
    int numLevels = configs.size();
    levels.resize(numLevels);

    for (int i = numLevels-1; i>=0; i--){
        Cache* next = (i == numLevels - 1) ? nullptr : levels[i+1];
        levels[i] = new Cache(configs[i], next);
    }

}

// Destructor
MemoryHierarchy::~MemoryHierarchy(){
    for (Cache* level : levels){
        delete level;
    }
}

const AccessStats& MemoryHierarchy::getLevelStats(int level) const {
    return levels[level]->getStats();
}

bool MemoryHierarchy::access(unsigned long address, char op){
    if(levels.empty()) {
        return false;
    }
    return levels[0]->access(address, op);
}

void MemoryHierarchy::printAllStats() const {
    for (int i = 0; i < static_cast<int>(levels.size()); i++) {
        const AccessStats& s = levels[i]->getStats();
        std::cout << "L" << (i + 1)
                  << ": reads=" << s.reads
                  << " readMisses=" << s.readMisses
                  << " writes=" << s.writes
                  << " writeMisses=" << s.writeMisses
                  << " writeBacks=" << s.writeBacks << "\n";
    }
}