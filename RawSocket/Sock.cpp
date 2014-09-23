#include <arpa/inet.h>
#include <cstring>
#include "Sock.h"
#include <cstdio>

using namespace std;

Sock::~Sock()
{	raw_socket_close(fileDesc);}


Sock::Sock(const char* host_str, int port_num)
{	strcpy(hostName_str, host_str);
	this->portNum = port_num;
	status_num = SOCKET_CLOSE;}


int Sock::reconnectSocket()
{	return createSocket();}


int Sock::createSocket()
{
	struct hostent* hosten = NULL;

	hosten = gethostbyname(hostName_str);

	if (hosten == NULL)
	{	fprintf(stderr, "no such host:%s\n", hostName_str);
		return -1;}

	struct sockaddr_in destAddr;
	memcpy(&destAddr.sin_addr.s_addr,
			hosten->h_addr, hosten->h_length);
	destAddr.sin_port =
			htons(portNum);
	destAddr.sin_family =
			AF_INET;
	fileDesc = rawSocket(AF_INET, SOCK_STREAM, 0);

	if (fileDesc < 0)
	{	fprintf(stderr, "socket creation failed\n");}

	if (raw_socket_connect(fileDesc, (struct sockaddr*) &destAddr, sizeof(destAddr)) < 0)
	{	fprintf(stderr, " %s, while connecting to %s.\n", strerror(errno), hostName_str);
		close(fileDesc);
		free(hosten);
		return -1;}
	status_num = SOCKET_ACTIVE;
	return 0;}

int Sock::readStr(char* buffer, int size_num)
{
	if (this->alive())
	{return raw_socket_receive(fileDesc, buffer, size_num);}

	else
	{fprintf(stderr, "cannot read: socket closed\n");}
}



int Sock::writeStr(const char* buffer, int size_num)
{
	if (this->alive())
	{	int write_completed = 0;
		int temp = 0;
		while(write_completed < size_num)
			{	temp = raw_socket_send(fileDesc, buffer + write_completed, size_num - write_completed);
			if (temp <= 0)
			{return write_completed;}
			write_completed += temp;
		}
		return write_completed;}
	else
	{	fprintf(stderr, "ERROR: cannot write: socket closed\n");
		return -1;}}


int Sock::alive()
{return status_num;}

FILE* Sock::readfilePtr()
{return filePtrIn;}



