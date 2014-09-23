#include <list>
#include <vector>
#include <iostream>
#include "Cookies.h"

using namespace std;

typedef list<class Cookies> cookies_list;

class ManageCookie
{
	private:cookies_list l_cookies;

	public:ManageCookie();

	vector< key_value_string > getCookiesList(string domainName, string path_str);

	int setCookieValue
	(int expiry, key_value_string keyValue, string path_str, string domainName);
}
;
