
#include "Interprete.h"
//#include "../MDOX/Errores.cpp"
//#include "../MDOX/Parser.cpp"
//#include "../MDOX/Operaciones.cpp"
//#include "../MDOX/LoadCache.cpp"

Interprete* Interprete::instance;

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
	std::vector<arbol_operacional*> var_globales;

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


		parser->isGlobal = true;
		std::vector<Variable> variables;
		arbol_operacional* p3 = parser->getOperacion(local, variables);
		parser->isGlobal = false;

		if (p3)
		{
			if (parser->tokenizer.getTokenValue(local) == ";")
			{
				var_globales.push_back(p3);
				continue;
			}
		}


		OutData_Parametros data = OutData_Parametros(parser->tokenizer.token_actual->linea, parser->tokenizer.token_actual->char_horizontal, this->nombre_ficheros.back());
		Errores::generarError(Errores::ERROR_DE_SINTAXIS, &data, parser->tokenizer.token_actual->token);
		return false;

	}

	this->variables_globales = new Variable_Runtime[parser->numero_variables_globales];

	for (std::vector<arbol_operacional*>::iterator it = var_globales.begin(); it != var_globales.end(); ++it)
	{
		lectura_arbol_operacional((*it), NULL);
	}

	return true;
}

// Esta función solo será llamada para archivos incompletos, que no contienen una función main de inicio.
// así como para el interprete por consola.
// Se considerará una función propiamente.

void Interprete::Interpretar(Parser* parser)
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

	Parser_Sentencia* inst = new Sentencia_Recursiva(valor);

	Variable_Runtime* variables = new Variable_Runtime[parser->numero_variables_funcion];

	if (!Interprete_Sentencia(inst, variables))
	{
		Errores::generarError(Errores::ERROR_CRITICO, &(inst)->parametros);
		return;
	}

	delete inst;
	delete[] variables;
}

