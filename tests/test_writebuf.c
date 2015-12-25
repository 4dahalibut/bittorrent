#include <stdio.h>
#include <unistd.h> 
#include <sys/types.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <assert.h>
#include "minunit.h"
#include "wrbuf.h"

int tests_run = 0;
 
WrBuf *wb;
 
int initialize_stub(){
    int sock[2];
    int i;
    uint8_t buf[10];
    socketpair(AF_UNIX, SOCK_STREAM, 0, sock);

    if (fork()) {
        close(sock[0]);
        i = wb_consume(wb,sock[1]);
        return i;
    }
    close(sock[1]);
    i = recv(sock[0], buf, 3,0);
    assert(memcmp((void *)buf,(void *)"Foo",3) == 0);
    close(sock[0]);
    _exit(0);
}

static char * test_filled() {
    mu_assert("error, improper amt remaining", wb_remaining(wb) == 0);
    return 0;
}
 
static char * test_write() {
    wb_append(wb,(uint8_t *)"Foo",3);
    int i = initialize_stub();
    mu_assert("consumed wrong amt", i  == 3);
    mu_assert("error, improper amt remaining 2", wb_remaining(wb) == 0);
    return 0;
}
 
static char * all_tests() {
    wb = &(WrBuf){};
    wb_init(wb,5);
    mu_run_test(test_filled);
    mu_run_test(test_write);
    wb_free(wb);
    return 0;
}
 
int main() {
     char *result = all_tests();
     if (result != 0) {
         printf("%s\n", result);
     }
     else {
         printf("ALL TESTS PASSED\n");
     }
     printf("Tests run: %d\n", tests_run);
 
     return result != 0;
}
