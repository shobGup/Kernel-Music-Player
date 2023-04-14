#include "ide.h"
#include "ext2.h"
#include "libk.h"
#include "threads.h"
#include "semaphore.h"
#include "future.h"
#include "pit.h"

 //Play sound using built in speaker
//  static void play_sound(uint32_t nFrequence) {
//  	uint32_t Div;
//  	uint8_t tmp;
 
//         //Set the PIT to the desired frequency
//  	Div = 1193180 / nFrequence;
//  	outb(0x43, 0xb6);
//  	outb(0x42, (uint8_t) (Div) );
//  	outb(0x42, (uint8_t) (Div >> 8));
 
//         //And play the sound using the PC speaker
//  	tmp = inb(0x61);
//   	if (tmp != (tmp | 3)) {
//  		outb(0x61, tmp | 3);
//  	}
//  }

 static void play_sound(uint32_t nFrequence) {
    // ...

    //And play the sound using the audio device

    uint32_t Div;
 	// uint8_t tmp;
 
 	Div = 1193180 / nFrequence;

    outb(0x43, 0xb6);
    outb(0x42, (uint8_t) (Div) );
    outb(0x42, (uint8_t) (Div >> 8));
    outb(0x61, inb(0x61) | 0x3);
    Debug::printf("PLAY: Port value: %x\n", inb(0x61));
    outb(0x3D4, 0x0D);
    outb(0x3D5, (inb(0x3D5) & 0xFC) | 0x02);
    //This is the output to the "pcm" device
    outb(0x43, 0x0B);
    outb(0x42, (uint8_t)(nFrequence & 0xff));
    outb(0x42, (uint8_t)(nFrequence >> 8));
}

 
 //make it shutup
 static void nosound() {
 	uint8_t tmp = inb(0x61) & 0xFC;
 
 	outb(0x61, tmp);
    Debug::printf("STOP: Port value: %x\n", inb(0x61));
 }

 void timer_wait(uint32_t ticks) {
    volatile uint32_t target = 0;
    while (target < ticks) {
        target++;
        // loop until desired number of ticks has elapsed
    }
}

 
 //Make a beep
 void beep() {
 	 play_sound(1000);
 	 timer_wait(1000);
 	 nosound();
          //set_PIT_2(old_frequency);
 }

void kernelMain(void) {
    for (;;) {
        // outb(0x43, 0xb6); // set mode 3, square wave generator
        // outb(0x42, 0x4d); // set low byte of frequency to 262 (middle C)
        // outb(0x42, 0x09); // set high byte of frequency to 262 (middle C)
        // outb(0x61, inb(0x61) | 0x3); // turn speaker on
        // for (volatile int i = 0; i < 1000000; ++i); // delay loop
        // outb(0x61, inb(0x61) & ~0x3); // turn speaker off
        // for (volatile int i = 0; i < 1000000; ++i); // delay loop
        // Debug::printf("Hello\n");
        beep();
        Debug::printf("BEEP\n");
    }
}
