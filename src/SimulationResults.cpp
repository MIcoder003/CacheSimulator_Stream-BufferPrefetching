#include "SimulationResults.h"

#include <iomanip>
#include <iostream>

SimulationResults::SimulationResults(const std::vector<AccessStats>& perLevel)
    : perLevel(perLevel) {}

bool SimulationResults::hasL2() const {
    return perLevel.size() > 1;
}

// e. (L1 read misses + L1 write misses) / (L1 reads + L1 writes)
double SimulationResults::l1MissRate() const {
    const AccessStats& l1 = perLevel[0];
    long long total = l1.reads + l1.writes;
    if (total == 0) return 0.0;
    return static_cast<double>(l1.readMisses + l1.writeMisses) / total;
}

// n. L2 read misses / L2 reads -- reads only, unlike L1
double SimulationResults::l2MissRate() const {
    if (!hasL2()) return 0.0;
    const AccessStats& l2 = perLevel[1];
    if (l2.reads == 0) return 0.0;
    return static_cast<double>(l2.readMisses) / l2.reads;
}

// q. blocks transferred to and from memory
long long SimulationResults::totalMemoryTraffic() const {
    const AccessStats& last = hasL2() ? perLevel[1] : perLevel[0];
    return last.readMisses + last.writeMisses + last.writeBacks + last.prefetches;
}


void SimulationResults::print() const {
    const AccessStats& l1 = perLevel[0];

    std::cout << "a. L1 reads:                   " << l1.reads << "\n";
    std::cout << "b. L1 read misses:             " << l1.readMisses << "\n";
    std::cout << "c. L1 writes:                  " << l1.writes << "\n";
    std::cout << "d. L1 write misses:            " << l1.writeMisses << "\n";

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "e. L1 miss rate:               " << l1MissRate() << "\n";
    std::cout.unsetf(std::ios::fixed);

    std::cout << "f. L1 writebacks:              " << l1.writeBacks << "\n";
    std::cout << "g. L1 prefetches:              " << l1.prefetches << "\n";

    if (hasL2()) {
        const AccessStats& l2 = perLevel[1];

        // With no prefetch unit yet, every L2 read is demand traffic from
        // L1, so items j and k are zero and h and i cover all L2 reads.
        std::cout << "h. L2 reads (demand):          " << l2.reads << "\n";
        std::cout << "i. L2 read misses (demand):    " << l2.readMisses << "\n";
        std::cout << "j. L2 reads (prefetch):        " << 0 << "\n";
        std::cout << "k. L2 read misses (prefetch):  " << 0 << "\n";
        std::cout << "l. L2 writes:                  " << l2.writes << "\n";
        std::cout << "m. L2 write misses:            " << l2.writeMisses << "\n";

        std::cout << std::fixed << std::setprecision(4);
        std::cout << "n. L2 miss rate:               " << l2MissRate() << "\n";
        std::cout.unsetf(std::ios::fixed);

        std::cout << "o. L2 writebacks:              " << l2.writeBacks << "\n";
        std::cout << "p. L2 prefetches:              " << l2.prefetches << "\n";
    } else {
        std::cout << "h. L2 reads (demands):         0\n";
        std::cout << "i. L2 read misses (demand):    0\n";
        std::cout << "j. L2 reads (prefetch):        0\n";
        std::cout << "k. L2 read misses (prefetch):  0\n";
        std::cout << "l. L2 writes:                  0\n";
        std::cout << "m. L2 write misses:            0\n";
        std::cout << "n. L2 miss rate:               0\n";
        std::cout << "o. L2 writebacks:              0\n";
        std::cout << "p. L2 prefetches:              0\n";
    }

    std::cout << "q. memory traffic:             " << totalMemoryTraffic() << "\n";
}