
#include "RegularExp.h"
#include "HTTP.h"
#include <cstdio>
#include <cstdlib>
#include <regex.h>
#include <cstring>

using namespace std;


class ManageCookie* HTTP::getManageCookie()
{return mc;}

HTTP::HTTP(class ManageCookie* mc)
{
	this->mc = mc;
	sock = NULL;
}


HTTP::HTTP()
{
	mc = new ManageCookie();
	sock = NULL;
}

HTTP::~HTTP()
{
	if (sock != 0)
	{delete sock;}

}


void HTTP::_closeSocket()
{
	delete sock;
	url_domain = "";
	url_port = 80;
	sock = NULL;}


int HTTP::_openSocket(string domainName, int portNum)
{
	if (sock != NULL)
	{_closeSocket();}
	sock = new Sock(domainName.c_str(), portNum);
	if (sock->createSocket() < 0)
	{
		fprintf(stderr, "Connection failed to remote server: %s\n", domainName.c_str());
		delete sock;
		sock = NULL;
		return -1;}

	url_domain = domainName;
	url_port = portNum;

	return 0;}


string HTTP::getResp(string url)
{
	if (!_isValidURL(url))
	{
		fprintf(stderr, "Illegal URL given: %s\n", url.c_str());
		return "";}

	string domain = _getDomainName(url);
	int port = _getPortNum(url);

	if (port != url_port || domain.compare(url_domain) != 0)
	{if (_openSocket(domain, port) < 0)
		{return "";}}

	url_path = _URIString(url);
	string first_line = "GET " + _URIString(url) + " HTTP/1.1\r\n";

	int debug = sock->writeStr(first_line.c_str(), first_line.length());

	if (_sendGetHeader() < 0)
	{
		fprintf(stderr, "Can't send HTTP header\n");
		_closeSocket();
		return "";}

	int err = _handleResponse();
	if (err == -2)
	{return _getResponse();}

	else if (err == -3)
	{
		_closeSocket();
		return getResp(url);}

	else if (err == -4)
	{return _getResponse();}

	else if (err < 0)
	{
		_closeSocket();
		return _getResponse();}

	return _getResponse();
}


string HTTP::_getDomainName(string url_str)
{
	url_str = url_str.substr(7);

	int slash_pos = url_str.find("/");

	if (slash_pos == string::npos)
	{
		url_str += "/";
		slash_pos = url_str.length() - 1;
	}

	string retVal =  url_str.substr(0, slash_pos);

	if (retVal.find(":") != string::npos)
	{return retVal.substr(0, retVal.find(":"));}

	else
	{return retVal;}

}

int HTTP::_getPortNum(string url_str)
{
	url_str = url_str.substr(7);

	int slash_pos = url_str.find("/");
	if (slash_pos == string::npos)
	{
		url_str += "/";
		slash_pos = url_str.length() - 1;
	}

	int i = slash_pos - 1;

	while (i >= 0)
	{if (url_str[i] == ':')
		{break;}

	i --;
	}

	if (i < 0)
	{return 80;}

	else
	{
		int retVal = 0;
		int current = 1;
		int posn = slash_pos - 1;
		while (posn > i)
		{
			retVal += current * (url_str[posn] - '0');
			posn --;
			current *= 10;
		}

		return retVal;
	}
}

string HTTP::postResp(string url_str, vector < key_value_string > reponseFields)
{
	if (!_isValidURL(url_str))
	{
		fprintf(stderr, "Invalid URL: %s\n", url_str.c_str());
		return "";}

	string domain = _getDomainName(url_str);
	int port = _getPortNum(url_str);

	if (port != url_port || domain.compare(url_domain) != 0)
	{if (_openSocket(domain, port) < 0)
		{return "";}}

	url_path = _URIString(url_str);
	string init_line = "POST " + _URIString(url_str) + " HTTP/1.1\r\n";

	int debug = sock->writeStr(init_line.c_str(), init_line.length());

	if (_sendPostRequest(reponseFields) < 0)
	{fprintf(stderr, "Can't send HTTP post request\n");
		_closeSocket();
		return "";}

	int error = _handleResponse();
	if (error == -2)
	{return getResp(url_str);}

	else if (error == -3)
	{_closeSocket();
		return getResp(url_str);}

	else if (error == -4)
	{return getResp(redirect_loc);}

	else if (error < 0)
	{fprintf(stderr, "Bad Response %s\n", _getFullResponse().c_str());
		_closeSocket();
		return "";}

	return _getResponse();

}

