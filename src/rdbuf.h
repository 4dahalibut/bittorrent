#ifndef RD_BUF_H
#define RD_BUF_H

#include <stdio.h>
#include <stdint.h>

// A buffer for reading
typedef struct {
    const uint8_t *rbuf;
    size_t total;
    size_t filled;
} RdBuf;

void rb_init( RdBuf *rb, size_t len );

//rbuf.allocate(size_t length) [Allocates room for a given # of bytes]
int rb_allocate( RdBuf *rb, int len );

//rbuf.remaining() [Returns # of bytes allocated but not filled]
size_t rb_remaining( RdBuf *rb );

//rbuf.fill(int sock) [Calls recv() once based on remaining(). Returns # read or error]
int rb_fill( RdBuf *rb, int sock );

//rbuf.consume(size_t length) [Consumes a given amount of filled bytes from the internal buffer]
int rb_consume( RdBuf *rb, size_t length );

//rbuf.get() [Gets a const uint8_t pointer to the internal buffer]
const uint8_t *rb_get( RdBuf *rb );

void rb_free( RdBuf *rb );
#endif
