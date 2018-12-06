#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include <string>
#include <vector>
#include <list>
#include <tuple>
#include "Funciones.h"

#include<stdio.h>
#include<stdlib.h>
#include <iostream>
#include <memory>

enum tipo_permanente
{
	TP_ETIQUETA,
	TP_INCLUDE,
	TP_FUNCION,
};


enum metaetiquetas_codigo
{
	ME_PSEUDOCODIGO,
	ME_MDOX,
};



/* // Para pre-11 C++
template<class ENUM, class UNDERLYING = typename std::underlying_type<ENUM>::type>
class SafeEnum
{
public:
	SafeEnum() : mFlags(0) {}
	SafeEnum(ENUM singleFlag) : mFlags(singleFlag) {}
	SafeEnum(const SafeEnum& original) : mFlags(original.mFlags) {}

	SafeEnum&   operator |=(ENUM addValue) { mFlags |= addValue; return *this; }
	SafeEnum    operator |(ENUM addValue) { SafeEnum  result(*this); result |= addValue; return result; }
	SafeEnum&   operator &=(ENUM maskValue) { mFlags &= maskValue; return *this; }
	SafeEnum    operator &(ENUM maskValue) { SafeEnum  result(*this); result &= maskValue; return result; }
	SafeEnum    operator ~() { SafeEnum  result(*this); result.mFlags = ~result.mFlags; return result; }
	explicit operator bool() { return mFlags != 0; }

protected:
	UNDERLYING  mFlags;
};
*/

enum Flags_lectura
{
	FLAG_LECT_NONE = 0,
	FLAG_LECT_2 = (1 << 0), //2
	FLAG_LECT_3 = (1 << 1), //4
	FLAG_LECT_4 = (1 << 2), //8
	FLAG_LECT_5 = (1 << 3), //16
	FLAG_LECT_6 = (1 << 4), //32

};

//Templates para flags.

template<class T> inline T operator~ (T a) { return (T)~(int)a; }
template<class T> inline T operator| (T a, T b) { return (T)((int)a | (int)b); }
template<class T> inline T operator& (T a, T b) { return (T)((int)a & (int)b); }
template<class T> inline T operator^ (T a, T b) { return (T)((int)a ^ (int)b); }
template<class T> inline T& operator|= (T& a, T b) { return (T&)((int&)a |= (int)b); }
template<class T> inline T& operator&= (T& a, T b) { return (T&)((int&)a &= (int)b); }
template<class T> inline T& operator^= (T& a, T b) { return (T&)((int&)a ^= (int)b); }

/*
Ejemplos:
test_flags = test_flags ^ FLAG_LECT_COMENTARIO ^ FLAG_LECT_2; // De test_flags quitamos FLAG_LECT_COMENTARIO ^ FLAG_LECT_2
test_flags = FLAG_LECT_4 | FLAG_LECT_3; // test_flags ponemos FLAG_LECT_4 FLAG_LECT_3

if(test_flags & FLAG_LECT_COMENTARIO) si existe
if ((test_flags & FLAG_LECT_3) == FLAG_LECT_NONE) si NO existe

*/



// Para pre-11 C++
//typedef SafeEnum<enum _Flags_lectura>  Flags_lectura;

enum tipos_parametros
{
	PARAM_VOID,

	//ENTEROS
	PARAM_INT,	 // -2.147.483.647 a 2.147.483.647   (4 bytes)
	PARAM_UINT,	 // 0 a 4.294.967.295				 (4 bytes)
	PARAM_INT16,   // De –32.768 a 32.767              (2 bytes)
	PARAM_UINT16,  // De 0 a 65.535                    (2 bytes)
	PARAM_INT64,   // De –9.223.372.036.854.775.808 a 9.223.372.036.854.775.807  (8 bytes)
	PARAM_UINT64,   // De 0 a 18.446.744.073.709.551.615  (8 bytes)

	PARAM_CHAR,

	PARAM_FLOAT,   // 7 digitos						 (4 bytes)
	PARAM_DOUBLE,  // 15 digitos						 (8 bytes)
	PARAM_BOOL,   //  true/false                       (1 byte)
	PARAM_STRUCT, //Diseñado por el usuario.


	PARAM_ENUM,
	PARAM_STRING,
	PARAM_LIST,
	PARAM_VECTOR,
	PARAM_DEQUE,
	PARAM_TUPLA,
};

