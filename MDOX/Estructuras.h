#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include <variant>
#include <string>
#include <vector>
#include <list>
#include <tuple>
#include <iostream>

#include "Tokenizer.h"
//#include "Funciones.h"



#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <memory>


enum tipo_permanente
{
	TP_ETIQUETA,
	TP_INCLUDE,
	TP_FUNCION,
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



//Valores que tendrá cada NODO, la línea a la que pertenece, el offset, y el nombre del fichero.
class OutData_Parametros
{
public:
	int linea;
	int offset;
	Fichero * fichero = NULL;

	OutData_Parametros() {}
	OutData_Parametros(int a, int b, Fichero * c) : linea(a), offset(b), fichero(c) {}

	void loadDatas(int a, int b, Fichero * c)
	{
		linea = a;
		offset = b;
		fichero = c;
	}
};

enum tipos_parametros
{
	PARAM_VOID, // Void es un valor vacio, pero que puede llegar a tomar cualquier valor.
				//Un valor vacio como tal no se puede operar, pero se puede transformar en cualquier
				// otro valor.

	//ENTEROS
	PARAM_INT,	 // -2.147.483.647 a 2.147.483.647   (4 bytes)
	PARAM_LINT,	 // -9.223.372.036.854.775.807 a 9.223.372.036.854.775.807 (8 bytes)

	PARAM_CHAR,

	PARAM_FLOAT,   // 7 digitos						 (4 bytes)
	PARAM_DOUBLE,  // 15 digitos						 (8 bytes)
	PARAM_BOOL,   //  true/false                       (1 byte)
	//PARAM_STRUCT, //Diseñado por el usuario.


	//PARAM_ENUM,
	PARAM_STRING,
	PARAM_VECTOR,

};

enum NODE_type {
	NODE_NULL,
	NODE_SENTENCIA,
	NODE_FUNCION,
};


class Parser_NODE { 
public: 

	Parser_NODE() {}

	public:
		OutData_Parametros parametros;

		void generarPosicion(Tokenizer * tokenizer) 
		{ 
			parametros = OutData_Parametros(tokenizer->token_actual->linea, tokenizer->token_actual->char_horizontal, tokenizer->fichero);
		}

	virtual ~Parser_NODE() { }; 
	virtual NODE_type node_type() { return NODE_NULL; };
};




// ###################################################
// ################## DECLARATIVOS ###################
// ###################################################

class Parser_Declarativo {
public:
	tipos_parametros value;

	Parser_Declarativo(tipos_parametros a) : value(a) {}

	//Aseguramos el borrado de la memoria
	virtual ~Parser_Declarativo() {
	};
};

// ############################################################
// ####################### OPERADORES ######################### 
// ############################################################

enum OPERADORES {

	OP_NONE,

	//Operadores aritméticos
	//Prior 6
	OP_SCOPE_LEFT,
	OP_SCOPE_RIGHT,
	OP_NEGADO,
	ELEM_NEG_FIRST, // No se usa como tal, son negativos
	OP_ITR_PLUS,
	OP_ITR_MIN,

	//Prior 5
	OP_ARIT_MULT,
	OP_ARIT_DIV,
	OP_ARIT_DIV_ENTERA,
	OP_ARIT_MOD,

	//Prior 4
	OP_ARIT_SUMA,
	OP_ARIT_RESTA,

	//Operadores relacionales
	//Prior 3
	OP_REL_EQUAL, //NO MOD
	OP_REL_NOT_EQUAL,
	OP_REL_MINOR,
	OP_REL_MAJOR,
	OP_REL_MINOR_OR_EQUAL,
	OP_REL_MAJOR_OR_EQUAL, //NO MOD

	//Operadores lógicos
	//Prior 2
	OP_LOG_ADD,
	OP_LOG_OR,

