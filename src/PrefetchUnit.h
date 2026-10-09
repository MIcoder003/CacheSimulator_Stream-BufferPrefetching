#ifndef PREFETCHUNIT_H
#define PREFETCHUNIT_H

#include <vector>

struct StreamBuffer {
    bool valid;
    int lruCounter;
    std::vector<unsigned long> blocks;  // size is M
};

struct BufferHit {
    int bufferIndex;   // -1 when nothing hit
    int entryIndex;
};

class PrefetchUnit {

    public:
     PrefetchUnit(int N, int M);

     // Both return true if the block hit in a stream buffer.
     // Any blocks that need prefetching are appended to prefetches;
     // the caller issues those reads to the next level.
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