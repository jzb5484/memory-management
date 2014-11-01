CFLAGS=-std=c99 -g -Wpointer-to-int-cast
LINKFLAGS=-Wpointer-to-int-cast
OBJFILES=my_memory.o input/test1.o input/test2.o input/test3.o input/test4.o input/test5.o input/test6.o input/test7.o input/test8.o
OUTPUT_FILES=test1_output.txt test2_output.txt test3_output.txt test4_output.txt test5_output.txt test6_output.txt test7_output.txt test8_output.txt 
EXECUTABLES=test1 test2 test3 test4 test5 test6 test7 test8

all: main

main: $(OBJFILES)
	gcc -o test1 my_memory.o input/test1.o $(LINKFLAGS)
	gcc -o test2 my_memory.o input/test2.o $(LINKFLAGS)
	gcc -o test3 my_memory.o input/test3.o $(LINKFLAGS)
	gcc -o test4 my_memory.o input/test4.o $(LINKFLAGS)
	gcc -o test5 my_memory.o input/test5.o $(LINKFLAGS)
	gcc -o test6 my_memory.o input/test6.o $(LINKFLAGS)
	gcc -o test7 my_memory.o input/test7.o $(LINKFLAGS)
	gcc -o test8 my_memory.o input/test8.o $(LINKFLAGS)

clean:
	rm -f $(OBJFILES) $(OUTPUT_FILES) $(EXECUTABLES)

input/%.o: input/%.c input/memalloc.h
	gcc -c $(CFLAGS) -o $@ $<
%.o: %.c
	gcc -c $(CFLAGS) -o $@ $<

