#ifndef GET_H_
#define GET_H_
#include <netinet/in.h>
#include <arpa/inet.h>
#include "MyBencode.hh"
#include "utility.hh"

#define NUMWANT 50
namespace cmsc417 {

    enum Event { NONE, STARTED, STOPPED, COMPLETED };

    struct announcerequest {
        char *tracker_str;
        uint8_t info_hash[20];
        unsigned long long downloaded;
        unsigned long long left;
        unsigned long long uploaded;
        Event event;
        unsigned short my_port;
        // ONLY SET VALUES ABOVE THIS LINE
        struct sockaddr_in tracker;
        uint8_t peer_id[20];
        unsigned long numwant;
    };

    struct announceresponse {
        bool failed;
        const char *fail_message;
        long long fail_len;
        unsigned long interval;
        unsigned long min_interval;
        const char *tracker_id;
        unsigned long complete;
        unsigned long incomplete;
        struct sockaddr_in *peers;
        unsigned short num_peers;
    };
    int t_announce(struct announcerequest a, struct announceresponse **out,
                   unsigned long long downloaded, unsigned long long uploaded,
                   unsigned long long left, Event e);
    struct announcerequest a_req_init(struct TorrentInfo ti, uint16_t port);
}
#endif

//"GET /announce?info_hash=T%7f%bc%a6%92%bb%8a%8b%2aL%b9%a3%0f%a59%f3%98%e6%0c%eb&peer_id=-TR2840-e7s1q3kaltox&port=51413&uploaded=0&downloaded=0&left=2097152&numwant=80&key=32fda35d&compact=1&supportcrypto=1&event=started HTTP/1.1\r\nUser-Agent: Transmission/2.84\r\nHost: 128.8.124.8:8888\r\nAccept: */*\r\nAccept-Encoding: gzip;q=1.0, deflate, identity\r\n\r\n";
