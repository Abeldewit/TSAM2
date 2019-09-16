#include <iostream>
#include <cstdio>
#include <cstdlib>
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
// Created by Abel de Wit on 2019-09-12.
//

unsigned short csum(unsigned short *buf, int nwords)
{
    unsigned long sum;
    for(sum=0; nwords>0; nwords--)
        sum += *buf++;
    sum = (sum >> 16) + (sum &0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

struct pseudo_header {
    struct in_addr src, dst;
    uint8_t zeros;
    u_char protocol;
    u_short uh_len;
};

int main(int argc, char* argv[]) {
    int portMap[4];
    portMap[0] = 4080;
    portMap[1] = 4060;
    portMap[2] = 4070;
    portMap[3] = 4042;

    std::cout << "Easy port: ";
    std::cin >> portMap[0];
    std::cout << "Evil port: ";
    std::cin >> portMap[1];
    std::cout << "Csum port:";
    std::cin >> portMap[2];
    std::cout << "Oracle:";
    std::cin >> portMap[3];

    int port = portMap[0];

    /* Socket setup */
    int s = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    int one = 1;
    const int *val = &one;
    if(setsockopt(s, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0) {
        printf("Could not set HDRINCL!");
    }

    /* Setup of destination and source information */
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = inet_addr(argv[1]);

    struct sockaddr_in source;
    source.sin_family = AF_INET;
    source.sin_port = htons(60434);
    source.sin_addr.s_addr = inet_addr("10.3.37.124");

    /* ---------------- Packet ---------------- */
    /* Some things we need to make a packet */
    char datagram[4096];
    char payload[2048] = "Hi there!";
    memset(datagram, 0, 4096);

    struct ip* iph = (struct ip *)(datagram);
    struct udphdr* udph = (struct udphdr *) (datagram + sizeof(struct ip));

    /* Ip header */
    iph->ip_hl = 5;
    iph->ip_v = 4;
    iph->ip_tos = 0x00;
    iph->ip_len = sizeof(struct ip) + sizeof(struct udphdr) + strlen(payload);
    iph->ip_id = htons (54321);
    iph->ip_off = htons(0x80);
    iph->ip_ttl = 255;
    iph->ip_p = 17;
    iph->ip_sum = 0;
    iph->ip_src.s_addr = source.sin_addr.s_addr;
    iph->ip_dst.s_addr = sin.sin_addr.s_addr;

    /* UDP header */
    udph->uh_sport = htons(60434);
    udph->uh_dport = htons(port);
    udph->uh_sum = 0;
    udph->uh_ulen = htons(8 + strlen(payload));

    /* Payload */
    strcpy(datagram + iph->ip_len - strlen(payload), payload);

    /* Calculate IP checksum on completed header */
    iph->ip_sum = csum((unsigned short *)(datagram), sizeof(struct ip));

    /* ---------------- Checksum ---------------- */
    /* Making my package for the checksum */
    char udpsummer[224];
    struct pseudo_header *psh = (struct pseudo_header *) udpsummer;
    struct udphdr *sudph = (struct udphdr *) udpsummer + sizeof(struct udphdr);

    /* Pseudo Header */
    psh->src = iph->ip_src;
    psh->dst = iph->ip_dst;
    psh->zeros = 0;
    psh->protocol = IPPROTO_UDP;
    psh->uh_len = udph->uh_ulen;

    /* Copying the UDP Header */ // I should be able to copy the whole thing at once but ¯\_(ツ)_/¯
    sudph->uh_sport = udph->uh_sport;
    sudph->uh_dport = udph->uh_dport;
    sudph->uh_ulen = htons(8+ strlen(payload));

    /* We have to take the data into account as well! */
    strcpy(udpsummer + sizeof(struct pseudo_header) + sizeof(struct udphdr), payload);

    /* Calculating the sum */
    udph->uh_sum = csum((unsigned short *)(udpsummer), sizeof(struct udphdr) + strlen(payload));

    /* Loop for all four ports */
    for (int i = 0; i < 4; i ++) {
        sin.sin_port = htons(portMap[i]);
        udph->uh_dport = htons(portMap[i]);
        sudph->uh_dport = htons(portMap[i]);
        udph->uh_sum = csum((unsigned short *)(udpsummer), sizeof(struct udphdr) + strlen(payload));

        // And off it goes!
        if (sendto(s, datagram, iph->ip_len, MSG_WAITALL, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
            std::cout << "Send failed!" << std::endl;
            exit(1);
        }
    }

}