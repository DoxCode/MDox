#ifndef MAIN_H
#define MAIN_H

//#define _GLIBCXX_USE_CXX11_ABI 0
#define VERSION 0.2
#define ESTADO "BETA"

#include <iostream>
#include <string>
#include <vector>
#include "Comandos.h"
#include "Funciones.h"
#include "Estructuras.h"

//Prototipos
void Interprete();
bool ComandoInterprete(std::string getWr);
#endif