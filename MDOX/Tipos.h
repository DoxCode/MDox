#ifndef TIPOS_H
#define TIPOS_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdio>

#include "Funciones.h"
#include "Estructuras.h"

#include "Core.h"

void aaaa()
{
	Core_Functions::function_print(new Value());
}

tipos_parametros DeclarativoToParamType(Parser_Declarativo * pDec);



#endif