Value Interprete::TratarMultiplesValores(multi_value* arr, Variable_Runtime* variables)
{
	if (arr->is_vector)
	{
		std::shared_ptr<mdox_vector> res = std::make_shared<mdox_vector>();
		res->vector.reserve(arr->arr.size());

		//int itr = 0;
		for (std::vector<tipoValor>::iterator it = arr->arr.begin(); it != arr->arr.end(); ++it)
		{
			res->vector.emplace_back(std::visit(overloaded
				{
					[&](Value & a)->Value {return a; },
					[&](arbol_operacional * a)->Value { return lectura_arbol_operacional(a, variables);  },
					[&](Valor_Funcion * a)->Value { return ExecFuncion(a->ID->nombre, transformarEntradasFuncion(a, variables)); },
					[&](Parser_Identificador * a)->Value { return a->var_global ? this->variables_globales[a->index].value : variables[a->index].value;  },
					[&](auto&)->Value {  Errores::generarError(Errores::ERROR_OPERACION_INVALIDA, NULL); return std::monostate(); },
				}, *it));
		}
		return std::move(res);
	}
	else if (arr->contenedor)
	{
		// Un contenedor obtiene una sucesión de operaciones con operadores de vectores.
		//Por lo tanto siempre se tratará de un "OperadorEnVector"
		if (!OperacionOperadoresVectores(arr->operacionesVector->v1, arr->operacionesVector->v2, arr->operacionesVector->operador1, variables))
			return false;
		
		if (arr->operacionesVector->dobleOperador)
		{
			if (!OperacionOperadoresVectores(arr->operacionesVector->v2, arr->operacionesVector->v3, arr->operacionesVector->operador2, variables))
				return false;
		}
	
		return true;
	}
	else //Se tratan de operaciones del estilo:  a=2, b=3, c=2;
	{
		for (std::vector<tipoValor>::iterator it = arr->arr.begin(); it != arr->arr.end(); ++it)
		{
			std::visit(overloaded
				{
					[&](arbol_operacional * a) { lectura_arbol_operacional(a, variables);  },
					[&](Valor_Funcion * a) { ExecFuncion(a->ID->nombre, transformarEntradasFuncion(a, variables)); },
					[&](Parser_Identificador * a)
					{
						Variable_Runtime* identificador = a->var_global ? &this->variables_globales[a->index] : &variables[a->index];

						if (a->fuerte)
						{
							identificador->value.inicializacion(a->tipo);
							identificador->fuerte = true;
						}
					},
					[&](auto&) {},
				}, *it);
		}
		return std::monostate();
	}
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
					Variable_Runtime* identificador = a->var_global ? &this->variables_globales[a->index] : &variables[a->index];

					if (a->fuerte)
					{
						identificador->value.inicializacion(a->tipo);
						identificador->fuerte = true;
					}

					return identificador->value;
				},
				[&](Valor_Funcion * a)->Value { return ExecFuncion(a->ID->nombre, transformarEntradasFuncion(a, variables)); },
				[&](multi_value * a)->Value 
				{ 
						return TratarMultiplesValores(a, variables);  
				},
				[&](auto&)->Value { return std::monostate(); },
			}, node->_v1);
	}

	if (is_single_operator(node->operador))
	{
		return std::visit(overloaded{
		[&](arbol_operacional * a)->Value { return lectura_arbol_operacional(a, variables).operacion_Unitaria(node->operador); },
		[&](Value & a)->Value {return a.operacion_Unitaria(node->operador); },
		[&](Parser_Identificador * a)->Value
			{
				Value* id = a->var_global ? &this->variables_globales[a->index].value : &variables[a->index].value;

				//En estos dos casos, el operador, SI cambia directamente la variable.
				if (node->operador == OP_ITR_MIN || node->operador == OP_ITR_PLUS)
				{
					*id = id->operacion_Unitaria(node->operador);
					return *id;
				}
				else
				{
					return id->operacion_Unitaria(node->operador);
				}
			},
		[&](Valor_Funcion * a)->Value { return ExecFuncion(a->ID->nombre, transformarEntradasFuncion(a, variables)).operacion_Unitaria(node->operador); },
		[&](multi_value * a)->Value { return TratarMultiplesValores(a, variables).operacion_Unitaria(node->operador);  },
		[&](auto&)->Value { Errores::generarError(Errores::ERROR_OPERACION_INVALIDA, NULL);  return false; },
			}, node->_v2);
	}
	else if (is_assignment_operator(node->operador))
	{
		return std::visit(overloaded{
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
				[&](multi_value * a)->Value {  return TratarMultiplesValores(a, variables); },
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
			[&](multi_value * a)->Value {  return TratarMultiplesValores(a, variables); },
			[&](auto&)->Value { Errores::generarError(Errores::ERROR_OPERACION_INVALIDA, NULL);  return std::monostate(); },
				}, node->_v2);

			std::visit(overloaded{
				[&](Parser_Identificador * aID)
					{
						Variable_Runtime* ID = aID->var_global ? &this->variables_globales[aID->index] : &variables[aID->index];
						if (aID->fuerte)
						{
							ID->value.inicializacion(aID->tipo);
							ID->fuerte = true;
						}
						ID->value.operacion_Asignacion(vr, a->operador, ID->fuerte);
					},
				[&](auto&) {},
			}, a->_v2);

			return lectura_arbol_operacional(a, variables);

		},
		[&](multi_value * a)->Value
		{
			 return TratarMultiplesValores(a, variables);
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
		[&](multi_value * a)->Value {  return TratarMultiplesValores(a, variables); },
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
						 [&](multi_value * a)->Value {  return TratarMultiplesValores(a, variables); },
						 [&](auto&)->Value { Errores::generarError(Errores::ERROR_OPERACION_INVALIDA_VOID, NULL);  return std::monostate(); },
					  }, a->_v2);

					 if (r_back.OperacionRelacional(v2, node->operador))
						return Relacional_rec_arbol(a, variables, r_back);
					 else return false;
				 }
				 else return lectura_arbol_operacional(a, variables).OperacionRelacional(v2, node->operador);
			},
		[&](Value & a)->Value
			{
				return a.OperacionRelacional(v2, node->operador);
			},
		[&](Parser_Identificador * a)->Value { return (a->var_global ? this->variables_globales[a->index].value : variables[a->index].value).OperacionRelacional(v2, node->operador);  },
		[&](Valor_Funcion * a)->Value { return ExecFuncion(a->ID->nombre, transformarEntradasFuncion(a, variables)).OperacionRelacional(v2, node->operador); },
		[&](multi_value * a)->Value {  return TratarMultiplesValores(a, variables).OperacionRelacional(v2, node->operador); },
		[&](auto&)->Value { Errores::generarError(Errores::ERROR_OPERACION_INVALIDA_VOID, NULL);  return std::monostate(); },
			}, node->_v1);

		//return v2.operacion_Binaria(, node->operador);

	}
	else
	{
		return std::visit(overloaded{
		[&](arbol_operacional * a)->Value { return lectura_arbol_operacional(a,variables); },
		[](Value & a)->Value {return a; },
		[&](Parser_Identificador * a)->Value { return a->var_global ? this->variables_globales[a->index].value : variables[a->index].value;  },
		[&](Valor_Funcion * a)->Value { return ExecFuncion(a->ID->nombre, transformarEntradasFuncion(a, variables)); },
		[&](multi_value * a)->Value {  return TratarMultiplesValores(a, variables); },
		[&](auto&)->Value { Errores::generarError(Errores::ERROR_OPERACION_INVALIDA_VOID, NULL);  return std::monostate(); },
			}, node->_v1).operacion_Binaria(
				std::visit(overloaded{
				[&](arbol_operacional * a)->Value { return lectura_arbol_operacional(a, variables); },
				[](Value & a)->Value {return a; },
				[&](Parser_Identificador * a)->Value { return a->var_global ? this->variables_globales[a->index].value : variables[a->index].value;  },
				[&](Valor_Funcion * a)->Value { return ExecFuncion(a->ID->nombre, transformarEntradasFuncion(a, variables)); },
				[&](multi_value * a)->Value {  return TratarMultiplesValores(a, variables); },
				[&](auto&)->Value { Errores::generarError(Errores::ERROR_OPERACION_INVALIDA_VOID, NULL);  return std::monostate(); },
					}, node->_v2)
					, node->operador);
	}
}


