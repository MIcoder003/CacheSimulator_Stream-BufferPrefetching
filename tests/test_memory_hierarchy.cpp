// tests/test_memory_hierarchy.cpp
#include <iostream>
#include <vector>
#include "MemoryHierarchy.h"

static void check(const char* label, long long got, long long expected) {
    std::cout << (got == expected ? "[ok]   " : "[FAIL] ")
              << label
              << "  (got " << got << ", expected " << expected << ")\n";
}

static std::vector<CacheConfig> twoLevelConfig() {
    CacheConfig l1;
    l1.blockSize = 4;
    l1.associativity = 4;
    l1.numSets = 4;

    CacheConfig l2;
    l2.blockSize = 4;
    l2.associativity = 4;
    l2.numSets = 16;

    std::vector<CacheConfig> configs;
    configs.push_back(l1);
    configs.push_back(l2);
    return configs;
}

int main() {
    // These mirror test_two_levels.cpp exactly. The numbers should match,
    // since the only difference is who wired the levels together.

    // --- 1. an L1 miss forwards one read to L2 ---
    {
        MemoryHierarchy mh(twoLevelConfig());
        std::cout << "\n-- L1 miss forwards to L2 --\n";

        mh.access(0x00, 'r');

        check("L2 reads", mh.getLevelStats(1).reads, 1);
        check("L2 writes", mh.getLevelStats(1).writes, 0);
    }

    // --- 2. an L1 hit forwards nothing ---
    {
        MemoryHierarchy mh(twoLevelConfig());
        std::cout << "\n-- L1 hit forwards nothing --\n";

        mh.access(0x00, 'r');
        mh.access(0x00, 'r');

        check("L1 reads", mh.getLevelStats(0).reads, 2);
        check("L1 read misses", mh.getLevelStats(0).readMisses, 1);
        check("L2 reads", mh.getLevelStats(1).reads, 1);
    }

    // --- 3. evicting a clean block writes nothing back ---
    {
        MemoryHierarchy mh(twoLevelConfig());
        std::cout << "\n-- clean eviction --\n";

        mh.access(0x00, 'r');
        mh.access(0x10, 'r');
        mh.access(0x20, 'r');
        mh.access(0x30, 'r');
        mh.access(0x40, 'r');

        check("L1 write backs", mh.getLevelStats(0).writeBacks, 0);
        check("L2 reads", mh.getLevelStats(1).reads, 5);
        check("L2 writes", mh.getLevelStats(1).writes, 0);
    }

    // --- 4. evicting a dirty block writes it back ---
    {
        MemoryHierarchy mh(twoLevelConfig());
        std::cout << "\n-- dirty eviction --\n";

        mh.access(0x00, 'w');
        mh.access(0x10, 'r');
        mh.access(0x20, 'r');
        mh.access(0x30, 'r');
        mh.access(0x40, 'r');

        check("L1 write backs", mh.getLevelStats(0).writeBacks, 1);
        check("L2 writes", mh.getLevelStats(1).writes, 1);
        check("L2 write misses", mh.getLevelStats(1).writeMisses, 0);
    }

    // --- 5. the return value reports L1's hit or miss, not L2's ---
    {
        MemoryHierarchy mh(twoLevelConfig());
        std::cout << "\n-- return value comes from L1 --\n";

        check("cold access misses", mh.access(0x00, 'r'), 0);
        check("repeat access hits", mh.access(0x00, 'r'), 1);
    }

    // --- 6. a single level works with no level below it ---
    {
        CacheConfig only;
        only.blockSize = 4;
        only.associativity = 4;
        only.numSets = 4;

        std::vector<CacheConfig> configs;
        configs.push_back(only);

        MemoryHierarchy mh(configs);
        std::cout << "\n-- single level hierarchy --\n";

        mh.access(0x00, 'r');
        mh.access(0x00, 'r');

        check("reads", mh.getLevelStats(0).reads, 2);
        check("read misses", mh.getLevelStats(0).readMisses, 1);
    }

    return 0;
}