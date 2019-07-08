
#include "Interprete.h"
//#include "../MDOX/Errores.cpp"
//#include "../MDOX/Parser.cpp"
//#include "../MDOX/Operaciones.cpp"
//#include "../MDOX/LoadCache.cpp"

Interprete * Interprete::instance;

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

	return true;
}

// Esta función solo será llamada para archivos incompletos, que no contienen una función main de inicio.
// así como para el interprete por consola.
// Se considerará una función propiamente.

void Interprete::Interpretar(Parser * parser)
{

	int local = 0;

	//Parser_Sentencia * inst = new Sentencia_Recursiva()
	std::vector<Parser_Sentencia*> valor;

	std::vector<Variable> variables_funcion;
	parser->isGlobal = false;
	parser->numero_variables_funcion = 0;

	while (local < parser->tokenizer.tokens.size())
	{
		//	std::cout << "--:: " << parser.tokenizer.tokens.at(local) << " :: " << local << " \n";

		Parser_Sentencia* p2 = parser->getSentencia(local, variables_funcion);

		if (p2)
		{
			valor.push_back(p2);
		}
		else
		{
			OutData_Parametros data = OutData_Parametros(parser->tokenizer.token_actual->linea, parser->tokenizer.token_actual->char_horizontal, NULL);
			Errores::generarError(Errores::ERROR_DE_SINTAXIS, &data, parser->tokenizer.token_actual->token);
			//delete p2;
			return;
		}
	}

	Parser_Sentencia * inst = new Sentencia_Recursiva(valor);

	this->variables_globales = new Variable_Runtime[parser->numero_variables_globales];
	Variable_Runtime * variables = new Variable_Runtime[parser->numero_variables_funcion];

	if (!Interprete_Sentencia(inst, variables))
	{
		Errores::generarError(Errores::ERROR_CRITICO, &(inst)->parametros);
		return;
	}

	delete inst; 
	delete[] variables;
}


