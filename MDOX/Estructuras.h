#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include <variant>
#include <string>
#include <vector>
#include <list>
#include <tuple>
#include <iostream>
#include <unordered_map>
#include <algorithm>  
#include <map>
#include <bitset>
#include <fstream>
#include <assert.h>  
#include <filesystem>

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
	PARAM_NULL, //Se suele usar como null/indeterminado, solo usado internamente.

	PARAM_VOID, // Void es un valor vacio, pero que puede llegar a tomar cualquier valor.
				//Un valor vacio como tal no se puede operar, pero se puede transformar en cualquier
				// otro valor.

	//ENTEROS
	PARAM_INT,	 // -2.147.483.647 a 2.147.483.647   (4 bytes)
	PARAM_LINT,	 // -9.223.372.036.854.775.807 a 9.223.372.036.854.775.807 (8 bytes)

	//PARAM_CHAR,

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
	bool estricto = false;

	Parser_Declarativo(tipos_parametros a) : value(a) {}
	Parser_Declarativo(tipos_parametros a, bool b) : value(a), estricto(b) {}

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
	OP_CLASS_ACCESS, //Operación de acceso a clase '.'
	OP_NEGADO,
	ELEM_NEG_FIRST, // No se usa como tal, son negativos
	OP_ITR_PLUS,
	OP_ITR_MIN,

	//Prior 6
	OP_ARIT_MULT,
	OP_COPY,
	OP_IN_COPY,
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
	OP_LOG_AND,
	OP_LOG_OR,

	//Operadores especiales
	//Prior 2
	OP_POP_ADD, // :  -> xs:x
	OP_CHECK_GET, // ::  -> xs::x

	//Operadores asignacion
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
	case OP_COPY:
	case OP_IN_COPY:
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
	case OP_COPY:
	case OP_IN_COPY:
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
	else if (a == OP_LOG_AND || a == OP_LOG_OR)
		return PRIORIDAD_LOGICA;
	else if (a >= OP_SCOPE_LEFT && a <= OP_ITR_MIN)
		return PRIORIDAD_SCOPES_EXP;
	else if (a >= OP_IG_EQUAL && a <= OP_IG_EQUAL_MOD)
		return PRIORIDAD_IGUALDAD;
	else return PRIORIDAD_RELACIONAL;
}

static bool isRelationalOperator(OPERADORES a) { return (a >= OP_REL_EQUAL && a <= OP_REL_MAJOR_OR_EQUAL); }


class mdox_vector;

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;





class mdox_object;
class Variable_Runtime;
class Parser_Identificador;
class Value;

// Una llamada del estilo -> "a.x" lo transforma en un solo objeto.
//Por lo que para: a.b.x -> [ab].x -> [abx]
/*class wrapper_object_call
{
public:
	std::shared_ptr<mdox_object> objeto;
	Parser_Identificador* var;
	Value* getValue(bool, tipos_parametros* = nullptr);
	wrapper_object_call(std::shared_ptr<mdox_object>& a, Parser_Identificador* x) : objeto(a), var(x) {}
};*/

using val_variant = std::variant< std::monostate, int, bool, double, std::shared_ptr<mdox_vector>, std::shared_ptr<mdox_object>, std::string, long long, Variable_Runtime*>;

class Call_Value;

//template <class valueType>
class Value {
public:
	val_variant value;

	~Value() { };
	static Value Suma(Value& v1, Value& v2);
	static Value Resta(Value& v1, Value& v2);
	static Value Multiplicacion(Value& v1, Value& v2);
	static Value Div(Value& v1, Value& v2);
	static Value DivEntera(Value& v1, Value& v2);
	static Value Mod(Value& v1, Value& v2);
	static Value Offset(Value& v1, Value& v2); //[x]
	

