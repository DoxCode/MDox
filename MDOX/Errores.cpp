#include "Errores.h"

bool Errores::saltarErrores;
OutData_Parametros* Errores::outData;

void Errores::generarCabeceraError(OutData_Parametros * node, int numero_error, Errores::ERROR_TYPE tipo, bool runtime)
{


	std::string error = "ERROR";

	if (tipo == Errores::TY_WARNING)
		error = "WARNING";
	else if (tipo == Errores::TY_INFO)
		error = "INFO";
	else if (tipo == Errores::TY_ERROR)
		error = "ERROR";
	else error = "DESCONOCIDO";

	if (node == NULL && runtime)
	{
		std::cout << error << " " << numero_error << ": " << " Runtime Error: ";
		return;
	}

	if (node == NULL && !runtime)
	{
		std::cout << error << " " << numero_error << ": ";
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
		std::cout << "No se ha encontrado la funci�n de inicio del programa <main>.";
		break;
		
	case Errores::ERROR_CRITICO:
		Errores::generarCabeceraError(node, 11, tipo);
		std::cout << "Se ha producido un error CR�TICO, la ejecuci�n del interprete se ha detenido.";
		break;

	case Errores::ERROR_INESPERADO:
		Errores::generarCabeceraError(node, 12, tipo);
		std::cout << "Se ha producido un error inesperado. "<<value;
		break;
		

	case Errores::ERROR_FUNCION_PARAMETRO_OPERACION_INVALIDA:
		Errores::generarCabeceraError(node, 4001, tipo);
		std::cout << "Se ha introducido un par�metro inv�lido en la funci�n " << value << ". ";
		break;

	case Errores::ERROR_VARIABLE_NO_EXISTE:
		Errores::generarCabeceraError(node, 2001, tipo);
		std::cout << "La variable " << value << " no existe. ";
		break;

	case Errores::ERROR_CONVERSION_VARIABLE_INT:
		Errores::generarCabeceraError(node, 2004, tipo);
		std::cout << "No se puede realizar la conversi�n de la expresi�n, el valor asignado debe poder convertirse a un entero. ";
		break;

	case Errores::ERROR_CONVERSION_VARIABLE_REAL:
		Errores::generarCabeceraError(node, 2004, tipo);
		std::cout << "No se puede realizar la conversi�n de la expresi�n, el valor asignado debe poder convertirse a un real. ";
		break;

	case Errores::ERROR_CONVERSION_VARIABLE_LONG:
		Errores::generarCabeceraError(node, 2004, tipo);
		std::cout << "No se puede realizar la conversi�n de la expresi�n, el valor asignado debe poder convertirse a un lint. ";
		break;

	case Errores::ERROR_CONVERSION_VARIABLE_BOOL:
		Errores::generarCabeceraError(node, 2004, tipo);
		std::cout << "No se puede realizar la conversi�n de la expresi�n, el valor asignado debe poder convertirse a un booleano. ";
		break;

	case Errores::ERROR_CONVERSION_VARIABLE_STRING:
		Errores::generarCabeceraError(node, 2004, tipo);
		std::cout << "No se puede realizar la conversi�n de la expresi�n, el valor asignado debe poder convertirse a un string. ";
		break;

	case Errores::ERROR_COMPARACION_INT_STRING:
		Errores::generarCabeceraError(node, 2005, tipo);
		std::cout << "Se est� intentando comparar un entero con un string. ";
		break;

	case Errores::ERROR_COMPARACION_REAL_STRING:
		Errores::generarCabeceraError(node, 2005, tipo);
		std::cout << "Se est� intentando comparar un real con un string. ";
		break;

	case Errores::ERROR_COMPARACION_BOOL_STRING:
		Errores::generarCabeceraError(node, 2005, tipo);
		std::cout << "Se est� intentando comparar un booleano con un string. ";
		break;

	case Errores::ERROR_COMPARACION_DESCONOCIDO:
		Errores::generarCabeceraError(node, 2006, tipo);
		std::cout << "No se esperaba poder comparar los elementos indicados. ";
		break;

	case Errores::ERROR_DE_SINTAXIS:
		Errores::generarCabeceraError(node, 1001, tipo);
		std::cout << "Error de sintaxis. Se ha producido un error en la sintaxis, " << value << " no es una operaci�n reconocida o no fue cerrada correctamente. ";
		break;

	case Errores::ERROR_INICIALIZACION_VARIABLE:
		Errores::generarCabeceraError(node, 2002, tipo);
		std::cout << "Error al intentar inicializar la variable '" << value << "'. ";
		break;

	case Errores::ERROR_DESC_DECLARACION_VARIABLE:
		Errores::generarCabeceraError(node, 2003, tipo);
		std::cout << "Error desconocido al declarar una variable. ";
		break;

	case Errores::ERROR_FUNCION_NO_RECONOCIDA:
		Errores::generarCabeceraError(node, 4002, tipo);
		std::cout << "La funci�n '" << value << "' no se reconoce, es posible que no fuera declarada o que no coincidan los par�metros establecidos. ";
		break;

	case Errores::ERROR_OPERACION_INVALIDA_VOID:
		Errores::generarCabeceraError(node, 2007, tipo);
		std::cout << "Imposible realizar la operaci�n, uno de los valores es un 'void'. Los valores vacios no pueden tratarse como una operaci�n matem�tica. ";
		break;

	case Errores::ERROR_MOD_SOLO_ENTERO:
		Errores::generarCabeceraError(node, 2008, tipo);
		std::cout << "El valor de un m�dulo debe ser siempre entero. ";
		break;

	case Errores::ERROR_MATH_STRING:
		Errores::generarCabeceraError(node, 2009, tipo);
		std::cout << "El simbolo '"<< value << "' no se puede aplicar a una operaci�n con cadena de caracteres. ";
		break;

	case Errores::ERROR_OPERACION_INVALIDA_NULL:
		Errores::generarCabeceraError(node, 2010, tipo);
		std::cout << "Operaci�n inv�lida. Un miembro es nulo o no ha sido inicializado. ";
		break;

	case Errores::ERROR_EXPRESION_NO_CONVERTIBLE_BOOL:
		Errores::generarCabeceraError(node, 2011, tipo);
		std::cout << "La expresi�n debe tener un tipo bool o una expresi�n compatible convertible. ";
		break;

	case Errores::ERROR_MATH_MULT_STRING:
		Errores::generarCabeceraError(node, 2012, tipo);
		std::cout << "No se puede multiplicar un string con un " << value << ". ";
		break;

	case Errores::ERROR_MATH_MINUS_STRING:
		Errores::generarCabeceraError(node, 2013, tipo);
		std::cout << "No se puede restar un string con un " << value << ". ";
		break;

	case Errores::ERROR_FUNCION_ENTRADA_DECLARADA:
		Errores::generarCabeceraError(node, 4003, tipo);
		std::cout << "La entrada '" << value << "' de la funci�n '" << value2 << "' ya fue declarada.  ";
		break;

	case Errores::ERROR_FUNCION_ERROR_OPERACIONES_ENTRADA:
		Errores::generarCabeceraError(node, 4004, tipo);
		std::cout << "Se encontr� un error operacional en la funci�n '" << value << "' por lo que no pudo ser declarada.  ";
		break;

	case Errores::ERROR_INCREMENTO_VARIABLE_DESCONOCIDA:
		Errores::generarCabeceraError(node, 2007, tipo);
		std::cout << "Se est� intentando incrementar la variable '" << value << "', que nunca ha sido declarada.  ";
		break;


	case Errores::ERROR_DECREMENTO_VARIABLE_DESCONOCIDA:
		Errores::generarCabeceraError(node, 2007, tipo);
		std::cout << "Se est� intentando decrementar la variable '" << value << "', que nunca ha sido declarada.  ";
		break;

	case Errores::ERROR_INC_DEC_VARIABLE_INVALIDA:
		Errores::generarCabeceraError(node, 2008, tipo);
		std::cout << "Se est� intentando incrementar o decrementar la variable '" << value << "', pero no es de un tipo adecuado. Debe tener valor n�merico.  ";
		break;
	case Errores::ERROR_FUNCION_IMPOSIBLE_RECUPERAR_FECHA:
		Errores::generarCabeceraError(node, 4005, tipo);
		std::cout << "Error desconocido. No se ha podido obtener la fecha actual.  ";
		break;

	case Errores::ERROR_FUNCION_NO_DECLARADA:
		Errores::generarCabeceraError(node, 4006, tipo);
		std::cout << "La funci�n con nombre '" << value << "' no ha sido declarada.  ";
		break;

	case Errores::ERROR_CONVERSION_DESCONOCIDA:
		Errores::generarCabeceraError(node, 2014, tipo);
		std::cout << "Se ha producido un error al intentar convertir un valor en otro desconocido.  ";
		break;


	case Errores::ERROR_OPERACION_DESCONOCIDA:
		Errores::generarCabeceraError(node, 2015, tipo);
		std::cout << "Se ha producido un error desconocido al intentar realizar una operaci�n.  ";
		break;

	case Errores::ERROR_OPERACION_INVALIDA:
		Errores::generarCabeceraError(node, 2016, tipo);
		std::cout << "Se ha producido un error al intentar realizar una operaci�n, puede que esta sea inv�lida.  ";
		break;

	case Errores::ERROR_OPERADOR_INVALIDO:
		Errores::generarCabeceraError(node, 2017, tipo);
		std::cout << "No se ha podido aplicar el operador " << value << ". Es posible que el tipo de elemento no permita el uso del mismo.  ";
		break;

	case Errores::ERROR_ASIGNACION_VALOR_VOID:
		Errores::generarCabeceraError(node, 2018, tipo);
		std::cout << "No se puede establecer una variable con un valor vacio si esta fue fijada como '" << value << "' con anterioridad.  ";
		break;

	case Errores::ERROR_OFFSET_INVALIDO:
		Errores::generarCabeceraError(node, 2019, tipo, false);
		std::cout << "El valor introducido como offset ('" << value << "') est� fuera del rango aceptado.  ";
		break;

	case Errores::ERROR_CLASE_CONSTRUCTOR_SINTAXIS:
		Errores::generarCabeceraError(node, 4500, tipo, false);
		std::cout << "Se ha producido un error en la sintaxis al intentar crear un constructor. ";
		break;

	case Errores::ERROR_CLASE_CONSTRUCTOR_ID_NOT_FOUND:
		Errores::generarCabeceraError(node, 4501, tipo, false);
		std::cout << "El identificador '" << value << "' del constructor no existe en la clase referenciada.";
		break;

	case Errores::ERROR_CLASE_CONSTRUCTOR_NOT_ID:
		Errores::generarCabeceraError(node, 4502, tipo, false);
		std::cout << "El constructor debe tener par�metros que sean ID's existentes en la clase.";
		break;

	case Errores::ERROR_CLASE_STATIC_IS_NOT_VAR_FUNCT:
		Errores::generarCabeceraError(node, 4503, tipo,false);
		std::cout << "El par�metro 'static' s�lamente se puede aplicar a variables o funciones de una clase.";
		break;

	case Errores::ERROR_CLASE_SINTAXIS:
		Errores::generarCabeceraError(node, 4504, tipo, false);
		std::cout << "Se ha producido un error en la sintaxis a la hora de la creaci�n de la clase '"<<value<<"'.";
		break;

	case Errores::ERROR_CLASE_VARIABLE_NO_VALIDA:
		Errores::generarCabeceraError(node, 4505, tipo, false);
		std::cout << "No se ha podido validar la operaci�n en la clase '"<< value <<"'. Una clase solamente permite operaciones que identifiquen variables.";
		break;

	case Errores::ERROR_CLASE_CONSTRUCTOR_NO_VALIDO:
		Errores::generarCabeceraError(node, 4506, tipo, false);
		std::cout << "El constructor usado para la clase '" << value << "' no ha sido definido.";
		break;

	case Errores::ERROR_CLASE_OPERADOR_INVALIDO:
		Errores::generarCabeceraError(node, 4507, tipo, false);
		std::cout << "El operador '" << value << "' no se reconoce. ";
		break;

	case Errores::ERROR_CLASE_SENTENCIA_INVALIDA:
		Errores::generarCabeceraError(node, 4508, tipo, false);
		std::cout << "El operador '" << value << "' no tiene una sentencia adecuada establecida. ";
		break;

	case Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO:
		Errores::generarCabeceraError(node, 4509, tipo, false);
		std::cout << "El operador '" << value << "' no ha sido declarado en la clase '"<<value2<<"' a la cual pertenece el objeto que est� intentando acceder a dicho operador. ";
		break;

	case Errores::ERROR_CLASE_OPERADOR_SENTENCIA_INVALIDA:
		Errores::generarCabeceraError(node, 4510, tipo);
		std::cout << "Se ha producido un error al intentar ejecutar el operador. Compruebe que las sentencias del operador sean correctas. ";
		break;

	case Errores::ERROR_CLASE_OPERADOR_ES_BINARIO:
		Errores::generarCabeceraError(node, 4511, tipo, false);
		std::cout << "Se ha especificado un operador binario sin ninguna entrada, cuando este debe terner una. ";
		break;

	case Errores::ERROR_CLASE_OPERADOR_NO_ES_BINARIO:
		Errores::generarCabeceraError(node, 4512, tipo, false);
		std::cout << "Se ha especificado un operador unitario con entradas, cuando no debe tener ninguna. ";
		break;

	case Errores::ERROR_OPERADOR_ACCESO_CLASE_INVALIDO:
		Errores::generarCabeceraError(node, 2020, tipo, false);
		std::cout << "No se puede usar el operador '.' sobre un elemento que no sea un objeto u otro elemento que lo permita. ";
		break;

	case Errores::ERROR_CLASE_CALL_VARIABLE_NO_EXISTE:
		Errores::generarCabeceraError(node, 4513, tipo, true);
		std::cout << "La variable '" << value << "' a la cual se est� intentando acceder no existe en el objeto. ";
		break;

	case Errores::ERROR_CLASE_VAR_PRIVATE:
		Errores::generarCabeceraError(node, 4514, tipo, true);
		std::cout << "La variable '" << value << "' es privada y no puede ser modificada desde un �mbito externo. ";
		break;

	case Errores::ERROR_CLASE_STATIC_FUNCTION_CALL_NOT_STATIC_VAR:
		Errores::generarCabeceraError(node, 4515, tipo, true);
		std::cout << "No se puede llamar a la variable '"<< value <<"' desde una funci�n est�tica, pues '" << value << "' no es una variable est�tica."  ;
		break;

	case Errores::ERROR_CLASE_STATIC_FUNCTION_NOT_FOUND:
		Errores::generarCabeceraError(node, 4516, tipo, true);
		std::cout << "La funci�n '" << value << "' no existe como funci�n est�tica de la clase '" << value2 << "'.";
		break;

	case Errores::ERROR_CLASE_STATIC_ACCESS:
		Errores::generarCabeceraError(node, 4516, tipo, true);
		std::cout << "Se ha producido un error al intentar acceder a un valor est�tico de la clase '" << value << "'.";
		break;

	case Errores::ERROR_CLASE_STATIC_VAR_NOT_FOUND:
		Errores::generarCabeceraError(node, 4518, tipo, true);
		std::cout << "La variable '" << value << "' no existe como variable est�tica de la clase '" << value2 << "'.";
		break;

	case Errores::ERROR_CLASE_STATIC_ACCESS_NOT_PRIVATE:
		Errores::generarCabeceraError(node, 4519, tipo, true);
		std::cout << "La variable '" << value << "' no es est�tica, por lo que no se puede llamar desde un miembro est�tico.";
		break;
		
	}
	std::cout << "\n";
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
		std::cout << "Se ha elegido una ruta en la funci�n que no devuelve ning�n valor, pero la funci�n " << value << " requiere de un valor de salida. \n ";
		break;

	}
}