
#include "Parser.h"
#include <string>
#include <iomanip>

// ############################################################
// ######################## LITERALES  ######################## 
// ############################################################

Value Parser::getLiteral(bool& v, int& local_index)
{
	int index = local_index;
	//#########  Probando si puede tratarse de un STRING

	if (tokenizer.getTokenValue(index) == "\"")
	{
		std::string value = tokenizer.getTokenValue(index);

		if (value == "\"")
		{
			local_index = index;
			v = true;
			return Value((std::string)"");
		}

		if (tokenizer.getTokenValue(index) == "\"")
		{
			local_index = index;
			v = true;
			ReplaceAll(value, "\\n", "\n");
			return Value(value);
		}
	}

	//######### Probando si puede tratarse de un FLOAT o un ENTERO
	index = local_index;
	std::string token = tokenizer.getTokenValue(index);


	if (is_number(token))
	{
		int l_index = index;
		if (tokenizer.getTokenValue(l_index) == ".")
		{
			std::string token2 = tokenizer.getTokenValue(l_index);
			if (is_number(token2))
			{
				//Es un double
				// Siempre calcularemos doubles, si luego hay que reducirlo a float, se hará
				// en la creación del arbol de acción, nunca en el parser.
				std::string doub = token + "." + token2;

				double value = s2d(doub, std::locale::classic());
				//double value = std::stold(doub);

				local_index = l_index;
				v = true;
				return Value(value);
			}

			//Si llegamos aquí, no es un DOUBLE; pero tampoco un INT.
		}
		else
		{
			//Se trata de un ENTERO
			long long value = std::stoll(token);

			if (value > INT_MAX)
			{
				local_index = index;
				v = true;
				return Value(value);
			}
			else
			{
				local_index = index;
				v = true;
				return Value((int)value);
			}

			v = false;
			return Value();
		}
	}

	//######### Probando si puede tratarse de un booleano
	index = local_index;
	token = tokenizer.getTokenValue(index);
	if (token == "true")
	{
		v = true;
		local_index = index;
		return Value(true);
	}
	else if (token == "false")
	{
		v = true;
		local_index = index;
		return Value(false);
	}

	v = false;
	return Value();
}

// ############################################################
// ###################### DECLARATIVOS  ####################### 
// ############################################################

Parser_Declarativo* Parser::getDeclarativo(int& local_index)
{
	int index = local_index;
	std::string token = tokenizer.getTokenValue(index);


	if (token == "vector")
	{
		Parser_Declarativo* p = new Parser_Declarativo(PARAM_VECTOR);
		local_index = index;
		return p;
	}
	else if (token == "string")
	{
		Parser_Declarativo* p = new Parser_Declarativo(PARAM_STRING);
		local_index = index;
		return p;
	}
	else if (token == "int")
	{
		Parser_Declarativo* p = new Parser_Declarativo(PARAM_INT);
		local_index = index;
		return p;
	}
	else if (token == "lint")
	{
		Parser_Declarativo* p = new Parser_Declarativo(PARAM_LINT);
		local_index = index;
		return p;
	}
	else if (token == "double")
	{
		Parser_Declarativo* p = new Parser_Declarativo(PARAM_DOUBLE);
		local_index = index;
		return p;
	}
	else if (token == "bool")
	{
		Parser_Declarativo* p = new Parser_Declarativo(PARAM_BOOL);
		local_index = index;
		return p;
	}
	else if (token == "void")
	{
		Parser_Declarativo* p = new Parser_Declarativo(PARAM_VOID);
		local_index = index;
		return p;
	}

	//	index = local_index;
	return NULL;
}

// ############################################################
// ##################### IDENTIFICADOR  ####################### 
// ############################################################

Parser_Identificador* Parser::getIdentificador(int& local_index)
{
	int index = local_index;

	Parser_Declarativo* d = getDeclarativo(index);

	if (d)
	{
		deletePtr(d);
		return NULL;
	}

	index = local_index;


	bool l;
	getLiteral(l, index);

	if (l)
	{
		return NULL;
	}

	//TODO:
	//Habría que comprobar también que no se trate de una función específica del tipo:
	// return, break, if, else, while ... 

	index = local_index;
	std::string token = tokenizer.getTokenValue(index);

	if (is_Identificador(token))
	{
		local_index = index;
		Parser_Identificador* sif = new Parser_Identificador(token);
		sif->generarPosicion(&tokenizer);
		return sif;
	}

	return NULL;

}

// ############################################################
// ######################### VALOR  ########################### 
// ############################################################


multi_value* Parser::getValorList(bool& all_value, int& local_index, std::vector<Variable>& variables)
{
	int index = local_index;

	multi_value* mv = new multi_value();

	while (true)
	{
		int index2 = index;
		arbol_operacional* c = getOperacion(index2, variables, false);

		if (!c)
		{
			delete mv;
			return NULL;
		}

		if (c->operador == OP_NONE && all_value)
		{
			std::visit(overloaded
				{
					[&](Value & a) {mv->arr.emplace_back(std::move(a)); },
					[&](auto & a) { mv->arr.emplace_back(std::move(a));  all_value = false; },
				}, c->_v1);
			//delete c;
		}
		else
		{
			all_value = false;
			if(c->operador == OP_NONE)
				mv->arr.emplace_back(std::move(c->_v1));
			else mv->arr.emplace_back(std::move(c));
		//	delete c;
		}

		if (tokenizer.getTokenValue(index2) == ",")
		{
			index = index2;
			continue;
		}
		else
		{
			index2--;
			local_index = index2;
			return mv;
		}
	}

}

