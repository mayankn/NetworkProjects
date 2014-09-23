#include "RawSocket.h"
#include <iostream>
#include "HTTP.h"
using namespace std;

int main(int argc, char** argv)
{
	/* if incorrect number of command line arguments given, print usage instructions*/
		if (argc<2) {	
			fprintf(stderr, "Usage: %s url\n", argv[0]);

			return 1;
		}

	HTTP* http = new HTTP();
	string url_name(argv[1]);

	/* create a new http instance with a raw socket and get the given URL */
	string get_response_str =
			http->getResp(url_name);

	/* extract the file name from the given URL */
	string html_flName =
			http->url_path;

		if (html_flName == "/")
		{html_flName = "index.html";}

		else if (html_flName[html_flName.length() - 1] == '/')
		{html_flName = "index.html";}

		else
		{html_flName = html_flName.substr(html_flName.find_last_of('/') + 1);}

	delete http;

	FILE* filePtr =
			fopen(html_flName.c_str(), "w+");

		if (filePtr == NULL)
		{fprintf(stderr, "cannot open file:%s\n", html_flName.c_str());

			return 1;}

	/* write contents to a file */
	fputs(get_response_str.c_str(), filePtr);
	fclose(filePtr);

	return 0;
}
