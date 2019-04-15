#include <iostream>
#include <thread>
#include <vector>
using namespace std;

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <linux/ip.h>  /* for ipv4 header */
#include <linux/udp.h> /* for udp header */

#define ADDR_TO_BIND "127.0.0.1"
#define PORT_TO_BIND 9090

#define MSG_SIZE 65000
#define HEADER_SIZE 47

char msg[MSG_SIZE];
ssize_t msglen;
int raw_socket;
struct sockaddr_in sockstr, source_socket_address, dest_socket_address;
socklen_t socklen;
int retval=0;


// A dummy function
void foo(int id)
{
    memset(msg, 0, MSG_SIZE);
    int count = 0;
    while (1)
    {
        if ((msglen = recv(raw_socket, msg, MSG_SIZE, 0)) == -1)
        {
            perror("recv");
            retval = 1;
        }
        struct iphdr *ip_packet = (struct iphdr *)msg;
        struct data *datagram = (struct data *)(msg + sizeof(struct iphdr) + sizeof(struct udphdr));
        struct udphdr *udpheader = (struct udphdr *)(msg + sizeof(struct iphdr));
        memset(&source_socket_address, 0, sizeof(source_socket_address));
        source_socket_address.sin_addr.s_addr = ip_packet->saddr;
        memset(&dest_socket_address, 0, sizeof(dest_socket_address));
        dest_socket_address.sin_addr.s_addr = ip_packet->daddr;
        if ((ip_packet->saddr) == (ip_packet->daddr))
        {
            printf("thread id %d\n",id);
            printf("Incoming Packet: rcvd %d\n", count++);
            printf("received msg size: %d\n",msglen);
            // printf("Packet Size (bytes): %d\n", ntohs(ip_packet->tot_len));
            // printf("Source Address: %s\n", (char *)inet_ntoa(source_socket_address.sin_addr));
            // printf("Destination Address: %s\n", (char *)inet_ntoa(dest_socket_address.sin_addr));
            // printf("Identification: %d\n", ntohs(ip_packet->id));
            // printf("udp source port: %d\n", udpheader->source);
            // printf("udp dest port: %d\n", udpheader->dest);
            
            // for (int i = 0; i < MSG_SIZE; i++)
            //     printf("%x", msg[i]);
            // printf("\n");
        }
    }
}

int main()
{

    if ((raw_socket = socket(PF_INET, SOCK_RAW, IPPROTO_UDP)) == -1)
    {
        perror("socket");
        return 1;
    }

    sockstr.sin_family = AF_INET;
    sockstr.sin_port = htons(PORT_TO_BIND);
    sockstr.sin_addr.s_addr = inet_addr(ADDR_TO_BIND);
    socklen = (socklen_t)sizeof(sockstr);

    if (bind(raw_socket, (struct sockaddr *)&sockstr, socklen) == -1)
    {
        perror("bind");
        retval = 1;
    }

    int n = 0;
    cout << "how many threads you want" << endl;
    cin >> n;
    std::vector<std::thread> threads;
    for (int i = 0; i < n; i++)
    {
        threads.push_back(std::thread(foo, i));
    }

    for (auto &t : threads)
        t.join();

    return 0;
}