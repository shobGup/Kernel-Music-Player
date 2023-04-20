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
        const char * first_name = "breathe in the air";
        first->file_name = new char[K::strlen(first_name) + 1];
        memcpy(first->file_name, first_name, K::strlen(first_name));
        first->file_name[K::strlen(first_name)] = '\0';
        setWaveFile(first, "breathe in the air_", fs); 
        setSmallFile(first, "breathe in the air_s", fs);
        setBigFile(first, "breathe in the air" , fs);

         Debug::printf("First on contructor\n");
        

        // Second Song
        Shared<File_Node> second = Shared<File_Node>::make();
        const char * second_name = "new romantics";
        second->file_name = new char[K::strlen(second_name) + 1];
        memcpy(second->file_name, second_name, K::strlen(second_name));
        second->file_name[K::strlen(second_name)] = '\0';
        setWaveFile(second, "new romantics_", fs); 
        setSmallFile(second, "new romantics_s", fs);
        setBigFile(second, "new romantics" , fs);

         Debug::printf("Second on contructor\n");

        // Third Song
        Shared<File_Node> third = Shared<File_Node>::make();
        const char * third_name = "feel this moment";
        third->file_name = new char[K::strlen(third_name) + 1];
        memcpy(third->file_name, third_name, K::strlen(third_name));
        third->file_name[K::strlen(third_name)] = '\0';
        setWaveFile(third, "feel this moment_", fs); 
        setSmallFile(third, "feel this moment_s", fs);
        setBigFile(third, "feel this moment" , fs);

         Debug::printf("Third on contructor\n");

        // Fourth Song
        Shared<File_Node> fourth = Shared<File_Node>::make();
        const char * fourth_name = "heart-shaped box";
        fourth->file_name = new char[K::strlen(fourth_name) + 1];
        memcpy(fourth->file_name, fourth_name, K::strlen(fourth_name));
        fourth->file_name[K::strlen(fourth_name)] = '\0';
        setWaveFile(fourth, "heart-shaped box_", fs); 
        setSmallFile(fourth, "heart-shaped box_s", fs);
        setBigFile(fourth, "heart-shaped box" , fs);

        Debug::printf("Fourth on contructor\n");

        // Fifth Song
        Shared<File_Node> fifth = Shared<File_Node>::make();
        const char * fifth_name = "dream on";
        fifth->file_name = new char[K::strlen(fifth_name) + 1];
        memcpy(fifth->file_name, fifth_name, K::strlen(fifth_name));
        fifth->file_name[K::strlen(fifth_name)] = '\0';
        setWaveFile(fifth, "dream on_", fs); 
        setSmallFile(fifth, "dream on_s", fs);
        setBigFile(fifth, "dream on" , fs);

        Debug::printf("Fifth on contructor\n");

        // Sixth Song
        Shared<File_Node> sixth = Shared<File_Node>::make();
        const char * sixth_name = "just the way you are";
        sixth->file_name = new char[K::strlen(sixth_name) + 1];
        memcpy(sixth->file_name, sixth_name, K::strlen(sixth_name));
        sixth->file_name[K::strlen(sixth_name)] = '\0';
        setWaveFile(sixth, "just the way you are_", fs); 
        setSmallFile(sixth, "just the way you are_s", fs);
        setBigFile(sixth, "just the way you are" , fs);

        Debug::printf("Sixth on contructor\n");

        // Seventh Song
        Shared<File_Node> seventh = Shared<File_Node>::make();
        const char * seventh_name = "cant tell me nothing";
        seventh->file_name = new char[K::strlen(seventh_name) + 1];
        memcpy(seventh->file_name,seventh_name, K::strlen(seventh_name));
        seventh->file_name[K::strlen(seventh_name)] = '\0';
        setWaveFile(seventh, "cant tell me nothing_", fs); 
        setSmallFile(seventh, "cant tell me nothing_s", fs);
        setBigFile(seventh, "cant tell me nothing" , fs);

        Debug::printf("Seventh on contructor\n");

        // Eight Song
        Shared<File_Node> eight = Shared<File_Node>::make();
        const char * eight_name = "vamp anthem";
        eight->file_name = new char[K::strlen(eight_name) + 1];
        memcpy(eight->file_name,eight_name, K::strlen(eight_name));
        eight->file_name[K::strlen(eight_name)] = '\0';
        setWaveFile(eight, "vamp anthem_", fs); 
        setSmallFile(eight, "vamp anthem_s", fs);
        setBigFile(eight, "vamp anthem" , fs);

        Debug::printf("Eight on contructor\n");

        // Setup Dummy:
        setPrev(dummy, eight);
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
        setNext(fourth, fifth);

        // Setup fifth: 
        setPrev(fifth, fourth);
        setNext(fifth, sixth);

        // Setup sixth: 
        setPrev(sixth, fifth);
        setNext(sixth, seventh);

        // Setup seventh: 
        setPrev(seventh, sixth);
        setNext(seventh, eight);

        // Setup eight: 
        setPrev(eight, seventh);
        setNext(eight, dummy);

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

        Shared<File_Node> temp = dummy->next; 
        while(temp != dummy) {
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