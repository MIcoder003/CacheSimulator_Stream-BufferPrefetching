#include "Way.h"

Way::Way() : valid(false), dirty(false), tag(0), lruCounter(0) {}

bool Way::match(unsigned long t) const {
    return valid && tag == t; 
}
 
void Way::fill (unsigned long t) {
    tag = t;
    valid = true; 
    dirty = false;

}           // install a freshly fetched block

void Way::markDirty() {
    dirty = true;
}

void Way::invalidate() {
    tag = 0;
    valid = false;
    dirty = false;
}