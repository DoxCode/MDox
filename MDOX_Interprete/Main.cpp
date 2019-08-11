/**

Compilado bajo UBUNTU:   g++ *.cpp -std=gnu++11 -lstdc++


Rodrigo de Miguel Fernández. Usal.
**/

#include "Main.h"

int main(int argument_count, char * argument_list[])
{
	setlocale(LC_ALL, "");


	if (argument_count >= 2)
	{
		std::string ruta = argument_list[1];

		Parser parser = Parser();

		//Desde el parser, accedemos al tokenizer, desde el mismo podremos generarlo a través del fichero.
		bool correcto = parser.tokenizer.GenerarTokenizerDesdeFichero(ruta);

		if (!correcto)
		{
			return 0;
		}

		Core::Start();

		Parser_Identificador* vId = new Parser_Identificador("main");
		std::vector<Value> entradas;
		Valor_Funcion vf = Valor_Funcion(vId);
		parser.valores_funciones.emplace_back(&vf);

		Interprete * interprete = new Interprete();
		if (!interprete->CargarDatos(&parser))
		{
			return 0;
		}

		interprete->ExecFuncion(&vf, entradas);

		return 0;
	}

	std::cout << "Bienvenido a MDOX, version " << VERSION << "(" << ESTADO << ")  \n";
	std::cout << "Copyright (c) 2018-2019 Rodrigo de Miguel Fernández. Universidad de Salamanca. \n";
	std::cout << "MDOX es un software gratuito y SIN GARANTIAS, eres libre de distribuirlo. \n";
	//std::cout << "RUTA: " << getAbsolutePathFromRelative("MDOX_Interprete.ilk") << "\n";

	Core::Start();
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
		std::cout << "\nInterprete> ";
		std::string ibs;
		getline(std::cin, ibs);

		if (ibs.empty())
			continue;

		if (ComandoInterprete(ibs))
		{
			salida = Comandos(ibs, interprete);
			continue;
		}

		std::vector<Linea*> lines;


		if (ibs == "<?")
		{	
			int i = 0;
			while (true)
			{
				i++;
				std::cout << "   ";
				getline(std::cin, ibs);

				if (ibs == "?>")
					break;

				lines.push_back(new Linea(i, ibs));
			}
		}
		else lines.push_back(new Linea(1, ibs));

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





