/**

Compilado bajo UBUNTU:   g++ Comandos.cpp Main.cpp Funciones.cpp -std=gnu++11 -lstdc++


Rodrigo de Miguel Fernández. Usal.
**/

#include "Main.h"



int main()
{
	std::cout << "Bienvenido a MDOX, version " << VERSION << "(" << ESTADO << ") \n";
	std::cout << "Copyright (c) 2018-2019 Rodrigo de Miguel Fernandez. Universidad de Salamanca. \n";
	std::cout << "MDOX es un software gratuito y SIN GARANTIAS, eres libre de distribuirlo. \n\n";

	Iniciar_Interprete();
	return 0;
}

/* Interpreta lo escrito en la linea de comandos.
Teniendo en cuenta de que a parte de código, también puede haber comandos.
*/
void Iniciar_Interprete()
{
	Interprete * interprete = new Interprete();


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
			salida = Comandos(ibs, interprete);
			continue;
		}

		std::string lines = "";

		if (ibs == "<?")
		{	
			while (true)
			{
				std::cout << "   ";
				getline(std::cin, ibs);

				if (ibs == "?>")
					break;

				lines += ibs;
			}
		}
		else lines = ibs;

		//Comando adicional, tomado como una SENTENCIA.
		Parser parser = Parser();
		parser.tokenizer.generarTokens(lines);
		interprete->Interpretar(&parser);

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