string HTTP::_URIString(string url_str)
{
	url_str = url_str.substr(7);

	int slash_pos = url_str.find("/");
	if (slash_pos == string::npos)
	{return "/";}

	else
	{return url_str.substr(slash_pos);}

}



int HTTP::isValidURL(string url_str)
{return _isValidURL(url_str);}

int HTTP::_isValidURL(string url_str)
{
	class RegularExp url_regex("http://([0-9a-z\\-]+\\.)+[0-9a-z]+(:[0-9]+)?/?.*");
	if (url_regex.getMatch(url_str, REG_ICASE | REG_EXTENDED | REG_NOSUB) > 0)
	{return 1;}

	else
	{return 0;}

}

string HTTP::getDomainName(string url_str)
{return _getDomainName(url_str);}

int HTTP::_sendPostRequest(vector< key_value_string > postFields)
{
	string postMsg;
	char cport[16];
	sprintf(cport, "%d", url_port);

	if (url_port != 80) postMsg = "Host: " + url_domain + ":" + cport + "\r\n";

	else postMsg = "Host: " + url_domain + "\r\n";

	postMsg += "Connection: keep-alive\r\n";

	postMsg += "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n";

	postMsg += "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_8_2) AppleWebKit/537.17 (KHTML, like Gecko) Chrome/24.0.1312.57 Safari/537.17\r\n";

	postMsg += "Accept-Encoding: identity;q=1.0, *;q=0\r\n";

	vector <key_value_string> cookies = mc->getCookiesList(url_domain, url_path);

	vector <key_value_string>::iterator iter = cookies.begin();

	if (cookies.size() != 0)
	{
		postMsg += "Cookie: ";

		for (; iter != cookies.end(); ++ iter)
		{
			postMsg += (*iter).first + "=" + (*iter).second;
			if (iter + 1 != cookies.end()) postMsg += "; ";
		}

		postMsg += "\r\n";
	}

	string postData = "";

	for (iter = postFields.begin(); iter != postFields.end(); ++ iter)
	{if (iter != postFields.begin())
		{postData += "&";}

	postData += (*iter).first + "=" + (*iter).second;
	}

	char str_length[16];

	sprintf(str_length, "%lu", postData.length());

	string	string_length(str_length);
	postMsg += "Content-Length: " + string_length + "\r\n";

	postMsg += "\r\n";

	postMsg += postData + "\r\n";

	if (sock->writeStr(postMsg.c_str(), postMsg.length()) < postMsg.length())
	{
		fprintf(stderr, "remote server closed unexpectedly\n");
		return -1;}
}

int HTTP::_sendGetHeader()
{
	string getMsg;
	char cport[16];
	sprintf(cport, "%d", url_port);

	if (url_port != 80) getMsg = "Host: " + url_domain + ":" + cport + "\r\n";

	else getMsg = "Host: " + url_domain + "\r\n";

	getMsg += "Connection: keep-alive\r\n";

	getMsg += "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n";

	getMsg += "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_8_2) AppleWebKit/537.17 (KHTML, like Gecko) Chrome/24.0.1312.57 Safari/537.17\r\n";

	getMsg += "Accept-Encoding: identity;q=1.0, *;q=0\r\n";

	vector <key_value_string> cookies = mc->getCookiesList(url_domain, url_path);
	vector <key_value_string>::iterator iter = cookies.begin();

	if (cookies.size() != 0)
	{
		getMsg += "Cookie: ";

		for (; iter != cookies.end(); ++ iter)
		{
			getMsg += (*iter).first + "=" + (*iter).second;
			if (iter + 1 != cookies.end())
				getMsg += "; ";}

		getMsg += "\r\n";
	}

	getMsg += "\r\n";

	if (int i = sock->writeStr(getMsg.c_str(), getMsg.length()) < getMsg.length())
	{
		fprintf(stderr, "Remote server closed connection unexpectedly : %d\n", i);
		return -1;}

}