conmp Parser::getValor(bool& ret, int& local_index, std::vector<Variable>& variables)
{
	int index = local_index;
	ret = true;

	//Comprobamos que sea un literal.
	bool l;
	Value v1 = getLiteral(l, index);

	if (l)
	{
		local_index = index;
		return std::move(v1);
	}

	// ###### Comprobamos si se trata de un declarativo `+ Identificador ######
	// Declarativo + Identificados -> int varA
	index = local_index;

	Parser_Declarativo* pD1 = getDeclarativo(index);

	//Si el primer valor es un declarativo, puede tratarse de un declarativo + identificador
	// o de algún tipo de puntero. <<Punteros aún no implementados.>>
	if (pD1)
	{
		int index2 = index;
		Parser_Identificador* pID = getIdentificador(index2);

		if (pID)
		{
			local_index = index2;

			pID->fuerte = true;
			pID->tipo = pD1;
			pID->generarPosicion(&tokenizer);
			return pID;
		}

		deletePtr(pD1);
		//Comprobar PUNTEROS
	}

	//Comprobamos si se trata de un vector
	index = local_index;

	std::string val_pre = tokenizer.getTokenValue(index);


	if (val_pre == "[")
	{
		int indexX = index;
		if (tokenizer.getTokenValue(index) == "]")
		{
			local_index = index; 
			return Value(std::make_shared<mdox_vector>());
		}

		index = indexX;

		multi_value* contenedor_operacion_vector = new multi_value();
		contenedor_operacion_vector->contenedor = true;

		bool is_vector = true;
		bool all_value = true;

			int index2 = index;
			multi_value* c = getValorList(all_value, index2, variables);

			if (!c)
			{
				delete contenedor_operacion_vector;
				ret = false;
				return std::monostate();
			}

			std::string ss_v = tokenizer.getTokenValue(index2);
			if (ss_v == "]")
			{
				local_index = index2;
				if (is_vector)
				{
					c->is_vector = true;
					if (all_value)
					{
						std::shared_ptr<mdox_vector> res = std::make_shared<mdox_vector>();
						res->vector.resize(c->arr.size());
						int itr = 0;
						for (std::vector<tipoValor>::iterator it = c->arr.begin(); it != c->arr.end(); ++it)
						{
							res->vector[itr] = std::get<Value>(*it);
							itr++;
						}
						delete c;
						return Value(std::move(res));
					}
					return c;
				}

				//Aqui no deberia poder llegar.
				delete c;
				delete contenedor_operacion_vector;

				ret = false;
				return std::monostate();

			}
			else if (ss_v == ":" || ss_v == "::")
			{
				index2--;
				OPERADORES operador = getOperador(index2);

				int indexRet = index2;
				multi_value* nt2 = getValorList(all_value, indexRet, variables);

				if (nt2 == NULL)
				{
					delete c;
					delete contenedor_operacion_vector;
					ret = false;
					return std::monostate();
				}

				ss_v = tokenizer.getTokenValue(indexRet);

				if (ss_v == "]") // Solo hay 1 operador: x:xs ó x::xs -> Donde el vector puede ser x ó xs
				{
					if (c->arr.size() == 1 && nt2->arr.size() == 1)
					{
						contenedor_operacion_vector->operacionesVector = new OperacionesEnVector(c->arr[0], nt2->arr[0], operador);
						c->arr.clear();
						nt2->arr.clear();
						delete c;
						delete nt2;
					}
					else if (c->arr.size() == 1)
					{
						contenedor_operacion_vector->operacionesVector = new OperacionesEnVector(c->arr[0], nt2, operador);
						c->arr.clear();
						delete c;
					}
					else if (nt2->arr.size() == 1)
					{
						contenedor_operacion_vector->operacionesVector = new OperacionesEnVector(c, nt2->arr[0], operador);
						nt2->arr.clear();
						delete nt2;
					}
					else
					{
						contenedor_operacion_vector->operacionesVector = new OperacionesEnVector(c, nt2, operador);
					}

					local_index = indexRet;
					return contenedor_operacion_vector;
				}
				else if (ss_v == ":" || ss_v == "::")
				{
					indexRet--;
					OPERADORES operador2 = getOperador(indexRet);
					multi_value* nt3 = getValorList(all_value, indexRet, variables);

					if (nt3 == NULL)
					{
						delete c;
						delete nt2;
						delete contenedor_operacion_vector;
						ret = false;
						return std::monostate();
					}

					ss_v = tokenizer.getTokenValue(indexRet);

					if (ss_v == "]")
					{
						if (c->arr.size() == 1 && nt2->arr.size() == 1 && nt3->arr.size() == 1)
						{
							contenedor_operacion_vector->operacionesVector = new OperacionesEnVector(c->arr[0], nt2->arr[0], operador, nt3->arr[0], operador2);
							c->arr.clear();
							nt2->arr.clear();
							nt3->arr.clear();
							delete c;
							delete nt2;
							delete nt3;
						}
						else if (c->arr.size() == 1 && nt2->arr.size() == 1)
						{
							contenedor_operacion_vector->operacionesVector = new OperacionesEnVector(c->arr[0], nt2->arr[0], operador, nt3, operador2);
							c->arr.clear();
							nt2->arr.clear();
							delete c;
							delete nt2;
						}
						else if (c->arr.size() == 1 && nt3->arr.size() == 1)
						{
							contenedor_operacion_vector->operacionesVector = new OperacionesEnVector(c->arr[0], nt2, operador, nt3->arr[0], operador2);
							c->arr.clear();
							nt3->arr.clear();
							delete c;
							delete nt3;
						}
						else if (nt2->arr.size() == 1 && nt3->arr.size() == 1)
						{
							contenedor_operacion_vector->operacionesVector = new OperacionesEnVector(c, nt2->arr[0], operador, nt3->arr[0], operador2);
							nt2->arr.clear();
							nt3->arr.clear();
							delete nt2;
							delete nt3;
						}
						else if (nt2->arr.size() == 1)
						{
							contenedor_operacion_vector->operacionesVector = new OperacionesEnVector(c, nt2->arr[0], operador, nt3, operador2);
							nt2->arr.clear();
							delete nt2;
						}
						else if (nt3->arr.size() == 1)
						{
							contenedor_operacion_vector->operacionesVector = new OperacionesEnVector(c, nt2, operador, nt3->arr[0], operador2);
							nt3->arr.clear();
							delete nt3;
						}
						else if (c->arr.size() == 1)
						{
							contenedor_operacion_vector->operacionesVector = new OperacionesEnVector(c->arr[0], nt2, operador, nt3, operador2);
							c->arr.clear();
							delete c;
						}
						else
						{
							contenedor_operacion_vector->operacionesVector = new OperacionesEnVector(c, nt2, operador, nt3, operador2);
						}

						local_index = indexRet;
						return contenedor_operacion_vector;
					}
					else
					{
						delete c;
						delete contenedor_operacion_vector;
						delete nt2;
						delete nt3;
						ret = false;
						return std::monostate();
					}
				}
			}
			else
			{
				delete c;
				delete contenedor_operacion_vector;

				ret = false;
				return std::monostate();
			}
		
	}


	//En caso de no ser un literal, podrá ser o una llamada a una función o un identificador.
	index = local_index;
	Parser_Identificador* i = getIdentificador(index);

	if (i)
	{
		int i_index = index;
		if (tokenizer.getTokenValue(i_index) == "(") // Identificador (x1,x2,x3...) -> LLamada  a una función. Entradas = parámetros
		{
			std::vector<arbol_operacional*> entradas;

			while (true)
			{
				int t_index = i_index;
				arbol_operacional* p = getOperacion(t_index, variables, true);

				if (p)
				{
					entradas.push_back(p);
					std::string token = tokenizer.getTokenValue(t_index);
					i_index = t_index;

					if (token == ",")
					{
						continue;
					}
					else if (token == ")")
					{
						Valor_Funcion* v = new Valor_Funcion(i, entradas);
						local_index = i_index;
						v->generarPosicion(&tokenizer);
						return v;
					}
					else
					{
						for (std::vector<arbol_operacional*>::iterator it = entradas.begin(); it != entradas.end(); ++it)
						{
							deletePtr(*it);
						}
						deletePtr(i);
						break;
					}
				}
				else
				{
					std::string token = tokenizer.getTokenValue(t_index);

					if (token == ")")
					{
						Valor_Funcion* v = new Valor_Funcion(i, entradas);
						local_index = t_index;
						v->generarPosicion(&tokenizer);
						return v;
					}

					for (std::vector<arbol_operacional*>::iterator it = entradas.begin(); it != entradas.end(); ++it)
					{
						deletePtr(*it);
					}
					deletePtr(i);
					break;
				}
			}
		}
		else //Es un IDENTIFICADOR
		{
			local_index = index;
			return i;
		}
	}


	index = local_index;

	ret = false;
	return std::monostate();
}

