#include <iostream>
#include "Way.h"

int main(){
    Way w; 
    std::cout << "fresh: valid=" << w.valid
                << " dirty=" << w.dirty
                << " tag =" << w.tag << "\n";

    w.fill(0x1A);
    std::cout << " after fill: valid=" << w.valid
                << " dirty=" << w.dirty
                << " tag=" << w.tag << "\n";

    std::cout << "match(01xA)=" << w.match(0x1A) << "\n";
    std::cout << "match(0x2B)=" << w.match(0x2B) << "\n";

    w.markDirty();
    std::cout << "after markDirty: dirty=" << w.dirty << "\n";
    
    w.invalidate();
    std::cout << "after invalidate: valid=" << w.valid 
                <<" tag: " << w.tag <<"\n";

    return 0;
}

