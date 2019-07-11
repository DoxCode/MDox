#ifndef PARSER_H
#define PARSER_H



#include "Errores.h"
//#include "Tokenizer.h"


class Variable
{
public:
	std::string nombre;
	int index;

	Variable(std::string a, int i) : nombre(a), index(i) {}
};

class Parser {
protected:
public:
	Tokenizer tokenizer;
	
	bool existenErrores = false;

	//Cacheado de variables
	//Variales globales aún no implementadas.
	std::vector<Variable> variables_globales;
	bool isGlobal = false;

	int numero_variables_globales = 0;
	int numero_variables_funcion = 0;

	Parser() {  }

	//Funciones detectoras de parametros
	Value getLiteral(bool& v, int& local_index);
	Parser_Declarativo* getDeclarativo(int& local_index);
	Parser_Identificador* getIdentificador(int& local_index);
	conmp getValor(bool& v, int& local_index, std::vector<Variable>& variables);
	Parser_Operacion* getOperacion(int& local_index, std::vector<Variable>& variables);
	OPERADORES getOperador(int& local_index);
	Parser_Sentencia* getSentencia(int& local_index, std::vector<Variable>& variables);
	Parser_Funcion* getFuncion(int& local_index);

	//Cacheado de variables
	Variable* BusquedaVariable(Parser_Identificador * ID, std::vector<Variable>& variables);
	void clearVariables() { variables_globales.clear(); numero_variables_globales = 0; }
	void CargarEnCacheOperaciones(arbol_operacional* node, std::vector<Variable>& variables);
	void IncrementarVariables() { isGlobal ? numero_variables_globales++ : numero_variables_funcion++; }
	int getLastIndex() { return isGlobal ? numero_variables_globales : numero_variables_funcion; }

};

class Core_Function
{
public:
	std::string nombre;
	std::vector<tipos_parametros> entradas;
	tipos_parametros salida;

	Core_Function(std::string a, std::vector<tipos_parametros> b, tipos_parametros c) : nombre(a), entradas(b), salida(c) {}

	~Core_Function()
	{};
};

class Core
{
public:
	static std::vector<Core_Function*> core_functions;
	static void Start();

};

#endif