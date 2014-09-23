#include <netdb.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include "RawSocket.h"
#include <netpacket/packet.h>
#include <netinet/in.h>
#include "TCP.h"
#include <netinet/if_ether.h>
#include <net/if.h>
#include <map>

using namespace std;

#define STARTING_PORT 1025
#define ENDING_PORT 65534

map<int, TCP*> tcp_table;

static int getNextAvailablePort()
{
	char command[256];
	srand(time(NULL));
	int randomPortNum;

	do
		{randomPortNum =
					rand() % (ENDING_PORT - STARTING_PORT + 1)
					+ STARTING_PORT;
			sprintf(command, "netstat -an | grep :%d> /dev/null", randomPortNum);
		}
	while(!system(command));

	return randomPortNum;}


int rawSocket(int family, int type, int protocol)
{
	int fileDesc = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP));
	if (fileDesc < 0)
		{
			return fileDesc;}
	return fileDesc;}

int getLocalIP(char* buf)
{
	struct ifaddrs * ifAddrStruct =
			NULL;
	struct ifaddrs * ifAddr =
			NULL;
	void * tmpPtr =
			NULL;

	int error =
			getifaddrs(&ifAddrStruct);

	int flag = -1;

	if (error < 0)
		{
			fprintf(stderr, "error: %s\n\n",
					strerror(errno));

		exit(1);}

	for (ifAddr = ifAddrStruct; ifAddr != NULL; ifAddr = ifAddr->ifa_next)
		{
			if (ifAddr->ifa_addr == NULL) continue;
			if (ifAddr ->ifa_addr->sa_family==AF_INET)
				{
					tmpPtr=
							&((struct sockaddr_in *)ifAddr->ifa_addr)->sin_addr;
					char addrBuf[INET_ADDRSTRLEN];
					inet_ntop(AF_INET, tmpPtr, addrBuf, INET_ADDRSTRLEN);
					if (strcmp(addrBuf, "127.0.0.1") == 0) continue;
					strcpy(buf, addrBuf);
					flag = 0;

					break;}}

	if (ifAddrStruct!=NULL)
			freeifaddrs(ifAddrStruct);

	if (flag != 0)
		{
			fprintf(stderr, "Cannot get local IP\n");

		exit(1);}

	return 0;
}


int raw_socket_connect(int fileDesc, struct sockaddr* dst_addr, int addr_len)
	{

	char buffer[1024];
	getLocalIP(buffer);

	struct sockaddr_in dst;

	struct sockaddr_in src;


		memcpy
		(&dst,dst_addr,addr_len);

	int port = getNextAvailablePort();

	src.sin_addr.s_addr =
			inet_addr(buffer);
		src.sin_port =
				htons(port);

		/* Assigning the socket family to AF_INET for using a raw ethernet socket */
		src.sin_family =
			AF_INET;


	TCP* tcp = new TCP(fileDesc, &src, &dst);
	tcp_table[fileDesc] = tcp;

	int retVal = tcp->openConnection();}

int raw_socket_receive(int fileDesc, char* buf, int len)
	{ return tcp_table[fileDesc]->receive(buf, len);}

int raw_socket_send(int fileDesc, const char* buf, int len)
	{ return tcp_table[fileDesc]->send(buf, len);}


int raw_socket_close(int fileDesc)
	{return tcp_table[fileDesc]->closeConnection();}


