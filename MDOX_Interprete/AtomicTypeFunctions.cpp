
#include "Interprete_Core.h"
#include <regex>


//-------------- Iniciando valores -----------------------
std::map<std::string, Core_Function_AtomicTypes> Core::core_atomic_int =
{
	{ "toBinaryString",  Core_Function_AtomicTypes(&funcion_int_toBinaryString) },
	{ "toOctalString",  Core_Function_AtomicTypes(&funcion_int_toOctalString) },
	{ "toHexString",  Core_Function_AtomicTypes(&funcion_int_toHexString) },
	{ "highestBit",  Core_Function_AtomicTypes(&funcion_int_highestBit) },
	{ "lowestBit",  Core_Function_AtomicTypes(&funcion_int_lowestBit) },
	{ "bitCount", Core_Function_AtomicTypes(&funcion_int_bitCount) }
};

std::map<std::string, Core_Function_AtomicTypes> Core::core_atomic_lint =
{
	{ "toBinaryString",  Core_Function_AtomicTypes(&funcion_lint_toBinaryString) },
	{ "toOctalString",  Core_Function_AtomicTypes(&funcion_lint_toOctalString) },
	{ "toHexString",  Core_Function_AtomicTypes(&funcion_lint_toHexString) },
	{ "highestBit",  Core_Function_AtomicTypes(&funcion_lint_highestBit) },
	{ "lowestBit",  Core_Function_AtomicTypes(&funcion_lint_lowestBit) },
	{ "bitCount", Core_Function_AtomicTypes(&funcion_lint_bitCount) }
};

std::map<std::string, Core_Function_AtomicTypes> Core::core_atomic_vector =
{
	{ "size",  Core_Function_AtomicTypes(&funcion_vector_lenght) },
	{ "length",  Core_Function_AtomicTypes(&funcion_vector_lenght) },
	{ "isEmpty",  Core_Function_AtomicTypes(&funcion_vector_isEmpty) },
	{ "contains",  Core_Function_AtomicTypes(&funcion_vector_contains) },
	{ "indexOf",  Core_Function_AtomicTypes(&funcion_vector_indexOf) },
	{ "replace",  Core_Function_AtomicTypes(&funcion_vector_replace) },
	{ "remove",  Core_Function_AtomicTypes(&funcion_vector_remove) },
	{ "pushBack",  Core_Function_AtomicTypes(&funcion_vector_pushBack) },
	{ "push",  Core_Function_AtomicTypes(&funcion_vector_pushBack) },
	{ "pushFront",  Core_Function_AtomicTypes(&funcion_vector_pushFront) },
	{ "popBack",  Core_Function_AtomicTypes(&funcion_vector_popBack) },
	{ "popFront",  Core_Function_AtomicTypes(&funcion_vector_popFront) },
	{ "insert",  Core_Function_AtomicTypes(&funcion_vector_insert) },
	{ "containsAll", Core_Function_AtomicTypes(&funcion_vector_containsAll) }
};

std::map<std::string, Core_Function_AtomicTypes> Core::core_atomic_string =
{
	{ "length",  Core_Function_AtomicTypes(&funcion_string_lenght) },
	{ "size",  Core_Function_AtomicTypes(&funcion_string_lenght) },
	{ "substring",  Core_Function_AtomicTypes(&funcion_string_substring) },
	{ "toLower",  Core_Function_AtomicTypes(&funcion_string_toLower) },
	{ "toUpper",  Core_Function_AtomicTypes(&funcion_string_toUpper) },
	{ "replace",  Core_Function_AtomicTypes(&funcion_string_Replace) },
	{ "startsWith",  Core_Function_AtomicTypes(&funcion_string_startsWith) },
	{ "match",  Core_Function_AtomicTypes(&funcion_string_match) },
	{ "split",  Core_Function_AtomicTypes(&funcion_string_split) },
	{ "find", Core_Function_AtomicTypes(&funcion_string_find) }
};



//_----------------------- FUNCIONES DE VALORES ATOMICOS