// ############################################################
// ####################### OPERACIÓN  #########################
// ############################################################

OPERADORES Parser::getOperador(int& local_index)
{

	std::string v = tokenizer.getTokenValue(local_index);


	if (v == "(") return OPERADORES::OP_SCOPE_LEFT;
	else if (v == ")") return OPERADORES::OP_SCOPE_RIGHT;

	else if (v == "[") return OPERADORES::OP_BRACKET_LEFT;
	else if (v == "]") return OPERADORES::OP_BRACKET_RIGHT;

	else if (v == "!") return OPERADORES::OP_NEGADO;
	else if (v == "++") return OPERADORES::OP_ITR_PLUS;
	else if (v == "--") return OPERADORES::OP_ITR_MIN;

	else if (v == "+") return OPERADORES::OP_ARIT_SUMA;
	else if (v == "-") return OPERADORES::OP_ARIT_RESTA;
	else if (v == "*") return OPERADORES::OP_ARIT_MULT;
	else if (v == "/") return OPERADORES::OP_ARIT_DIV;
	else if (v == "div") return OPERADORES::OP_ARIT_DIV_ENTERA;
	else if (v == "%") return OPERADORES::OP_ARIT_MOD;

	else if (v == "=") return OPERADORES::OP_IG_EQUAL;
	else if (v == "+=") return OPERADORES::OP_IG_EQUAL_SUM;
	else if (v == "-=") return OPERADORES::OP_IG_EQUAL_MIN;
	else if (v == "*=") return OPERADORES::OP_IG_EQUAL_MULT;
	else if (v == "/=") return OPERADORES::OP_IG_EQUAL_DIV;
	else if (v == "%=") return OPERADORES::OP_IG_EQUAL_MOD;

	else if (v == "<") return OPERADORES::OP_REL_MINOR;
	else if (v == ">") return OPERADORES::OP_REL_MAJOR;
	else if (v == "<=") return OPERADORES::OP_REL_MINOR_OR_EQUAL;
	else if (v == ">=") return OPERADORES::OP_REL_MAJOR_OR_EQUAL;
	else if (v == "==") return OPERADORES::OP_REL_EQUAL;
	else if (v == "!=") return OPERADORES::OP_REL_NOT_EQUAL;

	else if (v == "&&") return OPERADORES::OP_LOG_ADD;
	else if (v == "||") return OPERADORES::OP_LOG_OR;

	else if (v == ":") return OPERADORES::OP_POP_ADD;
	else if (v == "::") return OPERADORES::OP_CHECK_GET;

	else
	{
		local_index--;
		return OP_NONE;
	}
}


