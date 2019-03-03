#ifndef CORE_H
#define CORE_H

#include "Estructuras.h"

class Core_Function
{
public:
	std::string nombre;
	std::vector<tipos_parametros> entradas;
	tipos_parametros salida;

	Core_Function(std::string a, std::vector<tipos_parametros> b, tipos_parametros c) : nombre(a), entradas(b), salida(c) {}

	~Core_Function()
	{};
};

class Core
{
public:
	static std::vector<Core_Function*> core_functions;
	static void Start();

};




#endif