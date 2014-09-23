#ifndef RW_SCK_H
#define RW_SCK_H 1

#include <netinet/tcp.h>
#include <errno.h>
#include <netinet/ip.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <stdio.h>


int raw_socket_send(int, const char*, int);

int raw_socket_receive(int, char*, int);

int raw_socket_close(int fd);

int rawSocket(int, int, int);

int raw_socket_connect(int fd, struct sockaddr* dst_addr, int addr_len);


#endif
