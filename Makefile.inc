COMP=gcc
STD=-std=c11
COMP_FLAGS=-Wall $(STD) -pedantic
BINARIES=server
LD=gcc
LD_FLAGS=-Wall $(STD) -pedantic -fsanitize=address