	Value copyIn();
	Value ClassAccess(Parser_Identificador* v2, Call_Value * call = nullptr, bool isThis = false, Variable_Runtime* variables = nullptr, Variable_Runtime* var_class = nullptr);
	bool OperadoresEspeciales_Check(Value* v, int index);
	short int OperadoresEspeciales_Pop(Value* v, bool& left);
	Value operacion_Binaria(Value& v, const OPERADORES op);
	bool OperacionRelacional(Value& v, const OPERADORES op);
	Value operacion_Unitaria(OPERADORES& op);
	bool operacion_Asignacion(Value& v, OPERADORES& op, bool fuerte, bool strict);
	bool asignacion(Value& v) { return asignacion(v, false, false); }; //Usado normalmente si sabemos que será Wrapper.
	bool asignacion(Value& v, bool fuerte, bool strict);
	void inicializacion(Parser_Declarativo* tipo);
	void inicializacion(tipos_parametros tipo);

	void print();

	bool mayorQue_Condicional(Value& v);
	bool menorQue_Condicional(Value& v);
	bool mayorIgualQue_Condicional(Value& v);
	bool menorIgualQue_Condicional(Value& v);
	bool igualdad_Condicional(Value& v);
	bool igualdad_CondicionalFuncion(Value& v);

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

	//Value(valueType v) : value(v);
	//Value(val_variant v) : value(v) {};

	Value(int& a) : value(a) {  };
	Value(double& a) : value(a) {  };
	Value(long long& a) : value(a) {  };
	Value(std::string& a) : value(a) {  };
	Value(bool& a) : value(a) {  };
	Value(std::monostate&) : value(std::monostate()) {  };
	Value(std::shared_ptr<mdox_vector>& a) : value(a) {  };
	Value(std::shared_ptr<mdox_object>& a) : value(a) { };
	Value(Variable_Runtime* a) : value(a) { };

	Value(int&& a) : value(std::move(a)) {  };
	Value(double&& a) : value(std::move(a)) {  };
	Value(long long&& a) : value(std::move(a)) {  };
	Value(std::string&& a) : value(std::move(a)) {  };
	Value(bool&& a) : value(std::move(a)) {  };
	Value(std::shared_ptr<mdox_vector>&& a) : value(std::move(a)) {  };

	Value(val_variant& a) : value(a) {  };

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
	*/
};


class mdox_vector
{
public:
	std::deque<Value> vector;
	mdox_vector() {};

	//mdox_vector(Value& v) { vector = { v }; };
	//mdox_vector(Value&& v) { vector = { std::move(v) }; };

	mdox_vector(std::deque<Value>& b) : vector(b) {};
	mdox_vector(std::deque<Value>&& b) : vector(std::move(b)) {};

	// Llamado al usar el operador @@
	std::shared_ptr<mdox_vector> createCopyIn();
};

// ################################################################
// ####################### IDENTIFICADOR ########################## 
// ################################################################
class Parser_Class;

class Parser_Identificador : public Parser_NODE {
public:
	bool var_global = false;
	bool var_class = false;
	
	//STATIC VAR?
	bool is_Static = false;
	Parser_Class* static_link = nullptr; //Enlace a la clase si es estatica la operación

	int index = -1;
	std::string nombre;
	bool fuerte = false; // Redundante, se obtiene se obtiene con el tipo (VOID = false, ELSE = true), pero bool es más rápido por runtime.
	bool inicializando = false;

	bool insideVectorOperator = false;


	Parser_Declarativo* tipo = NULL;

	Parser_Identificador(std::string a) : nombre(a) {}
	Parser_Identificador() {}
	virtual ~Parser_Identificador() {};
};


class arbol_operacional;



// ###############################################################
// ######################### VALOR PLY ###########################
// ###############################################################

class IndexCall_Function
{
public:
	bool isCore = false;
	int funcionesCoreItrData = 0;			//Dirección de la función Core
	std::vector<int> funcionesItrData;		//Dirección de la función
};

class IndexCall_Class
{
public:
	int class_index = -2; // -1: Constructor por defecto, -2: ERROR será enviado - llamada no valida
	int constructor_index;
	//int _especial_var; //usado para especificar la variable estática a la cual accederá.
};

//Basicamente se trataría de funciones con un retorno de un valor dado.
// También nos servirá como llamada al constructor de una clase
// DEFAULT: ClassName()
// CONSTRUCTORES: ClassName(x,y,z)
// Básicamente las llamadas son iguales a las de una función.
class Call_Value : public Parser_NODE
{
public:
	Parser_Identificador* ID;
	std::vector<arbol_operacional*> entradas;


	bool skip = false;

