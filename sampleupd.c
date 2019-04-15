#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <linux/ip.h>  /* for ipv4 header */
#include <linux/udp.h> /* for upd header */

#define ADDR_TO_BIND "127.0.0.1"
#define PORT_TO_BIND 9090

#define MSG_SIZE 256
#define HEADER_SIZE (sizeof(struct iphdr) + sizeof(struct udphdr))

struct udpheader
{
    unsigned short int udph_srcport;
    unsigned short int udph_destport;
    unsigned short int udph_len;
    unsigned short int udph_chksum;
};

int main(void)
{
    int raw_socket;
    struct sockaddr_in sockstr, source_socket_address, dest_socket_address;
    socklen_t socklen;

    int retval = 0; /* the return value (give a look when an error happens)
                     */

    /* no pointer to array!
     * >> It was like "a variable that contains an address -- and in this
     *    address begins an array of chars"! */
    /* now it is simple an array of chars :-)  */
    char msg[MSG_SIZE];
    ssize_t msglen; /* return value from recv() */

    /* do not use IPPROTO_RAW to receive packets */
    if ((raw_socket = socket(PF_INET, SOCK_RAW, IPPROTO_UDP)) == -1)
    {
        perror("socket");
        return 1; /* here there is no clean up -- retval was not used */
    }

    sockstr.sin_family = AF_INET;
    sockstr.sin_port = htons(PORT_TO_BIND);
    sockstr.sin_addr.s_addr = inet_addr(ADDR_TO_BIND);
    socklen = (socklen_t)sizeof(sockstr);

    /* use socklen instead sizeof()  Why had you defined socklen? :-)  */
    if (bind(raw_socket, (struct sockaddr *)&sockstr, socklen) == -1)
    {
        perror("bind");
        retval = 1; /* '1' means "Error" */
        goto _go_close_socket;
    }

    memset(msg, 0, MSG_SIZE);
    int count = 0;
    while (1)
    {
        if ((msglen = recv(raw_socket, msg, MSG_SIZE, 0)) == -1)
        {
            perror("recv");
            retval = 1;
            goto _go_close_socket;
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
            printf("Incoming Packet: rcvd %d\n", count++);
            printf("Packet Size (bytes): %d\n", ntohs(ip_packet->tot_len));
            printf("Source Address: %s\n", (char *)inet_ntoa(source_socket_address.sin_addr));
            printf("Destination Address: %s\n", (char *)inet_ntoa(dest_socket_address.sin_addr));
            printf("Identification: %d\n", ntohs(ip_packet->id));
            printf("udp source port: %d\n",udpheader->source);
            printf("udp dest port: %d\n",udpheader->dest);
            
              for(int i=0;i<256;i++)
                printf("%x",msg[i]);
                printf("\n");
        }
    }
_go_close_socket:
    close(raw_socket);

    return retval;
}