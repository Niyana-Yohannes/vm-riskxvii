TARGET = vm_riskxvii

CC = gcc

CFLAGS     = -c -O -s -std=c11
SRC        = vm_riskxvii.c LINKED_LIST.c HELPER.c 
OBJ        = $(SRC:.c=.o)

all:$(TARGET)

$(TARGET):$(OBJ)
	$(CC) -o $@ $(OBJ)

.SUFFIXES: .c .o

.c.o:
	 $(CC) $(CFLAGS) $(ASAN_FLAGS) $<

run:
	./$(TARGET)

run_tests:
	./tests/run_tests.sh

test:
	echo what are we testing?!

clean:
	rm -f *.o *.obj $(TARGET)
