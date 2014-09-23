#include "ManageCookie.h"
using namespace std;



vector< key_value_string > ManageCookie::getCookiesList
(string domainName, string path_str)
{
	cookies_list::iterator iter = l_cookies.begin();


	vector <key_value_string> retVal;
	for (; iter != l_cookies.end();)
	{	if ((*iter).expireCookie())
		{	iter = l_cookies.erase(iter);
			continue;}

		if ((*iter).cookieBelongsTo(domainName, path_str))
		{retVal.push_back((*iter).getKeyValue());}

		++ iter;
	}return retVal;}

ManageCookie::ManageCookie()
{}

int ManageCookie::setCookieValue(int expiry, key_value_string keyValue, string path_str, string domainName)
{
	class Cookies temp(expiry,
			keyValue, path_str, domainName);
	l_cookies.push_back(temp);}
