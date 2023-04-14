#include "ide.h"
#include "ext2.h"
#include "libk.h"
#include "threads.h"
#include "semaphore.h"
#include "future.h"
#include "pit.h"


void kernelMain(void) {
    for (;;) {
        outb(0x61, 0x3); // turn speaker on
        for (volatile int i = 0; i < 100000; ++i); // delay loop
        outb(0x61, 0x0); // turn speaker off
        for (volatile int i = 0; i < 100000; ++i); // delay loop
        Debug::printf("Hello\n");
    }

}