enum NODE_type {
	NODE_NULL,
	NODE_SENTENCIA,
	NODE_FUNCION,
};


class Parser_NODE { 
public: 
	int linea;
	int offset;
	virtual ~Parser_NODE() { }; 
	virtual NODE_type node_type() { return NODE_NULL; };
};


// ############################################################
// ################# PARAMETROS DE ENTRADA #################### 
// ############################################################

enum EntradaType {
	ENTRADA_OP,
	ENTRADA_PARAM,
};

class Funcion_ValorEntrada
{
public:
	EntradaType tipo;

	Funcion_ValorEntrada(EntradaType a) : tipo(a) {}

	//Aseguramos el borrado de la memoria
	virtual ~Funcion_ValorEntrada() {
	};
};

// ###################################################
// ################## DECLARATIVOS ###################
// ###################################################

enum DeclarativeType
{
	DEC_SINGLE,
	DEC_MULTI,
};

class D_Value { public: virtual ~D_Value() { }; };

class Parser_Declarativo {
public:
	D_Value* value;
	DeclarativeType tipo;

	Parser_Declarativo(DeclarativeType a, D_Value* b) : tipo(a), value(b) {}

	//Aseguramos el borrado de la memoria
	virtual ~Parser_Declarativo() {
		delete value;
	};
};

class Declarativo_SingleValue : public D_Value {
public:
	tipos_parametros value;
	Declarativo_SingleValue(tipos_parametros a) : value(a) {}
};

class Declarativo_MultiValue : public D_Value {
public:
	tipos_parametros tipo;
	std::vector<Parser_Declarativo*> value;
	Declarativo_MultiValue(tipos_parametros a, std::vector<Parser_Declarativo*> b) : value(b), tipo(a) {}

	//Aseguramos el borrado de la memoria cuando se libere el objeto.
	virtual ~Declarativo_MultiValue()
	{
		for (std::vector<Parser_Declarativo*>::iterator it = value.begin(); it != value.end(); ++it)
		{
			delete (*it);
		}
		value.clear();
	}
};
// ########################################################
// ####################### VALOR ##########################
// ########################################################
enum ValorType
{
	VAL_ID,
	VAL_LIT,
	VAL_FUNC,
};

class Parser_Valor {
public:
	bool negado = false;
	ValorType tipo;
	Parser_Valor(ValorType a) : tipo(a) {}
	Parser_Valor(ValorType a, bool b) : tipo(a), negado(b) {}

	virtual ~Parser_Valor() {}
};

// ############################################################
// ####################### PARAMETRO ##########################
// ############################################################
enum ParamType {

	PRM_DECLARATIVO_ID,
	PRM_ID,
	PRM_TUPLA,
	PRM_ARRAY,
	PRM_PUNTERO,
	PRM_PUNTERO_INT,
};

class Parser_Parametro : public Funcion_ValorEntrada
{
public:
	ParamType tipo;
	Parser_Parametro(ParamType a) : tipo(a), Funcion_ValorEntrada(ENTRADA_PARAM) {}
	//Aseguramos el borrado de la memoria
	virtual ~Parser_Parametro() {};
};

// ################################################################
// ####################### IDENTIFICADOR ########################## 
// ################################################################
class Parser_Identificador : public Parser_Valor, public Parser_Parametro {
public:
	std::string nombre;
	Parser_Identificador(std::string a) : nombre(a), Parser_Valor(VAL_ID), Parser_Parametro(PRM_ID) {}
	virtual ~Parser_Identificador() {};
};

// ################################################################
// ######################### OPERACIONES ########################## 
// ################################################################
enum OprtType
{
	OP_REC_OP,
	OP_ID,
	OP_MATH,
};

class Parser_Operacion : public Funcion_ValorEntrada {
public:
	OprtType tipo;
	Parser_Operacion(OprtType a) : tipo(a), Funcion_ValorEntrada(ENTRADA_OP) {}

	//Aseguramos el borrado de la memoria
	virtual ~Parser_Operacion()
	{}
};

// ############################################################
// #################### PARAMETROS POLY #######################
// ############################################################

class Parametro_Declarativo_ID : public Parser_Parametro {
public:
	Parser_Declarativo * pDec;
	Parser_Identificador * pID;

