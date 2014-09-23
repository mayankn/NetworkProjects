#include <regex.h>

#include <iostream>
using namespace std;

class RegularExp
{
	private:regex_t regexp;
	string patrn_str;
	regmatch_t matches_list[1024];

	public:RegularExp(string pattern);
	int getMatch(string target, int flag = 0);
	~RegularExp();
	string getString(string target_str, int& posn, int flg, int substr);

};
