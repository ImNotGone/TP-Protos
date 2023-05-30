include Makefile.inc
all: $(BINARIES)

debug: COMP_FLAGS+=-g
debug: LD_FLAGS+=-g
debug: all

%.o: %.c
	$(COMP) $(COMP_FLAGS) -c $< -o $@

server: server.o
	$(LD) $(LD_FLAGS) $< -o $@

cpp-check:
	cppcheck --quiet --enable=all --force --inconclusive --suppress=missingIncludeSystem .

clean:
	@rm -rf *.o
	@rm -rf $(BINARIES)

.PHONY: all clean debug cpp-check

