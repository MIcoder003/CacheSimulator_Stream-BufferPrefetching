#ifndef SIMULATIONRESULTS_H
#define SIMULATIONRESULTS_H
 
#include <vector>
#include "AccessStats.h"
 
// Holds a snapshot of the per-level counters at the end of a run and
// computes the reported figures from them. The counters themselves know
// nothing about which level they belong to, so every formula that depends
// on level position lives here.
class SimulationResults {
public:
    explicit SimulationResults(const std::vector<AccessStats>& perLevel);
 
    double l1MissRate() const;
    double l2MissRate() const;
    long long totalMemoryTraffic() const;
 
    void print() const;
 
private:
    bool hasL2() const;
 
    std::vector<AccessStats> perLevel;
};
 
#endif
 