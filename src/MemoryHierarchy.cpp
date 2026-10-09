#include <iostream>
#include <cstdio> 
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

void MemoryHierarchy::printAllContents() const {
    for (int i = 0; i < (int) levels.size(); i++) {
        char name[16];
        snprintf(name, sizeof(name), "L%d", i + 1);
        levels[i]->printContents(name);
    }

    for (int i = 0; i < (int) levels.size(); i++) {
    if (levels[i]->hasPrefetchUnit()) {
        levels[i]->printPrefetchContents();
    }
}

}