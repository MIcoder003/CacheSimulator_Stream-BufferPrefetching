#ifndef PREFETCHUNIT_H
#define PREFETCHUNIT_H

#include <vector>

struct StreamBuffer {
    bool valid;
    int lruCounter;
    std::vector<unsigned long> blocks;  
};

struct BufferHit {
    int bufferIndex;   
    int entryIndex;
};

class PrefetchUnit {

    public:
     PrefetchUnit(int N, int M);

     bool onCacheHit(unsigned long blockAddr, std::vector<unsigned long>& prefetches);
     bool onCacheMiss(unsigned long blockAddr, std::vector<unsigned long>& prefetches);

     void printContents() const;
     bool enabled() const;

    private:
        BufferHit searchBuffers(unsigned long blockAddr) const;
        int pickVictim() const;
        void updateLru(int bufferIndex);
        void newStream(int bufferIndex, unsigned long blockAddr,
                       std::vector<unsigned long>& prefetches);
        void advanceStream(int bufferIndex, int entryIndex,
                           std::vector<unsigned long>& prefetches);

        int numBuffers;
        int blocksPerBuffer;
        std::vector<StreamBuffer> buffers;
};

#endif