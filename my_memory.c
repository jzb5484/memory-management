#include "my_memory.h"

#include <stdlib.h>

void* (*mallocfunc)(int);
void (*freefunc)(void*);

static void* firstfit(int size);
static void* bestfit(int size);
static void* worstfit(int size);
static void* buddysystem(int size);

static void buddyfree(void* ptr);
static void regfree(void* ptr);

struct hole {
	void* location;
	unsigned int size;
	struct hole* next;
};

// The list of holes we maintain will be a linked list.
static struct hole* head;

void setup(int malloc_type, int mem_size, void* start_of_memory) {
	head = malloc(sizeof(struct hole));
	head->location = (void*) start_of_memory;
	head->size = mem_size;
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
}


void* my_malloc(int size) {
	return (*mallocfunc)(size);
}

void my_free(void* ptr) {
	(*freefunc)(ptr);
}

static void regfree(void* ptr) {

}
static void buddyfree(void* ptr) {

}

static void* firstfit(int size) {
	return NULL;
}
static void* bestfit(int size) {
	return NULL;
}
static void* worstfit(int size) {
	return NULL;
}
static void* buddysystem(int size) {
	return NULL;
}
