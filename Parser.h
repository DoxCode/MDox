#ifndef PARSER_H
#define PARSER_H

#include "Estructuras.h"
#include "Tokenizer.h"
#include <iostream>


class Parser {
protected:
public:
	Tokenizer tokenizer;

	Parser() { }

	//Parametros a overload
	 estructuraParametro* detectarParametro(std::string& linea, Flags_lectura flags);

	//Funciones detectoras de parametros
	estructuraParametro* getDeclarativo(std::string& linea);
	estructuraParametro* getString(std::string& linea);
	estructuraParametro* getVariable(std::string& linea);

};


#endif