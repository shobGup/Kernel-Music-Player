#include "ide.h"
#include "ext2.h"
#include "libk.h"
#include "threads.h"
#include "semaphore.h"
#include "future.h"
#include "pit.h"
#include "physmem.h"
#include "atomic.h"

// this header contains all the information from the song file
struct Header {
    char fmt_chunk_marker[4];          
    uint32_t length_of_fmt;                 
    uint16_t format_type;                   
    uint16_t channels;                      
    uint32_t sample_rate;                   
    uint32_t byte_rate;                      
    uint16_t bit_channels;                   
    uint16_t bits_per_sample;               
};


class WaveParser_list {

    public:
    Header * fmt;
    char * b_entries; 
    uint64_t offset; 
    uint64_t reset_offset; 
    Atomic<uint32_t> howMuchRead{0};
    uint32_t size; 
    uint32_t size_of_the_junk;
    Shared<Node> overallFile; 
    uint32_t size_of_the_whole_file;
    Atomic<uint32_t> ref_count{0};
    uint32_t bit_divsor; 
    uint32_t bit_per_sample; 

    WaveParser_list(Shared<Node> file) {
        overallFile = file; 
        offset = 0; 
        fmt = new Header();
        b_entries = (char *) PhysMem::alloc_frame();
        size_of_the_whole_file = 0;

        // RIFF CHECK 
        char* riff = new char[5];
        riff[4] = '\0';
        file->read_all(0, 4, riff);
        delete riff; 

        // - Size of File 

        char * size_of_file = new char[5];
        file->read_all(4, 4, size_of_file);
        size_of_the_whole_file = *(uint32_t *)size_of_file;
        delete size_of_file; 

        // WAVE CHECK 
        char* wave = new char[5];
        wave[4] = '\0';
        file->read_all(8, 4, wave);
        delete wave; 

        Header* fmt_stuff = fmt; 

        // reading the important info from the WAV file
        file->read_all(12, sizeof(Header), (char*)fmt_stuff);

        /*
            the contents of the SDnfmt register depends on the infroamtion from the header struct
            so based on it we set the bibts per sample and sample rate
        */
        if(fmt_stuff->sample_rate == 16000) {
            bit_divsor = 0x200; 
        } else {
            bit_divsor = 0x500; 
        }

        if(fmt_stuff->bits_per_sample == 16) {
            bit_per_sample = 0x10; 
        } else {
            bit_per_sample = 0; 
        }

        // Junk Check 
        char* junk = new char[5];
        junk[4] = '\0';
        file->read_all(12 + sizeof(Header), 4, junk);
        delete junk; 

        // Size of junk
        char * size_of_junk = new char[5];
        file->read_all(12 + sizeof(Header) + 4, 4, size_of_junk);
        size_of_the_junk = 12 + sizeof(Header) + 4 + *(uint32_t *) size_of_junk + 4;
        delete size_of_junk;

        // Data Check 
        char * data_chunk_header = new char[5];               // DATA string or FLLR string
        data_chunk_header[4] = '\0';
        file->read_all(size_of_the_junk, 4, data_chunk_header);
        size_of_the_junk += 4; 
        delete data_chunk_header; 
                    
        // NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
        char * data_size = new char[5];
        file->read_all(size_of_the_junk, 4, data_size);
        size = *(uint32_t*)data_size; 
        size_of_the_junk += 4;
        delete data_size;

        // Make 16 Pages of data 

        for(int i = 0; i < 16; i++) {
            char * current_entry = (b_entries + (i * 16));
            *(uint64_t *) current_entry = PhysMem::alloc_frame();
            uint64_t current_addy = *(uint64_t *) current_entry; 
            ASSERT(current_addy == *(uint64_t *) current_entry);
            *(uint32_t *) (current_entry + 8) = 4096; 
            *(uint32_t *) (current_entry + 12) = 0;
            file->read_all(size_of_the_junk+ (4096 * i), 4096, (char*) (uint64_t*)current_addy);
            offset = size_of_the_junk + (4096 * i) + 4096; 
            reset_offset = size_of_the_junk + (4096 * i) + 4096; 
        }
    }

    /*
        This function rebbuilds all the data of the current buffer of
        the song all over again
        Its called when we want to reset the song
    */
    void rebuildData(uint32_t index) {
        char * current_entry = (b_entries + (index * 16));
        uint64_t current_addy = *(uint64_t *) current_entry; 
        overallFile->read_all(offset, 4096, (char*) (uint64_t*)current_addy);
        offset+=4096; 

    }

    /*
        This function zeroes out all of data from the 16 buffers
    */
    void rebuildDataZero(uint32_t index) {
        char * current_entry = (b_entries + (index * 16));
        uint64_t current_addy = *(uint64_t *) current_entry; 
        bzero((void*)(uint64_t*)current_addy,4096);
        howMuchRead.set(0);
    }

};