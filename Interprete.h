#ifndef INTERPRETE_H
#define INTERPRETE_H

#include "Estructuras.h"
#include "Parser.h"
#include <math.h>  

class OperacionComp
{
public:
	Value * val;
	MATH_ACCION oper;
	bool delete_ready = false;

	OperacionComp( Value* a, MATH_ACCION b) : val(a), oper(b) {}

	virtual ~OperacionComp()
	{
		delete val;
	};
};


class OperacionCompCond
{
public:
	Value_BOOL * val;
	CondicionalAccionType oper;
	bool delete_ready = false;

	OperacionCompCond(Value_BOOL* a, CondicionalAccionType b) : val(a), oper(b) {}

	virtual ~OperacionCompCond()
	{
		delete val;
	};

};


class Variable 
{
public:
	std::string nombre;
	Value* valor;

	//Indica si la variable es fuertemente tipada.
	bool fuerte;
	//Profundidad de la variable
	int deep = 1;

	Variable(std::string a, Value* b, bool c) : nombre(a), valor(b), fuerte(c) {}
	virtual ~Variable()
	{
		delete valor;
	};

	Variable * Clone()
	{
		Variable * var = new Variable(nombre, valor->Clone(), fuerte);
		var->deep = deep;
		return var;
	}

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
	std::vector<Parser_Funcion*> funciones;

	//Variable de retorno actual
	void setRetorno(Value * v) { delete _retorno; _retorno = v; }
	Value * getRetorno() { Value * t = _retorno; _retorno = NULL; return t; }
	void nullRetorno() { delete _retorno;  _retorno = NULL; }

	int deep = 1; //Se usa para las variables.

	bool CargarDatos(Parser* parser);
	bool Interprete::Interpretar(Parser* parser);
	bool Interprete::Interprete_Sentencia(Parser_Sentencia * sentencia, std::vector<Variable*> * variables);
	Variable * Interprete::Interprete_NuevaVariable(Parser_Parametro * par, std::vector<Variable*> * variables, bool existe);

	Value* Interprete::ExecFuncion(std::string ID, Valor_Funcion * xFunc, std::vector<Variable*> * variables);
	bool Interprete::ConversionXtoBool(Value * valOp, bool& salida);
	Value_BOOL * Interprete::Condicionales(Parser_Condicional * pCond, std::vector<Variable*> * variables);
	Value_BOOL * Interprete::CondicionalDeDosValores(Value * value1, CondicionalAccionType accion, Value * value2);
	Value * Interprete::Operaciones(Parser_Operacion * pOp, std::vector<Variable*> * variables);
	Value * Interprete::Operaciones(Parser_Operacion * pOp, std::vector<Variable*> * variables, std::vector<OperacionComp*>* componente);
	Value * Interprete::OperacionSobreValores(Value * value1, MATH_ACCION accion, Value * value2);
	Value* Interprete::Transformar_Declarativo_Value(Parser_Declarativo * dec);
	Variable* Interprete::BusquedaVariable(std::string ID, std::vector<Variable*> * variables);
	bool Interprete::EstablecerVariable(Variable * var, Value ** value);

   ~Interprete() 
    {
	   delete _retorno;
	   for (std::vector<Parser_Funcion*>::iterator it = funciones.begin(); it != funciones.end(); ++it)
	   {
		   delete (*it);
	   }
	};

};

#endif
