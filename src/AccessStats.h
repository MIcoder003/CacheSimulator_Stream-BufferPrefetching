#ifndef ACCESSSTATS_H
#define ACCESSSTATS_H

struct AccessStats {
    int reads = 0;
    int readMisses = 0;
    int writes = 0;
    int writeMisses = 0;
    int writeBacks = 0;
    int prefetches = 0;
};

#endif