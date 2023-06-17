# compilation flags
include ../Makefile.inc

SOURCES=$(wildcard src/*.c src/states/*.c)
TESTS=$(wildcard test/*.c)
TESTS_TARGET=$(TESTS:.c=)
OBJECTS=$(SOURCES:.c=.o)

all: $(OBJECTS)

test: $(TESTS_TARGET)

test/test-parser: src/parser.c src/pop3-parser.c src/logger.c test/test-parser.c
	$(CC) $(CC_FLAGS) -I./include $^ -o $@
	@if ./$@; then \
		echo -e "\033[32m$@\033[0m"; \
	else \
		echo -e "\033[31m$@\033[0m"; \
	fi
	# borro los tests para que se corran denuevo
	rm $@

test/test-message-manager: src/message-manager.c test/test-message-manager.c
	$(CC) $(CC_FLAGS) -I./include $^ -o $@
	@if ./$@; then \
		echo -e "\033[32m$@\033[0m"; \
	else \
		echo -e "\033[31m$@\033[0m"; \
	fi
	# borro los tests para que se corran denuevo
	rm $@

test/test-user-manager: src/user-manager.c test/test-user-manager.c
	$(CC) $(CC_FLAGS) -I./include $^ -o $@
	@if ./$@; then \
		echo -e "\033[32m$@\033[0m"; \
	else \
		echo -e "\033[31m$@\033[0m"; \
	fi
	# borro los tests para que se corran denuevo
	rm $@

%.o: %.c
	# $< first prerequisite
	# $@ target
	$(CC) $(CC_FLAGS) -I./include -c $< -o $@

clean:
	rm -rf src/*.o
	rm -rf src/states/*.o
	rm -rf $(TESTS_TARGET)

.PHONY: all test clean
