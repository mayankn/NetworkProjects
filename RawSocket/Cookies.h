#ifndef COOKIES_H
#define COOKIES_H 1


#include <string>
#include <iostream>
#include <utility>
#include <ctime>

using namespace std;

typedef pair < string, string > key_value_string;

class Cookies
{	private:int expiry;
	string key_str;
	string path_str;
	string domainName;
	string val_str;
	public:~Cookies();
	Cookies(int expiry, key_value_string keyValue, string path_str, string domainName);
	Cookies(int expiry, string key_str, string val_str, string path_str, string domainName);
	int cookieBelongsTo(string domainName, string path_str);
	key_value_string getKeyValue();
	int expireCookie();
}
;

#endif
