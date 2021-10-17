#include <iostream>
#include <string>
#include <fstream>
#include <cmath>
#include <stdlib.h>
#include "HttpPost.h"
#include <sstream>

#pragma warning(disable:4996)

size_t receive_data(const char *buffer, size_t size, size_t nmemb, void *getmsg)
{
	((string *)getmsg)->append(buffer);
	return nmemb;
}

size_t receive_file(const char *buffer, size_t size, size_t nmemb, FILE *file)
{
	return fwrite(buffer, size, nmemb, file);
}

void HTMLParseToJson(Json &Root, string Area, ParseForm *Form,int FormCount)
{
	string Fhead;
	for (int i = 0; i < FormCount; i++)
	{
		Fhead.assign(Form[i].head);
		for (int pos = 0; (pos = Area.find(Form[i].head, pos)) != -1;)
			Root[Form[i].name].push_back(GetNextBlock(Area, Form[i].head, Form[i].end, pos));
	}
}

//Please CHECK THE va_list of c_func before USE TRUE c_func, and MAKE SURE the PARALIST, or there will be a TERRIBLE MISTAKE
int GetURL(string HURL, string Part, void *receive_func, void *receive_stream, int Timeout, int(*connect_func)(CURL*, va_list), ...)
{
	string sURL;
	CURL *curl = curl_easy_init();
	if (!curl)
		return CURLE_FAILED_INIT;
	CURLcode res;
	sURL.append(HURL).append(Part);
	curl_easy_setopt(curl, CURLOPT_URL, sURL);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	if (receive_func&&receive_stream)
	{
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, receive_func);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, receive_stream);
	}
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, Timeout);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, Timeout);
	if (connect_func)
	{
		va_list ap;
		va_start(ap, connect_func);
		res = (CURLcode)connect_func(curl, ap);
	}
	else
		res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	return res;
}

//After one time, the headpos will point at endpos or Area.length() or -1;
string GetNextBlock(string Area, string head, string end, int &headpos, int hroopc , int eroopc )
{
	if (headpos == -1)
		headpos = 0;
	for (int i = 0; i < hroopc; i++)
		if ((headpos = Area.find(head, headpos)) == -1)
			return "";
		else
			headpos += head.length();
	int endpos = headpos - 1;
	if (!end.length())
		endpos = -1;
	else
		for (int i = 0; i < eroopc; i++)
			if ((endpos = Area.find(end, endpos + 1)) == -1)
				break;
	int cutpos = headpos;
	if (endpos == -1)
		return Area.substr(cutpos, headpos = Area.length());
	else
	{
		headpos = endpos;
		return Area.substr(cutpos, endpos - cutpos);
	}
}

string ParseError(int Errno)
{
	string Res = "";
	if (Errno < FormError)
	{
		return Res.append(curl_easy_strerror((CURLcode)Errno)).append("\n");
	}
	else
	{
		char No[4] = { 0 };
		Res.append("格式化错误，代码：");
		_itoa_s(Errno - FormError, No, 10);
		Res.append(No).append("\n");
		return Res;
	}
}

int PostForm(CURL *curl, curl_forms *CForms)
{
	curl_httppost *head = NULL;
	curl_httppost *last = NULL;
	if (CForms != NULL)
	{
		CURLFORMcode FM = curl_formadd(&head, &last, CURLFORM_ARRAY, CForms, CURLFORM_END);
		if (CURL_FORMADD_OK != FM)
			return FormError + FM;
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, head);
	}
	return(curl_easy_perform(curl));
}

//PARALIST: CURL* curl, va_list(curl_forms **CForms, int CFCount); (CFCount is the number of curl_forms*)
int PostForm(CURL *curl, va_list ap)
{
	curl_forms **CForms = va_arg(ap, curl_forms **);
	int CFCount = va_arg(ap, int);
	CURLcode res;
	curl_httppost *head = NULL;
	curl_httppost *last = NULL;
	for (int i = 0; i < CFCount; i++)
	{
		CURLFORMcode FM = curl_formadd(&head, &last, CURLFORM_ARRAY, CForms[i], CURLFORM_END);
		if (CURL_FORMADD_OK != FM)
			return FormError + FM;
	}
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	curl_easy_setopt(curl, CURLOPT_HTTPPOST, head);
	res = curl_easy_perform(curl);
	curl_formfree(head);
	return res;
}

//PARALIST: CURL* curl, va_list(const char *HTTPHeaders, ..., const char *HTTPHeadersEnd = "", const char *PostFields)
int PostHTTPHeaders(CURL *curl, va_list ap)
{
	string Contents = "";
	CURLcode Cres;
	curl_slist *http_headers = NULL;
	for (;;)
	{
		Contents.assign(va_arg(ap, const char *));
		if (!Contents.length())
			break;
		http_headers = curl_slist_append(http_headers, Contents.c_str());
	}
	string post = va_arg(ap, const char *);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, http_headers);
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.c_str());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, post.length());
	//curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/71.0.3578.98 Safari/537.36 Vivaldi/2.2.1388.37");
	Cres = curl_easy_perform(curl);
	curl_slist_free_all(http_headers);
	return Cres;
}

const char *encodeURIComponent(const string &str)
{
	unsigned int len = str.length();
	if (!len)
		return str.c_str();
	stringstream output;
	for (unsigned int i = 0; i < len; i++)
	{
		switch (str[i])
		{
		case '-':
		case '_':
		case '.':
		case '!':
		case '~':
		case '*':
		case '\'':
		case '(':
		case ')':
			output << str[i];
			break;
		default:
			if ((str[i] >= '0'&&str[i] <= '9') ||
				(str[i] >= 'a'&&str[i] <= 'z') ||
				(str[i] >= 'A'&&str[i] <= 'Z'))
				output << str[i];
			else
			{
				char temp[4] = { 0 };
				sprintf_s(temp, 4, "%%%02X", (unsigned int)(unsigned char)str[i]);
				output << temp;
			}
		}
	}
	return output.str().c_str();
}