/*
	FUNCION insert
	Inserta el valor en la posición index indicada o
	Inserta el valor n cantidad de veces en la posición index
	ENTRADA: <VALUE>, <LINT> ó <VALUE>, <LINT>, <LINT>
	SALIDA: <VOID>
*/
bool funcion_vector_insert(Value caller, std::vector<Value>& a)
{
	std::shared_ptr<mdox_vector> n = std::get<std::shared_ptr<mdox_vector>>(caller.value);

	if (a.size() == 2)
	{
		Value* v = &a[0];
		Value* v2 = &a[1];

		if (!v2->Cast(PARAM_LINT))
		{
			Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "insert", "insert(<value> <lint>):<value>, insert(<lint>, <lint>, <lint>):<value>");
			return false;
		}
		long long t2 = std::get<long long>(v2->value);

		if (t2 > n->vector.size() || t2 < 0)
		{
			Errores::generarError(Errores::ERROR_OFFSET_INVALIDO_FUNCTION, Errores::outData, "insert");
			return false;
		}

		n->vector.insert(n->vector.begin() + t2, *v);
		Interprete::instance->setRetorno(caller);
		return true;
	}

	if (a.size() == 3)
	{
		Value* v = &a[0];
		Value* v2 = &a[1];
		Value* v3 = &a[2];

		if (!v2->Cast(PARAM_LINT))
		{
			Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "insert", "insert(<value> <lint>):<value>, insert(<lint>, <lint>, <lint>):<value>");
			return false;
		}
		if (!v3->Cast(PARAM_LINT))
		{
			Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "insert", "insert(<value> <lint>):<value>, insert(<lint>, <lint>, <lint>):<value>");
			return false;
		}

		long long t2_cantidad = std::get<long long>(v2->value); //cantidad
		long long t3_index = std::get<long long>(v3->value); //index

		if (t3_index > n->vector.size() || t3_index < 0)
		{
			Errores::generarError(Errores::ERROR_OFFSET_INVALIDO_FUNCTION, Errores::outData, "insert");
			return false;
		}

		n->vector.insert(n->vector.begin() + t3_index, t2_cantidad, *v);
		Interprete::instance->setRetorno(caller);
		return true;
	}

	Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "popFront", "popFront():<value>");
	return false;
}

/*
	FUNCION popFront
	Elimina y devuelve el primer valor del vector.
	ENTRADA: <VOID>
	SALIDA: <VALUE>
*/
bool funcion_vector_popFront(Value caller, std::vector<Value>& a)
{
	std::shared_ptr<mdox_vector> n = std::get<std::shared_ptr<mdox_vector>>(caller.value);

	if (a.size() == 0)
	{
		Interprete::instance->setRetorno(Value(n->vector.front()));
		n->vector.pop_front();
		return true;
	}

	Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "popFront", "popFront():<value>");
	return false;
}

/*
	FUNCION popBack
	Elimina y devuelve el último valor del vector.
	ENTRADA: <VOID>
	SALIDA: <VALUE>
*/
bool funcion_vector_popBack(Value caller, std::vector<Value>& a)
{
	std::shared_ptr<mdox_vector> n = std::get<std::shared_ptr<mdox_vector>>(caller.value);

	if (a.size() == 0)
	{
		Interprete::instance->setRetorno(Value(n->vector.back()));
		n->vector.pop_back();
		return true;
	}

	Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "popBack", "popBack():<value>");
	return false;
}


/*
	FUNCION front
	Obtiene el primer valor del vector
	ENTRADA: <VOID>
	SALIDA: <VALUE>
*/
bool funcion_vector_front(Value caller, std::vector<Value>& a)
{
	std::shared_ptr<mdox_vector> n = std::get<std::shared_ptr<mdox_vector>>(caller.value);

	if (a.size() == 0)
	{
		Interprete::instance->setRetorno(Value(n->vector.front()));
		return true;
	}

	Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "front", "front():<value>");
	return false;
}

/*
	FUNCION back
	Obtiene el último valor del vector
	ENTRADA: <VOID>
	SALIDA: <VALUE>
*/
bool funcion_vector_back(Value caller, std::vector<Value>& a)
{
	std::shared_ptr<mdox_vector> n = std::get<std::shared_ptr<mdox_vector>>(caller.value);

	if (a.size() == 0)
	{
		Interprete::instance->setRetorno(Value(n->vector.back()));
		return true;
	}

	Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "back", "back():<value>");
	return false;
}

/*
	FUNCION pushFront
	Agrega un nuevo valor al comienzo del vector.
	ENTRADA: <VALUE>
	SALIDA: <VECTOR>
*/
bool funcion_vector_pushFront(Value caller, std::vector<Value>& a)
{
	std::shared_ptr<mdox_vector> n = std::get<std::shared_ptr<mdox_vector>>(caller.value);

	if (a.size() == 0)
	{
		Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "pushFront", "pushFront(<value>, ...):<vector>");
		return false;
	}

	for (auto i = a.rbegin(); i != a.rend(); ++i)
	{
		n->vector.emplace_front(*i);
	}

	Interprete::instance->setRetorno(caller);
	return true;
}

