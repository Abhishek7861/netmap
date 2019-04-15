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
#include <stdio.h>

char data[1024] = "";

#define MSG_LEN 6
using namespace std;

unsigned short csum(unsigned short *buf, int nwords)
{
	//this function returns the checksum of a buffer
	unsigned long sum;
	for (sum = 0; nwords > 0; nwords--)
	{
		sum += *buf++;
	}
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	return (unsigned short)(~sum);
}

int createRaw(int protocol_to_sniff)
{
	int raw_fd = socket(AF_INET, SOCK_RAW, protocol_to_sniff);
	if (raw_fd < 0)
	{
		cout << "ERROR creating raw socket\n";
		exit(1);
	}
	else
	{
		cout << "Raw Socket Created!		:-D\n";
		return raw_fd;
	}
}
int bindRaw(int socketToBind, sockaddr_in *sin)
{
	int err = bind(socketToBind, (struct sockaddr *)sin, sizeof(*sin));
	if (err < 0)
	{
		cout << "ERROR binding socket.\n";
		exit(1);
	}
	else
	{
		cout << "Bound socket!			:-D\n";
		return 0;
	}
}

struct udpheader
{
	unsigned short int source;
	unsigned short int dest;
	unsigned short int len;
	unsigned short int chksum;
};

int main(int argc, char *argv[])
{
	//create raw socket for binding
	int bindSocket = createRaw(17);

	//create structures
	struct sockaddr_in sin, source_socket_address, dest_socket_address;
	unsigned char packetBuf[65000];

	//specify port to bind to
	bzero((char *)&sin, sizeof(sin));
	sin.sin_port = htons(55000);

	//bind socket
	bindRaw(bindSocket, &sin);

	//inform os of recieving raw ip packet
	{unsigned char packetBuf[65000];
		int tmp = 1;
		setsockopt(bindSocket, 0, IP_HDRINCL, &tmp, sizeof(tmp));
	}

	//re-use socket structure
	//Details about where this custom packet is going:
	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(55000);				  //port to send packet to
	sin.sin_addr.s_addr = inet_addr("127.0.0.1"); //IP to send packet to

	unsigned short buffer_size = sizeof(struct ip) + sizeof(struct udpheader); //+ sizeof(data);
	cout << "Buffer size: " << buffer_size << endl;

	struct iphdr *IPheader = (struct iphdr *)packetBuf;
	struct udpheader *udph = (struct udpheader *)(packetBuf + sizeof(struct iphdr));

	//Fill out IP Header information:
	IPheader->ihl = 5;
	IPheader->version = 4;					//IPv4
	IPheader->tos = 0;						//type of service
	IPheader->tot_len = htons(buffer_size); //length
	IPheader->id = htonl(54321);
	IPheader->frag_off = 0;
	IPheader->ttl = 255;					  //max routers to pass through
	IPheader->protocol = 6;					  //tcp
	IPheader->check = 0;					  //Set to 0 before calulating later
	IPheader->saddr = inet_addr("127.0.0.1"); //source IP address
	IPheader->daddr = inet_addr("127.0.0.1"); //destination IP address

	//Fill out TCP Header information:
	udph->source = htons(55000);
	udph->dest = htons(55000);
	udph->len = htons(sizeof(struct udpheader) + MSG_LEN);
	udph->chksum = 232;
	char hello[MSG_LEN];
	for (int i = 0; i < MSG_LEN; i++)
		hello[i] = 'h';
	memcpy((packetBuf + sizeof(iphdr) + sizeof(udpheader)), hello, strlen(hello));
	// TCPheader->udph_srcport = htons(55000);	//source port
	// TCPheader->udph_destport = htons(55000);			//destination port
	// TCPheader->th_seq = random();
	// TCPheader->th_ack = 0;	//Only 0 on initial SYN
	// TCPheader->th_off = 0;
	// TCPheader->th_flags = TH_SYN;	//SYN flag set
	// TCPheader->th_win = htonl(65535);	//used for segmentation
	// TCPheader->th_sum = 0;				//Kernel fill this out
	// TCPheader->th_urp = 0;

	//Now fill out the checksum for the IPheader
	// IPheader->check = csum((unsigned short *) packetBuf, IPheader->tot_len >> 1);
	cout << "IP Checksum: " << IPheader->check << endl;
	//create raw socket for sending ip packet
	int sendRaw = createRaw(17);
	if (sendRaw < 0)
	{
		cout << "ERROR creating raw socket for sending.\n";
		exit(1);
	}
	else
	{
		cout << "Raw socket created for sending!	:-D\n";
	}
	while (1)
	{
		int sendErr = sendto(sendRaw, packetBuf,
							 sizeof(packetBuf), 0, (struct sockaddr *)&sin, sizeof(sin));

		if (sendErr < sizeof(packetBuf))
		{
			cout << sendErr << " out of " << sizeof(packetBuf) << " were sent.\n";
			exit(1);
		}
		else
		{
			cout << "<" << sendErr << "> Sent message!!!		:-D\n";
		}
		break;
	}
	return 0;
}