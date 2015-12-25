#include <stdio.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include "minunit.h"
#include "rdbuf.h"

int tests_run = 0;
 
RdBuf *rb;
 

int initialize_stub(){
    int sock[2];

    socketpair(AF_UNIX, SOCK_STREAM, 0, sock);

    if (fork()) {
        close(sock[0]);
        return sock[1];
    }
    close(sock[1]);
    write(sock[0], "YOOHOO", 6);
    close(sock[0]);
    _exit(0);
}

static char * test_filled() {
    mu_assert("error, improper amt remaining", rb_remaining(rb) == 6);
    return 0;
}
 
static char * test_allocate() {
    mu_assert("improper amt allocated", rb_allocate(rb,20) == 20);
    mu_assert("improper amt consumed", rb_consume(rb,10) == -1);
    return 0;
}
static char * test_fill() {
    mu_assert("improper amt filled", rb_fill(rb,initialize_stub()) == 6);
    mu_assert("improper amt consumed2", rb_consume(rb,6) == 6);
    return 0;
}
 
static char * all_tests() {
    rb = &(RdBuf){};
    rb_init(rb,6);
    mu_run_test(test_filled);
    mu_run_test(test_allocate);
    mu_run_test(test_fill);
    rb_free(rb);
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
