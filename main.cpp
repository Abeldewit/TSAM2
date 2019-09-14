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
#include <map>
#include "knocker.h"
#include "knocker.cpp"

//
// Created by Abel de Wit on 2019-08-15.
//

std::vector<int> ports;
int portMap[4];
std::vector<int> closedP;
std::vector<int> testP;
int total;

// This is just pure aesthetics :)
void loadingbar(float progress) {
    int barWidth = 70;

    std::cout << "[";
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
    std::cout.flush();
}

void find_missing(std::vector<int> closed, int b_port) {
    std::sort(closed.begin(), closed.end());

    int last = b_port;
    for (int i = 0; i < closed.size(); i++) {
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
    int count = 0;
    int count2 = total - 3;
    while(count < count2) {
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
                closedP.push_back(port);
                //std::cout << "Port: " << port << " closed" << std::endl;
                loadingbar((float) count / (float) count2);
            }
        }
        count++;
    }
    printf("\nDone scanning!\n");
}

void udp_check(char * address) {
    for(int i = 0; i < 4; i++) {
        // Timer
        struct timeval timeInterval;
        timeInterval.tv_sec = 5;
        timeInterval.tv_usec = 0;
        int k = socket(AF_INET, SOCK_DGRAM, 0);
        setsockopt(k, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeInterval, sizeof(timeInterval));

        // Sockaddr
        sockaddr_in recvk;
        recvk.sin_addr.s_addr = inet_addr(address);
        recvk.sin_family = AF_INET;
        recvk.sin_port = htons(ports[i]);

        // "Knock knock"
        char datagram[16];
        memset(datagram, '\0', sizeof(datagram));
        strcpy(datagram, "knock");

        // Send!
        sendto(k, datagram, sizeof(datagram), 0, (struct sockaddr *) &recvk, sizeof(recvk));

        // Now try to receive something
        unsigned char buf[100];
        memset(buf, 0, sizeof(buf));
        socklen_t len;
        int n = recvfrom(k, buf, sizeof(buf), 0, (struct sockaddr *) &recvk, &len);
        if (n > 0) {
            std::cout << "Port " << ports[i] << ": " << buf << std::endl;
            if(strncmp((char *)buf, "I am", 4) == 0) {
                // Oracle = 3
                portMap[3] = ports[i];
            } else if(strncmp((char *) buf, "I only", 6) == 0) {
                // Evil bit = 1
                portMap[1] = ports[i];
            } else if(strncmp((char *)buf, "Please", 6) == 0) {
                // Checksum  = 2
                portMap[2] = ports[i];
            } else if(strncmp((char *)buf, "This", 4) == 0) {
                // 'Easy' port = 0
                portMap[0] = ports[i];
            }
        } else {
            std::cout << "failed" << std::endl;
        }
    }
}

int main(int argc, char* argv[])
{
    // Easier acces if a scan has just been done
    portMap[0] = 4099;
    portMap[1] = 4097;
    portMap[2] = 4098;
    portMap[3] = 4042;

    // Initializing a new scan
    if(argc > 4) {
        if (strncmp(argv[4], "-n", 2) == 0) {
            // Cast the input ports to int
            int b_port = atoi(argv[2]);
            int e_port = atoi(argv[3]);
            total = e_port - b_port + 1;

            std::thread icmp_ear(icmp_recv, argv[1]);

            std::vector<std::thread *> tasks;
            for (int i = b_port; i < (e_port + 1); i++) {
                testP.push_back(i);
                tasks.push_back(new std::thread(udp_send, argv[1], i));
                usleep(1000 * 1000); //sleep for 1000 seconds but it barely takes 1 second?
            }
            for (int i = 0; i < e_port - b_port; i++) {
                tasks[i]->join();
                delete tasks[i];
            }

            icmp_ear.join();
            find_missing(closedP, b_port);
            udp_check(argv[1]);
        }
    }


    // Part b!
    for (int i = 0; i < 4; i++) {
        std::cout << "Probing port " << portMap[i] << " with type " << i << std::endl;
        send_modpack(argv[1], portMap[i], i);
        usleep(100000);
    }

    //send_modpack(argv[1], portMap[2], 2);
    std::cout << "Done!";
}
