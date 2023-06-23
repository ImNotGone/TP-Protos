# compilation flags
include ../Makefile.inc

SOURCES=$(wildcard src/*.c)
OBJECTS=$(SOURCES:.c=.o)

all: $(OBJECTS)

%.o: %.c
	# $< first prerequisite
	# $@ target
	$(CC) $(CC_FLAGS) -I./include -c $< -o $@

clean:
	rm -rf src/*.o
	rm -rf src/states/*.o
	rm -rf $(TESTS_TARGET)

.PHONY: all test clean
