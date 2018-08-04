
#include "Parser.h"
#include "Funciones.h"

estructuraParametro* Parser::detectarParametro(std::string& linea, Flags_lectura flags)
{

	RetirarValoresNoUtiles(linea);

	// Si la linea está vacia, devuelve empty.
	if (linea.length() == 0)
		return new estructuraParametro(PARAM_EMPTY);

	estructuraParametro * v = this->getDeclarativo(linea);
	if (v)
		return v;
	//delete v;

	v = this->getString(linea);
	if (v)
		return v;
//	delete v;

	v = this->getVariable(linea);
	if (v)
		return v;
//	delete v;

	return new estructuraParametro(PARAM_UNKNOWN);
}
// ######################## FUNCIONES  ######################## 
// Cambiar funciones en su respectivo cpp, para que sea global con traduccion y lexing.
//
/*
estructuraFuncion * class_lector::getFuncionParamDeclarativo(std::string& linea)
{
	// Posibilidades de declaración:
	//   - <PARAM_DECLARATIVO> [Nombre] ;
	//   - <PARAM_DECLARATIVO> [Nombre] = <OPERACION> ;
	//   - [Nombre] = <OPERACION> ;

	estructuraFuncion * func = new estructuraFuncion(FUNC_DECLARATIVOS);

	//Constituida por :: <PARAMETRO> NOMBRE (<LISTA PARAMETROS>)


}
*/


/*estructuraFuncion * class_lector::getDeclararFuncion(std::string& linea)
{
	estructuraFuncInterna * func = new estructuraFuncInterna(FUNC_DECLARATIVOS);

	//Constituida por :: <PARAMETRO> NOMBRE (<LISTA PARAMETROS>)


}
*/




// ######################## PARAMETROS  ######################## 
// Cambiar funciones de parametros en su respectivo cpp, para que sea global con traduccion y lexing.
//
//
//     Se usa estructuraParMultiple:
//
//     (PARAM_DECLARATIVOS) -> PARAM_X
//
//     Puede generar una lista de conjuntos:
//
//     (PARAM_DECLARATIVO) -> TUPLA -> (PARAM_X, PARAM_X, TUPLA -> (PARAM_X, PARAM_X))
//
//

estructuraParametro * Parser::getDeclarativo(std::string& linea)
{
	estructuraParSingle * param = new estructuraParSingle(PARAM_DECLARATIVOS);

	/*if (nextStringLine(linea, "bool"))
	{
		param->addParametro(PARAM_BOOL);
		linea = linea.erase(0, 4);
	}
	else if (nextStringLine(linea, "int16"))
	{
		param->addParametro(PARAM_INT16);
		linea = linea.erase(0, 5);
	}
	else if (nextStringLine(linea, "int64"))
	{
		param->addParametro(PARAM_INT64);
		linea = linea.erase(0, 5);
	}
	else if (nextStringLine(linea, "int"))
	{
		param->addParametro(PARAM_INT);
		linea = linea.erase(0, 3);
	}
	else if (nextStringLine(linea, "string"))
	{
		param->addParametro(PARAM_STRING);
		linea = linea.erase(0, 6);
	}
	//tupla:   (<DECLARATIVO>, <DECLARATIVO> ... )
	else if (nextStringLine(linea, "("))
	{
		std::string temp = linea;
		temp = temp.erase(0, 1); // Borramos (

		estructuraParMultiple * tupla = new estructuraParMultiple(PARAM_TUPLA);
		int t_index = index;
		bool separador = true;

		while (true)
		{
			estructuraParametro* par2 = detectarParametro(temp, FLAG_LECT_NONE);

			if (par2->tipo == PARAM_DECLARATIVOS)
			{
				//Se agrega como estructura general, pues esta estructura podría ser otra tupla
				// O una tupla dentro de otra, de esta manera ganamos en recursividad.
				tupla->addGeneralParam(par2);
				separador = false;
			}
			else if (par2->tipo == PARAM_UNKNOWN)
			{
				if (nextStringLine(temp, ","))
				{
					separador = true;
					temp = temp.erase(0, 1);
					delete par2;
					continue;
				}
				else if (nextStringLine(temp, ")"))
				{
					delete par2;

					if (separador)
					{
						deletePtr(param);
						break;
					}

					param->addGeneralParam(tupla);
					temp = temp.erase(0, 1);
					index = t_index;
					linea = temp;
					return param;
				}
				else
				{
					delete tupla;
					delete par2;
					deletePtr(param);
					break;
				}
			}
			// Si termina la linea, pasamos a la siguiente, guardando como indice e incrementando el temporal.
			else if (par2->tipo == PARAM_EMPTY)
			{
				delete par2;
				t_index++;
				std::tuple<bool, std::string> nws = obtenerLinea(t_index);
				if (std::get<0>(nws))
				{
					temp = std::get<1>(nws);
					continue;
				}
			}
		}
	}
	else
	{
		deletePtr(param);
	}
	*/
	return param;
}

estructuraParametro * Parser::getString(std::string& linea)
{
	if (linea.size() < 1)
		return NULL;

	if (linea.at(0) != '"')
		return NULL;

	std::string temp = linea;

	temp = temp.erase(0, 1);
	int x = temp.find("\"");

	if (x >= 0)
	{
		std::string value = temp.substr(0, x);
		temp = temp.erase(0, x + 1);
		linea = temp;
		return new estructuraParValor(PARAM_STRING, value);
	}

	return NULL;
}


estructuraParametro * Parser::getVariable(std::string& linea)
{
	if (linea.size() < 1)
		return NULL;

	std::string temp = linea;
	std::string value = "";

	int alpha = 0, digit = 0;

	for (char& c : temp) {
		
		if (c == ' ' || c == '=' || c == ';' || c == ',')
			break;

		if (isalpha(c))
			alpha++;
		else if (isdigit(c))
			digit++;
		else 
			break;

		value += c;
	}

	if (alpha == 0)
		return NULL;

	if (temp.size() == 0)
		return NULL;

	temp = temp.erase(0, value.size());
	linea = temp;

	return new estructuraParValor(PARAM_VARIABLES, value);
}