//Devuelve el Identificador en caso de que lo sea.
//RET es por defecto NULO.
ValueCopyOrRef Interprete::tipoValorToValueOrRef(tipoValor& a, Variable_Runtime* variables, Parser_Identificador** ret)
{
 	return std::visit(overloaded{
		[&](arbol_operacional * a)->ValueCopyOrRef { return lectura_arbol_operacional(a, variables); },
		[](Value & a)->ValueCopyOrRef {return a; },
		[&](Parser_Identificador * a)->ValueCopyOrRef 
		{ 
			if (a->fuerte)
			{
				Variable_Runtime* vr = a->var_global ? &this->variables_globales[a->index] : &variables[a->index];
				vr->fuerte = true;

				//if (ret)
					*ret = a;

				return &vr->value;
			}

			//if (ret)
				*ret = a;

			return a->var_global ? &this->variables_globales[a->index].value : &variables[a->index].value;
			
		},
		[&](Valor_Funcion * a)->ValueCopyOrRef { return ExecFuncion(a->ID->nombre, transformarEntradasFuncion(a, variables)); },
		[&](multi_value * a)->ValueCopyOrRef {  return TratarMultiplesValores(a, variables); },
		[&](auto&)->ValueCopyOrRef { return std::monostate(); },
		}, a);
}

