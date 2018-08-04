/**

Compilado bajo UBUNTU:   g++ Comandos.cpp Main.cpp Funciones.cpp -std=gnu++11 -lstdc++


Rodrigo de Miguel Fern�ndez. Usal.
**/

#include "Main.h"



int main()
{
	std::cout << "Bienvenido a MDOX, version " << VERSION << "(" << ESTADO << ") \n";
	std::cout << "Copyright (c) 2018-2019 Rodrigo de Miguel Fernandez. Universidad de Salamanca. \n";
	std::cout << "MDOX es un software gratuito y SIN GARANTIAS, eres libre de distribuirlo. \n\n";

	Interprete();
	return 0;
}

/* Interpreta lo escrito en la linea de comandos.
Teniendo en cuenta de que a parte de c�digo, tambi�n puede haber comandos.
*/
void Interprete()
{
	bool salida = false;

	while (!salida)
	{
		std::cout << "Interprete> ";
		std::string ibs;
		getline(std::cin, ibs);

		if (ibs.empty())
			continue;

		if (ComandoInterprete(ibs))
		{
			salida = Comandos(ibs);
			continue;
		}



		//Lexing("",ibs);

	}
}

/* Devuelve verdadero si el usuario ha escrito un comando, es decir, una cadena que empieza por ':'*/
bool ComandoInterprete(std::string getWr)
{
	if (getWr.at(0) == ':')
		return true;
	return false;
}





