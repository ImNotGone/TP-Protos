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

.PHONY: all clean