/*
	FUNCION pushBack
	Agrega un nuevo valor en la cola del vector.
	ENTRADA: <VALUE>
	SALIDA: <VECTOR>
*/
bool funcion_vector_pushBack(Value caller, std::vector<Value>& a)
{
	std::shared_ptr<mdox_vector> n = std::get<std::shared_ptr<mdox_vector>>(caller.value);

	if (a.size() == 0)
	{
		Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "pushBack", "pushBack(<value>, ...):<vector>");
		return false;
	}
	for (Value& v : a)
	{
		n->vector.emplace_back(v);
	}

	Interprete::instance->setRetorno(caller);
	return true;
}


/*
	FUNCION remove
	Remueve el valor en el indice seleccionado, o todos los valores entre un indice y otro.
	ENTRADA: <LINT> ó <LINT>, <LINT>
	SALIDA: <VECTOR>
*/
bool funcion_vector_remove(Value caller, std::vector<Value>& a)
{
	std::shared_ptr<mdox_vector> n = std::get<std::shared_ptr<mdox_vector>>(caller.value);

	if (a.size() == 1)
	{
		Value* v = &a[0];

		if (!v->Cast(PARAM_LINT))
		{
			Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "remove", "remove(<lint>):<void>, remove(<lint>, <lint>):<vector>");
			return false;
		}
		long long t1 = std::get<long long>(v->value);

		if (t1 >= n->vector.size() || t1 < 0)
		{
			Errores::generarError(Errores::ERROR_OFFSET_INVALIDO_FUNCTION, Errores::outData, "remove");
			return false;
		}
		n->vector.erase(n->vector.begin() + t1);
		Interprete::instance->setRetorno(caller);
		return true;
	}
	else if (a.size() == 2)
	{
		Value* v = &a[0];

		if (!v->Cast(PARAM_LINT))
		{
			Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "remove", "remove(<lint>):<vector>, remove(<lint>, <lint>):<vector>");
			return false;
		}

		Value* v2 = &a[1];

		if (!v2->Cast(PARAM_LINT))
		{
			Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "remove", "remove(<lint>):<vector>, remove(<lint>, <lint>):<vector>");
			return false;
		}

		long long t1 = std::get<long long>(v->value);
		long long t2 = std::get<long long>(v2->value);



		if (t1 >= n->vector.size() || t1 < 0 || t2 < t1 || t2 > n->vector.size())
		{
			Errores::generarError(Errores::ERROR_OFFSET_INVALIDO_FUNCTION, Errores::outData, "remove");
			return false;
		}

		n->vector.erase(n->vector.begin() + t1, n->vector.begin() + t2);
		Interprete::instance->setRetorno(caller);
		return true;
	}

	Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "remove", "remove(<lint>):<vector>, remove(<lint>, <lint>):<vector>");
	return false;
}

/*
	FUNCION replace
	Remplaza todos los terminos encontrados por otro dado.
	ENTRADA: <VALUE>, <VALUE>
	SALIDA: <VECTOR>
*/
bool funcion_vector_replace(Value caller, std::vector<Value>& a)
{
	std::shared_ptr<mdox_vector> n = std::get<std::shared_ptr<mdox_vector>>(caller.value);

	if (a.size() == 2)
	{
		Value* v = &a[0];
		Value* v2 = &a[1];

		auto it = n->vector.begin();
		while (it != n->vector.end())
		{
			it = std::find_if(it, n->vector.end(), [&v](Value& obj)->bool {return obj.igualdad_Condicional(*v); });

			if (it == n->vector.end())
				break;

			n->vector[std::distance(n->vector.begin(), it)] = *v2;
		}
		Interprete::instance->setRetorno(caller);
		return true;
	}

	Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "replace", "replace(<value>,<value>):<vector>");
	return false;
}

