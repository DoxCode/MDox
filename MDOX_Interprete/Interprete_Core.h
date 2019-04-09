#ifndef INTERPRETE_CORE_H
#define INTERPRETE_CORE_H

#include "../MDOX/Core.h"
#include "Interprete.h"
#include <iostream>
#include <chrono>
#include <thread>

class Core_Function_Interprete : public Core_Function
{
public:
	Value*(*funcion_exec)(std::vector<Value*>*, OutData_Parametros*);

	Core_Function_Interprete(std::string a, std::vector<tipos_parametros> b, tipos_parametros c) : Core_Function(a, b, c) {}

	~Core_Function_Interprete() {};
};


// ----- FUNCIONES INTERPRETE
Value * funcion_print(std::vector<Value*>*, OutData_Parametros *);
Value * funcion_now(std::vector<Value*>* a, OutData_Parametros * b);
Value * funcion_sleep(std::vector<Value*>* a, OutData_Parametros * b);

#endif
