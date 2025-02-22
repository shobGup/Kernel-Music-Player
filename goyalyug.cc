#include "ide.h"
#include "ext2.h"
#include "libk.h"
#include "threads.h"
#include "semaphore.h"
#include "future.h"
#include "pit.h"
// #include "list_wave.h"
#include "vga.h"
#include "kb.h"
// #include "names.h"


// this function is used to get the response from the register after givinf it all the information in the set command 
// function
uint32_t get_response(char *base) {

    while((((*(uint32_t *) (base + 0x68)) & 0x2) >> 1) != 1) {
        Debug::printf("Response Not Ready\n");
    }

    ASSERT((((*(uint32_t *) (base + 0x68)) & 0x2) >> 1) == 1);

    Debug::printf("Response is ready to be retrieved, reading from: %x\n", (base + 0x64));

    // reset IRV
    (*(uint32_t *) (base + 0x68)) = (*(uint32_t *) (base + 0x68)) & 0xFFFFFFFD; 


    return *((uint32_t *) (base + 0x64));

}

/*
    in this function we pass the codec address, the node number, the command which we want the info about
    and the data to send in information about the things we want the codec to do
    Sometimes we use this function to set the speaker, turn the speaker on (mute/gain function)
    set the stream channel, set the pin widget control and various things
    we make a final command using all the parameters and then send that command to a particular
    register (base + 60h) which is a Immediate Command Output Interface register so that the device knows what we want
*/
bool send_command(uint32_t codec, uint32_t node, uint32_t command, uint32_t data, char *base) {

    Debug::printf("Send Command\n");

    codec = (codec & 0xf) << 28; 
    node = (node & 0xff) << 20; 
    command = (command & 0xfff) << 8; 
    data = data & 0xff; 

    uint32_t final_command = codec + node + command + data; 

    uint32_t * command_addy = &final_command; 

    ASSERT(*command_addy == final_command);

    Debug::printf("Setting ICB bit to 0\n");
    // Set ICB to 0 
    (*(uint32_t *)(base + 0x68)) = (*(uint32_t *)(base + 0x68)) & 0xFFFFFFFE; // (setting bit 0 to 0)

    // ASSERT((*(uint32_t *) (base + 0x68) & 0x1) == 0);

    Debug::printf("Setting IRV bit to 0\n");
    // Set IRV to 0 
    (*(uint32_t *)(base + 0x68)) = (*(uint32_t *)(base + 0x68)) & 0xFFFFFFFD; // (setting bit 1 to 0)

    // ASSERT(((*(uint32_t *) (base + 0x68) & 0x2) >> 1) == 0);

    Debug::printf("Setting ICOI to final command\n");
    // Sending request To ICOI

    Debug::printf("Setting ICOI ~ FINAL COMMAND: %x\n", *command_addy);
    Debug::printf("Setting ICOI ~ Mem Addy: %x\n", *(uint32_t *) (base + 0x60));

    // memcpy((void *)(base + 0x60), (void *)command_addy, 4);
    Debug::printf("Writing too: %x\n", (base + 0x60));
    *(uint32_t *)(base + 0x60) = final_command; 

    Debug::printf("Setting ICOI ~ FINAL COMMAND ~ AFTER: %x\n", *command_addy);
    Debug::printf("Setting ICOI ~ Mem Addy ~ AFTER: %x\n", *(uint32_t *) (base + 0x60));

    ASSERT(*(uint32_t *) (base + 0x60) == final_command);

    Debug::printf("Setting ICB bit to 1 ~ command is valid\n");
    // Set ICB to 1
    (*(uint32_t *) (base + 0x68)) = (*(uint32_t *) (base + 0x68)) | 0x1; // (setting bit 0 to 1)

    // ASSERT((*(uint32_t *) (base + 0x68) & 0x1) == 1);

    return true; 
}