/*
	FUNCION containsAll
	Comprueba si el vector contiene todos los valores indicados en otro vector
	ENTRADA: <VECTOR>
	SALIDA: <BOOL>
*/
bool funcion_vector_containsAll(Value caller, std::vector<Value>& a)
{
	std::shared_ptr<mdox_vector> n = std::get<std::shared_ptr<mdox_vector>>(caller.value);

	if (a.size() == 0)
	{
		Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "containsAll", "containsAll(<vector>):<bool>");
		return false;
	}

	for (auto reg : a)
	{
		auto it = std::find_if(n->vector.begin(), n->vector.end(), [&reg](Value& obj) {return obj.igualdad_Condicional(reg); });

		if (it != n->vector.end())
			continue;

		Interprete::instance->setRetorno(Value(false));
		return true;
	}

	Interprete::instance->setRetorno(Value(true));
	return true;

}

/*
	FUNCION isEmpty
	Comprobar si el vector está vacio
	ENTRADA: <VALUE>
	SALIDA: <INT>
*/
bool funcion_vector_isEmpty(Value caller, std::vector<Value>& a)
{
	std::shared_ptr<mdox_vector> n = std::get<std::shared_ptr<mdox_vector>>(caller.value);

	if (a.size() == 0)
	{
		Interprete::instance->setRetorno(Value(n->vector.empty()));
		return true;
	}

	Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "isEmpty", "isEmpty():<bool>");
	return false;
}

/*
	FUNCION indexOf
	Comprueba si el valor existe y devuelve el indice del primero encontrado, de no encontrar nada, decuelve el último indice del vector.
	ENTRADA: <VALUE>
	SALIDA: <INT>
*/
bool funcion_vector_indexOf(Value caller, std::vector<Value>& a)
{
	std::shared_ptr<mdox_vector> n = std::get<std::shared_ptr<mdox_vector>>(caller.value);

	if (a.size() == 1)
	{
		auto it = std::find_if(n->vector.begin(), n->vector.end(), [&a](Value& obj) {return obj.igualdad_Condicional(a[0]); });

		if (it != n->vector.end())
		{
			long long index = std::distance(n->vector.begin(), it);
			Interprete::instance->setRetorno(Value(index));
			return true;
		}
		Interprete::instance->setRetorno(Value((long long)n->vector.size()));
		return true;
	}

	Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "indexOf", "indexOf(<value>):<lint>");
	return false;
}

/*
	FUNCION contains
	Comprueba si el valor existe en el vector
	ENTRADA: <VALUE>
	SALIDA: <BOOL>
*/
bool funcion_vector_contains(Value caller, std::vector<Value>& a)
{
	std::shared_ptr<mdox_vector> n = std::get<std::shared_ptr<mdox_vector>>(caller.value);

	if (a.size() == 1)
	{
		auto it = std::find_if(n->vector.begin(), n->vector.end(), [&a](Value& obj) {return obj.igualdad_Condicional(a[0]); });

		if (it != n->vector.end())
		{
			Interprete::instance->setRetorno(Value(true));
			return true;
		}
		Interprete::instance->setRetorno(Value(false));
		return true;
	}

	Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "contains", "contains(<value>):<bool>");
	return false;
}

/*
	FUNCION startsWith.
	Comprueba si el string comienza por el substring indicado
	ENTRADA: <VALUE>
	SALIDA: <BOOL>
*/
bool funcion_string_startsWith(Value caller, std::vector<Value>& a)
{
	std::string n = std::get<std::string>(caller.value);

	if (a.size() == 1)
	{
		Value* v = &a[0];
		if (!v->Cast(PARAM_STRING))
		{
			Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "startsWith", "startsWith(<string>):<bool>");
			return false;
		}
		std::string t1 = std::get<std::string>(v->value);

		if (n.rfind(t1, 0) == 0) {
			Interprete::instance->setRetorno(Value(true));
			return true;
		}
		Interprete::instance->setRetorno(Value(false));
		return true;
	}

	Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "startsWith", "startsWith(<string>):<bool>");
	return false;

}

