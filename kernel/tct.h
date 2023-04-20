#include "vga.h"
#include "kb.h"

class Tct {
    VGA* vga;
    
    Tct(VGA* v) : vga(v) {};
};