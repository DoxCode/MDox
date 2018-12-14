#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdio>
#include "Funciones.h"

bool str_compare(std::string str, int itr, std::string busq);

class Token {
public:
	std::string token;
	int linea;
	int char_horizontal;
	Token(int a, int b, std::string c) : linea(a), char_horizontal(b), token(c) {}
};


class Linea {
public:
	std::string val;
	int linea;
	Linea(int a, std::string b) : linea(a), val(b) {}
};

class Tokenizer {
public:
	std::string fichero = "<interprete>";
	std::vector<Token*> tokens;
	Token * token_actual;

	Token * getToken(int& inx)
	{ 
		if (inx >= tokens.size()) 
			return NULL; 
		int temp = inx; 
		inx++; 
		token_actual = tokens.at(temp);
		return token_actual;
	}

	std::string getTokenValue(int& inx)
	{
		Token * t = getToken(inx);
		if (t)
			return t->token;
		else return "<Cierre inválido>";
	}



	void agregarToken(Token * a) { tokens.push_back(a); };
	bool GenerarTokenizerDesdeFichero(std::string ruta);
	void generarTokens(std::vector<Linea*> str);

	Tokenizer() {}

	~Tokenizer()
	{
		for (std::vector<Token*>::iterator it = tokens.begin(); it != tokens.end(); ++it)
		{
			delete (*it);
		}
	};
};



#endif
