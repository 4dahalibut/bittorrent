#ifndef WR_BUF_C
#define WR_BUF_C

#include "wrbuf.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>

void wb_init( WrBuf *wb, size_t len ) {
    wb->wbuf = ( const uint8_t * ) malloc( len );
    wb->total = len;
    wb->filled = 0;
}

int wb_append( WrBuf *wb, const uint8_t *buffer, size_t length ) {
    if ( length > wb->total - wb->filled ) {
        wb->wbuf = (uint8_t *) realloc( (void *) wb->wbuf, wb->filled + length );
        wb->total = wb->filled + length;
    }
    memcpy( (void *) (wb->wbuf + wb->filled), (void *) buffer, length );
    wb->filled += length;
    return wb->filled;
}

int wb_remaining( WrBuf *wb ) {
    return wb->filled;
}

int wb_consume( WrBuf *wb, int sock ) {
    int ret = send( sock, (void *) wb->wbuf, wb->filled, 0 );

    if ( ret > 0 ) {
        wb->filled -= ret;
        memmove( (void *) wb->wbuf, (void *) ( wb->wbuf + ret ), wb->filled ); 
    } else if ( ret == 0 ) {
        PRINTF("No data written by the WriteBuffer. Huh. \n");
    } else {
        PRINTF("Read error in ReadBuffer (%s). \n", strerror(errno));
    }

    return ret;
}

void wb_free( WrBuf *wb ) {
    free( (void *) wb->wbuf );
}
#endif
