#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

#define LEFT 0
#define RIGHT 1

void setup(int malloc_type, int mem_size, void* start_of_memory);
void *my_malloc(int size);
void my_free(void *ptr);

void* (*mallocfunc)(int);
void (*freefunc)(void*);

static void* firstfit(int size);
static void* bestfit(int size);
static void* worstfit(int size);
static void* buddysystem(int size);

static void buddyfree(void* ptr);
static void regfree(void* ptr);

struct buddy_header
{
    uint16_t size;
    uint8_t allocated;
    uint8_t orientation;
};

struct hole {
    void* location;
    unsigned int size;
    struct hole* next;
};

// The list of holes we maintain will be a linked list.
static struct hole* head;
static void* mem_start;
static uint64_t mem_finish;

void* MEM;
int MEM_SIZE;

// Auxiliary functions for first-fit, best-fit, worst-fit.
static void add_hole(void* location, uint32_t size);
static void hole_mem_setup(int malloc_type, uint64_t mem_size, void* start_of_memory);

// Auxiliary functions for buddy system
int round_nearest_power_two(int x);
int get_block_size(void *header);
int is_allocated(void *header);
int get_block_orientation(void *header);
void *split_block(void *header);
void write_header(void *location, int size, int allocated, int orientation);
void clear_header(void *location);


void setup(int malloc_type, int mem_size, void* start_of_memory) {
	switch(malloc_type) {
		case 0:
		// first fit.
    		mallocfunc = firstfit;
    		freefunc = regfree;
		break;
		case 1:
		// best fit.
    		mallocfunc = bestfit;
    		freefunc = regfree;
		break;
		case 2:
    		// worst fit.
    		mallocfunc = worstfit;
    		freefunc = regfree;
		break;
		case 3:
    		// buddy system.
    		mallocfunc = buddysystem;
    		freefunc = buddyfree;

    		MEM = start_of_memory;
    		MEM_SIZE = mem_size >> 10;

    		write_header(MEM, MEM_SIZE, 0, LEFT);

		break;
	}
	if (0 <= malloc_type && malloc_type <= 2)
		hole_mem_setup(malloc_type, (uint64_t) mem_size, start_of_memory);
}


void* my_malloc(int size) {
	return (*mallocfunc)(size);
}

void my_free(void* ptr) {
	(*freefunc)(ptr);
}

///////////////////////////////////////////////
// First fit, best fit, worst fit functions. //
///////////////////////////////////////////////

static void add_hole(void* location, uint32_t size) {
    struct hole *trace = head, *trail = NULL;
    while (trace != NULL) {
        if (location <= trace->location && (trail == NULL || location >= trail->location)) {
            if ((trail != NULL) && (trail->location + trail->size == location) && (location + size == trace->location)) {
                // merge the two holes.
                trail->next = trace->next;
                trail->size += size + trace->size;
                free(trace);
            } else if ((trail != NULL) && (trail->location + trail->size == location)) {
                // merge the trailing hole with this hole.
                trail->size += size;
            } else if (location + size == trace->location) {
                // merge the next hole with this hole.
                trace->location -= size;
                trace->size += size;
            } else {
                // create new hole.
                struct hole* node = malloc(sizeof(struct hole));
                node->location = location;
                node->size = size;
                if (trail == NULL)
                    head = node;
                else
                    trail->next = node;
                node->next = trace;
            }
            return;
        }
        trail = trace;
        trace = trace->next;
    }
    // the hole should be added to the end of the list.
    if (trail == NULL) {
        // The list is empty.
        trace = malloc(sizeof(struct hole));
        trace->location = location;
        trace->size = size;
        trace->next = NULL;
        head = trace;
    } else if (trail->location + trail->size == location) {
        // merge with the final node.
        trail->size += size;
    } else {
        // create new node.
        struct hole* node = malloc(sizeof(struct hole));
        node->location = location;
        node->size = size;
        trail->next = node;
        node->next = NULL;
    }
}
static void hole_mem_setup(int malloc_type, uint64_t mem_size, void* start_of_memory) {
    head = malloc(sizeof(struct hole));
    head->location = (void*) start_of_memory;
    head->size = mem_size;
    mem_start = start_of_memory;
    mem_finish = (uint64_t) start_of_memory + mem_size;
}
static void* firstfit(int size) {
    size += 4; // we need 4 bytes to hold the header information.
    struct hole* trace = head, *trail = NULL;
    while (trace != NULL) {
        if (trace->size == size) {
            // Get the address of the hole 
            void* addr = trace->location;
            uint32_t length = size;

            // the entire hole is filled.
            if (trail != NULL) {
                trail->next = trace->next;
            } else {
                head = trace->next;
            }

            // free the hole.
            free(trace);

            // write the size of the allocation in the memory region.
            *((uint32_t*)addr) = length;
            return addr + 4;
        } else if (trace->size > size) {
            // Get the address of the hole.
            void* addr = trace->location;
            uint32_t length = size;

            // Shrink the hole.
            trace->size = trace->size - size;
            trace->location = trace->location + size;

            // Write the size of the allocation in the memory region.
            *((uint32_t*) addr) = length;
            return addr + 4;
        }
        trail = trace;
        trace = trace -> next;
    }
    return (void*) -1;
}
static void* bestfit(int size) {
    size += 4; // we need 4 bytes to hold the header information.
    struct hole *trace = head, *trail = NULL;
    struct hole *best = NULL, *besttrail = NULL;
    int bestsize = INT_MAX;
    while (trace != NULL) {
        if (trace->size >= size && bestsize > trace->size) {
            best = trace;
            besttrail = trail;
            bestsize = trace->size;
        }
        trail = trace;
        trace = trace -> next;
    }

    // Allocate wherever the new memory space fits best.
    if (best != NULL) {
        if (best->size == size) {
            // Get the address of the hole 
            void* addr = best->location;
            uint32_t length = size;

            // the entire hole is filled.
            if (besttrail != NULL) {
                besttrail->next = best->next;
            } else {
                head = best->next;
            }

            // free the hole.
            free(best);

            // write the size of the allocation in the memory region.
            *((uint32_t*)addr) = length;
            return addr + 4;
        } else if (best->size > size) {
            // Get the address of the hole.
            void* addr = best->location;
            uint32_t length = size;

            // Shrink the hole.
            best->size = best->size - size;
            best->location = best->location + size;

            // Write the size of the allocation in the memory region.
            *((uint32_t*) addr) = length;
            return addr + 4;
        }
    }

    return (void*) -1;
}
static void* worstfit(int size) {
    size += 4; // we need 4 bytes to hold the header information.
    struct hole *trace = head, *trail = NULL;
    struct hole *best = NULL, *besttrail = NULL;
    int bestsize = 0;
    while (trace != NULL) {
        if (trace->size >= size && trace->size > bestsize) {
            best = trace;
            besttrail = trail;
            bestsize = trace->size;
        }
        trail = trace;
        trace = trace -> next;
    }

    // Allocate wherever the new memory space fits best.
    if (best != NULL) {
        if (best->size == size) {
            // Get the address of the hole 
            void* addr = best->location;
            uint32_t length = size;

            // the entire hole is filled.
            if (besttrail != NULL) {
                besttrail->next = best->next;
            } else {
                head = best->next;
            }

            // free the hole.
            free(best);

            // write the size of the allocation in the memory region.
            *((uint32_t*)addr) = length;
            return addr + 4;
        } else if (best->size > size) {
            // Get the address of the hole.
            void* addr = best->location;
            uint32_t length = size;

            // Shrink the hole.
            best->size = best->size - size;
            best->location = best->location + size;

            // Write the size of the allocation in the memory region.
            *((uint32_t*) addr) = length;
            return addr + 4;
        }
    }

    return (void*) -1;
}
static void regfree(void* ptr) {
    uint32_t size;
    ptr -= 4;
    size = *((uint32_t*) ptr);
    add_hole(ptr, size);
}

