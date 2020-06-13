#ifndef INTERPRETE_CORE_H
#define INTERPRETE_CORE_H

#include "Interprete.h"




static class MDOX_File
{
public:
	static int ruta_fichero;
	static int abierto_como;
	static void MDOX_File_Start();
	static void MDOX_Class_File();
	static bool MDOX_Class_File_Function_setWritePosition(std::shared_ptr<mdox_object> obj, std::vector<Value>& v);
	static bool MDOX_Class_File_Function_getWritePosition(std::shared_ptr<mdox_object> obj, std::vector<Value>& v);
	static bool MDOX_Class_File_Function_write(std::shared_ptr<mdox_object> obj, std::vector<Value>& v);
	static bool MDOX_Class_File_Function_setReadPosition(std::shared_ptr<mdox_object> obj, std::vector<Value>& v);
	static bool MDOX_Class_File_Function_getReadPosition(std::shared_ptr<mdox_object> obj, std::vector<Value>& v);
	static bool MDOX_Class_File_Function_getSize(std::shared_ptr<mdox_object> obj, std::vector<Value>& v);
	static bool MDOX_Class_File_Function_open(std::shared_ptr<mdox_object> obj, std::vector<Value>& v);
	static bool MDOX_Class_File_Function_isOpen(std::shared_ptr<mdox_object> obj, std::vector<Value>& v);
	static bool MDOX_Class_File_Function_close(std::shared_ptr<mdox_object> obj, std::vector<Value>& v);
	static bool MDOX_Class_File_Function_getChar(std::shared_ptr<mdox_object> obj, std::vector<Value>& v);
	static bool MDOX_Class_File_Function_isEOF(std::shared_ptr<mdox_object> obj, std::vector<Value>& v);
	static bool MDOX_Class_File_Function_getLine(std::shared_ptr<mdox_object> obj, std::vector<Value>& v);
	static bool MDOX_Class_File_Function_getAll(std::shared_ptr<mdox_object> obj, std::vector<Value>& v);
	static bool MDOX_Class_File_Constructor(std::shared_ptr<mdox_object> obj, std::vector<Value>& v);
	static bool MDOX_Class_File_Function_Static_createFile(std::shared_ptr<mdox_object> obj, std::vector<Value>& v);
};










// ----- CLASES CORE -----
/*
	Esta clase se usará internamente
*/
static class Clase_Core
{
public:
	static Parser_Class* crearClase(std::string&& nombre)
	{
		return new Parser_Class(nombre);
	}

	static int addVariable(Parser_Class* clase, std::string& nombre, Value& val)
	{
		int n = clase->getVariableOperarCore().size();
		clase->getVariableOperarCore().emplace_back(val);
		clase->variables_map.emplace(nombre, n);
		return n;
	}

	static int addVariable(Parser_Class* clase, std::string&& nombre, Value& val)
	{
		int n = clase->getVariableOperarCore().size();
		clase->getVariableOperarCore().emplace_back(val);
		clase->variables_map.emplace(nombre, n);
		return n;
	}

	static void addConstructor(Parser_Class* clase, bool (*constructor)(std::shared_ptr<mdox_object>, std::vector<Value>&))
	{
		clase->core->constructor = constructor;
	}

	static void addStaticVar(Parser_Class* clase, std::string& nombre, Value& val)
	{
		int a = clase->core->variables_static.size();
		clase->static_var_map.emplace(nombre, a);
		clase->core->variables_static.emplace_back(val);
	}


	static void addOperatorNormal(Parser_Class* clase, OPERADORES operador, bool (*op_exec)(Value*))
	{
		if (clase->getIndexOperator(operador, false) != -1)
			assert(!"ERROR CRITICO INTERNO: Repetido operador dentro de las funciones normales.");
		
		if (clase->core->operadores_map_normal == NULL)
			clase->core->operadores_map_normal = new std::unordered_map<OPERADORES, int>();

		int a = clase->core->operadores.size();
		clase->core->operadores_map_normal->emplace(operador, a);
		clase->core->operadores.emplace_back(op_exec);
	}

