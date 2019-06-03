#include "Recursos.h"
#include <string>
using namespace std;

Recursos::Recursos()
{
}


Recursos::~Recursos()
{
}

void Recursos::enviarPost(string info) {
	CURL* curl;
	CURLcode res;

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "http://165.22.199.177/"); // IP Servidor
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, info.c_str()); // se envia el string info mediante HTTP POST al servidor
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(info.c_str()));

		// realiza HTTP POST
		res = curl_easy_perform(curl);
		// comprueba que la petición haya salido bien
		if (res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));

		curl_easy_cleanup(curl);
	}
}


char* Recursos::dupcat(const char* s1, ...) {
	// Función para concatenar char* facilmente
	int len;
	char* p, * q, * sn;
	va_list ap;

	len = strlen(s1);
	va_start(ap, s1);
	while (1) {
		sn = va_arg(ap, char*);
		if (!sn)
			break;
		len += strlen(sn);
	}
	va_end(ap);

	p = new char[len + 1];
	strcpy(p, s1);
	q = p + strlen(p);

	va_start(ap, s1);
	while (1) {
		sn = va_arg(ap, char*);
		if (!sn)
			break;
		strcpy(q, sn);
		q += strlen(q);
	}
	va_end(ap);

	return p;
}