/*
	FUNCION Split.
	Corta la cadena usando una cadena regex
	ENTRADA: <> ó <STRING>
	SALIDA: <VECTOR>
*/
bool funcion_string_split(Value caller, std::vector<Value>& a)
{
	std::string n = std::get<std::string>(caller.value);


	if (a.size() == 0)
	{
		std::shared_ptr<mdox_vector> elems = std::make_shared<mdox_vector>();
		std::regex rgx("\\s+");
		std::sregex_token_iterator iter(n.begin(), n.end(), rgx, -1);
		std::sregex_token_iterator end;

		while (iter != end) {
			//std::cout << "S43:" << *iter << std::endl;
			elems->vector.push_back(Value(*iter));
			++iter;
		}

		Interprete::instance->setRetorno(Value(elems));
		return true;
	}
	if (a.size() == 1)
	{
		Value* v = &a[0];

		if (!v->Cast(PARAM_STRING))
		{
			Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "split", "split(<string:regex?>):<vector>");
			return false;
		}

		std::string t1 = std::get<std::string>(v->value);

		std::shared_ptr<mdox_vector> elems = std::make_shared<mdox_vector>();


		std::regex rgx;
		try {
			rgx = t1;
		}
		catch (std::regex_error & e) {
			Errores::generarError(Errores::ERROR_REGEX_INVALID, Errores::outData, t1);
			return false;
		}


		std::sregex_token_iterator iter(n.begin(), n.end(), rgx, -1);
		std::sregex_token_iterator end;

		while (iter != end) {
			//std::cout << "S43:" << *iter << std::endl;
			elems->vector.push_back(Value(*iter));
			++iter;
		}

		Interprete::instance->setRetorno(Value(elems));
		return true;
	}

	Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "split", "split(<string:regex?>):<vector>");
	return false;
}


/*
	FUNCION Match.
	Devuelve verdadero si encuentra un match especificado por un regex
	ENTRADA: <STRING>
	SALIDA: <BOOL>
*/
bool funcion_string_match(Value caller, std::vector<Value>& a)
{
	std::string n = std::get<std::string>(caller.value);

	if (a.size() == 1)
	{
		Value* v = &a[0];

		if (!v->Cast(PARAM_STRING))
		{
			Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "match", "match(<string:regex>):<bool>");
			return false;
		}

		std::string t1 = std::get<std::string>(v->value);

		std::regex e;
		try {
			e = t1;
		}
		catch (std::regex_error & e) {
			Errores::generarError(Errores::ERROR_REGEX_INVALID, Errores::outData, t1);
			return false;
		}

		if (std::regex_match(n, e))
		{
			Interprete::instance->setRetorno(Value(true));
			return true;
		}
		Interprete::instance->setRetorno(Value(false));
		return true;
	}


	Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "match", "match(<string:regex>):<bool>");
	return false;

}
/*
	FUNCION Replace.
	Remplaza un valor por otro usando regex
	ENTRADA: <STRING> <STRING>
	SALIDA: <STRING>
*/

bool funcion_string_Replace(Value caller, std::vector<Value>& a)
{
	std::string n = std::get<std::string>(caller.value);

	if (a.size() == 2)
	{
		Value* v = &a[0];
		if (!v->Cast(PARAM_STRING))
		{
			Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "replace", "replace(<string:regex>, <string>):<string>");
			return false;
		}

		std::string t1 = std::get<std::string>(v->value);
		std::regex re;
		try {
			re = t1;
		}
		catch (std::regex_error & e) {
			Errores::generarError(Errores::ERROR_REGEX_INVALID, Errores::outData, t1);
			return false;
		}


		Value* v2 = &a[1];
		if (!v->Cast(PARAM_STRING))
		{
			Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "replace", "replace(<string:regex>, <string>):<string>");
			return false;
		}

		std::string t2 = std::get<std::string>(v2->value);

		Interprete::instance->setRetorno(Value(std::regex_replace(n, re, t2)));
		return true;
	}


	Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "replace", "replace(<string:regex>, <string>):<string>");
	return false;

}

/*
	FUNCION To Lower.
	Transforma a mayusculas un texto
	ENTRADA: <>
	SALIDA: <STRING>
*/

bool funcion_string_toLower(Value caller, std::vector<Value>& a)
{
	std::string n = std::get<std::string>(caller.value);

	if (a.size() == 0)
	{
		std::transform(n.begin(), n.end(), n.begin(), ::tolower);
		Interprete::instance->setRetorno(Value(n));
		return true;
	}

	Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "toLower", "toLower():<string>");
	return false;
}

/*
	FUNCION To Upper.
	Transforma a mayusculas un texto
	ENTRADA: <>
	SALIDA: <STRING>
*/

bool funcion_string_toUpper(Value caller, std::vector<Value>& a)
{
	std::string n = std::get<std::string>(caller.value);

	if (a.size() == 0)
	{
		std::transform(n.begin(), n.end(), n.begin(), ::toupper);
		Interprete::instance->setRetorno(Value(n));
		return true;
	}

	Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "toUpper", "toUpper():<string>");
	return false;

}


