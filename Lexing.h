#ifndef LEXING_H
#define LEXING_H

#include <string>
#include <vector>
#include <tuple>
#include <iostream>
#include "Funciones.h"

enum enum_type
{
	DESCONOCIDA,
	FUNCION,
	OPERACION,
};

enum estilo_codigo_perteneciente
{
	PSEUDOCODIGO,
	MDOX,
};

enum estructuras_integradas
{
	IF,
	FOR,
	WHILE,
};

enum parametros_integrados
{
	INT,
	FLOAT,
	DOUBLE,
	BOOL,
	CHAR,
	STRING,
	LIST,
	VECTOR,
	ARRAY,
	TUPLA,
};


//typedef std::tuple<std::string, enum_type> lex_com;
typedef std::tuple<bool, int> boolVal;


class Objeto_estructura_abstract {
public:
	Objeto_estructura_abstract() {}
	bool es_estatico() { return false; }
};

class Objeto_estructura_estatico : public Objeto_estructura_abstract {
	std::string valor;
public:
	Objeto_estructura_estatico(std::string a) : Objeto_estructura_abstract(), valor(a) {}
	bool es_estatico() { return true; }
};


class Parametro : public Objeto_estructura_abstract {
	parametros_integrados tipo;
public:
	Parametro(parametros_integrados a) : tipo(a) {}
	bool esUnaTupla() { return false; }
	bool es_estatico() { return false; }
	parametros_integrados getTipo() { return tipo; }
};

class Parametro_tupla : public Parametro {
	std::vector<Parametro> params;
public:
	Parametro_tupla(std::vector<Parametro> a) : Parametro(TUPLA), params(a) {}
	bool esUnaTupla() { return true; }
	std::vector<Parametro> getParams() { return params; }
};

class Funcion {
	std::string nombre;
	std::vector<Parametro> parametros;

public:
	Funcion(std::string a, std::vector<Parametro> b ) : nombre(a), parametros(b) {}
};


class Estructura_interna {
	std::string id;
public:
	Estructura_interna(std::string a) : id(a) {}
};


void Lexing(std::string temporal_cache, std::string getWr);
bool funcionT(std::string& ID, std::string getWr);
bool AccionGrt(std::string getWr, std::deque<std::string> funcion);
boolVal Busqueda_Elemento_Medio(std::string& p, std::string getWr);
std::string ModificarValoresBusqueda(std::string& getWr, boolVal ret, const std::string& p2);
#endif