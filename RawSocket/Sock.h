
#include <cstdlib>
#include <cerrno>
#include <map>
#include <cstdio>
#include <sys/socket.h>
#include <iostream>
#include "RawSocket.h"
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>

#define SOCKET_ACTIVE 1

#define SOCKET_CLOSE 0

class Sock
{
	private:int fileDesc;
	FILE* filePtrIn;
	FILE* filePtrOut;
	char hostName_str[256];
	int portNum;
	int status_num;

	public:~Sock();
	Sock(const char* host, int port);
	FILE* readfilePtr();
	int readStr(char* buf, int length);
	int createSocket();
	int reconnectSocket();
	int putStr(const char* buf);
	int writeStr(const char* buf, int length);
	char* getStr(char* buf, int length);
	int alive();

};