/*
	FUNCION Substring.
	Devuelve la cadena entre  uno o dos puntos dados
	* -> SIEMPRE se da, por contexto.
	ENTRADA: <STRING>
	SALIDA: <LINT>
*/

bool funcion_string_find(Value caller, std::vector<Value>& a)
{
	std::string n = std::get<std::string>(caller.value);

	if (a.size() == 1)
	{
		Value* v = &a[0];

		if (!v->Cast(PARAM_STRING))
		{
			Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "find", "substring(<string>):<lint>");
			return false;
		}

		std::string t1 = std::get<std::string>(v->value);
		Interprete::instance->setRetorno(Value((long long)n.find(t1)));
		return true;
	}

	Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "find", "substring(<string>):<lint>");
	return false;
}


/*
	FUNCION Substring.
	Devuelve la cadena entre  uno o dos puntos dados
	* -> SIEMPRE se da, por contexto.
	ENTRADA: <INT> ó <INT, INT>
	SALIDA: <STRING>
*/
bool funcion_string_substring(Value caller, std::vector<Value>& a)
{
	std::string n = std::get<std::string>(caller.value);

	if (a.size() == 1)
	{
		Value* v = &a[0];

		if (!v->Cast(PARAM_LINT))
		{
			Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "substring", "substring(<int>, <int?>):<string>");
			return false;
		}

		long long t1 = std::get<long long>(v->value);

		if (t1 < 0 || t1 >= n.size())
		{
			Errores::generarError(Errores::ERROR_OFFSET_INVALIDO_FUNCTION, NULL, "substring");
			return false;
		}

		Interprete::instance->setRetorno(Value(n.substr(t1)));
		return true;
	}

	if (a.size() == 2)
	{
		Value* v = &a[0];
		Value* v2 = &a[1];

		if (!v->Cast(PARAM_LINT))
		{
			Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "substring", "substring(<int>, <int?>):<string>");
			return false;
		}

		long long t1 = std::get<long long>(v->value);

		if (t1 < 0 || t1 >= n.size())
		{
			Errores::generarError(Errores::ERROR_OFFSET_INVALIDO_FUNCTION, NULL, "substring");
			return false;
		}

		if (!v2->Cast(PARAM_LINT))
			return false;

		long long t2 = std::get<long long>(v2->value);

		if (t2 < 0)
		{
			Errores::generarError(Errores::ERROR_OFFSET_INVALIDO_FUNCTION, NULL, "substring");
			return false;
		}

		Interprete::instance->setRetorno(Value(n.substr(t1, t2)));
		return true;
	}

	Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "substring", "substring(<int>, <int?>):<string>");
	return false;

}


/*
	FUNCION lenght.
	Devuelve el menor bit activo en el entero.
	* -> SIEMPRE se da, por contexto.
	ENTRADA: <>
	SALIDA: <LINT>
*/
bool funcion_string_lenght(Value caller, std::vector<Value>& a)
{
	if (a.size() != 0)
	{
		Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "lenght", "lenght():<lint>");
		return false;
	}

	std::string n = std::get<std::string>(caller.value);
	Interprete::instance->setRetorno(Value((long long)n.size()));
	return true;
}

bool funcion_vector_lenght(Value caller, std::vector<Value>& a)
{
	if (a.size() != 0)
	{
		Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "lenght", "lenght():<lint>");
		return false;
	}

	std::shared_ptr<mdox_vector > n = std::get<std::shared_ptr<mdox_vector>>(caller.value);

	Interprete::instance->setRetorno(Value((long long)n->vector.size()));
	return true;
}

/*
	FUNCION lowestBit.
	Devuelve el menor bit activo en el entero.
	* -> SIEMPRE se da, por contexto.
	ENTRADA: <>
	SALIDA: <INT>
*/
template <class T>
bool funcion_lowestBit(Value& caller, std::vector<Value>& a)
{
	if (a.size() != 0)
	{
		Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "lowestBit", "lowestBit():<int>");
		return false;
	}

	T n = std::get<T>(caller.value);
	Interprete::instance->setRetorno(Value(n & -n));
	return true;
}

bool funcion_int_lowestBit(Value caller, std::vector<Value>& a)
{
	return funcion_lowestBit<int>(caller, a);
}

