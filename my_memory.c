#include <stdint.h>
#include <stdlib.h>

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

}


static void* buddysystem(int size) {
	return NULL;
}
