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
	 Parser_Valor * Parser::getValor(int& local_index);
	 Parser_Operacion * Parser::getOperacion(int& local_index);
	 Parser_Math * Parser::getMath(int& index);
	 Parser_Parametro * Parser::getParametro(int& local_index);
	 Parser_Igualdad * Parser::getIgualdad(int& local_index);
	 Parser_Condicional * Parser::getCondicional(int& local_index);
	 Condicional_Agregada * Parser::getCondAgregada(int& local_index);
	 Condicional_Agregada_Operacional* Parser::getCondAgregadaOperacional(int& local_index);
	 Parser_Sentencia * Parser::getSentencia(int& local_index);
	 Parser_Funcion * Parser::getFuncion(int& local_index);
	 Funcion_ValorEntrada * Parser::getEntrada(int& local_index);
};


#endif