	Parametro_Declarativo_ID(Parser_Declarativo * a, Parser_Identificador * b) : pDec(a), pID(b), Parser_Parametro(PRM_DECLARATIVO_ID) {}

	//Aseguramos el borrado de la memoria
	virtual ~Parametro_Declarativo_ID() {
		delete pDec;
		delete pID;
	};
};

class Parametro_Tupla : public Parser_Parametro {
public:
	std::vector<Parser_Operacion*> value;

	Parametro_Tupla(std::vector<Parser_Operacion*> a) : value(a), Parser_Parametro(PRM_TUPLA) {}

	//Aseguramos el borrado de la memoria
	virtual ~Parametro_Tupla() {
		for (std::vector<Parser_Operacion*>::iterator it = value.begin(); it != value.end(); ++it)
		{
			delete (*it);
		}
		value.clear();
	};
};

// ###############################################################
// ######################### VALOR PLY ###########################
// ###############################################################

//Basicamente se trataría de funciones con un retorno de un valor dado.
class Valor_Funcion : public Parser_Valor {
public:
	Parser_Identificador* ID;
	std::vector<Parser_Operacion*> entradas;

	Valor_Funcion(Parser_Identificador * a, std::vector<Parser_Operacion*> b) : ID(a), entradas(b), Parser_Valor(VAL_FUNC)  {}

	virtual ~Valor_Funcion() 
	{ 
		delete ID; 

		for (std::vector<Parser_Operacion*>::iterator it = entradas.begin(); it != entradas.end(); ++it)
		{
			delete (*it);
		}
		entradas.clear();
	};
};
// ############################################################
// ########################## MATH ############################ 
// ############################################################

enum MATH_ACCION {

	MATH_NONE,
	MATH_SUMA,
	MATH_MULT,
	MATH_DIV,
	MATH_DIV_ENTERA,
	MATH_RESTA,
	MATH_EXP,
	MATH_MOD,
};

class Parser_Math
{
public:
	MATH_ACCION accion;
	Parser_Operacion * op;

	Parser_Math(MATH_ACCION a, Parser_Operacion * b) : accion(a), op(b) {}

	virtual ~Parser_Math() {
		delete op;
	};
};

// ###############################################################
// ####################### OPERACIÓN PLY ######################### 
// ###############################################################

class Operacion_Recursiva : public Parser_Operacion
{
public:
	Parser_Operacion* op1 = NULL;
	Parser_Math* op2 = NULL;

	Operacion_Recursiva(Parser_Operacion * a, Parser_Math * b) : op1(a), op2(b), Parser_Operacion(OP_REC_OP) {}
	Operacion_Recursiva(Parser_Operacion * a) : op1(a), op2(NULL), Parser_Operacion(OP_REC_OP) {}

	//Aseguramos el borrado de la memoria cuando se libere el objeto.
	virtual ~Operacion_Recursiva()
	{
		delete op1;
		delete op2;
	}
};

class Operacion_Math : public Parser_Operacion
{
public:
	Parser_Valor * op1;
	Parser_Math * op2;

	Operacion_Math(Parser_Valor * a, Parser_Math * b) : op1(a), op2(b), Parser_Operacion(OP_MATH) {}
	Operacion_Math(Parser_Valor * a) : op1(a), op2(NULL), Parser_Operacion(OP_MATH) {}

	//Aseguramos el borrado de la memoria cuando se libere el objeto.
	virtual ~Operacion_Math()
	{
		delete op1;
		delete op2;
	}
};

enum ID_ACCION {

	ID_INCREMENTO,
	ID_DECREMENTO,
	ID_NEGACION,
};

class Operacion_ID : public Parser_Operacion
{
public:
	Parser_Identificador * ID;
	ID_ACCION accion;

	Operacion_ID(Parser_Identificador * a, ID_ACCION b) : ID(a), accion(b),  Parser_Operacion(OP_ID) {}

	//Aseguramos el borrado de la memoria cuando se libere el objeto.
	virtual ~Operacion_ID()
	{
		delete ID;
	}
};


// ###################################################
// #################### LITERALES ####################
// ###################################################

class Value {
public:
	virtual tipos_parametros getTypeValue() { return PARAM_VOID; };
	virtual ~Value() { };

	Value() { };

	virtual Value * Clone() { return new Value(*this); }
};


class Parser_Literal : public Parser_Valor {
public:
	Value* value;