////////////////////////////
// Buddy System functions //
////////////////////////////

static void buddyfree(void* ptr) {
    ptr -= sizeof(struct buddy_header);

	if (ptr < MEM || ptr >= MEM + (MEM_SIZE << 10))
		return;

    int size = get_block_size(ptr);
    int allocated = is_allocated(ptr);
    int orientation = get_block_orientation(ptr);

    if (ptr == MEM && size >> 10 == MEM_SIZE) {
        // base case: we do not need to recurse (or even find the buddy).
        write_header(ptr, (size*2)>>10, 0, LEFT);
        return;
    }

    void *buddy;
    if(orientation == LEFT)
    {
        buddy = ptr + size;
    }
    else
    {
        buddy = ptr - size;
    }
    int buddy_size = get_block_size(buddy);
    int buddy_allocated = is_allocated(buddy);
    int buddy_orientation = get_block_orientation(buddy);

    if((orientation == LEFT) && (buddy < MEM + (MEM_SIZE << 10)) && (size == buddy_size) && !buddy_allocated)
    {
        clear_header(buddy);
        write_header(ptr, (size*2) >> 10, 0, LEFT);
		buddyfree(ptr + sizeof(struct buddy_header));
    }
    else if (orientation == RIGHT && (buddy < MEM + (MEM_SIZE << 10)) && (size == buddy_size) && !buddy_allocated)
    {
        clear_header(ptr);
        write_header(buddy, (size*2) >> 10, 0, RIGHT);
		buddyfree(buddy + sizeof(struct buddy_header));
    }
    else
    {
        write_header(ptr, size >> 10, 0, orientation);
    }
}

static void* buddysystem(int size) {
	size += sizeof(struct buddy_header);
    size = round_nearest_power_two(size);

    void* new_block = MEM;

    while(get_block_size(new_block) < size || is_allocated(new_block))
    {
        new_block += get_block_size(new_block);

        if(new_block >=  MEM + (MEM_SIZE << 10))
            return (void*) -1;
    }

    while(get_block_size(new_block) > size)
    {
        new_block = split_block(new_block);
    }

    ((struct buddy_header*) new_block)->allocated = 1;

    return new_block + sizeof(struct buddy_header);
}

int round_nearest_power_two(int x)
{
    int nearest_two = 1024;

    while(nearest_two < x)
    {
        nearest_two *= 2;
    }

    return nearest_two;
}

int get_block_size(void* header)
{
    return (((struct buddy_header*) header)->size) << 10;
}

int is_allocated(void* header)
{
    return ((struct buddy_header*) header)->allocated;
}

int get_block_orientation(void* header)
{
    return ((struct buddy_header*) header)->orientation;
}

void *split_block(void* header)
{
    int size = get_block_size(header);

    void* new_location = header + size/2;

    size = size >> 10;

    write_header(header, size/2, 0, LEFT);
    write_header(new_location, size/2, 0, RIGHT);

    return header;
}

void write_header (void *location, int size, int allocated, int orientation)
{
    struct buddy_header header;
    header.size = size;
    header.allocated = allocated;
    header.orientation = orientation;

    memcpy(location, (void *) &(header), sizeof(header));
}

void clear_header (void *location)
{
    memset(location, 0, sizeof(struct buddy_header));
}