//Value, Parser_Identificador*, Valor_Funcion*, arbol_operacional*
Value Interprete::lectura_arbol_operacional(arbol_operacional* node, Variable_Runtime* variables)
{
	if (node->operador == OP_NONE)
	{
		return std::visit(overloaded
		{	
			[&](Value & a)->Value {return a; },
			[&](Parser_Identificador * a)->Value
			{ 
				Variable_Runtime * identificador = a->var_global ? &this->variables_globales[a->index] : &variables[a->index];

				if (a->fuerte)
				{
					identificador->value.inicializacion(a->tipo);
					identificador->fuerte = true;
				}

				return identificador->value;
			},
			[&](Valor_Funcion * a)->Value { return ExecFuncion(a->ID->nombre, transformarEntradasFuncion(a, variables)); },
			[&](auto&)->Value { return false; /*Podriamos cambiarlos por monostate*/},
		}, node->_v1);
	}

	if (is_single_operator(node->operador))
	{
		return std::visit(overloaded{
		[&](arbol_operacional * a)->Value { return lectura_arbol_operacional(a, variables); },
		[&](Value & a)->Value {return a.operacion_Unitaria(node->operador); },
		[&](Parser_Identificador * a)->Value 
			{ 
				Value * id =  a->var_global ? &this->variables_globales[a->index].value : &variables[a->index].value;

				//En estos dos casos, el operador, SI cambia directamente la variable.
				if (node->operador == OP_ITR_MIN || node->operador == OP_ITR_PLUS)
				{
					*id = id->operacion_Unitaria(node->operador);
					return id;
				}
				else
				{
					return id->operacion_Unitaria(node->operador);
				}
			},
		[&](Valor_Funcion * a)->Value { return ExecFuncion(a->ID->nombre, transformarEntradasFuncion(a, variables)).operacion_Unitaria(node->operador); },
		[&](auto&)->Value { Errores::generarError(Errores::ERROR_OPERACION_INVALIDA, NULL);  return false; },
			}, node->_v2);
	}
	else if (is_assignment_operator(node->operador))
	{
		return std::visit(overloaded{
	/*	[&](arbol_operacional * a)
		{
				return lectura_arbol_operacional(a,variables).operacion_Asignacion(
				std::visit(overloaded{
				[&](arbol_operacional * a) { return lectura_arbol_operacional(a, variables); },
				[](Value & a) {return a; },
				[&](Parser_Identificador * a) { return a->var_global ? this->variables_globales[a->index].value : variables[a->index].value;  },
				[](auto&) { std::cout << "ERROR.. Funcion"; return Value(); },
					}, node->_v2)
					, node->operador, false); //False por ahora
		},
		[&](Value & a)
		{		 
				return std::visit(overloaded{
				[&](arbol_operacional * a) { return lectura_arbol_operacional(a,variables); },
				[](Value & a) {return a; },
				[&](Parser_Identificador * a) { return a->var_global ? this->variables_globales[a->index].value : variables[a->index].value;  },
				[](auto&) { std::cout << "ERROR.. Funcion"; return Value(); },
					}, node->_v2);
		},*/
		[&](Parser_Identificador * a)
		{
			Variable_Runtime* identificador = a->var_global ? &this->variables_globales[a->index] : &variables[a->index];

			if (a->fuerte)
			{
				identificador->value.inicializacion(a->tipo);
				identificador->fuerte = true;
			}

			identificador->value.operacion_Asignacion(
				std::visit(overloaded{
				[&](arbol_operacional * a)->Value { return lectura_arbol_operacional(a,variables); },
				[](Value & a)->Value {return a; },
				[&](Parser_Identificador * a)->Value 
					{ 
						return a->var_global ? this->variables_globales[a->index].value : variables[a->index].value;  
					},
				[&](Valor_Funcion * a)->Value { return ExecFuncion(a->ID->nombre, transformarEntradasFuncion(a, variables)); },
				[&](auto&)->Value { Errores::generarError(Errores::ERROR_OPERACION_INVALIDA, NULL);  return std::monostate(); },
					}, node->_v2)
					, node->operador, identificador->fuerte); //False por ahora



			return identificador->value;
		},
		[&](Valor_Funcion * a)->Value { return ExecFuncion(a->ID->nombre, transformarEntradasFuncion(a, variables)); },
		
			//Arbol operacional para aceptar casos como: a = b = 5;
		[&](arbol_operacional * a)->Value 
		{
			Value vr = std::visit(overloaded{
			[&](arbol_operacional * a)->Value { return lectura_arbol_operacional(a,variables); },
			[](Value & a)->Value {return a; },
			[&](Parser_Identificador * a)->Value { return a->var_global ? this->variables_globales[a->index].value : variables[a->index].value;  },
			[&](Valor_Funcion * a)->Value { return ExecFuncion(a->ID->nombre, transformarEntradasFuncion(a, variables)); },
			[&](auto&)->Value { Errores::generarError(Errores::ERROR_OPERACION_INVALIDA, NULL);  return std::monostate(); },
				}, node->_v2);

			std::visit(overloaded{
				[&](Parser_Identificador * aID)
					{ 
						Variable_Runtime* ID  = aID->var_global ? &this->variables_globales[aID->index] : &variables[aID->index];
						if (aID->fuerte)
						{
							ID->value.inicializacion(aID->tipo);
							ID->fuerte = true;
						}
						ID->value.operacion_Asignacion(vr, a->operador, ID->fuerte);
					},
				[&](auto&) { },
			}, a->_v2);

			return lectura_arbol_operacional(a, variables);
		
		},
		[&](auto&)->Value {Errores::generarError(Errores::ERROR_OPERACION_INVALIDA, NULL);  return std::monostate(); },
			}, node->_v1);
	 }
	else if (isRelationalOperator(node->operador))
	{
		Value v2 = std::visit(overloaded{
		[&](arbol_operacional * a)->Value { return lectura_arbol_operacional(a,variables); },
		[](Value & a)->Value {return a; },
		[&](Parser_Identificador * a)->Value { return a->var_global ? this->variables_globales[a->index].value : variables[a->index].value;  },
		[&](Valor_Funcion * a)->Value { return ExecFuncion(a->ID->nombre, transformarEntradasFuncion(a, variables)); },
		[&](auto&)->Value { Errores::generarError(Errores::ERROR_OPERACION_INVALIDA_VOID, NULL);  return std::monostate(); },
			}, node->_v2);


			return std::visit(overloaded{
			[&](arbol_operacional * a)->Value 
				{ 
					 if (isRelationalOperator(a->operador))
					 {
						 Value r_back = std::visit(overloaded{
							 [&](arbol_operacional * a)->Value { return lectura_arbol_operacional(a,variables); },
							 [](Value & a)->Value {return a; },
							 [&](Parser_Identificador * a)->Value { return a->var_global ? this->variables_globales[a->index].value : variables[a->index].value;  },
							 [&](Valor_Funcion * a)->Value { return ExecFuncion(a->ID->nombre, transformarEntradasFuncion(a, variables)); },
							 [&](auto&)->Value { Errores::generarError(Errores::ERROR_OPERACION_INVALIDA_VOID, NULL);  return std::monostate(); },
						  }, a->_v2);

						 if (r_back.OperacionRelacional(v2, node->operador))
							return Relacional_rec_arbol(a, variables, r_back);
						 else return false;
					 }
					 else return v2.operacion_Binaria(lectura_arbol_operacional(a, variables), node->operador);
				},
			[&](Value & a)->Value {return v2.operacion_Binaria(a, node->operador); },
			[&](Parser_Identificador * a)->Value { return v2.operacion_Binaria(a->var_global ? this->variables_globales[a->index].value : variables[a->index].value, node->operador);  },
			[&](Valor_Funcion * a)->Value { return v2.operacion_Binaria(ExecFuncion(a->ID->nombre, transformarEntradasFuncion(a, variables)), node->operador); },
			[&](auto&)->Value { Errores::generarError(Errores::ERROR_OPERACION_INVALIDA_VOID, NULL);  return std::monostate(); },
				}, node->_v1);

			//return v2.operacion_Binaria(, node->operador);

	}
	else
	{
		Value v2 = std::visit(overloaded{
		[&](arbol_operacional * a)->Value { return lectura_arbol_operacional(a,variables); },
		[](Value & a)->Value {return a; },
		[&](Parser_Identificador * a)->Value { return a->var_global ? this->variables_globales[a->index].value : variables[a->index].value;  },
		[&](Valor_Funcion * a)->Value { return ExecFuncion(a->ID->nombre, transformarEntradasFuncion(a, variables)); },
		[&](auto&)->Value { Errores::generarError(Errores::ERROR_OPERACION_INVALIDA_VOID, NULL);  return std::monostate(); },
			}, node->_v1);

		return v2.operacion_Binaria(
			std::visit(overloaded{
			[&](arbol_operacional * a)->Value { return lectura_arbol_operacional(a, variables); },
			[](Value & a)->Value {return a; },
			[&](Parser_Identificador * a)->Value { return a->var_global ? this->variables_globales[a->index].value : variables[a->index].value;  },
			[&](Valor_Funcion * a)->Value { return ExecFuncion(a->ID->nombre, transformarEntradasFuncion(a, variables)); },
			[&](auto&)->Value { Errores::generarError(Errores::ERROR_OPERACION_INVALIDA_VOID, NULL);  return std::monostate(); },
				}, node->_v2)
				, node->operador);
	}
}

