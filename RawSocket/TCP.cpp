#include <unistd.h>
#include "TCP.h"
using namespace std;

static void* receiver_fn(void* context)
{
	TCP* tcpObj = 
			(TCP*) context;
	tcpObj->receiver_thread();	}

static void* sender_fn(void* context)
{
	TCP* tcpObj = 
			(TCP*) context;
	tcpObj->sender_thread();}


TCP::TCP(){};

int TCP::_constructPacket(struct tcphdr* tcpHeader, char* options, char* data, int len, char* buf)
{
	memcpy(buf, tcpHeader, sizeof(struct tcphdr));
	len = data == NULL ? 0: len;

	if (tcpHeader->doff > 5)
	{
		memcpy(buf + 20, options, tcpHeader->doff * 4 - 20);
	}

	if (data != NULL)
	{
		memcpy(buf + tcpHeader->doff * 4, data, len);
	}

	struct pseudo_tcp_header p_tcpHeader;
	p_tcpHeader.src_address = source.sin_addr.s_addr;
	p_tcpHeader.dst_address = dest.sin_addr.s_addr;
	p_tcpHeader.place_holder = 0;
	p_tcpHeader.protocol_type = 6;
	p_tcpHeader.tcp_len = htons(len + tcpHeader->doff * 4);

	int p_tcpHeader_len = sizeof(struct pseudo_tcp_header) + tcpHeader->doff * 4 + len;

	char* tmp = (char*) malloc(p_tcpHeader_len + 1000);
	memcpy(tmp, (char*)&p_tcpHeader, sizeof(struct pseudo_tcp_header));
	memcpy(tmp + sizeof(struct pseudo_tcp_header), buf, tcpHeader->doff * 4 + len);

	tcpHeader = (struct tcphdr*) buf;
	tcpHeader->check = IP::checksum((unsigned short*)tmp, p_tcpHeader_len);
	free(tmp);
	return 0;

}


TCP::TCP(int fileDesc, struct sockaddr_in* src, struct sockaddr_in* dst)
{
	this->fd = fileDesc;
	
	memcpy(&(this->dest), dst, sizeof(struct sockaddr_in));
	
	memcpy(&(this->source), src, sizeof(struct sockaddr_in));
	
	
	ip = new IP(src, dst, fileDesc);

	pthread_mutex_init(&mutex, NULL);
	
	pthread_cond_init(&wait_for_ack, NULL);
	
	pthread_cond_init(&send_when_queue_empty, NULL);
	
	pthread_cond_init(&receive_when_queue_empty, NULL);

		srand(time(NULL));
	
	ack_num = 0;
	
	
	seq_num = 
			htonl(rand() % 0x0fffffff);

	
	sender_closed = 0;

	receiver_closed = 0;
	
	sender_tmp_len = 0;
	
	sender_tmp = NULL;}



int TCP::_ack_fn()
{
	char tcp_packet[65535];
	int error;
	error = ip->receive(tcp_packet, 65535);

	if (error == 1)
	{	fprintf(stderr, "FATAL: timeout occurred while receiving syn-ack_num packet\n");
		exit(1);}

	struct tcphdr* tcpHeader;

	struct iphdr* ipHeader;

	ipHeader = (struct iphdr*) tcp_packet;
	tcpHeader = (struct tcphdr*) (tcp_packet + ipHeader->ihl * 4);

	seq_num = tcpHeader->ack_seq;
	ack_num = htonl(ntohl(tcpHeader->seq) + 1);

	struct tcphdr tcpHeader_sender;
	memset(&tcpHeader_sender, 0, sizeof(struct tcphdr));
	tcpHeader_sender.source = source.sin_port;
	tcpHeader_sender.dest = dest.sin_port;
	tcpHeader_sender.seq = seq_num;
	tcpHeader_sender.ack_seq = ack_num;
	tcpHeader_sender.doff = 5;
	tcpHeader_sender.ack = 1;
	tcpHeader_sender.window = htons(4096);

	char buf[4];
	_constructPacket(&tcpHeader_sender, buf, NULL, 0, tcp_packet);

	return ip->sender(tcp_packet, 20);
}

int TCP::openConnection()
{
	int error;
	error = _syn_fn();

	if (error < 0)
	{
		fprintf(stderr, "cannot send syn packet\n");
		exit(1);}

	error = _ack_fn();

	if (error < 0)
	{
		fprintf(stderr, "FATAL: cannot ack_num syn-ack_num packet\n");
		exit(1);}

	pthread_create(&sender, NULL, sender_fn, (void*) this);

	pthread_create(&receiver, NULL, receiver_fn, (void*) this);
}