bool Interprete::OperacionOperadoresVectores(tipoValor& v1, tipoValor& v2, OPERADORES& operador, Variable_Runtime* variables)
{
	Parser_Identificador* f1 = NULL;  Parser_Identificador* f2 = NULL;
	//Value, Parser_Identificador*, Valor_Funcion*, arbol_operacional*, multi_value* 
	return std::visit(overloaded{
		//	[&](Value& a, Value& b)->bool { return a.OperadoresEspeciales(b, node->operador); },
		//	[&](Value& a, Parser_Identificador * b)->bool { return a.OperadoresEspeciales((b->var_global ? this->variables_globales[b->index].value : variables[b->index].value), node->operador);  },
		//	[&](Value& a, Valor_Funcion * b)->bool { return  a.OperadoresEspeciales(ExecFuncion(b->ID->nombre, transformarEntradasFuncion(b, variables)), node->operador); },
		//	[&](Value& a, arbol_operacional * b)->bool { return  a.OperadoresEspeciales(lectura_arbol_operacional(b, variables), node->operador); },
			
		[&](Value & a, multi_value* b)->bool
		{ 
			if (operador == OP_CHECK_GET)
			{
				if (b->is_vector)
				{
					return a.OperadoresEspeciales_Check(TratarMultiplesValores(b,variables), -1);
				}

				//Dado que b no es un vector, el vector debe ser, en todo caso, a, de no serlo, la operación devolverá un error a posteriori.
				//Igualmente, trabajaremos considerando A-> vector, y dejamos el error al check del operador.

				//Esto implica que los valores se checkean desde atrás->   [vector::a,b,c]
				int itr = 0;
				for (std::vector<tipoValor>::iterator it = b->arr.begin(); it != b->arr.end(); ++it)
				{
					if (!a.OperadoresEspeciales_Check(*tipoValorToValueOrRef(*it, variables, &f2).ref, itr, NULL, f2))
					{
						return false;
					}
					itr++;
				}
			}
			else // POP
			{
				if (b->is_vector)
				{
					return a.OperadoresEspeciales_Pop(TratarMultiplesValores(b,variables));
				}

				for (std::vector<tipoValor>::iterator it = b->arr.begin(); it != b->arr.end(); ++it)
				{
					if (!a.OperadoresEspeciales_Pop(*tipoValorToValueOrRef(*it, variables, &f2).ref,NULL,f2))
					{
						return false;
					}
				}
			}
			return true;
		},

		[&](Parser_Identificador * a, multi_value * b)->bool
		{
			Value * _a = a->var_global ? &this->variables_globales[a->index].value : &variables[a->index].value;

			if (operador == OP_CHECK_GET)
			{
				if (b->is_vector)
				{
					return _a->OperadoresEspeciales_Check(TratarMultiplesValores(b,variables), -1, a);
				}

				//Dado que b no es un vector, el vector debe ser, en todo caso, a, de no serlo, la operación devolverá un error a posteriori.
				//Igualmente, trabajaremos considerando A-> vector, y dejamos el error al check del operador.

				//Esto implica que los valores se checkean desde atrás->   [vector::a,b,c]
				int itr = 0;
				for (std::vector<tipoValor>::iterator it = b->arr.begin(); it != b->arr.end(); ++it)
				{
					if (!_a->OperadoresEspeciales_Check(*tipoValorToValueOrRef(*it, variables, &f2).ref, itr, a,f2))
					{
						return false;
					}
					itr++;
				}
			}
			else // POP
			{
				if (b->is_vector)
				{
					return _a->OperadoresEspeciales_Pop(TratarMultiplesValores(b,variables), a);
				}

				for (std::vector<tipoValor>::iterator it = b->arr.begin(); it != b->arr.end(); ++it)
				{
					if (!_a->OperadoresEspeciales_Pop(*tipoValorToValueOrRef(*it, variables, &f2).ref, a,f2))
					{
						return false;
					}
				}
			}
			return true;
		},

		[&](Valor_Funcion * a, multi_value * b)->bool
		{
			Value _a = ExecFuncion(a->ID->nombre, transformarEntradasFuncion(a, variables));

			if (operador == OP_CHECK_GET)
			{
				if (b->is_vector)
				{
					return _a.OperadoresEspeciales_Check(TratarMultiplesValores(b,variables), -1);
				}

				//Dado que b no es un vector, el vector debe ser, en todo caso, a, de no serlo, la operación devolverá un error a posteriori.
				//Igualmente, trabajaremos considerando A-> vector, y dejamos el error al check del operador.

				//Esto implica que los valores se checkean desde atrás->   [vector::a,b,c]
				int itr = 0;
				for (std::vector<tipoValor>::iterator it = b->arr.begin(); it != b->arr.end(); ++it)
				{
					if (!_a.OperadoresEspeciales_Check(*tipoValorToValueOrRef(*it, variables, &f2).ref, itr, NULL, f2))
					{
						return false;
					}
					itr++;
				}
			}
			else // POP
			{
				if (b->is_vector)
				{
					return _a.OperadoresEspeciales_Pop(TratarMultiplesValores(b,variables));
				}

				for (std::vector<tipoValor>::iterator it = b->arr.begin(); it != b->arr.end(); ++it)
				{
					if (!_a.OperadoresEspeciales_Pop(*tipoValorToValueOrRef(*it, variables,&f2).ref, NULL, f2))
					{
						return false;
					}
				}
			}
			return true;
		},

		[&](arbol_operacional* a, multi_value* b)->bool
		{
			Value _a = lectura_arbol_operacional(a, variables);

			if (operador == OP_CHECK_GET)
			{
				if (b->is_vector)
				{
					return _a.OperadoresEspeciales_Check(TratarMultiplesValores(b, variables), -1);
				}

				//Dado que b no es un vector, el vector debe ser, en todo caso, a, de no serlo, la operación devolverá un error a posteriori.
				//Igualmente, trabajaremos considerando A-> vector, y dejamos el error al check del operador.

				//Esto implica que los valores se checkean desde atrás->   [vector::a,b,c]
				int itr = 0;
				for (std::vector<tipoValor>::iterator it = b->arr.begin(); it != b->arr.end(); ++it)
				{
					if (!_a.OperadoresEspeciales_Check(*tipoValorToValueOrRef(*it, variables, &f2).ref, itr, NULL,f2))
					{
						return false;
					}
					itr++;
				}
			}
			else // POP
			{
				if (b->is_vector)
				{
					return _a.OperadoresEspeciales_Pop(TratarMultiplesValores(b, variables));
				}

				for (std::vector<tipoValor>::iterator it = b->arr.begin(); it != b->arr.end(); ++it)
				{
					if (!_a.OperadoresEspeciales_Pop(*tipoValorToValueOrRef(*it, variables, &f2).ref, NULL,f2))
					{
						return false;
					}
				}
			}
			return true;
		},


		[&](multi_value* a, Value& b)->bool
		{
			if (operador == OP_CHECK_GET)
			{
				if (a->is_vector)
				{
					return TratarMultiplesValores(a, variables).OperadoresEspeciales_Check(b, -1);
				}

				//Dado que a no es un vector, el vector debe ser, en todo caso, b, de no serlo, la operación devolverá un error a posteriori.
				//Igualmente, trabajaremos considerando B-> vector, y dejamos el error al check del operador.

				//Esto implica que los valores se checkean desde atrás->   [a,b,c::vector]
				int itr = 0;
				for (std::vector<tipoValor>::iterator it = a->arr.begin(); it != a->arr.end(); ++it)
				{					
					 if (!tipoValorToValueOrRef(*it, variables, &f1).ref->OperadoresEspeciales_Check(b, itr,f1))
						 return false;
					itr++;
				}
			}
			else // POP
			{
				if (a->is_vector)
				{
					return TratarMultiplesValores(a, variables).OperadoresEspeciales_Pop(b);
				}
				
				for (std::vector<tipoValor>::iterator it = a->arr.begin(); it != a->arr.end(); ++it)
				{
					if (!tipoValorToValueOrRef(*it, variables, &f1).ref->OperadoresEspeciales_Pop(b,f1))
						return false;
				}
			}
			return true;
		},

		[&](multi_value* a, Parser_Identificador* b)->bool
		{
			Value* _p = b->var_global ? &this->variables_globales[b->index].value : &variables[b->index].value;

			if (operador == OP_CHECK_GET)
			{
				if (a->is_vector)
				{
					return TratarMultiplesValores(a, variables).OperadoresEspeciales_Check(*_p, -1, b);
				}

				//Dado que a no es un vector, el vector debe ser, en todo caso, b, de no serlo, la operación devolverá un error a posteriori.
				//Igualmente, trabajaremos considerando B-> vector, y dejamos el error al check del operador.

				//Esto implica que los valores se checkean desde atrás->   [a,b,c::vector]
				int itr = 0;
				for (std::vector<tipoValor>::iterator it = a->arr.begin(); it != a->arr.end(); ++it)
				{
					if (!tipoValorToValueOrRef(*it, variables, &f1).ref->OperadoresEspeciales_Check(*_p, itr, f1, b))
						return false; 
					itr++;
				}
			}
			else // POP
			{
				if (a->is_vector)
				{
					return TratarMultiplesValores(a, variables).OperadoresEspeciales_Pop(*_p, b);
				}

				for (std::vector<tipoValor>::iterator it = a->arr.begin(); it != a->arr.end(); ++it)
				{
					if (!tipoValorToValueOrRef(*it, variables, &f1).ref->OperadoresEspeciales_Pop(*_p, f1, b))
						return false;
				}
			}
			return true;
		},

		[&](multi_value* a, Valor_Funcion* b)->bool
		{
			Value _p = ExecFuncion(b->ID->nombre, transformarEntradasFuncion(b, variables));

			if (operador == OP_CHECK_GET)
			{
				if (a->is_vector)
				{
					return TratarMultiplesValores(a, variables).OperadoresEspeciales_Check(_p, -1);
				}

				//Dado que a no es un vector, el vector debe ser, en todo caso, b, de no serlo, la operación devolverá un error a posteriori.
				//Igualmente, trabajaremos considerando B-> vector, y dejamos el error al check del operador.

				//Esto implica que los valores se checkean desde atrás->   [a,b,c::vector]
				int itr = 0;
				for (std::vector<tipoValor>::iterator it = a->arr.begin(); it != a->arr.end(); ++it)
				{
					if (!tipoValorToValueOrRef(*it, variables, &f1).ref->OperadoresEspeciales_Check(_p, itr,f1))
						return false;
					itr++;
				}
			}
			else // POP
			{
				if (a->is_vector)
				{
					return TratarMultiplesValores(a, variables).OperadoresEspeciales_Pop(_p);
				}

				for (std::vector<tipoValor>::iterator it = a->arr.begin(); it != a->arr.end(); ++it)
				{
					if (!tipoValorToValueOrRef(*it, variables, &f1).ref->OperadoresEspeciales_Pop(_p,f1))
						return false;
				}
			}
			return true;
		},

		[&](multi_value* a, arbol_operacional* b)->bool
		{
			Value _p = lectura_arbol_operacional(b, variables);

			if (operador == OP_CHECK_GET)
			{
				if (a->is_vector)
				{
					return TratarMultiplesValores(a, variables).OperadoresEspeciales_Check(_p, -1);
				}

				//Dado que a no es un vector, el vector debe ser, en todo caso, b, de no serlo, la operación devolverá un error a posteriori.
				//Igualmente, trabajaremos considerando B-> vector, y dejamos el error al check del operador.

				//Esto implica que los valores se checkean desde atrás->   [a,b,c::vector]
				int itr = 0;
				for (std::vector<tipoValor>::iterator it = a->arr.begin(); it != a->arr.end(); ++it)
				{
					if (!tipoValorToValueOrRef(*it, variables, &f1).ref->OperadoresEspeciales_Check(_p, itr,f1))
						return false;
					itr++;
				}
			}
			else // POP
			{
				if (a->is_vector)
				{
					return TratarMultiplesValores(a, variables).OperadoresEspeciales_Pop(_p);
				}

				for (std::vector<tipoValor>::iterator it = a->arr.begin(); it != a->arr.end(); ++it)
				{
					 if (!tipoValorToValueOrRef(*it, variables, &f1).ref->OperadoresEspeciales_Pop(_p,f1))
						 return false;
				}
			}
			return true;
		},

		//Se trata de un caso especial, primero habría que descubrir cual de los dos es el vector, después actuar en consecuencia.
		[&](multi_value* a, multi_value* b)->bool
		{
			if (operador == OP_CHECK_GET)
			{

				if (a->is_vector)
				{
					//Si tanto A como B son vectores, se tratan ambos, se opera con ellos y se devuelve el resultado.
					if (b->is_vector)
						return TratarMultiplesValores(a, variables).OperadoresEspeciales_Check(TratarMultiplesValores(b, variables), -1);
					else //Si A es vector y b no, implica  vectorA::a,b,c
					{
						int itr = 0;
						for (std::vector<tipoValor>::iterator it = b->arr.begin(); it != b->arr.end(); ++it)
						{
								if (!TratarMultiplesValores(a, variables).OperadoresEspeciales_Check(*tipoValorToValueOrRef(*it, variables, &f2).ref, itr, NULL, f2))
									return false;
							itr++;
						}
					}
				}
				else // Es decir, A NO es un vector.
				{
					if (b->is_vector) //Si B es un vector se trata de un patrón del estilo a,b,c::vectorB
					{
						Value vectorB = TratarMultiplesValores(a, variables);
						int itr = 0;
						for (std::vector<tipoValor>::iterator it = a->arr.begin(); it != a->arr.end(); ++it)
						{
							if (!tipoValorToValueOrRef(*it, variables, &f1).ref->OperadoresEspeciales_Check(vectorB, itr, f1))
								return false;
							itr++;
						}
					}
				}
			}
			else // Hacemos lo mismo para POP
			{
				if (a->is_vector)
				{
					//Si tanto A como B son vectores, se tratan ambos, se opera con ellos y se devuelve el resultado.
					if (b->is_vector)
						return TratarMultiplesValores(a, variables).OperadoresEspeciales_Pop(TratarMultiplesValores(b, variables));
					else //Si A es vector y b no, implica  vectorA::a,b,c
					{
						for (std::vector<tipoValor>::iterator it = b->arr.begin(); it != b->arr.end(); ++it)
						{
							f2 = false;
							if (!TratarMultiplesValores(a, variables).OperadoresEspeciales_Pop(*tipoValorToValueOrRef(*it, variables, &f2).ref, NULL, f2))
								return false;
						}
					}
				}
				else // Es decir, A NO es un vector.
				{
					if (b->is_vector) //Si B es un vector se trata de un patrón del estilo a,b,c::vectorB
					{
						Value vectorB = TratarMultiplesValores(a, variables);
						for (std::vector<tipoValor>::iterator it = a->arr.begin(); it != a->arr.end(); ++it)
						{
							f1 = false;
							if (!tipoValorToValueOrRef(*it, variables, &f1).ref->OperadoresEspeciales_Pop(vectorB, f1))
								return false;
						}
					}
				}
			}
			return true;
		},


			[&](auto&,auto&)->bool {
				if(operador == OP_CHECK_GET)
					return tipoValorToValueOrRef(v1, variables, &f1).ref->OperadoresEspeciales_Check(*tipoValorToValueOrRef(v2, variables, &f2).ref, -1, f1,f2);
				else 
					return tipoValorToValueOrRef(v1, variables, &f1).ref->OperadoresEspeciales_Pop(*tipoValorToValueOrRef(v2, variables, &f2).ref, f1,f2);
			},
		}, v1, v2);


}