	Parser_Literal(Value* a) : value(a), Parser_Valor(VAL_LIT) {}

	//Aseguramos el borrado de la memoria
	virtual ~Parser_Literal() {
		delete value;
	};

};

class Value_DOUBLE : public Value
{
public:
	double value = 0;
	virtual tipos_parametros getTypeValue() { return PARAM_DOUBLE; };
	Value_DOUBLE(double a) : value(a) {}
	Value_DOUBLE() {}
	virtual Value * Clone() { return new Value_DOUBLE(*this); }
};

class Value_INT : public Value
{
public:
	int value = 0;
	virtual tipos_parametros getTypeValue() { return PARAM_INT; };
	Value_INT() {}
	Value_INT(int a) : value(a) {}
	virtual Value * Clone() { return new Value_INT(*this); }
};

class Value_STRING : public Value
{
public:
	std::string value = "";
	virtual tipos_parametros getTypeValue() { return PARAM_STRING; };
	Value_STRING() {}
	Value_STRING(std::string a) : value(a) {}
	virtual Value * Clone() { return new Value_STRING(*this); }
};

class Value_BOOL : public Value
{
public:
	bool value = false;
	virtual tipos_parametros getTypeValue() { return PARAM_BOOL; };
	Value_BOOL(bool a) : value(a) {}
	virtual Value * Clone() { return new Value_BOOL(*this); }
};

class Value_TUPLA : public Value
{
public:
	std::vector<Value*> value;

	Value_TUPLA(std::vector<Value*> a) : value(a) {}

	virtual tipos_parametros getTypeValue() { return PARAM_TUPLA; };
	virtual ~Value_TUPLA()
	{
		for (std::vector<Value*>::iterator it = value.begin(); it != value.end(); ++it)
		{
			delete (*it);
		}
	};
	virtual Value * Clone() { return new Value_TUPLA(*this); }
};

// ############################################################
// ###################### CONDICIONAL ######################### 
// ############################################################

enum CondicionalType {
	COND_REC,
	COND_OP,
};

enum CondicionalAgregadoType {
	COND_NONE,
	COND_AG_AND,
	COND_AG_OR,
};

enum CondicionalAccionType {
	COND_ACC_NONE,
	COND_ACC_EQUAL,
	COND_ACC_NOT_EQUAL,
	COND_ACC_MINOR,
	COND_ACC_MAJOR,
	COND_ACC_MINOR_OR_EQUAL,
	COND_ACC_MAJOR_OR_EQUAL,
};


class Parser_Condicional {
public:
	CondicionalType tipo;

	Parser_Condicional(CondicionalType a) : tipo(a) {};

	//Aseguramos el borrado de la memoria
	virtual ~Parser_Condicional() {
	};
};

//Condición agregada a otra condición para formar cadenas condicionales AND, OR ->   &&/|| <COND>
class Condicional_Agregada {
public:
	CondicionalAgregadoType accion;
	Parser_Condicional* cond;

	Condicional_Agregada(CondicionalAgregadoType a, Parser_Condicional* b) : accion(a), cond(b) {};
	virtual ~Condicional_Agregada() {
		delete cond;
	}
};

class Condicional_Agregada_Operacional {
public:
	CondicionalAccionType AccType;
	Parser_Operacion* op;

	Condicional_Agregada_Operacional(CondicionalAccionType a, Parser_Operacion* b) : AccType(a), op(b) {};
	virtual ~Condicional_Agregada_Operacional() {
		delete op;
	}
};

class Condicional_Recursiva : public Parser_Condicional {
public:
	bool negada;
	Parser_Condicional * c1;
	Condicional_Agregada * ca;

	//Ruta sin el segundo condicional: !(<COND>)
	Condicional_Recursiva(bool a, Parser_Condicional * b) : negada(a), c1(b), ca(NULL), Parser_Condicional(COND_REC) {};
	//Ruta con una condicional adicional agregada:   !(<COND>) &&/|| <COND>
	Condicional_Recursiva(bool a, Parser_Condicional * b, Condicional_Agregada * c) : negada(a), c1(b), ca(c), Parser_Condicional(COND_REC) {};

	//Aseguramos el borrado de la memoria
	virtual ~Condicional_Recursiva() {
		delete c1;
		delete ca;
	}
};

