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

	case Errores::ERROR_MAIN_NO_ENCONTRADO: // Obsoleto.
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
		std::cout << "La variable " << value << " no existe o no fue declarada. ";
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

	case Errores::ERROR_CONVERSION_STRICT:
		Errores::generarCabeceraError(node, 2004, tipo);
		std::cout << "No se puede realizar la conversi�n a la variable strict proporcionada. Los tipos no coinciden. ";
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
		std::cout << "La funci�n con nombre '" << value << "' no ha sido declarada. ";
		break;

	case Errores::ERROR_FUNCION_NO_DECLARADA_HA:
		Errores::generarCabeceraError(node, 4006, tipo);
		std::cout << "La funci�n con nombre '" << value << "' no ha sido declarada. " << " Quiz�s quer�a usar: " << value2;
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
		std::cout << "No se ha podido aplicar el operador '" << value << "'. Es posible que el tipo de elemento no permita el uso del mismo.  ";
		break;

	case Errores::ERROR_ASIGNACION_VALOR_VOID:
		Errores::generarCabeceraError(node, 2018, tipo);
		std::cout << "No se puede establecer una variable con un valor vacio si esta fue fijada como '" << value << "' con anterioridad.  ";
		break;

	case Errores::ERROR_OFFSET_INVALIDO:
		Errores::generarCabeceraError(node, 2019, tipo, true);
		std::cout << "El valor introducido como indice ('" << value << "') est� fuera del rango aceptado.  ";
		break;

	case Errores::ERROR_OFFSET_INVALIDO_FUNCTION:
		Errores::generarCabeceraError(node, 2019, tipo, true);
		std::cout << "El valor �ndice introducido en la funci�n '"<<value<<"' est� fuera de rango.";
		break;

	case Errores::ERROR_CLASE_CONSTRUCTOR_SINTAXIS:
		Errores::generarCabeceraError(node, 4500, tipo, false);
		std::cout << "Se ha producido un error en la sintaxis al intentar crear un constructor. ";
		break;

	case Errores::ERROR_CLASE_CONSTRUCTOR_ID_NOT_FOUND:
		Errores::generarCabeceraError(node, 4501, tipo, false);
		std::cout << "El identificador '" << value << "' del constructor no existe en la clase referenciada.";
		break;

	case Errores::ERROR_CLASE_CONSTRUCTOR_SENTENCIA_ERROR:
		Errores::generarCabeceraError(node, 4502, tipo, false);
		std::cout << "Se ha producido un error al intentar construir el objeto.";
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
		std::cout << "La variable '" << value << "' es privada y no puede ser accedida desde un �mbito externo. ";
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
		
	case Errores::ERROR_FORMAT_NO_ACEPTADA:
		Errores::generarCabeceraError(node, 4007, tipo, true);
		std::cout << "No se ha podido formatear la cadena obtenida al valor proporcionado.";
		break;

	case Errores::ERROR_REGEX_NOT_MATCH:
		Errores::generarCabeceraError(node, 4008, tipo, true);
		std::cout << "No se ha producido ninguna coincidencia para el regex '" + value + "' en la cadena '" + value2 + "'.";
		break;

	case Errores::ERROR_CLASE_VAR_NOT_EXIST:
		Errores::generarCabeceraError(node, 4520, tipo, true);
		std::cout << "El objeto no contiene ninguna variable con el nombre '" + value + "' declarado.";
		break;

	case Errores::ERROR_REGEX_VAR_NOT_AVAILABLE_TRANSFORM:
		Errores::generarCabeceraError(node, 4009, tipo, true);
		std::cout << "La variable '" + value + "' no es de un tipo convertible a regex.";
		break;
	case Errores::ERROR_INPUT_NOT_MATCH:
		Errores::generarCabeceraError(node, 4010, tipo, true);
		std::cout << "El valor de entrada no coincide con las especificaciones del input.";
		break;

	case Errores::ERROR_REGEX_INVALID:
		Errores::generarCabeceraError(node, 4011, tipo, true);
		std::cout << "El regex introducido no es correcto: " + value;
		break;

	case Errores::ERROR_INTEGER_OVERFLOW:
		Errores::generarCabeceraError(node, 2021, tipo, true);
		std::cout << "Integer overflow, el literal no se ha podido transformar, su tama�o supera el l�mite permitido.";
		break;

	case Errores::ERROR_DOUBLE_OVERFLOW:
		Errores::generarCabeceraError(node, 2022, tipo, true);
		std::cout << "Double overflow, el literal no se ha podido transformar, su tama�o supera el l�mite permitido.";
		break;

	case Errores::ERROR_OPERATION_DOUBLE_OPERATOR:
		Errores::generarCabeceraError(node, 2023, tipo, true);
		std::cout << "Error al realizar la operaci�n, no puede haber dos operadores seguidos en una operaci�n, puede que falten par�ntesis.";
		break;

	case Errores::ERROR_SINTAXIS_PARENTESIS:
		Errores::generarCabeceraError(node, 2024, tipo, true);
		std::cout << "No se ha detectado el cierre corecto del par�ntesis.";
		break;		

	case Errores::ERROR_ASIGNACION_FALLO:
		Errores::generarCabeceraError(node, 2025, tipo, false);
		std::cout << "Un valor se debe asignar a una variable y se est� asignando a otro valor.";
		break;

	case Errores::ERROR_CLASE_ATOMIC_VAR_NOT_EXIST:
		Errores::generarCabeceraError(node, 4521, tipo, false);
		std::cout << "El tipo '"<<value<<"' no contiene ninguna variable con el nombre de '"<< value2 << "'.";
		break;

	case Errores::ERROR_CLASE_ATOMIC_FUNCTION_NOT_EXIST:
		Errores::generarCabeceraError(node, 4522, tipo, false);
		std::cout << "El tipo '" << value << "' no contiene ninguna funci�n con el nombre de '" << value2 << "'.";
		break;
	case Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER:
		Errores::generarCabeceraError(node, 4523, tipo, false);
		std::cout << "La funci�n '" << value << "' solo puede contener los par�metros: '" << value2 << "'.";
		break;

	case Errores::ERROR_CLASE_CORE_FUNCTION_NOT_EXIST:
		Errores::generarCabeceraError(node, 4525, tipo, false);
		std::cout << "La clase '" << value << "' no contiene ninguna funci�n con el nombre de '" << value2 << "'.";
		break;
	case Errores::ERROR_FUNCTION_PARAMETER:
		Errores::generarCabeceraError(node, 4012, tipo, false);
		std::cout << "La funci�n '" << value << "' solo puede contener los par�metros: '" << value2 << "'.";
		break;
	case Errores::ERROR_CLASE_CORE_CONSTRUCTOR:
		Errores::generarCabeceraError(node, 4526, tipo, false);
		std::cout << "El constructor de la clase '" << value << "' no es valido. Par�metros v�lidos: '" << value2 << "'.";
		break;


		/*
			ERROR_INCLUDE_FALLO, //Error 2026
			ERROR_INCLUDE_RUTA_INVALIDA, //Error 2027
			ERROR_INCLUDE_PARAMETRO, //Error 2028
		*/
	case Errores::ERROR_INCLUDE_FALLO:
		Errores::generarCabeceraError(node, 2026, tipo, false);
		std::cout << "Se ha producido un error al leer el fichero '"+value+"' desde el include.";
		break;

	case Errores::ERROR_INCLUDE_RUTA_INVALIDA:
		Errores::generarCabeceraError(node, 2026, tipo, false);
		std::cout << "Error en el include. El fichero de la ruta '" + value + "' no existe o no se puede acceder hasta el.";
		break;

	case Errores::ERROR_INCLUDE_PARAMETRO:
		Errores::generarCabeceraError(node, 2026, tipo, false);
		std::cout << "Error de parseado, un include debe tener asociado un string. 'include <string>'";
		break;

	case Errores::ERROR_INCLUDE_REF_CIRCULAR:
		Errores::generarCabeceraError(node, 2029, tipo, false);
		std::cout << "Existe una referencia circular en el import. Compruebe que los includes no se est�n llamando entre ellos. Referencia circular en: " + value + ".";
		break;

	case Errores::ERROR_INCLUDE_REQ_ALREADY:
		Errores::generarCabeceraError(node, 2030, tipo, false);
		std::cout << "Ya se ha hecho con anterioridad un require a '" + value + "'. No se puede realizar require del mismo documento m�s de una vez.";
		break;

	case Errores::ERROR_CLASE_NO_DECLARADA:
		Errores::generarCabeceraError(node, 4524, tipo, false);
		std::cout << "Se est� intentando inicializar la clase '" + value + "', pero no ha sido declarada.";
		break;

	case Errores::ERROR_CLASE_CORE_FILE_NOT_OPENED:
		Errores::generarCabeceraError(node, 5000, tipo, true);
		std::cout << "No se ha podido acceder al fichero, el sistema no pudo realizar la operaci�n si el fichero no se pudo abrir. Compruebe si no lo ha cerrado previamente. ";
		break;

	case Errores::ERROR_CLASE_CORE_FILE_FAIL_OPEN:
		Errores::generarCabeceraError(node, 5001, tipo, true);
		std::cout << "Ha ocurrido un error al intentar abrir el fichero. No se ha podido abrir el fichero '"+value+"'.";
		break;

	case Errores::ERROR_CLASE_CORE_FILE_OUT_INDEX:
		Errores::generarCabeceraError(node, 5002, tipo, true);
		std::cout << "Se intenta acceder al �ndice "+value+" pero el tama�o m�ximo del fichero es "+value2+".";
		break;

	case Errores::ERROR_CLASE_CORE_FILE_NOT_READ:
		Errores::generarCabeceraError(node, 5003, tipo, true);
		std::cout << "Se intenta acceder a la funci�n " + value + " pero el fichero no se abri� con modo lectura.";
		break;

	case Errores::ERROR_CLASE_CORE_FILE_NOT_WRITE:
		Errores::generarCabeceraError(node, 5003, tipo, true);
		std::cout << "Se intenta acceder a la funci�n " + value + " pero el fichero no se abri� con modo escritura.";
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