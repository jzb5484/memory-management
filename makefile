CFLAGS=-Wno-pointer-to-int-cast -Wformat=0 -g
LINKFLAGS=
OBJFILES=my_memory.o main.o
OUTPUT_FILES=test1_output.txt test2_output.txt test3_output.txt test4_output.txt test5_output.txt test6_output.txt test7_output.txt test8_output.txt test9_output.txt test10_output.txt
EXECUTABLES=main

all: $(EXECUTABLES)

main: $(OBJFILES) main.o
	gcc -o main $(OBJFILES) $(LINKFLAGS)

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
	./main <test_cases/test10.txt >test10_output.txt; diff test10_output.txt output/test10_output.txt || echo '\033[1;31mFailed on test 10 \033[00m'

clean:
	rm -f $(OBJFILES) $(OUTPUT_FILES) $(EXECUTABLES)

%.o: %.c memalloc.h
	gcc -c $(CFLAGS) -o $@ $<

package:
	tar -cvzf burlew-hayes-minimal.tgz proj-description.txt my_memory.c memalloc.h
	tar -cvzf burlew-hayes-complete.tgz test_cases/* output/* main.c makefile memalloc.h my_memory.c proj-description.txt
