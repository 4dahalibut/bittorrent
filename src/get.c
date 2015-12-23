#include "get.h"
#include "utility.hh"
#include "MyBencode.hh"
#include <regex.h>        
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <strings.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <poll.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

namespace cmsc417{

int fd;
static const char *EventStrings[]={"","started","stopped","completed"};
char *ip2str(struct sockaddr_in);

uint8_t table[256] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,45,46,0,48,49,50,51,52,53,54,55,56,57,0,0,0,0,
0,0,0,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,
89,90,0,0,0,0,95,0,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,
113,114,115,116,117,118,119,120,121,122,0,0,0,126,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

void print_hash(uint8_t *hash,int len){

        printf("0x");

        for(int i = 0; i < len; ++i) {
            printf("%02x", hash[i] & 0xFF);
        }

        putchar('\n');
}

struct announcerequest a_req_init(struct TorrentInfo ti,uint16_t port){
    
    struct announcerequest a;
    memset(&a,0,sizeof(announcerequest));
    a.numwant = NUMWANT;
    memcpy(a.info_hash,&ti.info_hash[0],20);
    printf("Announce url: %s\n",ti.tracker.c_str());
    a.tracker_str = (char *)malloc(ti.tracker.length());
    strcpy(a.tracker_str,ti.tracker.c_str());
    a.my_port = port + 1;
    return a;
}
int my_send(uint8_t *buf, int len){

    int total = 0;
    int n;

    while(total < len) {
        n = send(fd,buf + total, len - total, 0);
        if (n == -1) { break; }
        total += n;
    }
    return n == -1 ? -1 : 0;
} 
int simple_http_parse(char *response){

    regex_t regex;
    int reti;
    size_t nmatch = 2;
    regmatch_t pmatch[2];

    reti = regcomp(&regex, "Content-Length: ([0-9]+)", REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        return -1;
    }

    reti = regexec(&regex, response, nmatch, pmatch, 0);
    if (reti) {
        return -2;
    }

    char *result;
    result = (char*)malloc(pmatch[1].rm_eo - pmatch[1].rm_so);
    strncpy(result, &response[pmatch[1].rm_so], pmatch[1].rm_eo - pmatch[1].rm_so);

    regfree(&regex);
    return strtol(result,NULL,10);
}
int parse2(char *response){

    regex_t regex;
    int reti;
    size_t nmatch = 2;
    regmatch_t pmatch[2];

    reti = regcomp(&regex, "\r\n\r\n(.)", REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        return -1;
    }

    reti = regexec(&regex, response,nmatch,pmatch, 0);
    if (reti) {
        return -2;
    }

    regfree(&regex);
    return pmatch[1].rm_so;
}
int my_recv(uint8_t **out){

    uint8_t buf[4096];
    memset(buf,0,4096);
    int len = 0;
    int ret;
    int state = 0;
    int body_len = 4096;
    int tmp = 0;

    for(;;){

        ret = recv(fd,buf+len,body_len-len,0);
        if(ret==0){
            printf("Received %d bytes.\n",len);
            break;
        }else if(ret<0){
            printf("Recv errored: (%s), %d bytes read. \n",strerror(errno),len);
            break;
        }

        len += ret;

        switch (state){
        case 0: 
            ret = simple_http_parse((char *)buf);
            if(ret < 0) break;
            tmp = ret;
            state = 1;
        case 1:
            ret = parse2((char *)buf);
            if(ret < 0) break;
            body_len = ret+tmp;
            state = 2;
        } 
        if(body_len <= len)break;
    }

    if(state == 2){
        *out = (uint8_t *)malloc(tmp);
        memcpy(*out,buf+ret,tmp);
    } else {
        *out = (uint8_t *)malloc(len); 
        memcpy(*out,buf,len);
    }

    return state == 2 ? tmp : len;
}
char *str2hname(char *str){
    regex_t regex;
    int reti;
    size_t nmatch = 2;
    regmatch_t pmatch[2];

    reti = regcomp(&regex, "http://(.*):", REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        return NULL;
    }

    reti = regexec(&regex, str,nmatch,pmatch, 0);
    if (reti) {
        return NULL;
    }

    char *result;
    result = (char*)calloc(pmatch[1].rm_eo - pmatch[1].rm_so,1);
    strncpy(result, &str[pmatch[1].rm_so], pmatch[1].rm_eo - pmatch[1].rm_so);

    regfree(&regex);
    return result;
    
}
char *str2port(char *str){
    regex_t regex;
    int reti;
    size_t nmatch = 2;
    regmatch_t pmatch[2];

    reti = regcomp(&regex, ":([[:digit:]]+)", REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        return NULL;
    }

    reti = regexec(&regex,str,nmatch,pmatch, 0);
    if (reti) {
        return NULL;
    }

    regfree(&regex);
    char *result;
    result = (char*)calloc(pmatch[1].rm_eo - pmatch[1].rm_so,1);
    strncpy(result, &str[pmatch[1].rm_so], pmatch[1].rm_eo - pmatch[1].rm_so);

    regfree(&regex);
    strtol(result,NULL,10);
    if(errno == ERANGE){return NULL;}
    return result;
    
}
int host2addr(struct announcerequest *a){
    int on = 1;
    int sockfd;  
    struct addrinfo hints;
    struct addrinfo *servinfo;
    struct addrinfo *p;
    int rv;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
    hints.ai_socktype = SOCK_STREAM;
    char *hostname = str2hname(a->tracker_str);  
    if(hostname == NULL){ return -1;}
    char *port = str2port(a->tracker_str);
    if(port == NULL){return -1;} 
    
    if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("socket");
            continue;
        }
        int ret = setsockopt(sockfd, SOL_SOCKET,  SO_REUSEADDR,(char *)&on, sizeof(on));
        if (ret < 0){
            perror("setsockopt() failed");
            close(fd);
            return -1;
        }
    
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("connect");
            continue;
        }
    
        break;
    }
    
    if (p == NULL) {
        fprintf(stderr, "failed to connect\n");
        exit(2);
    }
    memcpy(&(a->tracker),p->ai_addr,sizeof(struct sockaddr_in));
    ip2str(a->tracker);
    
    freeaddrinfo(servinfo);
    return 0;
}

