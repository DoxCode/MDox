
#include "Interprete.h"
#include "../MDOX/Errores.cpp"
#include "../MDOX/Parser.cpp"

bool Interprete::CargarDatos(Parser* parser)
{
	//Borramos la lista de funciones existentes actualmente.
	for (std::vector<Parser_Funcion*>::iterator it = funciones.begin(); it != funciones.end(); ++it)
	{
		delete (*it);
	}
	funciones.clear();

/*	std::cout << "TOKENS: \n";

	for (unsigned i = 0; i < parser->tokenizer.tokens.size(); i++)
	{
		std::cout << "  [" << parser->tokenizer.tokens.at(i)->token << "]  ";
	}

	std::cout << "\n\n";*/


	this->nombre_ficheros.push_back(parser->tokenizer.fichero);
	
	//Interpretando datos...size_t
	int local = 0;
	while (local < parser->tokenizer.tokens.size())
	{
		//	std::cout << "--:: " << parser.tokenizer.tokens.at(local) << " :: " << local << " \n";

		Parser_Funcion* p2 = parser->getFuncion(local);

		if (p2)
		{
			funciones.push_back(p2);
			continue;
		}
		else
		{
			OutData_Parametros data = OutData_Parametros(parser->tokenizer.token_actual->linea, parser->tokenizer.token_actual->char_horizontal, this->nombre_ficheros.back());
			Errores::generarError(Errores::ERROR_DE_SINTAXIS, &data, parser->tokenizer.token_actual->token);
			return false;
		}
	}

	std::cout << "Fichero incluido correctamente. \n";
	return true;
}

bool Interprete::Interpretar(Parser * parser)
{
	std::vector<Variable*> variables;

	int local = 0;
	while (local < parser->tokenizer.tokens.size())
	{
		//	std::cout << "--:: " << parser.tokenizer.tokens.at(local) << " :: " << local << " \n";

		Parser_Sentencia* p2 = parser->getSentencia(local);
		

		if (p2)
		{
			if (!Interprete_Sentencia(p2, &variables))
			{
				//TODO: TESTING PARA LEER TODO
				//delete p2;
				//return false;
			}
		}
		else
		{
			OutData_Parametros data = OutData_Parametros(parser->tokenizer.token_actual->linea, parser->tokenizer.token_actual->char_horizontal, NULL);
			Errores::generarError(Errores::ERROR_DE_SINTAXIS, &data, parser->tokenizer.token_actual->token);
			//delete p2;
			return false;
		}
		//std::cout << "<LIBERANDO SENTENCIA>";
		delete p2;

	}

	for (std::vector<Variable*>::iterator it = variables.begin(); it != variables.end(); ++it)
	{
		delete (*it);
	}

	return true;
}

bool Interprete::Interprete_Sentencia(Parser_Sentencia * sentencia, std::vector<Variable*> * variables)
{
	switch (sentencia->tipo)
	{
		case SENT_REC:
		{
			Sentencia_Recursiva * x = static_cast<Sentencia_Recursiva*>(sentencia);

			std::vector<Variable*> variables2 = *variables;
			deep++;

			for (std::vector<Parser_Sentencia*>::iterator it = x->valor.begin(); it != x->valor.end(); ++it)
			{
				if (!Interprete_Sentencia(*it, &variables2))
				{
					//No devolverá error en principio
					return false;
				}
			
				if (_retorno != NULL)
					break;
				//if ((*it)->tipo == SENT_RETURN)
				//	break;		
			}

			for (std::vector<Variable*>::iterator it = variables2.begin(); it != variables2.end(); ++it)
			{
				if ((*it)->deep == deep)
					delete (*it);
			}
			deep--;

			return true;
		}

		//Inicializa una variable dada
		case SENT_VAR_INI:
		{
			Sentencia_Parametro * x = static_cast<Sentencia_Parametro*>(sentencia);
			if (Interprete_NuevaVariable(x->pPar, variables, false) == NULL) // Crea nueva variable, si ya existe devuelve NULL
			{
				//std::cout << " Linea: [" << x->linea << "::" << x->offset << "] ";
				return false;
			}

				return true;
		}

		// Add+1 -> También se podrá llamar a operaciones matemáticas, pero su resultado nunca se guardará. 
		//  Esto se debe a que se puede ejecutar directamente funciones aún sin ser return void.
		case SENT_OP:
		{
			Sentencia_Operacional * x = static_cast<Sentencia_Operacional*>(sentencia);

			if (!EstablecerOperacion(x->pOp, variables))
			{
				//std::cout << " Linea: [" << x->linea << "::" << x->offset << "] ";
				return false;
			}

			return true;
		}
		case SENT_IGU:
		{
			Sentencia_Igualdad * x = static_cast<Sentencia_Igualdad*>(sentencia);

			if (!EstablecerIgualdad(x->pIg, variables))
			{
				//std::cout << " Linea: [" << x->linea << "::" << x->offset << "] ";
				return false;
			}

			return true;
		}
		// FUNCION PRINT:
		// Permite escribir por consola un valor dado.
		case SENT_PRINT:
		{
			Sentencia_Print * x = static_cast<Sentencia_Print*>(sentencia);
			Value * v = Operaciones(x->pOp, variables);

			if (!v)
				Errores::generarError(Errores::ERROR_FUNCION_PARAMETRO_OPERACION_INVALIDA, &x->parametros, "print");

			if (v)
			{
				switch (v->getTypeValue())
				{
					case PARAM_VOID:
					{
						std::cout << "void";
						break;
					}
					case PARAM_LINT:
					{
						Value_LINT * x = static_cast<Value_LINT*>(v);
						std::cout << x->value;
						break;
					}
					case PARAM_INT:
					{
						Value_INT * x = static_cast<Value_INT*>(v);
						std::cout << x->value;
						break;
					}
					case PARAM_STRING:
					{
						Value_STRING * x = static_cast<Value_STRING*>(v);
						ReplaceAll(x->value, "\\n", "\n");
						std::cout << x->value;
						break;
					}
					case PARAM_DOUBLE:
					{
						Value_DOUBLE * x = static_cast<Value_DOUBLE*>(v);
						std::cout << x->value;
						break;
					}
					case PARAM_BOOL:
					{
						Value_BOOL * x = static_cast<Value_BOOL*>(v);
						std::cout << x->value;
						break;
					}
					default:
						std::cout << v;
				}

				delete v;
				return true;
			}
			else
				return false;

		}
		// Permite escribir por consola un valor dado.
		case SENT_RETURN:
		{
			Sentencia_Return * x = static_cast<Sentencia_Return*>(sentencia);
			if (x->pOp) 
			{
				setRetorno(Operaciones(x->pOp, variables));
			}
			else
			{
				nullRetorno();
			}

			return true;
		}
		// FUNCION IF:
		// Permite la comprobacion de operaciones o variables.
		case SENT_IF:
		{
			Sentencia_IF * x = static_cast<Sentencia_IF*>(sentencia);
			
			Value * b = Operaciones(x->pCond, variables);

			if (b == NULL)
				return false;

			bool value_bool = this->ConversionXtoBool(b);
			delete b; // No necesitamos más la operación

			if (value_bool)
			{
				if (x->pS)
				{
			
					if (!Interprete_Sentencia(x->pS, variables))
					{
						//std::cout << " Linea: [" << x->linea << "::" << x->offset << "] ";
						return false;
					}
					return true;
				}
			}
			else
			{
				if (x->pElse)
				{
					if (!Interprete_Sentencia(x->pElse, variables))
					{
						//std::cout << " Linea: [" << x->linea << "::" << x->offset << "] ";
						return false;
					}
					return true;
				}
			}

			return true;
		}
		// FUNCION WHILE:
		// Repite la sentencia hasta que la condición del bucle while se cumpla.
		case SENT_WHILE:
		{
			Sentencia_WHILE * x = static_cast<Sentencia_WHILE*>(sentencia);


			while (true)
			{
				Value * b = Operaciones(x->pCond, variables);

				if (b == NULL)
					return false;

				bool value_bool = this->ConversionXtoBool(b);
				delete b;

				if (value_bool)
				{
					if (x->pS)
					{
						if (!Interprete_Sentencia(x->pS, variables))
						{
							//std::cout << " Linea: [" << x->linea << "::" << x->offset << "] ";
							return false;
						}
					}
				}
				else
				{
					return true;
				}
			}
		}
		// FUNCION FOR:
		// Repite la sentencia hasta que la condición del bucle while se cumpla.
		case SENT_FOR:
		{
			Sentencia_FOR * x = static_cast<Sentencia_FOR*>(sentencia);
			//Interprete_Sentencia(x->pIguald, variables);

			if (x->pIguald)
			{
				if (!EstablecerIgualdad(x->pIguald, variables))
				{
					//std::cout << " Linea: [" << x->linea << "::" << x->offset << "] ";
					return false;
				}

			}

			while (true)
			{
				bool res = true;

				if (x->pCond)
				{
					Value * b = Operaciones(x->pCond, variables);

					if (b == NULL)
					{
						//std::cout << " Linea: [" << x->linea << "::" << x->offset << "] ";
						return false;
					}

					bool value_bool = this->ConversionXtoBool(b);
					delete b;

					res = value_bool;
				}

				if (res)
				{
					if (!Interprete_Sentencia(x->pS, variables))
					{
						//std::cout << " Linea: [" << x->linea << "::" << x->offset << "] ";
						return false;
					}			

					if (x->pOp)
					{
						if (!EstablecerOperacion(x->pOp, variables))
						{
						//	std::cout << " Linea: [" << x->linea << "::" << x->offset << "] ";
							return false;
						}
					}
				}
				else
				{
					return true;
				}
			}
		}


	}

}



