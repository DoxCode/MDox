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
template<class T> inline T& operator|= (T & a, T b) { return (T&)((int&)a |= (int)b); }
template<class T> inline T& operator&= (T & a, T b) { return (T&)((int&)a &= (int)b); }
template<class T> inline T& operator^= (T & a, T b) { return (T&)((int&)a ^= (int)b); }

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
	Fichero* fichero = NULL;

	OutData_Parametros() {}
	OutData_Parametros(int a, int b, Fichero* c) : linea(a), offset(b), fichero(c) {}

	void loadDatas(int a, int b, Fichero* c)
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

	void generarPosicion(Tokenizer* tokenizer)
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
	//Prior 8
	OP_BRACKET_LEFT, //El operador offset es [op]
	OP_BRACKET_RIGHT,

	//Prior 7
	OP_SCOPE_LEFT,
	OP_SCOPE_RIGHT,
	OP_NEGADO,
	ELEM_NEG_FIRST, // No se usa como tal, son negativos
	OP_ITR_PLUS,
	OP_ITR_MIN,

	//Prior 6
	OP_ARIT_MULT,
	OP_ARIT_DIV,
	OP_ARIT_DIV_ENTERA,
	OP_ARIT_MOD,

	//Prior 5
	OP_ARIT_SUMA,
	OP_ARIT_RESTA,

	//Operadores relacionales
	//Prior 4
	OP_REL_EQUAL, //NO MOD
	OP_REL_NOT_EQUAL,
	OP_REL_MINOR,
	OP_REL_MAJOR,
	OP_REL_MINOR_OR_EQUAL,
	OP_REL_MAJOR_OR_EQUAL, //NO MOD

	//Operadores lógicos
	//Prior 3
	OP_LOG_ADD,
	OP_LOG_OR,

	//Operadores especiales
	//Prior 2
	OP_POP_ADD, // :  -> xs:x
	OP_CHECK_GET, // ::  -> xs::x

	//Operadores igualdad
	//Prior 1
	OP_IG_EQUAL,
	OP_IG_EQUAL_SUM,
	OP_IG_EQUAL_MIN,
	OP_IG_EQUAL_DIV,
	OP_IG_EQUAL_MULT,
	OP_IG_EQUAL_MOD,

};


static bool is_single_operator(OPERADORES & p)
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

static bool is_assignment_operator(OPERADORES & p)
{
	if (p >= OP_IG_EQUAL && p <= OP_IG_EQUAL_MOD)
		return true;
	return false;
}