	//Operadores igualdad
	//Prior 1
	OP_GET_FIRST, // :  -> x:xs 
	OP_GET_LAST, // ::  -> xs::x
	OP_IG_EQUAL,
	OP_IG_EQUAL_SUM,
	OP_IG_EQUAL_MIN,
	OP_IG_EQUAL_DIV,
	OP_IG_EQUAL_MULT,
	OP_IG_EQUAL_MOD,

};


static bool is_single_operator(OPERADORES& p)
{
	switch (p)
	{
	case OP_NEGADO:
	case ELEM_NEG_FIRST:
	case OP_ITR_PLUS:
	case OP_ITR_MIN:
		return true;
	}

	return false;
}

static bool is_assignment_operator(OPERADORES& p)
{
	if (p >= OP_IG_EQUAL && p <= OP_IG_EQUAL_MOD)
		return true;
	return false;
}

static bool is_left(OPERADORES & p)
{
	switch (p)
	{
	case OP_NEGADO:
	case OP_ITR_PLUS:
	case OP_ITR_MIN:
	case OP_ARIT_SUMA:
	case OP_ARIT_RESTA:
		return true;
	}
	return false;
}

static bool transform_left(OPERADORES & p)
{
	if (p == OP_ARIT_RESTA)
		p = ELEM_NEG_FIRST;
	else if (p == OP_ARIT_SUMA)
		return false;
	return true;
}

#define PRIORIDAD_IGUALDAD  1
#define PRIORIDAD_LOGICA  2
#define PRIORIDAD_RELACIONAL  3
#define PRIORIDAD_SUMATORIA 4
#define PRIORIDAD_MULT 5
#define PRIORIDAD_SCOPES_EXP 6

static int prioridad(OPERADORES & a) {
	if (a == OP_ARIT_SUMA || a == OP_ARIT_RESTA)
		return PRIORIDAD_SUMATORIA;
	else if (a >= OP_ARIT_MULT && a <= OP_ARIT_MOD)
		return PRIORIDAD_MULT;
	else if (a == OP_LOG_ADD || a == OP_LOG_OR)
		return PRIORIDAD_LOGICA;
	else if (a >= OP_SCOPE_LEFT && a <= OP_ITR_MIN)
		return PRIORIDAD_SCOPES_EXP;
	else if (a >= OP_IG_EQUAL && a <= OP_IG_EQUAL_MOD)
		return PRIORIDAD_IGUALDAD;
	else return PRIORIDAD_RELACIONAL;
}

static bool isRelationalOperator(OPERADORES a) { return (a >= OP_REL_EQUAL && a <= OP_REL_MAJOR_OR_EQUAL); }

class Value;

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;
using val_variant = std::variant<int, bool, double, std::vector<Value>, std::monostate, long long, std::string>;

//template <class valueType>
class Value {
public:
	val_variant value;
	//	tipos_parametros tipo;

	~Value() { };

	Value operacion_Binaria(Value& v, const OPERADORES op);
	bool OperacionRelacional(const Value& v, const OPERADORES op);
	Value operacion_Unitaria(OPERADORES& op);
	bool operacion_Asignacion(Value& v, OPERADORES& op, bool fuerte);
	bool asignacion(Value& v, bool& fuerte);
	void inicializacion(Parser_Declarativo* tipo);

	void print();

	bool mayorQue_Condicional(const Value& v);
	bool menorQue_Condicional(const Value& v);
	bool mayorIgualQue_Condicional(const Value& v);
	bool menorIgualQue_Condicional(const Value& v);
	bool igualdad_Condicional(const Value& v);

	bool ValueToBool();
	bool Cast(Parser_Declarativo* pDec);
	bool Cast(const tipos_parametros);

	bool operator==(const Value& lhs)
	{
		return this->igualdad_Condicional(lhs);
	};

	template <typename T>
	static std::string to_string_p(const T a_value, const int n = 10);

	Value() : value(std::monostate()) {};
	//	Value(valueType v) : value(v);

		//Value(val_variant v) : value(v) {};

	Value(int a) : value(a) {};
	Value(double a) : value(a) {};
	Value(long long a) : value(a) {};
	Value(std::string& a) : value(a) {};
	Value(bool a) : value(a) {};
	Value(std::monostate) : value(std::monostate()) {};
	Value(std::vector<Value> a) : value(a) {};

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
	ValorType tipo;
	Parser_Valor(ValorType a) : tipo(a) {}

	virtual ~Parser_Valor() {}
};


// ################################################################
// ####################### IDENTIFICADOR ########################## 
// ################################################################

class Parser_Identificador : public Parser_Valor, public Parser_NODE {
public:
	bool var_global = false;

	int index;
	std::string nombre;
	bool fuerte = false;

	Parser_Declarativo * tipo = NULL;

	Parser_Identificador(std::string a) : nombre(a), Parser_Valor(VAL_ID) {}
	Parser_Identificador() : Parser_Valor(VAL_ID) {}
	virtual ~Parser_Identificador() {};
};


class Parser_Operacion;



// ###############################################################
// ######################### VALOR PLY ###########################
// ###############################################################

//Basicamente se trataría de funciones con un retorno de un valor dado.
class Valor_Funcion : public Parser_Valor, public Parser_NODE
 {
public:
	Parser_Identificador* ID;
	std::vector<Parser_Operacion*> entradas;

	Valor_Funcion(Parser_Identificador* a) : ID(a), Parser_Valor(VAL_FUNC) {}
	Valor_Funcion(Parser_Identificador * a, std::vector<Parser_Operacion*> b) : ID(a), entradas(b), Parser_Valor(VAL_FUNC)  {}

