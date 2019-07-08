#ifndef INTERPRETE_CORE_H
#define INTERPRETE_CORE_H

#include "Interprete.h"

// ----- FUNCIONES INTERPRETE
bool funcion_print(std::vector<Value>&, Interprete*);
bool funcion_now(std::vector<Value>&, Interprete*);
bool funcion_sleep(std::vector<Value>&, Interprete*);

#endif
