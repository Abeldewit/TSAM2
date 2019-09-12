#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <zconf.h>
#include <string.h>
#include <strings.h>
#include <cmath>
#include <thread>
#include <vector>
#include <sstream>
#include "test.cpp"

//
// Created by Abel de Wit on 2019-08-15.
//

std::vector<int> ports;
std::vector<int> closedP;

void find_missing(std::vector<int> closed, int b_port, int e_port) {
    int n = e_port - b_port;
    std::sort(closed.begin(), closed.end());

    int last = b_port;
    for (int i = 0; i < n; i++) {
        if(last == closed[i]) {
            // it's in the list
            last++;
        } else {
            ports.push_back(last);
            i--;
            last++;
        }
    }
}

void udp_send(char * address, int port) {
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    sockaddr_in current;
    current.sin_addr.s_addr = inet_addr(address);
    current.sin_family = AF_INET;
    current.sin_port = htons(port);
    char datagram[16];
    memset(datagram, '\0', sizeof(datagram));
    strcpy(datagram, "knock");

    sendto(s, datagram, sizeof(datagram), 0, (struct sockaddr *) &current, sizeof(current));

}


void icmp_recv(char * address) {
    int r = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
    sockaddr_in recv;
    recv.sin_family = AF_INET;
    recv.sin_addr.s_addr = inet_addr(address);

    unsigned char read_buffer[4096] = {0};
    struct ip * iphdr = (struct ip *) (read_buffer);
    struct icmphdr * icmph = (struct icmphdr *) (read_buffer + sizeof(struct ip));

    int count = 0;
    while(count < 97) {
        memset(read_buffer, 0, 4096);
        socklen_t len;
        // Then we wait to receive a response
        int n = recvfrom(r, read_buffer, sizeof(read_buffer), 0, (struct sockaddr *) &recv, &len);
        if ( n > 0) {
            // feels like these values are too hardcoded but I don't know how to find
            // these values based on header length etc.
            int port = (read_buffer[50] << 8) | read_buffer[51];
            // same goes for this
            if(read_buffer[20] == 3 && read_buffer[21] == 3) {
//                std::cout << "\nReceived port ICMP" << std::endl;
//                printf("%02x ", read_buffer[50]);
//                printf(" ");
//                printf("%02x ", read_buffer[51]);
//                printf(" \n");
                closedP.push_back(port);
                //std::cout << "Port: " << port << " closed" << std::endl;
                count++;
            }

        } else {
            //std::cout << n << std::endl;
        }
    }
}


int main(int argc, char* argv[])
{
    // Cast the input ports to int
    int b_port = atoi(argv[2]);
    int e_port = atoi(argv[3]);

    std::thread icmp_ear(icmp_recv, argv[1]);

    std::vector<std::thread *> tasks;
    for ( int i = b_port; i < (e_port+1); i++) {
        tasks.push_back(new std::thread(udp_send, argv[1], i));
        usleep(1000 * 1000); //sleep for 100 seconds but it barely takes 1 second?
    }
    for (int i=0; i< e_port - b_port ; i++){
        tasks[i]->join();
        delete tasks[i];
    }

    icmp_ear.join();
    find_missing(closedP, b_port, e_port);
    std::cout << ports[0] << " " <<
                 ports[1] << " " <<
                 ports[2] << " " <<
                 ports[3] << " " << std::endl;
    std::cout << "Done!";
    return 0;
}
