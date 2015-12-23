#ifndef WR_BUF_C
#define WR_BUF_C

#include "wrbuf.h"
#include "utils.h"
#include <errno.h>
#include <string.h>
#include <sys/socket.h>

void wb_init( WrBuf *wb, size_t len ) {
    wb->wbuf = ( const uint8_t * ) malloc( len );
    wbuf->total = len;
}

void wb_append( WrBuf *wb, const uint8_t *buffer, size_t length ) {
    if ( length > wb->total - wb->filled ) {
        wb->wbuf = (uint8_t *) realloc( wb->wbuf, wb->filled + length );
    }
    memcpy( wb->wbuf + wb->filled, buffer, length );
}

int wb_remaining( WrBuf *wb ) {
    return wb->filled;
}

int wb_consume( WrBuf *wb, int sock ) {
    int ret = send( sock, wb->wbuf + wb->filled, wb->total - wb->filled, 0 );

    if ( ret > 0 ) {
        memmove( wb->wbuf, wb->wbuf + ret, ret ); 
        wb->filled -= ret;
    } else if ( ret == 0 ) {
        PRINTF("No data written by the WriteBuffer. Huh. \n");
    } else {
        PRINTF("Read error in ReadBuffer (%s). \n", strerror(errno));
    }

    return ret;
}

void wb_free( WrBuf *wb ) {
    free( wb->wbuf );
}
#endif