	virtual ~Valor_Funcion() 
	{ 
		delete ID; 
	};
};

// ################################################################
// ######################### OPERACIONES ########################## 
// ################################################################
class arbol_operacional;
class multi_value;

using tipoValor = std::variant<Value, Parser_Identificador*, Valor_Funcion*, arbol_operacional*, multi_value* >;

class multi_value
{
public:
	std::vector<tipoValor> arr;
//	bool is_vector = false;

	~multi_value()
	{
		for (std::vector<tipoValor>::iterator it = arr.begin(); it != arr.end(); ++it)
		{
			std::visit(overloaded{
			[](auto & a) { delete a; },
			[](Value&) {},
			}, *it);
		}
	}
};

using conmp = std::variant<Value, Parser_Identificador*, Valor_Funcion*, OPERADORES, multi_value* >;
using stack_conmp = std::deque<conmp>;

class arbol_operacional
{
public:
	OPERADORES operador;
	tipoValor _v1; // Primer valor 
	tipoValor _v2; // segundo valor  

	arbol_operacional(tipoValor a, tipoValor b, OPERADORES op) : _v1(a), _v2(b), operador(op) {}
	arbol_operacional(tipoValor a) : _v1(a), operador(OP_NONE) {}
	arbol_operacional(OPERADORES op) : operador(op) {}
	arbol_operacional() : operador(OP_NONE) {}

	virtual ~arbol_operacional()
	{
		std::visit(overloaded{
			[](auto& a) { delete a; },
			[](Value&) {},
			}, _v1);
		std::visit(overloaded{
			[](auto & a) { delete a; },
			[](Value&) {},
			}, _v2);

	}
};

class Parser_Operacion : public Parser_NODE {
public:
	arbol_operacional* val;

	Parser_Operacion() {}
	Parser_Operacion(arbol_operacional* a) : val(a) {}

	//Aseguramos el borrado de la memoria
	virtual ~Parser_Operacion()
	{
		delete val;
	}
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
	Parser_Operacion * pCond;
	Parser_Sentencia * pS;
	Parser_Sentencia * pElse;

	// Sentencia IF sin ELSE asignado.
	Sentencia_IF(Parser_Operacion * a, Parser_Sentencia* b) : pCond(a), pS(b), pElse(NULL), Parser_Sentencia(SENT_IF) {}
	// Sentencia IF con ELSE asignado
	Sentencia_IF(Parser_Operacion * a, Parser_Sentencia* b, Parser_Sentencia* c) : pCond(a), pS(b), pElse(c), Parser_Sentencia(SENT_IF) {}

	//Aseguramos el borrado de la memoria
	virtual ~Sentencia_IF() {
		delete pCond;
		delete pS;
		delete pElse;
	}
};

class Sentencia_WHILE : public Parser_Sentencia {
public:
	Parser_Operacion * pCond;
	Parser_Sentencia * pS;

	Sentencia_WHILE(Parser_Operacion * a, Parser_Sentencia* b) : pCond(a), pS(b), Parser_Sentencia(SENT_WHILE) {}
	
	//Aseguramos el borrado de la memoria
	virtual ~Sentencia_WHILE() {
		delete pCond;
		delete pS;
	}
};

class Sentencia_FOR : public Parser_Sentencia {
public:
	Parser_Operacion * pIguald;
	Parser_Operacion * pCond;
	Parser_Operacion * pOp;
	Parser_Sentencia * pS;

	Sentencia_FOR(Parser_Operacion* a, Parser_Operacion * b, Parser_Operacion * c, Parser_Sentencia * d) : pIguald(a), pCond(b), pOp(c), pS(d), Parser_Sentencia(SENT_FOR) {};

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

// ############################################################
// ####################### FUNCIONES ########################## 
// ############################################################

class Parser_Funcion : public Parser_NODE {
public:
	Parser_Identificador * pID;
	std::vector<Parser_Operacion*> entradas;
	Parser_Declarativo * salida;
	Parser_Sentencia * body;

	//Variables precargadas en cada funcion
	int preload_var = 0;

	// Función sin valor devuelto, el valor devuelto puede ser automático o no tenerlo
	Parser_Funcion(Parser_Identificador * a, std::vector<Parser_Operacion*> b, Parser_Sentencia * c) : pID(a), entradas(b), body(c), salida(NULL){}
	//Función completa
	Parser_Funcion(Parser_Identificador * a, std::vector<Parser_Operacion*> b, Parser_Sentencia * c, Parser_Declarativo * d) : pID(a), entradas(b), body(c), salida(d) {}

	virtual NODE_type node_type() { return NODE_FUNCION; }

	//Aseguramos el borrado de la memoria
	virtual ~Parser_Funcion() 
	{
		delete pID;
		delete salida;
		delete body;

		for (std::vector<Parser_Operacion*>::iterator it = entradas.begin(); it != entradas.end(); ++it)
		{
			delete (*it);
		}
		entradas.clear();

	};
};

#endif