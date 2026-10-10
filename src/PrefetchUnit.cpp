#include <iostream>
#include <iomanip>
#include "PrefetchUnit.h"

PrefetchUnit::PrefetchUnit(int N, int M)
    : numBuffers(N), blocksPerBuffer(M)
{
    buffers.resize(N);
    for (int i = 0; i < N; i++) {
        buffers[i].valid = false;
        buffers[i].lruCounter = i;          // gives a defined initial recency order
        buffers[i].blocks.resize(M, 0);
    }
}

bool PrefetchUnit::enabled() const {
    return numBuffers > 0;
}

// Searches in recency order and stops at the first hit, so a block present in
// several buffers is only ever credited to the most-recently-used one.
BufferHit PrefetchUnit::searchBuffers(unsigned long blockAddr) const {
    BufferHit result;
    result.bufferIndex = -1;
    result.entryIndex = -1;

    for (int recency = 0; recency < numBuffers; recency++) {
        for (int i = 0; i < numBuffers; i++) {
            if (buffers[i].lruCounter != recency) continue;
            if (!buffers[i].valid) continue;

            for (int k = 0; k < blocksPerBuffer; k++) {
                if (buffers[i].blocks[k] == blockAddr) {
                    result.bufferIndex = i;
                    result.entryIndex = k;
                    return result;
                }
            }
        }
    }

    return result;
}

int PrefetchUnit::pickVictim() const {
    for (int i = 0; i < numBuffers; i++) {
        if (!buffers[i].valid) {
            return i;
        }
    }

    int victim = 0;
    for (int i = 1; i < numBuffers; i++) {
        if (buffers[i].lruCounter > buffers[victim].lruCounter) {
            victim = i;
        }
    }
    return victim;
}

void PrefetchUnit::updateLru(int bufferIndex) {
    int oldCounter = buffers[bufferIndex].lruCounter;

    for (int i = 0; i < numBuffers; i++) {
        if (i == bufferIndex) {
            buffers[i].lruCounter = 0;
        } else if (buffers[i].lruCounter < oldCounter) {
            buffers[i].lruCounter++;
        }
    }
}


void PrefetchUnit::newStream(int bufferIndex, unsigned long blockAddr,
                             std::vector<unsigned long>& prefetches) {
    StreamBuffer& buf = buffers[bufferIndex];

    for (int k = 0; k < blocksPerBuffer; k++) {
        buf.blocks[k] = blockAddr + 1 + k;
        prefetches.push_back(buf.blocks[k]);
    }
    buf.valid = true;
}

void PrefetchUnit::advanceStream(int bufferIndex, int entryIndex,
                                 std::vector<unsigned long>& prefetches) {
    StreamBuffer& buf = buffers[bufferIndex];

    int removed = entryIndex + 1;
    unsigned long last = buf.blocks[blocksPerBuffer - 1];

    for (int k = 0; k + removed < blocksPerBuffer; k++) {
        buf.blocks[k] = buf.blocks[k + removed];
    }

    for (int k = blocksPerBuffer - removed; k < blocksPerBuffer; k++) {
        last++;
        buf.blocks[k] = last;
        prefetches.push_back(last);
    }
}

void PrefetchUnit::printContents() const {
    std::cout << "===== Stream Buffer(s) contents =====\n";

    for (int recency = 0; recency < numBuffers; recency++) {
        for (int i = 0; i < numBuffers; i++) {
            if (buffers[i].lruCounter != recency) continue;
            if (!buffers[i].valid) continue;

            for (int k = 0; k < blocksPerBuffer; k++) {
                std::cout << " " << std::hex << buffers[i].blocks[k] << std::dec << " ";
            }
            std::cout << "\n";
        }
    }
    std::cout << "\n";
}


bool PrefetchUnit::onCacheHit(unsigned long blockAddr,
                              std::vector<unsigned long>& prefetches) {
    if (!enabled()) return false;

    BufferHit hit = searchBuffers(blockAddr);
    if (hit.bufferIndex == -1) {
        return false;
    }

    advanceStream(hit.bufferIndex, hit.entryIndex, prefetches);
    updateLru(hit.bufferIndex);
    return true;
}

bool PrefetchUnit::onCacheMiss(unsigned long blockAddr,
                               std::vector<unsigned long>& prefetches) {
    if (!enabled()) return false;

    BufferHit hit = searchBuffers(blockAddr);

    if (hit.bufferIndex != -1) {
        advanceStream(hit.bufferIndex, hit.entryIndex, prefetches);
        updateLru(hit.bufferIndex);
        return true;
    }

    int victim = pickVictim();
    newStream(victim, blockAddr, prefetches);
    updateLru(victim);
    return false;
}