//Establece una operación, una operación puede ser el incremento/decremento de una variable o una operación matemática
bool Interprete::EstablecerOperacion(Parser_Operacion * pOp, std::vector<Variable*> * variables)
{
	if (pOp->tipo == OP_ID)
	{
		Operacion_ID * x2 = static_cast<Operacion_ID*>(pOp);

		if (x2->accion == ID_INCREMENTO)
		{
			Variable * v1 = BusquedaVariable(x2->ID->nombre, variables);

			if (v1)
			{
				if (v1->valor->getTypeValue() == PARAM_INT)
				{
					Value_INT * x3 = static_cast<Value_INT*>(v1->valor);
					Value * xR = new Value_INT(x3->value + 1);

					if (!EstablecerVariable(v1, &xR, &x2->ID->parametros))
					{
						delete xR;
						return false;
					}
					return true;
				}
				else if (v1->valor->getTypeValue() == PARAM_LINT)
				{
					Value_LINT * x3 = static_cast<Value_LINT*>(v1->valor);
					Value * xR = new Value_LINT(x3->value + 1);

					if (!EstablecerVariable(v1, &xR, &x2->ID->parametros))
					{
						delete xR;
						return false;
					}
					return true;
				}
				else if (v1->valor->getTypeValue() == PARAM_DOUBLE)
				{
					Value_DOUBLE * x3 = static_cast<Value_DOUBLE*>(v1->valor);
					Value * xR = new Value_DOUBLE(x3->value + 1);

					if (!EstablecerVariable(v1, &xR, &x2->ID->parametros))
					{
						delete xR;
						return false;
					}
					return true;
				}
				else return false;
				

			}
			else {
				Errores::generarError(Errores::ERROR_INCREMENTO_VARIABLE_DESCONOCIDA, &x2->parametros, x2->ID->nombre);
				return false;
			}
		}
		else  if (x2->accion == ID_DECREMENTO)
		{
			Variable * v1 = BusquedaVariable(x2->ID->nombre, variables);

			if (v1)
			{
				if (v1->valor->getTypeValue() == PARAM_INT)
				{
					Value_INT * x3 = static_cast<Value_INT*>(v1->valor);
					Value * xR = new Value_INT(x3->value - 1);

					if (!EstablecerVariable(v1, &xR, &x2->ID->parametros))
					{
						delete xR;
						return false;
					}
					return true;
				}
				else if (v1->valor->getTypeValue() == PARAM_LINT)
				{
					Value_LINT * x3 = static_cast<Value_LINT*>(v1->valor);
					Value * xR = new Value_LINT(x3->value - 1);

					if (!EstablecerVariable(v1, &xR, &x2->ID->parametros))
					{
						delete xR;
						return false;
					}
					return true;
				}
				else if (v1->valor->getTypeValue() == PARAM_DOUBLE)
				{
					Value_DOUBLE * x3 = static_cast<Value_DOUBLE*>(v1->valor);
					Value * xR = new Value_DOUBLE(x3->value - 1);

					if (!EstablecerVariable(v1, &xR, &x2->ID->parametros))
					{
						delete xR;
						return false;
					}
					return true;
				}
				else 
				{
					Errores::generarError(Errores::ERROR_INC_DEC_VARIABLE_INVALIDA, &x2->parametros, x2->ID->nombre);
					return false;
				}
				

			}
			else {
				Errores::generarError(Errores::ERROR_INCREMENTO_VARIABLE_DESCONOCIDA, &x2->parametros, x2->ID->nombre);
				return false;
			}
			
		}
		else return false;

	}
	else //Si se trata de una operación matemática.
	{
		Value * _res = Operaciones(pOp, variables);

		//Si devuelve algún valor la operación, lo borra, no lo usaremos.
		if (_res)
			delete(_res);
		else
			return false;

		return true;
	}

	return false;
}

bool Interprete::EstablecerIgualdad(Parser_Igualdad * pIg, std::vector<Variable*> * variables)
{
	Variable * var = Interprete_NuevaVariable(pIg->param, variables, true);

	if (var)
	{
		if (pIg->cond)
		{
			//TODO BORRAR
			/*if (pIg->cond->tipo == COND_OP)
			{
				Condicional_Operacion * xOp = static_cast<Condicional_Operacion*>(pIg->cond);
				if (xOp->ca == NULL && xOp->adicional_ops == NULL)
				{
					Value * v = Operaciones(xOp->op1, variables);

					if (v == NULL)
					{
						delete v;
						return false;
					}

					if (!EstablecerVariable(var, &v, &pIg->parametros))
					{
						delete v;
						return false;
					}
					delete v;
					return true;
				}
			}*/

			Value * v_bol = Operaciones(pIg->cond, variables);

			if (v_bol == NULL)
			{
				return false;
			}

			if (!EstablecerVariable(var, &v_bol, &pIg->parametros))
			{
				delete v_bol;
				return false;
			}
			return true;
		}
	}
	return false;
}

Variable * Interprete::Interprete_NuevaVariable(Parser_Parametro * par, std::vector<Variable*> * variables, bool existe)
{
	switch (par->tipo)
	{
		case PRM_DECLARATIVO_ID: // Se proporciona la ID y el tipo de variable.
		{
			Parametro_Declarativo_ID * x = static_cast<Parametro_Declarativo_ID*>(par);
			
			Variable * var1 = BusquedaVariable(x->pID->nombre, variables);
			// 1º comprobamos si dicha ID existe.
			if (var1 == NULL)
			{
				//No existe
				Value* v = Transformar_Declarativo_Value(x->pDec);

				if (v)
				{
					if (v->getTypeValue() == PARAM_VOID)
					{
						Variable * v = new Variable(x->pID->nombre, NULL, false);
						v->deep = deep;
						variables->push_back(v);
						return v;
					}
					else
					{
						Variable * var = new Variable(x->pID->nombre, v, true);
						var->deep = deep;
						variables->push_back(var);
						return var;
					}
				}

				Errores::generarError(Errores::ERROR_INICIALIZACION_VARIABLE, &x->pID->parametros, x->pID->nombre);
				return NULL;
			}
			else
			{
				Errores::generarWarning(Errores::WARNING_VARIABLE_YA_DECLARADA, &x->pID->parametros, x->pID->nombre);

				if (existe)
				{
					return var1;
				}
				//Existe

				return NULL; // O NULL si queremos que lo de arriba sea error
			}
			break;
		}

		case PRM_ID: // Se proporciona la ID el tipo de parametro de la variable no se conocerá hasta que se inicialice.
		{
			Parser_Identificador * x = static_cast<Parser_Identificador*>(par);
			Variable * var1 = BusquedaVariable(x->nombre, variables);
			
			if (var1)
				return var1;		

			Variable * v = new Variable(x->nombre, NULL, false);
			v->deep = deep;
			variables->push_back(v);
			return v;
		}
		default:
			Errores::generarError(Errores::ERROR_DESC_DECLARACION_VARIABLE, &par->parametros);
			return NULL;
	}
}

Variable* Interprete::BusquedaVariable(std::string ID, std::vector<Variable*> * variables)
{
	for (std::vector<Variable*>::iterator it = variables->begin(); it != variables->end(); ++it)
	{
		if ((*it)->nombre == ID)
			return (*it);
	}
	return NULL;
}

