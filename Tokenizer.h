#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "Funciones.h"

bool str_compare(std::string str, int itr, std::string busq);

class Tokenizer {
private:
	int index = 0;
public:
	std::vector<std::string> tokens;
	std::vector<int> num_Lines;

	void cambiarIndice(int newIndice) { index = newIndice; };
	void incrementarIndice() { index++; };
	void agregarToken(std::string a) { tokens.push_back(a); };
	void generarTokens(std::string vc);
	bool GenerarTokenizerDesdeFichero(std::string ruta);

	Tokenizer() {}
};

#endif
