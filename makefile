CFLAGS=-Wno-pointer-to-int-cast -Wformat=0 -std=c99 -g -Iinput
LINKFLAGS=-Wno-pointer-to-int-cast -lm -Iinput
OBJFILES=my_memory.o hole-mem.o
TESTFILES=input/test1.o input/test2.o input/test3.o input/test4.o input/test5.o input/test6.o input/test7.o input/test8.o main.o
OUTPUT_FILES=test1_output.txt test2_output.txt test3_output.txt test4_output.txt test5_output.txt test6_output.txt test7_output.txt test8_output.txt test9_output.txt 
EXECUTABLES=test1 test2 test3 test4 test5 test6 test7 test8

all: main

main: $(OBJFILES) main.o
	gcc -o main main.o $(OBJFILES) $(LINKFLAGS)

#test: $(OBJFILES) $(TESTFILES)
#	gcc -o test1 $(OBJFILES) input/test1.o $(LINKFLAGS)
#	gcc -o test2 $(OBJFILES) input/test2.o $(LINKFLAGS)
#	gcc -o test3 $(OBJFILES) input/test3.o $(LINKFLAGS)
#	gcc -o test4 $(OBJFILES) input/test4.o $(LINKFLAGS)
#	gcc -o test5 $(OBJFILES) input/test5.o $(LINKFLAGS)
#	gcc -o test6 $(OBJFILES) input/test6.o $(LINKFLAGS)
#	gcc -o test7 $(OBJFILES) input/test7.o $(LINKFLAGS)
#	gcc -o test8 $(OBJFILES) input/test8.o $(LINKFLAGS)
#	./test1; diff test1_output.txt output/test1_output.txt || echo '\033[1;31mFailed on test 1 \033[00m'
#	./test2; diff test2_output.txt output/test2_output.txt || echo '\033[1;31mFailed on test 2 \033[00m'
#	./test3; diff test3_output.txt output/test3_output.txt || echo '\033[1;31mFailed on test 3 \033[00m'
#	./test4; diff test4_output.txt output/test4_output.txt || echo '\033[1;31mFailed on test 4 \033[00m'
#	./test5; diff test5_output.txt output/test5_output.txt || echo '\033[1;31mFailed on test 5 \033[00m'
#	./test6; diff test6_output.txt output/test6_output.txt || echo '\033[1;31mFailed on test 6 \033[00m'
#	./test7; diff test7_output.txt output/test7_output.txt || echo '\033[1;31mFailed on test 7 \033[00m'
#	./test8; diff test8_output.txt output/test8_output.txt || echo '\033[1;31mFailed on test 8 \033[00m'

test: main
	./main <test_cases/test1.txt >test1_output.txt; diff test1_output.txt output/test1_output.txt || echo '\033[1;31mFailed on test 1 \033[00m'
	./main <test_cases/test2.txt >test2_output.txt; diff test2_output.txt output/test2_output.txt || echo '\033[1;31mFailed on test 2 \033[00m'
	./main <test_cases/test3.txt >test3_output.txt; diff test3_output.txt output/test3_output.txt || echo '\033[1;31mFailed on test 3 \033[00m'
	./main <test_cases/test4.txt >test4_output.txt; diff test4_output.txt output/test4_output.txt || echo '\033[1;31mFailed on test 4 \033[00m'
	./main <test_cases/test5.txt >test5_output.txt; diff test5_output.txt output/test5_output.txt || echo '\033[1;31mFailed on test 5 \033[00m'
	./main <test_cases/test6.txt >test6_output.txt; diff test6_output.txt output/test6_output.txt || echo '\033[1;31mFailed on test 6 \033[00m'
	./main <test_cases/test7.txt >test7_output.txt; diff test7_output.txt output/test7_output.txt || echo '\033[1;31mFailed on test 7 \033[00m'
	./main <test_cases/test8.txt >test8_output.txt; diff test8_output.txt output/test8_output.txt || echo '\033[1;31mFailed on test 8 \033[00m'
	./main <test_cases/test9.txt >test9_output.txt; diff test9_output.txt output/test9_output.txt || echo '\033[1;31mFailed on test 9 \033[00m'

main2: $(OBJFILES) $(TESTFILES)

clean:
	rm -f $(OBJFILES) $(OUTPUT_FILES) $(EXECUTABLES) $(TESTFILES)

input/%.o: input/%.c input/memalloc.h
	gcc -c $(CFLAGS) -o $@ $<
%.o: %.c
	gcc -c $(CFLAGS) -o $@ $<