	static void addOperatorRight(Parser_Class* clase, OPERADORES operador, bool (*op_exec)(Value*))
	{
		if (clase->getIndexOperator(operador, false) != -1)
			assert(!"ERROR CRITICO INTERNO: Repetido operador dentro de las funciones right.");

		if (clase->core->operadores_map_right == NULL)
			clase->core->operadores_map_right = new std::unordered_map<OPERADORES, int>();

		int a = clase->core->operadores.size();
		clase->core->operadores_map_right->emplace(operador, a);
		clase->core->operadores.emplace_back(op_exec);
	}

	static void addFuncion(Parser_Class* clase, CoreClass_Function&& c)
	{
		int a = clase->core->funciones.size();
		clase->core->function_map.emplace(c.nombre, a);
		clase->core->funciones.emplace_back(c);
	}

	static void addFuncion(Parser_Class* clase, CoreClass_Function& c)
	{
		int a = clase->core->funciones.size();
		clase->core->function_map.emplace(c.nombre, a);
		clase->core->funciones.emplace_back(c);
	}

	static void endClassCore(Parser_Class* clase)
	{
		if(clase->core->constructor == NULL)
			assert(!"ERROR CRITICO INTERNO: Se ha localizado una clase core sin constructor. ");

		clase->preload_var = clase->variables_map.size();
		//clase->preloadFunctions();

		Parser::clases.emplace_back(clase);
	}

};



// ----- FUNCIONES INTERPRETE
bool funcion_print(std::vector<Value>&);
bool funcion_now(std::vector<Value>&);
bool funcion_sleep(std::vector<Value>&);
bool funcion_typeOf(std::vector<Value>&);

// ----- FUNCIONES DE V ATOMICAS

// -------- STRING --------
bool funcion_string_find(Value, std::vector<Value>&);
bool funcion_string_substring(Value, std::vector<Value>&);
bool funcion_string_find(Value, std::vector<Value>&);
bool funcion_string_searchMatch(Value, std::vector<Value>&);
bool funcion_string_toUpper(Value, std::vector<Value>&);
bool funcion_string_toLower(Value, std::vector<Value>&);
bool funcion_string_Replace(Value, std::vector<Value>&);
bool funcion_string_split(Value, std::vector<Value>&);
bool funcion_string_lenght(Value, std::vector<Value>&);
bool funcion_string_match(Value, std::vector<Value>&);
bool funcion_string_startsWith(Value, std::vector<Value>&);

// -------- INT --------
bool funcion_int_lowestBit(Value, std::vector<Value>&);
bool funcion_int_toBinaryString(Value, std::vector<Value>&);
bool funcion_int_highestBit(Value, std::vector<Value>&);
bool funcion_int_bitCount(Value, std::vector<Value>&);
bool funcion_int_toOctalString(Value, std::vector<Value>&);
bool funcion_int_toHexString(Value, std::vector<Value>&);

// -------- LONG LONG --------
bool funcion_lint_lowestBit(Value, std::vector<Value>&);
bool funcion_lint_toBinaryString(Value, std::vector<Value>&);
bool funcion_lint_highestBit(Value, std::vector<Value>&);
bool funcion_lint_bitCount(Value, std::vector<Value>&);
bool funcion_lint_toOctalString(Value, std::vector<Value>&);
bool funcion_lint_toHexString(Value, std::vector<Value>&);


bool funcion_vector_lenght(Value, std::vector<Value>&);
bool funcion_vector_isEmpty(Value, std::vector<Value>&);
bool funcion_vector_contains(Value, std::vector<Value>&);
bool funcion_vector_indexOf(Value, std::vector<Value>&);
bool funcion_vector_replace(Value, std::vector<Value>&);
bool funcion_vector_remove(Value, std::vector<Value>&);
bool funcion_vector_pushBack(Value, std::vector<Value>&);
bool funcion_vector_pushFront(Value, std::vector<Value>&);
bool funcion_vector_popBack(Value, std::vector<Value>&);
bool funcion_vector_popFront(Value, std::vector<Value>&);
bool funcion_vector_insert(Value, std::vector<Value>&);
bool funcion_vector_containsAll(Value, std::vector<Value>&);
bool funcion_vector_containsAllElements(Value, std::vector<Value>&);
bool funcion_vector_isPermutation(Value, std::vector<Value>&);
bool funcion_vector_front(Value, std::vector<Value>&);
bool funcion_vector_back(Value, std::vector<Value>&);




#endif
