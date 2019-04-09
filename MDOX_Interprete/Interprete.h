#ifndef INTERPRETE_H
#define INTERPRETE_H

#include "Interprete_Core.h"
#include "../MDOX/Funciones.h"
#include "../MDOX/Estructuras.h"
#include "../MDOX/Errores.h"
#include "../MDOX/Parser.h"
#include <iostream>
#include <math.h>  




class OperacionComp
{
public:
	Value * val;
	OPERADORES oper;
	bool delete_ready = false;

	OperacionComp( Value* a, OPERADORES b) : val(a), oper(b) {}

	virtual ~OperacionComp()
	{
		delete val;
	};
};

class Variable 
{
public:
	std::string nombre;
	int index;
	//Profundidad de la variable
	int deep = 1;
	Variable(std::string a,  int b, int i) : nombre(a), deep(b), index(i) {}
};

class VariablePreloaded
{
public:
	Value* valor = NULL;

	//Indica si la variable es fuertemente tipada.
	bool fuerte;

	VariablePreloaded(Value* b, bool c) : valor(b), fuerte(c) {}
	VariablePreloaded() : valor(NULL) {}
	virtual ~VariablePreloaded()
	{
		delete valor;
	};
};

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


class Interprete {
private:
	Value * _retorno = NULL;
public:
	static Interprete * instance;

	std::vector<Fichero*> nombre_ficheros; //Nombre de ficheros cargados en la instancia actual del interprete.
	std::vector<Parser_Funcion*> funciones;

	//Variable de retorno actual
	void setRetorno(Value * v) { delete _retorno; _retorno = v; }
	Value * getRetorno() { Value * t = _retorno; _retorno = NULL; return t; }
	Value * viewRetorno() { return _retorno; }
	void nullRetorno() { if (_retorno != NULL) { delete _retorno;  _retorno = NULL; } }

	bool CargarDatos(Parser* parser);
	bool Interpretar(Parser* parser);
	bool Interprete_Sentencia(Parser_Sentencia * sentencia, VariablePreloaded * variables);
	VariablePreloaded * Interprete_NuevaVariable(Parser_Parametro * par, VariablePreloaded * variables);

	Value* ExecFuncion(Valor_Funcion * xFunc, VariablePreloaded * variables);
	bool ConversionXtoBool(Value * valOp);
	Value_BOOL * CondicionalDeDosValores(Value * value1, OPERADORES accion, Value * value2, OutData_Parametros * outData);
	Value * Operaciones(Parser_Operacion * pOp, VariablePreloaded * variables);
	Value * Operaciones(Parser_Operacion * pOp, VariablePreloaded * variables, std::vector<OperacionComp*>* componente);
	Value * OperacionSobreValores(Value * value1, OPERADORES accion, Value * value2, OutData_Parametros * outData);
	Value* Transformar_Declarativo_Value(Parser_Declarativo * dec);
	
	bool EstablecerOperacion(Parser_Operacion * pOp, VariablePreloaded* variables);
	bool EstablecerVariable(VariablePreloaded * var, Value ** value, OutData_Parametros * salidaError);
	bool ValueConversion(Value ** val1, Value ** val2, OutData_Parametros * outData);
	bool GestionarOperacionesPorPrioridad(Parser_Operacion * pOp, std::vector<OperacionComp*>* componente, std::vector<OPERADORES> * operators, OPERADORES_TIPOS op);
	Value_BOOL *  CondicionalLogico(Value * value1, OPERADORES accion, Value * value2);
	Value * EstablecerIgualdad(Operacion_Igualdad*, VariablePreloaded*);
	bool ValueToConsole(Value * v);
	Value * FuncionCore(std::string, OutData_Parametros*, Interprete_Funcion_Entradas*);


	bool PreloadError = false;
	void RemoverVariablesScope(int deep, std::vector<Variable> * variables);
	Variable * BusquedaVariable(std::string ID, std::vector<Variable> * variables);

	void PreLoad();
	void PreLoad_Funcion(Parser_Funcion *);
	void PreLoad_Sentencia(Parser_Sentencia * sentencia, int*, std::vector<Variable> * variables, int deep);
	void PreLoad_Operacion(Parser_Operacion * op, int*, std::vector<Variable> * variables, int deep);
	void PreLoad_Identificador(Parser_Identificador * x, int* total_vars, std::vector<Variable> * variables, int deep, OutData_Parametros * node, bool add = false);
	void PreLoad_Valor(Parser_Valor * pV, int*, std::vector<Variable> * variables, int deep, OutData_Parametros * data);
	void PreLoad_Param(Parser_Parametro * pV, int*, std::vector<Variable> * variables, int deep);



	Interprete()
	{
		Interprete::instance = this;
	}

   ~Interprete() 
    {
	   delete _retorno;
	   for (std::vector<Parser_Funcion*>::iterator it = funciones.begin(); it != funciones.end(); ++it)
	   {
		   delete (*it);
	   }
	};
};



//Interprete * Interprete::instance;

#endif