// Ejecuta la función dentro del ENTORNO. Es decir, se trata de una función que NO está fuera del entorno de llamada. (Es decir, no forma parte de una clase diferente)
// Las variables a las cuales tiene acceso esta función, serán las variables del entorno propio, es decir VARIABLES GLOBALES, variables declaradas a nivel de main.
Value* Interprete::ExecFuncion(std::string ID, Valor_Funcion * xFunc, std::vector<Variable*> * variablesActuales /*Variables del entorno anterior*/ )
{
	std::vector<Value*> * vec_func = new std::vector<Value*>();
	vec_func->reserve(xFunc->entradas.size());

	for (std::vector<Parser_Operacion*>::iterator it = xFunc->entradas.begin(); it != xFunc->entradas.end(); ++it)
	{
		Value * val_itr = Operaciones((*it), variablesActuales);

		if (val_itr == NULL)
		{
			break;
		}

		vec_func->push_back(val_itr);
	}

	//Si los tamaños de los arrays no son iguales, es que ha habido algún fallo operacional.
	// Por lo que borramos el array que creamos en el heap.
	if (vec_func->size() != xFunc->entradas.size())
	{
		for (std::vector<Value*>::iterator it = vec_func->begin(); it != vec_func->end(); ++it)
		{
			delete (*it);
		}
		vec_func->clear();

		Errores::generarError(Errores::ERROR_FUNCION_ERROR_OPERACIONES_ENTRADA, &xFunc->parametros, ID);
		delete vec_func;
		return NULL;
	}

	std::vector<Variable*> variablesPublicas;
	//Tomamos 
	for (std::vector<Variable*>::iterator it = variablesActuales->begin(); it != variablesActuales->end(); ++it)
	{
		if ((*it)->deep == 0) // DEEP 0 = VARIABLE GLOBAL.
			variablesPublicas.push_back((*it));
	}

	Interprete_Funcion_Entradas * xCallEnt = new Interprete_Funcion_Entradas(vec_func);

	//De entre todas las funciones declaradas, buscamos la que realmente se está llamando.
	for (std::vector<Parser_Funcion*>::iterator funcion = funciones.begin(); funcion != funciones.end(); ++funcion)
	{
		//Debe coincidir el nombre de la misma.
		if ((*funcion)->pID->nombre == ID)
		{
			//si no tiene el mismo numero de entradas, saltamos, no es esta función.
			if ((*funcion)->entradas.size() != xCallEnt->entradas->size())
				continue;

			bool correcto = true; // Si es FALSE se limpiará el heap de las variables creadas, pero no ejecutará la función.
			bool forzar_salida = false; //Se devolverá NULL

			std::vector<Variable*> variablesEntorno = variablesPublicas;

			for (unsigned ent_itr = 0; ent_itr < xCallEnt->entradas->size(); ent_itr++)
			{
				//La entrada de la funcion requiere de una operación.
				// Esta operación actuará como un if, solamente se podrá acceder a la función si la operación
				// de dicha entrada es igual al valor de llegada.
				if ((*funcion)->entradas.at(ent_itr)->tipo == ENTRADA_OP)
				{
					Parser_Operacion * xOperacion = static_cast<Parser_Operacion*>((*funcion)->entradas.at(ent_itr));

					//Realizamos la operación oportuna de la entrada de la función dada, esta operación se realiza
					//con las variables ya declaradas en la propia entrada de la función y/o con variables públicas
					Value* valor_operacion = Operaciones(xOperacion, &variablesEntorno);

					if (valor_operacion == NULL)
					{
						forzar_salida = true; 
						break;
					}

					//Comprobamos el valor de entrada con el operacional.
					Value_BOOL * _Cond = CondicionalDeDosValores(valor_operacion, OP_REL_EQUAL, xCallEnt->entradas->at(ent_itr), &xOperacion->parametros);

					//De no poder convertir la operación correctamente, nos saltamos esta función. Pues no es posible realizarla.
					if (_Cond == NULL)
					{
						delete valor_operacion;
						correcto = false;
						break;
					}

					// En el caso de que el valor no sea IGUAL, también salimos de esta función pues no será la correcta.
					if (!_Cond->value)
					{
						delete valor_operacion;
						delete _Cond;
						correcto = false;
						break;
					}

					//La entrada coincide con el valor operacional.
					//Por lo que ya no necesitamos los valores creados, los borramos de memoria y continuamos con el resto de entradas.
					delete valor_operacion;
					delete _Cond;
					continue;
				}
				//La entrada se trata "en principio" de un parametro. Es decir, una variable nueva declarada.
				else if ((*funcion)->entradas.at(ent_itr)->tipo == ENTRADA_PARAM)
				{
					Parser_Parametro* xParametro = static_cast<Parser_Parametro*>((*funcion)->entradas.at(ent_itr));

					//Si el parametro es DECLARATIVO + ID, es decir algo como: "int x"
					// se tratará siempre de una VARIABLE LOCAL de la función, que se debe pasar al llamar a la función.

					// SE producirá UN ERROR: Si dicha variable ya fue declarada.
					if (xParametro->tipo == PRM_DECLARATIVO_ID)
					{
						Parametro_Declarativo_ID* _xParID = static_cast<Parametro_Declarativo_ID*>(xParametro);
						if (BusquedaVariable(_xParID->pID->nombre, &variablesEntorno) != NULL)
						{
							Errores::generarError(Errores::ERROR_FUNCION_ENTRADA_DECLARADA, &_xParID->pID->parametros, _xParID->pID->nombre, ID);
							correcto = false;
							break;
						}

						Value* v = Transformar_Declarativo_Value(_xParID->pDec);
						Variable * _var = new Variable(_xParID->pID->nombre, v, true);
						_var->deep = deep;

						if (!EstablecerVariable(_var, &(xCallEnt->entradas->at(ent_itr)), &_xParID->pID->parametros))
						{
							forzar_salida = true;
							break;
						}

						variablesEntorno.push_back(_var);
					}
					//PRM_ID  -> Si es un identificador, puede ser tomado como VALOR o como PARAMETRO DEBIL. ESTO HAY QUE TENERLO EN CUENTA. Dependerá de si existe o no la variable ID.
					//EJEMPLO:  function funcion(X,Y,Y) -> El parametro X e Y son considerados parametros declarados de la función, para la segunda Y implicará que el parametro
					// de entrada 2 y 3 deben ser iguales. En caso contrario la función no debe ser elegida.
					else if (xParametro->tipo == PRM_ID)
					{
						Parser_Identificador* _xID = static_cast<Parser_Identificador*>(xParametro);

						Variable * _var = BusquedaVariable(_xID->nombre, &variablesEntorno);

						//La variable YA existe, ende no estamos inicializándola, si no más bien comparándola.
						if (_var)
						{
							Value_BOOL * _Cond = CondicionalDeDosValores(_var->valor, OP_REL_EQUAL, xCallEnt->entradas->at(ent_itr), &_xID->parametros);

							//De no poder convertir la operación correctamente, nos saltamos esta función. Pues no es posible realizarla.
							if (_Cond == NULL)
							{
								correcto = false;
								break;
							}

							// En el caso de que el valor no sea IGUAL, también salimos de esta función pues no será la correcta.
							if (!_Cond->value)
							{
								correcto = false;
								delete _Cond;
								break;
							}

							delete _Cond;
							continue;
						}
						else // La variable NO existe, ende la creamos.
						{
							Variable * _var = new Variable(_xID->nombre, xCallEnt->entradas->at(ent_itr)->Clone(), false);
							_var->deep = deep;
							variablesEntorno.push_back(_var);
						}
					}
					//El resto de valors posibles del parametro dan igual, pues se supone que son variables recibidas.
				}
				else
				{
					forzar_salida = true;
					break;
				}
			}

			//Limpiamos variables si algo ha ido mal.
			//Y con mal decimos, que la función buscada no es la del iterador actual, si no, otra con la misma ID, o simplemente hay un fallo.
			if (!correcto || forzar_salida)
			{
				for (std::vector<Variable*>::iterator it = variablesEntorno.begin(); it != variablesEntorno.end(); ++it)
				{
					if ((*it)->deep != 0) // Si no es una variable pública la eliminamos
						deletePtr(*it);
				}
				variablesEntorno.clear();


				if (forzar_salida)
				{
					delete xCallEnt;
					return NULL;
				}

				continue;
			}


			if (Interprete_Sentencia((*funcion)->body, &variablesEntorno))
			{
				for (std::vector<Variable*>::iterator it = variablesEntorno.begin(); it != variablesEntorno.end(); ++it)
				{
					if ((*it)->deep != 0) // Si no es una variable pública la eliminamos
						deletePtr(*it);
				}
				variablesEntorno.clear();	
				deletePtr(xCallEnt);

				//Si la salida es NULA es que no se ha especificado una, la salida será igual al valor de retorno.
				if((*funcion)->salida == NULL)
					return getRetorno();
				else
				{
					Value* v = Transformar_Declarativo_Value((*funcion)->salida);
					Value * ret = viewRetorno()->Clone();

					if (v->getTypeValue() == PARAM_VOID)
					{
						if (ret == NULL)
						{
							return v;
						}
						else
						{
							return v;
						}

					}



					if (ret != NULL)
					{
						if (ValueConversion(v, &ret, &(*funcion)->parametros, ID))
						{
							nullRetorno();
							return v;
						}
						else
						{
							//ERROR;
							nullRetorno();
							return NULL;
						}
					}

				}

			}
			else
			{
				for (std::vector<Variable*>::iterator it = variablesEntorno.begin(); it != variablesEntorno.end(); ++it)
				{
					if ((*it)->deep != 0) // Si no es una variable pública la eliminamos
						deletePtr(*it);
				}
				variablesEntorno.clear();


				nullRetorno();

				deletePtr(xCallEnt);

				return new Value();
			}
		}
	}
	//AGREGADO TODO
	delete xCallEnt;

	Errores::generarError(Errores::ERROR_FUNCION_NO_RECONOCIDA, &xFunc->parametros, ID);
	return NULL;
}

