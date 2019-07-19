#include "Errores.h"

bool Errores::saltarErrores;
OutData_Parametros* Errores::outData;

void Errores::generarCabeceraError(OutData_Parametros * node, int numero_error, Errores::ERROR_TYPE tipo)
{


	std::string error = "ERROR";

	if (tipo == Errores::TY_WARNING)
		error = "WARNING";
	else if (tipo == Errores::TY_INFO)
		error = "INFO";
	else if (tipo == Errores::TY_ERROR)
		error = "ERROR";
	else error = "DESCONOCIDO";

	if (node == NULL)
	{
		std::cout << error << " " << numero_error << ": " << " Runtime Error: ";
		return;
	}


	int linea = node->linea;
	int offset = node->offset;

	std::string ruta = "";

	if (node->fichero != NULL)
	{
		if (node->fichero->ruta != "")
			ruta = "<" + node->fichero->ruta + "> ";
	}


	std::cout << error << " " << numero_error << ": " << ruta << "[" << linea << ":" << offset << "] ";
}

void Errores::generarError(Errores::NUM_ERRORES error, OutData_Parametros * node, std::string value, std::string value2)
{
	if (Errores::saltarErrores)
		return;

	Errores::ERROR_TYPE tipo = Errores::TY_ERROR;

	switch (error)
	{

	case Errores::ERROR_MAIN_NO_ENCONTRADO:
		std::cout << "ERROR: 10. ";
		std::cout << "No se ha encontrado la función de inicio del programa <main>.\n";
		break;
		
	case Errores::ERROR_CRITICO:
		Errores::generarCabeceraError(node, 11, tipo);
		std::cout << "Se ha producido un error CRÍTICO, la ejecución del interprete se ha detenido.\n";
		break;

	case Errores::ERROR_FUNCION_PARAMETRO_OPERACION_INVALIDA:
		Errores::generarCabeceraError(node, 4001, tipo);
		std::cout << "Se ha introducido un parámetro inválido en la función " << value << ". \n";
		break;

	case Errores::ERROR_VARIABLE_NO_EXISTE:
		Errores::generarCabeceraError(node, 2001, tipo);
		std::cout << "La variable " << value << " no existe. \n";
		break;

	case Errores::ERROR_CONVERSION_VARIABLE_INT:
		Errores::generarCabeceraError(node, 2004, tipo);
		std::cout << "No se puede realizar la conversión de la expresión, el valor asignado debe poder convertirse a un entero. \n";
		break;

	case Errores::ERROR_CONVERSION_VARIABLE_REAL:
		Errores::generarCabeceraError(node, 2004, tipo);
		std::cout << "No se puede realizar la conversión de la expresión, el valor asignado debe poder convertirse a un real. \n";
		break;

	case Errores::ERROR_CONVERSION_VARIABLE_LONG:
		Errores::generarCabeceraError(node, 2004, tipo);
		std::cout << "No se puede realizar la conversión de la expresión, el valor asignado debe poder convertirse a un lint. \n";
		break;

	case Errores::ERROR_CONVERSION_VARIABLE_BOOL:
		Errores::generarCabeceraError(node, 2004, tipo);
		std::cout << "No se puede realizar la conversión de la expresión, el valor asignado debe poder convertirse a un booleano. \n";
		break;

	case Errores::ERROR_CONVERSION_VARIABLE_STRING:
		Errores::generarCabeceraError(node, 2004, tipo);
		std::cout << "No se puede realizar la conversión de la expresión, el valor asignado debe poder convertirse a un string. \n";
		break;

	case Errores::ERROR_COMPARACION_INT_STRING:
		Errores::generarCabeceraError(node, 2005, tipo);
		std::cout << "Se está intentando comparar un entero con un string. \n";
		break;

	case Errores::ERROR_COMPARACION_REAL_STRING:
		Errores::generarCabeceraError(node, 2005, tipo);
		std::cout << "Se está intentando comparar un real con un string. \n";
		break;

	case Errores::ERROR_COMPARACION_BOOL_STRING:
		Errores::generarCabeceraError(node, 2005, tipo);
		std::cout << "Se está intentando comparar un booleano con un string. \n";
		break;

	case Errores::ERROR_COMPARACION_DESCONOCIDO:
		Errores::generarCabeceraError(node, 2006, tipo);
		std::cout << "No se esperaba poder comparar los elementos indicados. \n";
		break;

	case Errores::ERROR_DE_SINTAXIS:
		Errores::generarCabeceraError(node, 1001, tipo);
		std::cout << "Error de sintaxis. Se ha producido un error en la sintaxis, " << value << " no es una operación reconocida o no fue cerrada correctamente. \n";
		break;

	case Errores::ERROR_INICIALIZACION_VARIABLE:
		Errores::generarCabeceraError(node, 2002, tipo);
		std::cout << "Error al intentar inicializar la variable '" << value << "'. \n";
		break;

	case Errores::ERROR_DESC_DECLARACION_VARIABLE:
		Errores::generarCabeceraError(node, 2003, tipo);
		std::cout << "Error desconocido al declarar una variable. \n";
		break;

	case Errores::ERROR_FUNCION_NO_RECONOCIDA:
		Errores::generarCabeceraError(node, 4002, tipo);
		std::cout << "La función '" << value << "' no se reconoce, es posible que no fuera declarada o que no coincidan los parámetros establecidos. \n";
		break;

	case Errores::ERROR_OPERACION_INVALIDA_VOID:
		Errores::generarCabeceraError(node, 2007, tipo);
		std::cout << "Imposible realizar la operación, uno de los valores es un 'void'. Los valores vacios no pueden tratarse como una operación matemática. \n";
		break;

	case Errores::ERROR_MOD_SOLO_ENTERO:
		Errores::generarCabeceraError(node, 2008, tipo);
		std::cout << "El valor de un módulo debe ser siempre entero. \n";
		break;

	case Errores::ERROR_MATH_STRING:
		Errores::generarCabeceraError(node, 2009, tipo);
		std::cout << "El simbolo '"<< value << "' no se puede aplicar a una operación con cadena de caracteres. \n";
		break;

	case Errores::ERROR_OPERACION_INVALIDA_NULL:
		Errores::generarCabeceraError(node, 2010, tipo);
		std::cout << "Operación inválida. Un miembro es nulo o no ha sido inicializado. \n";
		break;

	case Errores::ERROR_EXPRESION_NO_CONVERTIBLE_BOOL:
		Errores::generarCabeceraError(node, 2011, tipo);
		std::cout << "La expresión debe tener un tipo bool o una expresión compatible convertible. \n";
		break;

	case Errores::ERROR_MATH_MULT_STRING:
		Errores::generarCabeceraError(node, 2012, tipo);
		std::cout << "No se puede multiplicar un string con un " << value << ". \n";
		break;

	case Errores::ERROR_MATH_MINUS_STRING:
		Errores::generarCabeceraError(node, 2013, tipo);
		std::cout << "No se puede restar un string con un " << value << ". \n";
		break;

	case Errores::ERROR_FUNCION_ENTRADA_DECLARADA:
		Errores::generarCabeceraError(node, 4003, tipo);
		std::cout << "La entrada '" << value << "' de la función '" << value2 << "' ya fue declarada. \n ";
		break;

	case Errores::ERROR_FUNCION_ERROR_OPERACIONES_ENTRADA:
		Errores::generarCabeceraError(node, 4004, tipo);
		std::cout << "Se encontró un error operacional en la función '" << value << "' por lo que no pudo ser declarada. \n ";
		break;

	case Errores::ERROR_INCREMENTO_VARIABLE_DESCONOCIDA:
		Errores::generarCabeceraError(node, 2007, tipo);
		std::cout << "Se está intentando incrementar la variable '" << value << "', que nunca ha sido declarada. \n ";
		break;


	case Errores::ERROR_DECREMENTO_VARIABLE_DESCONOCIDA:
		Errores::generarCabeceraError(node, 2007, tipo);
		std::cout << "Se está intentando decrementar la variable '" << value << "', que nunca ha sido declarada. \n ";
		break;

	case Errores::ERROR_INC_DEC_VARIABLE_INVALIDA:
		Errores::generarCabeceraError(node, 2008, tipo);
		std::cout << "Se está intentando incrementar o decrementar la variable '" << value << "', pero no es de un tipo adecuado. Debe tener valor númerico. \n ";
		break;
	case Errores::ERROR_FUNCION_IMPOSIBLE_RECUPERAR_FECHA:
		Errores::generarCabeceraError(node, 4005, tipo);
		std::cout << "Error desconocido. No se ha podido obtener la fecha actual. \n ";
		break;

	case Errores::ERROR_CONVERSION_DESCONOCIDA:
		Errores::generarCabeceraError(node, 2014, tipo);
		std::cout << "Se ha producido un error al intentar convertir un valor en otro desconocido. \n ";
		break;


	case Errores::ERROR_OPERACION_DESCONOCIDA:
		Errores::generarCabeceraError(node, 2015, tipo);
		std::cout << "Se ha producido un error desconocido al intentar realizar una operación. \n ";
		break;

	case Errores::ERROR_OPERACION_INVALIDA:
		Errores::generarCabeceraError(node, 2016, tipo);
		std::cout << "Se ha producido un error al intentar realizar una operación, puede que esta sea inválida. \n ";
		break;

	case Errores::ERROR_OPERADOR_INVALIDO:
		Errores::generarCabeceraError(node, 2017, tipo);
		std::cout << "No se ha podido aplicar el operador" << value << ". Es posible que el tipo de elemento no permita el uso del mismo. \n ";
		break;

		
		
		
	}
}

void Errores::generarWarning(Errores::NUM_WARNING error, OutData_Parametros * node, std::string value, std::string value2)
{
	Errores::ERROR_TYPE tipo = Errores::TY_WARNING;

	switch (error)
	{
	case Errores::WARNING_VARIABLE_YA_DECLARADA:
		Errores::generarCabeceraError(node, 2001, tipo);
		std::cout << "La variable '" << value << "' ya fue declarada previamente. \n";
		break;
	case Errores::WARNING_FUNCION_VALOR_DEVUELTO_VOID:
		Errores::generarCabeceraError(node, 2002, tipo);
		std::cout << "Se ha elegido una ruta en la función que no devuelve ningún valor, pero la función " << value << " requiere de un valor de salida. \n ";
		break;

	}
}