#include <cstdio>
#include "ARPReq.h"
#include <iostream>
#include <cstdlib>
#include <net/if.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <netpacket/packet.h>
#include <ifaddrs.h>
#include <sys/types.h>
#include <memory.h>
#include <netinet/if_ether.h>
#include <unistd.h>
using namespace std;


static int getInterfaceIndex(int fileDesc, const char* interface_name)
{
	struct ifreq ifrq;
	memset(&ifrq, 0, sizeof(ifrq));
	strcpy(ifrq.ifr_name, interface_name);
	
	if (ioctl(fileDesc, SIOCGIFINDEX, &ifrq) == -1)
	{	fprintf(stderr, "cannot get interface index\n");
		return -1;}


	return ifrq.ifr_ifindex;
}

static int getLocalIPAddr(char* buffer)
{
	struct ifaddrs * ifAddressStruct=NULL;
    struct ifaddrs * ifAddress=NULL;
    void * tempAddressPtr=NULL;

    int error = getifaddrs(&ifAddressStruct);

	int returnVal = -1;

	if (error < 0)
	{	fprintf(stderr, " %s\n", strerror(errno));
		exit(1);}

    for (ifAddress = ifAddressStruct; ifAddress != NULL; ifAddress = ifAddress->ifa_next) {
    	if (ifAddress->ifa_addr == NULL) continue;

		if (ifAddress ->ifa_addr->sa_family
				==AF_INET)
		{
			tempAddressPtr=&((struct sockaddr_in *)ifAddress->ifa_addr)->sin_addr;
            char addrBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET,
            		tempAddressPtr,
            		addrBuffer,
            		INET_ADDRSTRLEN);

            if (strcmp(addrBuffer, "127.0.0.1") == 0) continue;

            strcpy(buffer, addrBuffer);

            returnVal = 0;

            break;}}

    if (ifAddressStruct!=NULL)
    	freeifaddrs(ifAddressStruct);

	if (returnVal != 0)
	{	fprintf(stderr, "cannot get local IP address\n");
		exit(1);}

	return 0;
}

static int to_dec(char c)
{
	if (c >= '0' && c <= '9')
	{return c - '0';}

	else if (c >= 'A' && c <= 'F')
	{return c - 'A' + 10;}

	else
	{return c - 'a' + 10;}
	
}

static int stripAddr(char* in, char* out)
{
	int j = 0;
	
	int addr[4];

	for (j = 0; j < 4; j ++)
	{addr[j] = to_dec(in[j * 2]) * 16 + to_dec(in[j * 2 + 1]);}

	sprintf(out, "%d.%d.%d.%d", addr[3], addr[2], addr[1], addr[0]);
	return 0;
	
}






static int getLocalMacAddr(char* result_str, char* interface_name)
{
	struct ifreq ifrq;
    struct ifconf ifcnf;
    char buffer[1024];
    int returnVal = -1;

    int fileDesc = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

    if (fileDesc == -1)
	{	fprintf(stderr, "cannot resolve local MAC address\n");
		return returnVal;};

    ifcnf.ifc_len = sizeof(buffer);
    ifcnf.ifc_buf = buffer;

    if (ioctl(fileDesc,
    		SIOCGIFCONF, &ifcnf) == -1)
	{	fprintf(stderr, "cannot resolve local MAC address 1\n");
		return returnVal;}

    struct ifreq* ifr = ifcnf.ifc_req;
    struct ifreq* ifr_end = ifr + (ifcnf.ifc_len / sizeof(struct ifreq));

    for (; ifr != ifr_end; ++ifr)
	{	strcpy(ifrq.ifr_name, ifr->ifr_name);
		strcpy(interface_name, ifrq.ifr_name);

		if (ioctl(fileDesc, SIOCGIFFLAGS, &ifrq) == 0)
		{	if (! (ifrq.ifr_flags & IFF_LOOPBACK))
			{	if (ioctl(fileDesc, SIOCGIFHWADDR, &ifrq) == 0)
				{	returnVal = 0;
                    break;}}}

		else
		{	fprintf(stderr, "cannot resolve local MAC address 2\n");
			return returnVal;}
   }

    if (returnVal == 0)
	{	memcpy(result_str, ifrq.ifr_hwaddr.sa_data, 6);}

	
    return returnVal;
}

static int getGatewayIP(char* buf)
{
	FILE* filePtr = 
			fopen("/proc/net/route", "r");
	char cline[1024];

	while(fgets(cline, 1024, filePtr))
	{
		if (cline[0] 
		          == 'e' && cline[1] 
		                          == 't' && cline[2] == 'h')
		{
			string temp(cline);
			int posn = temp.find("\t");

			if (posn == string::npos)
			{return -1;}

			temp = temp.substr(posn + 1);
			char buffer0[128], buffer1[128];
			sscanf(temp.c_str(), "%s%s", buffer0, buffer1);
			
			char ipAddr[128], gtwayAddr[128];
			stripAddr(buffer0, ipAddr);
			stripAddr(buffer1, gtwayAddr);
			if (strcmp(ipAddr, "0.0.0.0") == 0)
			{	strcpy(buf, gtwayAddr);
				return 0;}
		}
	}
	return -1;
}



