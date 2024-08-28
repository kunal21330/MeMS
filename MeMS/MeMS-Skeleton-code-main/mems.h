/*
All the main functions with respect to the MeMS are inplemented here
read the function discription for more details

NOTE: DO NOT CHANGE THE NAME OR SIGNATURE OF FUNCTIONS ALREADY PROVIDED
you are only allowed to implement the functions 
you can also make additional helper functions a you wish

REFER DOCUMENTATION FOR MORE DETAILS ON FUNSTIONS AND THEIR FUNCTIONALITY
*/
// add other headers as required
#include<stdio.h>
#include<stdlib.h>
#include<sys/mman.h>
#include<unistd.h>



/*
Use this macro where ever you need PAGE_SIZE.
As PAGESIZE can differ system to system we should have flexibility to modify this 
macro to make the output of all system same and conduct a fair evaluation. 
*/
#define PAGE_SIZE 4096

typedef enum { PROCESS, HOLE } SegmentType;

/*
Initializes all the required parameters for the MeMS system. The main parameters to be initialized are:
1. the head of the free list i.e. the pointer that points to the head of the free list
2. the starting MeMS virtual address from which the heap in our MeMS virtual address space will start.
3. any other global variable that you want for the MeMS implementation can be initialized here.
Input Parameter: Nothing
Returns: Nothing
*/
typedef struct Segment {
    SegmentType type;
    size_t size;
    size_t offset;
    struct Segment *next;
    struct Segment *prev;
} Segment;

typedef struct Node {
    void *physical_address;
    struct Node *next;
    struct Node *prev;
    Segment *sub_chain;
} Node;

typedef struct {
    Node *head;
    size_t next_virtual_address;
} FreeList;

FreeList free_list;

void mems_init(){
    free_list.head = NULL;
    free_list.next_virtual_address = 1000;
}


/*
This function will be called at the end of the MeMS system and its main job is to unmap the 
allocated memory using the munmap system call.
Input Parameter: Nothing
Returns: Nothing
*/
void mems_finish(){
    Node *current = free_list.head;
    while (current != NULL) {
        munmap(current->physical_address, PAGE_SIZE);
        Node *temp = current;
        current = current->next;
        free(temp);
    }
}

/*
Allocates memory of the specified size by reusing a segment from the free list if 
a sufficiently large segment is available. 

Else, uses the mmap system call to allocate more memory on the heap and updates 
the free list accordingly.

Note that while mapping using mmap do not forget to reuse the unused space from mapping
by adding it to the free list.
Parameter: The size of the memory the user program wants
Returns: MeMS Virtual address (that is created by MeMS)
*/ 
void *mems_malloc(size_t size) {
    if (size == 0) return NULL;

    // Try to find a HOLE in the free_list that can fit the size.
    Node *current = free_list.head;
    while (current) {
        Segment *seg = current->sub_chain;
        while (seg) {
            if (seg->type == HOLE && seg->size >= size) {
                // Split the hole if there's space left after allocation
                if (seg->size > size) {
                    Segment *new_hole = (Segment *)malloc(sizeof(Segment));
                    new_hole->type = HOLE;
                    new_hole->size = seg->size - size;
                    new_hole->offset = seg->offset + size;
                    new_hole->next = seg->next;
                    new_hole->prev = seg;
                    seg->next = new_hole;
                    if (new_hole->next) {
                        new_hole->next->prev = new_hole;
                    }
                }
                seg->type = PROCESS;
                seg->size = size;
                return (void *)seg->offset;
            }
            seg = seg->next;
        }
        current = current->next;
    }

    // If we couldn't find a fitting HOLE, allocate new memory
    size_t virtual_address = free_list.next_virtual_address;
    free_list.next_virtual_address += PAGE_SIZE;

    // Allocate physical memory
    void *physical_address = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (physical_address == MAP_FAILED) return NULL;

    // Create a node and a segment
    Node *new_node = (Node *)malloc(sizeof(Node));
    Segment *new_segment = (Segment *)malloc(sizeof(Segment));

    new_node->physical_address = physical_address;
    new_node->sub_chain = new_segment;
    new_node->next = NULL; // By default, point to NULL, adjust below
    new_node->prev = NULL;

    new_segment->type = PROCESS;
    new_segment->size = size;
    new_segment->offset = virtual_address;
    new_segment->next = NULL;
    new_segment->prev = NULL;

    // If there's space left in the page, create a HOLE
    if (size < PAGE_SIZE) {
        Segment *hole_segment = (Segment *)malloc(sizeof(Segment));
        hole_segment->type = HOLE;
        hole_segment->size = PAGE_SIZE - size;
        hole_segment->offset = virtual_address + size;
        hole_segment->next = NULL;
        hole_segment->prev = new_segment;
        new_segment->next = hole_segment;
    }

    // Insert node in a sorted order
    if (!free_list.head || free_list.head->sub_chain->offset > virtual_address) {
        new_node->next = free_list.head;
        if (free_list.head) free_list.head->prev = new_node;
        free_list.head = new_node;
    } else {
        Node *current_node = free_list.head;
        while (current_node->next && current_node->next->sub_chain->offset < virtual_address) {
            current_node = current_node->next;
        }
        new_node->next = current_node->next;
        if (current_node->next) current_node->next->prev = new_node;
        current_node->next = new_node;
        new_node->prev = current_node;
    }

    return (void *)virtual_address;
}






