#ifndef COMANDOS_H
#define COMANDOS_H



//#include "../MDOX/Parser.h"
//#include "../MDOX/Funciones.h"
//#include "../MDOX/Tokenizer.h"
#include "Interprete.h"

// Lista de comandos existentes en MDOX, así como errores
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