static bool is_mult_value_operator(OPERADORES& p)
{
	if (p >= OP_POP_ADD && p <= OP_CHECK_GET)
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
#define PRIORIDAD_MULT_VALUE_OP  2
#define PRIORIDAD_LOGICA  3
#define PRIORIDAD_RELACIONAL  4
#define PRIORIDAD_SUMATORIA 5
#define PRIORIDAD_MULT 6
#define PRIORIDAD_SCOPES_EXP 7
#define PRIORIDAD_OP_EJ 8

static int prioridad(OPERADORES & a) {
	if (a == OP_ARIT_SUMA || a == OP_ARIT_RESTA)
		return PRIORIDAD_SUMATORIA;
	else if (a == OP_BRACKET_LEFT || a == OP_BRACKET_RIGHT)
		return PRIORIDAD_OP_EJ;
	else if (a == OP_POP_ADD || a == OP_CHECK_GET)
		return PRIORIDAD_MULT_VALUE_OP;
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

class mdox_vector
{
public:
	std::vector<Value> vector;
	mdox_vector() {};

	//mdox_vector(Value& v) { vector = { v }; };
	//mdox_vector(Value&& v) { vector = { std::move(v) }; };

	mdox_vector(std::vector<Value>& b) : vector(b) {};
	mdox_vector(std::vector<Value>&& b) : vector(std::move(b)) {};
};


using val_variant = std::variant< std::monostate, int, bool, double, std::shared_ptr<mdox_vector>, long long, std::string>;
class Parser_Identificador;

//template <class valueType>
class Value {
public:
	val_variant value;
	//	tipos_parametros tipo;

	~Value() { };
	static Value Value::Suma(Value& v1, Value& v2);
	static Value Value::Resta(Value& v1, Value& v2);
	static Value Value::Multiplicacion(Value& v1, Value& v2);
	static Value Value::Div(Value& v1, Value& v2);
	static Value Value::DivEntera(Value& v1, Value& v2);
	static Value Value::Mod(Value& v1, Value& v2);
	static Value Value::Offset(Value& v1, Value& v2); //[x]

	bool OperadoresEspeciales_Check(Value& v, int index, Parser_Identificador * f1 = NULL, Parser_Identificador * f2 = NULL);
	bool OperadoresEspeciales_Pop(Value& v, bool& left, Parser_Identificador * f1 = NULL, Parser_Identificador * f2 = NULL);
	Value operacion_Binaria(Value& v, const OPERADORES op);
	bool OperacionRelacional(Value& v, const OPERADORES op);
	Value operacion_Unitaria(OPERADORES& op);
	bool operacion_Asignacion(Value& v, OPERADORES& op, bool fuerte);
	bool asignacion(Value& v, bool fuerte);
	void inicializacion(Parser_Declarativo* tipo);

	void print();

	bool mayorQue_Condicional(Value& v);
	bool menorQue_Condicional(Value& v);
	bool mayorIgualQue_Condicional(Value& v);
	bool menorIgualQue_Condicional(Value& v);
	bool igualdad_Condicional(Value& v);

	bool ValueToBool();
	bool Cast(Parser_Declarativo* pDec);
	bool Cast(const tipos_parametros);

	bool operator==(Value& lhs)
	{
		return this->igualdad_Condicional(lhs);
	};

	template <typename T>
	static std::string to_string_p(const T& a_value, const int n = 10);

	Value() : value(std::monostate()) {};
	//	Value(valueType v) : value(v);

		//Value(val_variant v) : value(v) {};

	Value(int& a) : value(a) {  };
	Value(double& a) : value(a) {  };
	Value(long long& a) : value(a) {  };
	Value(std::string& a) : value(a) {  };
	Value(bool& a) : value(a) {  };
	Value(std::monostate&) : value(std::monostate()) {  };
	Value(std::shared_ptr<mdox_vector>& a) : value(a) {  };

	Value(int&& a) : value(std::move(a)) {  };
	Value(double&& a) : value(std::move(a)) {  };
	Value(long long&& a) : value(std::move(a)) {  };
	Value(std::string&& a) : value(std::move(a)) {  };
	Value(bool&& a) : value(std::move(a)) {  };
	Value(std::shared_ptr<mdox_vector>&& a) : value(std::move(a)) {  };


	/*
	Value operator=(Value& lhs)
	{
		std::cout << "Copia =\n";
		value = lhs.value;
		return *this;

	};

	Value operator=(Value&& lhs)
	{
		std::cout << "Mover =\n";
		value = std::move(lhs.value);
		return *this;
	};


	Value(const Value& v)
	{
		std::cout << "Copia Const\n";
		value = v.value;
	}

	Value(Value&& v)
	{
		std::cout << "Mover Const\n";
		value = std::move(v.value);
	}
		*/

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
	bool inicializando = false;

	Parser_Declarativo* tipo = NULL;

	Parser_Identificador(std::string a) : nombre(a), Parser_Valor(VAL_ID) {}
	Parser_Identificador() : Parser_Valor(VAL_ID) {}
	virtual ~Parser_Identificador() {};
};


class arbol_operacional;



// ###############################################################
// ######################### VALOR PLY ###########################
// ###############################################################

//Basicamente se trataría de funciones con un retorno de un valor dado.
class Valor_Funcion : public Parser_Valor, public Parser_NODE
{
public:
	Parser_Identificador* ID;
	std::vector<arbol_operacional*> entradas;

	Valor_Funcion(Parser_Identificador* a) : ID(a), Parser_Valor(VAL_FUNC) {}
	Valor_Funcion(Parser_Identificador* a, std::vector<arbol_operacional*> b) : ID(a), entradas(b), Parser_Valor(VAL_FUNC) {}

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
//using ValueCopyOrRef = std::variant<Value, Value*, std::monostate>;

class ValueCopyOrRef
{
	Value _v;
public:
	Value* ref = NULL;
	multi_value* mv = NULL;
	
	ValueCopyOrRef(Value&& a) : _v(std::move(a)), ref(&_v) {};
	ValueCopyOrRef(Value& a) : ref(&a) {};
	ValueCopyOrRef(Value * a) : ref(a) {};
	ValueCopyOrRef(multi_value* a) : mv(a) {};
	ValueCopyOrRef(std::monostate&) : ref(NULL) {};
};

class OperacionesEnVector
{
public:
	tipoValor v1; // Será el vector si se tratan de dos operaciones
	tipoValor v2; // Será el vector en caso de 3 operaciones
	tipoValor v3;

	bool onlyValue = false; // Si es true, no existen identificadores las operaciones
	bool dobleOperador = true;

	Parser_Identificador* id_v =  NULL;
	/**
	 Funcion que comprueba si el dato introducido se trata de un Valor que puede tratarse de un vector.
	 Devolverá valor a id_v si se trata de un identificador débil, pues puede tratarse de un vector.
	 Devolverá NULL si es un multivalor o un identificador fuerte.
	**/
	void isOnlyValue()
	{
		onlyValue = !std::visit(overloaded{
			[&](Parser_Identificador * a) { if (a->fuerte) return false; else { id_v = a;  return true; } },
			[](auto&) { return false; },
			}, v1);

		if (!onlyValue && !dobleOperador)
			return;

		onlyValue = !std::visit(overloaded{
		[&](Value&) {return false; },
		[&](Parser_Identificador * a) { if (a->fuerte) return false; else { id_v = a;  return true; } },
		[&](multi_value * a) { return false; },
		[&](Valor_Funcion * a) { return false; },
		[&](arbol_operacional * a) { return false; },
			}, v2);

		if (!onlyValue || !dobleOperador)
			return;

		onlyValue = !std::visit(overloaded{
		[&](Value&) {return false; },
		[&](Parser_Identificador * a) { return !a->fuerte; },
		[&](multi_value * a) { return false; },
		[&](Valor_Funcion * a) { return false; },
		[&](arbol_operacional * a) { return false; },
		}, v3);
	}

	//Si existen operador1 y operador2, el vector DEBE estar en v2
	OPERADORES operador1; // OP v1-v2
	OPERADORES operador2; // OP v2-v3


	OperacionesEnVector(tipoValor& l, tipoValor& v, OPERADORES& op) : v1(l), v2(v), operador1(op), operador2(OP_NONE), dobleOperador(false) { isOnlyValue(); };
	OperacionesEnVector(tipoValor&& l, tipoValor&& v, OPERADORES& op) : v1(std::move(l)), v2(std::move(v)), operador1(op), operador2(OP_NONE), dobleOperador(false) { isOnlyValue(); };
	OperacionesEnVector(tipoValor& l, tipoValor&& v, OPERADORES& op) : v1(l), v2(std::move(v)), operador1(op), operador2(OP_NONE), dobleOperador(false) { isOnlyValue(); };
	OperacionesEnVector(tipoValor&& l, tipoValor& v, OPERADORES& op) : v1(std::move(l)), v2(v), operador1(op), operador2(OP_NONE), dobleOperador(false) { isOnlyValue(); };
	
	OperacionesEnVector(tipoValor& l, tipoValor& v, OPERADORES& op, tipoValor& v2, OPERADORES& op2) : v1(l), v2(v), operador1(op), operador2(op2), v3(v2) { isOnlyValue();  };

	OperacionesEnVector(tipoValor&& l, tipoValor& v, OPERADORES& op, tipoValor& v2, OPERADORES& op2) : v1(std::move(l)), v2(v), operador1(op), operador2(op2), v3(v2) { isOnlyValue(); };
	OperacionesEnVector(tipoValor&& l, tipoValor&& v, OPERADORES& op, tipoValor& v2, OPERADORES& op2) : v1(std::move(l)), v2(std::move(v)), operador1(op), operador2(op2), v3(v2) { isOnlyValue(); };
	OperacionesEnVector(tipoValor&& l, tipoValor& v, OPERADORES& op, tipoValor&& v2, OPERADORES& op2) : v1(std::move(l)), v2(v), operador1(op), operador2(op2), v3(std::move(v2)) { isOnlyValue(); };

	OperacionesEnVector(tipoValor&& l, tipoValor&& v, OPERADORES& op, tipoValor&& v2, OPERADORES& op2) : v1(std::move(l)), v2(std::move(v)), operador1(op), operador2(op2), v3(std::move(v2)) { isOnlyValue(); };
	OperacionesEnVector(tipoValor& l, tipoValor&& v, OPERADORES& op, tipoValor&& v2, OPERADORES& op2) : v1(l), v2(std::move(v)), operador1(op), operador2(op2), v3(std::move(v2)) { isOnlyValue(); };
	OperacionesEnVector(tipoValor& l, tipoValor&& v, OPERADORES& op, tipoValor& v2, OPERADORES& op2) : v1(l), v2(std::move(v)), operador1(op), operador2(op2), v3(v2) { isOnlyValue(); };
	OperacionesEnVector(tipoValor& l, tipoValor& v, OPERADORES& op, tipoValor&& v2, OPERADORES& op2) : v1(l), v2(v), operador1(op), operador2(op2), v3(std::move(v2)) { isOnlyValue(); };
};

class multi_value
{
public:
	std::vector<tipoValor> arr;
	bool is_vector = false; // Si no es un vector, es multivalor/operacion, es decir (a,b,c)-> EJ: a = 1, b = 2, c = 3;
	bool contenedor = false; // Si es true, implica operaciones de editado de vectores ':' o '::' 

	OperacionesEnVector* operacionesVector = NULL;

	~multi_value()
	{
		deletePtr(operacionesVector);
		for (std::vector<tipoValor>::iterator it = arr.begin(); it != arr.end(); ++it)
		{
			std::visit(overloaded{
			[](auto & a) { if(a) deletePtr(a); },
			[](Value&) {},
				}, *it);
		}
	}
};

using conmp = std::variant< std::monostate, Value, Parser_Identificador*, Valor_Funcion*, OPERADORES, multi_value*>;
using stack_conmp = std::deque<conmp>;

class arbol_operacional : public Parser_NODE
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
			[](auto & a) { delete a; },
			[](Value&) {},
			}, _v1);
		std::visit(overloaded{
			[](auto & a) { delete a; },
			[](Value&) {},
			}, _v2);

	}
};
/*
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
};*/

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
	arbol_operacional* pCond;
	Parser_Sentencia* pS;
	Parser_Sentencia* pElse;

	// Sentencia IF sin ELSE asignado.
	Sentencia_IF(arbol_operacional* a, Parser_Sentencia* b) : pCond(a), pS(b), pElse(NULL), Parser_Sentencia(SENT_IF) {}
	// Sentencia IF con ELSE asignado
	Sentencia_IF(arbol_operacional* a, Parser_Sentencia* b, Parser_Sentencia* c) : pCond(a), pS(b), pElse(c), Parser_Sentencia(SENT_IF) {}

	//Aseguramos el borrado de la memoria
	virtual ~Sentencia_IF() {
		delete pCond;
		delete pS;
		delete pElse;
	}
};

