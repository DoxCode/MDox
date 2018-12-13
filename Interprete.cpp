
#include "Interprete.h"


bool Interprete::CargarDatos(Parser* parser)
{
	//Borramos la lista de funciones existentes actualmente.
	for (std::vector<Parser_Funcion*>::iterator it = funciones.begin(); it != funciones.end(); ++it)
	{
		delete (*it);
	}

/*	std::cout << "TOKENS: \n";

	for (unsigned i = 0; i < parser->tokenizer.tokens.size(); i++)
	{
		std::cout << "  [" << parser->tokenizer.tokens.at(i)->token << "]  ";
	}

	std::cout << "\n\n";*/
	
	//Interpretando datos...size_t
	int local = 0;
	while (local < parser->tokenizer.tokens.size())
	{
		//	std::cout << "--:: " << parser.tokenizer.tokens.at(local) << " :: " << local << " \n";

		Parser_Funcion* p2 = parser->getFuncion(local);

		if (p2)
		{
			p2->source = parser->tokenizer.fichero;
			funciones.push_back(p2);
			continue;
		}
		else
		{
			std::cout << " --Error de sintaxis en la linea [" << parser->tokenizer.token_actual->linea << "::" << parser->tokenizer.token_actual->char_horizontal << "]. <"+parser->tokenizer.fichero +"> '" <<
				parser->tokenizer.token_actual->token << "' no es una operación reconocida o no fue cerrada correctamente. \n";
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
				std::cout << (" \n");
				delete p2;
				return false;
			}
		}
		else
		{
			std::cout << " --Error de sintaxis en la linea [" << parser->tokenizer.token_actual->linea <<"::" << parser->tokenizer.token_actual->char_horizontal << "]. <Interprete> '" <<
				parser->tokenizer.token_actual->token << "' no es una operación reconocida. \n";
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
					std::cout << " Linea: [" << x->linea << "::" << x->offset << "] ";
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

		//Crea una VARIABLE
		case SENT_VAR_INI:
		{
			Sentencia_Parametro * x = static_cast<Sentencia_Parametro*>(sentencia);
			if (Interprete_NuevaVariable(x->pPar, variables, false) == NULL) // Crea nueva variable, si ya existe devuelve NULL
			{
				std::cout << " Linea: [" << x->linea << "::" << x->offset << "] ";
				return false;
			}

				return true;
		}

		//Operaciones, si son llamadas como funciones solo podrán ser sumatorios, etc. var++, ++var etc.
		// Add+1 -> También se podrá llamar a operaciones matemáticas, pero su resultado nunca se guardará. 
		//  Esto se debe a que se puede ejecutar directamente funciones aún sin ser return void.
		case SENT_OP:
		{
			Sentencia_Operacional * x = static_cast<Sentencia_Operacional*>(sentencia);

			if (!EstablecerOperacion(x->pOp, variables))
			{
				std::cout << " Linea: [" << x->linea << "::" << x->offset << "] ";
				return false;
			}

			return true;
		}
		case SENT_IGU:
		{
			Sentencia_Igualdad * x = static_cast<Sentencia_Igualdad*>(sentencia);

			if (!EstablecerIgualdad(x->pIg, variables))
			{
				std::cout << " Linea: [" << x->linea << "::" << x->offset << "] ";
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
				std::cout << "El valor introducido No es una operación.";

			if (v)
			{
				switch (v->getTypeValue())
				{
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
			
			Value_BOOL * b = Condicionales(x->pCond, variables);

			if (b == NULL)
				return false;

			if (b->value)
			{
				if (x->pS)
				{
					delete b;
					if (!Interprete_Sentencia(x->pS, variables))
					{
						std::cout << " Linea: [" << x->linea << "::" << x->offset << "] ";
						return false;
					}
					return true;
				}
			}
			else
			{
				delete b;
				if (x->pElse)
				{
					if (!Interprete_Sentencia(x->pElse, variables))
					{
						std::cout << " Linea: [" << x->linea << "::" << x->offset << "] ";
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
				Value_BOOL * b = Condicionales(x->pCond, variables);

				if (b == NULL)
					return false;

				if (b->value)
				{
					delete b;

					if (x->pS)
					{
						if (!Interprete_Sentencia(x->pS, variables))
						{
							std::cout << " Linea: [" << x->linea << "::" << x->offset << "] ";
							return false;
						}
					}
				}
				else
				{
					delete b;
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
					std::cout << " Linea: [" << x->linea << "::" << x->offset << "] ";
					return false;
				}

			}

			while (true)
			{
				bool res = true;

				if (x->pCond)
				{
					Value_BOOL * b = Condicionales(x->pCond, variables);

					if (b == NULL)
					{
						std::cout << " Linea: [" << x->linea << "::" << x->offset << "] ";
						return false;
					}

					res = b->value;
					delete b;
				}

				if (res)
				{
					if (!Interprete_Sentencia(x->pS, variables))
					{
						std::cout << " Linea: [" << x->linea << "::" << x->offset << "] ";
						return false;
					}			

					if (x->pOp)
					{
						if (!EstablecerOperacion(x->pOp, variables))
						{
							std::cout << " Linea: [" << x->linea << "::" << x->offset << "] ";
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

					if (!EstablecerVariable(v1, &xR))
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

					if (!EstablecerVariable(v1, &xR))
					{
						delete xR;
						return false;
					}
					return true;
				}
				else return false;
				

			}
			else return false;
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

					if (!EstablecerVariable(v1, &xR))
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

					if (!EstablecerVariable(v1, &xR))
					{
						delete xR;
						return false;
					}
					return true;
				}
				else return false;


			}
			else return false;
			
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
			if (pIg->cond->tipo == COND_OP)
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

					if (!EstablecerVariable(var, &v))
					{
						delete v;
						return false;
					}
					delete v;
					return true;
				}
			}

			Value * v_bol = Condicionales(pIg->cond, variables);

			if (v_bol == NULL)
			{
				return false;
			}

			if (!EstablecerVariable(var, &v_bol))
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
					Variable * var = new Variable(x->pID->nombre, v, true);
					var->deep = deep;
					variables->push_back(var);
					return var;
				}
				std::cout << "Error al intentar inicializar la variable '" << x->pID->nombre << "'. ";
				return false;
			}
			else
			{
				if (existe)
				{
					return var1;
				}
				//Existe
				std::cout << "Error de ejecución: La variable con nombre '" << x->pID->nombre << "' fue declarada más de una vez. ";
				return NULL;
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
			std::cout << "Error al declarar parámetro.";
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
			break;

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

	for (std::vector<Parser_Funcion*>::iterator funcion = funciones.begin(); funcion != funciones.end(); ++funcion)
	{
		if ((*funcion)->pID->nombre == ID)
		{
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
					Value_BOOL * _Cond = CondicionalDeDosValores(valor_operacion, COND_ACC_EQUAL, xCallEnt->entradas->at(ent_itr));

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

					// SE producirá UN WARNING: Si dicha variable ya fue declarada.
					if (xParametro->tipo == PRM_DECLARATIVO_ID)
					{
						Parametro_Declarativo_ID* _xParID = static_cast<Parametro_Declarativo_ID*>(xParametro);
						if (BusquedaVariable(_xParID->pID->nombre, &variablesEntorno) != NULL)
						{
							std::cout << "La entrada " << _xParID->pID->nombre << " de la función " << ID << " ya fue declarada.\n ";
							correcto = false;
							break;
						}

						Value* v = Transformar_Declarativo_Value(_xParID->pDec);
						Variable * _var = new Variable(_xParID->pID->nombre, v, true);
						_var->deep = deep;

						if (!EstablecerVariable(_var, &(xCallEnt->entradas->at(ent_itr))))
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
							Value_BOOL * _Cond = CondicionalDeDosValores(_var->valor, COND_ACC_EQUAL, xCallEnt->entradas->at(ent_itr));

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
							Variable * _var = new Variable(ID, xCallEnt->entradas->at(ent_itr)->Clone(), false);
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
				return getRetorno();
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

				std::cout << " - " + (*funcion)->source + "\n";
				deletePtr(xCallEnt);

				return NULL;
			}
		}
	}
	//AGREGADO TODO
	delete xCallEnt;
	std::cout << "La función '" + ID + "' no está declarada o los parámetros de entrada no son correctos. ";
	return NULL;
}

bool Interprete::EstablecerVariable(Variable * var, Value ** value)
{
	if (var)
	{
		if (!var->fuerte)
		{
			deletePtr(var->valor);
			var->valor = (*value)->Clone();
			return true;
		}

		switch (var->valor->getTypeValue())
		{
			//Si la variable es automatica, es decir, no se especifico ningún valor.
			case PARAM_VOID:
			{
				deletePtr(var->valor);
				var->valor = (*value)->Clone();
				return true;
			}
			case PARAM_INT:
			{

				if ((*value)->getTypeValue() == PARAM_INT)
				{
					Value_INT * xVar = static_cast<Value_INT*>(var->valor);
					Value_INT * xVal = static_cast<Value_INT*>(*value);
					xVar->value = xVal->value;
					deletePtr(*value);
					return true;
				}
				else
				{
					std::cout << "No se puede convertir expresión, la variable '" << var->nombre << "' debe ser un entero. ";
						return false;
				}
			}
			case PARAM_DOUBLE:
			{
				if ((*value)->getTypeValue() == PARAM_INT)
				{
					Value_DOUBLE * xVar = static_cast<Value_DOUBLE*>(var->valor);
					Value_INT * xVal = static_cast<Value_INT*>(*value);
					xVar->value = xVal->value;
					deletePtr(*value);
					return true;
				}
				else if ((*value)->getTypeValue() == PARAM_DOUBLE)
				{
					Value_DOUBLE * xVar = static_cast<Value_DOUBLE*>(var->valor);
					Value_DOUBLE * xVal = static_cast<Value_DOUBLE*>(*value);
					xVar->value = xVal->value;
					deletePtr(*value);
					return true;
				}
				else
				{
					std::cout << "No se puede convertir expresión, la variable " << var->nombre << " debe ser un real. ";
					return false;
				}
			}
			case PARAM_STRING:
			{
				if ((*value)->getTypeValue() == PARAM_STRING)
				{
					Value_STRING * xVar = static_cast<Value_STRING*>(var->valor);
					Value_STRING * xVal = static_cast<Value_STRING*>(*value);
					xVar->value = xVal->value;
					ReplaceAll(xVar->value, "\\n", "\n");
					deletePtr(*value);
					return true;
				}
				else
				{
					std::cout << "No se puede convertir expresión, la variable " << var->nombre << " debe ser un string. ";
					return false;
				}
			}
			case PARAM_BOOL:
			{
				if ((*value)->getTypeValue() == PARAM_BOOL)
				{
					Value_BOOL * xVar = static_cast<Value_BOOL*>(var->valor);
					Value_BOOL * xVal = static_cast<Value_BOOL*>(*value);
					xVar->value = xVal->value;
					deletePtr(*value);
					return true;
				}
				else if ((*value)->getTypeValue() == PARAM_INT)
				{
					Value_BOOL * xVar = static_cast<Value_BOOL*>(var->valor);
					Value_INT * xVal = static_cast<Value_INT*>(*value);
					xVar->value = xVal->value;
					deletePtr(*value);
					return true;
				}
				else if ((*value)->getTypeValue() == PARAM_DOUBLE)
				{
					Value_BOOL * xVar = static_cast<Value_BOOL*>(var->valor);
					Value_DOUBLE * xVal = static_cast<Value_DOUBLE*>(*value);
					xVar->value = xVal->value;
					deletePtr(*value);
					return true;
				}
				else
				{
					std::cout << "No se puede convertir expresión, la variable " << var->nombre << " debe ser un booleano. ";
					return false;
				}
			}
		}
		return false;
	}
	return false;
}


Value_BOOL *  Interprete::CondicionalDeDosValores(Value * value1, CondicionalAccionType accion, Value * value2)
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
				case COND_ACC_EQUAL: if (x1->value == x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_NOT_EQUAL:  if (x1->value != x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MINOR:  if (x1->value < x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MAJOR:  if (x1->value > x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MINOR_OR_EQUAL:  if (x1->value <= x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MAJOR_OR_EQUAL:  if (x1->value >= x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				}
				break;
			}

			case PARAM_DOUBLE:
			{
				Value_DOUBLE * x2 = static_cast<Value_DOUBLE*>(value2);

				switch (accion)
				{
				case COND_ACC_EQUAL: if (x1->value == x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_NOT_EQUAL:  if (x1->value != x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MINOR:  if (x1->value < x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MAJOR:  if (x1->value > x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MINOR_OR_EQUAL:  if (x1->value <= x2->value)  return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MAJOR_OR_EQUAL:  if (x1->value >= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				}
				break;
			}

			case PARAM_STRING:
			{
				//Value_STRING * x2 = static_cast<Value_STRING*>(value2);

				std::cout << "No se puede comparar un entero con una cadena de caracteres. "; 
				return NULL;
			}

			case PARAM_BOOL:
			{
				Value_BOOL * x2 = static_cast<Value_BOOL*>(value2);
				switch (accion)
				{
					case COND_ACC_EQUAL: if (x1->value == x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
					case COND_ACC_NOT_EQUAL:  if (x1->value != x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
					case COND_ACC_MINOR:  if (x1->value < x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
					case COND_ACC_MAJOR:  if (x1->value > x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
					case COND_ACC_MINOR_OR_EQUAL:  if (x1->value <= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
					case COND_ACC_MAJOR_OR_EQUAL:  if (x1->value >= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
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
				case COND_ACC_EQUAL: if (x1->value == x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_NOT_EQUAL:  if (x1->value != x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MINOR:  if (x1->value < x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MAJOR:  if (x1->value > x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MINOR_OR_EQUAL:  if (x1->value <= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MAJOR_OR_EQUAL:  if (x1->value >= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				}
				break;
			}

			case PARAM_DOUBLE:
			{
				Value_DOUBLE * x2 = static_cast<Value_DOUBLE*>(value2);

				switch (accion)
				{
				case COND_ACC_EQUAL: if (x1->value == x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_NOT_EQUAL:  if (x1->value != x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MINOR:  if (x1->value < x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MAJOR:  if (x1->value > x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MINOR_OR_EQUAL:  if (x1->value <= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MAJOR_OR_EQUAL:  if (x1->value >= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				}
				break;
			}

			case PARAM_STRING:
			{
				Value_STRING * x2 = static_cast<Value_STRING*>(value2);

				std::cout << "No se puede comparar un entero con una cadena de caracteres. ";
				return NULL;
			}

			case PARAM_BOOL:
			{
				Value_BOOL * x2 = static_cast<Value_BOOL*>(value2);
				switch (accion)
				{
				case COND_ACC_EQUAL: if (x1->value == x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_NOT_EQUAL:  if (x1->value != x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MINOR:  if (x1->value < x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MAJOR:  if (x1->value > x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MINOR_OR_EQUAL:  if (x1->value <= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MAJOR_OR_EQUAL:  if (x1->value >= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
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
		case PARAM_INT:
		{
			std::cout << "No se puede comparar un entero con una cadena de caracteres. ";
			return NULL;
		}

		case PARAM_DOUBLE:
		{
			std::cout << "No se puede comparar un real con una cadena de caracteres. ";
			return NULL;
		}

		case PARAM_STRING:
		{
			Value_STRING * x2 = static_cast<Value_STRING*>(value2);

			switch (accion)
			{
			case COND_ACC_EQUAL: if (x1->value == x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
			case COND_ACC_NOT_EQUAL:  if (x1->value != x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
			case COND_ACC_MINOR:  if (x1->value < x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
			case COND_ACC_MAJOR:  if (x1->value > x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
			case COND_ACC_MINOR_OR_EQUAL:  if (x1->value <= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
			case COND_ACC_MAJOR_OR_EQUAL:  if (x1->value >= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
			}
		}

		case PARAM_BOOL:
		{
			std::cout << "No se puede comparar un booleano con una cadena de caracteres. ";
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
				case COND_ACC_EQUAL: if (x1->value == x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_NOT_EQUAL:  if (x1->value != x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MINOR:  if (x1->value < x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MAJOR:  if (x1->value > x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MINOR_OR_EQUAL:  if (x1->value <= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MAJOR_OR_EQUAL:  if (x1->value >= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				}
				break;
			}

			case PARAM_DOUBLE:
			{
				Value_DOUBLE * x2 = static_cast<Value_DOUBLE*>(value2);

				switch (accion)
				{
				case COND_ACC_EQUAL: if (x1->value == x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_NOT_EQUAL:  if (x1->value != x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MINOR:  if (x1->value < x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MAJOR:  if (x1->value > x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MINOR_OR_EQUAL:  if (x1->value <= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MAJOR_OR_EQUAL:  if (x1->value >= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				}
				break;
			}

			case PARAM_STRING:
			{
				Value_STRING * x2 = static_cast<Value_STRING*>(value2);

				std::cout << "No se puede comparar un entero con una cadena de caracteres. ";
				return NULL;
			}

			case PARAM_BOOL:
			{
				Value_BOOL * x2 = static_cast<Value_BOOL*>(value2);
				switch (accion)
				{
				case COND_ACC_EQUAL: if (x1->value == x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_NOT_EQUAL:  if (x1->value != x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MINOR:  if (x1->value < x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MAJOR:  if (x1->value > x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MINOR_OR_EQUAL:  if (x1->value <= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				case COND_ACC_MAJOR_OR_EQUAL:  if (x1->value >= x2->value) return new Value_BOOL(true); else return new Value_BOOL(false);
				}
				return NULL;
			}
		}
	}
	}

	std::cout << "Condición inválida. ";
	return NULL;
}

Value * Interprete::OperacionSobreValores(Value * value1, MATH_ACCION accion, Value * value2)
{
	if (!value1 || !value2)
	{
		return NULL;
	}

	if (value1->getTypeValue() == PARAM_VOID || value2->getTypeValue() == PARAM_VOID)
	{
		std::cout << "Imposible realizar la operación, uno de los valores es un 'void'. Los valores vacios no pueden tratarse operacionalmente. ";
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
					case MATH_SUMA: return new Value_INT(x1->value + x2->value);
					case MATH_RESTA: return new Value_INT(x1->value - x2->value);
					case MATH_MULT: return new Value_INT(x1->value * x2->value);
					case MATH_DIV_ENTERA: return new Value_INT(((int)x1->value) / x2->value);
					case MATH_DIV:  return new Value_DOUBLE(x1->value / ((double)x2->value));
					case MATH_EXP: return new Value_INT(pow(x1->value, x2->value));
					case MATH_MOD: return new Value_INT(x1->value % x2->value);
					}
					return NULL;
				}

				case PARAM_DOUBLE:
				{
					Value_DOUBLE * x2 = static_cast<Value_DOUBLE*>(value2);

					switch (accion)
					{
					case MATH_SUMA: return new Value_DOUBLE(x1->value + x2->value);
					case MATH_RESTA: return new Value_DOUBLE(x1->value - x2->value);
					case MATH_MULT: return new Value_DOUBLE(x1->value * x2->value);
					case MATH_DIV_ENTERA: return new Value_INT(((int)x1->value) / x2->value);
					case MATH_DIV:  return new Value_DOUBLE(x1->value / x2->value);
					case MATH_EXP: return new Value_DOUBLE(pow(x1->value, x2->value));
					case MATH_MOD: std::cout << "El valor de un módulo debe ser entero. "; return NULL;
					}

					return NULL;
				}

				case PARAM_STRING:
				{
					Value_STRING * x2 = static_cast<Value_STRING*>(value2);

					switch (accion)
					{
					case MATH_SUMA: return new Value_STRING(std::to_string(x1->value) + x2->value);
					case MATH_RESTA: std::cout << "El simbolo '-' no se puede aplicar a una operación con cadena de caracteres. "; return NULL;
					case MATH_MULT: std::cout << "El simbolo '*' no se puede aplicar a una operación con cadena de caracteres. "; return NULL;
					case MATH_DIV_ENTERA: std::cout << "El simbolo 'div' no se puede aplicar a una operación con cadena de caracteres."; return NULL;
					case MATH_DIV: std::cout << "El simbolo '/' no se puede aplicar a una operación con cadena de caracteres."; return NULL;
					case MATH_EXP: std::cout << "El simbolo '^' no se puede aplicar a una operación con cadena de caracteres."; return NULL;
					case MATH_MOD: std::cout << "El simbolo '%' no se puede aplicar a una operación con cadena de caracteres. "; return NULL;
					}

					return NULL;
				}
				case PARAM_BOOL:
				{
					Value_BOOL * x2 = static_cast<Value_BOOL*>(value2);

					switch (accion)
					{
					case MATH_SUMA: return new Value_INT(x1->value + x2->value);
					case MATH_RESTA: return new Value_INT(x1->value - x2->value);
					case MATH_MULT: return new Value_INT(x1->value * x2->value);
					case MATH_DIV_ENTERA: return new Value_INT(((int)x1->value) / x2->value);
					case MATH_DIV:  return new Value_DOUBLE(x1->value / ((double)x2->value));
					case MATH_EXP: return new Value_INT(pow(x1->value, x2->value));
					case MATH_MOD: return new Value_INT(x1->value % x2->value);
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
				case MATH_SUMA: return new Value_DOUBLE(x1->value + x2->value);
				case MATH_RESTA: return new Value_DOUBLE(x1->value - x2->value);
				case MATH_MULT: return new Value_DOUBLE(x1->value * x2->value);
				case MATH_DIV_ENTERA: return new Value_INT((int)(x1->value / x2->value));
				case MATH_DIV:  return new Value_DOUBLE(x1->value / x2->value);
				case MATH_EXP: return new Value_DOUBLE(pow(x1->value, x2->value));
				case MATH_MOD: std::cout << "El valor de un módulo debe ser entero."; return NULL;
				}
				return NULL;
			}

			case PARAM_DOUBLE:
			{
				Value_DOUBLE * x2 = static_cast<Value_DOUBLE*>(value2);

				switch (accion)
				{
				case MATH_SUMA: return new Value_DOUBLE(x1->value + x2->value);
				case MATH_RESTA: return new Value_DOUBLE(x1->value - x2->value);
				case MATH_MULT: return new Value_DOUBLE(x1->value * x2->value);
				case MATH_DIV_ENTERA: return new Value_INT((int)(x1->value / x2->value));
				case MATH_DIV:  return new Value_DOUBLE(x1->value / x2->value);
				case MATH_EXP: return new Value_DOUBLE(pow(x1->value, x2->value));
				case MATH_MOD: std::cout << "El valor de un módulo debe ser entero."; return NULL;
				}

				return NULL;
			}

			case PARAM_STRING:
			{
				Value_STRING * x2 = static_cast<Value_STRING*>(value2);

				switch (accion)
				{
				case MATH_SUMA: return new Value_STRING(std::to_string(x1->value) + x2->value);
				case MATH_RESTA: std::cout << "El simbolo '-' no se puede aplicar a una operación con cadena de caracteres. "; return NULL;
				case MATH_MULT: std::cout << "El simbolo '*' no se puede aplicar a una operación con cadena de caracteres."; return NULL;
				case MATH_DIV_ENTERA: std::cout << "El simbolo 'div' no se puede aplicar a una operación con cadena de caracteres."; return NULL;
				case MATH_DIV:  std::cout << "El simbolo '/' no se puede aplicar a una operación con cadena de caracteres. "; return NULL;
				case MATH_EXP: std::cout << "El simbolo '^' no se puede aplicar a una operación con cadena de caracteres. "; return NULL;
				case MATH_MOD:std::cout << "El simbolo '%' no se puede aplicar a una operación con cadena de caracteres. "; return NULL;
				}

				return NULL;
			}
			case PARAM_BOOL:
			{
				Value_BOOL * x2 = static_cast<Value_BOOL*>(value2);

				switch (accion)
				{
				case MATH_SUMA: return new Value_DOUBLE(x1->value + x2->value);
				case MATH_RESTA: return new Value_DOUBLE(x1->value - x2->value);
				case MATH_MULT: return new Value_DOUBLE(x1->value * x2->value);
				case MATH_DIV_ENTERA: return new Value_INT((int)(x1->value / x2->value));
				case MATH_DIV:  return new Value_DOUBLE(x1->value / x2->value);
				case MATH_EXP: return new Value_DOUBLE(pow(x1->value, x2->value));
				case MATH_MOD: std::cout << "El valor de un módulo debe ser entero."; return NULL;
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
					case MATH_SUMA: return new Value_STRING(x1->value + std::to_string(x2->value));
					case MATH_RESTA: std::cout << "El simbolo '-' no se puede aplicar a una operación con cadena de caracteres."; return NULL;
					case MATH_MULT: std::cout << "El simbolo '*' no se puede aplicar a una operación con cadena de caracteres."; return NULL;
					case MATH_DIV_ENTERA: std::cout << "El simbolo 'div' no se puede aplicar a una operación con cadena de caracteres. "; return NULL;
					case MATH_DIV: std::cout << "El simbolo '/' no se puede aplicar a una operación con cadena de caracteres. "; return NULL;
					case MATH_EXP: std::cout << "El simbolo '^' no se puede aplicar a una operación con cadena de caracteres."; return NULL;
					case MATH_MOD: std::cout << "El simbolo '%' no se puede aplicar a una operación con cadena de caracteres. "; return NULL;
					}
					return NULL;
				}

				case PARAM_DOUBLE:
				{
					Value_DOUBLE * x2 = static_cast<Value_DOUBLE*>(value2);

					switch (accion)
					{
					case MATH_SUMA: return new Value_STRING(x1->value + std::to_string(x2->value));
					case MATH_RESTA: std::cout << "El simbolo '-' no se puede aplicar a una operación con cadena de caracteres. "; return NULL;
					case MATH_MULT: std::cout << "El simbolo '*' no se puede aplicar a una operación con cadena de caracteres."; return NULL;
					case MATH_DIV_ENTERA: std::cout << "El simbolo 'div' no se puede aplicar a una operación con cadena de caracteres."; return NULL;
					case MATH_DIV:  std::cout << "El simbolo '/' no se puede aplicar a una operación con cadena de caracteres."; return NULL;
					case MATH_EXP: std::cout << "El simbolo '^' no se puede aplicar a una operación con cadena de caracteres. "; return NULL;
					case MATH_MOD: std::cout << "El simbolo '%' no se puede aplicar a una operación con cadena de caracteres. "; return NULL;
					}

					return NULL;
				}

				case PARAM_STRING:
				{
					Value_STRING * x2 = static_cast<Value_STRING*>(value2);

					switch (accion)
					{
					case MATH_SUMA: return new Value_STRING(x1->value + x2->value);
					case MATH_RESTA: std::cout << "El simbolo '-' no se puede aplicar a una operación con cadena de caracteres."; return NULL;
					case MATH_MULT: std::cout << "El simbolo '*' no se puede aplicar a una operación con cadena de caracteres. "; return NULL;
					case MATH_DIV_ENTERA: std::cout << "El simbolo 'div' no se puede aplicar a una operación con cadena de caracteres. "; return NULL;
					case MATH_DIV:  std::cout << "El simbolo '/' no se puede aplicar a una operación con cadena de caracteres."; return NULL;
					case MATH_EXP: std::cout << "El simbolo '^' no se puede aplicar a una operación con cadena de caracteres. "; return NULL;
					case MATH_MOD: std::cout << "El simbolo '%' no se puede aplicar a una operación con cadena de caracteres. "; return NULL;
					}

					return NULL;
				}
				case PARAM_BOOL:
				{
					Value_BOOL * x2 = static_cast<Value_BOOL*>(value2);

					switch (accion)
					{
					case MATH_SUMA: return new Value_STRING(x1->value + std::to_string(x2->value));
					case MATH_RESTA: std::cout << "El simbolo '-' no se puede aplicar a una operación con cadena de caracteres."; return NULL;
					case MATH_MULT: std::cout << "El simbolo '*' no se puede aplicar a una operación con cadena de caracteres. "; return NULL;
					case MATH_DIV_ENTERA: std::cout << "El simbolo 'div' no se puede aplicar a una operación con cadena de caracteres. "; return NULL;
					case MATH_DIV:  std::cout << "El simbolo '/' no se puede aplicar a una operación con cadena de caracteres."; return NULL;
					case MATH_EXP: std::cout << "El simbolo '^' no se puede aplicar a una operación con cadena de caracteres. "; return NULL;
					case MATH_MOD: std::cout << "El simbolo '%' no se puede aplicar a una operación con cadena de caracteres. "; return NULL;
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
				case MATH_SUMA: return new Value_INT(x1->value + x2->value);
				case MATH_RESTA: return new Value_INT(x1->value - x2->value);
				case MATH_MULT: return new Value_INT(x1->value * x2->value);
				case MATH_DIV_ENTERA: return new Value_INT(((int)x1->value) / x2->value);
				case MATH_DIV:  return new Value_DOUBLE(x1->value / ((double)x2->value));
				case MATH_EXP: return new Value_INT(pow(x1->value, x2->value));
				case MATH_MOD: return new Value_INT(x1->value % x2->value);
				}
				return NULL;
			}

			case PARAM_DOUBLE:
			{
				Value_DOUBLE * x2 = static_cast<Value_DOUBLE*>(value2);

				switch (accion)
				{
				case MATH_SUMA: return new Value_DOUBLE(x1->value + x2->value);
				case MATH_RESTA: return new Value_DOUBLE(x1->value - x2->value);
				case MATH_MULT: return new Value_DOUBLE(x1->value * x2->value);
				case MATH_DIV_ENTERA: return new Value_INT(((int)x1->value) / x2->value);
				case MATH_DIV:  return new Value_DOUBLE(x1->value / x2->value);
				case MATH_EXP: return new Value_DOUBLE(pow(x1->value, x2->value));
				case MATH_MOD: std::cout << "El valor de un módulo debe ser entero. "; return NULL;
				}

				return NULL;
			}

			case PARAM_STRING:
			{
				Value_STRING * x2 = static_cast<Value_STRING*>(value2);

				switch (accion)
				{
				case MATH_SUMA: return new Value_STRING(std::to_string(x1->value) + x2->value);
				case MATH_RESTA: std::cout << "El simbolo '-' no se puede aplicar a una operación con cadena de caracteres. "; return NULL;
				case MATH_MULT: std::cout << "El simbolo '*' no se puede aplicar a una operación con cadena de caracteres. "; return NULL;
				case MATH_DIV_ENTERA: std::cout << "El simbolo 'div' no se puede aplicar a una operación con cadena de caracteres."; return NULL;
				case MATH_DIV: std::cout << "El simbolo '/' no se puede aplicar a una operación con cadena de caracteres."; return NULL;
				case MATH_EXP: std::cout << "El simbolo '^' no se puede aplicar a una operación con cadena de caracteres."; return NULL;
				case MATH_MOD: std::cout << "El simbolo '%' no se puede aplicar a una operación con cadena de caracteres. "; return NULL;
				}

				return NULL;
			}
			case PARAM_BOOL:
			{
				Value_BOOL * x2 = static_cast<Value_BOOL*>(value2);

				switch (accion)
				{
				case MATH_SUMA: return new Value_INT(x1->value + x2->value);
				case MATH_RESTA: return new Value_INT(x1->value - x2->value);
				case MATH_MULT: return new Value_INT(x1->value * x2->value);
				case MATH_DIV_ENTERA: return new Value_INT(((int)x1->value) / x2->value);
				case MATH_DIV:  return new Value_DOUBLE(x1->value / ((double)x2->value));
				case MATH_EXP: return new Value_INT(pow(x1->value, x2->value));
				case MATH_MOD: return new Value_INT(x1->value % x2->value);
				}
				return NULL;
			}
			}
		}

		std::cout << "Operación inválida. Un miembro es nulo o no ha sido inicializado. ";
		return NULL;
	}
}


bool Interprete::ConversionXtoBool(Value * valOp, bool& salida)
{
	switch (valOp->getTypeValue())
	{
	case PARAM_BOOL:
	{
		Value_BOOL * xIn = static_cast<Value_BOOL*>(valOp);
		salida = xIn->value;
		break;
	}

	case PARAM_INT:
	{
		Value_INT * xIn = static_cast<Value_INT*>(valOp);

		if (xIn->value)
			salida = true;
		else
			salida = false;

		break;
	}

	case PARAM_DOUBLE:
	{
		Value_DOUBLE * xIn = static_cast<Value_DOUBLE*>(valOp);

		if (xIn->value)
			salida = true;
		else
			salida = false;

		break;
	}

	case PARAM_STRING:
	{
		Value_STRING * xIn = static_cast<Value_STRING*>(valOp);

		if (xIn->value != "")
			salida = true;
		else
			salida = false;

		break;
	}

	default:
	{
		std::cout << "La expresión debe tener un tipo bool o una expresión compatible convertible.";
		return false;
	}
	}

	return true;
}


Value_BOOL * Interprete::Condicionales(Parser_Condicional * pCond, std::vector<Variable*> * variables)
{
	if (pCond->tipo == COND_REC)
	{
		Condicional_Recursiva * x = static_cast<Condicional_Recursiva*>(pCond);
		Value_BOOL * val = Condicionales(x->c1, variables);

		if (x->ca)
		{
			if(x->ca->accion == COND_AG_AND)
			{
				Value_BOOL * val2 = Condicionales(x->ca->cond, variables);
				val->value = val->value && val2->value;
				delete val2;
				
				if (x->negada)
					val->value = !val->value;

			    return val;
			}
			else if (x->ca->accion == COND_AG_OR)
			{
				Value_BOOL * val2 = Condicionales(x->ca->cond, variables);
				val->value = val->value || val2->value;
				delete val2;

				if (x->negada)
					val->value = !val->value;

				return val;
			}
			else
			{
				delete val;
				return NULL;
			}
		}
		else
		{
			if (x->negada)
				val->value = !val->value;

			return val;
		}
	}
	else if (pCond->tipo == COND_OP)
	{
		Condicional_Operacion * x = static_cast<Condicional_Operacion*>(pCond);

		Value * valOp = Operaciones(x->op1, variables);

		if (valOp == NULL)
			return NULL;

		Value_BOOL * res = new Value_BOOL(true);

		if (x->adicional_ops)
		{
			for (unsigned itr = 0; (itr+1) < x->adicional_ops->size(); itr++)
			{
				Value * op1 = Operaciones(x->adicional_ops->at(itr)->op, variables);
				Value * op2 = Operaciones(x->adicional_ops->at(itr+1)->op, variables);

				if (!op1 || !op2)
				{
					delete op1;
					delete op2;
					return NULL;
				}


				Value_BOOL * b = CondicionalDeDosValores(op1, x->adicional_ops->at(itr + 1)->AccType, op2);

				if (b == NULL)
				{
					delete op1;
					delete op2;
					return NULL;
				}

				res->value = b->value;

				delete b;
				delete op1;
				delete op2;
			}	

			if (res->value)
			{
				Value * opN = Operaciones(x->adicional_ops->at(0)->op, variables);

				if (!opN)
				{
					return NULL;
				}

				Value_BOOL * b = CondicionalDeDosValores(valOp, x->adicional_ops->at(0)->AccType, opN);

				if (b == NULL)
				{
					delete opN;
					return NULL;
				}

				res->value = b->value;
	
				delete b;
				delete opN;
			}
			delete valOp;
		}
		else
		{
			//Si es un condicional etc.
			if (!ConversionXtoBool(valOp, res->value))
			{
				delete valOp;
				delete res;
				return NULL;
			}

			delete valOp;
		}

		if (x->ca)
		{
			if (x->ca->accion == COND_AG_AND)
			{
				Value_BOOL * val2 = Condicionales(x->ca->cond, variables);
				res->value = res->value && val2->value;
				delete val2;
				return res;
			}
			else if (x->ca->accion == COND_AG_OR)
			{
				Value_BOOL * val2 = Condicionales(x->ca->cond, variables);
				res->value = res->value || val2->value;
				delete val2;
				return res;
			}
			else
			{
				delete res;
				return NULL;
			}
		}

		return res;
	}

	return NULL;

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

Value * Interprete::Operaciones(Parser_Operacion * pOp, std::vector<Variable*> * variables, std::vector<OperacionComp*>* componente)
{
	if (pOp->tipo == OP_REC_OP)
	{
		Operacion_Recursiva * x = static_cast<Operacion_Recursiva*>(pOp);

		std::vector<OperacionComp*>* componenteInterno = new std::vector<OperacionComp*>();
		Value * val = Operaciones(x->op1, variables, componenteInterno);

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
			componente->push_back(new OperacionComp(val, MATH_NONE));	
		}
	}
	else if (pOp->tipo == OP_MATH)
	{
		Operacion_Math * x = static_cast<Operacion_Math*>(pOp);
		
		Value * val = NULL;
		switch (x->op1->tipo)
		{
			case VAL_LIT:
			{
				Parser_Literal * xLit = static_cast<Parser_Literal*>(x->op1);

				if (xLit->negado)
				{
					bool resBol = true;
					if (!ConversionXtoBool(xLit->value, resBol))
					{
						return NULL;
					}
					else val = new Value_BOOL(!resBol);
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
						bool resBol = true;
						if (!ConversionXtoBool(var->valor, resBol))
						{
							return NULL;
						}
						else val = new Value_BOOL(!resBol);
					}
					else val = var->valor->Clone();
				}
				else
				{
					std::cout << "Variable '" + xID->nombre + "' no existe. ";
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
				componente->push_back(new OperacionComp(val, MATH_NONE));
				//deletePtr(val);
			}
		}
		else  return NULL;
	}
	else {
		std::cout << "MAL... ";
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
	// ^ sqr   >   * / div %    >    + -

	//###### PRIORIDAD 1 -> Exponenciales y raices cuadradas
	OperacionComp * temp = NULL;

	for (unsigned itr = 0; itr < componente->size(); itr++)
	{
		if (temp)
		{
			Value * v = OperacionSobreValores(temp->val, temp->oper, componente->at(itr)->val);
			
			if (v)
			{
				delete componente->at(itr)->val;
				componente->at(itr)->val = v;
			}
			else
			{
				delete temp;
				return NULL;
			}

			temp = NULL;
		}

		if (componente->at(itr)->oper == MATH_EXP)
		{
			componente->at(itr)->delete_ready = true;
			temp = componente->at(itr);
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

	//En el caso de que ya no se puedan realizar más operaciones, devolvemos resultado.
	if (componente->size() == 1)
	{
		Value * res = componente->at(0)->val->Clone();
		return res;
	}

	// ###### PRIORIDAD 2 -> Multiplicaciones, Divisiones, Divisiones enteras, Modulos
	temp = NULL;
	for (unsigned itr = 0; itr < componente->size(); itr++)
	{
		if (temp)
		{
			Value * v = OperacionSobreValores(temp->val, temp->oper, componente->at(itr)->val);

			if (v)
			{
				delete componente->at(itr)->val;
				componente->at(itr)->val = v;
			}
			else return NULL;

			temp = NULL;
		}

		if (componente->at(itr)->oper == MATH_DIV || componente->at(itr)->oper == MATH_DIV_ENTERA || componente->at(itr)->oper == MATH_MOD || componente->at(itr)->oper == MATH_MULT)
		{
			componente->at(itr)->delete_ready = true;
			temp = componente->at(itr);
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

	//En el caso de que ya no se puedan realizar más operaciones, devolvemos resultado.
	if (componente->size() == 1)
	{
		Value * res = componente->at(0)->val->Clone();
		return res;
	}

	// ###### PRIORIDAD 3 -> Sumas, Restas
	temp = NULL;
	for (unsigned itr = 0; itr < componente->size(); itr++)
	{
		if (temp)
		{
			Value * v = OperacionSobreValores(temp->val, temp->oper, componente->at(itr)->val);

			if (v)
			{
				delete componente->at(itr)->val;
				componente->at(itr)->val = v;
			}
			else return NULL;

			temp = NULL;
		}

		if (componente->at(itr)->oper == MATH_SUMA || componente->at(itr)->oper == MATH_RESTA)
		{
			componente->at(itr)->delete_ready = true;
			temp = componente->at(itr);
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

	//En el caso de que ya no se puedan realizar más operaciones, devolvemos resultado.
	if (componente->size() == 1)
	{
		Value * res = componente->at(0)->val->Clone();
		return res;
	}

	return NULL;
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