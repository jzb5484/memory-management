#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define LEFT 0
#define RIGHT 1

void setup(int malloc_type, int mem_size, void* start_of_memory);
void *my_malloc(int size);
void my_free(void *ptr);

void* (*mallocfunc)(int);
void (*freefunc)(void*);

extern void hole_mem_setup(int malloc_type, uint64_t mem_size, void* start_of_memory);

extern void* firstfit(int size);
extern void* bestfit(int size);
extern void* worstfit(int size);
static void* buddysystem(int size);

static void buddyfree(void* ptr);
extern void regfree(void* ptr);

struct hole {
	void* location;
	unsigned int size;
	struct hole* next;
};

struct buddy_header
{
    uint16_t size;
    uint8_t allocated;
    uint8_t orientation;
};

void* MEM;
int MEM_SIZE;

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

static void buddyfree(void* ptr) {
    ptr -= sizeof(struct buddy_header);

	if (ptr < MEM || ptr >= MEM + (MEM_SIZE << 10))
		return;

    int size = get_block_size(ptr);
    int allocated = is_allocated(ptr);
    int orientation = get_block_orientation(ptr);

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
        write_header(buddy, (size*2) >> 10, 0, LEFT);
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
            return -1;
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
    int nearest_two = 1;

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

