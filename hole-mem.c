#include <stdint.h>
#include <stdlib.h>

struct hole {
	void* location;
	unsigned int size;
	struct hole* next;
};

// The list of holes we maintain will be a linked list.
static struct hole* head;
static void* mem_start;
static uint64_t mem_finish;

void hole_mem_setup(int malloc_type, uint64_t mem_size, void* start_of_memory) {
	head = malloc(sizeof(struct hole));
	head->location = (void*) start_of_memory;
	head->size = mem_size;
	mem_start = start_of_memory;
	mem_finish = (uint64_t) start_of_memory + mem_size;
}

void* firstfit(int size) {
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
	}
	return (void*) -1;
}
void* bestfit(int size) {
	return (void*) -1;
}
void* worstfit(int size) {
	return (void*) -1;
}

// it is possible that head is NULL.
void regfree(void* ptr) {

}