int my_connect(struct announcerequest a){

    int on = 1;
    struct sockaddr_in my_addr;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0){
        perror("socket() failed");
        return -1;
    }

    struct timeval tv;

    tv.tv_sec = 3;  /* 30 Secs Timeout */
    tv.tv_usec = 0;  // Not init'ing this can cause strange errors

    int ret = setsockopt(fd, SOL_SOCKET,  SO_REUSEADDR,(char *)&on, sizeof(on));
    if (ret < 0){
        perror("setsockopt() failed");
        close(fd);
        return -1;
    }
    ret = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
    if (ret < 0){
        perror("setsockopt() failed");
        close(fd);
        return -1;
    }

    memset(&my_addr, 0, sizeof(struct sockaddr_in));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = a.my_port;
    my_addr.sin_addr.s_addr = INADDR_ANY;

    ret = bind(fd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr));
    if (ret < 0){
        printf("Bind failed: (%s)\n",strerror(errno));
        close(fd);
        return -1;
    }
    
    ret = connect(fd,(struct sockaddr *)&a.tracker,sizeof(a.tracker)); 
    if (ret < 0){
        printf("Could not connect to the tracker. (%s)\n",strerror(errno));
        close(fd);
        return -1;
    }
    return 0;
}
char *hash2str(uint8_t *s,int len){
    char enc[1000], *tmp = enc;

    for (int i = 0;i < len; i++){
        if (table[s[i]]) {
            sprintf(tmp, "%c", table[s[i]]);
            tmp+=1;
        } else {
            sprintf(tmp, "%%%02x", s[i]);
            tmp+=3;
        }
    }

    char *out = (char *)malloc(strlen(enc));
    strcpy(out,enc);

    return out;
}
uint8_t *store_str(const char *str){

    const char *tmp = str;
    uint8_t *val = (uint8_t *)malloc(20);

    for(int count = 0; count < 20; count++) {
        sscanf(tmp, "%2hhx", &val[count]);
        tmp += 2;
    }

    return val;
}
char *ip2str(struct sockaddr_in addr){

    char enc[100];
    char things[100];

    inet_ntop(AF_INET,&(addr.sin_addr),things,sizeof(addr));
    sprintf(enc,"%s:%d",things,htons(addr.sin_port));
    char *out = (char *)malloc(strlen(enc));
    strcpy(out,enc);
    return out; 
}
int send_announce(struct announcerequest a){

    char buffer[500]; 

    sprintf(buffer,"GET /announce?info_hash=%s&peer_id=%s&port=%d&uploaded=%llu\
&downloaded=%llu&left=%llu&numwant=%lu&compact=1&event=%s HTTP/1.1\r\n\
User-Agent: Best Client/0.0\r\nHost: %s\r\nAccept: */*\r\nAccept-Encoding: \
\r\n\r\n",hash2str(a.info_hash,20),hash2str(a.peer_id,20),a.my_port,a.uploaded,
    a.downloaded,a.left,a.numwant,EventStrings[a.event],ip2str(a.tracker));

    printf("Sending out %s\n",buffer);
    return my_send((uint8_t *)buffer,strlen(buffer));
}
/*CURRENTLY UNUSED*/
void send_scrape(struct announcerequest a){
    char buffer[500]; 

    sprintf(buffer,"GET /scrape?info_hash=%s HTTP/1.1\r\nUser-Agent: Best Client/0.0\r\n\
Host: %s\r\nAccept: */*\r\n\r\n",hash2str(a.info_hash,20),ip2str(a.tracker));

    printf("%.*s\n",(int)strlen(buffer),buffer);
    my_send((uint8_t *)buffer,strlen(buffer));
}

/*Currently only public method*/
int t_announce(struct announcerequest a, struct announceresponse **res,
               unsigned long long downloaded, unsigned long long uploaded, unsigned long long left,
               Event e) {
    a.downloaded = downloaded;  
    a.uploaded = uploaded;
    a.left = left;
    a.event = e;
    
    if (a.peer_id[0] == 0){
        memcpy(a.peer_id,PEER_ID,20);
    }

    if(a.numwant == 0) {
        a.numwant = NUMWANT;
    }
    int ret = host2addr(&a);
    if(ret == -1){
        printf("Host 2 addr Error\n");
        return ret;
    }
    ret = my_connect(a);
    if(ret == -1)
        return ret;
    ret = send_announce(a);    
    if(ret == -1){
        printf("Send Error\n");
        return ret;
    }

    uint8_t *out;
    int len= my_recv(&out); 
    if(len == -1){
        return -1;
    }
    printf("Received announce  %.*s\n",len,out);
    MyBencode b;
    *res = b.decode_announce((const char *)out,(long long)len);

    //DEBUG
    printf("Interval= %lu, Min_interval= %lu, tracker_id = %s, complete = %lu, \
incomplete = %lu\n Peers: \n",(*res)->interval,(*res)->min_interval,(*res)->tracker_id,
    (*res)->complete, (*res)->incomplete); 
    printf("The number of peers is: %d\n",(*res)->num_peers);
    for(int i = 0; i < (*res)->num_peers; i++){
        printf("> %s\n",ip2str((*res)->peers[i])); 
    }
    return 0;
}
}
