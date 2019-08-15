
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
	
	// Tratamiento de valores de funciones por el parser.
	parser->preloadFunciones(this->funciones);

	this->variables_globales = new Variable_Runtime[parser->numero_variables_globales];

	for (std::vector<arbol_operacional*>::iterator it = var_globales.begin(); it != var_globales.end(); ++it)
	{
		lectura_arbol_operacional((*it), NULL);
	}

	return true;
}

// Esta funci�n solo ser� llamada para archivos incompletos, que no contienen una funci�n main de inicio.
// as� como para el interprete por consola.
// Se considerar� una funci�n propiamente.

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
	parser->preloadFunciones(this->funciones);

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
		std::shared_ptr <mdox_vector> res = std::make_shared<mdox_vector>();
		res->vector.reserve(arr->arr.size());

		//int itr = 0;
		for (std::vector<tipoValor>::iterator it = arr->arr.begin(); it != arr->arr.end(); ++it)
		{
			res->vector.emplace_back(std::visit(overloaded
				{
					[&](Value & a)->Value {return a; },
					[&](arbol_operacional * a)->Value { return lectura_arbol_operacional(a, variables);  },
					[&](Valor_Funcion * a)->Value { return ExecFuncion(a, transformarEntradasFuncion(a, variables)); },
					[&](Parser_Identificador * a)->Value { return a->var_global ? this->variables_globales[a->index].value : variables[a->index].value;  },
					[&](auto&)->Value {  Errores::generarError(Errores::ERROR_OPERACION_INVALIDA, NULL); return std::monostate(); },
				}, *it));
		}
		return std::move(res);
	}
	else if (arr->contenedor)
	{
		// Un contenedor obtiene una sucesi�n de operaciones con operadores de vectores.
		//Por lo tanto siempre se tratar� de un "OperadorEnVector"


		Parser_Identificador* f1 = NULL;
		Parser_Identificador* f2 = NULL;

		ValueCopyOrRef v1 = tipoValorToValueOrRef(arr->operacionesVector->v1, variables, &f1);
		ValueCopyOrRef v2 = tipoValorToValueOrRef(arr->operacionesVector->v2, variables, &f2);

		bool left = false;
		bool isPop = false, isPop2=false;
		if (v1.ref)
		{
			if (v2.ref)
			{		
				if (!OperacionOperadoresVectores(v1.ref, v2.ref, arr->operacionesVector->operador1, isPop, left, f1,f2))
					return false;
			}
			else
			{
				if (!OperacionOperadoresVectores(v1.ref, v2.mv, arr->operacionesVector->operador1, variables, isPop, left, f1, f2))
					return false;
			}
		}
		else
		{
			if (v2.ref)
			{
				if (!OperacionOperadoresVectores(v1.mv, v2.ref, arr->operacionesVector->operador1, variables, isPop, left, f1, f2))
					return false;
			}
		}
		
		if (arr->operacionesVector->dobleOperador)
		{
			if (v2.ref == NULL)
				return false;

			ValueCopyOrRef v3 = tipoValorToValueOrRef(arr->operacionesVector->v3, variables, &f1);

			if (v3.ref)
			{
				if (!OperacionOperadoresVectores(v2.ref, v3.ref, arr->operacionesVector->operador2, isPop2, left,f2,f1))
					return false;
			}
			else
			{
				if (!OperacionOperadoresVectores(v2.ref, v3.mv, arr->operacionesVector->operador2,variables, isPop2, left, f2, f1))
					return false;
			}

			if (isPop || isPop2)
				return *v2.ref;

			return true;
		}

		if (isPop)
		{
			if (left)
				return *v1.ref;
			else return *v2.ref;
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
					[&](Valor_Funcion * a) { ExecFuncion(a, transformarEntradasFuncion(a, variables)); },
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
				[&](Valor_Funcion * a)->Value { return ExecFuncion(a, transformarEntradasFuncion(a, variables)); },
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
		[&](Valor_Funcion * a)->Value { return ExecFuncion(a, transformarEntradasFuncion(a, variables)).operacion_Unitaria(node->operador); },
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
				[&](Valor_Funcion * a)->Value { return ExecFuncion(a, transformarEntradasFuncion(a, variables)); },
				[&](multi_value * a)->Value {  return TratarMultiplesValores(a, variables); },
				[&](auto&)->Value { Errores::generarError(Errores::ERROR_OPERACION_INVALIDA, NULL);  return std::monostate(); },
					}, node->_v2)
					, node->operador, identificador->fuerte); //False por ahora



			return identificador->value;
		},
		[&](Valor_Funcion * a)->Value { return ExecFuncion(a, transformarEntradasFuncion(a, variables)); },

			//Arbol operacional para aceptar casos como: a = b = 5;
		[&](arbol_operacional * a)->Value
		{
			Value vr = std::visit(overloaded{
			[&](arbol_operacional * a)->Value { return lectura_arbol_operacional(a,variables); },
			[](Value & a)->Value {return a; },
			[&](Parser_Identificador * a)->Value { return a->var_global ? this->variables_globales[a->index].value : variables[a->index].value;  },
			[&](Valor_Funcion * a)->Value { return ExecFuncion(a, transformarEntradasFuncion(a, variables)); },
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
		[&](Valor_Funcion * a)->Value { return ExecFuncion(a, transformarEntradasFuncion(a, variables)); },
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
						 [&](Valor_Funcion * a)->Value { return ExecFuncion(a, transformarEntradasFuncion(a, variables)); },
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
		[&](Valor_Funcion * a)->Value { return ExecFuncion(a, transformarEntradasFuncion(a, variables)).OperacionRelacional(v2, node->operador); },
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
		[&](Valor_Funcion * a)->Value { return ExecFuncion(a, transformarEntradasFuncion(a, variables)); },
		[&](multi_value * a)->Value {  return TratarMultiplesValores(a, variables); },
		[&](auto&)->Value { Errores::generarError(Errores::ERROR_OPERACION_INVALIDA_VOID, NULL);  return std::monostate(); },
			}, node->_v1).operacion_Binaria(
				std::visit(overloaded{
				[&](arbol_operacional * a)->Value { return lectura_arbol_operacional(a, variables); },
				[](Value & a)->Value {return a; },
				[&](Parser_Identificador * a)->Value { return a->var_global ? this->variables_globales[a->index].value : variables[a->index].value;  },
				[&](Valor_Funcion * a)->Value { return ExecFuncion(a, transformarEntradasFuncion(a, variables)); },
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
				vr->value = std::monostate();
				*ret = a;

				return &vr->value;
			}
			*ret = a;

			if (a->inicializando)
			{
				Variable_Runtime* vr = a->var_global ? &this->variables_globales[a->index] : &variables[a->index];
				vr->value = std::monostate();
				return &vr->value;
			}

			return a->var_global ? &this->variables_globales[a->index].value : &variables[a->index].value;
			
		},
		[&](Valor_Funcion * a)->ValueCopyOrRef { return ExecFuncion(a, transformarEntradasFuncion(a, variables)); },
		[&](multi_value * a)->ValueCopyOrRef { if (a->contenedor || a->is_vector) return TratarMultiplesValores(a, variables); return a; },
		[&](auto&)->ValueCopyOrRef { return std::monostate(); },
		}, a);
}

