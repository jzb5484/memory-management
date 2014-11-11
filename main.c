#include "memalloc.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

// alphabetical allocations.
#define MAX_ALLOCATIONS 26
#define SIZE 1024
#define STDIN 0
#define STDOUT 1
#define STDERR 2

enum output_mode {
	byte,
	kilobyte
};

int main() {
	size_t len = 0;
	char buffer[SIZE + 1], *buf;
	char *end_of_line;
	void *start_of_memory;
	void *loc[MAX_ALLOCATIONS];
	int i = 0;
	enum output_mode mode = byte;
	memset(loc, 0, sizeof(void*) * MAX_ALLOCATIONS);

	buffer[SIZE] = 0;
	fcntl(STDIN, F_SETFL, O_NONBLOCK);

	do {
		len += read(STDIN, &buffer[len], SIZE - len);
		buf = buffer;

		while (1) {
			end_of_line = strchr(buf, '\n');
			if (end_of_line)
				*end_of_line = 0;
			if (end_of_line != NULL) {
				if (!memcmp("init", buf, 4)) {

					// init the system with some number of bytes.
					int mem_size, malloc_type;
					sscanf(&buf[4], "%d %d", &malloc_type, &mem_size);
					start_of_memory = malloc((size_t) mem_size);
					setup(malloc_type, mem_size, start_of_memory);
					if (malloc_type == 3)
						mode = kilobyte;

				} else if (!memcmp("alloc", buf, 5)) {

					// allocate a certain size.
					int size;
					sscanf(&buf[5], "%d", &size);
//					printf("size: %d; i: %d; loc: %p\n", size, i, loc);
					loc[i] = my_malloc(size);
					if ((int)(loc[i]) == -1) {
						printf("This size can not be allocated!\n\n");
					} else {
						if (mode == byte) {
							printf("start of the chunk %c: %d\n", 97+i, (int)(loc[i]-start_of_memory));
							printf("End of the chunk %c: %d\n\n", 97+i, (int)(loc[i]+size-start_of_memory));
						} else {
							printf("start of the chunk %c: %f K\n", 97+i, (float)(loc[i]-start_of_memory)/1024.0f);
							printf("End of the chunk %c: %f K\n\n", 97+i, (float)(loc[i]+size-start_of_memory)/1024.0f);
						}
					}
					i++;

				} else if (!memcmp("free", buf, 4)) {

					// Free loc[index].
					int index;
					sscanf(&buf[4], "%d", &index);
					index--;
					if (loc[index] != NULL) {
						my_free(loc[index]);
					}
					loc[index] = NULL;

				} else if (!memcmp("term", buf, 4)) {
					return 0;
				}
				len -= (int) (end_of_line - buf) + 1;
				buf = end_of_line + 1;
			} else {
				if (buf == buffer && len == SIZE) {
					fprintf(stderr, "cannot handle lines longer than %d\n", SIZE);
					exit(-1);
				} else {
					memmove(buffer, buf, len);
					break;
				}
			}
		}
	} while (!feof(stdin));

	return 0;
}