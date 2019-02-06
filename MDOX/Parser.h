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

	//Funciones detectoras de parametros
	 Parser_Literal * getLiteral(int& local_index);
	 Parser_Declarativo * getDeclarativo(int& local_index);
	 Parser_Identificador * getIdentificador(int& local_index);
	 Parser_Valor * getValor(int& local_index);
	 Parser_Operacion * getOperacion(int& local_index);
	 Parser_Math * getMath(int& index);
	 Parser_Parametro * getParametro(int& local_index);
	 Parser_Igualdad * getIgualdad(int& local_index);
	 Parser_Condicional * getCondicional(int& local_index);
	 Condicional_Agregada * getCondAgregada(int& local_index);
	 Condicional_Agregada_Operacional* getCondAgregadaOperacional(int& local_index);
	 Parser_Sentencia * getSentencia(int& local_index);
	 Parser_Funcion * getFuncion(int& local_index);
	 Funcion_ValorEntrada * getEntrada(int& local_index);
};


#endif