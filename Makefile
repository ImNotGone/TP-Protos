# compilation settings
include Makefile.inc

# server
SERVER_BINARY=pop3_server
SOURCES_SERVER=$(wildcard server/src/*.c)
HEADERS_SERVER=$(wildcard server/include/*.h)
OBJECTS_SERVER=$(SOURCES_SERVER:.c=.o)

# binaries
BINARIES=$(SERVER_BINARY)


# make rules
all: $(BINARIES)

debug: COMP_FLAGS+=-g
debug: LD_FLAGS+=-g
debug: all

cpp-check:
	cppcheck --quiet --enable=all --force --inconclusive --suppress=missingIncludeSystem .

clean:
	@rm -rf *.o
	@rm -rf $(BINARIES)
	cd server; make clean

.PHONY: all debug cpp-check clean


$(OBJECTS_SERVER): $(SOURCES_SERVER) $(HEADERS_SERVER)
	cd server; make all

$(SERVER_BINARY): $(OBJECTS_SERVER)
	# $^ all prerequisites
	# $@ target
	$(CC) $(LD_FLAGS) $^ -o $@
