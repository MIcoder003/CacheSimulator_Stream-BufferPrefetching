#include <iostream>
#include <iomanip>
#include "Cache.h"


//Constructor
Cache::Cache(const CacheConfig& config, Cache* nextLevel)
    : blockSize(config.blockSize), associativity(config.associativity), numSets(config.numSets),
    nextLevel(nextLevel), prefetch(config.prefN, config.prefM)
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

void Cache::issuePrefetches(const std::vector<unsigned long>& prefetches) {
    stats.prefetches += prefetches.size();
    for (size_t i = 0; i < prefetches.size(); i++) {
        if (nextLevel != nullptr) {
            nextLevel->access(prefetches[i] << offsetBits, 'r', true);
        }
    }
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

bool Cache::hasPrefetchUnit() const {
    return prefetch.enabled();
}

void Cache::printPrefetchContents() const {
    prefetch.printContents();
}

void Cache::printContents(const char* name) const {
    std::cout << "===== " << name << " contents =====\n";

    for (int i = 0; i < numSets; i++) {
        std::cout << "set" << std::setw(7) << i << ":";

        // visit ways in recency order
        for (int r = 0; r < associativity; r++) {
            for (int j = 0; j < associativity; j++) {
                if (sets[i][j].lruCounter != r) continue;
                if (!sets[i][j].valid) continue;

                std::cout << "   " << std::hex << sets[i][j].tag << std::dec
                          << (sets[i][j].dirty ? " D" : "  ");
            }
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

bool Cache::access(unsigned long address, char op, bool isPrefetch) {

    AddressParts parts = addressResolution(address);
    int wayIndex = findMatch(parts.index, parts.tag);
    unsigned long blockAddr = address >> offsetBits;
    std::vector<unsigned long> prefetches; 
    bool hit = (wayIndex != -1);

    if (hit) {
        if (op == 'w') {
            stats.writes++;
            sets[parts.index][wayIndex].markDirty();
        } else {
            stats.reads++;
        }
        updateLru(parts.index, wayIndex);

        prefetch.onCacheHit(blockAddr, prefetches);
        issuePrefetches(prefetches);

        return true;
    } else {
        bool bufferHit = prefetch.onCacheMiss(blockAddr, prefetches);
        if (op == 'w') {
            stats.writes++;
            if (!bufferHit) stats.writeMisses++;
        } else {
            stats.reads++;
            if (!bufferHit) stats.readMisses++;
        }

        wayIndex = pickVictim(parts.index);
        Way& victimWay = sets[parts.index][wayIndex];

        unsigned long victimAddress = (victimWay.tag << (offsetBits + indexBits))
                            | (parts.index << offsetBits);

        if (victimWay.valid && victimWay.dirty) {
            stats.writeBacks++;
            if (nextLevel != nullptr) {
                nextLevel->access(victimAddress, 'w');
            }
        }

        if (!bufferHit && nextLevel != nullptr) {
            nextLevel->access(address, 'r');
        }

        victimWay.fill(parts.tag);
        victimWay.lruCounter = associativity - 1;

        if (op == 'w') {
            victimWay.markDirty();
        }
        updateLru(parts.index, wayIndex);

        issuePrefetches(prefetches);
        return false;
    }
}