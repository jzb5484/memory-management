#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

#include <stdarg.h>
#include <stdio.h>
#include <math.h>

struct hole {
	void* location;
	unsigned int size;
	struct hole* next;
};

// The list of holes we maintain will be a linked list.
static struct hole* head;
static void* mem_start;
static uint64_t mem_finish;

#define DEBUG_LEVEL 0b11111111
static void debug(int level, const char *fmt, ...) {
	if (!(level & DEBUG_LEVEL))
		return;
	va_list args;
	va_start(args, fmt);

	// Provide color coding.
	fprintf(stderr, "\033[%dm", 31 + (int) log2(level));
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\033[00m");
	va_end(args);
}
#define debug(...)

void add_hole(void* location, uint32_t size) {
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
		trail = trace;
		trace = trace -> next;
	}
	return (void*) -1;
}
void* bestfit(int size) {
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
void* worstfit(int size) {
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

// it is possible that head is NULL.
void regfree(void* ptr) {
	uint32_t size;
	ptr -= 4;
	size = *((uint32_t*) ptr);
	add_hole(ptr, size);
}
