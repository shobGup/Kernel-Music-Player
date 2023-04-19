#include "kb.h"

kb::kb(VGA* vga): vga(vga) {}

void kb::kbInit() {
    // legacy is already disabled?
    initializeController();
}

void kb::initializeController() {
    tapped = false;
    // disable interrupts:
    // cli();

    // disable devices
    while (inb(STATUS_REG) & 0x2);
    outb(CMD_REG, 0xAD);
    while (inb(STATUS_REG) & 0x2);
    outb(CMD_REG, 0xA7);

    // flushing the output buffer
    inb(DATA_PORT);

    // set configuration byte to enable keyboard port.
    while (inb(STATUS_REG) & 0x2);
    outb(CMD_REG, 0x20); // tell device I want to read Controller config byte.
    uint8_t configByte = inb(DATA_PORT) | 0x37; // and it with what I need to be set

    while ((inb(STATUS_REG) & 0x2));
    // now we can write the config byte
    outb(CMD_REG, 0x60); // tell device I want to write config byte
    while (inb(STATUS_REG) & 0x2);
    outb(CMD_REG, configByte); // write config byte

    // enable device
    while (inb(STATUS_REG) & 0x2);
    outb(CMD_REG, 0xAE); // enable first port?
    char* name = new char[25];
    int len = 0;
    bool start = 0;
    // start polling/interrupts
    while (1) {
        while ((inb(STATUS_REG) & 0x1) == 0) {}
        int val = inb(DATA_PORT);
        char c = ascii[val];
        if (val == 57 && !start) {
            tapped = 1;
            vga->play_pause();
        }
        if (val == 0x4D) { // right arrow: 4d
            // go to next song.
        }
        if (val == 0x4B) { // left arrow: 4b
            // go to previous song?
        }
        if (val == 0xF) { // tab, start reading for input to string
            vga->drawRectangle(0, 0, 320, 20, vga->bg_color, true);
            len = 0;
            name = new char[25];
            start = 1;
        }
        if (c == '\n') { // enter
            name[len] = 0;
            start = 0;
            char * temp= new char[5];
            temp[0] = '.';
            temp[1] = 'w';
            temp[2] = 'a';
            temp[3] = 'v';
            temp[4] = '\0';
            memcpy(filename, name, (len));
            memcpy((filename + (len)), temp, 5);
            entered = true;
            vga->drawRectangle(0, 0, 320, 20, vga->bg_color, true);
            vga->new_song = true;
            vga->spotify(name);
            // call wav finder
            delete[] name;
        }
        if (val == 0xE) { // backspace
            if (len > 0) {
                len--;
                name[len] = 0;
                int x = (320/2) - (len/2)*8;
                vga->drawRectangle(0, 0, 320, 20, vga->bg_color, true);
                vga->drawString(x, 10, name, 0);
            }
        }
        if(val == 208) { // Reset Song
            reset = true; 
        }
        if(val == 203) {
            precend = true; 
        }
        if(val == 205) {
            skip = true; 
        }
        // if numbers 0-9 || 16 - 25 || 30 - 38 || 44 - 50
        if (((val >= 2 && val <= 13) || (val >= 16 && val <= 25) || (val >= 30 && val <=38) || (val >= 44 && val <= 50) || val==0x39) && start) { // add char to string
            name[len++] = c;
            name[len] = 0;
            int x = (320/2) - (len/2)*8;
            vga->drawRectangle(0, 0, 320, 20, vga->bg_color, true);
            vga->drawString(x, 10, name, 0);
        }
    }
}