bool scope_right_do(int& left_scope, int& is_op, stack_conmp& stack, std::vector<OPERADORES>& op_stack)
{
	is_op = 2;
	left_scope--;
	if (left_scope < 0)
	{
		return false;
	}

	while (!op_stack.empty())
	{
		OPERADORES c = op_stack.back();

		if (c != OPERADORES::OP_SCOPE_LEFT)
		{
			stack.push_back(c);
			op_stack.pop_back();
			continue;
		}
		op_stack.pop_back();
		break;
	}
	return true;
}


int getLeftPosition(int right, stack_conmp& res)
{
	int left = right + 1;

	bool r_op = std::visit(overloaded{
	[](auto) {return false; },
	[](OPERADORES & val) {return true; },
		}, *(res.end() - (right)));

	if (r_op)
	{
		int pasos = 2;
		int itr;
		for (itr = (right + 1); pasos > 0; itr++)
		{
			std::visit(overloaded{
				[&](auto) {},
				[&](OPERADORES & val) { pasos += 2; },
				}, *(res.end() - itr));

			--pasos;
		}

		left = itr;
	}

	return left;
}

bool operadores_compatibles(OPERADORES & a, OPERADORES & b)
{
	if (a == b)
		return true;

	if (a == OPERADORES::OP_ARIT_SUMA && b == OPERADORES::OP_ARIT_RESTA)
		return true;
	else if (b == OPERADORES::OP_ARIT_SUMA && a == OPERADORES::OP_ARIT_RESTA)
		return true;
	else return false;
}

void normal_operator_do(int& left_scope, int& is_op, OPERADORES & aux, stack_conmp & stack, std::vector<OPERADORES> & op_stack)
{
	is_op = 1;

	if (op_stack.empty() || prioridad(op_stack.back()) < prioridad(aux))
	{
		op_stack.push_back(aux);
	}
	else
	{
		while (true)
		{
			if (op_stack.empty() || prioridad(op_stack.back()) < prioridad(aux) || op_stack.back() == OPERADORES::OP_SCOPE_LEFT)
				break;

			stack.push_back(op_stack.back());
			op_stack.pop_back();
		}
		op_stack.push_back(aux);
	}
}

arbol_operacional* GenerarArbolDesdeShuntingYard(stack_conmp & res)
{
	conmp a = res.back();
	arbol_operacional* arbol = NULL;

	if (res.size() == 1)
	{
		if (std::visit(overloaded{
			 [](const OPERADORES&) { return false; },
			 [&](std::shared_ptr<Value> & v) {  arbol = new arbol_operacional(*v); return true;  },
			 [&](std::monostate&) { return false; },
			 [&](const auto & v) { arbol = new arbol_operacional(v); return true; },
			}, res.back())
			) return arbol;
		else return NULL;
	}

	bool is_ok = std::visit(overloaded{
		 [](auto&) { return false; },
		 [&](OPERADORES op)
		 {
			res.pop_back();
			arbol = new arbol_operacional(op);


			//VALOR 1 -> Empezando desde atras, seria el v2
			if (res.size() == 0)
				return false;

			bool r1 = std::visit(overloaded{
				[&](auto & v) { arbol->_v2 = v; res.pop_back(); return true; },
				[&](std::shared_ptr<Value> & v) { arbol->_v2 = *v; res.pop_back(); return true; },
				[&](OPERADORES & op)
				{
					auto a = GenerarArbolDesdeShuntingYard(res);
					if (a)
						arbol->_v2 = a;
					else return false;
					return true;
				},
			}, res.back());

			if (!r1)
				return false;

			//VALOR 2 -> Empezando desde atras, seria el v1
			if (!is_single_operator(op))
			{
				if (res.size() == 0)
					return false;

				return std::visit(overloaded{
				[&](auto & v) { arbol->_v1 = v; res.pop_back(); return true; },
				[&](std::shared_ptr<Value> & v) { arbol->_v1 = *v; res.pop_back(); return true; },
				[&](OPERADORES & op)
				{
					arbol_operacional* a = GenerarArbolDesdeShuntingYard(res);



					/*if (is_assignment_operator(op))
					{
						if (std::visit(overloaded{
							[](Parser_Identificador * a) { return true; },
							[](auto&) { return false; },
							}, arbol->_v2))
						{
							return false;
						}
					}*/


					if (a != NULL)
						arbol->_v1 = a;
					else return false;

					return true;
				},
				}, res.back());
			}

			return true;
		 }
		}, a);

	if (is_ok)
		return arbol;

	deletePtr(arbol);
	return NULL;

}



arbol_operacional* Parser::getOperacionInd(int& local_index, std::vector<Variable>& variables, bool inside)
{
	int index = local_index;
	arbol_operacional * Op = getOperacion(index, variables, inside);

	if (Op)
	{
		std::string next_token = tokenizer.getTokenValue(index);
		if (next_token == ",")
		{
			multi_value* mv = new multi_value();
			mv->arr.emplace_back(Op);

			do {
				arbol_operacional * t1 = getOperacion(index, variables, inside);
				if (t1)
				{
					mv->arr.emplace_back(t1);
				}
				else
				{
					delete mv;
					return NULL;
				}

				next_token = tokenizer.getTokenValue(index);

			} while (next_token == ",");

			index--;
			local_index = index;
			return new arbol_operacional(mv);
		}

		index--;
		local_index = index;
		return Op;
	}

	return NULL;
}


