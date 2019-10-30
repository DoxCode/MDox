/**
Funciones internas del interprete, se diferenciara de MDOX/Funciones, en que necesitará siempre el acceso a todos los parametros
del interprete mismo, cosa que MDOX/Funciones no tiene.
**/
#include "Interprete.h"

#include <iostream>
#include <string>
#include <sstream>
#include <regex>



bool MDOX_RegexMatch(std::string text, std::string regex)
{
	std::regex e(regex);
	if (std::regex_match(text, e))
		return true;
	return false;
}

std::string Tipo_to_StringRegex(Variable_Runtime * vr)
{
	switch (vr->tipo)
	{

	case PARAM_STRING:
	case PARAM_VOID:
	{
		return MDOX_Regex_Transform::REGEX_TRANSFORM_STRING;
	}
	case PARAM_INT:
	case PARAM_LINT:
		return MDOX_Regex_Transform::REGEX_TRANSFORM_INT;
	case PARAM_DOUBLE:
		return MDOX_Regex_Transform::REGEX_TRANSFORM_DOUBLE;
	case PARAM_BOOL:
		return MDOX_Regex_Transform::REGEX_TRANSFORM_BOOL;
	default:
	{
		Errores::generarError(Errores::ERROR_REGEX_VAR_NOT_AVAILABLE_TRANSFORM, NULL);	
		return "";
	}

	}
}


//Damos por hecho que multivalue no es un contenedor o un vector, antes de llamar a esta función se debe comprobar.
void Interprete::String_to_MultiValue(std::string& text, multi_value* mv, Variable_Runtime* variables, Variable_Runtime* var_class)
{
	std::string regex = "";
	bool last_var = false; // Si la ultima fuer una variable y ponemos otra, por defecto, esperamos un espacio.

	std::vector<Variable_Runtime*> vars;
	vars.reserve(mv->arr.size());

	for (arbol_operacional* a : mv->arr)
	{
		//Value v = lectura_arbol_operacional(a, variables, var_class);
		if (a->operador == OP_NONE)
		{
			//Value, Parser_Identificador*, Call_Value*, arbol_operacional*, multi_value*
			std::visit(overloaded
				{
					[&](Value& a2)
					{
						if (a2.Cast(PARAM_STRING))
						{
							regex += std::get<std::string>(a2.value);
							last_var = false;
						}
					},
					[&](Call_Value * a)
					{ 
						Value v = a->is_class ? ExecClass(a, transformarEntradasCall(a, variables,var_class)) : ExecFuncion(a, transformarEntradasCall(a, variables,var_class),var_class);
						if (v.Cast(PARAM_STRING))
						{
							regex += std::get<std::string>(v.value);
							last_var = false;
						}
					},
					[&](Parser_Identificador * a)
					{
						Variable_Runtime* v = a->var_global ? &this->variables_globales[a->index] : a->var_class ? &var_class[a->index] : a->is_Static ? &a->static_link->static_var_runtime[a->index] : &variables[a->index];
						
						if (a->fuerte)
						{
							v->value.inicializacion(a->tipo);
							v->tipo = a->tipo->value;
						}
						
						if (last_var)
							regex += " ";

						regex += Tipo_to_StringRegex(v);
						last_var = true;
						vars.emplace_back(v);
					},
					[&](auto&)
					{
						Errores::generarError(Errores::ERROR_REGEX_VAR_NOT_AVAILABLE_TRANSFORM, NULL);
						return;
					},
				}, a->_v1);
		}
		else
		{
			Value v = lectura_arbol_operacional(a, variables, var_class);
			std::visit(overloaded
			{
				[&](Variable_Runtime * a2)
				{
					regex += Tipo_to_StringRegex(a2);
					if (last_var)
						regex += " ";
					last_var = true;
					vars.emplace_back(a2);
				},				
				[&](auto&)
				{
					if (v.Cast(PARAM_STRING))
					{
						regex += std::get<std::string>(v.value);
						last_var = false;
					}
				}
			}, v.value);
		}
	}

	try
	{
		std::regex r(regex);
		std::smatch sm;

		if (std::regex_search(text, sm, r))
		{
			if (sm.size() < 1)
			{
				Errores::generarError(Errores::ERROR_INPUT_NOT_MATCH, NULL);
				return;
			}

			for (int i = 1; i < sm.size(); i++)
			{
				if ((i - 1) >= vars.size())
					return;

				Value t = Value(sm[i]);
				if (t.Cast(vars[i - 1]->tipo))
				{
					vars[i - 1]->value = t;
				}
				else
				{
					Errores::generarError(Errores::ERROR_INPUT_NOT_MATCH, NULL);
					return;
				}
			}

			return;
		}
	}
	catch (const std::regex_error & e)
	{
		Errores::generarError(Errores::ERROR_REGEX_INVALID, NULL, e.what());
		return;
	}

	Errores::generarError(Errores::ERROR_INPUT_NOT_MATCH, NULL);
	return;
}