bool funcion_lint_lowestBit(Value caller, std::vector<Value>& a)
{
	return funcion_lowestBit<long long>(caller, a);
}
/*
	FUNCION highestBit.
	Devuelve el mayor bit activo en el entero.
	* -> SIEMPRE se da, por contexto.
	ENTRADA: <>
	SALIDA: <INT>
*/
template <class T>
bool funcion_highestBit(Value& caller, std::vector<Value>& a)
{
	if (a.size() != 0)
	{
		Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "highestBit", "highestBit():<int>");
		return false;
	}

	T n = std::get<T>(caller.value);
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	n = n + 1;
	Interprete::instance->setRetorno(Value((n >> 1)));
	return true;
}

bool funcion_int_highestBit(Value caller, std::vector<Value>& a)
{
	return funcion_highestBit<int>(caller, a);
}

bool funcion_lint_highestBit(Value caller, std::vector<Value>& a)
{
	return funcion_highestBit<long long>(caller, a);
}

/*
	FUNCION bitCount.
	Devuelve el numero de bits 1 existentes en un número determinado.
	* -> SIEMPRE se da, por contexto.
	ENTRADA: <>
	SALIDA: <INT>
*/
template <class T>
bool funcion_bitCount(Value& caller, std::vector<Value>& a)
{
	if (a.size() != 0)
	{
		Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "bitCount", "bitCount():<int>");
		return false;
	}

	T n = std::get<T>(caller.value);
	int count = 0;
	while (n) {
		count += n & 1;
		n >>= 1;
	}

	Interprete::instance->setRetorno(Value(count));
	return true;
}

bool funcion_int_bitCount(Value caller, std::vector<Value>& a)
{
	return funcion_bitCount<int>(caller, a);
}

bool funcion_lint_bitCount(Value caller, std::vector<Value>& a)
{
	return funcion_bitCount<long long>(caller, a);
}

/*
	FUNCION ToBinaryString.
	Devuelve el entero convertido a Binario en un String
	* -> SIEMPRE se da, por contexto.
	ENTRADA: <>
	SALIDA: <STRING>
*/

template <class T>
bool funcion_toBinaryString(Value& caller, std::vector<Value>& a) {

	if (a.size() != 0)
	{
		Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "toBinaryString", "toBinaryString():<string>");
		return false;
	}

	T v = std::get<T>(caller.value);

	using U = typename std::make_unsigned<T>::type;
	U input(v);

	std::string result;
	unsigned bits = std::numeric_limits<U>::digits;

	for (int i = bits - 1; i > -1; i--) {
		auto val = input >> i;
		result.push_back((val & 1) + '0');
	}
	Interprete::instance->setRetorno(Value(result));
	return true;
}

bool funcion_int_toBinaryString(Value caller, std::vector<Value>& a)
{
	return funcion_toBinaryString<int>(caller, a);
}

bool funcion_lint_toBinaryString(Value caller, std::vector<Value>& a)
{
	return funcion_toBinaryString<long long>(caller, a);
}


/*
	FUNCION ToBinaryString.
	Devuelve el entero convertido a HEX
	* -> SIEMPRE se da, por contexto.
	ENTRADA: <>
	SALIDA: <STRING>
*/

template <class T>
bool funcion_toHexString(Value& caller, std::vector<Value>& a)
{
	T v = std::get<T>(caller.value);
	std::ostringstream vStream;
	vStream << std::hex << v;
	Interprete::instance->setRetorno(Value(vStream.str()));
	return true;
}

bool funcion_int_toHexString(Value caller, std::vector<Value>& a)
{
	return funcion_toHexString<int>(caller, a);
}

bool funcion_lint_toHexString(Value caller, std::vector<Value>& a)
{
	return funcion_toHexString<long long>(caller, a);
}

/*
	FUNCION ToBinaryString.
	Devuelve el entero convertido a OCTAL
	* -> SIEMPRE se da, por contexto.
	ENTRADA: <>
	SALIDA: <STRING>
*/
template <class T>
bool funcion_toOctalString(Value& caller, std::vector<Value>& a)
{
	T v = std::get<T>(caller.value);
	std::ostringstream vStream;
	vStream << std::oct << v;
	Interprete::instance->setRetorno(Value(vStream.str()));
	return true;
}

bool funcion_int_toOctalString(Value caller, std::vector<Value>& a)
{
	return funcion_toOctalString<int>(caller, a);
}

bool funcion_lint_toOctalString(Value caller, std::vector<Value>& a)
{
	return funcion_toOctalString<long long>(caller, a);
}