int TCP::send(const char* buf, int len)
{
	pthread_mutex_lock(&mutex);

	if (sender_closed)
	{
		pthread_mutex_unlock(&mutex);
		return -1;
	}
	char* tmp = (char*) malloc(len + 1000);
	memcpy(tmp, buf, len);
	pair<char*, int> next_in_queue(tmp, len);
	sender_queue.push_back(next_in_queue);

	pthread_cond_signal(&send_when_queue_empty);

	pthread_mutex_unlock(&mutex);
	return len;
}

int TCP::_syn_fn()
{
	struct tcphdr tcpHeader;
	memset(&tcpHeader, 0, sizeof(struct tcphdr));
	tcpHeader.source = source.sin_port;
	tcpHeader.dest = dest.sin_port;
	tcpHeader.seq = seq_num;
	tcpHeader.doff = 5;
	tcpHeader.syn = 1;
	tcpHeader.window = htons(4096);

	char buf[4];
	char tcp_packet[65535];
	_constructPacket(&tcpHeader, buf, NULL, 0, tcp_packet);

	return ip->sender(tcp_packet, 20);
}


void TCP::sender_thread()
{
	while (1) {

		pthread_mutex_lock(&mutex);

		if (sender_closed && sender_queue.empty())
		{
			pthread_mutex_unlock(&mutex);
			return;}

		if (!sender_closed && sender_queue.empty())
		{pthread_cond_wait(&send_when_queue_empty, &mutex);}

		if (sender_closed && sender_queue.empty())
		{	pthread_mutex_unlock(&mutex);
			return;}

		pair<char*, int> next_in_queue =
				sender_queue.front();
		sender_queue.pop_front();

		if (next_in_queue.second > 1000)
		{	pair<char*, int> temp;
			temp.first = (char*) malloc(next_in_queue.second - 1000 + 1000);
			temp.second = next_in_queue.second - 1000;
			memcpy(temp.first, next_in_queue.first + 1000, next_in_queue.second - 1000);
			sender_queue.push_front(temp);
			next_in_queue.second = 1000;}

		sender_tmp = next_in_queue.first;
		sender_tmp_len = next_in_queue.second;

		struct tcphdr tcpHeader;
		memset(&tcpHeader, 0, sizeof(struct tcphdr));
		tcpHeader.source = source.sin_port;
		tcpHeader.dest = dest.sin_port;
		tcpHeader.seq = seq_num;
		tcpHeader.ack_seq = ack_num;
		tcpHeader.doff = 5;
		tcpHeader.ack = 1;
		tcpHeader.window = htons(4096);

		char buffer[4];
		char tcp_packet[65535];
		_constructPacket(&tcpHeader, buffer, sender_tmp, sender_tmp_len, tcp_packet);

		ip->sender(tcp_packet, 20 + sender_tmp_len);

		wait_for_ack_start = time(NULL);
		pthread_cond_wait(&wait_for_ack, &mutex);

		pthread_mutex_unlock(&mutex);
	}
}


