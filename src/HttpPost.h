#pragma once
#include "openssl/md5.h"
#include "sdk/third_party/json.hpp"
#include "curl/curl.h"

#define TIMEOUT 30
#define FormError 100
#define FormCount(x,type) sizeof(x)/sizeof(type) 
#define HTTPHeaderEnd ""

using namespace std;
using Json = nlohmann::json;

static string WebRespons;

struct ParseForm
{
	const char *head;
	const char *end;
	const char *name;
	ParseForm(const char *first, const char *second = "\"", const char *third = NULL)
	{
		head = first;
		if (second[0] == 0)
			end = "\"";
		else
			end = second;
		if (third == NULL)
			name = head;
		else
		{
			if (third[0] == 0)
				name = head;
			else
				name = third;
		}
	};
};

struct BlockForm
{
public:
	const char *AreaStart;
	const char *BlockStart;
	const char *BlockEnd;
	int Hroop;
	int Eroop;
	BlockForm(const char *first = "", const char *second = "", const char *third = "", int start = 1, int end = 1)
	{
		AreaStart = first;
		BlockStart = second;
		BlockEnd = third;
		Hroop = start;
		Eroop = end;
	}
};

size_t receive_data(const char *buffer, size_t size, size_t nmemb, void *getmsg);	//Html request function

size_t receive_file(const char *buffer, size_t size, size_t nmemb, FILE* file);	//Download files

void HTMLParseToJson(Json &Root, string Area, ParseForm *Form,int FormCount);

int GetURL(string HURL, string Part, void *receive_func = NULL, void *receive_stream = NULL, int Timeout = TIMEOUT, int(*connect_func)(CURL*, va_list) = NULL, ...);

string GetNextBlock(string Area, string head, string end, int &headpos, int hroopc = 1, int eroopc = 1);

string ParseError(int Errno);

int PostForm(CURL *curl, va_list ap);

int PostHTTPHeaders(CURL *curl, va_list ap);

const char *encodeURIComponent(const string &str);