bool Interprete::Relacional_rec_arbol(arbol_operacional* node, Variable_Runtime* variables, Value& v2)
{
	return std::visit(overloaded{
	[&](arbol_operacional * a)
		{
			 if (isRelationalOperator(a->operador))
			 {
				 Value r_back = std::visit(overloaded{
					 [&](arbol_operacional * a)->Value { return lectura_arbol_operacional(a,variables); },
					 [](Value & a)->Value {return a; },
					 [&](Parser_Identificador * a)->Value { return a->var_global ? this->variables_globales[a->index].value : variables[a->index].value;  },
					 [&](Valor_Funcion * a)->Value { return ExecFuncion(a->ID->nombre, transformarEntradasFuncion(a, variables)); },
					 [&](auto&)->Value { Errores::generarError(Errores::ERROR_OPERACION_INVALIDA_VOID, NULL);  return std::monostate(); },
				  }, a->_v2);

				 if (r_back.OperacionRelacional(v2, node->operador))
					return Relacional_rec_arbol(a, variables, r_back);
				 else return false;
			 }
			 else
			 {
				return lectura_arbol_operacional(a, variables).OperacionRelacional(v2, node->operador);
			 }
		},
	[&](Value & a) {return a.OperacionRelacional(v2, node->operador); },
	[&](Parser_Identificador * a) { Value ValID = a->var_global ? this->variables_globales[a->index].value : variables[a->index].value; return ValID.OperacionRelacional(v2, node->operador); },
	[&](Valor_Funcion * a) {  Value ValID = ExecFuncion(a->ID->nombre, transformarEntradasFuncion(a, variables)); return ValID.OperacionRelacional(v2, node->operador); },
	[&](auto&) { Errores::generarError(Errores::ERROR_OPERACION_INVALIDA_VOID, NULL);  return false },
		}, node->_v1);
}

std::vector<Value> Interprete::transformarEntradasFuncion(Valor_Funcion* vF, Variable_Runtime* variables)
{
	std::vector<Value> entradas(vF->entradas.size());



	//entradas.reserve(vF->entradas.size());

	int k = 0;
	for (std::vector<Parser_Operacion*>::iterator entrada = vF->entradas.begin(); entrada != vF->entradas.end(); ++entrada)
	{		
		entradas[k] = lectura_arbol_operacional((*entrada)->val, variables);
		k++;
	}
	return entradas;
}

