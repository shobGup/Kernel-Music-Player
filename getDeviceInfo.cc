#include "ide.h"
#include "ext2.h"
#include "libk.h"
#include "threads.h"
#include "semaphore.h"
#include "future.h"
#include "pit.h"
#include "wave.h"

/*
    Credit to OS dev
*/

uint16_t vendorID;
uint8_t headerType;


uint16_t readConfigDWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, bool b) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint16_t tmp = 0;
 
    // Create configuration address as per Figure 1
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

    if(b) {
        Debug::printf("Address: %x\n", address);
    }
    
    // Write out the address
    outl(0xCF8, address);
    // Read in the data
    // (offset & 2) * 8) = 0 will choose the first word of the 32-bit register
    tmp = (uint16_t)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
    return tmp;
}

uint32_t readConfigDWord_prive(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, bool b) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint32_t tmp = 0;
 
    // Create configuration address as per Figure 1
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

    if(b) {
        Debug::printf("Address: %x\n", address);
    }
    
    // Write out the address
    outl(0xCF8, address);
    // Read in the data
    // (offset & 2) * 8) = 0 will choose the first word of the 32-bit register
    tmp = inl(0xCFC); 
    return tmp;
}

// Function to get the Vendor ID of a device
uint16_t getVendorID(uint8_t bus, uint8_t device, uint8_t function) {
    uint32_t val = readConfigDWord(bus, device, function, 0x00, false);
    return val & 0xFFFF;
}

uint16_t getVendorID_priv(uint8_t bus, uint8_t device, uint8_t function) {
    uint32_t val = readConfigDWord(bus, device, function, 0x00, true);
    return val & 0xFFFF;
}

// Function to get the Device ID of a device
uint16_t getDeviceID(uint8_t bus, uint8_t device, uint8_t function) {
    uint32_t val = readConfigDWord(bus, device, function, 0x02, false);
    return (val) & 0xFFFF;
}


// Function to get the Class ID of a device
uint8_t getClassID(uint8_t bus, uint8_t device, uint8_t function) {
    uint32_t val = readConfigDWord(bus, device, function, 0x08, false);
    return (val >> 24) & 0xFF;
}

uint16_t getCommandRegister(uint8_t bus, uint8_t device, uint8_t function) {
    uint32_t val = readConfigDWord(bus, device, function, 0x04, false);
    return val & 0xFFFF;
}

// Function to get the Subclass ID of a device
uint8_t getSubclassID(uint8_t bus, uint8_t device, uint8_t function) {
    uint32_t val = readConfigDWord(bus, device, function, 0x08, false);
    return (val >> 16) & 0xFF;
}

// Function to get the Header Type of a device
uint8_t getHeaderType(uint8_t bus, uint8_t device, uint8_t function) {
    uint32_t val = readConfigDWord(bus, device, function, 0x0C, false);
    return (val >> 16) & 0xFF;
}

// Function to get the Header Type of a device
uint32_t getBarZero(uint8_t bus, uint8_t device, uint8_t function) {
    uint32_t val = readConfigDWord_prive(bus, device, function, 0x10, false);
    return val;
}

void checkFunction(uint8_t bus, uint8_t device, uint8_t function) {

    int vId = getVendorID_priv(bus, device, function);
    Debug::printf("vendor id %x\n", vId);
    int dId = getDeviceID(bus, device, function);
    Debug::printf("device id %x\n", dId);
    uint32_t barz = getBarZero(bus, device, function);
    Debug::printf("Bar 0: %x \n", barz);
}

void checkDevice(uint8_t bus, uint8_t device) {
    uint8_t function = 0;

    vendorID = getVendorID(bus, device, function);
    if (vendorID == 0xFFFF) return; // Device doesn't exist
    checkFunction(bus, device, function);
    headerType = getHeaderType(bus, device, function);
    if( (headerType & 0x80) != 0) {
        // It's a multi-function device, so check remaining functions
        for (function = 1; function < 8; function++) {
            if (getVendorID(bus, device, function) != 0xFFFF) {
                checkFunction(bus, device, function);
            }
        }
    }
}


void checkAllBuses(void) {
    uint16_t bus;
    uint8_t device;

    for (bus = 0; bus < 256; bus++) {
        for (device = 0; device < 32; device++) {
            checkDevice(bus, device);
        }
    }
}

void kernelMain(void) {

    checkAllBuses();

    Debug::printf("Finding Codec\n");

    Debug::printf("After flipping bit: STATESTS: %x\n", *(base + 0x0E));


    Debug::printf("ATTEMPT TO SEND COMMAND: get number of nodes\n");
    send_command(0, 0, 0xf00, 0x4, base);
    Debug::printf("We got back from sending command\n");

    uint32_t result = get_response(base);
    uint32_t nodes = result & 0xFF;
    
    Debug::printf("ATTEMPT TO GET RESULT: get number of nodes %d\n", nodes);

    send_command(0, 1, 0xf00, 0x4, base);
    result = get_response(base);
    nodes = result & 0xFF;
    
    Debug::printf("ATTEMPT TO GET RESULT for 1: get number of nodes %d\n", nodes);


    send_command(0, 2, 0xf00, 0x9, base);
    result = get_response(base);
    
    Debug::printf("ATTEMPT TO GET RESULT for 2: TYPE ~ %x\n", result);

    send_command(0, 3, 0xf00, 0x9, base);
    result = get_response(base);
    
    Debug::printf("ATTEMPT TO GET RESULT for 3: TYPE ~ %x\n", result);

    send_command(0, 4, 0xf00, 0x9, base);
    result = get_response(base);
    
    Debug::printf("ATTEMPT TO GET RESULT for 4: TYPE ~ %x\n", result);

    send_command(0, 5, 0xf00, 0x9, base);
    result = get_response(base);
    
    Debug::printf("ATTEMPT TO GET RESULT for 5: TYPE ~ %x\n", result);


    send_command(0, 0, 0xf00, 0x0, base);

    result = get_response(base);

    Debug::printf("ATTEMPT TO GET RESULT: get number of nodes %x\n", result);

}
