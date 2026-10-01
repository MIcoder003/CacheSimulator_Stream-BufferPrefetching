#ifndef WAY_H
#define WAY_H

class Way {
    public:
    bool valid;
    bool dirty;
    unsigned long tag;
    int lruCounter;
    
    Way();
    
    bool match(unsigned long t) const;
    
    void fill(unsigned long t);

    void markDirty();

    void invalidate();

};
#endif