bool Interprete::Relacional_rec_arbol(arbol_operacional * node, Variable_Runtime * variables, Value & v2)
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
						  [&](multi_value * a)->Value {  return TratarMultiplesValores(a, variables); },
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
		[&](multi_value * a) {  Value ValID = TratarMultiplesValores(a, variables); return ValID.OperacionRelacional(v2, node->operador); },
		[&](auto&) { Errores::generarError(Errores::ERROR_OPERACION_INVALIDA_VOID, NULL);  return false; },
		}, node->_v1);
}

std::vector<Value> Interprete::transformarEntradasFuncion(Valor_Funcion * vF, Variable_Runtime * variables)
{
	std::vector<Value> entradas(vF->entradas.size());


	int k = 0;
	for (std::vector<arbol_operacional*>::iterator entrada = vF->entradas.begin(); entrada != vF->entradas.end(); ++entrada)
	{
		entradas[k] = lectura_arbol_operacional((*entrada), variables);
		k++;
	}
	return entradas;
}

bool Interprete::Interprete_Sentencia(Parser_Sentencia * sentencia, Variable_Runtime * variables)
{
	switch (sentencia->tipo)
	{
	case SENT_REC:
	{
		Sentencia_Recursiva* x = static_cast<Sentencia_Recursiva*>(sentencia);

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
		Sentencia_Operacional* x = static_cast<Sentencia_Operacional*>(sentencia);
		lectura_arbol_operacional(x->pOp, variables);
		return true;
	}
	// FUNCION SALIDA DATOS :
	// Permite escribir por consola un valor dado.
	case SENT_PRINT:
	{
		Sentencia_Print* x = static_cast<Sentencia_Print*>(sentencia);
		Value v = lectura_arbol_operacional(x->pOp, variables);

		v.print();
		return true;
	}
	// Permite escribir por consola un valor dado.
	case SENT_RETURN:
	{
		Sentencia_Return* x = static_cast<Sentencia_Return*>(sentencia);
		if (x->pOp)
		{
			setRetorno(lectura_arbol_operacional(x->pOp, variables));
		}
		else retornoSinValor();

		return true;
	}
	// FUNCION IF:
	// Permite la comprobacion de operaciones o variables.
	case SENT_IF:
	{
		Sentencia_IF* x = static_cast<Sentencia_IF*>(sentencia);

		Value b = lectura_arbol_operacional(x->pCond, variables);

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
		Sentencia_WHILE* x = static_cast<Sentencia_WHILE*>(sentencia);


		while (true)
		{
			Value b = lectura_arbol_operacional(x->pCond, variables);

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
		Sentencia_FOR* x = static_cast<Sentencia_FOR*>(sentencia);
		//Interprete_Sentencia(x->pIguald, variables);

		if (x->pIguald)
		{
			lectura_arbol_operacional(x->pIguald, variables);
		}

		while (true)
		{
			bool res = true;

			if (x->pCond)
			{
				res = lectura_arbol_operacional(x->pCond, variables).ValueToBool();
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
					lectura_arbol_operacional(x->pOp, variables);
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
			Core_Function_Interprete * funcionInterprete = static_cast<Core_Function_Interprete*>(*funcion);

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

			Variable_Runtime * variables = NULL;

			if ((*funcion)->preload_var > 0)
				variables = new Variable_Runtime[(*funcion)->preload_var];

			bool entradas_incorrectas = false;

			int ent_itr = 0;
			for (std::vector<arbol_operacional*>::iterator it = (*funcion)->entradas.begin(); it != (*funcion)->entradas.end(); ++it)
			{
				if ((*it)->operador == OP_NONE)
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

								if (a->inicializando)
									return identificador->value.asignacion(entradas[ent_itr], identificador->fuerte);
								else
									return identificador->value.igualdad_Condicional(entradas[ent_itr]);
							},
							[&](Valor_Funcion * a) { return entradas[ent_itr].igualdad_Condicional(ExecFuncion(a->ID->nombre, transformarEntradasFuncion(a,variables))); },
							[&](multi_value * a) 
							{  
								if (a->contenedor)
								{
									if (!a->operacionesVector->onlyValue && a->operacionesVector->id_v)
									{
										Variable_Runtime* identificador = a->operacionesVector->id_v->var_global ? &this->variables_globales[a->operacionesVector->id_v->index] : &variables[a->operacionesVector->id_v->index];

										if (!identificador->value.asignacion(entradas[ent_itr], false))
											return false;

										if (a->operacionesVector->dobleOperador)
										{			
											if (!OperacionOperadoresVectores(a->operacionesVector->v1, a->operacionesVector->v2, a->operacionesVector->operador1, variables))
												return false;

											if (!OperacionOperadoresVectores(a->operacionesVector->v2, a->operacionesVector->v3, a->operacionesVector->operador2, variables))
												return false;

											return true;
										}
										else //No hay doble operador
										{
											return OperacionOperadoresVectores(a->operacionesVector->v1, a->operacionesVector->v2, a->operacionesVector->operador1, variables);
										}
									}
								}

								return entradas[ent_itr].igualdad_Condicional(TratarMultiplesValores(a, variables)); 
							},
							[&](auto&) { return false; },
						}, (*it)->_v1))
					{
						entradas_incorrectas = true;
						break;
					}
				}
				else
				{
					if (!lectura_arbol_operacional((*it), variables).igualdad_Condicional(entradas[ent_itr]))
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

						if ((*funcion)->salida->value != PARAM_VOID)
							Errores::generarWarning(Errores::WARNING_FUNCION_VALOR_DEVUELTO_VOID, &(*funcion)->parametros, (*funcion)->pID->nombre);

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