	//Si valor enlace = NULL implica que en ningún momento podrá tratarse de un valor estático.
	//En caso contrario la posibilidad existe y tiene que ser tratada en el preloadCalls.
	//Este valos se modifica en el preload de operaciones.
	bool is_Static = false;
	//arbol_operacional* valor_enlace = NULL; // Retocar para cambiar el acceso a estaticos

	bool function_parent_is_static = false; // Indica si la función en la que se ejecuta la llamada es estática o no.

	//En el caso de que la llamada sea a una función
	IndexCall_Function* inx_funcion = NULL;

	//En el caso de que la llamda sea a una clase
	IndexCall_Class* inx_class = NULL;

	//Indica si la clase o función pertenece a otra clase, y en caso de hacerlo, a cual.

	Parser_Class* inside_class;
	bool isInsideClass = false;


	bool is_class;

	void AddFuncion_Core(int inx);
	void AddFuncion(int inx);
	void AddClass(int inx, int constructor);
	void setFuncion();
	void setClass();

	Call_Value(Parser_Identificador* a) : ID(a) {}
	Call_Value(Parser_Identificador* a, std::vector<arbol_operacional*> b) : ID(a), entradas(b) {}

	virtual ~Call_Value()
	{


		delete ID;

		if (inx_class)
			delete inx_class;

		if (inx_funcion)
			delete inx_funcion;
	};
};

// ################################################################
// ######################### OPERACIONES ########################## 
// ################################################################
class arbol_operacional;
class multi_value;

using tipoValor = std::variant<Value, Parser_Identificador*, Call_Value*, arbol_operacional*, multi_value* >;
//using ValueCopyOrRef = std::variant<Value, Value*, std::monostate>;

using ValueOrMulti = std::variant<Value, multi_value*>;