class Sentencia_WHILE : public Parser_Sentencia {
public:
	arbol_operacional* pCond;
	Parser_Sentencia* pS;

	Sentencia_WHILE(arbol_operacional* a, Parser_Sentencia* b) : pCond(a), pS(b), Parser_Sentencia(SENT_WHILE) {}

	//Aseguramos el borrado de la memoria
	virtual ~Sentencia_WHILE() {
		delete pCond;
		delete pS;
	}
};

class Sentencia_FOR : public Parser_Sentencia {
public:
	arbol_operacional* pIguald;
	arbol_operacional* pCond;
	arbol_operacional* pOp;
	Parser_Sentencia* pS;

	Sentencia_FOR(arbol_operacional* a, arbol_operacional* b, arbol_operacional* c, Parser_Sentencia* d) : pIguald(a), pCond(b), pOp(c), pS(d), Parser_Sentencia(SENT_FOR) {};

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
	arbol_operacional* pOp;

	Sentencia_Return(arbol_operacional* a) : pOp(a), Parser_Sentencia(SENT_RETURN) {}

	//Aseguramos el borrado de la memoria
	virtual ~Sentencia_Return() {
		delete pOp;
	}
};

class Sentencia_Print : public Parser_Sentencia {
public:
	arbol_operacional* pOp;

