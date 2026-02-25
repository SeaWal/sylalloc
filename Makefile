CC = cc
CFLAGS = -std=c11 -Wall -Wextra -Werror -g
INC = -Iinclude

SRC = src/sylalloc.c

TEST = tests/test.c

all: test

test:
	$(CC) $(CFLAGS) $(SAN) $(INC) $(SRC) $(TEST) -o test

clean:
	rm -f test

