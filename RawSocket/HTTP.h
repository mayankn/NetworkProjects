#include <map>
#include <iostream>
#include <cstdlib>
#include <vector>
#include "Sock.h"
#include "ManageCookie.h"
#include <cerrno>
#include "Cookies.h"
#include <cstdio>
using namespace std;


#define RES_PH 1
#define RESPONSE_INIT 0
#define RESPONSE_NORMAL_END 5
#define RESPONSE_CHUNK_END 3
#define RESPONSE_NORMAL_PB 4
#define RESPONSE_CHUNK_PB 2


typedef struct response_header
{
	int chunk_len;
	int content_len;
	int response_code;
	int close_num;
	string loc;

}response_header_t;

class HTTP
{
	private:class ManageCookie* mc;
	class Sock* sock;
	string url_domain;

	int url_port;
	string response_body;

	string redirect_loc;
	string response_header;

	int chunked;

	void _closeSocket();
	int _openSocket(string domain, int port);
	int _isValidURL(string url);
	int _getPortNum(string url);
	string _getDomainName(string url);
	int _sendGetHeader();
	int _sendPostRequest(vector<key_value_string>);
	int _handleSetCookieVal(string value);
	string _URIString(string url);
	int _handleResponse();
	string _getFullResponse();
	string _getResponse();

	response_header_t _parseHeader(string head);
	vector<string> _breakEachLine(string input);

	public:class ManageCookie* getManageCookie();
	~HTTP();
	HTTP(class ManageCookie* manageCookie);
	HTTP();

	string getResp(string url);
	string postResp(string url, vector< key_value_string > fields);
	string getDomainName(string url);
	int isValidURL(string url);
	string parseChunks(string in);
	string url_path;
	int toDec(char x);
	int isHexa(char x);
};
