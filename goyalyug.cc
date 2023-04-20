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

void changeFile(Shared<WaveParser_list> file, Shared<WaveParser_list> oldFile) {

}

void kernelMain(void) {

    Shared<Names_List> fileSystem = Shared<Names_List>::make();
    // 0x8000200

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

    // Esuring the Device is being Turned On 
    turnOnDevice(base_u);

    // auto ide = Shared<Ide>::make(1);
    
    // // We expect to find an ext2 file system there
    // auto fs = Shared<Ext2>::make(ide);

    // // VGA *thisVGA = new VGA();
    // // thisVGA->setup(fs);

    // Debug::printf("*** block size is %d\n",fs->get_block_size());
    // Debug::printf("*** inode size is %d\n",fs->get_inode_size());
   
    // auto root = fs->root;

    // auto hello = fs->find(root,"swift_");

    // Shared<WaveParser_list> currentFile = Shared<WaveParser_list>::make(hello);

    // Debug::printf("Addy: %x\n",currentFile->b_entries);

    // uint16_t GCAP = * (uint16_t *)base; 
    // Debug::printf("GCAP: %x\n", GCAP);
    // Debug::printf("Value ISS:%d\n", (GCAP >> 8) & 0xF); // 4

    auto currentNode = fileSystem->findName("swift",fileSystem->dummy);
    auto currentFile = currentNode->wave_file; 

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

    VGA *thisVGA = new VGA();
    // Debug::printf("This VGA: %x\n",thisVGA );
    thisVGA->setup(fileSystem, currentNode ,1);

    Shared<kb> thisKB = Shared<kb>::make(thisVGA);
    Shared<WaveParser_list>* my_wave = &(currentFile);

    thread([thisVGA, my_wave] {
        // thisVGA->progressBarInit();
        thisVGA->last_jif = Pit::jiffies;
        while(true) {
            uint32_t percentage = ((*my_wave)->howMuchRead.get() * 100) / (*my_wave)->size;
            // Debug::printf("percentage: %d, read in: %d, size: %d\n", percentage, (*my_wave)->howMuchRead.get(), (*my_wave)->size);
            thisVGA->playingSong(percentage);
        }
    });

    thread([thisKB] {
        thisKB->kbInit();
    });

    while(thisKB->tapped) {
        Debug::printf("WTF Man\n");
    }
    
    while(true) {
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
            
            // Reset Sound 
            written = 0; 
            index = 0; 
            currentFile->offset = currentFile->reset_offset;
            reset(currentFile);

            // Reset VGA
            currentFile->howMuchRead.set(0);
            thisVGA->new_song = true; 
            thisVGA->elapsed_time.set(0); 

            Debug::printf("Should be Reset\n");
        }

        // Space Bar
        if(thisKB->tapped) {

            // Change playing mode 
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

            // Changes File 
            currentFile = currentNode->next->wave_file;
            currentNode = currentNode->next; 

            /* VGA Animation */
            thisVGA->spotify(currentNode, true);

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

            // Changes File 
            currentFile = currentNode->prev->wave_file;
            currentNode = currentNode->prev; 

            /* VGA Animation */
            thisVGA->spotify(currentNode, true);

            reset(currentFile);
        }

        if(thisKB->entered) {
            thisKB->entered = false; 

            // Turn Off Sound 
            *((uint32_t*)SDnCTL) = (*((uint32_t*)SDnCTL) & (0xFFFFFFFD));

            // Reset Offsets 
            written = 0; 
            index = 0; 
            currentFile->offset = currentFile->reset_offset;
            currentFile->howMuchRead.set(0);
            thisVGA->new_song = true; 
            thisVGA->elapsed_time.set(0); 

            Debug::printf("File Found: %s\n", thisKB->filename);

            // Changes File 
            currentNode = fileSystem->findName((const char *) thisKB->filename, currentNode);
            currentFile = currentNode->wave_file;

            Debug::printf("Came Back from life\n");

            /* VGA Animation */
            thisVGA->spotify(currentNode, true);

            reset(currentFile);
        }

   }

}