bool Interprete::OperacionOperadoresVectores(Value * v1, Value * v2, OPERADORES& operador, bool& isPop, bool& left, Parser_Identificador* f1, Parser_Identificador * f2)
{
	if (operador == OP_CHECK_GET)
	{
		isPop = false;
		return v1->OperadoresEspeciales_Check(*v2, -1, f1, f2);
	}
	else
	{
		isPop = true;
		return v1->OperadoresEspeciales_Pop(*v2, left, f1, f2);
	}
}

bool Interprete::OperacionOperadoresVectores(multi_value* v1, Value* v2,  OPERADORES& operador, Variable_Runtime* variables, bool& isPop, bool& left, Parser_Identificador* f1, Parser_Identificador* f2)
{
	if (operador == OP_CHECK_GET)
	{
		isPop = false;
		//Dado que v1 no puede ser un vector, el vector debe ser, en todo caso, v2, de no serlo, la operaci�n devolver� un error a posteriori.
		//Igualmente, trabajaremos considerando v2-> vector, y dejamos el error al check del operador.
		//Esto implica que los valores se checkean desde atr�s->   [a,b,c::vector]
		int itr = 0;
		for (std::vector<tipoValor>::iterator it = v1->arr.begin(); it != v1->arr.end(); ++it)
		{
			if (!tipoValorToValueOrRef(*it, variables, &f1).ref->OperadoresEspeciales_Check(*v2, itr, f1))
				return false;
			itr++;
		}
	}
	else // POP
	{
		isPop = true;
		for (std::vector<tipoValor>::iterator it = v1->arr.begin(); it != v1->arr.end(); ++it)
		{
			if (!tipoValorToValueOrRef(*it, variables, &f1).ref->OperadoresEspeciales_Pop(*v2, left, f1))
				return false;
		}
	}
	return true;
}

