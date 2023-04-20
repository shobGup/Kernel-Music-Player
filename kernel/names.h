#include "ide.h"
#include "ext2.h"
#include "libk.h"
#include "threads.h"
#include "semaphore.h"
#include "future.h"
#include "pit.h"
#include "physmem.h"
#include "list_wave.h"

struct File_Node {
    Atomic<uint32_t> ref_count{0};
    Shared<File_Node> prev;
    Shared<File_Node> next;
    Shared<WaveParser_list> wave_file;
    char * file_name;
    Shared<Node> small;
    Shared<Node> big;
};

class Names_List {
    public:
    Atomic<uint32_t> ref_count{0};
    Shared<File_Node> dummy;
    Names_List() {

        auto ide = Shared<Ide>::make(1);
    
        // We expect to find an ext2 file system there
        auto fs = Shared<Ext2>::make(ide);

        Debug::printf("*** block size is %d\n",fs->get_block_size());
        Debug::printf("*** inode size is %d\n",fs->get_inode_size());
     
        // dummy
        dummy = Shared<File_Node>::make();
        dummy->file_name = new char[1];
        dummy->file_name[0] = '\0';

        Debug::printf("Dummy on contructor\n");

        // First Song 
        Shared<File_Node> first = Shared<File_Node>::make();
        const char * first_name = "swift";
        first->file_name = new char[K::strlen(first_name) + 1];
        memcpy(first->file_name, first_name, K::strlen(first_name));
        first->file_name[K::strlen(first_name)] = '\0';
        setWaveFile(first, "swift_", fs); 
        setSmallFile(first, "swift_s", fs);
        setBigFile(first, "swift" , fs);

         Debug::printf("F on contructor\n");
        

        // Second Song
        Shared<File_Node> second = Shared<File_Node>::make();
        const char * second_name = "travis";
        second->file_name = new char[K::strlen(second_name) + 1];
        memcpy(second->file_name, second_name, K::strlen(second_name));
        second->file_name[K::strlen(second_name)] = '\0';
        setWaveFile(second, "travis_", fs); 
        setSmallFile(second, "travis_s", fs);
        setBigFile(second, "travis" , fs);

         Debug::printf("S on contructor\n");

        // Third Song
        Shared<File_Node> third = Shared<File_Node>::make();
        const char * third_name = "red";
        third->file_name = new char[K::strlen(third_name) + 1];
        memcpy(third->file_name, third_name, K::strlen(third_name));
        third->file_name[K::strlen(third_name)] = '\0';
        setWaveFile(third, "red_", fs); 
        setSmallFile(third, "red_s", fs);
        setBigFile(third, "red" , fs);

         Debug::printf("T on contructor\n");

        // Fourth Song
        Shared<File_Node> fourth = Shared<File_Node>::make();
        const char * fourth_name = "dark side of the moon";
        fourth->file_name = new char[K::strlen(fourth_name) + 1];
        memcpy(fourth->file_name, fourth_name, K::strlen(fourth_name));
        fourth->file_name[K::strlen(fourth_name)] = '\0';
        setWaveFile(fourth, "dark side of the moon_", fs); 
        setSmallFile(fourth, "dark side of the moon_s", fs);
        setBigFile(fourth, "dark side of the moon" , fs);

        Debug::printf("F on contructor\n");

        // Setup Dummy:
        setPrev(dummy, fourth);
        setNext(dummy, first);


        // Setup First: 
        setPrev(first, dummy);
        setNext(first, second);
        
        // Setup Second
        setPrev(second, first);
        setNext(second, third);


        // Setup third:
        setPrev(third, second);
        setNext(third, fourth);


        // Setup fourth: 
        setPrev(fourth, third);
        setNext(fourth, dummy);

        printList(dummy);

        Debug::printf("End on contructor\n");
    }

    void setSmallFile(Shared<File_Node> current, const char* name, Shared<Ext2> fs) {
        current->small = fs->find(fs->root,name); 
    }

    void setBigFile(Shared<File_Node> current, const char* name, Shared<Ext2> fs) {
        current->big = fs->find(fs->root,name); 
    }

    void setWaveFile(Shared<File_Node> current, const char* name, Shared<Ext2> fs) {

        auto waveFile = fs->find(fs->root,name);
        Shared<WaveParser_list> returned_wave_file = Shared<WaveParser_list>::make(waveFile);
        current->wave_file = returned_wave_file; 

    }

    void setPrev(Shared<File_Node> current, Shared<File_Node> prev) {
        current->prev = prev; 
    }

    void setNext(Shared<File_Node> current, Shared<File_Node> next) {
        current->next = next; 
    }

    void printList(Shared<File_Node> current) {
        Shared<File_Node> temp = current; 
        temp = temp->next;
        while(temp != current) {
            Debug::printf("Node Name: %s\n", temp->file_name);
            temp = temp->next;
        }
    }

    Shared<File_Node> findName(const char * name, Shared<File_Node> current) {
        Debug::printf("Finding Name: %s\n", name);
        Shared<File_Node> temp = current; 
        while(temp->next != current) {
            if(K::streq(name, temp->file_name)) {
                Debug::printf("YAY found it: %s\n", name);
                return temp; 
            }
            temp = temp->next; 
        }

        Debug::printf("Did Not Find Name\n");
        return dummy;
    }

    void resetFileSettings(Shared<File_Node> to_reset) {
        to_reset->wave_file->offset = 0;
    }

};