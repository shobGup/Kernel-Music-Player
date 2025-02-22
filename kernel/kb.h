#ifndef KB_H
#define KB_H

#include "atomic.h"
#include "queue.h"
#include "machine.h"
#include "debug.h"
#include "vga.h"
#include "ext2.h"
#include "semaphore.h"

// 0x60	Read/Write	Data Port
// 0x64	Read	Status Register
// 0x64	Write	Command Register

#define DATA_PORT 0x60
#define STATUS_REG 0x64
#define CMD_REG 0x64

class kb {
    public:

    const char ascii[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
    };

    VGA* vga;
    Atomic<uint32_t> ref_count{0};
    bool tapped = false;
    bool reset = false; 
    bool precend = false; 
    bool skip = false;
    bool entered = false;
    bool shutdown = false; 
    char* filename = new char[25];

    kb(VGA* vga);

    void kbInit(Shared<Node> logo, Shared<Semaphore> spot);
};

#endif