arbol_operacional * Parser::getOperacion(int& local_index, std::vector<Variable> & variables, bool inside)
{
	int index = local_index;
	//Posibilidades
	// TOKEN + XXX  ->  -2+4<2..., !b && b + 2...
	// VALUE + XXX  ->   x = 3+d...,  4+3*3-(2+1)

	//Seguiremos un esquema similar al Algoritmo Shunting Yard, con varios
	//cambios orientados al mejor rendimiento en tiempo de interprete a cambio de tiempo de parseado.
	stack_conmp stack;
	std::vector<OPERADORES> op_stack;


	// TIPOS 0 -> Puede ser cualquier cosa. (left <- No existe o fue un parentesis)
	// TIPOS 1 -> Puede ser un Valor ó un paréntesis de inicio. (left <- Fue un operador) EJ: 3 +  ó 3(xx) 3( = 3*(
	// TIPOS 2 -> Puede ser un Operador (left <- Fue un Valor)
	//

	int is_op = 0;
	int left_scope = 0;

	//std::cout << "#1. \n";
	while (true)
	{
		if (is_op == 0)
		{
			OPERADORES aux = this->getOperador(index);
			if (aux == OPERADORES::OP_NONE)
			{
				bool ok;
				conmp pV = this->getValor(ok, index, variables);

				if (!ok)
				{
					return NULL;
				}
				stack.push_back(std::move(pV));
				is_op = 2;
				continue;
			}
			else if (aux == OPERADORES::OP_BRACKET_LEFT)
			{
				index--;
				bool ok;
				conmp pV = this->getValor(ok, index, variables);

				if (!ok)
				{
					return NULL;
				}
				stack.push_back(std::move(pV));
				is_op = 2;
				continue;
			}
			else if (aux == OPERADORES::OP_SCOPE_LEFT)
			{
				is_op = 0;
				left_scope++;
				op_stack.push_back(aux);
				continue;
			}
			else if (aux == OPERADORES::OP_SCOPE_RIGHT)
			{
				if (scope_right_do(left_scope, is_op, stack, op_stack))
					continue;
				else return NULL;
			}
			else
			{
				if (is_left(aux))
				{
					if (transform_left(aux))
						normal_operator_do(left_scope, is_op, aux, stack, op_stack);
					else continue;
				}
				else
				{
					return NULL;
				}

				continue;
			}
		}
		else if (is_op == 1) //Anterior, fue un operador.
		{
			bool ok;
			conmp pV = this->getValor(ok, index, variables);

			if (!ok)
			{
				OPERADORES aux = this->getOperador(index);
				if (aux == OPERADORES::OP_NONE)
				{
					break;
				}

				if (aux == OPERADORES::OP_SCOPE_LEFT)
				{
					is_op = 0;
					op_stack.push_back(aux);
					left_scope++;
					continue;
				}
				else if (aux == OPERADORES::OP_SCOPE_RIGHT)
				{
					if (op_stack.back() == OPERADORES::OP_ITR_MIN || op_stack.back() == OPERADORES::OP_ITR_PLUS)
					{
						if (scope_right_do(left_scope, is_op, stack, op_stack))
							continue;
						else
						{
							if (left_scope < 0)
							{
								index--;
								break;
							}
							else return NULL;
						}
					}

					return NULL;
				}
				else if (is_assignment_operator(op_stack.back()) || isRelationalOperator(op_stack.back()))
				{
					if (is_left(aux))
					{
						if (transform_left(aux))
							normal_operator_do(left_scope, is_op, aux, stack, op_stack);
						else continue;

					}
					else
					{
						return NULL;
					}
					continue;
				}
				else	//No puede haber dos operadores seguidos, de haberlos, no es una operación EJ: 5+2+-3
				{
					return NULL;
				}

				return NULL;
			}

			stack.push_back(std::move(pV));
			is_op = 2;
			continue;
		}
		else if (is_op == 2) //Anterior, fue un valor.
		{
			OPERADORES aux = this->getOperador(index);
			if (aux == OPERADORES::OP_NONE)
			{
				break;
			}
			else if (aux == OPERADORES::OP_SCOPE_LEFT)
			{
				is_op = 0;
				left_scope++;
				OPERADORES a = OPERADORES::OP_ARIT_MULT;
				normal_operator_do(left_scope, is_op, a, stack, op_stack);
				op_stack.push_back(aux);
				continue;
			}
			else if (aux == OPERADORES::OP_SCOPE_RIGHT)
			{
				if (scope_right_do(left_scope, is_op, stack, op_stack))
					continue;
				else
				{
					if (left_scope < 0)
					{
						index--;
						break;
					}
					else return NULL;
				}
			}
			else if (aux == OPERADORES::OP_BRACKET_LEFT)
			{			
				normal_operator_do(left_scope, is_op, aux, stack, op_stack);
				is_op = 0;
				op_stack.push_back(OPERADORES::OP_SCOPE_LEFT);
				left_scope++;
				continue;
			}
			else if (aux == OPERADORES::OP_BRACKET_RIGHT)
			{
				if (scope_right_do(left_scope, is_op, stack, op_stack))
					continue;
				else
				{
					if (left_scope < 0)
					{
						index--;
						break;
					}
					else return NULL;
				}
				continue;
			}
			else if (aux == OPERADORES::OP_POP_ADD || aux == OPERADORES::OP_CHECK_GET)
			{
				index--;
				break;
			}
			else
			{
				normal_operator_do(left_scope, is_op, aux, stack, op_stack);
				continue;
			}
		}
	}


	if (left_scope > 0) // Fallos con los parentesis.
	{
		return NULL;
	}
	//std::cout << "#3. \n";
	while (!op_stack.empty())
	{
		stack.push_back(op_stack.back());
		op_stack.pop_back();
	}
	//std::cout << "#4. \n";


	//DEBUG  
	//Value, Parser_Identificador*, Valor_Funcion*, OPERADORES
	/*for (auto it = stack.begin(); it != stack.end(); ++it)
	{
		std::visit(overloaded{
			 [](auto) {std::cout << "-Var o Funcion-"; },
			 [](Value & a) {  a.print(); std::cout << ", "; },
			 [](Parser_Identificador * a) { std::cout << a->nombre << ", "; },
			 [](OPERADORES & a) { std::cout << "OP: " << a << ", ";  },
			}, *it);
	}*/


	stack_conmp res;

	while (!stack.empty())
	{
		bool _aerr = std::visit(overloaded{
		 [&](auto)
			{
				res.push_back(stack.front());
				stack.pop_front();
				return true;
			},
		 [&](OPERADORES & a)
			{
				if (is_single_operator(a))
				{
					if (res.size() < 1)
						return false;

					std::visit(overloaded{
						 [&](auto) {res.push_back(stack.front()); stack.pop_front(); },
						 [&](Value b)
						{
							res.pop_back();
							res.push_back(b.operacion_Unitaria(a));
							stack.pop_front();
						},
					}, res.back());
					return true;
				}
				else if (is_assignment_operator(a))
				{
					res.push_back(stack.front());
					stack.pop_front();
					return true;
				}
				else if (isRelationalOperator(a))
				{
					res.push_back(stack.front());
					stack.pop_front();
					return true;
				}
				else
				{
					if (res.size() < 2)
						return false;

					return std::visit(overloaded{
						//cb
						 [&](Value & b,Value & c)
							{
								Value _op = c.operacion_Binaria(b, a);
								res.pop_back();
								res.pop_back();

								res.push_back(_op);
								stack.pop_front();
								return true;
							},
							[&](auto b,auto c)
							{
								res.push_back(stack.front());
								stack.pop_front();
								return true;
							},
							}, res.back(),*(res.end() - 2));
					}
				return false;
				},
			}, stack.front());


		if (!_aerr)
			return NULL;
	}

	/*	std::cout << "\n #DEBUG 2. \n";
		//DEBUG
		for (auto it = res.begin(); it != res.end(); ++it)
		{
			std::visit(overloaded{
				 [](auto) {std::cout << "-Var o Funcion-"; },
				 [](Value & a) { a.print(); std::cout << ", "; },
				 [](Parser_Identificador * a) { std::cout << a->nombre << ", "; },
				 [](OPERADORES & a) { std::cout << "OP: " << a << ", ";  },
				}, *it);
		}
		*/
	local_index = index;

	arbol_operacional* rr = GenerarArbolDesdeShuntingYard(res);

	if (rr == NULL)
	{
		existenErrores = true;
		return NULL;
	}


	//Preload de variables.
	CargarEnCacheOperaciones(rr, variables, inside);

	if (existenErrores)
		return NULL;


	return rr;
}



