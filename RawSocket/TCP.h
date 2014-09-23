#ifndef TCP_H

#define TCP_H 1

#include <utility>
#include "pthread.h"
#include <list>
#include "RawSocket.h"
#include "IP.h"




struct pseudo_tcp_header
{	u_int32_t src_address;
	u_int32_t dst_address;
	u_int8_t place_holder;
	u_int8_t protocol_type;
	u_int16_t tcp_len;};

class TCP
{

	private: int fd;

	IP* ip;

	struct sockaddr_in dest;

	struct sockaddr_in source;


	// POSIX Threads

	pthread_t receiver;
	pthread_mutex_t mutex;
	pthread_t sender;
	pthread_cond_t wait_for_ack;
	pthread_cond_t receive_when_queue_empty;
	pthread_cond_t send_when_queue_empty;


	u_int32_t ack_num;
	u_int32_t seq_num;

	int receiver_closed;
	int sender_closed;

	std::list< std::pair<char*, int> > sender_queue;

	int wait_for_ack_start;
	char* sender_tmp;
	int sender_tmp_len;

	std::list< std::pair<char*, int> > receiver_queue;

	int _constructPacket(struct tcphdr* thdr, char* option, char* data, int len, char* buf);
	int _syn_fn();
	int _ack_fn();

	public: TCP(int fd, struct sockaddr_in* src, struct sockaddr_in* dst);

	TCP();

	int send(const char* buffer, int length);
	int openConnection();
	int receive(char* buffer, int length);
	int closeConnection();
	void sender_thread();
	void receiver_thread();};

#endif
