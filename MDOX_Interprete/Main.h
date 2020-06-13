#ifndef MAIN_H
#define MAIN_H

//#define _GLIBCXX_USE_CXX11_ABI 0
#define VERSION "0.88"
#define ESTADO "BETA"

#include <iostream>
#include <string>
#include <vector>

#include "Comandos.h"
//#include "../MDOX/Funciones.h"
//#include "Interprete.h"





//Prototipos
void Iniciar_Interprete();
bool ComandoInterprete(std::string getWr);
#endif