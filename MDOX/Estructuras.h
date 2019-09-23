#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include <variant>
#include <string>
#include <vector>
#include <list>
#include <tuple>
#include <iostream>
#include <unordered_map>

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



//Valores que tendr� cada NODO, la l�nea a la que pertenece, el offset, y el nombre del fichero.
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
	//PARAM_STRUCT, //Dise�ado por el usuario.


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

	//Operadores aritm�ticos
	//Prior 8
	OP_BRACKET_LEFT, //El operador offset es [op]
	OP_BRACKET_RIGHT,
	

	//Prior 7
	OP_SCOPE_LEFT,
	OP_SCOPE_RIGHT,
	OP_CLASS_ACCESS, //Operaci�n de acceso a clase '.'
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

	//Operadores l�gicos
	//Prior 3
	OP_LOG_ADD,
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





class mdox_object;
class Variable_Runtime;
class Parser_Identificador;
class Value;

// Una llamada del estilo -> "a.x" lo transforma en un solo objeto.
//Por lo que para: a.b.x -> [ab].x -> [abx]
class wrapper_object_call
{
public:
	std::shared_ptr<mdox_object> objeto;
	Parser_Identificador* var;
	Value* getValue(bool, bool b = false);
	wrapper_object_call(std::shared_ptr<mdox_object>& a, Parser_Identificador* x) : objeto(a), var(x) {}
};

using val_variant = std::variant< std::monostate, int, bool, double, std::shared_ptr<mdox_vector>, std::shared_ptr<mdox_object>, long long, std::string, wrapper_object_call, Variable_Runtime*>;

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
	
	Value ClassAccess(Parser_Identificador* v2, Call_Value * call = nullptr, Variable_Runtime* variables = nullptr, Variable_Runtime* var_class = nullptr);
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
	Value(std::shared_ptr<mdox_vector>& a) : value(std::make_shared<mdox_vector>(*a)) {  };
	Value(std::shared_ptr<mdox_object>& a) : value(a) { };
	Value(wrapper_object_call& a) : value(a) { };
	Value(Variable_Runtime* a) : value(a) { };


	Value(int&& a) : value(std::move(a)) {  };
	Value(double&& a) : value(std::move(a)) {  };
	Value(long long&& a) : value(std::move(a)) {  };
	Value(std::string&& a) : value(std::move(a)) {  };
	Value(bool&& a) : value(std::move(a)) {  };
	Value(std::shared_ptr<mdox_vector>&& a) : value(std::move(a)) {  };

	Value(val_variant& a) : value(std::visit(overloaded
		{ 
			[&](std::shared_ptr<mdox_vector>& c)->val_variant {  return std::make_shared<mdox_vector>(*c); },
			[&](auto & c)->val_variant {  return a; },
		}, a)) {  };

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
	Parser_Class* static_link = nullptr; //Enlace a la clase si es estatica la operaci�n

	int index = -1;
	std::string nombre;
	bool fuerte = false;
	bool inicializando = false;


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
	std::vector<int> funcionesCoreItrData;  //Direcci�n de la funci�n Core
	std::vector<int> funcionesItrData;		//Direcci�n de la funci�n
};

class IndexCall_Class
{
public:
	int class_index = -2; // -1: Constructor por defecto, -2: ERROR ser� enviado - llamada no valida
	int constructor_index;
	int _especial_var; //usado para especificar la variable est�tica a la cual acceder�.
};

//Basicamente se tratar�a de funciones con un retorno de un valor dado.
// Tambi�n nos servir� como llamada al constructor de una clase
// DEFAULT: ClassName()
// CONSTRUCTORES: ClassName(x,y,z)
// B�sicamente las llamadas son iguales a las de una funci�n.
class Call_Value : public Parser_NODE
{
public:
	Parser_Identificador* ID;
	std::vector<arbol_operacional*> entradas;


