#include "Cache.h"

//Constructor
Cache::Cache(const CacheConfig& config, Cache* nextLevel)
    : blockSize(config.blockSize), associativity(config.associativity), numSets(config.numSets),
    nextLevel(nextLevel)
    {
        sets.resize(numSets, std::vector<Way>(associativity));
        offsetBits = 0;
        while((1<< offsetBits) < blockSize) offsetBits++;
        
        indexBits = 0;
        while((1 << indexBits) < numSets) indexBits++;
    }

AddressParts Cache::addressResolution(unsigned long address) const {
    AddressParts parts;

    parts.offset = address & ((1UL << offsetBits) - 1);
    parts.index = (address >> offsetBits) & ((1UL << indexBits) - 1);
    parts.tag = address >> (offsetBits + indexBits);

    return parts; 
}

int Cache::findMatch(unsigned int setIndex, unsigned long tag) const {
    const std::vector<Way>& set = sets[setIndex];
    for (int i = 0; i < associativity; i++){
        if (set[i].match(tag)){
            return i; 
        }
    }
    return -1;
}

int Cache::pickVictim(unsigned int setIndex) const {
    const std::vector<Way>& set = sets[setIndex];

    //picking empty way if set has space
    for (int i = 0; i < associativity; i++){
        if (!set[i].valid) {
            return i;
        }
    }

    // Evicting the LRU
    int victim = 0;
    for (int i = 1; i<associativity; i++) {
        if (set[i].lruCounter > set[victim].lruCounter){
            victim = i;
        }
    }

    return victim;
}

void Cache::updateLru(unsigned int setIndex, unsigned int accessedWay) {
    std::vector<Way>& set = sets[setIndex];
    int oldCounter = set[accessedWay].lruCounter;

    for(int i = 0; i<associativity; i++) {
        if (i == static_cast<int>(accessedWay)) {
            set[i].lruCounter = 0;
        } else if (set[i].valid && set[i].lruCounter < oldCounter) {
            set[i].lruCounter++;
        }
    }
}

const AccessStats& Cache::getStats() const {
    return stats;
}

bool Cache::access(unsigned long address, char op) {

    AddressParts parts = addressResolution(address);
    int wayIndex = findMatch(parts.index, parts.tag);
    bool hit = (wayIndex != -1);

    if (hit) {
        if (op == 'w') {
            stats.writes++;
            sets[parts.index][wayIndex].markDirty();
        } else {
            stats.reads++;
        }
        updateLru(parts.index, wayIndex);

        return true;
    } else {
        if (op == 'w') {
            stats.writes++;
            stats.writeMisses++;
        } else {
            stats.reads++;
            stats.readMisses++;
        }

        wayIndex = pickVictim(parts.index);
        Way& victimWay = sets[parts.index][wayIndex];

        unsigned long victimAddress = (victimWay.tag << (offsetBits + indexBits))
                            | (parts.index << offsetBits);

        if (victimWay.valid && victimWay.dirty && nextLevel != nullptr) {
            stats.writeBacks++;
            nextLevel->access(victimAddress, 'w');
        }

        if (nextLevel != nullptr) {
            nextLevel->access(address, 'r');
        }

        victimWay.fill(parts.tag);
        victimWay.lruCounter = associativity - 1;

        if (op == 'w') {
            victimWay.markDirty();
        }
        updateLru(parts.index, wayIndex);
        return false;
    }
}