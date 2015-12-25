#ifndef RD_BUF_C
#define RD_BUF_C

#include "rdbuf.h"
#include "utils.h"
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>

void rb_init( RdBuf *rb, size_t len ) {
    rb->rbuf = ( const uint8_t * ) malloc( len );    
    rb->total = len;
    rb->filled = 0;
}

// is currently len is absolute length
int rb_allocate( RdBuf *rb, int len ) {
    rb->rbuf = (uint8_t *) realloc( ( void * ) rb->rbuf, len );
    rb->total = len;
    return rb->total;
}

size_t rb_remaining( RdBuf *rb ){
    return rb->total - rb->filled;
}

int rb_fill( RdBuf *rb, int sock ) {
    int ret = recv( sock, ( void * ) ( rb->rbuf + rb->filled ) , rb->total - rb->filled, 0 );

    if ( ret > 0 ) {
        rb->filled += ret;
    } else if ( ret == 0 ) {
        PRINTF("Connection being read by ReadBuffer has been closed\n");
    } else {
        PRINTF("Read error in ReadBuffer (%s). \n", strerror(errno));
    }

    return ret;
}

int rb_consume( RdBuf *rb, size_t length ) {
    if( rb->filled < length ){
        PRINTF("Invalid ReadBuffer Consume\n");
        return -1;
    }
    memmove( (void *) rb->rbuf, rb->rbuf + length, length ); 
    rb->filled -= length;

    return length;
}

const uint8_t *rb_get( RdBuf *rb ) {
    return rb->rbuf;
}

void rb_free( RdBuf *rb ){
    free( (void *) rb->rbuf );
}

#endif