// ############################################################
// ####################### SENTENCIA  #########################
// ############################################################

Parser_Sentencia* Parser::getSentencia(int& local_index, std::vector<Variable> & variables)
{
	//##########   -- SENTENCIA RECURSIVA --   ##########
	int index = local_index;

	if (tokenizer.getTokenValue(index) == "{")
	{
		std::vector<Parser_Sentencia*> valor;
		//Copiamos todas las variables actuales	
		std::vector<Variable> variables_local(variables);

		while (true)
		{
			int t_index = index;

			Parser_Sentencia* pS = getSentencia(t_index, variables_local);

			if (pS)
			{
				valor.push_back(pS);

				index = t_index;

				if (tokenizer.getTokenValue(t_index) == "}")
				{
					local_index = t_index;
					Sentencia_Recursiva* sif = new Sentencia_Recursiva(valor);
					sif->generarPosicion(&tokenizer);
					return sif;
				}
			}
			else
			{
				local_index = index;
				return NULL;
			}
		}
	}

	//##########   -- SENTENCIA IF --   ##########
	index = local_index;

	if (tokenizer.getTokenValue(index) == "if")
	{
		if (tokenizer.getTokenValue(index) == "(")
		{
			arbol_operacional* pCond = getOperacion(index, variables, true);

			if (pCond)
			{
				if (tokenizer.getTokenValue(index) == ")")
				{
					Parser_Sentencia* pSent = getSentencia(index, variables);

					if (pSent)
					{
						int t_index = index;

						if (tokenizer.getTokenValue(t_index) == "else")
						{
							Parser_Sentencia* pElse = getSentencia(t_index, variables);

							if (pElse)
							{
								local_index = t_index;
								Sentencia_IF* sif = new Sentencia_IF(pCond, pSent, pElse);
								sif->generarPosicion(&tokenizer);
								return sif;
							}
							else
							{
								deletePtr(pSent);
								deletePtr(pCond);
								return NULL;
							}
						}
						else
						{
							local_index = index;
							Sentencia_IF* sif = new Sentencia_IF(pCond, pSent);
							sif->generarPosicion(&tokenizer);
							return sif;
						}

						deletePtr(pSent);
					}
				}

				deletePtr(pCond);
			}
			else/*else condicional*/ return NULL;
		}
	}

	//##########   -- SENTENCIA WHILE --   ##########
	index = local_index;

	if (tokenizer.getTokenValue(index) == "while")
	{
		if (tokenizer.getTokenValue(index) == "(")
		{
			arbol_operacional* pCond = getOperacion(index, variables, true);

			if (pCond)
			{
				if (tokenizer.getTokenValue(index) == ")")
				{
					Parser_Sentencia* pSent = getSentencia(index, variables);

					if (pSent)
					{
						local_index = index;
						Sentencia_WHILE* sif = new Sentencia_WHILE(pCond, pSent);
						sif->generarPosicion(&tokenizer);
						return sif;
					}
					else
					{
						deletePtr(pCond);
						return NULL;
					}
				}

				deletePtr(pCond);
			}
			else return NULL;
		}
	}

	//##########   -- SENTENCIA FOR --   ##########
	index = local_index;

	if (tokenizer.getTokenValue(index) == "for")
	{
		if (tokenizer.getTokenValue(index) == "(")
		{
			arbol_operacional* pIg = getOperacion(index, variables, true);
			if (tokenizer.getTokenValue(index) == ";")
			{
				arbol_operacional* pCond = getOperacion(index, variables, true);

				if (tokenizer.getTokenValue(index) == ";")
				{
					arbol_operacional* pOp = getOperacion(index, variables, true);

					if (tokenizer.getTokenValue(index) == ")")
					{
						Parser_Sentencia* pSent = getSentencia(index, variables);

						if (pSent)
						{
							local_index = index;
							Sentencia_FOR* sif = new Sentencia_FOR(pIg, pCond, pOp, pSent);
							sif->generarPosicion(&tokenizer);
							return sif;
						}
						else
						{
							deletePtr(pOp);
							deletePtr(pCond);
							deletePtr(pIg);
							return NULL;
						}
					}
					else
					{
						deletePtr(pOp);
						deletePtr(pCond);
						deletePtr(pIg);
						return NULL;
					}
				}
				else
				{
					deletePtr(pCond);
					deletePtr(pIg);
					return NULL;
				}
			}
			else
			{
				deletePtr(pIg);
				return NULL;
			}
		}
	}

	//##########   -- SENTENCIA RETURN --   ##########
	index = local_index;

	if (tokenizer.getTokenValue(index) == "return")
	{
		arbol_operacional* pOp = getOperacion(index, variables, true);

		if (pOp)
		{
			if (tokenizer.getTokenValue(index) == ";")
			{
				local_index = index;
				Sentencia_Return* sif = new Sentencia_Return(pOp);
				sif->generarPosicion(&tokenizer);
				return sif;
			}
			deletePtr(pOp);
		}
		else
		{
			if (tokenizer.getTokenValue(index) == ";")
			{
				Sentencia_Return* sif = new Sentencia_Return(NULL);
				sif->generarPosicion(&tokenizer);
				return sif;
			}
		}
	}

	//##########   -- SENTENCIA PRINT --   ##########
	index = local_index;
	if (tokenizer.getTokenValue(index) == "<::")
	{
		arbol_operacional* pOp = getOperacion(index, variables, true);
		if (pOp)
		{
			if (tokenizer.getTokenValue(index) == ";")
			{
				local_index = index;
				Sentencia_Print* sif = new Sentencia_Print(pOp);
				sif->generarPosicion(&tokenizer);
				return sif;
			}
			deletePtr(pOp);
		}
	}


	//##########   -- SENTENCIA OPERACIONAL --   ##########
	index = local_index;

	arbol_operacional* pOp = getOperacionInd(index, variables); //IND : Una o varias operaciones.

	if (pOp)
	{
		if (tokenizer.getTokenValue(index) == ";")
		{
			local_index = index;
			Sentencia_Operacional* sif = new Sentencia_Operacional(pOp);
			sif->generarPosicion(&tokenizer);
			return sif;
		}
		deletePtr(pOp);
	}


	index = local_index;
	return NULL;
}

