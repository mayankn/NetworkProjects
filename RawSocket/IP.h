#ifndef IP_H
#define IP_H 1

#include "RawSocket.h"

class IP
{	private:int send_fileDesc;
	int receive_fileDesc;
	struct sockaddr_in dst;
	int mac_addr_found;
	int interface_idx;
	struct sockaddr_in src;
	char localMAC[6];
	char gtwayMAC[6];

	public:IP() : send_fileDesc(0), receive_fileDesc(0){};
		IP(struct sockaddr_in* source, struct sockaddr_in* dest, int fileDesc);
	static unsigned short checksum(unsigned short* buf, int len);
	int receive(char* buf, int len);
	int sender(char* buf, int len);
	int receiveIP(int fileDesc, char* data, int data_len, int flag, struct sockaddr* sckaddr, int dst_len);
	int sendIP(int fileDesc, char* data, int data_len, int flag, struct sockaddr* sckaddr, int dst_len);
};

#endif