/*
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
};*/

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
		[&](Call_Value * a) { return false; },
		[&](arbol_operacional * a) { return false; },
			}, v2);

		if (!onlyValue || !dobleOperador)
			return;

		onlyValue = !std::visit(overloaded{
		[&](Value&) {return false; },
		[&](Parser_Identificador * a) { return !a->fuerte; },
		[&](multi_value * a) { return false; },
		[&](Call_Value * a) { return false; },
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



using conmp = std::variant< std::monostate, Value, Parser_Identificador*, Call_Value*, OPERADORES, multi_value*>;
using stack_conmp = std::deque<conmp>;

class arbol_operacional : public Parser_NODE
{
public:
	OPERADORES operador;
	tipoValor _v1; // Primer valor 
	tipoValor _v2; // segundo valor  

	bool is_Public = true;
	bool is_Static = false;

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

class multi_value
{
public:
	std::vector<arbol_operacional*> arr;

	bool is_vector = false; // Si no es un vector, es multivalor/operacion, es decir (a,b,c)-> EJ: a = 1, b = 2, c = 3;
	bool contenedor = false; // Si es true, implica operaciones de editado de vectores ':' o '::' 

	OperacionesEnVector* operacionesVector = NULL;

	~multi_value()
	{
		deletePtr(operacionesVector);
		for (std::vector<arbol_operacional*>::iterator it = arr.begin(); it != arr.end(); ++it)
		{
			deletePtr(*it);
		}
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
	SENT_EMPTY,
	SENT_REC,
	SENT_IF,
	SENT_WHILE,
	SENT_FOR,
	SENT_RETURN,
	SENT_ACCION,
	SENT_PRINT,
	SENT_INPUT,
	SENT_OP,
	SENT_INCLUDE,
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

class Parser;
class Sentencia_Include : public Parser_Sentencia
{
public:
	Parser* parser;
	Sentencia_Include(Parser* p) : parser(p),Parser_Sentencia(SENT_INCLUDE) {}
	virtual ~Sentencia_Include()
	{
		delete parser;
	}
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

class Sentencia_Empty : public Parser_Sentencia {
public:
	Sentencia_Empty() : Parser_Sentencia(SENT_EMPTY) {}
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

class Sentencia_Input : public Parser_Sentencia {
public:
	arbol_operacional* pOp;

	Sentencia_Input(arbol_operacional* a) : pOp(a), Parser_Sentencia(SENT_INPUT) {}

	//Aseguramos el borrado de la memoria
	virtual ~Sentencia_Input() {
		delete pOp;
	}
};

enum TipoAccion
{
	BREAK,		//ROMPE bucles
	CONTINUE,	//CONTINUA bucles
	IGNORE,		//IGNORA la función actual y pasa a buscar la siguiente.
};

class Sentencia_Accion : public Parser_Sentencia {
public:
	TipoAccion accion;
	Sentencia_Accion(TipoAccion& a) : accion(a), Parser_Sentencia(SENT_ACCION) {}
	Sentencia_Accion(TipoAccion&& a) : accion(std::move(a)), Parser_Sentencia(SENT_ACCION) {}
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

	bool is_Public = false;
	bool is_Static = false;

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

// ############################################################
// ######################### CLASES ########################### 
// ############################################################

enum etiquetas_class
{
	LABEL_PUBLIC,
	LABEL_PRIVATE,
	LABEL_STATIC,
	LABEL_NONE
};

class Parser_ClassConstructor
{
public:
	std::vector<int> entradas;
	Parser_Sentencia* op = NULL;
	int preLoadSize = 0;

	Parser_ClassConstructor()  {};
	Parser_ClassConstructor(Parser_Sentencia* b) : op(b) {};
	Parser_ClassConstructor(std::vector<int>& a, Parser_Sentencia* b, int c) : entradas(a), op(b), preLoadSize(c){};
};

class Operator_Class
{
public:
	//Consideraremos
	bool isBinaryOperator = true; //si es true se establece la variable en '0'
	Parser_Sentencia* body = NULL;

	int num_variables;

	Operator_Class(Parser_Sentencia* b, bool c, int d) : body(b), isBinaryOperator(c), num_variables(d){};

	~Operator_Class() {deletePtr(body); }
};

class Operators_List
{
public:
	//Operadores disponibles
	//No todos los operadores serán permitidos.
	//Por ejemplo, los '()' no se considerarán operadores.
	Operator_Class* OPERATOR_suma = NULL; // operator + (r) {}
	Operator_Class* OPERATOR_resta = NULL; // operator - (r) {}
	Operator_Class* OPERATOR_divEntera = NULL; // operator div (r) {}
	Operator_Class* OPERATOR_div = NULL; // operator / (r) {}
	Operator_Class* OPERATOR_multiplicacion = NULL; // operator * (r) {}
	Operator_Class* OPERATOR_mod = NULL; // operator % (r) {}

	Operator_Class* OPERATOR_asignacion = NULL; // operator = (r) {}
	Operator_Class* OPERATOR_asignacion_sum = NULL; // operator += (r) {}
	Operator_Class* OPERATOR_asignacion_res = NULL; // operator -= (r) {}
	Operator_Class* OPERATOR_asignacion_div = NULL; // operator /= (r) {}
	Operator_Class* OPERATOR_asignacion_mult = NULL; // operator *= (r) {}
	Operator_Class* OPERATOR_asignacion_mod = NULL; // operator %= (r) {}

	Operator_Class* OPERATOR_rel_igualdad = NULL; // operator == (r) {}
	Operator_Class* OPERATOR_rel_no_igual = NULL; // operator != (r) {}
	Operator_Class* OPERATOR_rel_menor = NULL; // operator < (r) {}
	Operator_Class* OPERATOR_rel_mayor = NULL; // operator > (r) {}
	Operator_Class* OPERATOR_rel_menor_igual = NULL; // operator <= (r) {}
	Operator_Class* OPERATOR_rel_mayor_igual = NULL; // operator >= (r) {}

	Operator_Class* OPERATOR_log_and = NULL; // operator && (r) {}
	Operator_Class* OPERATOR_log_or = NULL; // operator || (r) {}

	Operator_Class* OPERATOR_brack = NULL; // operator [] (r) {}
	Operator_Class* OPERATOR_pop_add = NULL; // operator : (r) {}
	Operator_Class* OPERATOR_check_get = NULL; // operator :: (r) {}

	//Operadores unitarios
	Operator_Class* OPERATOR_plus = NULL; // operator ++ () {}
	Operator_Class* OPERATOR_min = NULL; // operator -- () {}
	Operator_Class* OPERATOR_negado = NULL; // operator ! () {}
	Operator_Class* OPERATOR_negado_first = NULL; // operator - () {}


	~Operators_List()
	{
		deletePtr(OPERATOR_suma);
		deletePtr(OPERATOR_resta); // operator - (r) {}
		deletePtr(OPERATOR_divEntera); // operator div (r) {}
		deletePtr(OPERATOR_div); // operator / (r) {}
		deletePtr(OPERATOR_multiplicacion); // operator * (r) {}
		deletePtr(OPERATOR_mod); // operator % (r) {}
		deletePtr(OPERATOR_asignacion); // operator = (r) {}
		deletePtr(OPERATOR_asignacion_sum); // operator += (r) {}
		deletePtr(OPERATOR_asignacion_res); // operator -= (r) {}
		deletePtr(OPERATOR_asignacion_div); // operator /= (r) {}
		deletePtr(OPERATOR_asignacion_mult); // operator *= (r) {}
		deletePtr(OPERATOR_asignacion_mod); // operator %= (r) {}
		deletePtr(OPERATOR_rel_igualdad); // operator == (r) {}
		deletePtr(OPERATOR_rel_no_igual); // operator != (r) {}
		deletePtr(OPERATOR_rel_menor); // operator < (r) {}
		deletePtr(OPERATOR_rel_mayor); // operator > (r) {}
		deletePtr(OPERATOR_rel_menor_igual); // operator <= (r) {}
		deletePtr(OPERATOR_rel_mayor_igual); // operator >= (r) {}
		deletePtr(OPERATOR_log_and); // operator && (r) {}
		deletePtr(OPERATOR_log_or); // operator || (r) {}
		deletePtr(OPERATOR_brack); // operator [] (r) {}
		deletePtr(OPERATOR_pop_add); // operator : (r) {}
		deletePtr(OPERATOR_check_get); // operator :: (r) {}
		deletePtr(OPERATOR_plus); // operator ++ () {}
		deletePtr(OPERATOR_min); // operator -- () {}
		deletePtr(OPERATOR_negado); // operator ! () {}
		deletePtr(OPERATOR_negado_first); // operator - () {}
	}
};



//Clase creada al llamarlo
class Parser_Class_Created
{
public:
	Parser_Identificador* pID;

	//Variables estáticas de la clase
	std::vector<arbol_operacional*> variables_static;


	//Mapeado de las funciones.
	// Nos servirá para crear un pair entre el nombre de la función y el índice (funciones[indice]) con coste O(1) de
	// todas las funciones ya agregadas en la clase, este valor se creará al finalizar la clase durante el parseado, llamando a la función preloadFunctions
	std::unordered_map<std::string, std::vector<int>> function_map; 
	std::vector<Parser_Funcion*> funciones; //Vector donde guardaremos la funcione de la clase


	std::vector<Parser_ClassConstructor*> constructores;
	std::vector<arbol_operacional*> variables_operar;

	Operators_List* normal_operators = NULL; //Operadores del estilo this+x;  El objeto se encuentra en la izquierda de la operación.
	Operators_List* right_operators = NULL;  //Operadores del estilo x+this;  El objeto se encuentra en la derecha de la operación.


	//Función que deberá ser llamada al finalizar de construir la clase de forma obligatoria para crear el pair entre indices y funciones, que nos servirá como tabla
	// hash para mayor rapidez de ejecución en runtime
	void preloadFunctions()
	{
		if (this->funciones.size() == 0)
			return;

		int itr_inx = 0;
		for (std::vector<Parser_Funcion*>::iterator it = this->funciones.begin(); it != this->funciones.end(); ++it)
		{
			auto got = function_map.find((*it)->pID->nombre);

			if (got != function_map.end())
			{
				got->second.emplace_back(itr_inx);
			}
			else
			{
				auto a = function_map.emplace((*it)->pID->nombre, 0);
				if (a.second)
				{
					a.first->second.emplace_back(itr_inx);
				}
			}

			itr_inx++;
		}
	}

	std::vector<int>* findFuncion(std::string& id)
	{
		//Buscamos los nombres de los índices de la función con el nombre dado entre el mapeado creado durante el parseado.
		std::unordered_map<std::string, std::vector<int>>::iterator got = function_map.find(id);
		if (got != function_map.end())
			return &got->second;

		return NULL;
	}


	Parser_Class_Created(Parser_Identificador* p) : pID(p) {};

	//Aseguramos el borrado de la memoria
	virtual ~Parser_Class_Created()
	{
		delete pID;
		delete normal_operators;
		delete right_operators;
		//deletePtr(static_var_runtime);

		for (std::vector<Parser_Funcion*>::iterator it = funciones.begin(); it != funciones.end(); ++it)
		{
			delete (*it);
		}
		funciones.clear();

		for (std::vector<Parser_ClassConstructor*>::iterator it = constructores.begin(); it != constructores.end(); ++it)
		{
			delete (*it);
		}
		constructores.clear();
		for (std::vector<arbol_operacional*>::iterator it = variables_operar.begin(); it != variables_operar.end(); ++it)
		{
			delete (*it); 
		}
		variables_operar.clear();

		for (std::vector<arbol_operacional*>::iterator it = variables_static.begin(); it != variables_static.end(); ++it)
		{
			delete (*it);
		}
		variables_static.clear();

	};
};


class CoreClass_Function
{
public:
	std::string nombre;
	bool is_Static = false;
	bool (*funcion_exec)(std::shared_ptr<mdox_object>, std::vector<Value>&);

	CoreClass_Function(std::string a) : nombre(a) {}
	CoreClass_Function(std::string a, bool (*b)(std::shared_ptr<mdox_object>, std::vector<Value>&)) : nombre(a), funcion_exec(b) {}
	CoreClass_Function(std::string a, bool (*b)(std::shared_ptr<mdox_object>, std::vector<Value>&), bool c) : nombre(a), funcion_exec(b), is_Static(c) {}

	~CoreClass_Function()
	{};
};

//Clase creada en core
class Parser_Class_Core
{
public:
	std::string nombre;

	//variables de la clase core, siempre se considerarán públicas.
	std::vector<Value> variables_operar;
	std::vector<Value> variables_static;

	std::unordered_map<OPERADORES, int>* operadores_map_right = NULL;
	std::unordered_map<OPERADORES, int>* operadores_map_normal = NULL;
	std::vector<bool (*)(Value*)> operadores;

	std::unordered_map<std::string, int> function_map;
	std::vector<CoreClass_Function> funciones; //Vector donde guardaremos la funcione de la clase

	bool (*constructor)(std::shared_ptr<mdox_object>,std::vector<Value>&);

	int findFuncion(std::string& id)
	{
		//Buscamos los nombres de los índices de la función con el nombre dado entre el mapeado creado durante el parseado.
		std::unordered_map<std::string, int>::iterator got = function_map.find(id);
		if (got != function_map.end())
			return got->second;

		return -1;
	}

	Parser_Class_Core(std::string& n) : nombre(n) {};

};

class Parser_Class : public Parser_NODE
{
public:
	bool isCore;
	Parser_Class_Core* core;
	Parser_Class_Created* created;

	//Mapeo de variables estáticas
	std::unordered_map<std::string, int> static_var_map;
	Variable_Runtime* static_var_runtime = NULL;
	
	//Usado para guardar el nombre de los identificadores y asignarlo a la runtime variable en su momento.
	std::unordered_map<std::string, int> variables_map;

	int preload_var = 0;

	int getIndexOperator(OPERADORES op, bool right)
	{
		if (!isCore)
			assert("ERROR CRITICO INTERNO: Una función normal intenta acceder a execCoreOperator");
		else
		{
			if (right)
			{
				if (core->operadores_map_right == NULL)
					return -1;

				auto got = core->operadores_map_right->find(op);

				if (got != core->operadores_map_right->end())
					return got->second;
				else
					return -1;
			}
			else
			{
				if (core->operadores_map_normal == NULL)
					return -1;

				auto got = core->operadores_map_normal->find(op);

				if (got != core->operadores_map_normal->end())
					return got->second;
				else
					return -1;
			}
		}
	}

	Value execCoreOperator(int index);
	Value execCoreOperator(int index, Value& entrada);

	bool execConstructor(std::shared_ptr<mdox_object> obj, std::vector<Value>& v)
	{
		if (isCore)
			return core->constructor(obj, v);
		else assert(!"ERROR CRITICO INTERNO: Una función normal intenta acceder a execConstructor");
	}


	std::vector<arbol_operacional*>& getVariablesStaticBase()
	{
		if (!isCore)
			return created->variables_static;
		else assert(!"ERROR CRITICO INTERNO: Una función core intenta acceder a getVariablesStaticBase");
	}

	std::vector<Value>& getVariablesStaticCore()
	{
		if (isCore)
			return core->variables_static;
		else assert(!"ERROR CRITICO INTERNO: Una función no CORE intenta acceder a getVariablesStaticCore");
	}

	void preloadFunctions()
	{
		if (!isCore)
			created->preloadFunctions();
		else assert(!"ERROR CRITICO INTERNO: Una función core intenta acceder a preloadFunctions");
	}

	std::vector<Parser_Funcion*>& getFuncionesBase()
	{
		if (!isCore)
			return created->funciones;
		else  assert(!"ERROR CRITICO INTERNO: Una función core intenta acceder a getFuncionesBase");
	}

	std::vector<CoreClass_Function>& getFuncionesCore()
	{
		if (isCore)
			return core->funciones;
		else assert(!"ERROR CRITICO INTERNO: Una función no CORE intenta acceder a getFuncionesCore");
	}

/*	template <class T>
	T getVariableOperar()
	{
		if (!isCore)
			return created->variables_operar;
		else return core->variables_operar; 
	}*/

	std::vector<arbol_operacional*>& getVariableOperarBase()
	{
		if (!isCore)
			return created->variables_operar;
		else assert(!"ERROR CRITICO INTERNO: Una función core intenta acceder a getVariableOperarBase"); //return core->variables_operar;
	}

	std::vector<Value>& getVariableOperarCore()
	{
		if (isCore)
			return core->variables_operar;
		else assert(!"ERROR CRITICO INTERNO: Una función no CORE intenta acceder a getVariableOperarCore"); 
	}

	std::vector<int>* findFuncionBase(std::string& id)
	{
		if (!isCore)
			return created->findFuncion(id);
		else assert(!"ERROR CRITICO INTERNO: Una función del core intenta acceder a findFuncionBase");
		return NULL;
	}

	int findFuncionCore(std::string& id)
	{
		if (isCore)
			return core->findFuncion(id);
		else assert(!"ERROR CRITICO INTERNO: Una función que no es core intenta acceder a findFuncionCore");
	}

	std::vector<Parser_ClassConstructor*>& getConstructores()
	{
		if (!isCore)
			return created->constructores;
		else assert(!"ERROR CRITICO INTERNO: Una función del core intenta acceder a getConstructores");
	}


	void createNormalOperators()
	{
		if (!isCore)
			created->normal_operators = new Operators_List();
		else assert(!"ERROR CRITICO INTERNO: Una función del core intenta acceder a createNormalOperators");
	}

	void createRightOperators()
	{
		if (!isCore)
			created->right_operators = new Operators_List();
		else assert(!"ERROR CRITICO INTERNO: Una función del core intenta acceder a createNormalOperators");
	}

	Operators_List* getNormalOperators()
	{
		if (!isCore)
			return created->normal_operators;
		else assert(!"ERROR CRITICO INTERNO: Una función del core intenta acceder a getNormalOperators");
	}

	Operators_List* getRightOperators()
	{
		if (!isCore)
			return created->right_operators;
		else assert(!"ERROR CRITICO INTERNO: Una función del core intenta acceder a getRightOperators");
	}

	std::string& getNombre()
	{
		if (!isCore)
			return created->pID->nombre;
		else return core->nombre;
	}

	//Valores caché que se limpiarán después del parseado.
	void clearAfterParser()
	{
		static_var_map.clear();
	}

	Parser_Class(Parser_Identificador* p)
	{
		isCore = false;
		created = new Parser_Class_Created(p);
	}

	Parser_Class(std::string& p)
	{
		isCore = true;
		core = new Parser_Class_Core(p);
	}

	~Parser_Class()
	{
		if (isCore)
			delete core;
		else delete created;

		delete[] static_var_runtime;
	}
};


#endif