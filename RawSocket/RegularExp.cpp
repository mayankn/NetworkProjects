#include <cstdio>
#include "RegularExp.h"


RegularExp::~RegularExp()
{}

string RegularExp::getString(string target_str, int& posn,int flg, int substr)
{
	int error = regcomp
			(&regexp, patrn_str.c_str(), flg);

	posn = -1;

	if (error != 0)
	{
		char buffer[4096];
		regerror(error, &regexp, buffer, 4096);

		fprintf(stderr, "error in regexp: %s\n", buffer);

		regfree(&regexp);
		return 0;}

	error = regexec
			(&regexp, target_str.c_str(), substr, matches_list, 0);

	if (error == REG_NOMATCH)
	{regfree(&regexp);
		return "";}

	else if (error == 0)
	{posn = matches_list[0].rm_eo;
		regfree(&regexp);
		return
				target_str.substr(matches_list[substr - 1].rm_so, matches_list[substr - 1].rm_eo - matches_list[substr - 1].rm_so);}

	else
	{char buffer1[4096];
		regerror(error, &regexp, buffer1, 4096);
		fprintf(stderr, "error in regexp: %s\n", buffer1);
		regfree(&regexp);
		return "";}}

RegularExp::RegularExp(string pattern)
{this->patrn_str = pattern;}

int RegularExp::getMatch(string target_str, int flg)
{
	int error = regcomp
			(&regexp, patrn_str.c_str(), flg);

	if (error != 0)
	{	char buffer[4096];
		regerror(error, &regexp, buffer, 4096);
		
		fprintf(stderr, "error in regexp: %s\n", buffer);
		
		regfree(&regexp);
		
		return 0;}

	error = regexec
			(&regexp, target_str.c_str(), 0, NULL, 0);
	
	if (error == REG_NOMATCH)
	{	regfree(&regexp);
		return 0;}
	
	else if (error == 0)
	{regfree(&regexp);
		return 1;}
	
	else {	char buffer1[4096];

		regerror(error, &regexp, buffer1, 4096);

		fprintf(stderr, "error in regexp: %s\n", buffer1);

		regfree(&regexp);
		return -1;}}
