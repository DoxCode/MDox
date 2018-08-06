#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "Funciones.h"

bool str_compare(std::string str, int itr, std::string busq);

class Tokenizer {
public:
	std::vector<std::string> tokens;
	std::vector<int> num_Lines;

	std::string getToken(int& inx) { if (inx >= tokens.size()) return NULL; int temp = inx; inx++; return tokens.at(temp); }
	void agregarToken(std::string a) { tokens.push_back(a); };
	void generarTokens(std::string vc);
	bool GenerarTokenizerDesdeFichero(std::string ruta);

	Tokenizer() {}
};

#endif
