# compilation flags
include ../Makefile.inc

SOURCES=$(wildcard src/*.c)
TESTS=$(wildcard test/*.c)
TESTS_TARGET=$(TESTS:.c=)
OBJECTS=$(SOURCES:.c=.o)

all: $(OBJECTS)

test: $(TESTS_TARGET)

test/test_parser: src/parser.c src/pop3-parser.c src/logger.c test/test_parser.c
	$(CC) $(CC_FLAGS) -I./include $^ -o $@
	@if ./$@; then \
		echo "\033[32m$@\033[0m"; \
	else \
		echo "\033[31m$@\033[0m"; \
	fi
	# borro los tests para que se corran denuevo
	rm $@

%.o: %.c
	# $< first prerequisite
	# $@ target
	$(CC) $(CC_FLAGS) -I./include -c $< -o $@

clean:
	rm -rf src/*.o
	rm -rf $(TESTS_TARGET)

.PHONY: all test clean
