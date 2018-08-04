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


enum tipos_funciones
{
	FUNC_UNKNOWN,
	FUNC_DECLARATIVOS, // Declaracion de funciones

	FUNC_IF,


};

enum tipos_parametros
{
	PARAM_NONE,  // Usos varios
	PARAM_UNKNOWN, //Desconocido
	PARAM_EMPTY, //Salto de linea, linea vacia

	PARAM_DECLARATIVOS, //Parametros declarativos, como "int", "float", etc
	PARAM_VARIABLES,

	//ENTEROS
	PARAM_INT,	 // -2.147.483.647 a 2.147.483.647   (4 bytes)
	PARAM_UINT,	 // 0 a 4.294.967.295				 (4 bytes)
	PARAM_INT16,   // De –32.768 a 32.767              (2 bytes)
	PARAM_UINT16,  // De 0 a 65.535                    (2 bytes)
	PARAM_INT64,   // De –9.223.372.036.854.775.808 a 9.223.372.036.854.775.807  (8 bytes)
	PARAM_UINT64,   // De 0 a 18.446.744.073.709.551.615  (8 bytes)
	PARAM_VOID,
	
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

enum ParamEnumClass
{
	EP_BASE,
	EP_VALOR,
	EP_SINGLE,
	EP_MULTIPLE,

};

enum LexEnumClass
{
	LEX_BASE,
	LEX_FUNCION,  // Se trata de una funcion con valores de entrada y retorno
	  LEX_FUNCION_INTERNA, //Se trata de un modelo de función que tiene además una estructura interna.
};

class estructura_lineas {
public:
	std::string linea;
	int index = 0;
	estructura_lineas(){}
	estructura_lineas(std::string a, int b) : linea(a), index(b) {}

};




// #################### Estructuras de los diferentes parametros. ####################
class estructuraParametro {
public:
	virtual ~estructuraParametro() { };

	tipos_parametros tipo;

	// Usaremos el type para saber de forma estática que clase polimórfica trata. De esta forma
	// evitaremos el uso de dynamic_cast y a cambio usaremos static_cast, que únicamente gastará tiempo en 
	// fase de compilación y evitamos así acceder al proceso en tiempo real y con ello ganar tiempo de procesamiento a cambio
	// de incrementar el coste de memoria.
	ParamEnumClass _getType = EP_BASE;

	estructuraParametro(tipos_parametros a, ParamEnumClass z) : tipo(a), _getType(z) {}
	estructuraParametro(tipos_parametros a) : tipo(a) {}
};

class estructuraParValor : public estructuraParametro {
public:
	virtual ~estructuraParValor(){}

	std::string value = "";

	estructuraParValor(tipos_parametros a, std::string b) : estructuraParametro(a, EP_VALOR), value(b) {}
	estructuraParValor(tipos_parametros a) : estructuraParametro(a, EP_VALOR) {}
};

class estructuraParMultiple : public estructuraParametro {
public:
	std::vector<estructuraParametro*> valores;

	//Destructor ~ Liberamos la memoria de todos los punteros de valores.
	virtual ~estructuraParMultiple() {
		for (std::vector<estructuraParametro*>::iterator it = valores.begin(); it != valores.end(); ++it)
		{
			delete (*it);
		}
		valores.clear();
	};

	estructuraParMultiple(tipos_parametros a, std::vector<estructuraParametro*> b) : estructuraParametro(a, EP_MULTIPLE), valores(b) {}
	estructuraParMultiple(tipos_parametros a) : estructuraParametro(a, EP_MULTIPLE) {}

	//Se usará para agregar parametros comunes, no habra valores añadidos, solo definición
	void addParametro(tipos_parametros a) { addGeneralParam(new estructuraParametro(a)); }
	//Se usará para agregar parametros con valores, estos valores se guardan en un campo std::string
	void addParametroValor(tipos_parametros a) { addGeneralParam(new estructuraParValor(a)); }
	//Se usará para agregar parametros declarativos adicionales, estos podrán llamarse nuevamente para formar listas, se usan para tuplas, estructuras, etc.
	void addParametroMultiple(tipos_parametros a) { addGeneralParam(new estructuraParMultiple(a)); }
	//Estructura general, agrega cualquier clase sobrecargada de estructuraParametro
	void addGeneralParam(estructuraParametro * a) { valores.push_back(a); }
};

class estructuraParSingle : public estructuraParametro {
public:
	estructuraParametro * valor;

	//Destructor ~ Liberamos la memoria de valor.
	virtual ~estructuraParSingle() { 
		delete valor;
	};

	estructuraParSingle(tipos_parametros a, estructuraParametro* b) : estructuraParametro(a, EP_SINGLE), valor(b) {}
	estructuraParSingle(tipos_parametros a) : estructuraParametro(a, EP_SINGLE) {}

	void addParametro(tipos_parametros a) { addGeneralParam(new estructuraParametro(a)); }
	//Se usará para agregar parametros con valores, estos valores se guardan en un campo std::string
	void addParametroValor(tipos_parametros a) { addGeneralParam(new estructuraParValor(a)); }
	//Se usará para agregar parametros declarativos adicionales, estos podrán llamarse nuevamente para formar listas, se usan para tuplas, estructuras, etc.
	void addParametroMultiple(tipos_parametros a) { addGeneralParam(new estructuraParMultiple(a)); }

	void addParametroSingle(tipos_parametros a) { addGeneralParam(new estructuraParSingle(a)); }

	//Estructura general, agrega cualquier clase sobrecargada de estructuraParametro
	void addGeneralParam(estructuraParametro* a) { valor = a; }
};

// #################### Estructuras de las diferentes <Funciones>. ####################
class estructuraLexBase {
public:

	LexEnumClass _getType = LEX_BASE;

	virtual ~estructuraLexBase()
	{
	}

	estructuraLexBase(LexEnumClass a) : _getType(a) {}

};

class estructuraFuncion: public estructuraLexBase {
public:

	tipos_funciones tipo;
	estructuraParametro * paramDevuelto = NULL;
	std::vector<estructuraParametro *> paramsEntrada;

	virtual ~estructuraFuncion()  
	{ 
		deletePtr(paramDevuelto);

		for (std::vector<estructuraParametro*>::iterator it = paramsEntrada.begin(); it != paramsEntrada.end(); ++it)
		{
			delete (*it);
		}
		paramsEntrada.clear();
	}

	estructuraFuncion(tipos_funciones a) : tipo(a), estructuraLexBase(LEX_FUNCION) {}
	estructuraFuncion(tipos_funciones a, LexEnumClass z) : tipo(a), estructuraLexBase(z) {}

};

class estructuraFuncInterna : public estructuraFuncion {
public:
	std::vector<estructuraFuncion *> funcionesInternas;

	virtual ~estructuraFuncInterna()
	{
		for (std::vector<estructuraFuncion*>::iterator it = funcionesInternas.begin(); it != funcionesInternas.end(); ++it)
		{
			delete (*it);
		}
		funcionesInternas.clear();
	}

	estructuraFuncInterna(tipos_funciones a) : estructuraFuncion(a, LEX_FUNCION_INTERNA) {}
};



#endif