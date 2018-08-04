#ifndef COMANDOS_H
#define COMANDOS_H

#include <map>
#include <string>
#include <tuple>
#include <iostream>
#include <fstream>
#include <list>
#include <vector>

#include "Tester.h"
#include "Parser.h"

// Lista de comandos existentes en MDOX, así como errores
enum comandos_enum
{
	NULO,
	AYUDA,
	CARGAR_FICHERO,
	CARGAR_FICHERO_MALFORMACION,
	SALIR,
};

bool Comandos(std::string comando);
std::tuple<bool, comandos_enum> comandoAyuda(std::string comando);
void limpiarComando(std::string head, std::string& comando);
void EscribirAyudaComando(comandos_enum com);
void EscribirAyudaComando(comandos_enum com);
void AccionesComandosIndividuales(comandos_enum e_com);
#endif