/*
    This funtion serves similar to the send command function above but in this function
    the number of bits for the command and data are different as mentioned in the 
    Intel HDA documentation. Otherwise it serves the same purpose as before

*/
bool send_comman_extended(uint32_t codec, uint32_t node, uint32_t command, uint32_t data, char *base) {

    Debug::printf("Send Command\n");

    codec = (codec & 0xf) << 28; 
    node = (node & 0xff) << 20; 
    command = (command & 0xf) << 16; 
    data = data & 0xffff; 

    uint32_t final_command = codec + node + command + data; 

    uint32_t * command_addy = &final_command; 

    ASSERT(*command_addy == final_command);

    Debug::printf("Setting ICB bit to 0\n");
    // Set ICB to 0 
    (*(uint32_t *)(base + 0x68)) = (*(uint32_t *)(base + 0x68)) & 0xFFFFFFFE; // (setting bit 0 to 0)

    // ASSERT((*(uint32_t *) (base + 0x68) & 0x1) == 0);

    Debug::printf("Setting IRV bit to 0\n");
    // Set IRV to 0 
    (*(uint32_t *)(base + 0x68)) = (*(uint32_t *)(base + 0x68)) & 0xFFFFFFFD; // (setting bit 1 to 0)

    // ASSERT(((*(uint32_t *) (base + 0x68) & 0x2) >> 1) == 0);

    Debug::printf("Setting ICOI to final command\n");
    // Sending request To ICOI

    Debug::printf("Setting ICOI ~ FINAL COMMAND: %x\n", *command_addy);
    Debug::printf("Setting ICOI ~ Mem Addy: %x\n", *(uint32_t *) (base + 0x60));

    // memcpy((void *)(base + 0x60), (void *)command_addy, 4);
    Debug::printf("Writing too: %x\n", (base + 0x60));
    *(uint32_t *)(base + 0x60) = final_command; 

    Debug::printf("Setting ICOI ~ FINAL COMMAND ~ AFTER: %x\n", *command_addy);
    Debug::printf("Setting ICOI ~ Mem Addy ~ AFTER: %x\n", *(uint32_t *) (base + 0x60));

    ASSERT(*(uint32_t *) (base + 0x60) == final_command);

    Debug::printf("Setting ICB bit to 1 ~ command is valid\n");
    // Set ICB to 1
    (*(uint32_t *) (base + 0x68)) = (*(uint32_t *) (base + 0x68)) | 0x1; // (setting bit 0 to 1)

    // ASSERT((*(uint32_t *) (base + 0x68) & 0x1) == 1);

    return true; 
}


/*
    In this function we are passed the current node (song) which want to restart
    We are zeroing out all the 16 bubffers and rebuilding the data all over again
    In simple words we are restarting the song

*/
void reset(Shared<WaveParser_list> currentFile) {

    char *base = (char *) 0xfebf0000;
    char * base_addy_plus_x = (char *) (base + (0x80 + 4 * 0x20)); 
    char * SDnCTL = (base_addy_plus_x); 

    *((uint32_t*)SDnCTL) = (*((uint32_t*)SDnCTL) & (0xFFFFFFFD));

    // SDnBDL Lower Set Up 
    // Debug::printf("Addy: %x\n",currentFile.b_entries);
    uint32_t entries = (uint32_t) (currentFile->b_entries + (1 * 16));
    *(uint32_t *)(base_addy_plus_x + 0x18) = entries; 
    ASSERT((*(uint32_t *)(base_addy_plus_x + 0x18)) == entries);
    // Debug::printf("Addy ~ Entries: %x\n",entries);

    for(int x = 0; x < 16; x++) {
        currentFile->rebuildDataZero(x);
    }

    for(int x = 0; x < 16; x++) {
        currentFile->rebuildData(x);
    }

    *((uint32_t*)SDnCTL) = (*((uint32_t*)SDnCTL) | 0x2);
}

