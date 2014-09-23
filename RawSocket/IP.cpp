#include <time.h>
#include "IP.h"
#include "netpacket/packet.h"
#include "ARPReq.h"
#include "netinet/if_ether.h"
#include <unistd.h>
#include <iostream>

using namespace std;

static unsigned packet_id = 0;

IP::IP(struct sockaddr_in* source, struct sockaddr_in* dest, int fileDesc)
{
	send_fileDesc = receive_fileDesc = fileDesc;
	memcpy(&src, source, sizeof(struct sockaddr_in));
	memcpy(&dst, dest, sizeof(struct sockaddr_in));

	struct timeval timeout = {15, 0};
	int error = setsockopt(receive_fileDesc, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(struct timeval));

	if (error < 0)
	{
		fprintf(stderr, "error: setsockopt: %s\n", strerror(errno));
	}

	mac_addr_found = 0;

}

int IP::receiveIP(int fileDesc, char* buffer, int len, int flag, struct sockaddr* sckaddr, int dst_len)
{
	struct ether_header* ethernetHeader;
	struct iphdr* ipHeader;

	char ip_packet[65535];
	int error = recvfrom(fileDesc, ip_packet, 65535, flag, sckaddr, 0);

	if (error < 0)
	return error;

	ipHeader = (struct iphdr*) 
		(ip_packet + sizeof(struct ether_header));
	memcpy
	(buffer, ip_packet + sizeof(struct ether_header), ntohs(ipHeader->tot_len));

	return 0;
}


int IP::receive(char* buf, int len)
{	int error;
	int previous_time = time(NULL);

	while (1) {	
		if (time(NULL) - previous_time > 2)
		{return 1;}

		error = receiveIP(receive_fileDesc, buf, len, 0, NULL, 0);

		if (errno == EAGAIN
				|| errno == EWOULDBLOCK)
		{continue;}

		if (error < 0)
		{	fprintf(stderr, "receive failed from: %s\n", strerror(errno));
			return -1;}

		struct iphdr* ipHeader;
		struct tcphdr* tcpHeader;

		ipHeader = (struct iphdr*) buf;

		unsigned short check_sum = ipHeader->check;
		ipHeader->check = 0;
		ipHeader->check = checksum((unsigned short*)buf, ipHeader->ihl * 4);

		if (ipHeader->check != check_sum)
		{	fprintf(stderr, "wrong IP checksum\n");
			continue;}

		if (ipHeader->protocol == 6)
		{
			tcpHeader = (struct tcphdr*)(buf + ipHeader->ihl * 4);

			if (tcpHeader->dest == src.sin_port)
			{return 0;}
		}}
}

int IP::sender(char* buf, int len)
{return sendIP(send_fileDesc, buf, len, 0, (struct sockaddr*)&dst, sizeof (struct sockaddr_in));}

int IP::sendIP(int fileDesc, char* data, int data_len, int flag, struct sockaddr* sckaddr, int dst_len)
{
	struct ether_header ethernetHeader;
	struct iphdr ipHeader;
	memset(&ethernetHeader, 0, sizeof(ethernetHeader));
	memset(&ipHeader, 0, sizeof(ipHeader));
	
	char gtway_mac[256];
	char local_mac[256];
	int ifc_idx = 0;
	int error;

	if (!mac_addr_found)
	{
		error = ARPReq::getGatewayMacAddr(gtway_mac, local_mac, &ifc_idx);

		if (error < 0)
		{fprintf(stderr, "FATAL: cannot resolve gateway MAC address");
			return -1;}

		mac_addr_found = 1;
		interface_idx = ifc_idx;

		memcpy
		(localMAC, local_mac, 6);

		memcpy
		(gtwayMAC, gtway_mac, 6);

	}

	ifc_idx = interface_idx;

	memcpy
	(&(ethernetHeader.ether_dhost), gtwayMAC, 6);

	memcpy
	(&(ethernetHeader.ether_shost), localMAC, 6);
	ethernetHeader.ether_type = htons(ETH_P_IP);

	ipHeader.version = 4;
	ipHeader.ihl = 5;
	ipHeader.tos = 0;
	ipHeader.tot_len = htons(data_len + sizeof(struct iphdr));
	ipHeader.id = htons(packet_id++);
	ipHeader.frag_off = htons(0x4000);
	ipHeader.ttl = 64;
	ipHeader.protocol = IPPROTO_TCP;
	ipHeader.check = 0;
	ipHeader.saddr = src.sin_addr.s_addr;
	ipHeader.daddr = dst.sin_addr.s_addr;
	ipHeader.check = checksum((unsigned short*) &ipHeader, sizeof(struct iphdr));

	char buffer[65535];

	memcpy
	(buffer, (char*)&ethernetHeader, sizeof(struct ether_header));

	memcpy
	(buffer + sizeof(struct ether_header), (char*)&ipHeader, sizeof(struct iphdr));

	memcpy
	(buffer + sizeof(struct ether_header) + sizeof(struct iphdr),data, data_len);

	struct sockaddr_ll saddr;

	memset
	(&saddr, 0, sizeof(saddr));

	saddr.sll_family = AF_PACKET;
	saddr.sll_ifindex = ifc_idx;
	saddr.sll_protocol = htons(ETH_P_IP);
	error = sendto(fileDesc, buffer, sizeof(struct ether_header) 
	+ sizeof(struct iphdr) 
	+ data_len, flag, (struct sockaddr*)&saddr, sizeof(saddr));

	return error;
}

unsigned short IP::checksum(unsigned short* ptr, int num_bytes)
{
	register long c_sum;
	unsigned short odd_byte;
	register short csum;

	c_sum = 0;

	while (num_bytes > 1)
	{
		c_sum += *ptr ++;
		num_bytes -= 2;
	}

	if (num_bytes == 1)
	{	odd_byte = 0;
		*((u_char*)&odd_byte) = *(u_char*)ptr;
		c_sum += odd_byte;}

	c_sum = (c_sum >> 16) + (c_sum & 0xffff);
	c_sum = c_sum + (c_sum >> 16);
	csum = (short)~c_sum;

	return csum;
}

