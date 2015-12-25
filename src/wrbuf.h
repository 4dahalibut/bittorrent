#ifndef RD_BUF_H
#define RD_BUF_H

#include <stdio.h>
#include <stdint.h>

// A buffer for reading
typedef struct {
    const uint8_t *wbuf;
    size_t total;
    size_t filled;
} WrBuf;

void wb_init( WrBuf *wb, size_t len );
//wbuf.append(const uint8_t *buffer, size_t length) [Appends buffer to internal buffer]
int wb_append( WrBuf *wb, const uint8_t *buffer, size_t length );
//wbuf.remaining() [Returns # of remaining bytes to be written to the socket]
int wb_remaining( WrBuf *wb );
//wbuf.consume(int sock) [Calls write() once based on remaining(). Returns # written or error]
int wb_consume( WrBuf *wb, int sock );

void wb_free( WrBuf *wb );
#endif