	//Si valor enlace = NULL implica que en ning�n momento podr� tratarse de un valor est�tico.
	//En caso contrario la posibilidad existe y tiene que ser tratada en el preloadCalls.
	//Este valos se modifica en el preload de operaciones.
	bool is_Static = false;
	arbol_operacional* valor_enlace = NULL;
	Parser_Class* class_link_static = nullptr; // SOLO si es estatico
	bool function_parent_is_static = false; // Indica si la funci�n en la que se ejecuta la llamada es est�tica o no.

	bool skip = false; //Indica si este call debe ser saltado o no en el preload.

	
	//En el caso de que la llamada sea a una funci�n
	IndexCall_Function* inx_funcion = NULL;

	//En el caso de que la llamda sea a una clase
	IndexCall_Class* inx_class = NULL;

	//Indica si la clase o funci�n pertenece a otra clase, y en caso de hacerlo, a cual.

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
	tipoValor v1; // Ser� el vector si se tratan de dos operaciones
	tipoValor v2; // Ser� el vector en caso de 3 operaciones
	tipoValor v3;

	bool onlyValue = false; // Si es true, no existen identificadores las operaciones
	bool dobleOperador = true;

	Parser_Identificador* id_v =  NULL;
	/**
	 Funcion que comprueba si el dato introducido se trata de un Valor que puede tratarse de un vector.
	 Devolver� valor a id_v si se trata de un identificador d�bil, pues puede tratarse de un vector.
	 Devolver� NULL si es un multivalor o un identificador fuerte.
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
	SENT_ACCION,
	SENT_PRINT,
	SENT_INPUT,
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
	IGNORE,		//IGNORA la funci�n actual y pasa a buscar la siguiente.
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

	// Funci�n sin valor devuelto, el valor devuelto puede ser autom�tico o no tenerlo
	Parser_Funcion(Parser_Identificador* a, std::vector<arbol_operacional*> b, Parser_Sentencia* c) : pID(a), entradas(b), body(c), salida(NULL) {}
	//Funci�n completa
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
	Parser_ClassConstructor()  {};
	Parser_ClassConstructor(std::vector<int>& a) : entradas(a) {};
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
	//No todos los operadores ser�n permitidos.
	//Por ejemplo, los '()' no se considerar�n operadores.
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


class Parser_Class : public Parser_NODE
{
public:
	Parser_Identificador* pID;
	std::vector<Parser_Funcion*> funciones; //Vector donde guardaremos la funcione de la clase

	//Variables est�ticas de la clase
	bool valores_estaticos_iniciados = false;
	std::vector<arbol_operacional*> variables_static;
	std::unordered_map<std::string, int> static_var_map;
	Variable_Runtime* static_var_runtime=NULL;


	//Mapeado de las funciones.
	// Nos servir� para crear un pair entre el nombre de la funci�n y el �ndice (funciones[indice]) con coste O(1) de
	// todas las funciones ya agregadas en la clase, este valor se crear� al finalizar la clase durante el parseado, llamando a la funci�n preloadFunctions
	std::unordered_map<std::string, std::vector<int>> function_map; 
	
	std::vector<Parser_ClassConstructor*> constructores;
	std::vector<arbol_operacional*> variables_operar;

	//Usado para guardar el nombre de los identificadores y asignarlo a la runtime variable en su momento.
	std::unordered_map<std::string, int> _variables_map;

	Operators_List* normal_operators = NULL; //Operadores del estilo this+x;  El objeto se encuentra en la izquierda de la operaci�n.
	Operators_List* right_operators = NULL;  //Operadores del estilo x+this;  El objeto se encuentra en la derecha de la operaci�n.

	int preload_var = 0;

	//Funci�n que deber� ser llamada al finalizar de construir la clase de forma obligatoria para crear el pair entre indices y funciones, que nos servir� como tabla
	// hash para mayor rapidez de ejecuci�n en runtime
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
		//Buscamos los nombres de los �ndices de la funci�n con el nombre dado entre el mapeado creado durante el parseado.
		std::unordered_map<std::string, std::vector<int>>::iterator got = function_map.find(id);
		if (got != function_map.end())
			return &got->second;

		return NULL;
	}


	Parser_Class(Parser_Identificador* p) : pID(p) {};

	//Aseguramos el borrado de la memoria
	virtual ~Parser_Class()
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


#endif