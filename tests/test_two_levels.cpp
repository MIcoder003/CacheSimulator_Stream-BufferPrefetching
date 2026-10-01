#include <iostream>
#include "Cache.h"

static void check(const char* label, long long got, long long expected) {
    std::cout << (got == expected ? "[ok]   " : "[FAIL] ")
              << label
              << "  (got " << got << ", expected " << expected << ")\n";
}



int main(){

    CacheConfig l1cfg;
    l1cfg.blockSize = 4;
    l1cfg.associativity = 4;
    l1cfg.numSets = 4;

    CacheConfig l2cfg;
    l2cfg.blockSize = 4;
    l2cfg.associativity = 4;
    l2cfg.numSets = 16;

    Cache l2(l2cfg, nullptr);
    Cache l1(l1cfg, &l2);

    // L1 read miss forward to L2
    {
        Cache l2(l2cfg, nullptr);
        Cache l1(l1cfg, &l2);
        std::cout << "\n-- L1 miss forwards to L2 --\n";
 
        l1.access(0x00, 'r');
 
        check("L2 reads", l2.getStats().reads, 1);
        check("L2 writes", l2.getStats().writes, 0);
    }

    // --- 2. an L1 hit forwards nothing ---

    {
        Cache l2(l2cfg, nullptr);
        Cache l1(l1cfg, &l2);
        std::cout << "\n-- L1 hit forwards nothing --\n";
 
        l1.access(0x00, 'r');
        l1.access(0x00, 'r');
 
        check("L1 reads", l1.getStats().reads, 2);
        check("L1 read misses", l1.getStats().readMisses, 1);
        check("L2 reads", l2.getStats().reads, 1);
    }

    // --- 3. evicting a clean block writes nothing back ---

    {
        Cache l2(l2cfg, nullptr);
        Cache l1(l1cfg, &l2);
        std::cout << "\n-- clean eviction --\n";
 
        l1.access(0x00, 'r');
        l1.access(0x10, 'r');
        l1.access(0x20, 'r');
        l1.access(0x30, 'r');
        l1.access(0x40, 'r');   // evicts 0x00, which is clean
 
        check("L1 write backs", l1.getStats().writeBacks, 0);
        check("L2 reads", l2.getStats().reads, 5);
        check("L2 writes", l2.getStats().writes, 0);
    }

    // --- 4. evicting a dirty block writes it back ---
    {
        Cache l2(l2cfg, nullptr);
        Cache l1(l1cfg, &l2);
        std::cout << "\n-- dirty eviction --\n";
 
        l1.access(0x00, 'w');   // installs 0x00 and marks it dirty
        l1.access(0x10, 'r');
        l1.access(0x20, 'r');
        l1.access(0x30, 'r');
        l1.access(0x40, 'r');   // evicts 0x00, which is dirty
 
        check("L1 write backs", l1.getStats().writeBacks, 1);
        check("L2 writes", l2.getStats().writes, 1);
        check("L2 reads", l2.getStats().reads, 5);
    }

    // --- 5. the written-back block lands at the right address in L2 ---
    {
        Cache l2(l2cfg, nullptr);
        Cache l1(l1cfg, &l2);
        std::cout << "\n-- write back goes to the right block --\n";
 
        l1.access(0x00, 'w');
        l1.access(0x10, 'r');
        l1.access(0x20, 'r');
        l1.access(0x30, 'r');
        l1.access(0x40, 'r');   // 0x00 written back to L2 here
 
        // L2 already holds 0x00 from the original fetch, so the write back
        // should have hit, not missed.
        check("L2 write misses", l2.getStats().writeMisses, 0);
    }

    return 0;
}
