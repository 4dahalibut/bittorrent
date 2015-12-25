#ifndef NETWORK_C_
#define NETWORK_C_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
//nonblocking connect
void my_connect( struct sockaddr_in addr ){
    int fd = my_socket();
    ret = connect(fd,(struct sockaddr *)&addr,sizeof(addr)); 
    if (ret != 0 && errno != EINPROGRESS ) {
        printf("Could not connect to the chat server. (%s)\n",strerror(errno));
        close(fd);
        return -1;
    }
    return fd;
}
//make socket
int my_socket(){
    int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK , 0);
    if (fd < 0){
        perror("socket() failed");
    }
    return fd;
}
//bind
#endif