int HTTP::_handleSetCookieVal(string val_str)
{
	int expiry = -1;
	string domain_name = url_domain;
	string path_str = "/";
	string key_str = "";
	string value;
	while (1)
	{	while(val_str[0] == ' ')
			{val_str = val_str.substr(1);}

		int posn = val_str.find(";");
		string temp = val_str.substr(0, posn);
		val_str = val_str.substr(posn + 1);


		if (temp.find("expires=") == 0)
		{continue;}

		else if (temp.find("Max-Age=") == 0)
		{continue;}

		else if (temp.find("Path") == 0)
		{path_str = temp.substr(strlen("Path="));}

		else
		{
			if (key_str.compare("") != 0)
			{continue;}

			if (temp.find("=") != string::npos)
			{
				key_str = temp.substr(0, temp.find("="));
				value = temp.substr(temp.find("=") + 1);
			}

			else
			{key_str = temp;}
		}

		if (posn == string::npos)
		{break;}
	}

	key_value_string keyValue(key_str, value);

	mc->setCookieValue(expiry, keyValue, path_str, domain_name);
}

vector<string> HTTP::_breakEachLine(string input_str)
{
	vector<string> received_str;
	while(input_str.length() > 0)
	{	if (input_str.find("\r\n") == string::npos)
		{	received_str.push_back(input_str);
			break;}

		received_str.push_back(input_str.substr(0, input_str.find("\r\n")));
		input_str = input_str.substr(input_str.find("\r\n") + 2);}

	return received_str;
}


string HTTP::_getResponse()
{
	if (chunked) return parseChunks(response_body);
	
	else return response_body;
}

string HTTP::_getFullResponse()
{return response_header + response_body;}

int HTTP::_handleResponse()
{
	int response_status = RESPONSE_INIT;
	char buffer[65536];

	string head_str = "";
	string body_str = "";

	int chunks_left;

	response_header_t resHeader;

	while (response_status != RESPONSE_CHUNK_END &&
			response_status != RESPONSE_NORMAL_END)
	{
		int retVal = sock->readStr(buffer, 65535);
		if (retVal <= 0)
		{
			fprintf(stderr, "Connection closed by remote host: %d\n", retVal);
			return -3;}

		buffer[retVal] = '\0';

		if (response_status == RESPONSE_INIT)
		{
			int flg = 0;
			string temp(buffer);
			int posn = temp.find("\r\n\r\n");
			if (posn != string::npos)
			{
				head_str += temp.substr(0, posn);
				body_str += temp.substr(posn + 4);
				flg = 1;}

			else
			{head_str += temp;
				flg = 0;}

			if (!flg)
			{response_status = RESPONSE_INIT;}

			else
			{response_header = head_str;
				response_body = body_str;
				resHeader = _parseHeader(head_str);
				if (resHeader.response_code < 0)
				{_closeSocket();
					return -1;}

				if (resHeader.chunk_len)
				{	chunked = 1;
					response_status = RESPONSE_CHUNK_PB;}

				else
				{	chunked = 0;
					response_status = RESPONSE_NORMAL_PB;}

				if (response_status == RESPONSE_CHUNK_PB)
				{
					class RegularExp regexp("\r\n0( [^\r\n]*)*\r\n(.*\r\n)*\r\n");
					if (regexp.getMatch(body_str, REG_EXTENDED | REG_NOSUB))
					{response_status = RESPONSE_CHUNK_END;}
				}

				else if (response_status == RESPONSE_NORMAL_PB)
				{
					chunks_left = resHeader.content_len - body_str.length();
					if (chunks_left <= 0)
					{
						response_status = RESPONSE_NORMAL_END;}
				
				}}

		}
		
		else if (response_status == RESPONSE_CHUNK_PB)
		{
			string temp(buffer);
			
			body_str += temp;
			response_body = body_str;
			
			class RegularExp regexp("\r\n0( [^\r\n]*)*\r\n(.*\r\n)*\r\n");

			if (regexp.getMatch(response_body, REG_EXTENDED | REG_NOSUB))
			{response_status = RESPONSE_CHUNK_END;}
		}

		else if (response_status == RESPONSE_NORMAL_PB)
		{
			string temp(buffer);
			body_str += temp;
			response_body = body_str;
			chunks_left -= temp.length();
			if (chunks_left <= 0)
			{response_status = RESPONSE_NORMAL_END;}
		}}

	if (resHeader.close_num)
	{_closeSocket();}

	if (resHeader.response_code < 200)
	{return -1;}

	else if (resHeader.response_code < 300)
	{return 0;}

	else if (resHeader.response_code < 400)
	{
		redirect_loc = resHeader.loc;
		return -4;}

	else if (resHeader.response_code < 500)
	{return -1;}

	else
	{return -2;}

}

