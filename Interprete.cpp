
#include "Interprete.h"


bool Interprete::CargarDatos(Parser* parser)
{
	//Borramos la lista de funciones existentes actualmente.
	for (std::vector<Parser_Funcion*>::iterator it = funciones.begin(); it != funciones.end(); ++it)
	{
		delete (*it);
	}

	std::cout << "TOKENS: \n";

	for (unsigned i = 0; i < parser->tokenizer.tokens.size(); i++)
	{
		std::cout << "  [" << parser->tokenizer.tokens.at(i) << "]  ";
	}

	std::cout << "LINEAS: \n";
	for (unsigned i = 0; i < parser->tokenizer.num_Lines.size(); i++)
	{
		std::cout << parser->tokenizer.num_Lines.at(i) << "  ";
	}

	std::cout << "\n\n";

	//Interpretando datos...
	/*
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
			std::cout << "ERROR!!!!\n";
	}
	*/
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
			if (Interprete_Sentencia(p2, &variables))
				continue;
			else
			{
				delete p2;
				return false;
			}
		}
		else
		{
			delete p2;
			std::cout << "ERROR del PARSER!!!!\n";
			return false;
		}
		delete p2;
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
					return false;
				}
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
				return false;
			return true;
		}

		//Operaciones, si son llamadas como funciones solo podrán ser sumatorios, etc. var++, ++var etc.
		case SENT_OP:
		{
			Sentencia_Operacional * x = static_cast<Sentencia_Operacional*>(sentencia);

			if (x->pOp->tipo == OP_ID)
			{
				Operacion_ID * x2 = static_cast<Operacion_ID*>(x->pOp);

				if (x2->accion == ID_INCREMENTO)
				{
					Variable * v1 = BusquedaVariable(x2->ID->nombre, variables);

					if (v1)
					{
						if (v1->valor->getTypeValue() == PARAM_INT)
						{
							Value_INT * x = static_cast<Value_INT*>(v1->valor);
							return EstablecerVariable(v1, new Value_INT(x->value+1));
						}
						else if (v1->valor->getTypeValue() == PARAM_DOUBLE)
						{
							Value_DOUBLE * x = static_cast<Value_DOUBLE*>(v1->valor);
							return EstablecerVariable(v1, new Value_DOUBLE(x->value + 1));
						}
						else
						{
							std::cout << "La variable '" << v1->nombre << "' no puede ser incrementada.\n";
							return false;
						}

					}
					else
					{
						std::cout << "La variable '" << v1->nombre << "' no existe.\n";
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
							Value_INT * x = static_cast<Value_INT*>(v1->valor);
							return EstablecerVariable(v1, new Value_INT(x->value - 1));
						}
						else if (v1->valor->getTypeValue() == PARAM_DOUBLE)
						{
							Value_DOUBLE * x = static_cast<Value_DOUBLE*>(v1->valor);
							return EstablecerVariable(v1, new Value_DOUBLE(x->value - 1));
						}
						else
						{
							std::cout << "La variable '" << v1->nombre << "' no puede ser decrementada.\n";
							return false;
						}

					}
					else
					{
						std::cout << "La variable '" << v1->nombre << "' no existe.\n";
						return false;
					}
				}
				else
				{
					std::cout << "La expresión no se iguala a ninguna variable.\n";
					return false;
				}

			}
			return false;
		}
		case SENT_IGU:
		{
			Sentencia_Igualdad * x = static_cast<Sentencia_Igualdad*>(sentencia);

			Variable * var = Interprete_NuevaVariable(x->pIg->param, variables, true);

			if (var)
			{
			 if(x->pIg->cond)
				{
					if (x->pIg->cond->tipo == COND_OP)
					{
						Condicional_Operacion * xOp = static_cast<Condicional_Operacion*>(x->pIg->cond);
						if (xOp->ca == NULL && xOp->adicional_ops == NULL)
						{
							Value * v = Operaciones(xOp->op1, variables);

							if (v == NULL)
								return false;

							if (!EstablecerVariable(var, v))
							{
								return false;
							}
							return true;
						}
					}

					Value_BOOL * v = Condicionales(x->pIg->cond, variables);

					if (v == NULL)
						return false;

					if (!EstablecerVariable(var, v))
					{
						return false;
					}
					return true;
				}
			}
			return false;
		}
		// FUNCION PRINT:
		// Permite escribir por consola un valor dado.
		case SENT_PRINT:
		{
			Sentencia_Print * x = static_cast<Sentencia_Print*>(sentencia);
			Value * v = Operaciones(x->pOp, variables);

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

				return true;
			}
			else
				return false;

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
					if (!Interprete_Sentencia(x->pS, variables))
						return false;

					return true;
				}
			}
			else
			{
				if (x->pElse)
				{
					if (!Interprete_Sentencia(x->pElse, variables))
						return false;

					return true;
				}
			}

			return true;
		}

	}

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
				std::cout << "Error al intentar inicializar la variable '" << x->pID->nombre << "'.\n";
				return false;
			}
			else
			{
				if (existe)
				{
					return var1;
				}
				//Existe
				std::cout << "Error de ejecución: La variable con nombre '" << x->pID->nombre << "' fue declarada más de una vez. \n";
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
			std::cout << "Error al declarar parámetro.\n";
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

bool Interprete::EstablecerVariable(Variable * var, Value * value)
{
	if (var)
	{
		if (!var->fuerte)
		{
			delete var->valor;
			var->valor = value->Clone();
			return true;
		}

		switch (var->valor->getTypeValue())
		{
			//Si la variable es automatica, es decir, no se especifico ningún valor.
			case PARAM_NONE:
			{
				delete var->valor;
				var->valor = value->Clone();
				return true;
			}
			case PARAM_INT:
			{
				if (value->getTypeValue() == PARAM_INT)
				{
					Value_INT * xVar = static_cast<Value_INT*>(var->valor);
					Value_INT * xVal = static_cast<Value_INT*>(value);
					xVar->value = xVal->value;
					delete value;
					return true;
				}
				else
				{
					std::cout << "No se puede convertir expresión, la variable " << var->nombre << " es un entero.\n";
						return false;
				}
			}
			case PARAM_DOUBLE:
			{
				if (value->getTypeValue() == PARAM_INT)
				{
					Value_DOUBLE * xVar = static_cast<Value_DOUBLE*>(var->valor);
					Value_INT * xVal = static_cast<Value_INT*>(value);
					xVar->value = xVal->value;
					delete value;
					return true;
				}
				else if (value->getTypeValue() == PARAM_DOUBLE)
				{
					Value_DOUBLE * xVar = static_cast<Value_DOUBLE*>(var->valor);
					Value_DOUBLE * xVal = static_cast<Value_DOUBLE*>(value);
					xVar->value = xVal->value;
					delete value;
					return true;
				}
				else
				{
					std::cout << "No se puede convertir expresión, la variable " << var->nombre << " es un real.\n";
					return false;
				}
			}
			case PARAM_STRING:
			{
				if (value->getTypeValue() == PARAM_STRING)
				{
					Value_STRING * xVar = static_cast<Value_STRING*>(var->valor);
					Value_STRING * xVal = static_cast<Value_STRING*>(value);
					xVar->value = xVal->value;
					ReplaceAll(xVar->value, "\\n", "\n");
					delete value;
					return true;
				}
				else
				{
					std::cout << "No se puede convertir expresión, la variable " << var->nombre << " es un string.\n";
					return false;
				}
			}
			case PARAM_BOOL:
			{
				if (value->getTypeValue() == PARAM_BOOL)
				{
					Value_BOOL * xVar = static_cast<Value_BOOL*>(var->valor);
					Value_BOOL * xVal = static_cast<Value_BOOL*>(value);
					xVar->value = xVal->value;
					delete value;
					return true;
				}
				else if (value->getTypeValue() == PARAM_INT)
				{
					Value_BOOL * xVar = static_cast<Value_BOOL*>(var->valor);
					Value_INT * xVal = static_cast<Value_INT*>(value);
					xVar->value = xVal->value;
					delete value;
					return true;
				}
				else if (value->getTypeValue() == PARAM_DOUBLE)
				{
					Value_BOOL * xVar = static_cast<Value_BOOL*>(var->valor);
					Value_DOUBLE * xVal = static_cast<Value_DOUBLE*>(value);
					xVar->value = xVal->value;
					delete value;
					return true;
				}
				else
				{
					std::cout << "No se puede convertir expresión, la variable " << var->nombre << " es un booleano.\n";
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

				std::cout << "No se puede comparar un entero con una cadena de caracteres. \n"; 
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

				std::cout << "No se puede comparar un entero con una cadena de caracteres. \n";
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
			std::cout << "No se puede comparar un entero con una cadena de caracteres. \n";
			return NULL;
		}

		case PARAM_DOUBLE:
		{
			std::cout << "No se puede comparar un real con una cadena de caracteres. \n";
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
			std::cout << "No se puede comparar un booleano con una cadena de caracteres. \n";
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

				std::cout << "No se puede comparar un entero con una cadena de caracteres. \n";
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

	std::cout << "Condición inválida. \n";
	return NULL;
}

Value * Interprete::OperacionSobreValores(Value * value1, MATH_ACCION accion, Value * value2)
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
					case MATH_RESTA: std::cout << "El simbolo '-' no se puede aplicar a una operación con cadena de caracteres. \n"; return NULL;
					case MATH_MULT: std::cout << "El simbolo '*' no se puede aplicar a una operación con cadena de caracteres. \n"; return NULL;
					case MATH_DIV_ENTERA: std::cout << "El simbolo 'div' no se puede aplicar a una operación con cadena de caracteres.\n "; return NULL;
					case MATH_DIV: std::cout << "El simbolo '/' no se puede aplicar a una operación con cadena de caracteres.\n "; return NULL;
					case MATH_EXP: std::cout << "El simbolo '^' no se puede aplicar a una operación con cadena de caracteres.\n "; return NULL;
					case MATH_MOD: std::cout << "El simbolo '%' no se puede aplicar a una operación con cadena de caracteres. \n"; return NULL;
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
				case MATH_MOD: std::cout << "El valor de un módulo debe ser entero.\n "; return NULL;
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
				case MATH_MOD: std::cout << "El valor de un módulo debe ser entero.\n "; return NULL;
				}

				return NULL;
			}

			case PARAM_STRING:
			{
				Value_STRING * x2 = static_cast<Value_STRING*>(value2);

				switch (accion)
				{
				case MATH_SUMA: return new Value_STRING(std::to_string(x1->value) + x2->value);
				case MATH_RESTA: std::cout << "El simbolo '-' no se puede aplicar a una operación con cadena de caracteres. \n"; return NULL;
				case MATH_MULT: std::cout << "El simbolo '*' no se puede aplicar a una operación con cadena de caracteres.\n "; return NULL;
				case MATH_DIV_ENTERA: std::cout << "El simbolo 'div' no se puede aplicar a una operación con cadena de caracteres.\n "; return NULL;
				case MATH_DIV:  std::cout << "El simbolo '/' no se puede aplicar a una operación con cadena de caracteres. \n"; return NULL;
				case MATH_EXP: std::cout << "El simbolo '^' no se puede aplicar a una operación con cadena de caracteres. \n"; return NULL;
				case MATH_MOD:std::cout << "El simbolo '%' no se puede aplicar a una operación con cadena de caracteres. \n"; return NULL;
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
				case MATH_MOD: std::cout << "El valor de un módulo debe ser entero.\n "; return NULL;
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
					case MATH_RESTA: std::cout << "El simbolo '-' no se puede aplicar a una operación con cadena de caracteres.\n "; return NULL;
					case MATH_MULT: std::cout << "El simbolo '*' no se puede aplicar a una operación con cadena de caracteres.\n "; return NULL;
					case MATH_DIV_ENTERA: std::cout << "El simbolo 'div' no se puede aplicar a una operación con cadena de caracteres. \n"; return NULL;
					case MATH_DIV: std::cout << "El simbolo '/' no se puede aplicar a una operación con cadena de caracteres. \n"; return NULL;
					case MATH_EXP: std::cout << "El simbolo '^' no se puede aplicar a una operación con cadena de caracteres.\n "; return NULL;
					case MATH_MOD: std::cout << "El simbolo '%' no se puede aplicar a una operación con cadena de caracteres. \n"; return NULL;
					}
					return NULL;
				}

				case PARAM_DOUBLE:
				{
					Value_DOUBLE * x2 = static_cast<Value_DOUBLE*>(value2);

					switch (accion)
					{
					case MATH_SUMA: return new Value_STRING(x1->value + std::to_string(x2->value));
					case MATH_RESTA: std::cout << "El simbolo '-' no se puede aplicar a una operación con cadena de caracteres. \n"; return NULL;
					case MATH_MULT: std::cout << "El simbolo '*' no se puede aplicar a una operación con cadena de caracteres.\n "; return NULL;
					case MATH_DIV_ENTERA: std::cout << "El simbolo 'div' no se puede aplicar a una operación con cadena de caracteres.\n "; return NULL;
					case MATH_DIV:  std::cout << "El simbolo '/' no se puede aplicar a una operación con cadena de caracteres.\n "; return NULL;
					case MATH_EXP: std::cout << "El simbolo '^' no se puede aplicar a una operación con cadena de caracteres. \n"; return NULL;
					case MATH_MOD: std::cout << "El simbolo '%' no se puede aplicar a una operación con cadena de caracteres. \n"; return NULL;
					}

					return NULL;
				}

				case PARAM_STRING:
				{
					Value_STRING * x2 = static_cast<Value_STRING*>(value2);

					switch (accion)
					{
					case MATH_SUMA: return new Value_STRING(x1->value + x2->value);
					case MATH_RESTA: std::cout << "El simbolo '-' no se puede aplicar a una operación con cadena de caracteres.\n "; return NULL;
					case MATH_MULT: std::cout << "El simbolo '*' no se puede aplicar a una operación con cadena de caracteres. \n"; return NULL;
					case MATH_DIV_ENTERA: std::cout << "El simbolo 'div' no se puede aplicar a una operación con cadena de caracteres. \n"; return NULL;
					case MATH_DIV:  std::cout << "El simbolo '/' no se puede aplicar a una operación con cadena de caracteres.\n "; return NULL;
					case MATH_EXP: std::cout << "El simbolo '^' no se puede aplicar a una operación con cadena de caracteres. \n"; return NULL;
					case MATH_MOD: std::cout << "El simbolo '%' no se puede aplicar a una operación con cadena de caracteres. \n"; return NULL;
					}

					return NULL;
				}
				case PARAM_BOOL:
				{
					Value_BOOL * x2 = static_cast<Value_BOOL*>(value2);

					switch (accion)
					{
					case MATH_SUMA: return new Value_STRING(x1->value + std::to_string(x2->value));
					case MATH_RESTA: std::cout << "El simbolo '-' no se puede aplicar a una operación con cadena de caracteres.\n "; return NULL;
					case MATH_MULT: std::cout << "El simbolo '*' no se puede aplicar a una operación con cadena de caracteres. \n"; return NULL;
					case MATH_DIV_ENTERA: std::cout << "El simbolo 'div' no se puede aplicar a una operación con cadena de caracteres. \n"; return NULL;
					case MATH_DIV:  std::cout << "El simbolo '/' no se puede aplicar a una operación con cadena de caracteres.\n "; return NULL;
					case MATH_EXP: std::cout << "El simbolo '^' no se puede aplicar a una operación con cadena de caracteres. \n"; return NULL;
					case MATH_MOD: std::cout << "El simbolo '%' no se puede aplicar a una operación con cadena de caracteres. \n"; return NULL;
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
				case MATH_RESTA: std::cout << "El simbolo '-' no se puede aplicar a una operación con cadena de caracteres. \n"; return NULL;
				case MATH_MULT: std::cout << "El simbolo '*' no se puede aplicar a una operación con cadena de caracteres. \n"; return NULL;
				case MATH_DIV_ENTERA: std::cout << "El simbolo 'div' no se puede aplicar a una operación con cadena de caracteres.\n "; return NULL;
				case MATH_DIV: std::cout << "El simbolo '/' no se puede aplicar a una operación con cadena de caracteres.\n "; return NULL;
				case MATH_EXP: std::cout << "El simbolo '^' no se puede aplicar a una operación con cadena de caracteres.\n "; return NULL;
				case MATH_MOD: std::cout << "El simbolo '%' no se puede aplicar a una operación con cadena de caracteres. \n"; return NULL;
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

		std::cout << "Operación inválida. \n";
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
		std::cout << "La expresión debe tener un tipo bool o una expresión compatible convertible.\n";
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
			else return NULL;
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

				if (!CondicionalDeDosValores(op1, x->adicional_ops->at(itr + 1)->AccType, op2))
				{
					res->value = false;
					delete op1;
					delete op2;
				}
				delete op1;
				delete op2;
			}	

			if (res->value)
			{
				Value * opN = Operaciones(x->adicional_ops->at(0)->op, variables);

				res->value = CondicionalDeDosValores(valOp, x->adicional_ops->at(0)->AccType, opN);
				delete opN;
				delete valOp;

			}
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
			else return NULL;
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

	if (val == NULL)
		return NULL;

	for (std::vector<OperacionComp*>::iterator it = componenteInterno->begin(); it != componenteInterno->end(); ++it)
	{
		delete (*it);
	}
	delete componenteInterno;
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

		if (x->op2)
		{
			componente->push_back(new OperacionComp(val, x->op2->accion));
			if (Operaciones(x->op2->op, variables, componente) == NULL)
				return NULL;
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
					std::cout << "Variable '" + xID->nombre + "' no existe. \n";
					return NULL;
				}
				break;
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
		else return NULL;
	}
	else return NULL;


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
			else return NULL;
	
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