class Condicional_Operacion : public Parser_Condicional {
public:
	Parser_Operacion * op1;
	std::vector<Condicional_Agregada_Operacional*>* adicional_ops;
	Condicional_Agregada * ca;

	//Ruta de salida simple, solamente valor de operación:  <OP>
	Condicional_Operacion(Parser_Operacion * a) : op1(a), adicional_ops(NULL), ca(NULL), Parser_Condicional(COND_OP) {};
	//Ruta de salida con más de un valor operacional: <OP> <= <OP> < <OP> == <OP> ->  (a <= 5 < b == c)
	Condicional_Operacion(Parser_Operacion * a, std::vector<Condicional_Agregada_Operacional*>* b) : op1(a), adicional_ops(b), ca(NULL), Parser_Condicional(COND_OP) {};
	//Ruta de salida con más de un valor operacional y una condición agregada: <OP> <= <OP> < <OP> || <COND> ->  (a <= 5 < b) || c
	Condicional_Operacion(Parser_Operacion * a, std::vector<Condicional_Agregada_Operacional*>* b, Condicional_Agregada* c) : op1(a), adicional_ops(b), ca(c), Parser_Condicional(COND_OP) {};

	//Aseguramos el borrado de la memoria
	virtual ~Condicional_Operacion() {
		delete op1;
		delete ca;

		if (adicional_ops)
		{
			for (std::vector<Condicional_Agregada_Operacional*>::iterator it = adicional_ops->begin(); it != adicional_ops->end(); ++it)
			{
				delete (*it);
			}
			adicional_ops->clear();
			delete adicional_ops;
		}
	}
};

// ############################################################
// ####################### IGUALDAD ########################### 
// ############################################################

enum IgualdadType {
	IG_EQUAL,
	IG_ADD_EQ,
	IG_SUB_EQ,
};

class Parser_Igualdad {
public:
	Parser_Parametro* param;
	Parser_Condicional* cond;
	IgualdadType valor;

	//Igualdad con valor en la derecha CONDICIONAL si es OPERACIONAL, COND solo tendrá el valor de la operacion.
	Parser_Igualdad(Parser_Parametro* a, Parser_Condicional* b, IgualdadType c) : param(a), cond(b), valor(c) {};

	//Aseguramos el borrado de la memoria
	~Parser_Igualdad() {

		bool p = false; bool c = false;
		if (cond) c = true;
		if (param) p = true;
		delete cond;


		p = false;  c = false;
		if (cond) c = true;
		if (param) p = true;
		deletePtr(param);
	};
};

// ############################################################
// ####################### SENTENCIA ########################## 
// ############################################################

enum SentenciaType {
	SENT_REC,
	SENT_IF,
	SENT_WHILE,
	SENT_FOR,
	SENT_RETURN,
	SENT_PRINT,
	SENT_OP,
	SENT_IGU,
	SENT_VAR_INI,

};

class Parser_Sentencia : public Parser_NODE {
public:
	SentenciaType tipo;

	Parser_Sentencia(SentenciaType a) : tipo(a) {}

	virtual NODE_type node_type() { return NODE_SENTENCIA; }

	//Aseguramos el borrado de la memoria
	virtual ~Parser_Sentencia() {
	};
};

class Sentencia_Recursiva : public Parser_Sentencia {
public:
	std::vector<Parser_Sentencia*> valor;

	Sentencia_Recursiva(std::vector<Parser_Sentencia*> a) : valor(a), Parser_Sentencia(SENT_REC) {}

	//Aseguramos el borrado de la memoria
	virtual ~Sentencia_Recursiva() {
		for (std::vector<Parser_Sentencia*>::iterator it = valor.begin(); it != valor.end(); ++it)
		{
			delete (*it);
		}
		valor.clear();
	}
};

class Sentencia_IF : public Parser_Sentencia {
public:
	Parser_Condicional * pCond;
	Parser_Sentencia * pS;
	Parser_Sentencia * pElse;

	// Sentencia IF sin ELSE asignado.
	Sentencia_IF(Parser_Condicional * a, Parser_Sentencia* b) : pCond(a), pS(b), pElse(NULL), Parser_Sentencia(SENT_IF) {}
	// Sentencia IF con ELSE asignado
	Sentencia_IF(Parser_Condicional * a, Parser_Sentencia* b, Parser_Sentencia* c) : pCond(a), pS(b), pElse(c), Parser_Sentencia(SENT_IF) {}

