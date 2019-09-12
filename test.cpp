#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <zconf.h>
#include <string.h>
#include <strings.h>
#include <cmath>

//
// Created by Abel de Wit on 2019-09-10.
//

unsigned short csum (unsigned short *buf, int nwords)
{
    unsigned long sum;
    for (sum = 0; nwords > 0; nwords--)
        sum += *buf++;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return ~sum;
}

int main(int argc, char* argv[]) {
    int s = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    char read_buffer[4096] = {0};
    int P = 4064;

    // Setting up the
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(P);
    sin.sin_addr.s_addr = inet_addr(argv[1]);

    struct sockaddr_in source;
    source.sin_family = AF_INET;
    // TODO find my ip without hardcoding
    source.sin_addr.s_addr = inet_addr("130.208.243.61");

    // Making the header!
    char datagram[4096];

    struct ip* iph = (struct ip *) datagram;
    struct udphdr* udph = (struct udphdr *) (sizeof(struct ip) + datagram);
    memset(datagram, 0, 4096);
    iph->ip_hl = 5;
    iph->ip_v = 4;
    iph->ip_tos = 0;
    iph->ip_len = sizeof(struct ip) + sizeof(struct udphdr);
    iph->ip_id = htons (14118);
    iph->ip_off = 0;
    iph->ip_ttl = 255;
    iph->ip_p = 17;
    iph->ip_sum = 0;
    iph->ip_src.s_addr = source.sin_addr.s_addr;
    iph->ip_dst.s_addr = sin.sin_addr.s_addr;

    // Making the UDP header
    udph->uh_sport = htons(2323);
    udph->uh_dport = htons(P);
    udph->uh_sum = csum ((unsigned short *) datagram, iph->ip_len >> 1);;
    udph->uh_ulen = htons(sizeof(udph));

    iph->ip_sum = csum ((unsigned short *) datagram, iph->ip_len >> 1);

    int one = 1;
    const int *val = &one;
    if(setsockopt(s, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0) {
        printf("Could not set HDRINCL!");
    }
    struct timeval timeInterval;
    timeInterval.tv_sec = 1;
    timeInterval.tv_usec = 5000;
    int r = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    setsockopt(r, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeInterval, sizeof(timeInterval));
    bzero(read_buffer, sizeof(read_buffer));

    for (int i = 4000; i < 4100; i++) {
        P = htons(i);
        udph->uh_dport = P;
        sin.sin_port = P;
        sendto(s, datagram, iph->ip_len, 0, (struct sockaddr *) &sin, sizeof(sin));
        std::cout << "Send to port: " << i << std::endl;
    }

}