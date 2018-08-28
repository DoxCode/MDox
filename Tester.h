#ifndef TESTER_H
#define TESTER_H

#include "Estructuras.h"

void debug_mostrar(Parser_Literal* p, int tb);
void debug_mostrarDec(Parser_Declarativo* p, int tb);
void debug_mostrarID(Parser_Identificador* p, int tb);
void debug_mostrarValor(Parser_Valor* p, int tb);
void debug_mostrarMath(Parser_Math* p, int tb);
void debug_mostrarOperaciones(Parser_Operacion* p, int tb);
void debug_mostrarCondicional(Parser_Condicional* p, int tb);
void debug_mostrarIgualdad(Parser_Igualdad* p, int tb);
void debug_mostrarParametro(Parser_Parametro* p, int tb);
void debug_mostrarSentencia(Parser_Sentencia* p, int tb);
void debug_mostrarFuncion(Parser_Funcion* p, int tb);

std::string NameConditions(CondicionalAccionType a);
std::string NameParameter(tipos_parametros a);
std::string tabulacion(int n);
#endif
