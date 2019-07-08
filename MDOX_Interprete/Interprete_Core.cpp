
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

	Core_Function_Interprete * sleep = new Core_Function_Interprete("sleep", {PARAM_INT}, PARAM_VOID);
	sleep->funcion_exec = &funcion_sleep;
	Core::core_functions.push_back(sleep);

}


//_----------------------- FUNCIONES
/*
	FUNCION PRINT(X).
	Escribe el operador X por el stream default.
	ENTRADA: <VOID>
	SALIDA: <VOID>
*/
bool funcion_print(std::vector<Value>& a, Interprete* interprete)
{
		//Errores::generarError(Errores::ERROR_FUNCION_PARAMETRO_OPERACION_INVALIDA, b, "print");
		a[0].print();
		std::cout << "\n";
		return true;
}


/*
	FUNCION NOW().
	Devuelve el tiempo actual en long transcurrido desde el 1 de Enero de 1970.
	ENTRADA: <NONE>
	SALIDA: <LINT>
*/
// TODO: Cuando esten implementados los enums, NOW recibira ms, s, etc y devolvera eso.
bool funcion_now(std::vector<Value>& a, Interprete* interprete)
{
	//Value_LINT * v = new Value_LINT(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
	
	Value v = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	interprete->setRetorno(v);

	//Errores::generarError(Errores::ERROR_FUNCION_IMPOSIBLE_RECUPERAR_FECHA, b);
	return true;
}


bool funcion_sleep(std::vector<Value>& a, Interprete* interprete)
{
	Value v = a[0];

	if (!v.Cast(PARAM_INT))
		return false;

	std::this_thread::sleep_for(std::chrono::milliseconds(std::get<int>(v.value)));

	return true;
}


