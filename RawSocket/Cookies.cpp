#include "Cookies.h"

using namespace std;

Cookies::~Cookies()
{}

Cookies::Cookies(int expiry, string key_str,
		string val_str, string path_str, string domainName)
{
	key_value_string temp(key_str, val_str);

	Cookies(expiry, temp, path_str, domainName);
}

int Cookies::cookieBelongsTo(string domainName, string path_str)
{
	if (this->domainName.compare(domainName) != 0)
	{return 0;}

	if (path_str.find(this->path_str) == 0
			&& path_str.find(this->path_str)
			!= string::npos
		&& (this->path_str[this->path_str.length() - 1] == '/'
			|| path_str.length() == this->path_str.length()
			|| path_str[this->path_str.length()] == '/'))
	{return 1;}

	else
	{return 0;}

}

Cookies::Cookies(int expiry, key_value_string keyValue, string path_str, string domainName)
{	if (expiry < 0) this->expiry = 0;

	else this->expiry = time(NULL) + expiry;


	this->val_str = keyValue.second;

	this->key_str = keyValue.first;


	this->path_str = path_str;

	this->domainName = domainName;

	if (path_str.length() != 1 &&
			path_str[path_str.length() - 1] == '/')
	{	this->path_str =
			path_str.substr(0, path_str.length() - 1);}

}

int Cookies::expireCookie()
{
	time_t secs;

	secs = time(NULL);

	if (expiry == 0)
	{return 0;}

	if (secs > expiry)
	{return 1;}

	else
	{return 0;}

}


key_value_string Cookies::getKeyValue()
{
	key_value_string retVal(key_str, val_str);

	return retVal;}
