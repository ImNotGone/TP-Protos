# compilation settings
include Makefile.inc

# server
SERVER_BINARY=pop3_server
MONITOR_BINARY=pop3_monitor
SOURCES_SERVER=$(wildcard server/src/*.c server/src/states/*.c)
HEADERS_SERVER=$(wildcard server/include/*.h)
SOURCES_MONITOR=$(wildcard monitor/src/*.c)
HEADERS_MONITOR=$(wildcard monitor/include/*.h)
OBJECTS_SERVER=$(SOURCES_SERVER:.c=.o)
OBJECTS_MONITOR=$(SOURCES_MONITOR:.c=.o)


# binaries
BINARIES=$(SERVER_BINARY) $(MONITOR_BINARY)

# make rules
all: $(BINARIES)

debug: CC_FLAGS+=-g
debug: LD_FLAGS+=-g
debug: all

test:
	cd server; make test

cpp-check:
	cppcheck --quiet --enable=all --force --inconclusive --suppress=missingIncludeSystem -I server/include .

clean:
	rm -rf *.o
	rm -rf $(BINARIES)
	cd server; make clean
	cd monitor; make clean

.PHONY: all debug test cpp-check clean


$(OBJECTS_SERVER): $(SOURCES_SERVER) $(HEADERS_SERVER)
	cd server; make all

$(OBJECTS_MONITOR): $(SOURCES_MONITOR) $(HEADERS_MONITOR)
	cd monitor; make all

$(SERVER_BINARY): $(OBJECTS_SERVER)
	# $^ all prerequisites
	# $@ target
	$(CC) $(LD_FLAGS) $^ -o $@

$(MONITOR_BINARY): $(OBJECTS_MONITOR)
	# $^ all prerequisites
	# $@ target
	$(CC) $(LD_FLAGS) $^ -o $@