	//Aseguramos el borrado de la memoria
	virtual ~Sentencia_IF() {
		delete pCond;
		delete pS;
		delete pElse;
	}
};

class Sentencia_WHILE : public Parser_Sentencia {
public:
	Parser_Condicional * pCond;
	Parser_Sentencia * pS;

	Sentencia_WHILE(Parser_Condicional * a, Parser_Sentencia* b) : pCond(a), pS(b), Parser_Sentencia(SENT_WHILE) {}
	
	//Aseguramos el borrado de la memoria
	virtual ~Sentencia_WHILE() {
		delete pCond;
		delete pS;
	}
};

class Sentencia_FOR : public Parser_Sentencia {
public:
	Parser_Igualdad * pIguald;
	Parser_Condicional * pCond;
	Parser_Operacion * pOp;
	Parser_Sentencia * pS;

	Sentencia_FOR(Parser_Igualdad * a, Parser_Condicional * b, Parser_Operacion * c, Parser_Sentencia * d) : pIguald(a), pCond(b), pOp(c), pS(d), Parser_Sentencia(SENT_FOR) {};

	//Aseguramos el borrado de la memoria
	virtual ~Sentencia_FOR() {
		delete pIguald;
		delete pOp;
		delete pCond;
		delete pS;
	}
};

class Sentencia_Return : public Parser_Sentencia {
public:
	Parser_Operacion * pOp;

	Sentencia_Return(Parser_Operacion * a) : pOp(a), Parser_Sentencia(SENT_RETURN) {}

	//Aseguramos el borrado de la memoria
	virtual ~Sentencia_Return() {
		delete pOp;
	}
};

class Sentencia_Print : public Parser_Sentencia {
public:
	Parser_Operacion * pOp;

	Sentencia_Print(Parser_Operacion * a) : pOp(a), Parser_Sentencia(SENT_PRINT) {}

	//Aseguramos el borrado de la memoria
	virtual ~Sentencia_Print() {
		delete pOp;
	}
};

class Sentencia_Operacional : public Parser_Sentencia {
public:
	Parser_Operacion * pOp;

	Sentencia_Operacional( Parser_Operacion * a) : pOp(a), Parser_Sentencia(SENT_OP) {}

	//Aseguramos el borrado de la memoria
	virtual ~Sentencia_Operacional() {
		delete pOp;
	}
};

class Sentencia_Igualdad : public Parser_Sentencia {
public:
	Parser_Igualdad * pIg;

	Sentencia_Igualdad(Parser_Igualdad * a) : pIg(a), Parser_Sentencia(SENT_IGU) {}

	//Aseguramos el borrado de la memoria
	virtual ~Sentencia_Igualdad() {
		delete pIg;
	}
};


class Sentencia_Parametro : public Parser_Sentencia {
public:
	Parser_Parametro * pPar;

	Sentencia_Parametro(Parser_Parametro * a) : pPar(a), Parser_Sentencia(SENT_VAR_INI) {}

	//Aseguramos el borrado de la memoria
	virtual ~Sentencia_Parametro() {
		delete pPar;
	}
};

// ############################################################
// ####################### FUNCIONES ########################## 
// ############################################################

class Parser_Funcion : public Parser_NODE {
public:
	Parser_Identificador * pID;
	std::vector<Funcion_ValorEntrada*> entradas;
	Parser_Declarativo * salida;
	Parser_Sentencia * body;

	std::string source;


	// Función sin valor devuelto, el valor devuelto puede ser automático o no tenerlo
	Parser_Funcion(Parser_Identificador * a, std::vector<Funcion_ValorEntrada*> b, Parser_Sentencia * c) : pID(a), entradas(b), body(c), salida(NULL){}
	//Función completa
	Parser_Funcion(Parser_Identificador * a, std::vector<Funcion_ValorEntrada*> b, Parser_Sentencia * c, Parser_Declarativo * d) : pID(a), entradas(b), body(c), salida(d) {}

	virtual NODE_type node_type() { return NODE_FUNCION; }

	//Aseguramos el borrado de la memoria
	virtual ~Parser_Funcion() 
	{
		delete pID;
		delete salida;
		delete body;

		for (std::vector<Funcion_ValorEntrada*>::iterator it = entradas.begin(); it != entradas.end(); ++it)
		{
			delete (*it);
		}
		entradas.clear();

	};
};



#endif