bool Interprete::ValueConversion(Value * val1, Value ** val2, OutData_Parametros * outData, std::string nombre)
{
	switch (val1->getTypeValue())
	{

	case PARAM_VOID:
	{
		deletePtr(val1);
		val1 = (*val2)->Clone();
		return true;
	}
	case PARAM_INT:
	{
		if ((*val2)->getTypeValue() == PARAM_INT)
		{
			Value_INT * xVar = static_cast<Value_INT*>(val1);
			Value_INT * xVal = static_cast<Value_INT*>(*val2);
			xVar->value = xVal->value;
			deletePtr(*val2);
			return true;
		}
		else if ((*val2)->getTypeValue() == PARAM_LINT)
		{
			Value_INT * xVar = static_cast<Value_INT*>(val1);
			Value_LINT * xVal = static_cast<Value_LINT*>(*val2);
			xVar->value = xVal->value;
			deletePtr(*val2);
			return true;
		}
		else
		{
				Errores::generarError(Errores::ERROR_CONVERSION_VARIABLE_INT, outData, nombre);
			return false;
		}
	}
	case PARAM_LINT:
	{
		if ((*val2)->getTypeValue() == PARAM_INT)
		{
			Value_LINT * xVar = static_cast<Value_LINT*>(val1);
			Value_INT * xVal = static_cast<Value_INT*>(*val2);
			xVar->value = xVal->value;
			deletePtr(*val2);
			return true;
		}
		else if ((*val2)->getTypeValue() == PARAM_LINT)
		{
			Value_LINT * xVar = static_cast<Value_LINT*>(val1);
			Value_LINT * xVal = static_cast<Value_LINT*>(*val2);
			xVar->value = xVal->value;
			deletePtr(*val2);
			return true;
		}
		else
		{
			Errores::generarError(Errores::ERROR_CONVERSION_VARIABLE_INT, outData, nombre);
			return false;
		}
	}
	case PARAM_DOUBLE:
	{
		if ((*val2)->getTypeValue() == PARAM_INT)
		{
			Value_DOUBLE * xVar = static_cast<Value_DOUBLE*>(val1);
			Value_INT * xVal = static_cast<Value_INT*>(*val2);
			xVar->value = xVal->value;
			deletePtr(*val2);
			return true;
		}
		else if ((*val2)->getTypeValue() == PARAM_LINT)
		{
			Value_DOUBLE * xVar = static_cast<Value_DOUBLE*>(val1);
			Value_LINT * xVal = static_cast<Value_LINT*>(*val2);
			xVar->value = xVal->value;
			deletePtr(*val2);
			return true;
		}
		else if ((*val2)->getTypeValue() == PARAM_DOUBLE)
		{
			Value_DOUBLE * xVar = static_cast<Value_DOUBLE*>(val1);
			Value_DOUBLE * xVal = static_cast<Value_DOUBLE*>(*val2);
			xVar->value = xVal->value;
			deletePtr(*val2);
			return true;
		}
		else
		{
			Errores::generarError(Errores::ERROR_CONVERSION_VARIABLE_REAL, outData, nombre);
			return false;
		}
	}
	case PARAM_STRING:
	{
		if ((*val2)->getTypeValue() == PARAM_STRING)
		{
			Value_STRING * xVar = static_cast<Value_STRING*>(val1);
			Value_STRING * xVal = static_cast<Value_STRING*>(*val2);
			xVar->value = xVal->value;
			ReplaceAll(xVar->value, "\\n", "\n");
			deletePtr(*val2);
			return true;
		}
		else
		{
			Errores::generarError(Errores::ERROR_CONVERSION_VARIABLE_STRING, outData,nombre);
			return false;
		}
	}
	case PARAM_BOOL:
	{
		if ((*val2)->getTypeValue() == PARAM_BOOL)
		{
			Value_BOOL * xVar = static_cast<Value_BOOL*>(val1);
			Value_BOOL * xVal = static_cast<Value_BOOL*>(*val2);
			xVar->value = xVal->value;
			deletePtr(*val2);
			return true;
		}
		else if ((*val2)->getTypeValue() == PARAM_INT)
		{
			Value_BOOL * xVar = static_cast<Value_BOOL*>(val1);
			Value_INT * xVal = static_cast<Value_INT*>(*val2);
			xVar->value = xVal->value;
			deletePtr(*val2);
			return true;
		}
		else if ((*val2)->getTypeValue() == PARAM_LINT)
		{
			Value_BOOL * xVar = static_cast<Value_BOOL*>(val1);
			Value_LINT * xVal = static_cast<Value_LINT*>(*val2);
			xVar->value = xVal->value;
			deletePtr(*val2);
			return true;
		}
		else if ((*val2)->getTypeValue() == PARAM_DOUBLE)
		{
			Value_BOOL * xVar = static_cast<Value_BOOL*>(val1);
			Value_DOUBLE * xVal = static_cast<Value_DOUBLE*>(*val2);
			xVar->value = xVal->value;
			deletePtr(*val2);
			return true;
		}
		else
		{
			Errores::generarError(Errores::ERROR_CONVERSION_VARIABLE_BOOL, outData, nombre);
			return false;
		}
	}
	}
	return false;
}


bool Interprete::EstablecerVariable(Variable * var, Value ** value, OutData_Parametros * outData)
{
	if (var)
	{
		if (!var->fuerte)
		{
			deletePtr(var->valor);
			var->valor = (*value)->Clone(); 
			return true;
		}

		return ValueConversion(var->valor, value, outData, var->nombre);
	}
	return false;
}


