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
	int deep = 0;

	Variable(std::string a, Value* b, bool c) : nombre(a), valor(b), fuerte(c) {}
	virtual ~Variable()
	{
		delete valor;
	};
};

class Interprete {
public:
	std::vector<Parser_Funcion*> funciones;
	int deep = 0; //Se usa para las variables.

	bool CargarDatos(Parser* parser);
	bool Interprete::Interpretar(Parser* parser);
	bool Interprete::Interprete_Sentencia(Parser_Sentencia * sentencia, std::vector<Variable*> * variables);
	Variable * Interprete::Interprete_NuevaVariable(Parser_Parametro * par, std::vector<Variable*> * variables, bool existe);

	bool Interprete::ConversionXtoBool(Value * valOp, bool& salida);
	Value_BOOL * Interprete::Condicionales(Parser_Condicional * pCond, std::vector<Variable*> * variables);
	Value_BOOL * Interprete::CondicionalDeDosValores(Value * value1, CondicionalAccionType accion, Value * value2);
	Value * Interprete::Operaciones(Parser_Operacion * pOp, std::vector<Variable*> * variables);
	Value * Interprete::Operaciones(Parser_Operacion * pOp, std::vector<Variable*> * variables, std::vector<OperacionComp*>* componente);
	Value * Interprete::OperacionSobreValores(Value * value1, MATH_ACCION accion, Value * value2);
	Value* Interprete::Transformar_Declarativo_Value(Parser_Declarativo * dec);
	Variable* Interprete::BusquedaVariable(std::string ID, std::vector<Variable*> * variables);
	bool Interprete::EstablecerVariable(Variable * var, Value * value);

   ~Interprete() 
    {
	   for (std::vector<Parser_Funcion*>::iterator it = funciones.begin(); it != funciones.end(); ++it)
	   {
		   delete (*it);
	   }
	};

};

#endif