response_header_t HTTP::_parseHeader(string head)
{
	vector<string> tmp = _breakEachLine(head);
	vector<string>::iterator iter;

	response_header_t received_response;

	received_response.chunk_len = 0;
	received_response.loc = "";
	received_response.close_num = 0;
	received_response.content_len = 0;

	for(iter = tmp.begin(); iter != tmp.end(); ++ iter)
	{if (iter == tmp.begin())
		{	string line_str = (*iter);
			class RegularExp response_regex("HTTP/1.1 [0-9]+ .*");
			if (response_regex.getMatch(line_str, REG_EXTENDED | REG_NOSUB) <= 0)
			{
				received_response.response_code = -1;
				return received_response;}

			sscanf(line_str.c_str(), "HTTP/1.1 %d", &(received_response.response_code));
		}

		else
		{
			string line_str = (*iter);
			int posn = line_str.find(":");
			if (posn == string::npos)
			{
				received_response.response_code = -1;
				fprintf(stderr, "Bad header field: %s\n", line_str.c_str());
				return received_response;
			}

			string key_str = (*iter).substr(0, posn);
			string val = (*iter).substr(posn + 1);
			if (key_str.compare("Transfer-Encoding") == 0)
			{if (val.find("chunked") != string::npos)
				{received_response.chunk_len = 1;}
				else
				{received_response.chunk_len = 0;}
			}

			else if (key_str.compare("Content-Length") == 0)
			{
				while(val[0] == ' ')
				{val = val.substr(1);}

				int contentLen;
				sscanf(val.c_str(), "%d", &contentLen);
				received_response.content_len = contentLen;
			}

			else if (key_str.compare("Location") == 0)
			{
				while(val[0] == ' ')
				{
					val = val.substr(1);
				}	
				received_response.loc = val;
			}

			else if (key_str.compare("Connection") == 0)
			{
				if (val.find("close") != string::npos)
				{received_response.close_num = 1;}

				else
				{received_response.close_num = 0;}
			}}}

	return received_response;
}


string HTTP::parseChunks(string body_str)
{
	int posn = 0;
	string received_response = "";
	if (!isHexa(body_str[posn]))
	{fprintf(stderr, "Invalid chunks received_response\n");
		return body_str;}

	int chunks_left = 0;

	while (posn < body_str.length())
	{
		if (chunks_left == 0)
		{
			chunks_left = 0;
				while (posn < body_str.length()&&isHexa(body_str[posn]))
				{
					chunks_left *= 16;
					chunks_left += toDec(body_str[posn]);
					posn ++;
				}

			int temp = body_str.find("\r\n", posn);
			if (temp == string::npos)
			{fprintf(stderr, "error in parsing chunks\n");
				return body_str;}

			posn = temp + 2;

			if (chunks_left == 0)
			{return received_response;}
		}

		else
		{
			received_response += body_str[posn];
			posn ++;
			chunks_left --;
			if (chunks_left == 0)
			{posn += 2;}
		}

	}

	return received_response;
}



int HTTP::toDec(char c)
{
	if ((c >= '0') &&
			c <= '9')
	{return c - '0';}

	else if (c >= 'A' && c <= 'F')
	{return c - 'A' + 10;}

	else
	{return c - 'a' + 10;}

}

int HTTP::isHexa(char c)
{
	if ((c >= '0' && c <= '9') ||
			(c >='A' && c <='F') || (c >= 'a' && c <='f'))
	{return 1;}

	else
	{return 0;}

}