Value_BOOL *  Interprete::CondicionalDeDosValores(Value * value1, OPERADORES accion, Value * value2, OutData_Parametros * outData)
{
	switch (value1->getTypeValue())
	{
	case PARAM_INT:
	{
		Value_INT * x1 = static_cast<Value_INT*>(value1);
		switch (value2->getTypeValue())
		{
			case PARAM_INT:
			{
				Value_INT * x2 = static_cast<Value_INT*>(value2);

				switch (accion)
				{
				case OP_REL_EQUAL: if (x1->value == x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_NOT_EQUAL:  if (x1->value != x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MINOR:  if (x1->value < x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MAJOR:  if (x1->value > x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MINOR_OR_EQUAL:  if (x1->value <= x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MAJOR_OR_EQUAL:  if (x1->value >= x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				}
				break;
			}

			case PARAM_LINT:
			{
				Value_LINT * x2 = static_cast<Value_LINT*>(value2);

				switch (accion)
				{
				case OP_REL_EQUAL: if (x1->value == x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_NOT_EQUAL:  if (x1->value != x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MINOR:  if (x1->value < x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MAJOR:  if (x1->value > x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MINOR_OR_EQUAL:  if (x1->value <= x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MAJOR_OR_EQUAL:  if (x1->value >= x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				}
				break;
			}

			case PARAM_DOUBLE:
			{
				Value_DOUBLE * x2 = static_cast<Value_DOUBLE*>(value2);

				switch (accion)
				{
				case OP_REL_EQUAL: if (x1->value == x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_NOT_EQUAL:  if (x1->value != x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MINOR:  if (x1->value < x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MAJOR:  if (x1->value > x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MINOR_OR_EQUAL:  if (x1->value <= x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MAJOR_OR_EQUAL:  if (x1->value >= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				}
				break;
			}

			case PARAM_STRING:
			{
				//Value_STRING * x2 = static_cast<Value_STRING*>(value2);

				Errores::generarError(Errores::ERROR_COMPARACION_INT_STRING, outData);
				return NULL;
			}

			case PARAM_BOOL:
			{
				Value_BOOL * x2 = static_cast<Value_BOOL*>(value2);
				switch (accion)
				{
					case OP_REL_EQUAL: if (x1->value == x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
					case OP_REL_NOT_EQUAL:  if (x1->value != x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
					case OP_REL_MINOR:  if (x1->value < x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
					case OP_REL_MAJOR:  if (x1->value > x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
					case OP_REL_MINOR_OR_EQUAL:  if (x1->value <= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
					case OP_REL_MAJOR_OR_EQUAL:  if (x1->value >= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				}
				break;
			}
		}
	}

	case PARAM_LINT:
	{
		Value_LINT * x1 = static_cast<Value_LINT*>(value1);
		switch (value2->getTypeValue())
		{
		case PARAM_INT:
		{
			Value_INT * x2 = static_cast<Value_INT*>(value2);

			switch (accion)
			{
			case OP_REL_EQUAL: if (x1->value == x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
			case OP_REL_NOT_EQUAL:  if (x1->value != x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
			case OP_REL_MINOR:  if (x1->value < x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
			case OP_REL_MAJOR:  if (x1->value > x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
			case OP_REL_MINOR_OR_EQUAL:  if (x1->value <= x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
			case OP_REL_MAJOR_OR_EQUAL:  if (x1->value >= x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
			}
			break;
		}

		case PARAM_LINT:
		{
			Value_LINT * x2 = static_cast<Value_LINT*>(value2);

			switch (accion)
			{
			case OP_REL_EQUAL: if (x1->value == x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
			case OP_REL_NOT_EQUAL:  if (x1->value != x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
			case OP_REL_MINOR:  if (x1->value < x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
			case OP_REL_MAJOR:  if (x1->value > x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
			case OP_REL_MINOR_OR_EQUAL:  if (x1->value <= x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
			case OP_REL_MAJOR_OR_EQUAL:  if (x1->value >= x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
			}
			break;
		}

		case PARAM_DOUBLE:
		{
			Value_DOUBLE * x2 = static_cast<Value_DOUBLE*>(value2);

			switch (accion)
			{
			case OP_REL_EQUAL: if (x1->value == x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
			case OP_REL_NOT_EQUAL:  if (x1->value != x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
			case OP_REL_MINOR:  if (x1->value < x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
			case OP_REL_MAJOR:  if (x1->value > x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
			case OP_REL_MINOR_OR_EQUAL:  if (x1->value <= x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
			case OP_REL_MAJOR_OR_EQUAL:  if (x1->value >= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
			}
			break;
		}

		case PARAM_STRING:
		{
			//Value_STRING * x2 = static_cast<Value_STRING*>(value2);

			Errores::generarError(Errores::ERROR_COMPARACION_INT_STRING, outData);
			return NULL;
		}

		case PARAM_BOOL:
		{
			Value_BOOL * x2 = static_cast<Value_BOOL*>(value2);
			switch (accion)
			{
			case OP_REL_EQUAL: if (x1->value == x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
			case OP_REL_NOT_EQUAL:  if (x1->value != x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
			case OP_REL_MINOR:  if (x1->value < x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
			case OP_REL_MAJOR:  if (x1->value > x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
			case OP_REL_MINOR_OR_EQUAL:  if (x1->value <= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
			case OP_REL_MAJOR_OR_EQUAL:  if (x1->value >= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
			}
			break;
		}
		}
	}

	case PARAM_DOUBLE:
	{
		Value_DOUBLE * x1 = static_cast<Value_DOUBLE*>(value1);
		switch (value2->getTypeValue())
		{
			case PARAM_INT:
			{
				Value_INT * x2 = static_cast<Value_INT*>(value2);

				switch (accion)
				{
				case OP_REL_EQUAL: if (x1->value == x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_NOT_EQUAL:  if (x1->value != x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MINOR:  if (x1->value < x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MAJOR:  if (x1->value > x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MINOR_OR_EQUAL:  if (x1->value <= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MAJOR_OR_EQUAL:  if (x1->value >= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				}
				break;
			}

			case PARAM_LINT:
			{
				Value_LINT * x2 = static_cast<Value_LINT*>(value2);

				switch (accion)
				{
				case OP_REL_EQUAL: if (x1->value == x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_NOT_EQUAL:  if (x1->value != x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MINOR:  if (x1->value < x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MAJOR:  if (x1->value > x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MINOR_OR_EQUAL:  if (x1->value <= x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MAJOR_OR_EQUAL:  if (x1->value >= x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				}
				break;
			}

			case PARAM_DOUBLE:
			{
				Value_DOUBLE * x2 = static_cast<Value_DOUBLE*>(value2);

				switch (accion)
				{
				case OP_REL_EQUAL: if (x1->value == x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_NOT_EQUAL:  if (x1->value != x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MINOR:  if (x1->value < x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MAJOR:  if (x1->value > x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MINOR_OR_EQUAL:  if (x1->value <= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MAJOR_OR_EQUAL:  if (x1->value >= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				}
				break;
			}

			case PARAM_STRING:
			{
				Value_STRING * x2 = static_cast<Value_STRING*>(value2);

				Errores::generarError(Errores::ERROR_COMPARACION_INT_STRING, outData);
				return NULL;
			}

			case PARAM_BOOL:
			{
				Value_BOOL * x2 = static_cast<Value_BOOL*>(value2);
				switch (accion)
				{
				case OP_REL_EQUAL: if (x1->value == x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_NOT_EQUAL:  if (x1->value != x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MINOR:  if (x1->value < x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MAJOR:  if (x1->value > x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MINOR_OR_EQUAL:  if (x1->value <= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MAJOR_OR_EQUAL:  if (x1->value >= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				}
				break;
			}
		}
	}

	case PARAM_STRING:
	{
		Value_STRING * x1 = static_cast<Value_STRING*>(value1);
		switch (value2->getTypeValue())
		{
		case PARAM_LINT:
		case PARAM_INT:
		{
			Errores::generarError(Errores::ERROR_COMPARACION_INT_STRING, outData);
			return NULL;
		}

		case PARAM_DOUBLE:
		{
			Errores::generarError(Errores::ERROR_COMPARACION_REAL_STRING, outData);
			return NULL;
		}

		case PARAM_STRING:
		{
			Value_STRING * x2 = static_cast<Value_STRING*>(value2);

			switch (accion)
			{
			case OP_REL_EQUAL: if (x1->value == x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
			case OP_REL_NOT_EQUAL:  if (x1->value != x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
			case OP_REL_MINOR:  if (x1->value < x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
			case OP_REL_MAJOR:  if (x1->value > x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
			case OP_REL_MINOR_OR_EQUAL:  if (x1->value <= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
			case OP_REL_MAJOR_OR_EQUAL:  if (x1->value >= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
			}
		}

		case PARAM_BOOL:
		{
			Errores::generarError(Errores::ERROR_COMPARACION_BOOL_STRING, outData);
			return NULL;
		}
		break;
		}
	}

	case PARAM_BOOL:
	{
		Value_BOOL* x1 = static_cast<Value_BOOL*>(value1);
		switch (value2->getTypeValue())
		{
			case PARAM_INT:
			{
				Value_INT * x2 = static_cast<Value_INT*>(value2);

				switch (accion)
				{
				case OP_REL_EQUAL: if (x1->value == x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_NOT_EQUAL:  if (x1->value != x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MINOR:  if (x1->value < x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MAJOR:  if (x1->value > x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MINOR_OR_EQUAL:  if (x1->value <= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MAJOR_OR_EQUAL:  if (x1->value >= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				}
				break;
			}

			case PARAM_LINT:
			{
				Value_LINT * x2 = static_cast<Value_LINT*>(value2);

				switch (accion)
				{
				case OP_REL_EQUAL: if (x1->value == x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_NOT_EQUAL:  if (x1->value != x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MINOR:  if (x1->value < x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MAJOR:  if (x1->value > x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MINOR_OR_EQUAL:  if (x1->value <= x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MAJOR_OR_EQUAL:  if (x1->value >= x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				}
				break;
			}

			case PARAM_DOUBLE:
			{
				Value_DOUBLE * x2 = static_cast<Value_DOUBLE*>(value2);

				switch (accion)
				{
				case OP_REL_EQUAL: if (x1->value == x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_NOT_EQUAL:  if (x1->value != x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MINOR:  if (x1->value < x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MAJOR:  if (x1->value > x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MINOR_OR_EQUAL:  if (x1->value <= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MAJOR_OR_EQUAL:  if (x1->value >= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				}
				break;
			}

			case PARAM_STRING:
			{
				Value_STRING * x2 = static_cast<Value_STRING*>(value2);

				Errores::generarError(Errores::ERROR_COMPARACION_INT_STRING, outData);
				return NULL;
			}

			case PARAM_BOOL:
			{
				Value_BOOL * x2 = static_cast<Value_BOOL*>(value2);
				switch (accion)
				{
				case OP_REL_EQUAL: if (x1->value == x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_NOT_EQUAL:  if (x1->value != x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MINOR:  if (x1->value < x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MAJOR:  if (x1->value > x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MINOR_OR_EQUAL:  if (x1->value <= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case OP_REL_MAJOR_OR_EQUAL:  if (x1->value >= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				}
				return NULL;
			}
		}
	}
	}

	Errores::generarError(Errores::ERROR_COMPARACION_DESCONOCIDO, outData);
	return NULL;
}

Value * Interprete::OperacionSobreValores(Value * value1, OPERADORES accion, Value * value2, OutData_Parametros * outData)
{
	if (!value1 || !value2)
	{
		return NULL;
	}

	if (value1->getTypeValue() == PARAM_VOID || value2->getTypeValue() == PARAM_VOID)
	{
		Errores::generarError(Errores::ERROR_OPERACION_INVALIDA_VOID, outData);
		return NULL;
	}

	switch (value1->getTypeValue())
	{
		case PARAM_INT:
		{
			Value_INT * x1 = static_cast<Value_INT*>(value1);

			switch (value2->getTypeValue())
			{
				case PARAM_INT:
				{
					Value_INT * x2 = static_cast<Value_INT*>(value2);

					switch (accion)
					{
					case OP_ARIT_SUMA: return new Value_INT(x1->value + x2->value);
					case OP_ARIT_RESTA: return new Value_INT(x1->value - x2->value);
					case OP_ARIT_MULT: return new Value_INT(x1->value * x2->value);
					case OP_ARIT_DIV_ENTERA: return new Value_INT(((int)x1->value) / x2->value);
					case OP_ARIT_DIV:  return new Value_DOUBLE(x1->value / ((double)x2->value));
					case OP_ARIT_MOD: return new Value_INT(x1->value % x2->value);
					}
					return NULL;
				}

				case PARAM_LINT:
				{
					Value_LINT * x2 = static_cast<Value_LINT*>(value2);

					switch (accion)
					{
					case OP_ARIT_SUMA: return new Value_LINT(x1->value + x2->value);
					case OP_ARIT_RESTA: return new Value_LINT(x1->value - x2->value);
					case OP_ARIT_MULT: return new Value_LINT(x1->value * x2->value);
					case OP_ARIT_DIV_ENTERA: return new Value_LINT(((int)x1->value) / x2->value);
					case OP_ARIT_DIV:  return new Value_DOUBLE(x1->value / ((double)x2->value));
					case OP_ARIT_MOD: return new Value_LINT(x1->value % x2->value);
					}
					return NULL;
				}

				case PARAM_DOUBLE:
				{
					Value_DOUBLE * x2 = static_cast<Value_DOUBLE*>(value2);

					switch (accion)
					{
					case OP_ARIT_SUMA: return new Value_DOUBLE(x1->value + x2->value);
					case OP_ARIT_RESTA: return new Value_DOUBLE(x1->value - x2->value);
					case OP_ARIT_MULT: return new Value_DOUBLE(x1->value * x2->value);
					case OP_ARIT_DIV_ENTERA: return new Value_INT(((int)x1->value) / x2->value);
					case OP_ARIT_DIV:  return new Value_DOUBLE(x1->value / x2->value);
					case OP_ARIT_MOD: Errores::generarError(Errores::ERROR_MOD_SOLO_ENTERO, outData); return NULL;
					}

					return NULL;
				}

				case PARAM_STRING:
				{
					Value_STRING * x2 = static_cast<Value_STRING*>(value2);

					switch (accion)
					{
					case OP_ARIT_SUMA: return new Value_STRING(std::to_string(x1->value) + x2->value);
					case OP_ARIT_RESTA: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "-"); return NULL;
					case OP_ARIT_MULT: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "*"); return NULL;
					case OP_ARIT_DIV_ENTERA: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "div"); return NULL;
					case OP_ARIT_DIV: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "/"); return NULL;
					case OP_ARIT_MOD: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "%"); return NULL;
					}

					return NULL;
				}
				case PARAM_BOOL:
				{
					Value_BOOL * x2 = static_cast<Value_BOOL*>(value2);

					switch (accion)
					{
					case OP_ARIT_SUMA: return new Value_INT(x1->value + x2->value);
					case OP_ARIT_RESTA: return new Value_INT(x1->value - x2->value);
					case OP_ARIT_MULT: return new Value_INT(x1->value * x2->value);
					case OP_ARIT_DIV_ENTERA: return new Value_INT(((int)x1->value) / x2->value);
					case OP_ARIT_DIV:  return new Value_DOUBLE(x1->value / ((double)x2->value));
					case OP_ARIT_MOD: return new Value_INT(x1->value % x2->value);
					}
					return NULL;
				}
			}
			break;
		}

		case PARAM_LINT:
		{
			Value_LINT * x1 = static_cast<Value_LINT*>(value1);

			switch (value2->getTypeValue())
			{
			case PARAM_INT:
			{
				Value_INT * x2 = static_cast<Value_INT*>(value2);

				switch (accion)
				{
				case OP_ARIT_SUMA: return new Value_LINT(x1->value + x2->value);
				case OP_ARIT_RESTA: return new Value_LINT(x1->value - x2->value);
				case OP_ARIT_MULT: return new Value_LINT(x1->value * x2->value);
				case OP_ARIT_DIV_ENTERA: return new Value_LINT(((int)x1->value) / x2->value);
				case OP_ARIT_DIV:  return new Value_DOUBLE(x1->value / ((double)x2->value));
				case OP_ARIT_MOD: return new Value_LINT(x1->value % x2->value);
				}
				return NULL;
			}

			case PARAM_LINT:
			{
				Value_LINT * x2 = static_cast<Value_LINT*>(value2);

				switch (accion)
				{
				case OP_ARIT_SUMA: return new Value_LINT(x1->value + x2->value);
				case OP_ARIT_RESTA: return new Value_LINT(x1->value - x2->value);
				case OP_ARIT_MULT: return new Value_LINT(x1->value * x2->value);
				case OP_ARIT_DIV_ENTERA: return new Value_LINT(((int)x1->value) / x2->value);
				case OP_ARIT_DIV:  return new Value_DOUBLE(x1->value / ((double)x2->value));
				case OP_ARIT_MOD: return new Value_LINT(x1->value % x2->value);
				}
				return NULL;
			}

			case PARAM_DOUBLE:
			{
				Value_DOUBLE * x2 = static_cast<Value_DOUBLE*>(value2);

				switch (accion)
				{
				case OP_ARIT_SUMA: return new Value_DOUBLE(x1->value + x2->value);
				case OP_ARIT_RESTA: return new Value_DOUBLE(x1->value - x2->value);
				case OP_ARIT_MULT: return new Value_DOUBLE(x1->value * x2->value);
				case OP_ARIT_DIV_ENTERA: return new Value_LINT(((int)x1->value) / x2->value);
				case OP_ARIT_DIV:  return new Value_DOUBLE(x1->value / x2->value);
				case OP_ARIT_MOD: Errores::generarError(Errores::ERROR_MOD_SOLO_ENTERO, outData); return NULL;
				}

				return NULL;
			}

			case PARAM_STRING:
			{
				Value_STRING * x2 = static_cast<Value_STRING*>(value2);

				switch (accion)
				{
				case OP_ARIT_SUMA: return new Value_STRING(std::to_string(x1->value) + x2->value);
				case OP_ARIT_RESTA: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "-"); return NULL;
				case OP_ARIT_MULT: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "*"); return NULL;
				case OP_ARIT_DIV_ENTERA: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "div"); return NULL;
				case OP_ARIT_DIV: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "/"); return NULL;
				case OP_ARIT_MOD: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "%"); return NULL;
				}

				return NULL;
			}
			case PARAM_BOOL:
			{
				Value_BOOL * x2 = static_cast<Value_BOOL*>(value2);

				switch (accion)
				{
				case OP_ARIT_SUMA: return new Value_LINT(x1->value + x2->value);
				case OP_ARIT_RESTA: return new Value_LINT(x1->value - x2->value);
				case OP_ARIT_MULT: return new Value_LINT(x1->value * x2->value);
				case OP_ARIT_DIV_ENTERA: return new Value_LINT(((int)x1->value) / x2->value);
				case OP_ARIT_DIV:  return new Value_DOUBLE(x1->value / ((double)x2->value));
				case OP_ARIT_MOD: return new Value_LINT(x1->value % x2->value);
				}
				return NULL;
			}
			}
			break;
		}

		case PARAM_DOUBLE:
		{
			Value_DOUBLE * x1 = static_cast<Value_DOUBLE*>(value1);

			switch (value2->getTypeValue())
			{
			case PARAM_INT:
			{
				Value_INT * x2 = static_cast<Value_INT*>(value2);

				switch (accion)
				{
				case OP_ARIT_SUMA: return new Value_DOUBLE(x1->value + x2->value);
				case OP_ARIT_RESTA: return new Value_DOUBLE(x1->value - x2->value);
				case OP_ARIT_MULT: return new Value_DOUBLE(x1->value * x2->value);
				case OP_ARIT_DIV_ENTERA: return new Value_INT((int)(x1->value / x2->value));
				case OP_ARIT_DIV:  return new Value_DOUBLE(x1->value / x2->value);

				case OP_ARIT_MOD:  Errores::generarError(Errores::ERROR_MOD_SOLO_ENTERO, outData);  return NULL;
				}
				return NULL;
			}

			case PARAM_LINT:
			{
				Value_LINT * x2 = static_cast<Value_LINT*>(value2);

				switch (accion)
				{
				case OP_ARIT_SUMA: return new Value_DOUBLE(x1->value + x2->value);
				case OP_ARIT_RESTA: return new Value_DOUBLE(x1->value - x2->value);
				case OP_ARIT_MULT: return new Value_DOUBLE(x1->value * x2->value);
				case OP_ARIT_DIV_ENTERA: return new Value_LINT(((int)x1->value) / x2->value);
				case OP_ARIT_DIV:  return new Value_DOUBLE(x1->value / ((double)x2->value));
				case OP_ARIT_MOD:  Errores::generarError(Errores::ERROR_MOD_SOLO_ENTERO, outData);  return NULL;
				}
				return NULL;
			}

			case PARAM_DOUBLE:
			{
				Value_DOUBLE * x2 = static_cast<Value_DOUBLE*>(value2);

				switch (accion)
				{
				case OP_ARIT_SUMA: return new Value_DOUBLE(x1->value + x2->value);
				case OP_ARIT_RESTA: return new Value_DOUBLE(x1->value - x2->value);
				case OP_ARIT_MULT: return new Value_DOUBLE(x1->value * x2->value);
				case OP_ARIT_DIV_ENTERA: return new Value_INT((int)(x1->value / x2->value));
				case OP_ARIT_DIV:  return new Value_DOUBLE(x1->value / x2->value);
				case OP_ARIT_MOD: Errores::generarError(Errores::ERROR_MOD_SOLO_ENTERO, outData); return NULL;
				}

				return NULL;
			}

			case PARAM_STRING:
			{
				Value_STRING * x2 = static_cast<Value_STRING*>(value2);

				switch (accion)
				{
				case OP_ARIT_SUMA: return new Value_STRING(std::to_string(x1->value) + x2->value);
				case OP_ARIT_RESTA: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "-"); return NULL;
				case OP_ARIT_MULT: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "*"); return NULL;
				case OP_ARIT_DIV_ENTERA: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "div"); return NULL;
				case OP_ARIT_DIV: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "/"); return NULL;
				case OP_ARIT_MOD: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "%"); return NULL;
				}

				return NULL;
			}
			case PARAM_BOOL:
			{
				Value_BOOL * x2 = static_cast<Value_BOOL*>(value2);

				switch (accion)
				{
				case OP_ARIT_SUMA: return new Value_DOUBLE(x1->value + x2->value);
				case OP_ARIT_RESTA: return new Value_DOUBLE(x1->value - x2->value);
				case OP_ARIT_MULT: return new Value_DOUBLE(x1->value * x2->value);
				case OP_ARIT_DIV_ENTERA: return new Value_INT((int)(x1->value / x2->value));
				case OP_ARIT_DIV:  return new Value_DOUBLE(x1->value / x2->value);
				case OP_ARIT_MOD: Errores::generarError(Errores::ERROR_MOD_SOLO_ENTERO, outData); return NULL;
				}
				return NULL;
			}
			}
			break;
		}

		case PARAM_STRING:
		{
			Value_STRING * x1 = static_cast<Value_STRING*>(value1);

			switch (value2->getTypeValue())
			{
				case PARAM_INT:
				{
					Value_INT * x2 = static_cast<Value_INT*>(value2);

					switch (accion)
					{
					case OP_ARIT_SUMA: return new Value_STRING(x1->value + std::to_string(x2->value));
					case OP_ARIT_RESTA: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "-"); return NULL;
					case OP_ARIT_MULT: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "*"); return NULL;
					case OP_ARIT_DIV_ENTERA: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "div"); return NULL;
					case OP_ARIT_DIV: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "/"); return NULL;
					case OP_ARIT_MOD: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "%"); return NULL;
					}
					return NULL;
				}

				case PARAM_LINT:
				{
					Value_LINT * x2 = static_cast<Value_LINT*>(value2);

					switch (accion)
					{
					case OP_ARIT_SUMA: return new Value_STRING(x1->value + std::to_string(x2->value));
					case OP_ARIT_RESTA: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "-"); return NULL;
					case OP_ARIT_MULT: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "*"); return NULL;
					case OP_ARIT_DIV_ENTERA: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "div"); return NULL;
					case OP_ARIT_DIV: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "/"); return NULL;
					case OP_ARIT_MOD: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "%"); return NULL;
					}
					return NULL;
				}

				case PARAM_DOUBLE:
				{
					Value_DOUBLE * x2 = static_cast<Value_DOUBLE*>(value2);

					switch (accion)
					{
					case OP_ARIT_SUMA: return new Value_STRING(x1->value + std::to_string(x2->value));
					case OP_ARIT_RESTA: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "-"); return NULL;
					case OP_ARIT_MULT: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "*"); return NULL;
					case OP_ARIT_DIV_ENTERA: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "div"); return NULL;
					case OP_ARIT_DIV: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "/"); return NULL;
					case OP_ARIT_MOD: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "%"); return NULL;
					}

					return NULL;
				}

				case PARAM_STRING:
				{
					Value_STRING * x2 = static_cast<Value_STRING*>(value2);

					switch (accion)
					{
					case OP_ARIT_SUMA: return new Value_STRING(x1->value + x2->value);
					case OP_ARIT_RESTA: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "-"); return NULL;
					case OP_ARIT_MULT: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "*"); return NULL;
					case OP_ARIT_DIV_ENTERA: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "div"); return NULL;
					case OP_ARIT_DIV: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "/"); return NULL;
					case OP_ARIT_MOD: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "%"); return NULL;
					}

					return NULL;
				}
				case PARAM_BOOL:
				{
					Value_BOOL * x2 = static_cast<Value_BOOL*>(value2);

					switch (accion)
					{
					case OP_ARIT_SUMA: return new Value_STRING(x1->value + std::to_string(x2->value));
					case OP_ARIT_RESTA: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "-"); return NULL;
					case OP_ARIT_MULT: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "*"); return NULL;
					case OP_ARIT_DIV_ENTERA: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "div"); return NULL;
					case OP_ARIT_DIV: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "/"); return NULL;
					case OP_ARIT_MOD: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "%"); return NULL;
					}
					return NULL;
				}
			}
			break;
		}

		case PARAM_BOOL:
		{
			Value_BOOL * x1 = static_cast<Value_BOOL*>(value1);

			switch (value2->getTypeValue())
			{
			case PARAM_INT:
			{
				Value_INT * x2 = static_cast<Value_INT*>(value2);

				switch (accion)
				{
				case OP_ARIT_SUMA: return new Value_INT(x1->value + x2->value);
				case OP_ARIT_RESTA: return new Value_INT(x1->value - x2->value);
				case OP_ARIT_MULT: return new Value_INT(x1->value * x2->value);
				case OP_ARIT_DIV_ENTERA: return new Value_INT(((int)x1->value) / x2->value);
				case OP_ARIT_DIV:  return new Value_DOUBLE(x1->value / ((double)x2->value));
				case OP_ARIT_MOD: return new Value_INT(x1->value % x2->value);
				}
				return NULL;
			}

			case PARAM_LINT:
			{
				Value_LINT * x2 = static_cast<Value_LINT*>(value2);

				switch (accion)
				{
				case OP_ARIT_SUMA: return new Value_LINT(x1->value + x2->value);
				case OP_ARIT_RESTA: return new Value_LINT(x1->value - x2->value);
				case OP_ARIT_MULT: return new Value_LINT(x1->value * x2->value);
				case OP_ARIT_DIV_ENTERA: return new Value_LINT(((int)x1->value) / x2->value);
				case OP_ARIT_DIV:  return new Value_DOUBLE(x1->value / ((double)x2->value));
				case OP_ARIT_MOD: return new Value_LINT(x1->value % x2->value);
				}
				return NULL;
			}

			case PARAM_DOUBLE:
			{
				Value_DOUBLE * x2 = static_cast<Value_DOUBLE*>(value2);

				switch (accion)
				{
				case OP_ARIT_SUMA: return new Value_DOUBLE(x1->value + x2->value);
				case OP_ARIT_RESTA: return new Value_DOUBLE(x1->value - x2->value);
				case OP_ARIT_MULT: return new Value_DOUBLE(x1->value * x2->value);
				case OP_ARIT_DIV_ENTERA: return new Value_INT(((int)x1->value) / x2->value);
				case OP_ARIT_DIV:  return new Value_DOUBLE(x1->value / x2->value);
				case OP_ARIT_MOD: Errores::generarError(Errores::ERROR_MOD_SOLO_ENTERO, outData); return NULL;
				}

				return NULL;
			}

			case PARAM_STRING:
			{
				Value_STRING * x2 = static_cast<Value_STRING*>(value2);

				switch (accion)
				{
				case OP_ARIT_SUMA: return new Value_STRING(std::to_string(x1->value) + x2->value);
				case OP_ARIT_RESTA: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "-"); return NULL;
				case OP_ARIT_MULT: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "*"); return NULL;
				case OP_ARIT_DIV_ENTERA: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "div"); return NULL;
				case OP_ARIT_DIV: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "/"); return NULL;
				case OP_ARIT_MOD: Errores::generarError(Errores::ERROR_MATH_STRING, outData, "%"); return NULL;
				}

				return NULL;
			}
			case PARAM_BOOL:
			{
				Value_BOOL * x2 = static_cast<Value_BOOL*>(value2);

				switch (accion)
				{
				case OP_ARIT_SUMA: return new Value_INT(x1->value + x2->value);
				case OP_ARIT_RESTA: return new Value_INT(x1->value - x2->value);
				case OP_ARIT_MULT: return new Value_INT(x1->value * x2->value);
				case OP_ARIT_DIV_ENTERA: return new Value_INT(((int)x1->value) / x2->value);
				case OP_ARIT_DIV:  return new Value_DOUBLE(x1->value / ((double)x2->value));
				case OP_ARIT_MOD: return new Value_INT(x1->value % x2->value);
				}
				return NULL;
			}
			}
		}

		Errores::generarError(Errores::ERROR_OPERACION_INVALIDA_NULL, outData);
		return NULL;
	}
}


bool Interprete::ConversionXtoBool(Value * valOp)
{
	switch (valOp->getTypeValue())
	{
	case PARAM_BOOL:
	{
		Value_BOOL * xIn = static_cast<Value_BOOL*>(valOp);
		return xIn->value;
		break;
	}

	case PARAM_INT:
	{
		Value_INT * xIn = static_cast<Value_INT*>(valOp);


		if (xIn->value)
			return true;
		else
			return false;
	}

	case PARAM_LINT:
	{
		Value_LINT * xIn = static_cast<Value_LINT*>(valOp);


		if (xIn->value)
			return true;
		else
			return false;
	}

	case PARAM_DOUBLE:
	{
		Value_DOUBLE * xIn = static_cast<Value_DOUBLE*>(valOp);

		if (xIn->value)
			return true;
		else
			return false;
	}

	case PARAM_STRING:
	{
		Value_STRING * xIn = static_cast<Value_STRING*>(valOp);

		if (xIn->value != "")
			return true;
		else
			return false;
	}

	default:
	{
		//Errores::generarError(Errores::ERROR_EXPRESION_NO_CONVERTIBLE_BOOL, outData);
		return false;
	}
	}

	return false;
}

Value * Interprete::Operaciones(Parser_Operacion * pOp, std::vector<Variable*> * variables)
{
	std::vector<OperacionComp*>* componenteInterno = new std::vector<OperacionComp*>();
	//	componenteInterno->reserve(10);
	Value * val = Operaciones(pOp, variables, componenteInterno);

	for (std::vector<OperacionComp*>::iterator it = componenteInterno->begin(); it != componenteInterno->end(); ++it)
	{
		delete (*it);
	}
	delete componenteInterno;

	if (val == NULL)
	{
		return NULL;
	}
	
	return val;
}

Value * Interprete::Operaciones (Parser_Operacion * pOp, std::vector<Variable*> * variables, std::vector<OperacionComp*>* componente)
{
	if (pOp->tipo == OP_REC_OP)
	{
		Operacion_Recursiva * x = static_cast<Operacion_Recursiva*>(pOp);

		std::vector<OperacionComp*>* componenteInterno = new std::vector<OperacionComp*>();
		Value * val = Operaciones(x->op1, variables, componenteInterno);

		if (x->negado)
		{
			bool b = ConversionXtoBool(val);
			delete val;
			val = new Value_BOOL(!b);
		}

		for (std::vector<OperacionComp*>::iterator it = componenteInterno->begin(); it != componenteInterno->end(); ++it)
		{
			delete (*it);
		}
		delete componenteInterno;


		if (val == NULL)
			return NULL;


		if (x->op2)
		{
			componente->push_back(new OperacionComp(val, x->op2->accion));
			Value * _vl = Operaciones(x->op2->op, variables, componente);
			if (_vl == NULL)
				return NULL;
			else
				delete _vl;
		}
		else
		{
			componente->push_back(new OperacionComp(val, OP_NONE));	
		}
	}
	else if (pOp->tipo == OP_MATH)
	{
		Operacion_Operador * x = static_cast<Operacion_Operador*>(pOp);
		
		Value * val = NULL;
		switch (x->op1->tipo)
		{
			case VAL_LIT:
			{
				Parser_Literal * xLit = static_cast<Parser_Literal*>(x->op1);

				if (xLit->negado)
				{
					bool resBol = ConversionXtoBool(xLit->value);
					val = new Value_BOOL(!resBol);
				}
				else
					val = xLit->value->Clone();
				break;
			}

			case VAL_ID:
			{
				Parser_Identificador * xID = static_cast<Parser_Identificador*>(x->op1);
				Variable * var = BusquedaVariable(xID->nombre, variables);
				if (var)
				{
					if (xID->negado)
					{

						bool resBol = ConversionXtoBool(var->valor);
						val = new Value_BOOL(!resBol);
					}
					else
					{
						if (var->valor == NULL)
						{
							Errores::generarError(Errores::ERROR_OPERACION_INVALIDA_VOID, &pOp->parametros);
							val = NULL;
						}
						else val = var->valor->Clone();
					}
				}
				else
				{
					Errores::generarError(Errores::ERROR_VARIABLE_NO_EXISTE, &xID->parametros , xID->nombre);
					return NULL;
				}
				break;
			}
			case VAL_FUNC:
			{
				Valor_Funcion * xFunc = static_cast<Valor_Funcion*>(x->op1);


				//Aquí habría que poner una opción para cuando se trate de modo: prolog.
				Value * _resFunc = ExecFuncion(xFunc->ID->nombre, xFunc, variables);

				if (_resFunc == NULL)
				{
					return NULL;
				}

				val = _resFunc;
			}
			//FUNCION FALTARIA...
		}

		if (val)
		{
			if (x->op2)
			{
				componente->push_back(new OperacionComp(val, x->op2->accion));
				return Operaciones(x->op2->op, variables, componente);
			}
			else
			{
				componente->push_back(new OperacionComp(val, OP_NONE));
				//deletePtr(val);
			}
		}
		else  return NULL;
	}
	else {
		return NULL;
	}

	if (componente->size() == 1)
	{
		Value * res = componente->at(0)->val->Clone();
		return res;
	}

	// Preparamos devolver los componentes.
	//Teniendo en cuenta las siguientes premisas:
	// Por orden de operaciones, de más relevante a menos.
	//  * / div %    mayor que    + -    mayor que   == >= < > <=   mayor que   &&  ||

	// ################## PRIORIDAD 1 -> Multiplicaciones, Divisiones, Divisiones enteras, Modulos  ##################
	std::vector<OPERADORES> operadores = { OP_ARIT_DIV, OP_ARIT_DIV_ENTERA, OP_ARIT_MOD, OP_ARIT_MULT };
	
	if (!this->GestionarOperacionesPorPrioridad(pOp, componente, &operadores, OPERADOR_ARITMETICO))
		return NULL;

	//En el caso de que ya no se puedan realizar más operaciones, devolvemos resultado.
	if (componente->size() == 1)
	{
		Value * res = componente->at(0)->val->Clone();
		return res;
	}

	// ################## PRIORIDAD 2 -> Sumas, Restas ##################
	operadores = { OP_ARIT_SUMA, OP_ARIT_RESTA };

	if (!this->GestionarOperacionesPorPrioridad(pOp, componente, &operadores, OPERADOR_ARITMETICO))
		return NULL;

	//En el caso de que ya no se puedan realizar más operaciones, devolvemos resultado.
	if (componente->size() == 1)
	{
		Value * res = componente->at(0)->val->Clone();
		return res;
	}

	// ################## PRIORIDAD 3 -> OPERACIONES RELACIONALES  ##################
	operadores = { OP_REL_EQUAL, OP_REL_MAJOR, OP_REL_MAJOR_OR_EQUAL , OP_REL_MINOR, OP_REL_MINOR_OR_EQUAL, OP_REL_NOT_EQUAL };

	if (!this->GestionarOperacionesPorPrioridad(pOp, componente, &operadores, OPERADOR_RELACIONAL))
		return NULL;

	//En el caso de que ya no se puedan realizar más operaciones, devolvemos resultado.
	if (componente->size() == 1)
	{
		Value * res = componente->at(0)->val->Clone();
		return res;
	}

	// ################## PRIORIDAD 4 -> OPERACIONES LOGICAS  ##################
	operadores = { OP_LOG_ADD, OP_LOG_OR };

	if (!this->GestionarOperacionesPorPrioridad(pOp, componente, &operadores, OPERADOR_LOGICO))
		return NULL;

	//En el caso de que ya no se puedan realizar más operaciones, devolvemos resultado.
	if (componente->size() == 1)
	{
		Value * res = componente->at(0)->val->Clone();
		return res;
	}

	return NULL;
}



Value_BOOL *  Interprete::CondicionalLogico(Value * value1, OPERADORES accion, Value * value2)
{
	switch (accion)
	{
		case OP_LOG_ADD:
		{
			bool b1 = ConversionXtoBool(value1);
			bool b2 = ConversionXtoBool(value2);
			return new Value_BOOL(b1 && b2);
		}

		case OP_LOG_OR:
		{
			bool b1 = ConversionXtoBool(value1);
			bool b2 = ConversionXtoBool(value2);
			return new Value_BOOL(b1 || b2);
		}
	}
	return new Value_BOOL(false);
}


bool Interprete::GestionarOperacionesPorPrioridad(Parser_Operacion * pOp, std::vector<OperacionComp*>* componente, std::vector<OPERADORES> * operators, OPERADORES_TIPOS tipo)
{
	OperacionComp * temp = NULL;

	bool res_f = true; // Solo usado en relacionales

	for (unsigned itr = 0; itr < componente->size(); itr++)
	{
		if (temp)
		{
			Value * v = NULL;
			if (tipo == OPERADOR_ARITMETICO)
				v = this->OperacionSobreValores(temp->val, temp->oper, componente->at(itr)->val, &pOp->parametros);
			else if (tipo == OPERADOR_RELACIONAL)
				v = this->CondicionalDeDosValores(temp->val, temp->oper, componente->at(itr)->val, &pOp->parametros);
			else 
				v = this->CondicionalLogico(temp->val, temp->oper, componente->at(itr)->val);

			if (v)
			{
				if (tipo == OPERADOR_RELACIONAL)
				{
					if (!isRelationalOperator(componente->at(itr)->oper))
					{
						delete componente->at(itr)->val;

						Value_BOOL * b_r = static_cast<Value_BOOL*>(v);
						if (!res_f)
						{
							b_r->value = false;
						}

						componente->at(itr)->val = b_r;
						res_f = true;
					}				
					else
					{
						if (res_f)
						{
							Value_BOOL * b_r = static_cast<Value_BOOL*>(v);
							res_f = b_r->value;
						}
						delete v;
					}
				}
				else
				{
					delete componente->at(itr)->val;
					componente->at(itr)->val = v;
				}
			}
			else return false;

			temp = NULL;
		}

		for (std::vector<OPERADORES>::iterator it = operators->begin(); it != operators->end(); it++)
		{
			if (componente->at(itr)->oper == *it)
			{
				componente->at(itr)->delete_ready = true;
				temp = componente->at(itr);
				break;
			}
		}
	}

	//Borramos datos marcados
	for (std::vector<OperacionComp*>::iterator it = componente->begin(); it != componente->end(); )
	{
		if ((*it)->delete_ready)
		{
			delete(*it);
			it = componente->erase(it);
		}
		else it++;
	}
	return true;
}





Value* Interprete::Transformar_Declarativo_Value(Parser_Declarativo * dec)
{
	switch (dec->tipo)
	{
	case DEC_SINGLE:
	{
		Declarativo_SingleValue * x2 = static_cast<Declarativo_SingleValue*>(dec->value);
		switch (x2->value)
		{
			case PARAM_VOID:
			{
				return new Value();
			}
			case PARAM_INT:
			{
				return new Value_INT();
			}
			case PARAM_LINT:
			{
				return new Value_LINT();
			}
			case PARAM_STRING:
			{
				return new Value_STRING();
			}
			case PARAM_DOUBLE:
			{
				return new Value_DOUBLE();
			}
			case PARAM_BOOL:
			{
				return new Value_BOOL(true);
			}
		}
		return NULL;
	}
	case DEC_MULTI:
	{
		Declarativo_MultiValue * x2 = static_cast<Declarativo_MultiValue*>(dec->value);
		switch (x2->tipo)
		{
			case PARAM_TUPLA:
			{
				std::vector<Value*> value;

				for (std::vector<Parser_Declarativo*>::iterator it = x2->value.begin(); it != x2->value.end(); ++it)
				{
					Value * v = Transformar_Declarativo_Value(*it);

					if (v)
						value.push_back(v);
				}

				return new Value_TUPLA(value);
			}
		}
	}
	return NULL;
	}
}