/*
this function print the stats of the MeMS system like
1. How many pages are utilised by using the mems_malloc
2. how much memory is unused i.e. the memory that is in freelist and is not used.
3. It also prints details about each node in the main chain and each segment (PROCESS or HOLE) in the sub-chain.
Parameter: Nothing
Returns: Nothing but should print the necessary information on STDOUT
*/
void mems_print_stats() {
    printf("MeMS SYSTEM STATS\n");

    // Define counters
    size_t pages_used = 0;
    size_t spaces_unused = 0;
    size_t main_chain_length = 0;
    size_t sub_chain_length = 0;

    Node *current_node = free_list.head;
    while (current_node) {
        main_chain_length++;
        pages_used++;
        Segment *current_segment = current_node->sub_chain;
        while (current_segment) {
            sub_chain_length++;

            // Counting unused space in holes
            if (current_segment->type == HOLE) {
                spaces_unused += current_segment->size;
            }
            
            current_segment = current_segment->next;
        }
        current_node = current_node->next;
    }

 

    // Print the actual chain structures
    current_node = free_list.head;
    while (current_node) {
        printf("MAIN[%zu-%zu] -> ", current_node->sub_chain->offset, current_node->sub_chain->offset + PAGE_SIZE - 1);
        Segment *current_segment = current_node->sub_chain;
        while (current_segment) {
            printf("P[%zu:%zu] <-> ", current_segment->offset, current_segment->offset + current_segment->size - 1);
            current_segment = current_segment->next;
        }
        printf("NULL\n");
        current_node = current_node->next;
    }

       // Printing the memory stats
    printf("Pages Used: %zu\n", pages_used);
    printf("Spaces Unused: %zu bytes\n", spaces_unused);
    printf("Main Chain Length: %zu\n", main_chain_length);
    printf("Sub Chain Length: %zu\n", sub_chain_length);
}





/*
Returns the MeMS physical address mapped to ptr ( ptr is MeMS virtual address).
Parameter: MeMS Virtual address (that is created by MeMS)
Returns: MeMS physical address mapped to the passed ptr (MeMS virtual address).
*/
void *mems_get(void *v_ptr) {
    size_t virtual_address = (size_t)v_ptr;

    Node *current_node = free_list.head;
    while (current_node != NULL) {
        Segment *current_segment = current_node->sub_chain;
        while (current_segment != NULL) {
            if (current_segment->type == PROCESS && virtual_address >= current_segment->offset && virtual_address < current_segment->offset + current_segment->size) {
                size_t offset = virtual_address - current_segment->offset;
                return (char*)current_node->physical_address + offset;
            }
            current_segment = current_segment->next;
        }
        current_node = current_node->next;
    }
    return NULL;
}

/*
this function free up the memory pointed by our virtual_address and add it to the free list
Parameter: MeMS Virtual address (that is created by MeMS) 
Returns: nothing
*/
void mems_free(void *v_ptr) {
    size_t virtual_address = (size_t)v_ptr;

    Node *current_node = free_list.head;
    while (current_node) {
        Segment *current_segment = current_node->sub_chain;
        while (current_segment) {
            if (current_segment->type == PROCESS && virtual_address == current_segment->offset) {
                current_segment->type = HOLE;
                // Merge with adjacent HOLE segments
                if (current_segment->prev && current_segment->prev->type == HOLE) {
                    current_segment->prev->size += current_segment->size;
                    current_segment->prev->next = current_segment->next;
                    if (current_segment->next) {
                        current_segment->next->prev = current_segment->prev;
                    }
                    Segment *to_free = current_segment;
                    current_segment = current_segment->prev;
                    free(to_free);
                }
                if (current_segment->next && current_segment->next->type == HOLE) {
                    current_segment->size += current_segment->next->size;
                    Segment *to_free = current_segment->next;
                    current_segment->next = current_segment->next->next;
                    if (current_segment->next) {
                        current_segment->next->prev = current_segment;
                    }
                    free(to_free);
                }
                return;
            }
            current_segment = current_segment->next;
        }
        current_node = current_node->next;
    }
}


