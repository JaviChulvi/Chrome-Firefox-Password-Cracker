#define CURL_STATICLIB
#include "curl/curl.h"
#include <string>
#include <iostream>

using namespace std;

#ifdef _DEBUG
#  pragma comment (lib, "curl/libcurl_a_debug.lib")
#else
#  pragma comment (lib, "curl/libcurl_a.lib")
#endif // DEBUG

#pragma once
class Recursos
{
public:
	Recursos();
	~Recursos();

	void enviarPost(string info);
	char* dupcat(const char* s1, ...);
};

