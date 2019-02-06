#ifndef ERRORES_H
#define ERRORES_H

#include <string>
#include <vector>
#include <list>
#include <tuple>
#include <iostream>
#include "Parser.h"

class Errores
{
	public:
		enum ERROR_TYPE {
			TY_ERROR,
			TY_WARNING,
			TY_INFO,
		};

		enum NUM_ERRORES {
			ERROR_DE_SINTAXIS, // Error 1001

			ERROR_VARIABLE_NO_EXISTE, // Error 2001
			ERROR_INICIALIZACION_VARIABLE, // Error 2002
			ERROR_DESC_DECLARACION_VARIABLE, // Error 2003
			ERROR_CONVERSION_VARIABLE_INT,	// Error 2004
			ERROR_CONVERSION_VARIABLE_REAL,	// Error 2004
			ERROR_CONVERSION_VARIABLE_BOOL,	// Error 2004
			ERROR_CONVERSION_VARIABLE_STRING,	// Error 2004
			ERROR_COMPARACION_INT_STRING, //Error 2005
			ERROR_COMPARACION_REAL_STRING, //Error 2005
			ERROR_COMPARACION_BOOL_STRING, //Error 2005
			ERROR_COMPARACION_DESCONOCIDO,	// Error 2006

			//Operaciones matemáticas
			ERROR_OPERACION_INVALIDA_VOID, //Error 2007
			ERROR_MOD_SOLO_ENTERO, //Error 2008
			ERROR_MATH_STRING, //Error 2009
			ERROR_OPERACION_INVALIDA_NULL, //Error 2010

			ERROR_EXPRESION_NO_CONVERTIBLE_BOOL, //Error 2011

			ERROR_FUNCION_PARAMETRO_OPERACION_INVALIDA, // Error 4001
			ERROR_FUNCION_NO_RECONOCIDA, //Error 4002
			ERROR_FUNCION_ENTRADA_DECLARADA, //Error 4003
			ERROR_FUNCION_ERROR_OPERACIONES_ENTRADA, //Error 4004
		
		};

		enum NUM_WARNING {
			WARNING_VARIABLE_YA_DECLARADA, // Warning 2001
		};

     static void generarError(Errores::NUM_ERRORES error, OutData_Parametros * node, std::string value = "", std::string value2 = "");
	 static void generarWarning(Errores::NUM_WARNING error, OutData_Parametros * node, std::string value = "", std::string value2 = "");

	private:
	 static void generarCabeceraError(OutData_Parametros * node, int numero_error, Errores::ERROR_TYPE tipo);

};


#endif