bool Interprete::OperacionOperadoresVectores(Value* v1, multi_value* v2, OPERADORES& operador, Variable_Runtime* variables, bool& isPop, bool& left, Parser_Identificador* f1, Parser_Identificador* f2)
{
	if (operador == OP_CHECK_GET)
	{
		isPop = false;
		//Dado que v2 no es un vector, el vector debe ser, en todo caso, a, de no serlo, la operaci�n devolver� un error a posteriori.
		//Igualmente, trabajaremos considerando v1-> vector, y dejamos el error al check del operador.

		//Esto implica que los valores se checkean desde atr�s->   [vector::a,b,c]
		int itr = 0;
		for (std::vector<tipoValor>::iterator it = v2->arr.begin(); it != v2->arr.end(); ++it)
		{
			if (!v1->OperadoresEspeciales_Check(*tipoValorToValueOrRef(*it, variables, &f2).ref, itr, NULL, f2))
			{
				return false;
			}
			itr++;
		}
	}
	else // POP
	{
		isPop = true;
		for (std::vector<tipoValor>::iterator it = v2->arr.begin(); it != v2->arr.end(); ++it)
		{
			if (!v1->OperadoresEspeciales_Pop(*tipoValorToValueOrRef(*it, variables, &f2).ref, left, NULL, f2))
			{
				return false;
			}
		}
	}
	return true;
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
						 [&](Valor_Funcion * a)->Value { return ExecFuncion(a, transformarEntradasFuncion(a, variables)); },
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
		[&](Valor_Funcion * a) {  Value ValID = ExecFuncion(a, transformarEntradasFuncion(a, variables)); return ValID.OperacionRelacional(v2, node->operador); },
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
				//Fallo inesperado, no deber�a ocurrir nunca si se han filtrado correctamente los errores.
				Errores::generarError(Errores::ERROR_CRITICO, &(*it)->parametros);
				return false;
			}

			if (return_activo || break_activo || continue_activo || ignore_activo)
				break;

		}
		return true;
	}

	// Add+1 -> Tambi�n se podr� llamar a operaciones matem�ticas, pero su resultado nunca se guardar�. 
	//  Esto se debe a que se puede ejecutar directamente funciones a�n sin ser return void.
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
	// Acciones
	case SENT_ACCION:
	{
		Sentencia_Accion* x = static_cast<Sentencia_Accion*>(sentencia);
		switch (x->accion)
		{
			case TipoAccion::BREAK:
			{
				this->break_activo = true;
				break;
			}
			case TipoAccion::CONTINUE:
			{
				this->continue_activo = true;
				break;
			}
			case TipoAccion::IGNORE:
			{
				this->ignore_activo = true;
				break;
			}

		}
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
	// Repite la sentencia hasta que la condici�n del bucle while se cumpla.
	case SENT_WHILE:
	{
		Sentencia_WHILE* x = static_cast<Sentencia_WHILE*>(sentencia);


		while (true)
		{
			if (return_activo ||  breakCalled() || ignore_activo)
				break;

			if (continueCalled())
				continue;

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
		return true;
	}
	// FUNCION FOR:
	// Repite la sentencia hasta que la condici�n del bucle while se cumpla.
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

				if (return_activo || breakCalled()  || ignore_activo)
					break;

				if (continueCalled())
					continue;

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
		return true;
	}
	}
	return false;
}

bool Interprete::FuncionCore(Valor_Funcion* vf, std::vector<Value> entradas)
{
	//Core::core_functions
	for (std::vector<int>::iterator dItr = vf->funcionesCoreItrData.begin(); dItr != vf->funcionesCoreItrData.end(); ++dItr)
	{
		Core_Function_Interprete* funcionInterprete = static_cast<Core_Function_Interprete*>(Core::core_functions[*dItr]);

		//std::vector<Value>&, Interprete*, OutData_Parametros*);
		if (funcionInterprete->funcion_exec(entradas, this))
		{
			return true;
		}
	}

	return false;
}


