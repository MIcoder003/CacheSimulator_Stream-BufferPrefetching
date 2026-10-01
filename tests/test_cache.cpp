// tests/test_cache.cpp
#include <iostream>
#include "Cache.h"

void check(const char* label, bool got, bool expected) {
    std::cout << (got == expected ? "[ok]   " : "[FAIL] ")
              << label
              << "  (got " << got << ", expected " << expected << ")\n";
}

int main() {
    CacheConfig cfg;
    cfg.blockSize = 4;
    cfg.associativity = 4;
    cfg.numSets = 4;
    // 2 offset bits, 2 index bits -> addresses 16 apart share a set

    // --- 1. cold miss, then hit on the same address ---
    {
        Cache c(cfg, nullptr);
        std::cout << "\n-- cold miss then hit --\n";
        check("first access to 0x00", c.access(0x00, 'r'), false);
        check("second access to 0x00", c.access(0x00, 'r'), true);
    }

    // --- 2. two blocks in the same set coexist ---
    {
        Cache c(cfg, nullptr);
        std::cout << "\n-- two blocks, same set --\n";
        check("0x00 cold", c.access(0x00, 'r'), false);
        check("0x10 cold", c.access(0x10, 'r'), false);
        check("0x00 still cached", c.access(0x00, 'r'), true);
        check("0x10 still cached", c.access(0x10, 'r'), true);
    }

    // --- 3. fill the set, then force an LRU eviction ---
    {
        Cache c(cfg, nullptr);
        std::cout << "\n-- LRU eviction --\n";
        c.access(0x00, 'r');   // way 0
        c.access(0x10, 'r');   // way 1
        c.access(0x20, 'r');   // way 2
        c.access(0x30, 'r');   // way 3 -- set now full, 0x00 is LRU
        check("0x40 evicts something", c.access(0x40, 'r'), false);
        check("0x00 was the victim", c.access(0x00, 'r'), false);
        check("0x30 survived", c.access(0x30, 'r'), true);
    }

    // --- 4. a write installs the block too ---
    {
        Cache c(cfg, nullptr);
        std::cout << "\n-- write then read --\n";
        check("write 0x00 cold", c.access(0x00, 'w'), false);
        check("read 0x00 hits", c.access(0x00, 'r'), true);
    }

    // --- 5. different sets don't interfere ---
    {
        Cache c(cfg, nullptr);
        std::cout << "\n-- separate sets --\n";
        check("0x00 cold", c.access(0x00, 'r'), false);
        check("0x04 cold (set 1)", c.access(0x04, 'r'), false);
        check("0x00 still cached", c.access(0x00, 'r'), true);
    }

    return 0;
}
