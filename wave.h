#include "ide.h"
#include "ext2.h"
#include "libk.h"
#include "threads.h"
#include "semaphore.h"
#include "future.h"
#include "pit.h"

struct Header {
    char fmt_chunk_marker[4];          // fmt string with trailing null char
    uint32_t length_of_fmt;                 // length of the format data
    uint16_t format_type;                   // format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
    uint16_t channels;                      // no.of channels
    uint32_t sample_rate;                   // sampling rate (blocks per second)
    uint32_t byte_rate;                      // SampleRate * NumChannels * BitsPerSample/8
    uint16_t bit_channels;                   // NumChannels * BitsPerSample/8
    uint16_t bits_per_sample;               // bits per sample, 8- 8bits, 16- 16 bits etc

    // //questionable
    char data_chunk_header [4];               // DATA string or FLLR string
    uint32_t  data_size;                     // NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
};



void setUpWaveFile(Shared<Node> file) {


    // RIFF CHECK 
    char* riff = new char[5];
    riff[4] = '\0';
    file->read_all(0, 4, riff);
    Debug::printf("WOOF WOOF: %s\n", riff);
    delete riff; 

    // - Size of File 

    char * size_of_file = new char[5];
    file->read_all(4, 4, size_of_file);
    Debug::printf("Size Of File: %d\n", *(uint32_t *)size_of_file);
    delete size_of_file; 



    // WAVE CHECK 
    char* wave = new char[5];
    wave[4] = '\0';
    file->read_all(8, 4, wave);
    Debug::printf("Is it wave: %s\n", wave);
    delete wave; 

    // Junk Check 
    char* junk = new char[5];
    junk[4] = '\0';
    file->read_all(12, 4, junk);
    Debug::printf("Is it junk: %s\n", junk);
    delete wave; 

    // Size of junk
    char * size_of_junk = new char[5];
    file->read_all(16, 4, size_of_junk);
    uint32_t size_of_the_junk = *(uint32_t *)size_of_junk % 2 == 0 ? *(uint32_t *)size_of_junk : *(uint32_t *)size_of_junk + 1; 
    Debug::printf("Size Of junk: %d\n", size_of_the_junk);
    delete size_of_junk; 

    // Skip Junk 


    Header* fmt_stuff = new Header();
    Debug::printf("Size Of fmt: %d\n", sizeof(Header));
    file->read_all(20 + size_of_the_junk, sizeof(Header), (char*)fmt_stuff);
    Debug::printf("fmt_chunk_marker: %s\n", fmt_stuff->fmt_chunk_marker);
    Debug::printf("length_of_fmt: %d\n", fmt_stuff->length_of_fmt);
    Debug::printf("format_type: %d\n", fmt_stuff->format_type);
    Debug::printf("channels: %d\n", fmt_stuff->channels);
    Debug::printf("sample_rate: %d\n", fmt_stuff->sample_rate);
    Debug::printf("byte_rate: %d\n", fmt_stuff->byte_rate);
    Debug::printf("bit_channels: %d\n", fmt_stuff->bit_channels);
    Debug::printf("bits_per_sample: %d\n", fmt_stuff->bits_per_sample);
    Debug::printf("data_chunk_header: %s\n", fmt_stuff->data_chunk_header);
    Debug::printf("data size: %d\n", fmt_stuff->data_size);
    


    // "FMT " Check 

    // Extract and Fill Info 

    // DATA check 

    // Make 16 Pages of data 

}