void Interprete::MDOX_StringFormat(std::string val, arbol_operacional* operaciones, Variable_Runtime* variables, Variable_Runtime* var_class)
{

	if (operaciones->operador == OP_NONE)
	{
		std::visit(overloaded
		{
			//Si el esquema es solamente "x" o cualquier otra variable unica.
			// Se le da el valor total del string.
			[&](Parser_Identificador* a)
			{
				Variable_Runtime* identificador = a->var_global ? &this->variables_globales[a->index] : a->var_class ? &var_class[a->index] : &variables[a->index];

				Value value = val;
				if (a->fuerte)
				{
					identificador->value.inicializacion(a->tipo);
					identificador->tipo = a->tipo->value;
					identificador->value.asignacion(value, true);
					return;
				}
				identificador->value.asignacion(value, false);
			},
		//Existen multiples valores, que habrá que tratar más adelante.
			[&](multi_value* a)
			{
				if (a->contenedor || a->is_vector)
				{
					Errores::generarError(Errores::ERROR_FORMAT_NO_ACEPTADA, NULL);
					return;
				}

				String_to_MultiValue(val, a, variables, var_class);

			},
				//Como valores solo aceptaremos strings, que lo tomará siempre como un valore regex
			[&](Value& a) 
			{ 
					std::visit(overloaded{
					[&](std::string& a2) { if(!MDOX_RegexMatch(val, a2)) Errores::generarError(Errores::ERROR_REGEX_NOT_MATCH, NULL, a2, val); },
					[&](std::shared_ptr<mdox_object> a2) { a.asignacion(Value(val)); },
					[](auto&) {Errores::generarError(Errores::ERROR_FORMAT_NO_ACEPTADA, NULL); },
					}, a.value);
			},
			[&](Call_Value* a)
			{
				lectura_arbol_operacional(operaciones, variables, var_class);
			},
			[&](auto&) { Errores::generarError(Errores::ERROR_FORMAT_NO_ACEPTADA, NULL); },
		}, operaciones->_v1);
	}
	else if (operaciones->operador == OPERADORES::OP_CLASS_ACCESS)
	{
		Value value = val;
		lectura_arbol_operacional(operaciones, variables, var_class).asignacion(value);
	}
	else
	{
		Value test = lectura_arbol_operacional(operaciones, variables, var_class);
		std::visit(overloaded{
		[&](std::string& a) { if (!MDOX_RegexMatch(val, a)) Errores::generarError(Errores::ERROR_REGEX_NOT_MATCH, NULL, a, val); },
		[&](std::shared_ptr<mdox_object> a2) { test.asignacion(Value(val)); },
		[&](auto&) { Errores::generarError(Errores::ERROR_FORMAT_NO_ACEPTADA, NULL); },
		}, test.value);

		/*std::visit(overloaded{
			[&](std::string& a2) { if (!MDOX_Regex(val, a2)) Errores::generarError(Errores::ERROR_REGEX_NOT_MATCH, NULL, a2, val); },
			[](auto&) {Errores::generarError(Errores::ERROR_FORMAT_NO_ACEPTADA, NULL); },			
			}, lectura_arbol_operacional(operaciones, variables, var_class).value);	
			*/
	}




}

void Interprete::InputSystem(Sentencia_Input * input, Variable_Runtime* variables, Variable_Runtime* var_class)
{
	std::string str;
	std::getline(std::cin, str);
	MDOX_StringFormat(str, input->pOp, variables, var_class);
}