// ############################################################
// ######################## FUNCION  ##########################
// ############################################################

Parser_Funcion* Parser::getFuncion(int& local_index)
{
	int index = local_index;


	if (tokenizer.getTokenValue(index) == "function")
	{
		Parser_Identificador* pID = getIdentificador(index);

		if (pID)
		{
			if (tokenizer.getTokenValue(index) == "(")
			{

				//Variables por función
				this->numero_variables_funcion = 0;
				this->isGlobal = false;
				std::vector<Variable> variables_funcion;

				std::vector<arbol_operacional*> entradas;

				int t_index = index;
				while (true)
				{
					int t2_index = t_index;
					arbol_operacional* pFve = getOperacion(t2_index, variables_funcion);

					if (pFve)
					{
						entradas.emplace_back(pFve);

						std::string token = tokenizer.getTokenValue(t2_index);

						if (token == ",")
						{
							t_index = t2_index;
							continue;
						}
						else if (token == ")")
						{
							index = t2_index;
							break;
						}
						else
						{
							deletePtr(pID);

							for (std::vector<arbol_operacional*>::iterator it = entradas.begin(); it != entradas.end(); ++it)
							{
								deletePtr(*it);
							}

							return NULL;
						}
					}
					else
					{
						if (tokenizer.getTokenValue(t2_index) != ")")
						{
							deletePtr(pID);

							for (std::vector<arbol_operacional*>::iterator it = entradas.begin(); it != entradas.end(); ++it)
							{
								deletePtr(*it);
							}

							return NULL;
						}

						index = t2_index;
						break;
					}
				}

				int t3_index = index;

				if (tokenizer.getTokenValue(t3_index) == "::")
				{
					Parser_Declarativo* pDecl = getDeclarativo(t3_index);

					if (pDecl)
					{
						Parser_Sentencia* pSent = getSentencia(t3_index, variables_funcion);

						if (pSent)
						{
							local_index = t3_index;
							Parser_Funcion* sif = new Parser_Funcion(pID, entradas, pSent, pDecl);
							sif->preload_var = this->numero_variables_funcion;
							sif->generarPosicion(&tokenizer);
							return sif;
						}
						else
						{
							deletePtr(pDecl);
							deletePtr(pID);

							for (std::vector<arbol_operacional*>::iterator it = entradas.begin(); it != entradas.end(); ++it)
							{
								deletePtr(*it);
							}
							return NULL;
						}
					}
					else
					{
						deletePtr(pID);

						for (std::vector<arbol_operacional*>::iterator it = entradas.begin(); it != entradas.end(); ++it)
						{
							deletePtr(*it);
						}
						return NULL;
					}
				}
				else
				{
					Parser_Sentencia* pSent = getSentencia(index, variables_funcion);
					if (pSent)
					{
						local_index = index;
						Parser_Funcion* sif = new  Parser_Funcion(pID, entradas, pSent);
						sif->preload_var = this->numero_variables_funcion;
						sif->generarPosicion(&tokenizer);
						return sif;
					}
					else
					{
						deletePtr(pID);

						for (std::vector<arbol_operacional*>::iterator it = entradas.begin(); it != entradas.end(); ++it)
						{
							deletePtr(*it);
						}
						return NULL;
					}
				}

			}

			deletePtr(pID);
		}
	}

	return NULL;
}

