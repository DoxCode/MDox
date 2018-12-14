#ifndef COMANDOS_H
#define COMANDOS_H

#include <map>
#include <string>
#include <tuple>
#include <iostream>
#include <fstream>
#include <list>
#include <vector>

#include "Parser.h"
#include "Funciones.h"
#include "Tokenizer.h"
#include "Interprete.h"

// Lista de comandos existentes en MDOX, as� como errores
enum comandos_enum
{
	NULO,
	AYUDA,
	CARGAR_FICHERO,
	CARGAR_FICHERO_MALFORMACION,
	SALIR,
};

bool Comandos(std::string comando, Interprete * interprete);
std::tuple<bool, comandos_enum> comandoAyuda(std::string comando);
void limpiarComando(std::string head, std::string& comando);
void EscribirAyudaComando(comandos_enum com);
void EscribirAyudaComando(comandos_enum com);
void AccionesComandosIndividuales(comandos_enum e_com);
#endif