static int receiveARP(int fileDesc, char* ipAddr, char* macAddr, char* iinterface_name)
{
	char buffer[65535];

	while (1)
	{	int error = recvfrom(fileDesc, buffer, sizeof(buffer), 0, NULL, NULL);
		if (error < 0)
		{	fprintf(stderr, "error while receiving ARP: %s\n", strerror(errno));
			return -1;}

		struct ether_header* ethernetHeader;
		struct ether_arp* ethernetARPHeader;

		ethernetHeader = (struct ether_header*) buffer;
		ethernetARPHeader = (struct ether_arp*) (buffer + sizeof(struct ether_header));
	
		if (ethernetHeader->ether_type == htons(ETH_P_ARP) &&  ethernetARPHeader->ea_hdr.ar_op == htons(ARPOP_REPLY))
		{	if (*((u_int32_t*)&(ethernetARPHeader->arp_spa)) == inet_addr(ipAddr))
			{	memcpy(macAddr, ethernetARPHeader->arp_sha, 6);
				return 0;}}

		else
		{continue;}
	}

	return -1;
}

static int sendARP(int fileDesc, char* ipAddr, char* macAddr, char* interface_name, int* idx)
{
	struct ether_header ethernetHeader;
	struct ether_arp ethernetARPHeader;

	char localIpAddr[128];
	getLocalIPAddr(localIpAddr);

	u_int32_t localIP = inet_addr(localIpAddr);
	u_int32_t gtwayIP = inet_addr(ipAddr);
	char dest[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	memcpy
	(&(ethernetHeader.ether_dhost), dest, 6);
	memcpy
	(&(ethernetHeader.ether_shost), macAddr, 6);
	ethernetHeader.ether_type
	= htons(ETH_P_ARP);

	ethernetARPHeader.ea_hdr.ar_hrd
	= htons(ARPHRD_ETHER);
	ethernetARPHeader.ea_hdr.ar_pro
	= htons(0x800);
	ethernetARPHeader.ea_hdr.ar_hln
	= 6;
	ethernetARPHeader.ea_hdr.ar_pln
	= 4;
	ethernetARPHeader.ea_hdr.ar_op
	= htons(ARPOP_REQUEST);
	memcpy
	(&(ethernetARPHeader.arp_sha), macAddr, 6);
	memcpy
	(&(ethernetARPHeader.arp_spa), &localIP, 4);
	memcpy
	(&(ethernetARPHeader.arp_tha), dest, 6);
	memcpy
	(&(ethernetARPHeader.arp_tpa), &gtwayIP, 4);

	struct sockaddr_ll saddr;
	memset
	(&saddr, 0, sizeof(saddr));
	saddr.sll_family
	= AF_PACKET;
	saddr.sll_ifindex = getInterfaceIndex(fileDesc, interface_name);
	saddr.sll_protocol = htons(ETH_P_ARP);

	char ethernet_packet[65535];
	memcpy
	(ethernet_packet, (char*) &ethernetHeader, sizeof(ethernetHeader));
	memcpy
	(ethernet_packet + sizeof(ethernetHeader), (char*) &ethernetARPHeader, sizeof(ethernetARPHeader));
	int error = sendto(fileDesc, ethernet_packet, sizeof(ethernetHeader) + sizeof(ethernetARPHeader),
					 0, (struct sockaddr*)&saddr, sizeof(saddr));
	*idx = saddr.sll_ifindex;

	if (error < 0)
	{	fprintf(stderr, "error: %s\n", strerror(errno));
		return -1;}

	return 0;
}


int ARPReq::getGatewayMacAddr(char* buffer, char* temp, int *idx)
{
	int error;
	char ipAddr[128];
	error = getGatewayIP(ipAddr);

	if (error < 0)
	{return error;}

	char macAddr[128];
	char interface_name[128];
	error = getLocalMacAddr(macAddr, interface_name);
	memcpy
	(temp, macAddr, 6);

	if (error < 0)
	{return error;}

	int fileDesc = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));

	if (fileDesc < 0)
	{	fprintf(stderr, "Gateway MAC address not resolved: %s\n", strerror(errno));
		return -1;}
	
	error = sendARP(fileDesc, ipAddr, macAddr, interface_name, idx);

	if (error < 0)
	{	return -1;}

	error = receiveARP(fileDesc, ipAddr, macAddr, interface_name);

	if (error < 0)
	{return -1;}

	memcpy(buffer, macAddr, 6);
	return 0;
}