// Ejecuta la funci�n dentro del ENTORNO. Es decir, se trata de una funci�n que NO est� fuera del entorno de llamada. (Es decir, no forma parte de una clase diferente)
// Las variables a las cuales tiene acceso esta funci�n, ser�n las variables del entorno propio, es decir VARIABLES GLOBALES, variables declaradas a nivel de main.
Value Interprete::ExecFuncion(Valor_Funcion* vf, std::vector<Value> entradas)
{
	//xFunc->entradas

	/*int k = 0;
	for (std::vector<Parser_Operacion*>::iterato r it = xFunc->entradas.begin(); it != xFunc->entradas.end(); ++it)
	{
		valores_entrada[k] = lectura_arbol_operacional()
		k++;
	}*/

	// ------------ Tipos de Funciones  ------------
	// **** Comenzamos probando si se trata de una funci�n del core del lenguaje.
	Errores::saltarErrores = true;
	if (this->FuncionCore(vf, entradas))
	{
		if (this->returnCalled())
		{
			Errores::saltarErrores = false;
			return this->getRetorno();
		}
		Errores::saltarErrores = false;
		return std::monostate();
	}


	// **** EN EL CASO de que no se encontrara ninguna funci�n core, vamos con las normales:


	//De entre todas las funciones declaradas, buscamos la que realmente se est� llamando.

	for (std::vector<int>::iterator dItr = vf->funcionesItrData.begin(); dItr != vf->funcionesItrData.end(); ++dItr)
	{

			bool correcto = true; // Si es FALSE se limpiar� el heap de las variables creadas, pero no ejecutar� la funci�n.
			bool forzar_salida = false; //Se devolver� NULL


			//Value identificador = a->var_global ? this->variables_globales[a->index].value : variables[a->index].value;

			Variable_Runtime * variables = NULL;

			if (funciones[*dItr]->preload_var > 0)
				variables = new Variable_Runtime[funciones[*dItr]->preload_var];

			bool entradas_incorrectas = false;

			int ent_itr = 0;
			for (std::vector<arbol_operacional*>::iterator it = funciones[*dItr]->entradas.begin(); it != funciones[*dItr]->entradas.end(); ++it)
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
							[&](Valor_Funcion * a) { return entradas[ent_itr].igualdad_Condicional(ExecFuncion(a, transformarEntradasFuncion(a,variables))); },
							[&](multi_value * a) 
							{  
								if (a->contenedor)
								{
									if (!a->operacionesVector->onlyValue && a->operacionesVector->id_v)
									{
										Variable_Runtime* identificador = a->operacionesVector->id_v->var_global ? &this->variables_globales[a->operacionesVector->id_v->index] : &variables[a->operacionesVector->id_v->index];

										if (!identificador->value.asignacion(entradas[ent_itr], false))
											return false;

										//hackfix para evitar que en funciones [xs:x], xs se convierta en monostate
										a->operacionesVector->id_v->inicializando = false;

										Parser_Identificador* f1 = NULL;
										Parser_Identificador* f2 = NULL;

										ValueCopyOrRef v1 = tipoValorToValueOrRef(a->operacionesVector->v1, variables, &f1);
										ValueCopyOrRef v2 = tipoValorToValueOrRef(a->operacionesVector->v2, variables, &f2);

										bool left = false;
										bool isPop;

										if (v1.ref)
										{
											if (v2.ref)
											{
												if (!OperacionOperadoresVectores(v1.ref, v2.ref, a->operacionesVector->operador1, isPop, left, f1, f2))
													return false;
											}
											else
											{
												if (!OperacionOperadoresVectores(v1.ref, v2.mv, a->operacionesVector->operador1, variables, isPop, left, f1, f2))
													return false;
											}
										}
										else
										{
											if (v2.ref)
											{
												if (!OperacionOperadoresVectores(v1.mv, v2.ref, a->operacionesVector->operador1, variables, isPop, left, f1, f2))
													return false;
											}
										}

										if (a->operacionesVector->dobleOperador)
										{
											if (v2.ref == NULL)
												return false;

											ValueCopyOrRef v3 = tipoValorToValueOrRef(a->operacionesVector->v3, variables, &f1);

											if (v3.ref)
											{
												if (!OperacionOperadoresVectores(v2.ref, v3.ref, a->operacionesVector->operador2, isPop, left, f2, f1))
													return false;
											}
											else
											{
												if (!OperacionOperadoresVectores(v2.ref, v3.mv, a->operacionesVector->operador2, variables, isPop, left, f2, f1))
													return false;
											}
											return true;
										}

										return true;
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

			if (Interprete_Sentencia(funciones[*dItr]->body, variables))
			{
				if (this->ignoreCalled())
					continue;

				Errores::saltarErrores = false;
				delete[] variables;

				//Si la salida es NULA es que no se ha especificado una, la salida ser� igual al valor de retorno.
				if (funciones[*dItr]->salida == NULL)
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
						return retorno.Cast(funciones[*dItr]->salida);
					}
					else
					{

						if (funciones[*dItr]->salida->value != PARAM_VOID)
							Errores::generarWarning(Errores::WARNING_FUNCION_VALOR_DEVUELTO_VOID, &funciones[*dItr]->parametros, funciones[*dItr]->pID->nombre);

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

	Errores::saltarErrores = false;
	Errores::generarError(Errores::ERROR_FUNCION_NO_RECONOCIDA, NULL, vf->ID->nombre);
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

