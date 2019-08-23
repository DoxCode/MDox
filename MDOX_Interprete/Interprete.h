#ifndef INTERPRETE_H
#define INTERPRETE_H

//#include "../MDOX/Funciones.h"
//#include "Estructuras.h"
#include "Parser.h"
//#include "../MDOX/Parser.h"
#include <array>
#include <iostream>
#include <math.h>  



//#include "../MDOX/Core.h"
#include <chrono>
#include <thread>
/*
class Interprete_Funcion_Entradas
{
public:
	std::vector<Value*>* entradas = NULL;

	Interprete_Funcion_Entradas(std::vector<Value*>* a) : entradas(a) {}

	virtual ~Interprete_Funcion_Entradas()
	{
		for (std::vector<Value*>::iterator it = entradas->begin(); it != entradas->end(); ++it)
		{
			if(*it != NULL)
				deletePtr (*it);
		}
		entradas->clear();
		delete entradas;
	}
};
*/

class Variable_Runtime
{
public:
	Value value;
	bool fuerte;

	Variable_Runtime() : fuerte(false) {}
	Variable_Runtime(Value a) : value(a), fuerte(false) {}
	Variable_Runtime(Value a, bool b) : value(a), fuerte(b) {}
};

//Objeto producido por alguna clase
class mdox_object
{
public:
	//Clase de la cual proviene el objeto
	Parser_Class* clase;
	Variable_Runtime* variables_clase; //Variable prefijadas

	mdox_object(Parser_Class* c) : clase(c) {};

	~mdox_object()
	{
		delete[] variables_clase;
	}
};


class Interprete {
private:
	bool return_activo = false;
	bool break_activo = false;
	bool continue_activo = false;
	bool ignore_activo = false;


	Value _retorno;
public:
	static Interprete* instance;

	Variable_Runtime* variables_globales;


	std::vector<Fichero*> nombre_ficheros; //Nombre de ficheros cargados en la instancia actual del interprete.
	std::vector<Parser_Funcion*> funciones;
	std::vector<Parser_Class*> clases;

	//Retorno continue
	bool ignoreCalled() { if (ignore_activo) { ignore_activo = false; return true; } return false; }
	//Retorno continue
	bool continueCalled() { if (continue_activo) { continue_activo = false; return true; } return false; }
	//Retorno break
	bool breakCalled() { if (break_activo) { break_activo = false; return true; } return false; }
	//Variable de retorno actual
	void setRetorno(Value& v) { _retorno = v; return_activo = true; }
	bool returnCalled() { if (return_activo) { return_activo = false; return true; } return false; }
	void retornoSinValor() { _retorno = std::monostate(); return_activo = true; }
	Value getRetorno() { return _retorno; }


	ValueCopyOrRef tipoValorToValueOrRef(tipoValor& a, Variable_Runtime* variables, Variable_Runtime* var_clase, Parser_Identificador** ret = NULL);
	//bool OperacionOperadoresVectores(multi_value*, multi_value*, OPERADORES& op, Variable_Runtime* variables);
	bool OperacionOperadoresVectores(Value*, multi_value*, OPERADORES& operador, Variable_Runtime* variables, Variable_Runtime* var_clase, bool& isPop, bool& left, Parser_Identificador* f1 = NULL, Parser_Identificador* f2 = NULL);
	bool OperacionOperadoresVectores(multi_value*, Value*, OPERADORES& operador, Variable_Runtime* variables, Variable_Runtime* var_clase, bool& isPop, bool& left, Parser_Identificador* f1 = NULL, Parser_Identificador* f2 = NULL);
	bool OperacionOperadoresVectores(Value*, Value*, OPERADORES& operador, bool& isPop, bool& left, Parser_Identificador* f1 = NULL, Parser_Identificador* f2 = NULL);

	Value lectura_arbol_operacional(arbol_operacional* node, Variable_Runtime* variables, Variable_Runtime* var_clase);
	//void setRetorno(Value * v) { delete _retorno; _retorno = v; }
	//Value * getRetorno() { Value * t = _retorno; _retorno = NULL; return t; }
	//Value * viewRetorno() { return _retorno; }
	//void nullRetorno() { if (_retorno != NULL) { delete _retorno;  _retorno = NULL; } }

	Value TratarMultiplesValores(multi_value* arr, Variable_Runtime* variables, Variable_Runtime* var_clase);
	bool CargarDatos(Parser* parser);
	void Interpretar(Parser* parser);
	bool Interprete_Sentencia(Parser_Sentencia* sentencia, Variable_Runtime* variables, Variable_Runtime* var_clase);

	//VariablePreloaded * Interprete_NuevaVariable(Parser_Parametro * par, VariablePreloaded * variables);

	std::vector<Value> transformarEntradasCall(Call_Value* vF, Variable_Runtime* variables, Variable_Runtime* var_clase);
	bool Relacional_rec_arbol(arbol_operacional* node, Variable_Runtime* variables, Variable_Runtime* var_clase, Value& val_r);

	Value ExecOperador(Operator_Class* oc, Value& v, Variable_Runtime* var_class);
	Value ExecOperador(Operator_Class* oc, Variable_Runtime* var_class);
	Value ExecClass(Call_Value* vf, std::vector<Value>& entradas);
	Value ExecFuncion(Call_Value* vf, std::vector<Value>& entradas, Variable_Runtime* var_class);
	bool FuncionCore(Call_Value* vf, std::vector<Value>& entradas);


	Interprete()
	{
		Interprete::instance = this;
	}

	~Interprete()
	{

		for (std::vector<Parser_Funcion*>::iterator it = funciones.begin(); it != funciones.end(); ++it)
		{
			delete (*it);
		}
	};
};

class Core_Function_Interprete : public Core_Function
{
public:
	bool (*funcion_exec)(std::vector<Value>&, Interprete*);

	Core_Function_Interprete(std::string a, std::vector<tipos_parametros> b, tipos_parametros c) : Core_Function(a, b, c) {}

	~Core_Function_Interprete() {};
};


//Interprete * Interprete::instance;

#endif
