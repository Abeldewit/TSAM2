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
// Created by Abel de Wit on 2019-08-15.
//


int main(int argc, char* argv[])
{
    sockaddr_in source,destination;
    char read_buffer[4096] = {0};
    char datagram[4096];

    char *p;
    long b_port = strtol(argv[2], &p, 10);
    long e_port = strtol(argv[3], &p, 10);

    struct ip *iph = (struct ip *) datagram;
    struct udphdr *udph = (struct udphdr *) (datagram + sizeof(struct ip));

    source.sin_family = AF_INET;
    source.sin_port = htons(1234);
    source.sin_addr.s_addr = inet_addr("192.168.1.1");

    destination.sin_family = AF_INET;
    destination.sin_addr.s_addr = inet_addr(argv[1]);


    iph->ip_off = 0;
    iph->ip_ttl = 255;
    iph->ip_p = 17;
    iph->ip_sum = 0;
    iph->ip_src = source.sin_addr;       /* self explanatory :P */
    iph->ip_dst = destination.sin_addr;
    iph->ip_hl = 5;              /* Header length. 4 times this is 20 bytes. Mac value is 15 */
    iph->ip_v = 4;          /* IPv4 used */
    iph->ip_tos = 0;              /* Type of service. 0=> Routine. Kernel might replace this with DSCP/ECN :(.  */
    iph->ip_len = sizeof(datagram);     /* Total length of the datagram */
    iph->ip_id = htons (14118);

    udph->uh_sport = source.sin_port;
    udph->uh_sum = 0;
    udph->uh_ulen = htons(sizeof(udphdr));

    bzero(read_buffer, sizeof(read_buffer));
    int write_sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct timeval timeInterval;
    timeInterval.tv_sec = 0;
    timeInterval.tv_usec = 5000;
    setsockopt(write_sock, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeInterval, sizeof(timeInterval));

    for (int i = (int)b_port; i < e_port + 1; i++) {
        std::cout << "Port " << i;
        destination.sin_port = htons(i);
        udph->uh_dport = destination.sin_port;
        sendto(write_sock, datagram, sizeof(datagram), 0, (struct sockaddr *) &destination, sizeof(destination));

        socklen_t len;
        int n = recvfrom(write_sock, read_buffer, sizeof(read_buffer), MSG_WAITALL, (struct sockaddr *) &destination, &len);
        if (n > 0) {
            std::cout << ": " << read_buffer << std::endl;
        } else if (n == -1) {
            std::cout << ": closed" << std::endl;
        }
        memset(read_buffer, 0, sizeof(read_buffer));
    }

}