/*
    This is one of the important functions which makes sure that we are ready to play the song
    First we are supposed to set the set the SDnBDL register which contains the address of 
    where the bbuffer descriptor table is

    Then we set the LVI which is the last valid index which will be 15 for us since 
    we are dealing with 16 buffers

    Then we set the SDnCBL register which contains the Cyclic Buffer Length which
    is going to be 16 * 4096 for us

    Next we set the SDnFMT register which will contains a 32 bits integer which is put together
    after finding the Sample Base Rate, Sample Base Rate Multiple, Sample Base Rate Divisor,
    Bits per Sample, Number of Channels. All of these information are extracted from the WAV file
    (in the list_wave.h class)

*/
uint16_t ready_to_play(char * base, Shared<WaveParser_list> currentFile) {

    char * base_addy_plus_x = (char *) (base + (0x80 + 4 * 0x20)); 

    // SDnBDL Lower Set Up 
    Debug::printf("Addy: %x\n",currentFile->b_entries);
    uint32_t entries = (uint32_t) currentFile->b_entries;
    *(uint32_t *)(base_addy_plus_x + 0x18) = entries; 
    ASSERT((*(uint32_t *)(base_addy_plus_x + 0x18)) == entries);
    Debug::printf("Addy ~ Entries: %x\n",entries);

    // LVI -> 15 
        // *(uint16_t *)(LVI) = 15; 
    char * LVI = base_addy_plus_x + 12; 
    uint16_t num = 15; 
    uint16_t * value = &num; 
    memcpy(LVI, value, 2);
    ASSERT(*(uint16_t *)(base_addy_plus_x + 0xC) == 15);
    Debug::printf("LVI: %d\n", *(uint16_t *)(base_addy_plus_x + 0xC));

    // SDnCBL -> 16 * 4096 
    char * CBL = (base_addy_plus_x + 0x8); 
    uint32_t CBL_num = 65536; 
    uint32_t * CBL_value = &CBL_num; 
    *(uint32_t *)(CBL) = *CBL_value;
    Debug::printf("Value of CBL: %d and CLB Num: %d\n", *CBL_value, CBL_num); 
    Debug::printf("CBL: %d\n", *(uint32_t *)((base_addy_plus_x + 0x8)));
    ASSERT(*(uint32_t *)((base_addy_plus_x + 0x8)) == (4096*16));
    

    // SDnFMT -> 
    char * FMT = (base_addy_plus_x + 0x12); 

    uint16_t current_FMT = *(uint16_t *)(FMT);

    Debug::printf("Current FMT: %x\n", current_FMT);

    current_FMT = current_FMT & 0x8080; 
    current_FMT += (currentFile->bit_divsor + currentFile->bit_per_sample); 
    Debug::printf("After FMT: %x\n", current_FMT);
    *(uint16_t *)(FMT) = current_FMT;


    Debug::printf("FMT: %x\n", *(uint16_t *)((base_addy_plus_x + 0x12)));
    return current_FMT;
    // ASSERT(*(uint16_t *)((base_addy_plus_x + 0x12)) == (1280));
}

/*
    After traversing the memory to find the device we use the base address + 0x8
    to find the GCTL register and turn on the CRST bit to turn on the device
*/
void turnOnDevice(uint32_t *base_u) {
    
    while(*(base_u + 2) == 0) {
        Debug::printf("It's not on Yet, %x\n", *(base_u + 2));
    }

    Debug::printf("DEVICE IS TURNED ON: %x\n", (*(base_u + 2)));

    volatile uint32_t help = 0; 

    while(help  < 10000) {
        help += 1; 
    }
}

/*
    This function is used to flip the bit of SDnCTL register which technically means 
    if the stream is running or not
    (used to pause and play the song)
*/
void flipBit() {
    char *base = (char *) 0xfebf0000;
    char * base_addy_plus_x = (char *) (base + (0x80 + 4 * 0x20)); 
    char * SDnCTL = (base_addy_plus_x); 
    if(*((uint32_t*)SDnCTL) == 0x20100002) {
        *((uint32_t*)SDnCTL) = (*((uint32_t*)SDnCTL) ^ 0x2); 
    } else {
        *((uint32_t*)SDnCTL) = (*((uint32_t*)SDnCTL) | 0x2); 
    }
    Debug::printf("In Method SDnCTL: %x\n", (*((uint32_t*)SDnCTL)));
}

/*
    Magic happens here
*/

