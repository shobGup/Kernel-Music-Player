#include "kb.h"

kb::kb(VGA* vga): vga(vga) {}

void kb::kbInit(Shared<Node> logo, Shared<Semaphore> spot) {
    char* program = setupScreen();

    if (K::streq(program, (const char*)"spotify")) {
        delete[] program;
        vga->bootup(logo);
        spot->up();

        vga->initializeScreen(vga->bg_color);

        vga->drawRectangle(70, 9, 250, 19, 63, 1); // text box
        vga->drawString(70, 10, (const char*)"Press tab to search...", vga->bg_color); // enter spotify
        char* name = new char[40];
        int len = 0;
        bool start = 0;
        int size = 40;
        // start polling/interrupts
        while (1) {
            while ((inb(STATUS_REG) & 0x1) == 0) {}
            int val = inb(DATA_PORT);
            char c = ascii[val];
            if (val == 0xF) { // tab, start reading for input to string
                vga->drawRectangle(70, 9, 250, 19, 63, 1); // text box
                len = 0;
                if (start) name = new char[40];
                size = 40;
                start = 1;
            }
            if (c == '\n') { // enter
                if (name) {
                    name[len] = 0;
                    start = 0;
                    memcpy(filename, name, (len));
                    delete[] name;
                    filename[len] = '\0';
                    entered = true;
                    vga->drawRectangle(70, 9, 250, 19, 63, 1); // text box
                }
            }
            if (val == 0xE) { // backspace
                if (len > 0) {
                    len--;
                    name[len] = 0;
                    vga->drawRectangle(70, 9, 250, 19, 63, 1); // text box
                    if (len > 22) {
                        char* tempname = new char[22];
                        for (int i = 0; i < 22; i++) {
                            tempname[i] = name[len - 22 + i];
                        }
                        vga->drawRectangle(70, 9, 250, 19, 63, 1); // text box
                        vga->drawString(70, 10, tempname, vga->bg_color);
                        delete[] tempname;
                    } else {
                        vga->drawString(70, 10, name, vga->bg_color);
                    }
                }
            }
            // if numbers 0-9 || 16 - 25 || 30 - 38 || 44 - 50
            if (((val >= 2 && val <= 13) || (val >= 16 && val <= 25) || (val >= 30 && val <=38) || (val >= 44 && val <= 50) || val==0x39) && start) { // add char to string
                name[len++] = c;
                name[len] = 0;
                if (len > size) {
                    char* temp = new char[len * 2 + 10];
                    size = len * 2 + 10;
                    memcpy(temp, name, len);
                    delete[] name;
                    name = new char[len * 2 + 10];
                    memcpy(name, temp, len);
                    delete[] temp;
                }
                vga->drawRectangle(70, 9, 250, 19, 63, 1); // text box
                if (len > 22) {
                    char* tempname = new char[22];
                    for (int i = 0; i < 22; i++) {
                        tempname[i] = name[len - 22 + i];
                    }
                    vga->drawRectangle(70, 9, 250, 19, 63, 1); // text box
                    vga->drawString(70, 10, tempname, vga->bg_color);
                    delete[] tempname;
                } else {
                    vga->drawString(70, 10, name, vga->bg_color);
                }

                // vga->drawRectangle(151, 96, 232, 104, 63, 1); // text box
                // if (len > 22) {
                //     // vga->drawRectangle(151, 96, 232, 104, 63, 1); // text box
                //     vga->drawString(88, 96, (const char*)((*name) + (len-22)), vga->bg_color);
                // } else {
                //     // vga->drawRectangle(151, 96, 232, 104, 63, 1); // text box
                //     vga->drawString(88, 96, name, vga->bg_color);
                // }
            }
            if(val == 208) reset = true;
            if(val == 203) precend = true;
            if(val == 205) skip = true;
            if (val == 57 && !start) tapped = 1;
        }
    }
}

void kb::setupKB() {
    tapped = false;

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
}

char* kb::setupScreen() {
    vga->drawString(134, 71, (const char*)"PentOS", 63); // show PentOS
    vga->drawRectangle(87, 95, 232, 105, 63, 1); // text box
    vga->drawString(88, 96, (const char*)"Type program name:", vga->bg_color); // enter spotify

    char* program = new char[8];
    int len = 0;
    bool start = 0;
    int size = 8;
    program[7] = 0;
    while (1) {
        while ((inb(STATUS_REG) & 0x1) == 0) {} // poll for first key press.
        int val = inb(DATA_PORT);
        char c = ascii[val];
        if (val == 0xF) { // tab, start reading for input to string
            vga->drawRectangle(87, 95, 232, 104, 63, 1); // text box
            program = new char[8];
            start = 1;
        }
        if (c == '\n') { // enter
            if (program) {
                program[len] = 0;
                break;
            }
        }
        if (val == 0xE) { // backspace
            if (len > 0) {
                len--;
                program[len] = 0;
                vga->drawRectangle(87, 95, 232, 104, 63, 1); // text box
                if (len > 22) {
                    char* tempname = new char[22];
                    for (int i = 0; i < 22; i++) {
                        tempname[i] = program[len - 22 + i];
                    }
                    vga->drawRectangle(87, 95, 232, 104, 63, 1); // text box
                    vga->drawString(88, 96, program, vga->bg_color);
                    delete[] tempname;
                } else {
                    vga->drawString(88, 96, program, vga->bg_color);
                }
            }
        }
        // if numbers 0-9 || 16 - 25 || 30 - 38 || 44 - 50
        if (((val >= 2 && val <= 13) || (val >= 16 && val <= 25) || (val >= 30 && val <=38) || (val >= 44 && val <= 50) || val==0x39) && start) { // add char to string
            program[len++] = c;
            program[len] = 0;
            if (len > size) {
                char* temp = new char[len * 2 + 10];
                size = len * 2 + 10;
                memcpy(temp, program, len);
                delete[] program;
                program = new char[len * 2 + 10];
                memcpy(program, temp, len);
                delete[] temp;
            }
            vga->drawRectangle(151, 96, 232, 104, 63, 1); // text box
            if (len > 22) {
                char* tempname = new char[22];
                for (int i = 0; i < 22; i++) {
                    tempname[i] = program[len - 22 + i];
                }
                vga->drawRectangle(151, 96, 232, 104, 63, 1); // text box
                vga->drawString(88, 96, program, vga->bg_color);
                delete[] tempname;
            } else {
                vga->drawString(88, 96, program, vga->bg_color);
            }
        }
    }
    return program;
}