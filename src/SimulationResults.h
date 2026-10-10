#ifndef SIMULATIONRESULTS_H
#define SIMULATIONRESULTS_H
 
#include <vector>
#include "AccessStats.h"
 
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
 