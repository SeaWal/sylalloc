CC = cc
CFLAGS = -std=c11 -Wall -Wextra -Werror -g
SAN = -fsanitize=address,undefined
INC = -Iinclude

SRC = src/sylalloc.c

TEST_BASIC = tests/test_basic.c

all: test_basic

test_basic:
	$(CC) $(CFLAGS) $(SAN) $(INC) $(SRC) $(TEST_BASIC) -o test_basic

clean:
	rm -f test_basic

