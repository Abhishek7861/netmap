#include <sys/types.h>
#define __FAVOR_BSD
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <iostream>
#include <string.h>
#include <cstdlib>
#include<stdio.h>

char data[1024] = "";

using namespace std;

unsigned short csum(unsigned short *buf,int nwords)
{
	//this function returns the checksum of a buffer
	unsigned long sum;
	for (sum = 0; nwords > 0; nwords--){sum += *buf++;}
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	return (unsigned short) (~sum);
}

int createRaw(int protocol_to_sniff)
{
	int raw_fd = socket(AF_INET, SOCK_RAW, protocol_to_sniff);
	if (raw_fd < 0)
	{
		cout << "ERROR creating raw socket\n";
		exit(1);
	}else{
		cout << "Raw Socket Created!		:-D\n";
		return raw_fd;
	}
}
int bindRaw(int socketToBind,sockaddr_in* sin)
{
	int err = bind(socketToBind,(struct sockaddr *)sin,sizeof(*sin));
	if (err < 0)
	{
		cout << "ERROR binding socket.\n";
		exit(1);
	}else{
		cout << "Bound socket!			:-D\n";
		return 0;
	}
}


struct udpheader {
 unsigned short int source;
 unsigned short int dest;
 unsigned short int len;
 unsigned short int chksum;
};

int main()
{
    struct sockaddr_in sin;

    bzero((char *)& sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(55000);	//port to send packet to
	sin.sin_addr.s_addr = inet_addr("127.0.0.1");	//IP to send packet to

    unsigned char packetBuf[4096];
    unsigned short buffer_size = sizeof(struct ip) + sizeof(struct udpheader);//+ sizeof(data);
	cout << "Buffer size: " << buffer_size << endl;

    struct iphdr *IPheader = (struct iphdr *) packetBuf;
	struct udpheader *udph= (struct udpheader *) (packetBuf + sizeof (struct iphdr));
	
	//Fill out IP Header information:
	IPheader->ihl = 5;
	IPheader->version = 4;		//IPv4
	IPheader->tos = 0;		//type of service
	IPheader->tot_len = htons(buffer_size);	//length
	IPheader->id = htonl(54321);
	IPheader->frag_off = 0;
	IPheader->ttl = 255;	//max routers to pass through
	IPheader->protocol = 6;		//tcp
	IPheader->check = 0;	//Set to 0 before calulating later
	IPheader->saddr = inet_addr("123.4.5.6");	//source IP address
	IPheader->daddr = inet_addr("127.0.0.1");	//destination IP address
	
	//Fill out TCP Header information:
    udph->source = htons(55000);
    udph->dest = htons(55000);
    udph->len = htons(sizeof(struct udpheader)+20);
    memcpy((packetBuf+sizeof(iphdr)+sizeof(udpheader)),"hello",5);
	
	//Now fill out the checksum for the IPheader
	IPheader->check = csum((unsigned short *) packetBuf, IPheader->tot_len >> 1);
	cout << "IP Checksum: " << IPheader->check << endl;
	//create raw socket for sending ip packet
	int sendRaw = createRaw(17);
	if (sendRaw < 0)
	{
		cout << "ERROR creating raw socket for sending.\n";
		exit(1);
	}else{
		cout << "Raw socket created for sending!	:-D\n";
	}
	int sendErr = sendto(sendRaw,packetBuf,
		sizeof(packetBuf),0,(struct sockaddr *)&sin,sizeof(sin));
	
	if (sendErr < sizeof(packetBuf))
	{
		cout << sendErr << " out of " << sizeof(packetBuf) << " were sent.\n";
		exit(1);
	}else{
		cout << "<" << sendErr << "> Sent message!!!		:-D\n";
	}
}