/**
*
*	A la par que se realiza el Parseado, se guardará el orden de las variables para que en tiempo de ejecución
*	tengan un acceso directo a memoria, sin necesidad de buscar la variable correspondiente.
*	Esto aumentará el rendimiento, aunque bien producirá un leve aumento del consumo de ram,
*	disminuirá notablemente el tiempo de ejecución.
*
**/

Variable* Parser::BusquedaVariable(Parser_Identificador * ID, std::vector<Variable> & variables)
{
	for (std::vector<Variable>::iterator it = variables.begin(); it != variables.end(); ++it)
	{
		if (it->nombre == ID->nombre)
			return &(*it);
	}

	for (std::vector<Variable>::iterator it = this->variables_globales.begin(); it != this->variables_globales.end(); ++it)
	{
		if (it->nombre == ID->nombre)
		{
			ID->var_global = true;
			return &(*it);
		}
	}



	return NULL;
}

void Parser::CargarEnCacheOperaciones(arbol_operacional * arbol, std::vector<Variable> & variables, bool inside)
{
	if (arbol->operador == OP_NONE)
	{
		std::visit(overloaded
			{
				[&](Parser_Identificador * a)
				{
					Variable* var = this->BusquedaVariable(a, variables);
					if (var == NULL)
					{
						//Si no existe y la variable no esta en una localizacion inside, la creamos.
						if (inside)
						{
							Errores::generarError(Errores::ERROR_VARIABLE_NO_EXISTE, &a->parametros, a->nombre);
							existenErrores = true;
						}
						else
						{
							variables.push_back(Variable(a->nombre, this->getLastIndex()));
							IncrementarVariables();
							a->index = variables.back().index;
							a->inicializando = true;
						}
					}
					else a->index = var->index;
				},
				[&](auto&) { return; },
			}, arbol->_v1);

		return;
	}


	//Igualdad implicaría que una nueva variable se declara dentro de la operación.
	// La variable siempre estará en la izquierda, el valor en la derecha.
	if (arbol->operador == OP_IG_EQUAL || arbol->operador == OP_CHECK_GET || arbol->operador == OP_POP_ADD)
	{
		std::visit(overloaded
			{
				[&](arbol_operacional * a) { CargarEnCacheOperaciones(a, variables,inside); },
				[&](Parser_Identificador * a)
				{
					Variable* var = this->BusquedaVariable(a, variables);
					if (var == NULL)
					{
						//Si no existe, la creamos.
						if (this->isGlobal)
						{
							this->variables_globales.push_back(Variable(a->nombre, this->getLastIndex()));
							IncrementarVariables();
							a->index = this->variables_globales.back().index;
							a->var_global = true;
							a->inicializando = true;
						}
						else
						{
							variables.push_back(Variable(a->nombre, this->getLastIndex()));
							IncrementarVariables();
							a->index = variables.back().index;
							a->inicializando = true;
						}
					}
					else a->index = var->index;
				},
				[](auto&) {return; },
			}, arbol->_v1);

		std::visit(overloaded{
					[](const auto&) {},
					[&](arbol_operacional * a2) { CargarEnCacheOperaciones(a2, variables,inside); },
					[&](Parser_Identificador * a2)
					{
							Variable* var2 = this->BusquedaVariable(a2, variables);
							if (var2 == NULL)
							{
								//Si no existe, la creamos.
								if (this->isGlobal)
								{
									this->variables_globales.push_back(Variable(a2->nombre, this->getLastIndex()));
									IncrementarVariables();
									a2->index = this->variables_globales.back().index;
									a2->var_global = true;
									a2->inicializando = true;
								}
								else
								{
									variables.push_back(Variable(a2->nombre, this->getLastIndex()));
									IncrementarVariables();
									a2->index = variables.back().index;
									a2->inicializando = true;
								}
							}
							else a2->index = var2->index;
					},
			}, arbol->_v2);

	}
	else
	{
		std::visit(overloaded{
			[](const auto&) {},
			[&](arbol_operacional * a) { CargarEnCacheOperaciones(a, variables,inside); },
			[&](Parser_Identificador * a)
			{
				Variable* var = this->BusquedaVariable(a, variables);
				if (var == NULL)
				{
					Errores::generarError(Errores::ERROR_VARIABLE_NO_EXISTE, &a->parametros, a->nombre);
					existenErrores = true;
				}
				else a->index = var->index;
			},
			}, arbol->_v1);


		std::visit(overloaded{
		[](const auto&) {},
		[&](arbol_operacional * a2) { CargarEnCacheOperaciones(a2, variables,inside); },
		[&](Parser_Identificador * a2)
		{
			Variable* var2 = this->BusquedaVariable(a2, variables);
			if (var2 == NULL)
			{
				Errores::generarError(Errores::ERROR_VARIABLE_NO_EXISTE, &a2->parametros, a2->nombre);
				existenErrores = true;
			}
			else a2->index = var2->index;
		},
			}, arbol->_v2);

	}
}



