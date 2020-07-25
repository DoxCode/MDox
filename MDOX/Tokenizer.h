#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdio>

#include "Funciones.h"
//#include "Estructuras.h"


bool str_compare(std::string str, int itr, std::string busq);

class Token {
public:
	std::string token;
	int linea;
	int char_horizontal;
	bool firstNewLine;
	Token(int a, int b, std::string c, bool& nw) : linea(a), char_horizontal(b), token(c), firstNewLine(nw) { nw = false; }
};


class Linea {
public:
	std::string val;
	int linea;
	Linea(int a, std::string b) : linea(a), val(b) {}
};


class Fichero
{
public:
	std::string ruta = "";
	std::string nombre = "";

	Fichero(std::string rel)
	{
		ruta = getAbsolutePathFromRelative(rel);
		nombre = getFileNameAndExt(ruta);
	}
};

class Tokenizer {
public:
	std::vector<Token*> tokens;
	Token * token_actual;
	Fichero * fichero; //Se liberaría al finalizar el interprete/compilador

	bool end_file = false;

	Token * getToken(int& inx)
	{ 
		if (inx >= tokens.size()) 
			return NULL; 
		int temp = inx; 
		inx++; 
		token_actual = tokens[temp];
		return token_actual;
	}

	bool isCloseToken(int& inx)
	{
		Token* t = getToken(inx);
		if (t)
		{
			if (t->token == ";")
				return true;
			else if (t->firstNewLine)
			{
				inx--;
				return true;
			}
			else if (tokens.size() == inx)
				return true;
		}
		
		return true;
	}

	bool isNextNewLine(int& inx)
	{
		Token* t = getToken(inx);
		if (t)
		{
			if (t->firstNewLine)
			{
				inx--;
				return true;
			}
			else if (tokens.size() == inx)
				return true;
		}
		inx--;
		return false;
	}


	//Solo checkea, devuelve el indice actual.
	bool checkCloseToken(int& inx)
	{
		Token* t = getToken(inx);
		if (t)
			if (t->token == ";")
			{
				inx--;
				return true;
			}
			else if (t->firstNewLine)
			{
				inx--;
				return true;
			}
			else if (tokens.size() == inx)
			{
				inx--;
				return true;
			}
		inx--;
		return false;
	}

	//Devuelve únicamente tokens con valor, ignora \n
	std::string getTokenValue(int& inx)
	{
		Token * t = getToken(inx);
		if (t)
			return t->token;
		else
		{
			end_file = true;
			return "";
		}
	}


	//Obtiene el primer token que no sea un token de salida
	std::string getFirstNotCloseToken(int& inx)
	{
		while (true)
		{
			if (getTokenValue(inx) == ";")
			{
				continue;
			}

			inx--;
			return getToken(inx)->token;
		}
	}

	void agregarToken(Token * a) { tokens.push_back(a); };
	bool GenerarTokenizerDesdeFichero(std::string ruta);
	void generarTokens(std::vector<Linea*> str);

	~Tokenizer()
	{
		for (std::vector<Token*>::iterator it = tokens.begin(); it != tokens.end(); ++it)
		{
			delete (*it);
		}
	};
};



#endif