bool Interprete::Interprete_Sentencia(Parser_Sentencia * sentencia, Variable_Runtime* variables)
{
	switch (sentencia->tipo)
	{
		case SENT_REC:
		{
			Sentencia_Recursiva * x = static_cast<Sentencia_Recursiva*>(sentencia);

			for (std::vector<Parser_Sentencia*>::iterator it = x->valor.begin(); it != x->valor.end(); ++it)
			{
				if (!Interprete_Sentencia(*it, variables))
				{				
					//Fallo inesperado, no debería ocurrir nunca si se han filtrado correctamente los errores.
					Errores::generarError(Errores::ERROR_CRITICO, &(*it)->parametros);
					return false;
				}
			
				if (return_activo)
					break;
				
			}
			return true;
		}

		// Add+1 -> También se podrá llamar a operaciones matemáticas, pero su resultado nunca se guardará. 
		//  Esto se debe a que se puede ejecutar directamente funciones aún sin ser return void.
		case SENT_OP:
		{
			Sentencia_Operacional * x = static_cast<Sentencia_Operacional*>(sentencia);
			lectura_arbol_operacional(x->pOp->val, variables);
			return true;
		}
		// FUNCION SALIDA DATOS :
		// Permite escribir por consola un valor dado.
		case SENT_PRINT:
		{
			Sentencia_Print * x = static_cast<Sentencia_Print*>(sentencia);
			Value v = lectura_arbol_operacional(x->pOp->val, variables);

			v.print();
			return true;
		}
		// Permite escribir por consola un valor dado.
		case SENT_RETURN:
		{
			Sentencia_Return * x = static_cast<Sentencia_Return*>(sentencia);
			if (x->pOp) 
			{
				setRetorno(lectura_arbol_operacional(x->pOp->val, variables));
			}
			else retornoSinValor();

			return true;
		}
		// FUNCION IF:
		// Permite la comprobacion de operaciones o variables.
		case SENT_IF:
		{
			Sentencia_IF * x = static_cast<Sentencia_IF*>(sentencia);
			
			Value b = lectura_arbol_operacional(x->pCond->val, variables);

			if (b.ValueToBool())
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
				Value b = lectura_arbol_operacional(x->pCond->val, variables);

				if (b.ValueToBool())
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
				lectura_arbol_operacional(x->pIguald->val, variables);
			}

			while (true)
			{
				bool res = true;

				if (x->pCond)
				{
					res = lectura_arbol_operacional(x->pCond->val, variables).ValueToBool();
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
						lectura_arbol_operacional(x->pOp->val, variables);
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

//TODO: En un futuro hay que cambiar el estilo de busqueda de funciones por un método directo de cacheado.
bool Interprete::FuncionCore(std::string name, std::vector<Value> entradas)
{
	for (std::vector<Core_Function*>::iterator funcion = Core::core_functions.begin(); funcion != Core::core_functions.end(); ++funcion)
	{
		if ((*funcion)->nombre == name)
		{
			if ((*funcion)->entradas.size() != entradas.size())
				continue;
			
			//for (unsigned ent_itr = 0; ent_itr < xFunc->entradas.size(); ent_itr++)
			Core_Function_Interprete* funcionInterprete = static_cast<Core_Function_Interprete*>(*funcion);

			//std::vector<Value>&, Interprete*, OutData_Parametros*);
			if (funcionInterprete->funcion_exec(entradas, this))
			{
				return true;
			}
		}
	}
	return false;
}


// Ejecuta la función dentro del ENTORNO. Es decir, se trata de una función que NO está fuera del entorno de llamada. (Es decir, no forma parte de una clase diferente)
// Las variables a las cuales tiene acceso esta función, serán las variables del entorno propio, es decir VARIABLES GLOBALES, variables declaradas a nivel de main.
Value Interprete::ExecFuncion(std::string name, std::vector<Value> entradas)
{
	//xFunc->entradas

	/*int k = 0;
	for (std::vector<Parser_Operacion*>::iterato r it = xFunc->entradas.begin(); it != xFunc->entradas.end(); ++it)
	{
		valores_entrada[k] = lectura_arbol_operacional()
		k++;
	}*/

	// ------------ Tipos de Funciones  ------------
	// **** Comenzamos probando si se trata de una función del core del lenguaje.
	Errores::saltarErrores = true;
	if (this->FuncionCore(name, entradas))
	{
		if (this->returnCalled())
		{
			Errores::saltarErrores = false;
			return this->getRetorno();
		}
		Errores::saltarErrores = false;
		return std::monostate();
	}


	// **** EN EL CASO de que no se encontrara ninguna función core, vamos con las normales:


	//De entre todas las funciones declaradas, buscamos la que realmente se está llamando.

	for (std::vector<Parser_Funcion*>::iterator funcion = funciones.begin(); funcion != funciones.end(); ++funcion)
	{
		//Debe coincidir el nombre de la misma.
		if ((*funcion)->pID->nombre == name)
		{
			//si no tiene el mismo numero de entradas, saltamos, no es esta función.
			if ((*funcion)->entradas.size() != entradas.size())
				continue;

			bool correcto = true; // Si es FALSE se limpiará el heap de las variables creadas, pero no ejecutará la función.
			bool forzar_salida = false; //Se devolverá NULL


			//Value identificador = a->var_global ? this->variables_globales[a->index].value : variables[a->index].value;

			Variable_Runtime* variables = NULL;

			if((*funcion)->preload_var > 0)
				variables = new Variable_Runtime[(*funcion)->preload_var];

			bool entradas_incorrectas = false;

			int ent_itr = 0;
			for (std::vector<Parser_Operacion*>::iterator it = (*funcion)->entradas.begin(); it != (*funcion)->entradas.end(); ++it)
			{
				if ((*it)->val->operador == OP_NONE)
				{
					if (!std::visit(overloaded
						{
							[&](Value & a) { return a.igualdad_Condicional(entradas[ent_itr]); },
							[&](Parser_Identificador * a)
							{
								Variable_Runtime* identificador = a->var_global ? &this->variables_globales[a->index] : &variables[a->index];

								if (a->fuerte)
								{
									identificador->value.inicializacion(a->tipo);
									identificador->fuerte = true;
								}
								return identificador->value.asignacion(entradas[ent_itr], identificador->fuerte);
							},
							[&](Valor_Funcion * a) { return entradas[ent_itr].igualdad_Condicional(ExecFuncion(a->ID->nombre, transformarEntradasFuncion(a,variables))); },
							[&](auto&) { return false; },
						}, (*it)->val->_v1))
					{
						entradas_incorrectas = true;
						break;
					}
				}
				else
				{
					if (!lectura_arbol_operacional((*it)->val, variables).igualdad_Condicional(entradas[ent_itr]))
					{
						entradas_incorrectas = true;
						break;
					}
				}
				ent_itr++;
			}

			//No se trata de la funcion correcta
			if (entradas_incorrectas)
			{
				continue;
			}

			Errores::saltarErrores = false;

			if (Interprete_Sentencia((*funcion)->body, variables))
			{
				Errores::saltarErrores = false;
				delete[] variables;

				//Si la salida es NULA es que no se ha especificado una, la salida será igual al valor de retorno.
				if ((*funcion)->salida == NULL)
				{
					if (this->returnCalled())
					{

						return this->getRetorno();
					}
					else
					{
						 return std::monostate(); 
					}
				}
				else
				{
					if (this->returnCalled())
					{
						Value retorno = this->getRetorno();
						return retorno.Cast((*funcion)->salida);
					}
					else
					{
						if ((*funcion)->salida->_tipo == DEC_SINGLE)
						{
							Declarativo_SingleValue* x = static_cast<Declarativo_SingleValue*>((*funcion)->salida);
							if(x->value != PARAM_VOID)
								Errores::generarWarning(Errores::WARNING_FUNCION_VALOR_DEVUELTO_VOID, &(*funcion)->parametros, (*funcion)->pID->nombre);
						}
						
						return  std::monostate();
					}


				}

			}
			else
			{
				delete[] variables;
				return  std::monostate();
			}
		}
	}

	Errores::saltarErrores = false;
	Errores::generarError(Errores::ERROR_FUNCION_NO_RECONOCIDA, NULL, name);
	return std::monostate(); 
}
/*
bool Interprete::EstablecerVariable(VariablePreloaded * var, Value ** value, OutData_Parametros * outData)
{

	if (!var->fuerte)
	{
		deletePtr(var->valor);

		if ((*value)->vr)
			var->valor = (*value)->Clone();
		else var->valor = (*value);

		var->valor->vr = true;
		return true;
	}

	bool b;

	if ((*value)->vr)
	{
		Value * v = (*value)->Clone();
		b = ValueConversion(&(var->valor), &v, outData);
	}
	else b = ValueConversion(&(var->valor), value, outData);

	var->valor->vr = true;

	return b;
}
*/

