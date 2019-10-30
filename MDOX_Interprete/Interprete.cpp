
#include "Interprete.h"
//#include "../MDOX/Errores.cpp"
//#include "../MDOX/Parser.cpp"
//#include "../MDOX/Operaciones.cpp"
//#include "../MDOX/LoadCache.cpp"

Interprete* Interprete::instance;

bool Interprete::CargarDatos(Parser* parser)
{
	//Borramos la lista de funciones existentes actualmente.
	for (std::vector<Parser_Funcion*>::iterator it = this->funciones.begin(); it != this->funciones.end(); ++it)
	{
		delete (*it);
	}
	this->funciones.clear();

	//Borramos la lista de clases existentes actualmente.
	for (std::vector<Parser_Class*>::iterator it = this->clases.begin(); it != this->clases.end(); ++it)
	{
		delete (*it);
	}
	this->clases.clear();


	/*	std::cout << "TOKENS: \n";

		for (unsigned i = 0; i < parser->tokenizer.tokens.size(); i++)
		{
			std::cout << "  [" << parser->tokenizer.tokens.at(i)->token << "]  ";
		}

		std::cout << "\n\n";*/


	this->nombre_ficheros.push_back(parser->tokenizer.fichero);
	std::vector<arbol_operacional*> var_globales;
	SendVariables variables;

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

		Parser_Class* pClass = parser->getClass(local);

		if (pClass)
		{
			this->clases.push_back(pClass);
			continue;
		}


		parser->isGlobal = true;
		arbol_operacional* p3 = parser->getOperacion(local, variables);
		parser->isGlobal = false;

		if (p3)
		{
			if (parser->tokenizer.isCloseToken(local))
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
	if (!parser->preloadCalls(this->funciones, &this->clases))
		return false;

	if (parser->existenErrores)
		return false;

	this->variables_globales = new Variable_Runtime[parser->variables_globales.size()];

	for (std::vector<arbol_operacional*>::iterator it = var_globales.begin(); it != var_globales.end(); ++it)
	{
		lectura_arbol_operacional((*it), NULL, NULL);
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

	SendVariables variables_funcion;
	parser->isGlobal = false;

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

	Variable_Runtime* variables = new Variable_Runtime[*variables_funcion.num_local_var];
	if (!parser->preloadCalls(this->funciones, &this->clases))
	{
		delete inst;
		delete[] variables;
		return;
	}

	if (parser->existenErrores)
	{
		delete inst;
		delete[] variables;
		return;
	}

	if (!Interprete_Sentencia(inst, variables, NULL))
	{
		Errores::generarError(Errores::ERROR_CRITICO, &(inst)->parametros);
		delete inst;
		delete[] variables;
		return;
	}

	delete inst;
	delete[] variables;
}

/*
Value* wrapper_object_call::getValue(bool show_error,  tipos_parametros* tipo)
{
	Variable_Runtime* rtv = this->objeto->findVariable(this->var->nombre);
	if (rtv == nullptr)
	{
		Errores::generarError(Errores::ERROR_CLASE_CALL_VARIABLE_NO_EXISTE, NULL, this->var->nombre);
		return nullptr;
	}
	if (!rtv->publica)
	{
		Errores::generarError(Errores::ERROR_CLASE_VAR_PRIVATE, NULL, this->var->nombre);
		return nullptr;
	}
	else
	{
		if (tipo) *tipo = rtv->tipo;
		return &rtv->value;
	}
}*/

void Interprete::getRealValueFromValueWrapper(Value& v, tipos_parametros* tipo)
{
	std::visit(overloaded
		{
			[&](Variable_Runtime * a)
			{
				if (a == nullptr)
					v = std::monostate();
				else
				{
					v = a->value;
					if (tipo)* tipo = a->tipo;
				}
			},
			[&](auto&) { },
		}, v.value);
}

void Interprete::getRealValueFromValueWrapperRef(Value** v, tipos_parametros* tipo)
{
	std::visit(overloaded
		{
			[&](Variable_Runtime * a)
			{
				if (a == nullptr)
					**v = std::monostate();
				else
				{
					*v = &a->value;
					if (tipo)* tipo = a->tipo;
				}
			},
			[&](auto&) {},
		}, (*v)->value);
}

Value Interprete::TratarMultiplesValores(multi_value* arr, Variable_Runtime* variables, Variable_Runtime* var_class)
{
	if (arr->is_vector)
	{
		std::shared_ptr <mdox_vector> res = std::make_shared<mdox_vector>();
		res->vector.reserve(arr->arr.size());

		//int itr = 0;
		for (std::vector<arbol_operacional*>::iterator it = arr->arr.begin(); it != arr->arr.end(); ++it)
		{
			res->vector.emplace_back(lectura_arbol_operacional(*it, variables,var_class));
		}
		return std::move(res);
	}
	else if (arr->contenedor)
	{
		// Un contenedor obtiene una sucesión de operaciones con operadores de vectores.
		//Por lo tanto siempre se tratará de un "OperadorEnVector"

		ValueOrMulti v1 = getValueOrMulti(arr->operacionesVector->v1, variables, var_class);
		ValueOrMulti v2 = getValueOrMulti(arr->operacionesVector->v2, variables, var_class);

		bool left = false;
		bool isPop = false, isPop2=false;
		bool v2IsMulti = false;

		 if( !std::visit(overloaded
			{
				[&](Value& a1)
				{
						return std::visit(overloaded
						{
							[&](Value& a2)
							{
								if (!OperacionOperadoresVectores(&a1, &a2, arr->operacionesVector->operador1, isPop, left))
									return false;
								return true;
							},
							[&](multi_value * a2)
							{
								if (!OperacionOperadoresVectores(&a1, a2, arr->operacionesVector->operador1, variables, var_class, isPop, left))
									return false;

								v2IsMulti = true;
								return true;
							},
						}, v2);
				},
				[&](multi_value* a1)
				{
						return std::visit(overloaded
						{
							[&](Value & a2)
							{
								if (!OperacionOperadoresVectores(a1, &a2, arr->operacionesVector->operador1, variables, var_class, isPop, left))
									return false;
								return true;
							},
							[&](multi_value * a2)
							{
								//NO DEBERIA SUCEDER.
									return false;
							},
						}, v2);
				},
			 }, v1))
			 return false;
		
		if (arr->operacionesVector->dobleOperador)
		{
			if (v2IsMulti)
				return false;

			ValueOrMulti v3 = getValueOrMulti(arr->operacionesVector->v3, variables, var_class);

			if(!std::visit(overloaded
			{
				[&](Value & a3)
				{
					if (!OperacionOperadoresVectores(&std::get<Value>(v2), &a3, arr->operacionesVector->operador1, isPop, left))
						return false;
					return true;
				},
				[&](multi_value * a3)
				{
					if (!OperacionOperadoresVectores(&std::get<Value>(v2), a3, arr->operacionesVector->operador1, variables, var_class, isPop, left))
						return false;

					v2IsMulti = true;
					return true;
				},
				}, v3))
				return false;

			if (isPop || isPop2)
				return std::get<Value>(v2);

			return true;
		}

		if (isPop)
		{
			if (left)
				return std::get<Value>(v1);
			else return std::get<Value>(v2);
		}
	
		return true;
	}
	else //Se tratan de operaciones del estilo:  a=2, b=3, c=2;
	{
		for (std::vector<arbol_operacional*>::iterator it = arr->arr.begin(); it != arr->arr.end(); ++it)
		{
			lectura_arbol_operacional(*it, variables,var_class);
		}
		return std::monostate();
	}
}

//Igual que lectura_arbol_operacional, pero en caso de que la operacion sea un identificador
//el value, devolvera la referencia en lugar de el valor.
Value Interprete::lectura_arbol_MultiValue_ref(arbol_operacional* node, Variable_Runtime* variables, Variable_Runtime* var_class)
{
	if (node->operador == OP_NONE)
	{
		return std::visit(overloaded
			{
				[&](Value & a)->Value {return a; },
				[&](Parser_Identificador * a)->Value
				{
					Variable_Runtime* vr = a->var_global ? &this->variables_globales[a->index] : a->var_class ? &var_class[a->index] : a->is_Static ? &a->static_link->static_var_runtime[a->index] : &variables[a->index];

					if (a->fuerte)
					{		
						vr->tipo = a->tipo->value;
						vr->value = std::monostate();
						return vr;
					}

					if (a->inicializando)
					{	
						vr->value = std::monostate();
						return vr;
					}

					return vr;
				},
				[&](Call_Value * a)->Value { return a->is_class ? ExecClass(a, transformarEntradasCall(a, variables,var_class)) : ExecFuncion(a, transformarEntradasCall(a, variables,var_class),var_class); },
				[&](multi_value * a)->Value
				{
						return TratarMultiplesValores(a, variables,var_class);
				},
					// [&](wrapper_object_call & a)->Value { Value* v = a.getValue(true);  return v == nullptr ? std::monostate() : *v;  },
				[&](auto&)->Value { return std::monostate(); },
			}, node->_v1);
	}

	return lectura_arbol_operacional(node, variables, var_class);
}

//Value, Parser_Identificador*, Valor_Funcion*, arbol_operacional*
Value Interprete::lectura_arbol_operacional(arbol_operacional* node, Variable_Runtime* variables, Variable_Runtime* var_class)
{
	if (node->operador == OP_NONE)
	{
		return std::visit(overloaded
			{
				[&](Value & a)->Value {return a; },
				[&](Parser_Identificador * a)->Value
				{							
					Variable_Runtime* identificador = a->var_global ? &this->variables_globales[a->index] : a->var_class ? &var_class[a->index] : a->is_Static ? &a->static_link->static_var_runtime[a->index] : &variables[a->index];

					if (a->fuerte)
					{
						identificador->value.inicializacion(a->tipo);
						identificador->tipo = a->tipo->value;
					}
					

					identificador->publica = node->is_Public;

					return identificador->value;
				},
				[&](Call_Value * a)->Value { return a->is_class ? ExecClass(a, transformarEntradasCall(a, variables,var_class)) : ExecFuncion(a, transformarEntradasCall(a, variables,var_class),var_class); },
				[&](multi_value * a)->Value 
				{ 
						return TratarMultiplesValores(a, variables,var_class);
				},
					// [&](wrapper_object_call & a)->Value { Value* v = a.getValue(true);  return v == nullptr ? std::monostate() : *v;  },
				[&](auto&)->Value { return std::monostate(); },
			}, node->_v1);
	}

	if (is_single_operator(node->operador))
	{
		return std::visit(overloaded{
		[&](arbol_operacional * a)->Value { return lectura_arbol_operacional(a, variables,var_class).operacion_Unitaria(node->operador); },
		[&](Value & a)->Value {return a.operacion_Unitaria(node->operador); },
		[&](Parser_Identificador * a)->Value
			{
				Value* id = a->var_global ? &this->variables_globales[a->index].value : a->var_class ? &var_class[a->index].value : a->is_Static ? &a->static_link->static_var_runtime[a->index].value : &variables[a->index].value;

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
		//[&](wrapper_object_call & a)->Value { Value* v = a.getValue(true);  return (v == nullptr ? std::monostate() :  v->operacion_Unitaria(node->operador)); },
		[&](Call_Value * a)->Value { return a->is_class ? ExecClass(a, transformarEntradasCall(a, variables,var_class)).operacion_Unitaria(node->operador) : ExecFuncion(a, transformarEntradasCall(a, variables,var_class),var_class).operacion_Unitaria(node->operador); },
		[&](multi_value * a)->Value { return TratarMultiplesValores(a, variables,var_class).operacion_Unitaria(node->operador);  },
		[&](auto&)->Value { Errores::generarError(Errores::ERROR_OPERACION_INVALIDA, NULL);  return false; },
			}, node->_v2);
	}
	else if (is_assignment_operator(node->operador))
	{
		return std::visit(overloaded{
		[&](Parser_Identificador * a)->Value
		{
			Variable_Runtime* identificador = a->var_global ? &this->variables_globales[a->index] : a->var_class ? &var_class[a->index] : a->is_Static ? &a->static_link->static_var_runtime[a->index] : &variables[a->index];

			if (a->fuerte)
			{
				identificador->value.inicializacion(a->tipo);
				identificador->tipo = a->tipo->value;
			}

			if (a->inicializando)
			{
				identificador->publica = node->is_Public;
			}

			identificador->value.operacion_Asignacion(
				std::visit(overloaded{
				[&](arbol_operacional * a)->Value { return lectura_arbol_operacional(a,variables,var_class); },
				[](Value & a)->Value {return a; },
				[&](Parser_Identificador * a)->Value
					{
						return a->var_global ? this->variables_globales[a->index].value : a->var_class ? var_class[a->index].value : a->is_Static ? a->static_link->static_var_runtime[a->index].value : variables[a->index].value;
					},
				[&](Call_Value* a)->Value { return a->is_class ? ExecClass(a, transformarEntradasCall(a, variables,var_class)) : ExecFuncion(a, transformarEntradasCall(a, variables,var_class),var_class); },
				[&](multi_value * a)->Value {  return TratarMultiplesValores(a, variables,var_class); },
				[&](auto&)->Value { Errores::generarError(Errores::ERROR_OPERACION_INVALIDA, NULL);  return std::monostate(); },
					}, node->_v2)
					, node->operador, identificador->tipo != PARAM_VOID); //False por ahora



			return identificador->value;
		},
		[&](Call_Value* a)->Value { return a->is_class ? ExecClass(a, transformarEntradasCall(a, variables,var_class)) : ExecFuncion(a, transformarEntradasCall(a, variables,var_class),var_class); },
			//e(std::string& id, Value& v, bool fuerte = false)
		
		[&](arbol_operacional * a)->Value
		{
			if (a->operador == OPERADORES::OP_CLASS_ACCESS)
			{
				Value r1 = lectura_arbol_operacional(a, variables, var_class);
				return std::visit(overloaded
					{
						[&](Variable_Runtime* a2)->Value
						{

							a2->value.operacion_Asignacion(
							std::visit(overloaded{
							[&](arbol_operacional * a)->Value { return lectura_arbol_operacional(a,variables,var_class); },
							[](Value & a)->Value {return a; },
							[&](Parser_Identificador * a)->Value
								{
									return a->var_global ? this->variables_globales[a->index].value : a->var_class ? var_class[a->index].value : a->is_Static ? a->static_link->static_var_runtime[a->index].value : variables[a->index].value;
								},
							[&](Call_Value * a)->Value { return a->is_class ? ExecClass(a, transformarEntradasCall(a, variables,var_class)) : ExecFuncion(a, transformarEntradasCall(a, variables,var_class),var_class); },
							[&](multi_value * a)->Value {  return TratarMultiplesValores(a, variables,var_class); },
							[&](auto&)->Value { Errores::generarError(Errores::ERROR_OPERACION_INVALIDA, NULL);  return std::monostate(); },
								}, node->_v2), 	node->operador, a2->tipo != PARAM_VOID
							);
							return a2->value;
						},
						[&](auto&)->Value { return std::monostate(); },
					}, r1.value);
			}


			// ......................    Arbol operacional para aceptar casos como: a = b = 5;
			Value vr = std::visit(overloaded{
			[&](arbol_operacional * a)->Value { return lectura_arbol_operacional(a,variables,var_class); },
			[](Value & a)->Value {return a; },
			[&](Parser_Identificador * a)->Value { return a->var_global ? this->variables_globales[a->index].value : a->var_class ? var_class[a->index].value : a->is_Static ? a->static_link->static_var_runtime[a->index].value : variables[a->index].value; },
			[&](Call_Value* a)->Value { return a->is_class ? ExecClass(a, transformarEntradasCall(a, variables,var_class)) : ExecFuncion(a, transformarEntradasCall(a, variables,var_class),var_class); },
			[&](multi_value * a)->Value {  return TratarMultiplesValores(a, variables,var_class); },
			[&](auto&)->Value { Errores::generarError(Errores::ERROR_OPERACION_INVALIDA, NULL);  return std::monostate(); },
				}, node->_v2);

			std::visit(overloaded{
				[&](Parser_Identificador * aID)
					{				
						Variable_Runtime* ID = aID->var_global ? &this->variables_globales[aID->index] : aID->var_class ? &var_class[aID->index] : a->is_Static ? &aID->static_link->static_var_runtime[aID->index] : &variables[aID->index];
						if (aID->fuerte)
						{
							ID->value.inicializacion(aID->tipo);
							ID->tipo = aID->tipo->value;
						}
						ID->publica = node->is_Public;
						ID->value.operacion_Asignacion(vr, a->operador, ID->tipo != PARAM_VOID);
					},
				[&](auto&) {},
			}, a->_v2);

			return lectura_arbol_operacional(a, variables, var_class);

		},
		[&](multi_value * a)->Value
		{
			 return TratarMultiplesValores(a, variables,var_class);
		},
	
		[&](auto&)->Value {Errores::generarError(Errores::ERROR_OPERACION_INVALIDA, NULL);  return std::monostate(); },
			}, node->_v1);
	}
	else if (isRelationalOperator(node->operador))
	{
		Value v2 = std::visit(overloaded{
		[&](arbol_operacional * a)->Value { return lectura_arbol_operacional(a,variables,var_class); },
		[](Value & a)->Value {return a; },
		[&](Parser_Identificador * a)->Value { return a->var_global ? this->variables_globales[a->index].value : a->var_class ? var_class[a->index].value : a->is_Static ? a->static_link->static_var_runtime[a->index].value : variables[a->index].value; },
		[&](Call_Value* a)->Value { return a->is_class ? ExecClass(a, transformarEntradasCall(a, variables,var_class)) : ExecFuncion(a, transformarEntradasCall(a, variables,var_class),var_class); },
		[&](multi_value * a)->Value {  return TratarMultiplesValores(a, variables,var_class); },
		[&](auto&)->Value { Errores::generarError(Errores::ERROR_OPERACION_INVALIDA_VOID, NULL);  return std::monostate(); },
			}, node->_v2);


		return std::visit(overloaded{
		[&](arbol_operacional * a)->Value
			{
				 if (isRelationalOperator(a->operador))
				 {
					 Value r_back = std::visit(overloaded{
						 [&](arbol_operacional * a)->Value { return lectura_arbol_operacional(a,variables,var_class); },
						 [](Value & a)->Value {return a; },
						 [&](Parser_Identificador * a)->Value {return a->var_global ? this->variables_globales[a->index].value : a->var_class ? var_class[a->index].value : a->is_Static ? a->static_link->static_var_runtime[a->index].value : variables[a->index].value; },
						 [&](Call_Value* a)->Value { return a->is_class ? ExecClass(a, transformarEntradasCall(a, variables,var_class)) : ExecFuncion(a, transformarEntradasCall(a, variables,var_class),var_class); },  
						 [&](multi_value * a)->Value {  return TratarMultiplesValores(a, variables,var_class); },
						 [&](auto&)->Value { Errores::generarError(Errores::ERROR_OPERACION_INVALIDA_VOID, NULL);  return std::monostate(); },
					  }, a->_v2);

					 if (r_back.OperacionRelacional(v2, node->operador))
						return Relacional_rec_arbol(a, variables, var_class, r_back);
					 else return false;
				 }
				 else return lectura_arbol_operacional(a, variables,var_class).OperacionRelacional(v2, node->operador);
			},
		[&](Value & a)->Value
			{
				return a.OperacionRelacional(v2, node->operador);
			},
		[&](Parser_Identificador * a)->Value { return (a->var_global ? this->variables_globales[a->index].value : a->var_class ? var_class[a->index].value : a->is_Static ? a->static_link->static_var_runtime[a->index].value : variables[a->index].value).OperacionRelacional(v2, node->operador);  },
		[&](Call_Value * a)->Value { return a->is_class ? ExecClass(a, transformarEntradasCall(a, variables,var_class)).OperacionRelacional(v2, node->operador) : ExecFuncion(a, transformarEntradasCall(a, variables,var_class),var_class).OperacionRelacional(v2, node->operador); },
		[&](multi_value * a)->Value {  return TratarMultiplesValores(a, variables,var_class).OperacionRelacional(v2, node->operador); },
		[&](auto&)->Value { Errores::generarError(Errores::ERROR_OPERACION_INVALIDA_VOID, NULL);  return std::monostate(); },
			}, node->_v1);

		//return v2.operacion_Binaria(, node->operador);

	}
	//ACCESO LAS CLASES
	else if (node->operador == OPERADORES::OP_CLASS_ACCESS)
	{
		if (node->is_Static)
		{
			Call_Value* _c1 = std::get<Call_Value*>(node->_v1);

			if (_c1->inx_class)
			{
				if (_c1->class_link_static->valores_estaticos_iniciados)
				{
					return &_c1->class_link_static->static_var_runtime[_c1->inx_class->_especial_var];
				}
				else
				{
					_c1->class_link_static->static_var_runtime = new Variable_Runtime[_c1->class_link_static->variables_static.size()];

					for (std::vector<arbol_operacional*>::iterator it = _c1->class_link_static->variables_static.begin(); it != _c1->class_link_static->variables_static.end(); ++it)
					{
						lectura_arbol_operacional(*it, variables, var_class);
					}

					_c1->class_link_static->valores_estaticos_iniciados = true;
					return &_c1->class_link_static->static_var_runtime[_c1->inx_class->_especial_var];

				}
				
			}
			else if (_c1->inx_funcion) //se trata de la llamda a una función estatica
			{
				if (_c1->class_link_static)
				{
					if (!_c1->class_link_static->valores_estaticos_iniciados)
					{
						_c1->class_link_static->static_var_runtime = new Variable_Runtime[_c1->class_link_static->variables_static.size()];

						for (std::vector<arbol_operacional*>::iterator it = _c1->class_link_static->variables_static.begin(); it != _c1->class_link_static->variables_static.end(); ++it)
						{
							lectura_arbol_operacional(*it, variables, var_class);
						}

						_c1->class_link_static->valores_estaticos_iniciados = true;
					}
				}

				Call_Value* _c2 = std::get<Call_Value*>(node->_v2);

				//Se trata de la llamada  a una variable estatica
				if (_c1->inx_funcion)
				{
					return ExecFuncion(_c2, transformarEntradasCall(_c2, variables, var_class), var_class, _c1->class_link_static);
				}
			}
			else
			{
				Errores::generarError(Errores::ERROR_OPERACION_INVALIDA_VOID, NULL);
				return std::monostate();
			}
		}

		Parser_Identificador* id;
		Call_Value* cv = NULL;

		if (!std::visit(overloaded{
				[&](arbol_operacional * a)->bool { return false; },
				[](Value & a)->bool {return false; },
				[&](Parser_Identificador * a)->bool { id = a;  return true;  },
				[&](Call_Value* a)->bool { cv = a; id = a->ID; return true; },
				[&](multi_value * a)->bool {  return false; },
				[&](auto&)->bool { return false; },
			}, node->_v2))
		{
			Errores::generarError(Errores::ERROR_OPERADOR_ACCESO_CLASE_INVALIDO, NULL);
			return std::monostate();
		}

		return std::visit(overloaded{
			[&](arbol_operacional * a)->Value { return lectura_arbol_operacional(a, variables,var_class); },
			[](Value & a)->Value {return a; },
			[&](Parser_Identificador * a)->Value { return a->var_global ? this->variables_globales[a->index].value : a->var_class ? var_class[a->index].value : a->is_Static ? a->static_link->static_var_runtime[a->index].value : variables[a->index].value;  },
			[&](Call_Value * a)->Value { return a->is_class ? ExecClass(a, transformarEntradasCall(a, variables,var_class)) : ExecFuncion(a, transformarEntradasCall(a, variables,var_class),var_class); },
			[&](multi_value * a)->Value {  return TratarMultiplesValores(a, variables,var_class); },
			[&](auto&)->Value { Errores::generarError(Errores::ERROR_OPERACION_INVALIDA_VOID, NULL);  return std::monostate(); },
			}, node->_v1).ClassAccess(id, cv, variables, var_class);

	}
	else
	{
		return std::visit(overloaded{
		[&](arbol_operacional * a)->Value { return lectura_arbol_operacional(a,variables,var_class); },
		[](Value & a)->Value {return a; },
		[&](Parser_Identificador * a)->Value { return a->var_global ? this->variables_globales[a->index].value : a->var_class ? var_class[a->index].value : a->is_Static ? a->static_link->static_var_runtime[a->index].value : variables[a->index].value;  },
		[&](Call_Value* a)->Value { return a->is_class ? ExecClass(a, transformarEntradasCall(a, variables,var_class)) : ExecFuncion(a, transformarEntradasCall(a, variables,var_class),var_class); },
		[&](multi_value * a)->Value {  return TratarMultiplesValores(a, variables,var_class); },
		[&](auto&)->Value { Errores::generarError(Errores::ERROR_OPERACION_INVALIDA_VOID, NULL);  return std::monostate(); },
			}, node->_v1).operacion_Binaria(
				std::visit(overloaded{
				[&](arbol_operacional * a)->Value { return lectura_arbol_operacional(a, variables,var_class); },
				[](Value & a)->Value {return a; },
				[&](Parser_Identificador * a)->Value { return a->var_global ? this->variables_globales[a->index].value : a->var_class ? var_class[a->index].value : a->is_Static ? a->static_link->static_var_runtime[a->index].value : variables[a->index].value;  },
				[&](Call_Value* a)->Value { return a->is_class ? ExecClass(a, transformarEntradasCall(a, variables,var_class)) : ExecFuncion(a, transformarEntradasCall(a, variables,var_class),var_class); },
				[&](multi_value * a)->Value {  return TratarMultiplesValores(a, variables,var_class); },
				[&](auto&)->Value { Errores::generarError(Errores::ERROR_OPERACION_INVALIDA_VOID, NULL);  return std::monostate(); },
					}, node->_v2)
					, node->operador);
	}
}


//Devuelve el Identificador en caso de que lo sea.
//RET es por defecto NULO.
ValueOrMulti Interprete::getValueOrMulti(tipoValor& a, Variable_Runtime* variables, Variable_Runtime * var_class)
{
 	return std::visit(overloaded{
		[&](arbol_operacional * a)->ValueOrMulti { return lectura_arbol_operacional(a, variables, var_class); },
		[](Value & a)->ValueOrMulti {return a; },
		[&](Parser_Identificador * a)->ValueOrMulti
		{ 
			if (a->fuerte)
			{
				Variable_Runtime* vr = a->var_global ? &this->variables_globales[a->index] : a->var_class ? &var_class[a->index] : a->is_Static ? &a->static_link->static_var_runtime[a->index] : &variables[a->index];
				vr->tipo = a->tipo->value;
				vr->value = std::monostate();
				return vr;
			}

			if (a->inicializando)
			{
				Variable_Runtime* vr = a->var_global ? &this->variables_globales[a->index] : a->var_class ? &var_class[a->index] : a->is_Static ? &a->static_link->static_var_runtime[a->index] : &variables[a->index];
				vr->value = std::monostate();
				return vr;
			}

			return a->var_global ? &this->variables_globales[a->index] : a->var_class ? &var_class[a->index] : a->is_Static ? &a->static_link->static_var_runtime[a->index] : &variables[a->index];
			
		},		
		[&](Call_Value * a)->ValueOrMulti { return a->is_class ? ExecClass(a, transformarEntradasCall(a, variables, var_class)) : ExecFuncion(a, transformarEntradasCall(a, variables, var_class),var_class); },
		[&](multi_value * a)->ValueOrMulti { if (a->contenedor || a->is_vector) return TratarMultiplesValores(a, variables, var_class); return a; },
		[&](auto&)->ValueOrMulti { return std::monostate(); },
		}, a);
}

bool Interprete::OperacionOperadoresVectores(Value * v1, Value * v2, OPERADORES& operador, bool& isPop, bool& left)
{
	if (operador == OP_CHECK_GET)
	{
		isPop = false;
		return v1->OperadoresEspeciales_Check(v2, -1);
	}
	else
	{
		isPop = true;
		return v1->OperadoresEspeciales_Pop(v2, left);
	}
}

bool Interprete::OperacionOperadoresVectores(multi_value* v1, Value* v2,  OPERADORES& operador, Variable_Runtime* variables, Variable_Runtime* var_class, bool& isPop, bool& left)
{
	if (operador == OP_CHECK_GET)
	{
		isPop = false;
		//Dado que v1 no puede ser un vector, el vector debe ser, en todo caso, v2, de no serlo, la operación devolverá un error a posteriori.
		//Igualmente, trabajaremos considerando v2-> vector, y dejamos el error al check del operador.
		//Esto implica que los valores se checkean desde atrás->   [a,b,c::vector]
		int itr = 0;
		for (std::vector<arbol_operacional*>::iterator it = v1->arr.begin(); it != v1->arr.end(); ++it)
		{
			if (!lectura_arbol_MultiValue_ref(*it,variables, var_class).OperadoresEspeciales_Check(v2, itr))
				return false;
			itr++;
		}
	}
	else // POP
	{
		isPop = true;
		for (std::vector<arbol_operacional*>::iterator it = v1->arr.begin(); it != v1->arr.end(); ++it)
		{
			if (!lectura_arbol_MultiValue_ref(*it, variables, var_class).OperadoresEspeciales_Pop(v2, left))
				return false;
		}
	}
	return true;
}

bool Interprete::OperacionOperadoresVectores(Value* v1, multi_value* v2, OPERADORES& operador, Variable_Runtime* variables, Variable_Runtime* var_class, bool& isPop, bool& left)
{
	if (operador == OP_CHECK_GET)
	{
		isPop = false;
		//Dado que v2 no es un vector, el vector debe ser, en todo caso, a, de no serlo, la operación devolverá un error a posteriori.
		//Igualmente, trabajaremos considerando v1-> vector, y dejamos el error al check del operador.

		//Esto implica que los valores se checkean desde atrás->   [vector::a,b,c]
		int itr = 0;
		for (std::vector<arbol_operacional*>::iterator it = v2->arr.begin(); it != v2->arr.end(); ++it)
		{
			Value ref = lectura_arbol_MultiValue_ref(*it, variables, var_class);
			if (!v1->OperadoresEspeciales_Check(&ref, itr))
			{
				return false;
			}
			itr++;
		}
	}
	else // POP
	{
		isPop = true;
		for (std::vector<arbol_operacional*>::iterator it = v2->arr.begin(); it != v2->arr.end(); ++it)
		{
			Value ref = lectura_arbol_MultiValue_ref(*it, variables, var_class);
			if (!v1->OperadoresEspeciales_Pop(&ref, left))
			{
				return false;
			}
		}
	}
	return true;
}

bool Interprete::Relacional_rec_arbol(arbol_operacional * node, Variable_Runtime * variables, Variable_Runtime* var_class, Value & v2)
{
	return std::visit(overloaded{
		[&](arbol_operacional * a)
			{
				 if (isRelationalOperator(a->operador))
				 {
					 Value r_back = std::visit(overloaded{
						 [&](arbol_operacional * a)->Value { return lectura_arbol_operacional(a,variables, var_class); },
						 [](Value & a)->Value {return a; },
						 [&](Parser_Identificador * a)->Value { return a->var_global ? this->variables_globales[a->index].value : a->var_class ? var_class[a->index].value : a->is_Static ? a->static_link->static_var_runtime[a->index].value : variables[a->index].value;  },
						 [&](Call_Value * a)->Value {  return a->is_class ? ExecClass(a, transformarEntradasCall(a, variables, var_class)) : ExecFuncion(a, transformarEntradasCall(a, variables, var_class),var_class); },
						  [&](multi_value * a)->Value {  return TratarMultiplesValores(a, variables, var_class); },
						 [&](auto&)->Value { Errores::generarError(Errores::ERROR_OPERACION_INVALIDA_VOID, NULL);  return std::monostate(); },
					  }, a->_v2);

					 if (r_back.OperacionRelacional(v2, node->operador))
						return Relacional_rec_arbol(a, variables, var_class, r_back);
					 else return false;
				 }
				 else
				 {
					return lectura_arbol_operacional(a, variables, var_class).OperacionRelacional(v2, node->operador);
				 }
			},
		[&](Value & a) {return a.OperacionRelacional(v2, node->operador); },
		[&](Parser_Identificador * a) { return (a->var_global ? this->variables_globales[a->index].value : a->var_class ? var_class[a->index].value : a->is_Static ? a->static_link->static_var_runtime[a->index].value : variables[a->index].value).OperacionRelacional(v2, node->operador); },
		[&](Call_Value * a) {  return a->is_class ? ExecClass(a, transformarEntradasCall(a, variables, var_class)).OperacionRelacional(v2, node->operador) : ExecFuncion(a, transformarEntradasCall(a, variables, var_class),var_class).OperacionRelacional(v2, node->operador); },


		[&](multi_value * a) {  Value ValID = TratarMultiplesValores(a, variables, var_class); return ValID.OperacionRelacional(v2, node->operador); },
		[&](auto&) { Errores::generarError(Errores::ERROR_OPERACION_INVALIDA_VOID, NULL);  return false; },
		}, node->_v1);
}

std::vector<Value> Interprete::transformarEntradasCall(Call_Value * vF, Variable_Runtime * variables, Variable_Runtime * var_class)
{
	std::vector<Value> entradas(vF->entradas.size());

	int k = 0;
	for (std::vector<arbol_operacional*>::iterator entrada = vF->entradas.begin(); entrada != vF->entradas.end(); ++entrada)
	{
		entradas[k] = lectura_arbol_operacional((*entrada), variables, var_class);
		k++;
	}
	return entradas;
}

bool Interprete::Interprete_Sentencia(Parser_Sentencia * sentencia, Variable_Runtime * variables, Variable_Runtime* var_class)
{
	switch (sentencia->tipo)
	{
	case SENT_REC:
	{
		Sentencia_Recursiva* x = static_cast<Sentencia_Recursiva*>(sentencia);

		for (std::vector<Parser_Sentencia*>::iterator it = x->valor.begin(); it != x->valor.end(); ++it)
		{
			if (!Interprete_Sentencia(*it, variables, var_class))
			{
				//Fallo inesperado, no debería ocurrir nunca si se han filtrado correctamente los errores.
				Errores::generarError(Errores::ERROR_CRITICO, &(*it)->parametros);
				return false;
			}

			if (return_activo || break_activo || continue_activo || ignore_activo)
				break;

		}
		return true;
	}

	// Add+1 -> También se podrá llamar a operaciones matemáticas, pero su resultado nunca se guardará. 
	//  Esto se debe a que se puede ejecutar directamente funciones aún sin ser return void.
	case SENT_OP:
	{
		Sentencia_Operacional* x = static_cast<Sentencia_Operacional*>(sentencia);
		lectura_arbol_operacional(x->pOp, variables, var_class);
		return true;
	}
	// FUNCION SALIDA DATOS :
	// Permite escribir por consola un valor dado.
	case SENT_PRINT:
	{
		Sentencia_Print* x = static_cast<Sentencia_Print*>(sentencia);
		Value v = lectura_arbol_operacional(x->pOp, variables, var_class);
		getRealValueFromValueWrapper(v);
		v.print();
		return true;
	}
	// FUNCION ENTRADA DATOS :
	// Permite recibir por consola datos.
	case SENT_INPUT:
	{
		Sentencia_Input* x = static_cast<Sentencia_Input*>(sentencia);
		std::string res = "";
		std::getline(std::cin, res);
		MDOX_StringFormat(res, x->pOp, variables, var_class);
		return true;
	}
	// Permite escribir por consola un valor dado.
	case SENT_RETURN:
	{
		Sentencia_Return* x = static_cast<Sentencia_Return*>(sentencia);
		if (x->pOp)
		{
			setRetorno(lectura_arbol_operacional(x->pOp, variables, var_class));
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

		Value b = lectura_arbol_operacional(x->pCond, variables, var_class);
		getRealValueFromValueWrapper(b);

		if (b.ValueToBool())
		{
			if (x->pS)
			{

				if (!Interprete_Sentencia(x->pS, variables, var_class))
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
				if (!Interprete_Sentencia(x->pElse, variables, var_class))
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
			if (return_activo ||  breakCalled() || ignore_activo)
				break;

			if (continueCalled())
				continue;

			Value b = lectura_arbol_operacional(x->pCond, variables, var_class);
			getRealValueFromValueWrapper(b);

			if (b.ValueToBool())
			{
				if (x->pS)
				{
					if (!Interprete_Sentencia(x->pS, variables, var_class))
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
	// Repite la sentencia hasta que la condición del bucle while se cumpla.
	case SENT_FOR:
	{
		Sentencia_FOR* x = static_cast<Sentencia_FOR*>(sentencia);
		//Interprete_Sentencia(x->pIguald, variables);

		if (x->pIguald)
		{
			lectura_arbol_operacional(x->pIguald, variables, var_class);
		}

		while (true)
		{

			bool res = true;

			if (x->pCond)
			{
				res = lectura_arbol_operacional(x->pCond, variables, var_class).ValueToBool();
			}

			if (res)
			{
				if (!Interprete_Sentencia(x->pS, variables, var_class))
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
					lectura_arbol_operacional(x->pOp, variables, var_class);
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

bool Interprete::FuncionCore(Call_Value* vf, std::vector<Value>& entradas)
{
	if (vf->inx_funcion)
	{
		//Core::core_functions
		for (std::vector<int>::iterator dItr = vf->inx_funcion->funcionesCoreItrData.begin(); dItr != vf->inx_funcion->funcionesCoreItrData.end(); ++dItr)
		{
			Core_Function_Interprete* funcionInterprete = static_cast<Core_Function_Interprete*>(Core::core_functions[*dItr]);

			//std::vector<Value>&, Interprete*, OutData_Parametros*);
			if (funcionInterprete->funcion_exec(entradas, this))
			{
				return true;
			}
		}
	}

	return false;
}

//1 entrada(v) -> Operador binario
Value Interprete::ExecOperador(Operator_Class* oc, Value& v, Variable_Runtime* var_class)
{
	if (oc == NULL)
		return std::monostate();

	Variable_Runtime* variables = new Variable_Runtime[oc->num_variables];

	//Damos por hecho, que se trata de un operador binario.
	//TODO, quizás fuera mejor un asign, para conservar datos?
	variables[0] = v;
	if (!Interprete_Sentencia(oc->body, variables, var_class))
	{
		Errores::generarError(Errores::ERROR_CLASE_OPERADOR_SENTENCIA_INVALIDA,NULL);	
		return std::monostate();
	}		

	if (this->returnCalled())
	{

		return this->getRetorno();
	}
	else
	{
		return std::monostate();
	}
}

//0 entrada -> Operador unitario
Value Interprete::ExecOperador(Operator_Class* oc, Variable_Runtime* var_class)
{
	if (oc == NULL)
		return std::monostate();

	Variable_Runtime * variables = new Variable_Runtime[oc->num_variables];

	if (!Interprete_Sentencia(oc->body, variables, var_class))
	{
		Errores::generarError(Errores::ERROR_CLASE_OPERADOR_SENTENCIA_INVALIDA, NULL);
		return std::monostate();
	}

	if (this->returnCalled())
	{

		return this->getRetorno();
	}
	else
	{
		return std::monostate();
	}
}

Value Interprete::ExecClass(Call_Value* vf, std::vector<Value>& entradas)
{
	if(!vf->is_class)
	{
		Errores::generarError(Errores::ERROR_INESPERADO, &vf->parametros, " Intento de ejecutar la clase '" + vf->ID->nombre + "' pero esta no se trata de una clase.");
		return std::monostate();
	}

	//Tomamos la clase del objeto que estamos creando
	Parser_Class* claseMain = this->clases[vf->inx_class->class_index];

	std::shared_ptr<mdox_object> objeto_salida = std::make_shared<mdox_object>(claseMain);

	if (!claseMain->valores_estaticos_iniciados)
	{
		claseMain->static_var_runtime = new Variable_Runtime[claseMain->variables_static.size()];

		for (std::vector<arbol_operacional*>::iterator it = claseMain->variables_static.begin(); it != claseMain->variables_static.end(); ++it)
		{
			lectura_arbol_operacional(*it, claseMain->static_var_runtime, NULL);
		}

		claseMain->valores_estaticos_iniciados = true;
	}

	//Ejecutamos todas las operaciones de la clase, las operaciones en principio se encargan de crear las variables
	for (std::vector<arbol_operacional*>::iterator pOp = claseMain->variables_operar.begin(); pOp != claseMain->variables_operar.end(); ++pOp)
	{
		lectura_arbol_operacional(*pOp, objeto_salida->variables_clase, NULL);
	}
		
	// Si el indice es -1, indica que no está en memoria, es decir, no existen constructores creados por el usuario,
	// por lo que se considerará la predeterminada.
	if (vf->inx_class->constructor_index == -1)
	{
		return Value(objeto_salida);
	}

	Parser_ClassConstructor* constructor = claseMain->constructores[vf->inx_class->constructor_index];

	int itr = 0;
	for (std::vector<Value>::iterator dItr = entradas.begin(); dItr != entradas.end(); ++dItr)
	{
		objeto_salida->variables_clase[constructor->entradas[itr]].value.asignacion(*dItr,false);
		itr++;
	}

	return Value(objeto_salida);

}

// Ejecuta la función dentro del ENTORNO. Es decir, se trata de una función que NO está fuera del entorno de llamada. (Es decir, no forma parte de una clase diferente)
// Las variables a las cuales tiene acceso esta función, serán las variables del entorno propio, es decir VARIABLES GLOBALES, variables declaradas a nivel de main.
Value Interprete::ExecFuncion(Call_Value* vf, std::vector<Value>& entradas, Variable_Runtime* var_class,Parser_Class* pClass)
{
	if (vf->is_class)
	{
		Errores::generarError(Errores::ERROR_INESPERADO, &vf->parametros, " Intento de ejecutar la función '" + vf->ID->nombre +"' pero esta no se trata de una función.");
		return std::monostate();
	}

	std::vector<int>* indices_parseados;
	std::vector<Parser_Funcion*>* funciones;

	// ------------ Tipos de Funciones  ------------
	// **** Comenzamos probando si se trata de una función del core del lenguaje.
	Errores::saltarErrores = true;
	if (pClass)
	{
		indices_parseados = pClass->findFuncion(vf->ID->nombre);
		if (indices_parseados == nullptr)
		{

			Errores::saltarErrores = false;
			Errores::generarError(Errores::ERROR_FUNCION_NO_DECLARADA, NULL, vf->ID->nombre);
			return std::monostate();
		}

		funciones = &pClass->funciones;
		
	}
	else
	{
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
		if (vf->isInsideClass)
		{
			indices_parseados = &vf->inx_funcion->funcionesItrData;
			funciones = &vf->inside_class->funciones;
		}
		else
		{
			indices_parseados = &vf->inx_funcion->funcionesItrData;
			funciones = &this->funciones;
		}
	}


	// **** EN EL CASO de que no se encontrara ninguna función core, vamos con las normales:
	//De entre todas las funciones declaradas, buscamos la que realmente se está llamando.

	for (std::vector<int>::iterator dItr = indices_parseados->begin(); dItr != indices_parseados->end(); ++dItr)
	{
		if (pClass && !(*funciones)[*dItr]->is_Public)
			continue;

		if (entradas.size() != (*funciones)[*dItr]->entradas.size())
			continue;

			bool correcto = true; // Si es FALSE se limpiará el heap de las variables creadas, pero no ejecutará la función.
			bool forzar_salida = false; //Se devolverá NULL

			//Value identificador = a->var_global ? this->variables_globales[a->index].value : variables[a->index].value;

			Variable_Runtime * variables = NULL;

			if ((*funciones)[*dItr]->preload_var > 0)
				variables = new Variable_Runtime[(*funciones)[*dItr]->preload_var];

			bool entradas_incorrectas = false;

			int ent_itr = 0;
			for (std::vector<arbol_operacional*>::iterator it = (*funciones)[*dItr]->entradas.begin(); it != (*funciones)[*dItr]->entradas.end(); ++it)
			{
				if ((*it)->operador == OP_NONE)
				{
					if (!std::visit(overloaded
						{
							[&](Value & a) { return a.igualdad_Condicional(entradas[ent_itr]); },

							[&](Parser_Identificador * a)
							{
								Variable_Runtime* identificador = a->var_global ? &this->variables_globales[a->index] : a->var_class ? &var_class[a->index] : a->is_Static ? &a->static_link->static_var_runtime[a->index] : &variables[a->index];

								if (a->fuerte)
								{
									identificador->value.inicializacion(a->tipo);
									identificador->tipo = a->tipo->value;
								}

								if (a->inicializando)
									return identificador->value.asignacion(entradas[ent_itr], identificador->tipo != PARAM_VOID);
								else
									return identificador->value.igualdad_Condicional(entradas[ent_itr]);
							},

							[&](Call_Value * a) { return entradas[ent_itr].igualdad_Condicional(a->is_class ? ExecClass(a, transformarEntradasCall(a, variables,var_class)) : ExecFuncion(a, transformarEntradasCall(a, variables,var_class),var_class)); },
							[&](multi_value * a) 
							{  
								if (a->contenedor)
								{
									if (!a->operacionesVector->onlyValue && a->operacionesVector->id_v)
									{
										Variable_Runtime* identificador = a->operacionesVector->id_v->var_global ? &this->variables_globales[a->operacionesVector->id_v->index] : a->operacionesVector->id_v->is_Static ? &a->operacionesVector->id_v->static_link->static_var_runtime[a->operacionesVector->id_v->index] : &variables[a->operacionesVector->id_v->index];

										if (!identificador->value.asignacion(entradas[ent_itr], false))
											return false;

										//hackfix para evitar que en funciones [xs:x], xs se convierta en monostate
										a->operacionesVector->id_v->inicializando = false;

										Parser_Identificador* f1 = NULL;
										Parser_Identificador* f2 = NULL;

										ValueOrMulti v1 = getValueOrMulti(a->operacionesVector->v1, variables, var_class);
										ValueOrMulti v2 = getValueOrMulti(a->operacionesVector->v2, variables, var_class);

										bool left = false;
										bool isPop = false, isPop2 = false;
										bool v2IsMulti = false;

										if (!std::visit(overloaded
											{
												[&](Value & a1)
												{
														return std::visit(overloaded
														{
															[&](Value & a2)
															{
																if (!OperacionOperadoresVectores(&a1, &a2, a->operacionesVector->operador1, isPop, left))
																	return false;
																return true;
															},
															[&](multi_value * a2)
															{
																if (!OperacionOperadoresVectores(&a1, a2, a->operacionesVector->operador1, variables, var_class, isPop, left))
																	return false;

																v2IsMulti = true;
																return true;
															},
														}, v2);
												},
												[&](multi_value * a1)
												{
														return std::visit(overloaded
														{
															[&](Value & a2)
															{
																if (!OperacionOperadoresVectores(a1, &a2, a->operacionesVector->operador1, variables, var_class, isPop, left))
																	return false;
																return true;
															},
															[&](multi_value * a2)
															{
																//NO DEBERIA SUCEDER.
																	return false;
															},
														}, v2);
												},
											}, v1))
											return false;


											if (a->operacionesVector->dobleOperador)
											{
												if (v2IsMulti)
													return false;

												ValueOrMulti v3 = getValueOrMulti(a->operacionesVector->v3, variables, var_class);

												if (!std::visit(overloaded
													{
														[&](Value & a3)
														{
															if (!OperacionOperadoresVectores(&std::get<Value>(v2), &a3, a->operacionesVector->operador1, isPop, left))
																return false;
															return true;
														},
														[&](multi_value * a3)
														{
															if (!OperacionOperadoresVectores(&std::get<Value>(v2), a3, a->operacionesVector->operador1, variables, var_class, isPop, left))
																return false;

															v2IsMulti = true;
															return true;
														},
													}, v3))
													return false;
														return true;
											}

										return true;
									}
								}

								return entradas[ent_itr].igualdad_Condicional(TratarMultiplesValores(a, variables, var_class));
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
				Value v = lectura_arbol_operacional((*it), variables, var_class);
					this->getRealValueFromValueWrapper(v);
					if (!v.igualdad_Condicional(entradas[ent_itr]))
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

			if (Interprete_Sentencia((*funciones)[*dItr]->body, variables, var_class))
			{
				if (this->ignoreCalled())
					continue;

				Errores::saltarErrores = false;
				delete[] variables;

				//Si la salida es NULA es que no se ha especificado una, la salida será igual al valor de retorno.
				if ((*funciones)[*dItr]->salida == NULL)
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
						retorno.Cast((*funciones)[*dItr]->salida);
						return retorno;
					}
					else
					{

						if ((*funciones)[*dItr]->salida->value != PARAM_VOID)
							Errores::generarWarning(Errores::WARNING_FUNCION_VALOR_DEVUELTO_VOID, &(*funciones)[*dItr]->parametros, (*funciones)[*dItr]->pID->nombre);

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

