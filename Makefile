CC = cc
CFLAGS = -std=c11 -Wall -Wextra -Werror -g
DEBUGFLAGS = -DSYL_DEBUG -DSYL_TRACE
INC = -Iinclude

SRC = src/sylalloc.c src/syldebug.c src/logger.c

TEST = tests/test.c

DEBUG = build/debug.c

all: test

test:
	$(CC) $(CFLAGS) $(INC) $(SRC) $(TEST) -o test

debug:
	$(CC) $(CFLAGS) $(DEBUGFLAGS) $(INC) $(SRC) $(DEBUG) -o build/debug

clean:
	rm -f test build/DEBUG