void TCP::receiver_thread()
{
	while (1)
	{	pthread_mutex_lock(&mutex);
		if (receiver_closed && sender_closed)
		{	pthread_mutex_unlock(&mutex);
			return;}

		if (sender_tmp != NULL && time(NULL) - wait_for_ack_start > 15)
		{	pair<char*, int> next(sender_tmp, sender_tmp_len);
			sender_queue.push_front(next);
			sender_tmp = NULL;
			sender_tmp_len = 0;
			pthread_cond_signal(&wait_for_ack);
			pthread_mutex_unlock(&mutex);
			continue;}

		char tcp_packet[65535];
		pthread_mutex_unlock(&mutex);
		int error = ip->receive(tcp_packet, 65535);
		pthread_mutex_lock(&mutex);

		if (error == 1)
		{	pthread_mutex_unlock(&mutex);

			continue;}

		else if (error < 0)
		{	fprintf(stderr, "cannot receive from socket\n");
			exit(1);}

		struct tcphdr* tcpHeader;
		struct iphdr* ipHeader;

		ipHeader = (struct iphdr*) tcp_packet;
		tcpHeader = (struct tcphdr*) (tcp_packet + ipHeader->ihl * 4);

		if (tcpHeader->rst)
		{	fprintf(stderr, "FATAL: connection reset by remote host\n");
			exit(1);}

		if (tcpHeader->ack)
		{if (sender_tmp)
			{if (ntohl(seq_num) + sender_tmp_len == ntohl(tcpHeader->ack_seq))
				{	seq_num = tcpHeader->ack_seq;
					free (sender_tmp);
					sender_tmp = NULL;
					sender_tmp_len = 0;
					pthread_cond_signal(&wait_for_ack);
				}}}

		int data_length = ntohs(ipHeader->tot_len) - ipHeader->ihl * 4 - tcpHeader->doff * 4;
		if (data_length)
		{if (ntohl(ack_num) == ntohl(tcpHeader->seq))
			{	char* data = (char*) malloc(data_length + 1000);
				memcpy(data, tcp_packet + ipHeader->ihl * 4 + tcpHeader->doff * 4, data_length);

				pair<char*, int> new_received_data(data, data_length);
				receiver_queue.push_back(new_received_data);
				pthread_cond_signal(&receive_when_queue_empty);

				struct tcphdr tcpHeader_sender;

				memset(&tcpHeader_sender, 0, sizeof(struct tcphdr));
				tcpHeader_sender.source = source.sin_port;
				tcpHeader_sender.dest = dest.sin_port;
				tcpHeader_sender.seq = htonl(ntohl(seq_num) + sender_tmp_len);
				tcpHeader_sender.ack_seq = htonl(ntohl(tcpHeader->seq) + data_length);
				tcpHeader_sender.doff = 5;
				tcpHeader_sender.ack = 1;
				tcpHeader_sender.window = htons(4096);
				ack_num = tcpHeader_sender.ack_seq;

				char buf[4];
				_constructPacket(&tcpHeader_sender, buf, NULL, 0, tcp_packet);

				if(ip->sender(tcp_packet, 20) < 0)
				{	fprintf(stderr, "FATAL: failed to send data\n");
					exit(1);}
			}

		else
			{	pthread_mutex_unlock(&mutex);
				continue;}
		}

		if (tcpHeader->fin)
		{
			struct tcphdr tcpHeader_sender;
			memset(&tcpHeader_sender, 0, sizeof(struct tcphdr));
			tcpHeader_sender.source = source.sin_port;
			tcpHeader_sender.dest = dest.sin_port;
			tcpHeader_sender.seq = htonl(ntohl(seq_num) + sender_tmp_len);
			tcpHeader_sender.ack_seq = htonl(ntohl(tcpHeader->seq) + 1);
			tcpHeader_sender.doff = 5;
			tcpHeader_sender.ack = 1;
			tcpHeader_sender.window = htons(4096);
			ack_num = tcpHeader_sender.ack_seq;

			char buf[4];
			_constructPacket(&tcpHeader_sender, buf, NULL, 0, tcp_packet);

			if(ip->sender(tcp_packet, 20) < 0)
			{	fprintf(stderr, "error: failed to send data\n");
				exit(1);}

			receiver_closed = 1;

			pthread_cond_signal(&receive_when_queue_empty);}

		pthread_mutex_unlock(&mutex);
		usleep(1001);}}

int TCP::closeConnection()
{
	sender_closed = 1;
	pthread_mutex_lock(&mutex);

	pthread_cond_signal(&send_when_queue_empty);

	int size = sender_queue.size();
	while (size != 0)
	{	pthread_mutex_unlock(&mutex);

		pthread_mutex_lock(&mutex);
		size = sender_queue.size();}

	struct tcphdr tcpHeader;
	memset(&tcpHeader, 0, sizeof(struct tcphdr));
	tcpHeader.source = source.sin_port;
	tcpHeader.dest = dest.sin_port;
	tcpHeader.seq = seq_num;
	tcpHeader.doff = 5;
	tcpHeader.fin = 1;
	tcpHeader.window = htons(4096);

	char buffer[4];
	char tcp_packet[65535];
	_constructPacket(&tcpHeader, buffer, NULL, 0, tcp_packet);

	int error = ip->sender(tcp_packet, 20);

	int total_time = time(NULL);
	while (!receiver_closed || !sender_closed)
	{	if (time(NULL) - total_time > 15)
		{	break;	}

			pthread_mutex_unlock(&mutex);
			pthread_mutex_lock(&mutex);
	}

	pthread_mutex_unlock(&mutex);

	return error;}

int TCP::receive(char* buf, int len)
{
	pthread_mutex_lock(&mutex);

	if (receiver_closed && receiver_queue.empty())
	{
		pthread_mutex_unlock(&mutex);
		return -1;}

	if (!receiver_closed && receiver_queue.empty())
	{pthread_cond_wait(&receive_when_queue_empty, &mutex);}

	if (receiver_closed && receiver_queue.empty())
	{
		pthread_mutex_unlock(&mutex);
		return -1;}

	pair<char*, int> new_data_received = receiver_queue.front();

	receiver_queue.pop_front();

	int data_length = 0;

	if (len < new_data_received.second)
	{
		memcpy(buf, new_data_received.first, len);
		data_length = len;
		char* tmp = (char*)malloc(new_data_received.second - len + 1000);
		memcpy(tmp, new_data_received.first + len, new_data_received.second - len);
		free(new_data_received.first);
		new_data_received.first = tmp;
		receiver_queue.push_front(new_data_received);
	}

	else
	{	memcpy(buf, new_data_received.first, new_data_received.second);
		data_length = new_data_received.second;
		free(new_data_received.first);
	}

	pthread_mutex_unlock(&mutex);
	return data_length;
}

