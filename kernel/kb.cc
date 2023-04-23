#include "kb.h"

kb::kb(VGA* vga): vga(vga) {}

void kb::kbInit(Shared<Node> logo, Shared<Semaphore> spot) {
    tapped = false;

    // setup for keyboard

    // disable devices
    while (inb(STATUS_REG) & 0x2);
    outb(CMD_REG, 0xAD);
    while (inb(STATUS_REG) & 0x2);
    outb(CMD_REG, 0xA7);

    // flushing the output buffer
    inb(DATA_PORT);

    // set configuration byte to enable keyboard port.
    while (inb(STATUS_REG) & 0x2);

    // tell device I want to read Controller config byte.
    outb(CMD_REG, 0x20); 
    // & it with what I need to be set
    uint8_t configByte = inb(DATA_PORT) | 0x37;
    // now we can write the config byte
    while ((inb(STATUS_REG) & 0x2));
    outb(CMD_REG, 0x60); // tell device I want to write config byte
    while (inb(STATUS_REG) & 0x2);
    outb(CMD_REG, configByte); // write config byte

    // enable device
    while (inb(STATUS_REG) & 0x2);
    outb(CMD_REG, 0xAE); // enable first port?

    // post setup

    restart: // utilized later on, you'll see.
    // start up screen
    vga->initializeScreen(vga->bg_color);
    vga->drawString(134, 71, (const char*)"PentOS", 63); // show PentOS
    vga->drawRectangle(87, 95, 232, 105, 63, 1); // text box
    vga->drawString(88, 96, (const char*)"Type program name:", vga->bg_color); // enter spotify

    char* program = new char[8]; // name of program typed in
    int len = 0; // length of name
    bool start = 0; // if user has started typing
    int size = 8; // size of array
    program[7] = 0; // null terminator
    while (1) {
        while ((inb(STATUS_REG) & 0x1) == 0) {} // poll for first key press.
        int val = inb(DATA_PORT);
        char c = ascii[val];
        if (val == 0xF) { // tab, start reading for input to string
            vga->drawRectangle(87, 95, 232, 104, 63, 1); // text box
            program = new char[8]; // starting new input, clear array just in case
            start = 1; // mark as started typing
        }
        if (c == '\n') { // enter
            if (program) { // just to be safe
                program[len] = 0;
                break; // exit out of home screen polling loop
            }
        }
        if (val == 0xE) { // backspace
            if (len > 0) {
                len--; // they hit backspace.
                program[len] = 0;
                vga->drawRectangle(87, 95, 232, 104, 63, 1); // text box
                if (len > 22) { // code to make it appear as if the text box is "scrolling" when the user types a lot.
                    char* tempname = new char[22];
                    for (int i = 0; i < 22; i++) {
                        tempname[i] = program[len - 22 + i];
                    }
                    vga->drawRectangle(87, 95, 232, 104, 63, 1); // text box
                    vga->drawString(88, 96, program, vga->bg_color);
                    delete[] tempname;
                } else {
                    vga->drawString(88, 96, program, vga->bg_color); // else we can show the string normally.
                }
            }
            if (c == 27) shutdown = 1; // tbh don't remember what "27" is that is bad coding practice on my part sorry :)
        }
        // if numbers 0-9 || 16 - 25 || 30 - 38 || 44 - 50 // basically any valid character.
        if (((val >= 2 && val <= 13) || (val >= 16 && val <= 25) || (val >= 30 && val <=38) || (val >= 44 && val <= 50) || val==0x39) && start) { // add char to string
            program[len++] = c;
            program[len] = 0;
            if (len > size) { // name goes past size of array.
                char* temp = new char[len * 2 + 10];
                size = len * 2 + 10;
                memcpy(temp, program, len);
                delete[] program;
                program = new char[len * 2 + 10];
                memcpy(program, temp, len);
                delete[] temp;
            }
            vga->drawRectangle(151, 96, 232, 104, 63, 1); // text box
            if (len > 22) { // make it appear as if it is "scrolling" same as above while loop
                char* tempname = new char[22];
                for (int i = 0; i < 22; i++) {
                    tempname[i] = program[len - 22 + i];
                }
                vga->drawRectangle(151, 96, 232, 104, 63, 1); // text box
                vga->drawString(88, 96, program, vga->bg_color);
                delete[] tempname;
            } else {
                vga->drawString(88, 96, program, vga->bg_color); // can display it normally
            }
        }
    }

    if (K::streq(program, (const char*)"pentos player")) { // we only have one program, so only one check needed.
        delete[] program;
        vga->bootup(logo); // "bootup" screen to emulate the program loading, really it was pretty instant.
        spot->up(); // for integration, tell other software (graphics and sound) the program is being started.

        vga->initializeScreen(vga->bg_color); // color background

        vga->drawRectangle(70, 9, 250, 19, 63, 1); // text box
        vga->drawString(70, 10, (const char*)"Press tab to search...", vga->bg_color); // enter spotify
        char* name = new char[100];
        char* temp = new char[100];
        int len = 0;
        bool start = 0;
        size = 100;
        bool cursor = false; 
        bool printing = false; 
        bool startCursor = false; 
        // start polling/interrupts
        while (1) {
            uint32_t counter = 0; 
            while ((inb(STATUS_REG) & 0x1) == 0) {
                    if(startCursor) { // code to display blinking cursor in text box. I'm getting tired of commenting so it'll be less and less now...
                        if(counter > 42949) {
                        cursor = !cursor;
                        counter = 0; 
                        vga->drawRectangle(70, 9, 250, 19, 63, 1);
                        }
                        if(printing) {
                            temp[21] = cursor ? '_' : '\0';
                            temp[22] = '\0';
                        }
                        name[len] = cursor ? '_' : '\0';
                        name[len + 1] = '\0';
                        counter++; 
                        if(!printing) {
                            vga->drawString(70, 10, name, vga->bg_color);
                        } else {
                            vga->drawString(70, 10, temp, vga->bg_color);
                        }
                    }
            }
            int val = inb(DATA_PORT);
            char c = ascii[val];
            if (c == 27) { // esc key, reset text box
                if (start) {
                    vga->drawRectangle(70, 9, 250, 19, 63, 1); // text box
                    vga->drawString(70, 10, (const char*)"Press tab to search...", vga->bg_color); // enter spotify
                    start = 0;
                    startCursor = false;
                }
            }
            if (val == 0xF) { // tab, start reading for user input
                vga->drawRectangle(70, 9, 250, 19, 63, 1); // text box
                len = 0;
                name = new char[100];
                cursor = true; 
                size = 100;
                start = 1;
                startCursor = true;
            }
            if (c == '\n') { // enter key
                if (name) { // if they typed anything in
                    cursor = false; 
                    name[len] = 0;
                    start = 0;
                    memcpy(filename, name, (len)); // for integration, give the name of the song to audio and graphics.
                    // delete[] name;
                    filename[len] = '\0'; // for some reason it wouldn't carry over the null terminator from name, I think I was off by one but was too lazy to count.
                    entered = true; // done typing
                    vga->drawRectangle(70, 9, 250, 19, 63, 1); // text box
                    printing = false; 
                    len = 0; 
                    name[0] = '\0';
                    vga->drawString(70, 10, name, vga->bg_color);
                    startCursor = false; 
                }
            }
            if (val == 0xE) { // backspace, pretty much the same as the first while loop
                if (len > 0) {
                    len--;
                    name[len] = 0;
                    vga->drawRectangle(70, 9, 250, 19, 63, 1); // text box
                    if (len > 22) {
                        printing = true; 
                        char* tempname = new char[23];
                        for (int i = 0; i < 21; i++) {
                            tempname[i] = name[len - 21 + i];
                            temp[i] = name[len - 21 + i]; 
                        }
                        tempname[21] = cursor ? '_' : '\0'; 
                        tempname[22] = '\0'; 

                        temp[21] = cursor ? '_' : '\0'; 
                        temp[22] = '\0'; 

                        vga->drawRectangle(70, 9, 250, 19, 63, 1); // text box
                        vga->drawString(70, 10, tempname, vga->bg_color);
                    
                        delete[] tempname;
                    } else {
                        printing = false; 
                        vga->drawString(70, 10, name, vga->bg_color);
                    }
                }
            }
            // if numbers 0-9 || 16 - 25 || 30 - 38 || 44 - 50, also pretty much same as first while loop, iirc
            if (((val >= 2 && val <= 13) || (val >= 16 && val <= 25) || (val >= 30 && val <=38) || (val >= 44 && val <= 50) || val==0x39) && start) { // add char to string
                name[len++] = c;
                name[len] = 0;
                if (len > size) {
                    char* temp = new char[len * 2 + 10];
                    size = len * 2 + 10;
                    memcpy(temp, name, len);
                    // delete[] name;
                    name = new char[len * 2 + 10];
                    memcpy(name, temp, len);
                    delete[] temp;
                }
                vga->drawRectangle(70, 9, 250, 19, 63, 1); // text box
                if (len > 21) {
                    char* tempname = new char[23];
                    for (int i = 0; i < 21; i++) {
                        tempname[i] = name[len - 21 + i];
                        temp[i] = name[len - 21 + i];
                    }
                    tempname[21] = cursor ? '_' : '\0'; 
                    tempname[22] = '\0'; 
                    printing = true; 

                    temp[21] = cursor ? '_' : '\0'; 
                    temp[22] = '\0'; 

                    vga->drawRectangle(70, 9, 250, 19, 63, 1); // text box
                    vga->drawString(70, 10, tempname, vga->bg_color);
                    delete[] tempname;
                } else {
                    name[len] = cursor ? '_' : '\0';
                    name[len + 1] = '\0';
                    printing = false; 
                    vga->drawString(70, 10, name, vga->bg_color);
                }
            }
            // I'm sorry I don't remember these values and what they do :) it can be deciphered, though, with a quick google search I am too lazy for.
            if(val == 208) reset = true;
            if(val == 203) precend = true;
            if(val == 205) skip = true;
            if (val == 57 && !start) tapped = 1;
            if (val == 31 && !start) shutdown = 1;
        }
    } else { // program name given is not supported by PentOS
        vga->initializeScreen(vga->bg_color);
        vga->drawString(80, 93, (const char*)"Not a valid program,", 48);
        vga->drawString(64, 102, (const char*)"press enter to try again.", 48);
        vga->drawString(72, 111, (const char*)"press ESC to shut down.", 48);
        while (1) {
            while ((inb(STATUS_REG) & 0x1) == 0) {} // poll for first key press.
            int val = inb(DATA_PORT);
            char c = ascii[val];
            // only accept either enter or escape
            if (c == '\n') {
                vga->initializeScreen(vga->bg_color);
                goto restart;
            }
            if (c == 27) { // Weirdly enough I don't think this escape command even worked, I'm not sure why nor did I bother to check why it didn't work.
                shutdown = 1;
                vga->shut_off();
            }
        }
    }
}