
#include "Interprete_Core.h"

std::vector<Core_Function*> Core::core_functions;

void Core::Start()
{
	//Agregada función print.
	Core_Function_Interprete * print = new Core_Function_Interprete("print", { PARAM_VOID }, PARAM_VOID);
	print->funcion_exec = &funcion_print;
	Core::core_functions.push_back(print);

	Core_Function_Interprete * now = new Core_Function_Interprete("now", {}, PARAM_LINT);
	now->funcion_exec = &funcion_now;
	Core::core_functions.push_back(now);



}


//_----------------------- FUNCIONES
/*
	FUNCION PRINT(X).
	Escribe el operador X por el stream default.
	ENTRADA: <VOID>
	SALIDA: <VOID>
*/
Value * funcion_print(std::vector<Value*>* a, OutData_Parametros * b)
{
	if (a->size() < 1)
		return new Value();

	Value * v = a->at(0);

	if (!v)
		Errores::generarError(Errores::ERROR_FUNCION_PARAMETRO_OPERACION_INVALIDA, b, "print");
	else
	{
		Interprete::instance->ValueToConsole(v);
		std::cout << "\n";
	}

	return new Value();
}


/*
	FUNCION NOW().
	Devuelve el tiempo actual en long transcurrido desde el 1 de Enero de 1970.
	ENTRADA: <NONE>
	SALIDA: <LINT>
*/
// TODO: Cuando esten implementados los enums, NOW recibira ms, s, etc y devolvera eso.
Value * funcion_now(std::vector<Value*>* a, OutData_Parametros * b)
{
	Value_LINT * v = new Value_LINT(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

	if (!v)
	{
		Errores::generarError(Errores::ERROR_FUNCION_IMPOSIBLE_RECUPERAR_FECHA, b);
		return NULL;
	}
	
	return v;
}
