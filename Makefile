CC = cc
CFLAGS = -std=c11 -Wall -Wextra -Werror -g
DEBUGFLAGS = -DSYL_DEBUG
INC = -Iinclude

SRC = src/sylalloc.c

TEST = tests/test.c

all: test

test:
	$(CC) $(CFLAGS) $(DEBUGFLAGS) $(INC) $(SRC) $(TEST) -o test

clean:
	rm -f test

