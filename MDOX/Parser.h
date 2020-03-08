#ifndef PARSER_H
#define PARSER_H



#include "Errores.h"
//#include "Tokenizer.h"


class Variable
{
public:
	std::string nombre;
	int index;
	// Cacheado para comprobar si la variable fue inicializada en algun momento.
	// Luego se usará para dar valor al identificador de turno.
	bool inicializando = false;

	Variable(std::string a, int i) : nombre(a), index(i) {}
	Variable(std::string a, int i, bool init) : nombre(a), index(i), inicializando(init) {}
};

class SendVariables
{
private:
	int _num_local_var;
public:
	std::vector<Variable> variables_locales;
	std::vector<Variable>* variables_clase;

	int* num_local_var;

	bool existClaseVariable() 
	{
		return variables_clase != NULL;
	}

	void push_VarLocal(Variable& v)
	{
		variables_locales.emplace_back(v);
		(*num_local_var)++;
	}


	SendVariables() : variables_clase(NULL), _num_local_var(0), num_local_var(&_num_local_var) {};
	SendVariables(std::vector<Variable>& a) : variables_clase(&a), _num_local_var(0), num_local_var(&_num_local_var) {};
	SendVariables(std::vector<Variable>* a) :variables_clase(a), _num_local_var(0), num_local_var(&_num_local_var) {};
	SendVariables(SendVariables& a) : variables_locales(a.variables_locales), variables_clase(a.variables_clase), num_local_var(a.num_local_var) {};
};

class Parser {
protected:
public:
	Tokenizer tokenizer;

	//static Variable_Runtime* variablesMain; //Variables del scope superior que llamaremos MAIN
	//static Variable_Runtime* variables_globales; //Variable globales, aplicables en todo el entorno

	static std::vector<Fichero*> nombre_ficheros; //Nombre de ficheros cargados en la instancia actual del interprete.
	static std::vector<Parser_Funcion*> funciones;
	static std::vector<Parser_Class*> clases;

	static SendVariables variables_scope; //Usado para el transpaso de variables por ámbito

	bool existenErrores = false;
	
	bool readingFunction = false;
	//bool readingStaticFunction = false;
	//bool readingStaticVariable = false;
	bool readingStaticValue = false;

	//readingClass devolverá donde se está ejecutando el parser, será nulo si es a nivel de raiz o de función de raiz
	//y devolverá una clase en caso de estar leyendo una clase.
	//Este valor se usará para las llamadas a funciones.
	Parser_Class* readingClass = nullptr;

	static std::vector<Variable>* variables_globales_parser;


	bool Parser::GenerarArbol();
	//Variables y sentencias del scope superior que llamaremos Main
	std::vector<Parser_Sentencia*> sentenciasMain;
	//std::vector<Variable> variablesMain;
	bool isMain = false;

	std::vector<Call_Value*> valores_llamadas;


	
	Parser() 
	{  
		if (Parser::variables_globales_parser == NULL)
			Parser::variables_globales_parser = new std::vector<Variable>();
	}

	//Funciones detectoras de parametros
	Value getLiteral(bool& v, int& local_index);
	Parser_Declarativo* getDeclarativo(int& local_index);
	Parser_Identificador* getIdentificador(int& local_index);
	//multi_value* getValorLista(int& local_index, std::vector<Variable>& variables);
	//conmp getValorItr(bool& ret, int& local_index, std::vector<Variable>& variables);
	Parser_Identificador* getVariableID(int& local_index, SendVariables& variables);
	conmp getValor(bool& v, int& local_index, SendVariables& variables);
	multi_value* getValorList(bool& all_value, int& local_index, SendVariables& variables);
	arbol_operacional* getOperacion(int& local_index, SendVariables& variables, bool inside = false);
	arbol_operacional* getOperacionInd(int& local_index, SendVariables& variables, bool inside = false, bool isPublic = true);
	OPERADORES getOperador(int& local_index);
	Parser_Sentencia* getSentencia(int& local_index, SendVariables& variables);
	Parser_Funcion* getFuncion(int& local_index, std::vector<Variable>* class_var = NULL);
	Parser_Class* getClass(int& local_index);
	etiquetas_class getLabelClass(int& local_index);
	Parser_ClassConstructor* getClassConstructor(int& local_index, std::vector<Variable>& variable);
	bool getClassOperadores(int& local_index, Parser_Class* clase, std::vector<Variable>& variables_clases);
	void PreloadStaticCalls(std::vector<Parser_Class*>* clases);

	bool preloadCalls(std::vector<Parser_Funcion*>&, std::vector<Parser_Class*>* a = NULL);

	//Cacheado de variables
	Variable* BusquedaVariableLocal(Parser_Identificador* ID, std::vector<Variable>& variables);
	int BusquedaVariable(Parser_Identificador * ID, SendVariables& variables);
	int BusquedaVariable(Parser_Identificador* ID, SendVariables& variables, Variable**);
	void CargarEnCacheOperaciones(arbol_operacional* node, SendVariables& variables, bool inside);
//	void IncrementarVariables() { isGlobal ? numero_variables_globales++ : numero_variables_funcion++; }
	//int getLastIndex() { return isGlobal ? numero_variables_globales : numero_variables_funcion; }

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