	Sentencia_Print(arbol_operacional* a) : pOp(a), Parser_Sentencia(SENT_PRINT) {}

	//Aseguramos el borrado de la memoria
	virtual ~Sentencia_Print() {
		delete pOp;
	}
};

class Sentencia_Operacional : public Parser_Sentencia {
public:
	arbol_operacional* pOp;

	Sentencia_Operacional(arbol_operacional* a) : pOp(a), Parser_Sentencia(SENT_OP) {}

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
	Parser_Identificador* pID;
	std::vector<arbol_operacional*> entradas;
	Parser_Declarativo* salida;
	Parser_Sentencia* body;

	//Variables precargadas en cada funcion
	int preload_var = 0;

	// Función sin valor devuelto, el valor devuelto puede ser automático o no tenerlo
	Parser_Funcion(Parser_Identificador* a, std::vector<arbol_operacional*> b, Parser_Sentencia* c) : pID(a), entradas(b), body(c), salida(NULL) {}
	//Función completa
	Parser_Funcion(Parser_Identificador* a, std::vector<arbol_operacional*> b, Parser_Sentencia* c, Parser_Declarativo* d) : pID(a), entradas(b), body(c), salida(d) {}

	virtual NODE_type node_type() { return NODE_FUNCION; }

	//Aseguramos el borrado de la memoria
	virtual ~Parser_Funcion()
	{
		delete pID;
		delete salida;
		delete body;

		for (std::vector<arbol_operacional*>::iterator it = entradas.begin(); it != entradas.end(); ++it)
		{
			delete (*it);
		}
		entradas.clear();

	};
};

#endif