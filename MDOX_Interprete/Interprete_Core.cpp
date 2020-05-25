
#include "Interprete_Core.h"

std::vector<Core_Function*> Core::core_functions;


bool Core::IncludeStart(std::string& val)
{
	if (val == "File")
	{
		MDOX_File::MDOX_File_Start();
		return true;
	}
	return false;
}


void Core::Start()
{
	//Agregadas funciones
	Core_Function* print = new Core_Function("print");
	print->funcion_exec = &funcion_print;
	Core::core_functions.push_back(print);

	Core_Function* now = new Core_Function("now");
	now->funcion_exec = &funcion_now;
	Core::core_functions.push_back(now);

	Core_Function* sleep = new Core_Function("sleep");
	sleep->funcion_exec = &funcion_sleep;
	Core::core_functions.push_back(sleep);
	
	Core_Function* typeOf = new Core_Function("typeOf");
	typeOf->funcion_exec = &funcion_typeOf;
	Core::core_functions.push_back(typeOf);
}



//Funciones CORE del PARSER que requieren del Interprete


Value Parser_Class::execCoreOperator(int index, Value& entrada)
{
	if (!isCore)
		assert(!"ERROR CRITICO INTERNO: Una función normal intenta acceder a execCoreOperator");
	else
	{
		core->operadores[index](&entrada);

		if (Interprete::instance->returnCalled())
		{
			return Interprete::instance->getRetorno();
		}
		else
		{
			return std::monostate();
		}

	}
}

Value Parser_Class::execCoreOperator(int index)
{
	if (!isCore)
		assert(!"ERROR CRITICO INTERNO: Una función normal intenta acceder a execCoreOperator");
	else
	{
		core->operadores[index](NULL);

		if (Interprete::instance->returnCalled())
		{
			return Interprete::instance->getRetorno();
		}
		else
		{
			return std::monostate();
		}

	}
}







//ERROR_CLASE_CORE_FUNCTION_NOT_EXIST
//ERROR_FUNCTION_PARAMETER
//ERROR_FUNCION_NO_DECLARADA

//_----------------------- FUNCIONES
/*
	FUNCION PRINT(X).
	Escribe el operador X por el stream default.
	ENTRADA: <VOID>
	SALIDA: <VOID>
*/
bool funcion_print(std::vector<Value>& a)
{
	if (a.size() != 1)
	{
		Errores::generarError(Errores::ERROR_FUNCION_NO_DECLARADA_HA, Errores::outData, "print", "print(<value>):<void>");
		return false;
	}

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
bool funcion_now(std::vector<Value>& a)
{
	if (a.size() != 0)
	{
		Errores::generarError(Errores::ERROR_FUNCION_NO_DECLARADA_HA, Errores::outData, "now", "now(<value>):<lint>");
		return false;
	}
	Value v = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	Interprete::instance->setRetorno(v);
	return true;
}


/*
	FUNCION TypeOf().
	Compara un tipo con el introducido por le usuario.
	ENTRADA: <LINT>
	SALIDA: NONE
*/
bool funcion_typeOf(std::vector<Value>& a)
{
	if (a.size() != 2)
	{
		Errores::generarError(Errores::ERROR_FUNCION_NO_DECLARADA_HA, Errores::outData, "typeOf", "typeOf(<value>, <string>):<bool>");
		return false;
	}

	if (!a[1].Cast(PARAM_STRING))
	{
		Errores::generarError(Errores::ERROR_FUNCION_NO_DECLARADA_HA, Errores::outData, "typeOf", "typeOf(<value>, <string>):<bool>");
		return false;
	}

	std::string entrada = std::get<std::string>(a[1].value);

	Interprete::instance->getRealValueFromValueWrapper(a[0]);

		Value res = a[0];

		bool v = std::visit(overloaded{
			[&](int&)->bool { return entrada == "int"; },
			[&](double&)->bool { return entrada == "double"; },
			[&](long long&)->bool { return entrada == "lint"; },
			[&](bool&)->bool { return entrada == "bool"; },
			[&](std::shared_ptr<mdox_vector>&)->bool { return entrada == "vector"; },
			[&](std::shared_ptr<mdox_object>& a)->bool { return a->clase->created->pID->nombre == entrada; },
			[&](std::string&)->bool { return entrada == "string"; },
			[&](std::monostate&)->bool { return entrada == "void"; },
			[&](auto&)->bool { return false; },
		}, a[0].value);
	
	Interprete::instance->setRetorno(Value(v));

	return true;
}

/*
	FUNCION SLEEP().
	Pausa el hilo actual durante x ms
	ENTRADA: <LINT>
	SALIDA: NONE
*/
bool funcion_sleep(std::vector<Value>& a)
{
	if (a.size() != 1)
	{
		Errores::generarError(Errores::ERROR_FUNCION_NO_DECLARADA_HA, Errores::outData, "sleep", "sleep(<lint>):<void>");
		return false;
	}

	if (!a[0].Cast(PARAM_LINT))
	{
		Errores::generarError(Errores::ERROR_FUNCION_NO_DECLARADA_HA, Errores::outData, "sleep", "sleep(<lint>):<void>");
		return false;
	}


	std::this_thread::sleep_for(std::chrono::milliseconds(std::get<long long>(a[0].value)));

	return true;
}


