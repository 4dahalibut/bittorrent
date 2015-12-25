VPATH=src:tests
CC=clang
CFLAGS=-Wall -std=c99 -Wextra -ggdb -Isrc

all: test_readbuf test_writebuf

test_readbuf: rdbuf.c test_readbuf.c
test_writebuf: wrbuf.c test_writebuf.c

clean:
	rm -rf *~ *.o test_readbuf

.PHONY : clean all