void kernelMain(void) {

    Shared<Names_List> fileSystem = Shared<Names_List>::make();

    // PCI controll register -> command register -> outl (addy we want to read) -> inl to read what it responds
    outl(0xCF8, 0x80002004);
    uint32_t Status_Command_Regis =  inl(0xCFC) | 0x4;
    outl(0xCF8, 0x80002004);
    outl(0xCFC, Status_Command_Regis);

    // Base Adderress 
    char *base = (char *) 0xfebf0000;
    uint32_t *base_u = (uint32_t *) 0xfebf0000;

    // CRST Bit being flipped 
    *(base_u + 2) = *(base_u + 2) | 0x01;


    Debug::printf("After flipping bit: GCTL: %x\n", *(base_u + 2));

    // VMAJ
    ASSERT(*((base + 3)) == 1);
    
    // VMIN
    ASSERT(*((base + 2)) == 0);

    Debug::printf("After flipping bit: GCAP: %x\n", *base_u);

    Shared<Semaphore> startSpot = Shared<Semaphore>::make(0);

    // the first song that plays when we start the program
    auto currentNode = fileSystem->findName("breathe in the air",fileSystem->dummy);
    auto currentFile = currentNode->wave_file;

    // setting up the graphics
    VGA *thisVGA = new VGA();
    thisVGA->setup(fileSystem, currentNode ,1);

    Shared<kb> thisKB = Shared<kb>::make(thisVGA);
    thread([thisKB, startSpot] {
        auto ide = Shared<Ide>::make(1);
        // We expect to find an ext2 file system there
        auto fs = Shared<Ext2>::make(ide);
        auto root = fs->root;
        auto logo = fs->find(root,"logo");

        thisKB->kbInit(logo, startSpot);
    });

    

    


    // Esuring the Device is being Turned On 
    turnOnDevice(base_u);

    // ready to play returns the format which will be used later for commands
    uint16_t fmt = ready_to_play(base, currentFile);


    // Send SetPinWidgetControl to Node 3 ~ 0x707
    // 01000000

    // ~ Gets the inital value that is there 
    send_command(0, 3, 0xf07, 0, base);

    uint32_t current_pinCntl = get_response(base);
    current_pinCntl |= 0x40; 
    send_command(0, 3, 0x707, current_pinCntl, base); 

    // Send SetStreamChannel to Node 2 ~ 0x706 
    // 0x706 
    // 0001
    // 0000
    send_command(0, 2, 0x706, 0x10, base);

    // SetAmplifierGain 
    send_comman_extended(0, 2, 0x3, 0xB035, base);

    // 1 0 1 1 0 0 0 0 "0" '......' 
    // B035
    send_comman_extended(0, 2, 0x2, fmt, base);

    // SDnCTL -> Set RUN -> 1 
    char * base_addy_plus_x = (char *) (base + (0x80 + 4 * 0x20)); 
    char * SDnCTL = (base_addy_plus_x); 
    *((uint32_t*)SDnCTL) = (*((uint32_t*)SDnCTL) + 0x100000);
    // *((uint32_t*)SDnCTL) = (*((uint32_t*)SDnCTL) + 0x2);
    Debug::printf("SDnCTL: %x\n", (*((uint32_t*)SDnCTL)));

   // DPLBASE ~ Sanity Check ~ FAIL ~ RIP ~ WE ARE INSANE
    // uint64_t offset = 4096;
    uint64_t written = 0; 
    uint32_t index = 0; 
    // uint32_t size = currentFile->size_of_the_whole_file;

    Shared<WaveParser_list>* my_wave = &(currentFile);

    Debug::printf("Calling Down\n");

    startSpot->down();

    Debug::printf("After down\n");

    thisVGA->spotify(currentNode, false);


    thisVGA->new_song = true; 
    
    bool isItDown = false; 

    bool * keepGoing = &isItDown; 

    thread([thisVGA, my_wave, keepGoing] { // purpose of this thread is to keep giving the percentage of the song to the vga to draw
        // thisVGA->progressBarInit();
        thisVGA->last_jif = Pit::jiffies;
        while(!(*keepGoing)) {
            uint32_t percentage = ((*my_wave)->howMuchRead.get() * 100) / (*my_wave)->size;
            // Debug::printf("percentage: %d, read in: %d, size: %d\n", percentage, (*my_wave)->howMuchRead.get(), (*my_wave)->size);
            thisVGA->playingSong(percentage);
        }
    });


    while(thisKB->tapped) {
        Debug::printf("WTF Man\n");
    }
    
    while(true) {
        /* 
            makes sure the hardware and software are in sync and there are no race condition
        */
        volatile uint32_t hardware_offset = *(volatile uint32_t*) (base_addy_plus_x + 0x4);
        if (((hardware_offset - written) % 65536) > 4096) {
            currentFile->howMuchRead.fetch_add(4096); 
            currentFile->rebuildData(index++);
            written += 4096;
            written %= 65536;
            index %= 16; 
        }

        // Done with the song
        if(currentFile->howMuchRead.get() >= currentFile->size) {
        
            // Turn Off Sound 
            *((uint32_t*)SDnCTL) = (*((uint32_t*)SDnCTL) & (0xFFFFFFFD));

            // Reset Offsets 
            written = 0; 
            index = 0; 
            currentFile->offset = currentFile->reset_offset;
            currentFile->howMuchRead.set(0);
            thisVGA->new_song = true; 
            thisVGA->elapsed_time.set(0); 
            
            /* VGA Animation */
            thisVGA->spotify_move(currentNode, true, false);

            // Changes File 
            currentNode = currentNode->next; 

            // skip the dummy node
            if(K::streq(currentNode->file_name, "")) {
                currentNode = currentNode->next;
            }
            currentFile = currentNode->wave_file;

            // after the song is done playing it restarts the song again
            reset(currentFile);

            Debug::printf("Should be Reset\n");
        }

        // Space Bar
        if(thisKB->tapped) {

            // Change playing mode if playing then pause and if paused then plays
            flipBit();
            thisKB->tapped = false; 

            // VGA 
            thisVGA->play_pause();
        } 

        // Down Arrow
        if(thisKB->reset) {
            thisKB->reset = false; 

            // Reset Offset 
            currentFile->offset = currentFile->reset_offset;
            written = 0; 
            index = 0; 

            // Reset Buffer 
            reset(currentFile);

            // VGA Reset
            currentFile->howMuchRead.set(0);
            thisVGA->new_song = true; 
            thisVGA->elapsed_time.set(0); 

            Debug::printf("Should be Reset\n");
        }

        // Previous Song
        if(thisKB->precend) {
            thisKB->precend = false; 

            // Turn Off Sound 
            *((uint32_t*)SDnCTL) = (*((uint32_t*)SDnCTL) & (0xFFFFFFFD));

            // Reset Offsets 
            written = 0; 
            index = 0; 
            currentFile->offset = currentFile->reset_offset;
            currentFile->howMuchRead.set(0);
            thisVGA->new_song = true; 
            thisVGA->elapsed_time.set(0); 

            /* VGA Animation */
            thisVGA->spotify_move(currentNode, true, true);

            // Changes File 
            currentNode = currentNode->prev; 

            // skip the dummy node
            if(K::streq(currentNode->file_name, "")) {
                currentNode = currentNode->prev;
            }

            currentFile = currentNode->wave_file;

            reset(currentFile);

        }

        // Next Song
        if(thisKB->skip) {
            thisKB->skip = false; 

            // Turn Off Sound 
            *((uint32_t*)SDnCTL) = (*((uint32_t*)SDnCTL) & (0xFFFFFFFD));

            // Reset Offsets 
            written = 0; 
            index = 0; 
            currentFile->offset = currentFile->reset_offset;
            currentFile->howMuchRead.set(0);
            thisVGA->new_song = true; 
            thisVGA->elapsed_time.set(0); 

            /* VGA Animation */
            thisVGA->spotify_move(currentNode, true, false);

            // Changes File 
            currentNode = currentNode->next; 

            // skip the dummy node
            if(K::streq(currentNode->file_name, "")) {
                currentNode = currentNode->next;
            }
            currentFile = currentNode->wave_file;


            reset(currentFile);
        }

        // Enter ~ search song 
        if(thisKB->entered) {
            thisKB->entered = false; 

            // Changes File 
            auto temp = fileSystem->findName((const char *) thisKB->filename, currentNode);

            if(!K::streq(temp->file_name, "")) {

                // Turn Off Sound 
                *((uint32_t*)SDnCTL) = (*((uint32_t*)SDnCTL) & (0xFFFFFFFD));

                currentNode = temp; 
                currentFile = currentNode->wave_file;


                // Reset Offsets 
                written = 0; 
                index = 0; 
                currentFile->offset = currentFile->reset_offset;
                currentFile->howMuchRead.set(0);
                thisVGA->new_song = true; 
                thisVGA->elapsed_time.set(0); 

                /* VGA Animation */
                thisVGA->spotify(currentNode, true);

                reset(currentFile);
            } else {
                thisVGA->drawRectangle(70, 9, 250, 19, 63, 1); // text box
                thisVGA->drawString(96, 10, (const char*)"NOT A VALID SONG", 48); // enter spotify
            }

        }

        // Shutoff Screen 
        if(thisKB->shutdown) {
            // Turn Off Sound 
            *((uint32_t*)SDnCTL) = (*((uint32_t*)SDnCTL) & (0xFFFFFFFD));

            thisKB->shutdown = false; 
            isItDown = true; 
            thisVGA->shut_off();
            
        }

   }

}