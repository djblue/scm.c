BIN=scm
CC=gcc
CFLAGS=-std=gnu99 -g -O2 -Wall -pedantic
LDFLAGS=-lreadline

# find all source files
SOURCES=$(shell find | grep '\.c' | egrep -v "lexer.c|parser.c") lexer.c parser.c
OBJS=$(SOURCES:.c=.o)

.PHONY: clean test

all: $(BIN)

makefile.dep: $(SOURCES)
	$(CC) $^ -MG -MM > $@

# build interpreter
$(BIN): $(OBJS)
	$(CC) $(OBJS) -o $(BIN) $(LDFLAGS)

%.xxd: %.scm
	xxd -i $< | sed 's/\([0-9a-f]\)$$/\0, 0x00/' > $@

# build object files from sources
%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

lexer.h lexer.c: lexer.l
	flex --header-file=lexer.h --outfile=lexer.c $<

parser.h parser.c: parser.y
	bison --output-file=parser.c --defines=parser.h --warnings=all --feature=all $<

# clean up
clean:
	rm -rf $(BIN) lexer.h lexer.c parser.h parser.c *.o *.xxd tags makefile.dep

# code navigation tags
tags: $(SOURCES)
	ctags -R

# build and run tests
test: $(BIN)
	$(MAKE) test -C ../

ifneq ($(MAKECMDGOALS),clean)
-include makefile.dep
endif
