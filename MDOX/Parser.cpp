
#include "Parser.h"
#include <string>
#include <iomanip>

std::vector<Variable>* Parser::variables_globales_parser = NULL;
std::vector<Fichero*> Parser::nombre_ficheros;   //Nombre de ficheros cargados en la instancia actual del interprete.
std::vector<Parser_Funcion*> Parser::funciones;
std::vector<Parser_Class*> Parser::clases;
SendVariables Parser::variables_scope;
std::vector<std::string> Parser::requireList;

std::filesystem::path Parser::mainPathProgram;

void Parser::removeParserCache()
{
	deletePtr(Parser::variables_globales_parser);
	Parser::nombre_ficheros.clear();
	Parser::funciones.clear();
	Parser::clases.clear();
	Parser::requireList.clear();

	Parser::variables_scope.variables_locales.clear();

	if(Parser::variables_scope.variables_clase)
	Parser::variables_scope.variables_clase->clear();

	*Parser::variables_scope.num_local_var = 0;
	
}

bool Parser::GenerarArbol()
{
	//Borramos la lista de funciones existentes actualmente.
/*	for (std::vector<Parser_Funcion*>::iterator it = Parser::funciones.begin(); it != Parser::funciones.end(); ++it)
	{
		delete (*it);
	}
	this->funciones.clear();
	*/
	//Borramos la lista de clases existentes actualmente.
	/*for (std::vector<Parser_Class*>::iterator it = Parser::clases.begin(); it != Parser::clases.end(); ++it)
	{
		delete (*it);
	}
	this->clases.clear();
	*/

	Parser::nombre_ficheros.push_back(this->tokenizer.fichero);
	//SendVariables variables;

	//Interpretando datos...size_t
	int local = 0;
	while (local < this->tokenizer.tokens.size())
	{
		//	std::cout << "--:: " << parser.tokenizer.tokens.at(local) << " :: " << local << " \n";


		Parser_Funcion* p2 = this->getFuncion(local);

		if (p2)
		{
			funciones.push_back(p2);
			continue;
		}

		Parser_Class* pClass = this->getClass(local);

		if (pClass)
		{
			this->clases.push_back(pClass);
			continue;
		}

		this->isMain = true;
		Parser_Sentencia* p3 = this->getSentencia(local, variables_scope);
		this->isMain = false;

		if (p3)
		{
			this->sentenciasMain.push_back(p3);
			continue;
		}


		OutData_Parametros data = OutData_Parametros(this->tokenizer.token_actual->linea, this->tokenizer.token_actual->char_horizontal, this->nombre_ficheros.back());
		Errores::generarError(Errores::ERROR_DE_SINTAXIS, &data, this->tokenizer.token_actual->token);
		return false;
	}

	// Tratamiento de valores de funciones por el parser.
	if (!this->preloadCalls())
		return false;

	if (this->existenErrores)
		return false;

	return true;
}

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
			return Value(value);
		}
	}

	index = local_index;
	if (tokenizer.getTokenValue(index) == "'")
	{
		std::string value = tokenizer.getTokenValue(index);

		if (value == "'")
		{
			local_index = index;
			v = true;
			return Value((std::string)"");
		}

		if (tokenizer.getTokenValue(index) == "'")
		{
			local_index = index;
			v = true;
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
				double value;

				try {
					value = s2d(doub, std::locale::classic());
				}
				catch (const std::out_of_range)
				{
					auto out = OutData_Parametros(tokenizer.token_actual->linea, tokenizer.token_actual->char_horizontal, tokenizer.fichero);
					Errores::generarError(Errores::ERROR_DOUBLE_OVERFLOW, &out);
					v = false;
					return Value();
				}

				
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
			long long value;

			try {
				value = std::stoll(token);
			}
			catch (const std::out_of_range)
			{
				auto out = OutData_Parametros(tokenizer.token_actual->linea, tokenizer.token_actual->char_horizontal, tokenizer.fichero);
				Errores::generarError(Errores::ERROR_INTEGER_OVERFLOW, &out);
				v = false;
				return Value();
			}

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

	//######### Probando si puede tratarse de un void
	index = local_index;
	token = tokenizer.getTokenValue(index);
	if (token == "void")
	{
		v = true;
		local_index = index;
		return Value();
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

	bool esEstricto = false;

	if (token == "strict")
	{
		esEstricto = true;
		token = tokenizer.getTokenValue(index);
	}

	if (token == "vector")
	{
		Parser_Declarativo* p = new Parser_Declarativo(PARAM_VECTOR, esEstricto);
		local_index = index;
		return p;
	}
	else if (token == "string")
	{
		Parser_Declarativo* p = new Parser_Declarativo(PARAM_STRING, esEstricto);
		local_index = index;
		return p;
	}
	else if (token == "int")
	{
		Parser_Declarativo* p = new Parser_Declarativo(PARAM_INT, esEstricto);
		local_index = index;
		return p;
	}
	else if (token == "lint")
	{
		Parser_Declarativo* p = new Parser_Declarativo(PARAM_LINT, esEstricto);
		local_index = index;
		return p;
	}
	else if (token == "double")
	{
		Parser_Declarativo* p = new Parser_Declarativo(PARAM_DOUBLE, esEstricto);
		local_index = index;
		return p;
	}
	else if (token == "bool")
	{
		Parser_Declarativo* p = new Parser_Declarativo(PARAM_BOOL, esEstricto);
		local_index = index;
		return p;
	}
	else if (token == "auto")
	{
		Parser_Declarativo* p = new Parser_Declarativo(PARAM_VOID, esEstricto);
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


	index = local_index;
	std::string token = tokenizer.getTokenValue(index);

	//Comprobamos que el identificador no es una palabra reservada del sistema.
	std::vector<std::string> palabras_reservadas = { "strict","require", "include","global", "return", "break", "if", "else", "vector", "int", "double", "void", "auto", "bool", "operator", "constructor", "class", "function", "while", "for", "continue", "lint", "string" };

	if (std::find(palabras_reservadas.begin(), palabras_reservadas.end(), token) != palabras_reservadas.end())
	{
		return NULL;
	}

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

//Usado en vectores solo
multi_value* Parser::getValorList(bool& all_value, int& local_index, SendVariables& variables)
{
	int index = local_index; //##############

	multi_value* mv = new multi_value();
	this->readingVector = true;

	while (true)
	{
		int index2 = index;
		arbol_operacional* c = getOperacion(index2, variables, false);

		if (!c)
		{
			this->readingVector = false;
			delete mv;
			return NULL;
		}

		if (c->operador == OP_NONE && all_value)
		{
			std::visit(overloaded
				{
					[&](Value & a) {mv->arr.emplace_back(std::move(c)); },
					[&](auto & a) { mv->arr.emplace_back(std::move(c));  all_value = false; },
				}, c->_v1);
			//delete c;
		}
		else
		{
			all_value = false;
			//if(c->operador == OP_NONE)
				mv->arr.emplace_back(std::move(c));
			//else mv->arr.emplace_back(std::move(c));
		//	delete c;
		}

		if (tokenizer.getTokenValue(index2) == ",")
		{
			index = index2;
			continue;
		}
		else
		{
			this->readingVector = false;
			index2--;
			local_index = index2;
			return mv;
		}
	}

	this->readingVector = false;
}

/*
Tratamiento de la lista de vectores multi_value, para permitir que siempre se muestren en orden los valores.
De esta forma, [[1,2,3,4,5]:a,b,c,c,b,a] -> El primer a,b,c tomará los valores, dando 3,4,5 respectivamente.
											Mientras que los siguientes c,b,a al ya existir dejará los datos en el rango inverso
											teniendo al final: [1,2,5,4,3]
*/

/*

Interprete> b=6; <:: [[1,2,3]:a,2]; <:: a;
[1, 2, 2]3
Interprete> b=6; <:: [[1,2,3]:a,2+5]; <:: a;
[1, 2, 7]3
Interprete> b=6; <:: [[1,2,3]:a,b+5]; <:: a;
[1, 2, 3]11

*/
void TratarContenedorVectores(multi_value* contenedor_operacion_vector)
{
	if (!contenedor_operacion_vector->contenedor)
		return;

	std::visit(overloaded
		{
			[&](multi_value * a)
			{
				if (a->is_vector)
					return;
				if (a->contenedor)
				{
					TratarContenedorVectores(a);
					return;
				}

				if (contenedor_operacion_vector->operacionesVector->operador1 == OPERADORES::OP_CHECK_GET)
					return;

				bool reverse = false;
				long long first_reverse = 0;
				for (int itr = 0; itr < a->arr.size(); itr++)
				{
					/*				
					//Si devuelve true, implica que requiere reverse
					if (std::visit(overloaded
						{
							[&](Parser_Identificador * tr)
							{
								if (tr->inicializando)
									return false;
								else return true;
							},
							[](auto&) {return true; }

						}, a->arr[itr]))
					*/
					if (a->arr[itr]->operador != OP_NONE || std::visit(overloaded
						{
							[&](Parser_Identificador * tr)
							{
								if (tr->inicializando)
									return false;
								else return true;
							},
							[](auto&) {return true; }
						}, a->arr[itr]->_v1))
					{
						if (!reverse)
						{
							first_reverse = itr;
							reverse = true;
						}
					}
					else
					{
						if (reverse && first_reverse - 1 != itr)
						{
							std::reverse(a->arr.begin() + first_reverse, a->arr.begin() + itr);
							reverse = false;
						}
					}
				}

				if (reverse && first_reverse-1 != a->arr.size()-1)
				{
					std::reverse(a->arr.begin() + first_reverse, a->arr.end());
				}
			},
			[&](auto&) {}

		}, contenedor_operacion_vector->operacionesVector->v1);

	std::visit(overloaded
		{
			[&](multi_value * a)
			{
				if (a->is_vector) //################
					return;

				if (a->contenedor)
				{
					TratarContenedorVectores(a);
					return;
				}

				bool reverse = false;
				long long first_reverse = 0;
				for (int itr = 0; itr < a->arr.size(); itr++)
				{
					//Si devuelve true, implica que requiere reverse
					if (contenedor_operacion_vector->operacionesVector->operador1 == OPERADORES::OP_CHECK_GET || 
						/*a->arr[itr]->operador != OP_NONE || *//*Devuelve true si es un id sin inicializar que toma datos*/std::visit(overloaded
						{
							[&](Parser_Identificador * tr)
							{
								if (tr->inicializando)
									return true;
								else return false;
							},
							[](auto&) {return false; }
						}, a->arr[itr]->_v1))
					{
						if (!reverse)
						{
							first_reverse = itr;
							reverse = true;
						}
					}
					else
					{
						if (reverse && first_reverse - 1 != itr)
						{
							std::reverse(a->arr.begin() + first_reverse, a->arr.begin() + itr);
							reverse = false;
						}
					}
				}

				if (reverse && first_reverse - 1 != a->arr.size() - 1)
				{
					std::reverse(a->arr.begin() + first_reverse, a->arr.end());
				}
			},
			[&](auto&) {}

		}, contenedor_operacion_vector->operacionesVector->v2);

	if (contenedor_operacion_vector->operacionesVector->dobleOperador)
	{

		std::visit(overloaded
			{
				[&](multi_value * a)
				{
					if (a->is_vector)
						return;

					if (a->contenedor)
					{
						TratarContenedorVectores(a);
						return;
					}

					bool reverse = false;
					long long first_reverse = 0;
					for (int itr = 0; itr < a->arr.size(); itr++)
					{
						//Si devuelve true, implica que requiere reverse
						if (contenedor_operacion_vector->operacionesVector->operador1 == OPERADORES::OP_CHECK_GET ||
							/*a->arr[itr]->operador != OP_NONE ||*/ std::visit(overloaded
								{
									[&](Parser_Identificador * tr)
									{
										if (tr->inicializando)
											return true;
										else return false;
									},
									[](auto&) {return false; }
								}, a->arr[itr]->_v1))
						{
							if (!reverse)
							{
								first_reverse = itr;
								reverse = true;
							}
						}
						else
						{
							if (reverse && first_reverse - 1 != itr)
							{
								std::reverse(a->arr.begin() + first_reverse, a->arr.begin() + itr);
								reverse = false;
							}
						}
					}

					if (reverse && first_reverse - 1 != a->arr.size() - 1)
					{
						std::reverse(a->arr.begin() + first_reverse, a->arr.end());
					}
				},
				[&](auto&) {}

			}, contenedor_operacion_vector->operacionesVector->v3);
	}



}

//Reduce un arbol a un tipo
tipoValor ReducirArbolATipo(arbol_operacional * arbol)
{
	if (arbol->operador == OP_NONE)
	{
		return arbol->_v1;
	}
	return arbol;
}

Parser_Identificador* Parser::getVariableID(int& local_index, SendVariables& variables)
{
	int indexR = local_index;

	bool is_global = false;

	if (tokenizer.getTokenValue(indexR) == "global")
		is_global = true;
	else indexR = local_index;


	int index = indexR;
	// ###### Comprobamos si se trata de un declarativo `+ Identificador ######
	// Declarativo + Identificados -> int varA

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
			pID->inicializando = true;

			if (pD1->value != PARAM_VOID)
				pID->fuerte = true;

			pID->var_global = is_global;
			pID->tipo = pD1;
			pID->generarPosicion(&tokenizer);
			return pID;
		}
		deletePtr(pD1);
	}

	index = indexR;

	Parser_Identificador* i = getIdentificador(index);
	
	if(i)
	{
		i->var_global = is_global;
		local_index = index;
		return i;
	}


	return NULL;
}


conmp Parser::getValor(bool& ret, int& local_index, SendVariables& variables)
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
					delete contenedor_operacion_vector;
					if (all_value)
					{
						std::shared_ptr<mdox_vector> res = std::make_shared<mdox_vector>();
						res->vector.resize(c->arr.size());
						int itr = 0;
						for (std::vector<arbol_operacional*>::iterator it = c->arr.begin(); it != c->arr.end(); ++it)
						{
							res->vector[itr] = std::get<Value>((*it)->_v1);
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
						contenedor_operacion_vector->operacionesVector = new OperacionesEnVector(ReducirArbolATipo(c->arr[0]), ReducirArbolATipo(nt2->arr[0]), operador);
						c->arr.clear();
						nt2->arr.clear();
						delete c;
						delete nt2;
					}
					else if (c->arr.size() == 1)
					{
						contenedor_operacion_vector->operacionesVector = new OperacionesEnVector(ReducirArbolATipo(c->arr[0]), nt2, operador);
						c->arr.clear();
						delete c;
					}
					else if (nt2->arr.size() == 1)
					{
						contenedor_operacion_vector->operacionesVector = new OperacionesEnVector(c, ReducirArbolATipo(nt2->arr[0]), operador);
						nt2->arr.clear();
						delete nt2;
					}
					else
					{
						contenedor_operacion_vector->operacionesVector = new OperacionesEnVector(c, nt2, operador);
					}

					local_index = indexRet;
					TratarContenedorVectores(contenedor_operacion_vector);
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
							contenedor_operacion_vector->operacionesVector = new OperacionesEnVector(ReducirArbolATipo(c->arr[0]), ReducirArbolATipo(nt2->arr[0]), operador, ReducirArbolATipo(nt3->arr[0]), operador2);
							c->arr.clear();
							nt2->arr.clear();
							nt3->arr.clear();
							delete c;
							delete nt2;
							delete nt3;
						}
						else if (c->arr.size() == 1 && nt2->arr.size() == 1)
						{
							contenedor_operacion_vector->operacionesVector = new OperacionesEnVector(ReducirArbolATipo(c->arr[0]), ReducirArbolATipo(nt2->arr[0]), operador, nt3, operador2);
							c->arr.clear();
							nt2->arr.clear();
							delete c;
							delete nt2;
						}
						else if (c->arr.size() == 1 && nt3->arr.size() == 1)
						{
							contenedor_operacion_vector->operacionesVector = new OperacionesEnVector(ReducirArbolATipo(c->arr[0]), nt2, operador, ReducirArbolATipo(nt3->arr[0]), operador2);
							c->arr.clear();
							nt3->arr.clear();
							delete c;
							delete nt3;
						}
						else if (nt2->arr.size() == 1 && nt3->arr.size() == 1)
						{
							contenedor_operacion_vector->operacionesVector = new OperacionesEnVector(c, ReducirArbolATipo(nt2->arr[0]), operador, ReducirArbolATipo(nt3->arr[0]), operador2);
							nt2->arr.clear();
							nt3->arr.clear();
							delete nt2;
							delete nt3;
						}
						else if (nt2->arr.size() == 1)
						{
							contenedor_operacion_vector->operacionesVector = new OperacionesEnVector(c, ReducirArbolATipo(nt2->arr[0]), operador, nt3, operador2);
							nt2->arr.clear();
							delete nt2;
						}
						else if (nt3->arr.size() == 1)
						{
							contenedor_operacion_vector->operacionesVector = new OperacionesEnVector(c, nt2, operador, ReducirArbolATipo(nt3->arr[0]), operador2);
							nt3->arr.clear();
							delete nt3;
						}
						else if (c->arr.size() == 1)
						{
							contenedor_operacion_vector->operacionesVector = new OperacionesEnVector(ReducirArbolATipo(c->arr[0]), nt2, operador, nt3, operador2);
							c->arr.clear();
							delete c;
						}
						else
						{
							contenedor_operacion_vector->operacionesVector = new OperacionesEnVector(c, nt2, operador, nt3, operador2);
						}

						local_index = indexRet;
						TratarContenedorVectores(contenedor_operacion_vector);
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

	bool constructor_clase = false;
	//Se trata de un constructor de clase
	index = local_index;
	if (tokenizer.getTokenValue(index) == "new")
	{
		constructor_clase = true;
	}


	//En caso de no ser un literal, podrá ser o una llamada a una función o un identificador.
	if(!constructor_clase)
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
						Call_Value* v = new Call_Value(i, entradas);
						local_index = i_index;
						v->generarPosicion(&tokenizer);
						v->inside_class = this->readingClass;
						v->function_parent_is_static = this->readingStaticValue;
						v->is_class = constructor_clase;

						this->valores_llamadas.emplace_back(v);
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
						Call_Value* v = new Call_Value(i, entradas);
						local_index = t_index;
						v->generarPosicion(&tokenizer);
						v->inside_class = this->readingClass;
						v->function_parent_is_static = this->readingStaticValue;
						v->is_class = constructor_clase;

						this->valores_llamadas.emplace_back(v);
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
	}

	index = local_index;
	Parser_Identificador* i2 = getVariableID(index, variables);
	if (i2)
	{
		local_index = index;
		ret = true;
		return i2;
	}


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

	else if (v == ".") return OPERADORES::OP_CLASS_ACCESS;

	else if (v == "!") return OPERADORES::OP_NEGADO;
	else if (v == "++") return OPERADORES::OP_ITR_PLUS;
	else if (v == "--") return OPERADORES::OP_ITR_MIN;

	else if (v == "+") return OPERADORES::OP_ARIT_SUMA;
	else if (v == "-") return OPERADORES::OP_ARIT_RESTA;
	else if (v == "*") return OPERADORES::OP_ARIT_MULT;
	else if (v == "@") return OPERADORES::OP_COPY;
	else if (v == "@@") return OPERADORES::OP_IN_COPY;
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

	else if (v == "&&") return OPERADORES::OP_LOG_AND;
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



arbol_operacional* Parser::getOperacionInd(int& local_index, SendVariables& variables, bool inside, bool isPublic)
{
	int index = local_index;
	arbol_operacional * Op = getOperacion(index, variables, inside);

	if (Op)
	{
		Op->is_Public = isPublic;
		std::string next_token = tokenizer.getTokenValue(index);
		if (next_token == ",")
		{
			multi_value* mv = new multi_value();
			mv->arr.emplace_back(Op);

			do {
				arbol_operacional * t1 = getOperacion(index, variables, inside);
				if (t1)
				{
					t1->is_Public = isPublic;
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


arbol_operacional * Parser::getOperacion(int& local_index, SendVariables& variables, bool inside)
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

	int is_op = 0;
	int left_scope = 0;

	//Usado para comprobar si el último operador fue ++ ó --
	// Y en caso de serlo, comprobar que el siguiente valor no sea fin de linea.
	bool isEndLineOperator = false;

	while (true)
	{
		//Comprobamos si es itr++\n
		if (isEndLineOperator)
		{
			if (tokenizer.checkCloseToken(index))
				break;
			isEndLineOperator = false;
		}

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
					auto out = OutData_Parametros(tokenizer.token_actual->linea, tokenizer.token_actual->char_horizontal, tokenizer.fichero);
					Errores::generarError(Errores::ERROR_OPERATION_DOUBLE_OPERATOR, &out);
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
			else if (aux == OPERADORES::OP_CLASS_ACCESS)
			{
				is_op = 2;
				bool ok;
				conmp pN = this->getValor(ok, index, variables);

				if (ok)
				{
					if (!std::visit(overloaded{
						[&](Parser_Identificador * a)->bool {return true; },
						[&](Call_Value * a)->bool { return true; },
						[&](auto&)->bool {  return false; },
						}, pN))
					{
						auto out = OutData_Parametros(tokenizer.token_actual->linea, tokenizer.token_actual->char_horizontal, tokenizer.fichero);
						Errores::generarError(Errores::ERROR_OPERADOR_ACCESO_CLASE_INVALIDO, &out);
						return NULL;
					}
				}
				else
				{
					auto out = OutData_Parametros(tokenizer.token_actual->linea, tokenizer.token_actual->char_horizontal, tokenizer.fichero);
					Errores::generarError(Errores::ERROR_OPERADOR_ACCESO_CLASE_INVALIDO, &out);
					return NULL;
				}

				stack.push_back(pN);
				stack.push_back(aux);
				continue;
			}
			else if (aux == OPERADORES::OP_SCOPE_LEFT)
			{
				index--;
				break;
				/*is_op = 0;
				left_scope++;
				OPERADORES a = OPERADORES::OP_ARIT_MULT;
				normal_operator_do(left_scope, is_op, a, stack, op_stack);
				op_stack.push_back(aux);
				continue;*/
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
				if (this->tokenizer.tokens[index - (long long)1]->firstNewLine)
				{
					index--;
					break;
				}

				normal_operator_do(left_scope, is_op, aux, stack, op_stack);
				is_op = 0;
				op_stack.push_back(OPERADORES::OP_SCOPE_LEFT);
				left_scope++;
				continue;
			}
			else if (aux == OPERADORES::OP_BRACKET_RIGHT)
			{
				if (scope_right_do(left_scope, is_op, stack, op_stack))
				{
					OPERADORES c = op_stack.back();

					if (c == OPERADORES::OP_BRACKET_LEFT)
					{
						stack.push_back(c);
						op_stack.pop_back();
						continue;
					}
					else return NULL;
				}
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
				if (aux == OPERADORES::OP_ITR_MIN || aux == OPERADORES::OP_ITR_PLUS)
					isEndLineOperator = true;

				normal_operator_do(left_scope, is_op, aux, stack, op_stack);
				continue;
			}
		}
	}


	if (left_scope > 0) // Fallos con los parentesis.
	{
		auto out = OutData_Parametros(tokenizer.token_actual->linea, tokenizer.token_actual->char_horizontal, tokenizer.fichero);
		Errores::generarError(Errores::ERROR_SINTAXIS_PARENTESIS, &out);
		return NULL;
	}
	//std::cout << "#3. \n";
	while (!op_stack.empty())
	{
		stack.push_back(op_stack.back());
		op_stack.pop_back();
	}

	stack_conmp res;
	//Realiza operaciones de los valores literales
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

Parser_Sentencia* Parser::getSentencia(int& local_index, SendVariables& variables)
{
	//##########   -- SENTENCIA RECURSIVA --   ##########
	int index = local_index;

	if (tokenizer.getTokenValue(index) == "{")
	{
		int n_index = index;
		if (tokenizer.getTokenValue(n_index) == "}")
		{
			local_index = n_index;
			return new Sentencia_Empty();
		}

		std::vector<Parser_Sentencia*> valor;
		//Copiamos todas las variables actuales	
		//std::vector<Variable> variables_local(variables);
		SendVariables variables_local(variables);

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
				auto out = OutData_Parametros(tokenizer.token_actual->linea, tokenizer.token_actual->char_horizontal, tokenizer.fichero);
				Errores::generarError(Errores::ERROR_SINTAXIS_PARENTESIS, &out);
				deletePtr(pCond);
				return NULL;
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

				auto out = OutData_Parametros(tokenizer.token_actual->linea, tokenizer.token_actual->char_horizontal, tokenizer.fichero);
				Errores::generarError(Errores::ERROR_SINTAXIS_PARENTESIS, &out);
				deletePtr(pCond);
				return NULL;
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
						auto out = OutData_Parametros(tokenizer.token_actual->linea, tokenizer.token_actual->char_horizontal, tokenizer.fichero);
						Errores::generarError(Errores::ERROR_SINTAXIS_PARENTESIS, &out);
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
			if (tokenizer.isCloseToken(index))
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
			if (tokenizer.isCloseToken(index))
			{
				local_index = index;
				Sentencia_Return* sif = new Sentencia_Return(NULL);
				sif->generarPosicion(&tokenizer);
				return sif;
			}
		}
	}

	//##########   -- SENTENCIA BREAK --   ##########
	index = local_index;

	if (tokenizer.getTokenValue(index) == "break")
	{
		if (tokenizer.isCloseToken(index))
		{
			local_index = index;
			Sentencia_Accion* sif = new Sentencia_Accion(TipoAccion::BREAK);
			sif->generarPosicion(&tokenizer);
			return sif;
		}
	}

	//##########   -- SENTENCIA CONTINUE --   ##########
	index = local_index;

	if (tokenizer.getTokenValue(index) == "continue")
	{
		if (tokenizer.isCloseToken(index))
		{
			local_index = index;
			Sentencia_Accion* sif = new Sentencia_Accion(TipoAccion::CONTINUE);
			sif->generarPosicion(&tokenizer);
			return sif;
		}
	}

	//##########   -- SENTENCIA IGNORE --   ##########
	index = local_index;

	if (tokenizer.getTokenValue(index) == "ignore")
	{
		if (tokenizer.isCloseToken(index))
		{
			local_index = index;
			Sentencia_Accion* sif = new Sentencia_Accion(TipoAccion::IGNORE);
			sif->generarPosicion(&tokenizer);
			return sif;
		}
	}

	//##########   -- SENTENCIA PRINT --   ##########
	index = local_index;
	if (tokenizer.getTokenValue(index) == "<::")
	{
		arbol_operacional* pOp = getOperacionInd(index, variables, true);
		if (pOp)
		{
			if (tokenizer.isCloseToken(index))
			{
				local_index = index;
				Sentencia_Print* sif = new Sentencia_Print(pOp);
				sif->generarPosicion(&tokenizer);
				return sif;
			}
			deletePtr(pOp);
		}
	}

	//##########   -- SENTENCIA INPUT --   ##########
	index = local_index;
	if (tokenizer.getTokenValue(index) == "::>")
	{
		arbol_operacional* pOp = getOperacionInd(index, variables, false);
		if (pOp)
		{
			if (tokenizer.isCloseToken(index))
			{
				local_index = index;
				Sentencia_Input* sif = new Sentencia_Input(pOp);
				sif->generarPosicion(&tokenizer);
				return sif;
			}
			deletePtr(pOp);
		}
	}

	//##########   -- INCLUDE --   ##########
	index = local_index;
	if (tokenizer.getTokenValue(index) == "include")
	{
		bool l;
		Value v1 = getLiteral(l, index);

		if (l)
		{
			if (auto pStr = std::get_if<std::string>(&v1.value))
			{
				Parser* parser = new Parser();

				std::string ruta = getAbsolutePathFromRelative((std::string)* pStr);

				if (std::find(this->includeList.begin(), this->includeList.end(), ruta) != this->includeList.end())
				{
					auto out = OutData_Parametros(tokenizer.token_actual->linea, tokenizer.token_actual->char_horizontal, tokenizer.fichero);
					Errores::generarError(Errores::ERROR_INCLUDE_REF_CIRCULAR, &out, ruta);
					index = local_index;
					return NULL;
				}
				
				parser->includeList = this->includeList;
				parser->includeList.emplace_back(ruta);

				Sentencia_Include* pInclude = new Sentencia_Include(parser);
				//Desde el parser, accedemos al tokenizer, desde el mismo podremos generarlo a través del fichero.
				bool correcto = parser->tokenizer.GenerarTokenizerDesdeFichero((std::string)*pStr);

				if (!correcto)
				{
					auto out = OutData_Parametros(tokenizer.token_actual->linea, tokenizer.token_actual->char_horizontal, tokenizer.fichero);
					Errores::generarError(Errores::ERROR_INCLUDE_RUTA_INVALIDA, &out, (std::string)* pStr);
					return NULL;
				}
				if (!parser->GenerarArbol())
				{
					auto out = OutData_Parametros(tokenizer.token_actual->linea, tokenizer.token_actual->char_horizontal, tokenizer.fichero);
					Errores::generarError(Errores::ERROR_INCLUDE_FALLO, &out, (std::string) * pStr);
					return NULL;
				}

				local_index = index;
				return pInclude;
			}

			auto out = OutData_Parametros(tokenizer.token_actual->linea, tokenizer.token_actual->char_horizontal, tokenizer.fichero);
			Errores::generarError(Errores::ERROR_INCLUDE_PARAMETRO, &out);
			index = local_index;
			return NULL;		
		}
	}

	//##########   -- REQUIRE --   ##########
	index = local_index;
	if (tokenizer.getTokenValue(index) == "require")
	{
		int index2 = index;
		bool l;
		Value v1 = getLiteral(l, index2);

		if (l)
		{
			if (auto pStr = std::get_if<std::string>(&v1.value))
			{
				Parser* parser = new Parser();

				std::string ruta = getAbsolutePathFromRelative((std::string) * pStr);

				if (std::find(Parser::requireList.begin(), Parser::requireList.end(), ruta) != Parser::requireList.end())
				{
					auto out = OutData_Parametros(tokenizer.token_actual->linea, tokenizer.token_actual->char_horizontal, tokenizer.fichero);
					Errores::generarError(Errores::ERROR_INCLUDE_REQ_ALREADY, &out, ruta);
					index = local_index;
					return NULL;
				}

				Parser::requireList.emplace_back(ruta);

				Sentencia_Include* pInclude = new Sentencia_Include(parser);
				//Desde el parser, accedemos al tokenizer, desde el mismo podremos generarlo a través del fichero.
				bool correcto = parser->tokenizer.GenerarTokenizerDesdeFichero((std::string) * pStr);

				if (!correcto)
				{
					auto out = OutData_Parametros(tokenizer.token_actual->linea, tokenizer.token_actual->char_horizontal, tokenizer.fichero);
					Errores::generarError(Errores::ERROR_INCLUDE_RUTA_INVALIDA, &out, (std::string) * pStr);
					return NULL;
				}
				if (!parser->GenerarArbol())
				{
					auto out = OutData_Parametros(tokenizer.token_actual->linea, tokenizer.token_actual->char_horizontal, tokenizer.fichero);
					Errores::generarError(Errores::ERROR_INCLUDE_FALLO, &out, (std::string) * pStr);
					return NULL;
				}

				local_index = index2;
				return pInclude;
			}

			auto out = OutData_Parametros(tokenizer.token_actual->linea, tokenizer.token_actual->char_horizontal, tokenizer.fichero);
			Errores::generarError(Errores::ERROR_INCLUDE_PARAMETRO, &out);
			index = local_index;
			return NULL;
		}

		index2 = index;
		if (tokenizer.getTokenValue(index2) == "<")
		{
			int index3 = index2;

			std::string ruta = tokenizer.getTokenValue(index3);
			if (tokenizer.getTokenValue(index3) == ">")
			{

				if (std::find(Parser::requireList.begin(), Parser::requireList.end(), ruta) != Parser::requireList.end())
				{
					auto out = OutData_Parametros(tokenizer.token_actual->linea, tokenizer.token_actual->char_horizontal, tokenizer.fichero);
					Errores::generarError(Errores::ERROR_INCLUDE_REQ_ALREADY, &out, ruta);
					return NULL;
				}

				Parser::requireList.emplace_back(ruta);

				//Es una importacion core
				if (!Core::IncludeStart(ruta))
				{
					auto out = OutData_Parametros(tokenizer.token_actual->linea, tokenizer.token_actual->char_horizontal, tokenizer.fichero);
					Errores::generarError(Errores::ERROR_INCLUDE_PARAMETRO, &out);
					return NULL;
				}
				local_index = index3;
				return new Sentencia_Empty();
			}

			index3 = index2;
			Value v1 = getLiteral(l, index3);

			if (l)
			{
				if (auto pStr = std::get_if<std::string>(&v1.value))
				{
					Parser* parser = new Parser();

					std::string ruta = "lib/"+ (std::string) * pStr;

					if (std::find(Parser::requireList.begin(), Parser::requireList.end(), ruta) != Parser::requireList.end())
					{
						auto out = OutData_Parametros(tokenizer.token_actual->linea, tokenizer.token_actual->char_horizontal, tokenizer.fichero);
						Errores::generarError(Errores::ERROR_INCLUDE_REQ_ALREADY, &out, ruta);
						index = local_index;
						return NULL;
					}

					Parser::requireList.emplace_back(ruta);

					Sentencia_Include* pInclude = new Sentencia_Include(parser);
					//Desde el parser, accedemos al tokenizer, desde el mismo podremos generarlo a través del fichero.
					bool correcto = parser->tokenizer.GenerarTokenizerDesdeFichero((std::string) * pStr);

					if (!correcto)
					{
						auto out = OutData_Parametros(tokenizer.token_actual->linea, tokenizer.token_actual->char_horizontal, tokenizer.fichero);
						Errores::generarError(Errores::ERROR_INCLUDE_RUTA_INVALIDA, &out, (std::string) * pStr);
						return NULL;
					}
					if (!parser->GenerarArbol())
					{
						auto out = OutData_Parametros(tokenizer.token_actual->linea, tokenizer.token_actual->char_horizontal, tokenizer.fichero);
						Errores::generarError(Errores::ERROR_INCLUDE_FALLO, &out, (std::string) * pStr);
						return NULL;
					}

					local_index = index3;
					return pInclude;
				}
			}

		}
		auto out = OutData_Parametros(tokenizer.token_actual->linea, tokenizer.token_actual->char_horizontal, tokenizer.fichero);
		Errores::generarError(Errores::ERROR_INCLUDE_PARAMETRO, &out);
		index = local_index;
		return NULL;


	}

	//##########   -- SENTENCIA OPERACIONAL --   ##########
	index = local_index;

	arbol_operacional* pOp = getOperacionInd(index, variables); //IND : Una o varias operaciones.

	if (pOp)
	{
		if (tokenizer.isCloseToken(index))
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

Parser_Funcion* Parser::getFuncion(int& local_index, std::vector<Variable>* var_class)
{
	int index = local_index;


	if (tokenizer.getTokenValue(index) == "function")
	{
		Parser_Identificador* pID = getIdentificador(index);

		if (pID)
		{
			if (tokenizer.getTokenValue(index) == "(")
			{
				this->readingFunction = true;

				//Variables por función
				SendVariables variables(var_class);

				std::vector<arbol_operacional*> entradas;

				int t_index = index;
				while (true)
				{
					int t2_index = t_index;
					arbol_operacional* pFve = getOperacion(t2_index, variables);

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
							this->readingFunction = false;
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
							this->readingFunction = false;
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
						Parser_Sentencia* pSent = getSentencia(t3_index, variables);

						if (pSent)
						{
							local_index = t3_index;
							Parser_Funcion* sif = new Parser_Funcion(pID, entradas, pSent, pDecl);
							sif->preload_var = *variables.num_local_var;
							sif->generarPosicion(&tokenizer);
							this->readingFunction = false;
							return sif;
						}
						else
						{
							this->readingFunction = false;
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
						this->readingFunction = false;
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
					Parser_Sentencia* pSent = getSentencia(index, variables);
					if (pSent)
					{
						local_index = index;
						Parser_Funcion* sif = new  Parser_Funcion(pID, entradas, pSent);
						sif->preload_var = *variables.num_local_var;
						sif->generarPosicion(&tokenizer);
						this->readingFunction = false;
						return sif;
					}
					else
					{
						local_index = index;
						auto out = OutData_Parametros(tokenizer.token_actual->linea, tokenizer.token_actual->char_horizontal, tokenizer.fichero);
						Errores::generarError(Errores::ERROR_OPERACION_INVALIDA, &out);

						this->readingFunction = false;
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

	this->readingFunction = false;
	return NULL;
}

// ############################################################
// ######################## CLASS  ##########################
// ############################################################

etiquetas_class Parser::getLabelClass(int& local_index)
{
	int index = local_index;
	std::string token = tokenizer.getTokenValue(index);

	if (token == "public")
	{
		if (tokenizer.getTokenValue(index) == ":")
		{
			local_index = index;
			return LABEL_PUBLIC;
		}
	}
	else if (token == "private")
	{
		if (tokenizer.getTokenValue(index) == ":")
		{
			local_index = index;
			return LABEL_PRIVATE;
		}
	}
	else if(token == "static") //sigle row
	{
		local_index = index;
		return LABEL_STATIC;
	}

	return LABEL_NONE;
}

//El constructor/es de una clase
Parser_ClassConstructor* Parser::getClassConstructor(int& local_index, std::vector<Variable>& variables_clase)
{
	int index = local_index;
	SendVariables variables(variables_clase);

	if (tokenizer.getTokenValue(index) == "constructor")
	{
		if (tokenizer.getTokenValue(index) == "(")
		{
			if (tokenizer.getTokenValue(index) == ")")
			{
				this->readingFunction = true;
				Parser_Sentencia* op = getSentencia(index, variables);
				this->readingFunction = false;
				if (op == NULL)
				{
					Errores::generarError(Errores::ERROR_CLASE_CONSTRUCTOR_SENTENCIA_ERROR, NULL);
					return NULL;
				}

				local_index = index;
				return new Parser_ClassConstructor(op);
			}
			index--;

			std::vector<int> entradas;
			int sizeId = 0;
	
			do
			{
				Parser_Identificador* id = getIdentificador(index);
				if (id)
				{
					Variable* var = this->BusquedaVariableLocal(id, variables_clase);

					if (var == NULL)
					{
						int index_v = *variables.num_local_var;
						variables.push_VarLocal(Variable(id->nombre, index_v, true));
						sizeId++;
						entradas.emplace_back(0);
						entradas.emplace_back(index_v);

						//Errores::generarError(Errores::ERROR_CLASE_CONSTRUCTOR_ID_NOT_FOUND, NULL, id->nombre);
						//continue;
					}
					else
					{
						entradas.emplace_back(1);
						entradas.emplace_back(var->index);
					}

					if (tokenizer.getTokenValue(index) == ")")
					{
						this->readingFunction = true;
						Parser_Sentencia* op = getSentencia(index, variables);
						this->readingFunction = false;
						if (op == NULL)
						{
							Errores::generarError(Errores::ERROR_CLASE_CONSTRUCTOR_SENTENCIA_ERROR, NULL);
							return NULL;
						}

						local_index = index;
						return new Parser_ClassConstructor(entradas, op, *variables.num_local_var);
					}

					index--;
				}
				else
				{
					Errores::generarError(Errores::ERROR_CLASE_CONSTRUCTOR_NOT_ID, NULL);
					return NULL;
				}

			} while (tokenizer.getTokenValue(index) == ",");


		}

		Errores::generarError(Errores::ERROR_CLASE_CONSTRUCTOR_SINTAXIS, NULL);
	}
	return NULL;
}


void Call_Value::AddFuncion_Core(int inx)
{
	if (inx_funcion)
	{
		inx_funcion->funcionesCoreItrData = inx;
		return;
	}

	// ELSE: ERROR? Por qué se intenta introducir un dato en una función cuendo no lo es?
	Errores::generarError(Errores::ERROR_INESPERADO, &this->parametros, " Intento de seleccionar memoria para una función, cuando la llamada no es una función.");
}

void Call_Value::AddFuncion(int inx)
{
	if (inx_funcion)
	{
		inx_funcion->funcionesItrData.emplace_back(inx);
		return;
	}

	// ELSE: ERROR? Por qué se intenta introducir un dato en una función cuendo no lo es?
	Errores::generarError(Errores::ERROR_INESPERADO, &this->parametros, " Intento de seleccionar memoria para una función, cuando la llamada no es una función.");
}

void Call_Value::AddClass(int inx, int constructor)
{
	if (inx_class)
	{
		inx_class->class_index = inx;
		inx_class->constructor_index = constructor;
		return;
	}

	// ELSE: ERROR? Por qué se intenta introducir un dato en una función cuendo no lo es?
	Errores::generarError(Errores::ERROR_INESPERADO, &this->parametros, " Intento de seleccionar memoria para una clase, cuando la llamada no es una clase.");
}

void Call_Value::setFuncion()
{
	if (inx_funcion == NULL || inx_class)
	{
		inx_funcion = new IndexCall_Function();
		//is_class = false;
	}
}

void Call_Value::setClass()
{
	if (inx_class == NULL || inx_funcion)
	{
		inx_class = new IndexCall_Class();
		//is_class = true;
	}
}



/*Comprueba que todos los datos del árbol, son inicialización de identificados o multi-iniciación.*/
bool ComprobarIdentificadores(arbol_operacional* pOp)
{
	if (pOp->operador == OP_NONE)
	{
		return std::visit(overloaded{
		[&](arbol_operacional * a)->bool { return false; },
		[&](Value & a)->bool {return false; },
		[&](Parser_Identificador * a)->bool {return true; },
		[&](Call_Value * a)->bool { return false; },
		[&](multi_value * a)->bool
		{ 
			if (!a->contenedor && !a->is_vector)
			{
				//Comprobamos que todos los valores del multivalue, son identificadores.
				for (std::vector<arbol_operacional*>::iterator it = a->arr.begin(); it != a->arr.end(); ++it)
				{
					if (ComprobarIdentificadores(*it))
						continue;
					else return false;
				}
				return true;
			}
			return false;		
		},
		[&](auto&)->bool {  return false; },
		}, pOp->_v1);
	}

	else if (pOp->operador == OP_IG_EQUAL)
	{
		return true;
	}

	return false;
}

bool Parser::getClassOperadores(int& local_index, Parser_Class* clase, std::vector<Variable>& variables_clases)
{
	int index = local_index;

	//operator + (rlhs) { return rlhs + x } // Operador + normal, la posición del objeto será la izquierda en la operación: this+x
	//operator right + (lhs) {}// Operador inverso, La posición del objeto será la derecha en la operación: x+this
	if (tokenizer.getTokenValue(index) == "operator")
	{
		bool inverse_operator = false;
		if (tokenizer.getTokenValue(index) == "right")
		{
			inverse_operator = true;
		}
		else index--; // Si el token no es cierto, volvemos atrás el indice para luego volver a leerlo.


		OPERADORES op = getOperador(index);

		if (op == OPERADORES::OP_NONE)
		{
			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_INVALIDO, &clase->parametros, tokenizer.getTokenValue(index));
			return false;
		}
		//Operador usado para capturar []
		if (op == OPERADORES::OP_BRACKET_LEFT)
		{
			OPERADORES op2 = getOperador(index);
			if (op2 != OPERADORES::OP_BRACKET_RIGHT && op2 != OPERADORES::OP_NONE)
			{
				Errores::generarError(Errores::ERROR_CLASE_OPERADOR_INVALIDO, &clase->parametros, tokenizer.getTokenValue(index));
				return false;
			}
		}

		if (tokenizer.getTokenValue(index) == "(")
		{		
			int index2 = index;
			Parser_Identificador* entrada = getIdentificador(index2);
			bool es_op_binario = true;

			if (entrada == NULL)
			{
				es_op_binario = false;
			}
			else index = index2;

			if (op == OPERADORES::OP_ARIT_RESTA && !es_op_binario)
				op = OPERADORES::ELEM_NEG_FIRST;

			if (!is_single_operator(op) && !es_op_binario)
			{
				deletePtr(entrada);
				Errores::generarError(Errores::ERROR_CLASE_OPERADOR_ES_BINARIO, &clase->parametros);
				return false;
			}
			else if (is_single_operator(op) && es_op_binario)
			{
				deletePtr(entrada);
				Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_ES_BINARIO, &clase->parametros);
				return false;
			}

			if (tokenizer.getTokenValue(index) != ")")
			{
				Errores::generarError(Errores::ERROR_CLASE_OPERADOR_INVALIDO, &clase->parametros, tokenizer.getTokenValue(index));
				return false;
			}

			SendVariables variables_locales_sentencia = SendVariables(variables_clases);

			if(es_op_binario)
				variables_locales_sentencia.push_VarLocal(Variable(entrada->nombre, 0));

			Parser_Sentencia* body = getSentencia(index, variables_locales_sentencia);

			if (body == NULL)
			{
				Errores::generarError(Errores::ERROR_CLASE_SENTENCIA_INVALIDA, &clase->parametros, tokenizer.getTokenValue(index));
				return false;
			}

			if (inverse_operator)
			{
				if (clase->getRightOperators() == NULL)
					clase->createRightOperators();			
			}
			else
			{
				if (clase->getNormalOperators() == NULL)
					clase->createNormalOperators();
			}

			Operator_Class * operator_class = new Operator_Class(body, es_op_binario, *variables_locales_sentencia.num_local_var);


			switch (op)
			{
			     case OPERADORES::OP_ARIT_SUMA:
					{
						if(!inverse_operator)
							clase->getNormalOperators()->OPERATOR_suma = operator_class;
						else
							clase->getRightOperators()->OPERATOR_suma = operator_class;
						break;
					}
				case OPERADORES::OP_ARIT_RESTA:
					{
						if(!inverse_operator)
							clase->getNormalOperators()->OPERATOR_resta = operator_class;
						else
							clase->getRightOperators()->OPERATOR_resta = operator_class;
						break;
					}
				case OPERADORES::OP_ARIT_MULT:
					{
						if(!inverse_operator)
							clase->getNormalOperators()->OPERATOR_multiplicacion = operator_class;
						else
							clase->getRightOperators()->OPERATOR_multiplicacion = operator_class;
						break;
					}
				case OPERADORES::OP_ARIT_DIV:
					{
						if(!inverse_operator)
							clase->getNormalOperators()->OPERATOR_div = operator_class;
						else
							clase->getRightOperators()->OPERATOR_div = operator_class;
						break;
					}
				case OPERADORES::OP_ARIT_DIV_ENTERA:
					{
						if(!inverse_operator)
							clase->getNormalOperators()->OPERATOR_divEntera = operator_class;
						else
							clase->getRightOperators()->OPERATOR_divEntera = operator_class;
						break;
					}
				case OPERADORES::OP_ARIT_MOD:
					{
						if(!inverse_operator)
							clase->getNormalOperators()->OPERATOR_mod = operator_class;
						else
							clase->getRightOperators()->OPERATOR_mod = operator_class;
						break;
					}
				case OPERADORES::OP_IG_EQUAL:
					{
						if(!inverse_operator)
							clase->getNormalOperators()->OPERATOR_asignacion = operator_class;
						else
							clase->getRightOperators()->OPERATOR_asignacion = operator_class;
						break;
					}
				case OPERADORES::OP_IG_EQUAL_SUM:
					{
						if(!inverse_operator)
							clase->getNormalOperators()->OPERATOR_asignacion_sum = operator_class;
						else
							clase->getRightOperators()->OPERATOR_asignacion_sum = operator_class;
						break;
					}
				case OPERADORES::OP_IG_EQUAL_MIN:
					{
						if(!inverse_operator)
							clase->getNormalOperators()->OPERATOR_asignacion_res = operator_class;
						else
							clase->getRightOperators()->OPERATOR_asignacion_res = operator_class;
						break;
					}
				case OPERADORES::OP_IG_EQUAL_MULT:
					{
						if(!inverse_operator)
							clase->getNormalOperators()->OPERATOR_asignacion_mult = operator_class;
						else
							clase->getRightOperators()->OPERATOR_asignacion_mult = operator_class;
						break;
					}
				case OPERADORES::OP_IG_EQUAL_DIV:
					{
						if(!inverse_operator)
							clase->getNormalOperators()->OPERATOR_asignacion_div = operator_class;
						else
							clase->getRightOperators()->OPERATOR_asignacion_div = operator_class;
						break;
					}
				case OPERADORES::OP_IG_EQUAL_MOD:
					{
						if(!inverse_operator)
							clase->getNormalOperators()->OPERATOR_asignacion_mod = operator_class;
						else
							clase->getRightOperators()->OPERATOR_asignacion_mod = operator_class;
						break;
					}
				case OPERADORES::OP_REL_EQUAL:
					{
						if(!inverse_operator)
							clase->getNormalOperators()->OPERATOR_rel_igualdad = operator_class;
						else
							clase->getRightOperators()->OPERATOR_rel_igualdad = operator_class;
						break;
					}
				case OPERADORES::OP_REL_NOT_EQUAL:
					{
						if(!inverse_operator)
							clase->getNormalOperators()->OPERATOR_rel_no_igual = operator_class;
						else
							clase->getRightOperators()->OPERATOR_rel_no_igual = operator_class;
						break;
					}
				case OPERADORES::OP_REL_MINOR:
					{
						if(!inverse_operator)
							clase->getNormalOperators()->OPERATOR_rel_menor = operator_class;
						else
							clase->getRightOperators()->OPERATOR_rel_menor = operator_class;
						break;
					}
				case OPERADORES::OP_REL_MAJOR:
					{
						if(!inverse_operator)
							clase->getNormalOperators()->OPERATOR_rel_mayor = operator_class;
						else
							clase->getRightOperators()->OPERATOR_rel_mayor = operator_class;
						break;
					}
				case OPERADORES::OP_REL_MINOR_OR_EQUAL:
					{
						if(!inverse_operator)
							clase->getNormalOperators()->OPERATOR_rel_menor_igual = operator_class;
						else
							clase->getRightOperators()->OPERATOR_rel_menor_igual = operator_class;
						break;
					}
				case OPERADORES::OP_REL_MAJOR_OR_EQUAL:
					{
						if(!inverse_operator)
							clase->getNormalOperators()->OPERATOR_rel_mayor_igual = operator_class;
						else
							clase->getRightOperators()->OPERATOR_rel_mayor_igual = operator_class;
						break;
					}
					case OPERADORES::OP_LOG_AND:
					{
						if(!inverse_operator)
							clase->getNormalOperators()->OPERATOR_log_and = operator_class;
						else
							clase->getRightOperators()->OPERATOR_log_and = operator_class;
						break;
					}
					case OPERADORES::OP_LOG_OR:
					{
						if(!inverse_operator)
							clase->getNormalOperators()->OPERATOR_log_or = operator_class;
						else
							clase->getRightOperators()->OPERATOR_log_or = operator_class;
						break;
					}
					case OPERADORES::OP_ITR_MIN:
					{
						if(!inverse_operator)
							clase->getNormalOperators()->OPERATOR_min = operator_class;
						else
							clase->getRightOperators()->OPERATOR_min = operator_class;
						break;
					}
					case OPERADORES::OP_ITR_PLUS:
					{
						if(!inverse_operator)
							clase->getNormalOperators()->OPERATOR_plus = operator_class;
						else
							clase->getRightOperators()->OPERATOR_plus = operator_class;
						break;
					}
					case OPERADORES::OP_NEGADO:
					{
						if(!inverse_operator)
							clase->getNormalOperators()->OPERATOR_negado = operator_class;
						else
							clase->getRightOperators()->OPERATOR_negado = operator_class;
						break;
					}
					case OPERADORES::OP_BRACKET_LEFT:
					{
						if (!inverse_operator)
							clase->getNormalOperators()->OPERATOR_brack = operator_class;
						else
							clase->getRightOperators()->OPERATOR_brack = operator_class;
						break;
					}
					case OPERADORES::ELEM_NEG_FIRST:
					{
						if (!inverse_operator)
							clase->getNormalOperators()->OPERATOR_negado_first = operator_class;
						else
							clase->getRightOperators()->OPERATOR_negado_first = operator_class;
						break;
					}
					default:
					{
						deletePtr(operator_class);
						deletePtr(entrada);
						deletePtr(body);
						Errores::generarError(Errores::ERROR_CLASE_OPERADOR_INVALIDO, &clase->parametros, tokenizer.getTokenValue(index));
						return false;
					}
			}

			deletePtr(entrada);
			local_index = index;
			return true;
		}
	}
	return false;

}


Parser_Class* Parser::getClass(int& local_index)
{
	int index = local_index;

	if (tokenizer.getTokenValue(index) == "class")
	{
		Parser_Identificador* pID = getIdentificador(index);

		if (pID)
		{
			if (tokenizer.getTokenValue(index) == "{")
			{
				Parser_Class* clase = new Parser_Class(pID);

				//True si al menos hay un miembro estático.
				//Esto servirá para precargar la clase 
				bool haveStaticMember = false; 
				bool isPrivate = false;

				//Por ahora usaremos .variables_locales
				//En el caso de que en el futuro se decida implementar innerClass, se dejará .variables_clase para tomar valores del 'padre'
				SendVariables variables;

				//Creamos una variable automática - this - 
				this->readingClass = clase;
				variables.push_VarLocal(Variable("this", 0, true));
				this->readingClass->variables_map.emplace("this", 0);

				//Aquí buscaremos todos los valores, funciones, operadores y constructores de los que disponga la clase.
				while (tokenizer.getFirstNotCloseToken(index) != "}")
				{
					this->readingClass = clase; //Lo haremos cada vez que recorremos el while, pues en un futuro habra inner-class
					index--;
					bool isStatic = false;

					auto label = getLabelClass(index);

					if (label == LABEL_PUBLIC)
					{
						isPrivate = false;

						label = getLabelClass(index);
						if (label == LABEL_STATIC)
							isStatic = true;
					}
					else if (label == LABEL_PRIVATE)
					{
						isPrivate = true;

						label = getLabelClass(index);
						if (label == LABEL_STATIC)
							isStatic = true;
					}
					else if (label == LABEL_STATIC)
						isStatic = true;


					this->readingStaticValue = isStatic;

					Parser_ClassConstructor* pConst = getClassConstructor(index, variables.variables_locales);
					if (pConst)
					{
						if (isStatic)
						{
							Errores::generarError(Errores::ERROR_CLASE_STATIC_IS_NOT_VAR_FUNCT, NULL);
							delete clase;
							delete pConst;
							this->readingClass = nullptr;
							this->readingStaticValue = false;
							return NULL;
						}
						clase->getConstructores().emplace_back(pConst);
						continue;
					}

					bool operador = getClassOperadores(index, clase, variables.variables_locales);
					if (operador)
					{
						if (isStatic)
						{
							Errores::generarError(Errores::ERROR_CLASE_STATIC_IS_NOT_VAR_FUNCT, NULL);
							delete clase;
							this->readingClass = nullptr;
							this->readingStaticValue = false;
							return NULL;
						}
						continue;
					}


				
					Parser_Funcion* fnt = getFuncion(index, &variables.variables_locales);
					if (fnt)
					{
						fnt->is_Public = !isPrivate;
						fnt->is_Static = isStatic;

						if (isStatic)
							haveStaticMember = true;
						clase->getFuncionesBase().emplace_back(fnt);
						continue;	
					}


					arbol_operacional* pOp = getOperacionInd(index, variables, false, !isPrivate);
					if (pOp)
					{
						//Comprobamos que se trata de identificadores u operaciones de asignación.
				
						if (ComprobarIdentificadores(pOp) == NULL)
						{
							Errores::generarError(Errores::ERROR_CLASE_VARIABLE_NO_VALIDA, NULL, pID->nombre);
							this->readingClass = nullptr;
							this->readingStaticValue = false;
							return NULL;
						}

						pOp->is_Public = !isPrivate;
							
						if (isStatic)
						{
							clase->getVariablesStaticBase().emplace_back(pOp);
							haveStaticMember = true;
						}
						else
							clase->getVariableOperarBase().emplace_back(pOp); //Nunca será una clase core

						continue;
					}

					Errores::generarError(Errores::ERROR_CLASE_SINTAXIS, NULL, pID->nombre);
					deletePtr(clase);
					this->readingClass = nullptr;
					this->readingStaticValue = false;

					return NULL;
				}

				this->readingClass = nullptr;
				clase->preload_var = *variables.num_local_var;
				clase->preloadFunctions();

				this->readingStaticValue = false;
				local_index = index;
				return clase;
			}
		}
	}

	this->readingStaticValue = false;
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

/*
Buscamos la variable únicamente entre variables locales
*/
Variable* Parser::BusquedaVariableLocal(Parser_Identificador* ID, std::vector<Variable>& variables)
{
	for (std::vector<Variable>::iterator it = variables.begin(); it != variables.end(); ++it)
	{
		if (it->nombre == ID->nombre)
			return &(*it);
	}
	return NULL;
}



int Parser::BusquedaVariable(Parser_Identificador* ID, SendVariables& variables)
{
	return BusquedaVariable(ID, variables, NULL);
}


/*
Buscamos variables entre las globales y variables locales
*/
int Parser::BusquedaVariable(Parser_Identificador * ID, SendVariables& variables, Variable ** res)
{
	Variable* v = BusquedaVariableLocal(ID, variables.variables_locales);

	if (v != NULL)
	{
		if (res != NULL)
			*res = v;

		return v->index;
	}

	for (std::vector<Variable>::iterator it = this->variables_globales_parser->begin(); it != this->variables_globales_parser->end(); ++it)
	{
		if (it->nombre == ID->nombre)
		{
			ID->var_global = true;
			
			if (res != NULL)
				*res = &(*it);

			return (*it).index;
		}
	}

	if (/*this->readingFunction &&this->readingStaticValue &&*/ this->readingClass)
	{
		std::unordered_map<std::string, int>::iterator got = this->readingClass->static_var_map.find(ID->nombre);
		if (got != this->readingClass->static_var_map.end())
		{
			ID->index = got->second;
			ID->is_Static = true;
			ID->static_link = this->readingClass;

			return ID->index;
		}
	}

	if (variables.variables_clase != NULL && !this->readingStaticValue)
	{
		v = BusquedaVariableLocal(ID, *variables.variables_clase);
							
		if (v != NULL)
		{
			if (res != NULL)
				*res = v;

			ID->var_class = true;
			return v->index;
		}
	}

	return -1;
}

void Parser::CargarEnCacheOperaciones(arbol_operacional * arbol, SendVariables& variables, bool inside)
{
	if (arbol->operador == OP_NONE)
	{
		std::visit(overloaded
			{
				[&](Parser_Identificador * a)
				{
					Variable* var;
					int IndexVar = this->BusquedaVariable(a, variables, &var);
					if (IndexVar == -1)
					{
						//Si no existe y la variable no esta en una localizacion inside, la creamos.
						if (inside)
						{
							Errores::generarError(Errores::ERROR_VARIABLE_NO_EXISTE, &a->parametros, a->nombre);
							existenErrores = true;
						}
						else
						{
							a->inicializando = true;		

							if (this->isMain && a->var_global)
							{
									this->variables_globales_parser->push_back(Variable(a->nombre, this->variables_globales_parser->size(), true));
									a->index = this->variables_globales_parser->back().index;
							}
							else if (!this->readingFunction && this->readingStaticValue && this->readingClass) //Si no es una funcion, es que esta leyendo una variable
							{
								int inx = this->readingClass->static_var_map.size();
								this->readingClass->static_var_map.emplace(a->nombre, inx); // MIRAR - se ha cambiado
								a->index = inx;
								a->is_Static = true;
								a->static_link = this->readingClass;
							}
							else if (this->readingClass && !this->readingFunction)
							{
								int d = *variables.num_local_var;
								variables.push_VarLocal(Variable(a->nombre, d, true));
								this->readingClass->variables_map.emplace(a->nombre, d);
								a->index = d;
							}
							else
							{
								int d = *variables.num_local_var;
								variables.push_VarLocal(Variable(a->nombre, d, true)); 
								//if (this->readingClass)
								//	this->readingClass->variables_map.emplace(std::move(a->nombre), d);
								a->index = variables.variables_locales.back().index;
							}
						}
					}
					else
					{
						
						if (a->inicializando)
						{
							if (this->readingFunction || this->readingVector)
								var->inicializando = false;
							else
								var->inicializando = true;
						}

						else if (var->inicializando)
						{
							if(this->readingFunction || this->readingVector)
								var->inicializando = false;
							else
								a->inicializando = true;
						   //var->inicializando = false;
						}

						a->index = IndexVar;
					}

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
				[&](arbol_operacional * a) 
				{ 
					if (arbol->operador == OP_IG_EQUAL)
					{
						if (a->operador == OP_CLASS_ACCESS)
						{
							CargarEnCacheOperaciones(a, variables, inside);
						}
						else
						{
							Errores::generarError(Errores::ERROR_ASIGNACION_FALLO, &a->parametros);
							existenErrores = true;
						}
					}
					else
						CargarEnCacheOperaciones(a, variables,inside); 
				},
				[&](Parser_Identificador * a)
				{
					Variable* var;
					int IndexVar = this->BusquedaVariable(a, variables, &var);
					if (IndexVar == -1)
					{
						a->inicializando = true;

						//Si no existe, la creamos.
						if (this->isMain && a->var_global)
						{
							this->variables_globales_parser->push_back(Variable(a->nombre, this->variables_globales_parser->size()));
							a->index = this->variables_globales_parser->back().index;
						}
						else if (!this->readingFunction && this->readingStaticValue && this->readingClass)
						{
							int inx = this->readingClass->static_var_map.size();
							this->readingClass->static_var_map.emplace(a->nombre, inx);
							a->index = inx;
							a->is_Static = true;
							a->static_link = this->readingClass;
						}
						else if (this->readingClass && !this->readingFunction)
						{
							int d = *variables.num_local_var;
							variables.push_VarLocal(Variable(a->nombre, d));
							this->readingClass->variables_map.emplace(a->nombre, d);
							a->index = d;
						}
						else
						{
							int d = *variables.num_local_var;
							variables.push_VarLocal(Variable(a->nombre, d));
							//if (this->readingClass)
							//	this->readingClass->variables_map.emplace(std::move(a->nombre), d);
							a->index = variables.variables_locales.back().index;
						}
					}
					else
					{
						a->inicializando = var->inicializando;
						var->inicializando = false;
						a->index = IndexVar;
					}
				},
				[](auto&) {return; },
			}, arbol->_v1);

		std::visit(overloaded{
					[](const auto&) {},
					[&](arbol_operacional * a2) { CargarEnCacheOperaciones(a2, variables,inside); },
					[&](Parser_Identificador * a2)
					{
							Variable* var;
							int IndexVar = this->BusquedaVariable(a2, variables, &var);
							if (IndexVar == -1)
							{
								if (arbol->operador == OP_IG_EQUAL)
								{
									Errores::generarError(Errores::ERROR_VARIABLE_NO_EXISTE, &a2->parametros, a2->nombre);
									existenErrores = true;
								}
								else
								{
									a2->inicializando = true;

									//Si no existe, la creamos.
									if (this->isMain && a2->var_global)
									{
										this->variables_globales_parser->push_back(Variable(a2->nombre, this->variables_globales_parser->size()));
										a2->index = this->variables_globales_parser->back().index;

									}
									else if (!this->readingFunction && this->readingStaticValue && this->readingClass)
									{
										int inx = this->readingClass->static_var_map.size();
										this->readingClass->static_var_map.emplace(a2->nombre, inx);
										a2->index = inx;
										a2->is_Static = true;
										a2->static_link = this->readingClass;
									}
									else if (this->readingClass && !this->readingFunction)
									{
										int d = *variables.num_local_var;
										variables.push_VarLocal(Variable(a2->nombre, d));
										this->readingClass->variables_map.emplace(a2->nombre, d);
										a2->index = d;
									}
									else
									{
										int d = *variables.num_local_var;
										variables.push_VarLocal(Variable(a2->nombre, d));
										//if (this->readingClass)
										//	this->readingClass->variables_map.emplace(std::move(a2->nombre), d);
										a2->index = variables.variables_locales.back().index;
									}
								}
							}
							else 
							{
								a2->inicializando = var->inicializando;
								var->inicializando = false;
								a2->index = IndexVar;
							}
							
					},
			}, arbol->_v2);

	}
	else if (arbol->operador == OPERADORES::OP_CLASS_ACCESS)
	{
		std::visit(overloaded{
			[](auto&) {},
			[&](arbol_operacional * a) { CargarEnCacheOperaciones(a, variables,inside); },
			[&](Parser_Identificador * a)
			{
				int IndexVar= this->BusquedaVariable(a, variables);
				if (IndexVar == -1)
				{
					//En caso de que no exista el identificado, es posible que se trate de un identificador que hace referencia
					//a una clase y por lo tanto, el segundo parametro sea una operación estática sobre la miosma.
					//Guardamos por tanto la operación en lista_llamadas_estaticas, para comprobarlo al final del parseado, debido
					//que en el momento de pasar por esta función es posible que las clases aún no hayan sido leídas en su totalidad.
					lista_llamadas_estaticas.emplace_back(arbol);
				}
				else a->index = IndexVar;
			}
			}, arbol->_v1);

		std::visit(overloaded{
			[](auto&) {},
			[&](Call_Value * a)
			{

				 a->skip = true;
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
				int IndexVar = this->BusquedaVariable(a, variables);
				if (IndexVar == -1)
				{
					Errores::generarError(Errores::ERROR_VARIABLE_NO_EXISTE, &a->parametros, a->nombre);
					existenErrores = true;
				}
				else a->index = IndexVar;
			},
			}, arbol->_v1);


		std::visit(overloaded{
		[](const auto&) {},
		[&](arbol_operacional * a2) { CargarEnCacheOperaciones(a2, variables,inside); },
		[&](Parser_Identificador * a2)
		{
			int IndexVar2 = this->BusquedaVariable(a2, variables);
			if (IndexVar2 == -1)
			{
				Errores::generarError(Errores::ERROR_VARIABLE_NO_EXISTE, &a2->parametros, a2->nombre);
				existenErrores = true;
			}
			else a2->index = IndexVar2;
		},
			}, arbol->_v2);

	}
}

/**
	<identificador>.<operacion>
	Usado para confirmar si las operaciones en lista_llamadas_estaticas, son realmente llamadas a clases estáticas.
	De no serlo, devuelve error, de serlo será una llamada estatica a esa clase.
**/
void Parser::PreloadStaticCalls()
{

	for (std::vector<arbol_operacional*>::iterator it = this->lista_llamadas_estaticas.begin(); it != this->lista_llamadas_estaticas.end(); ++it)
	{
		//Para haber llegado hasta aquí, el primer punto (v1) damos por hecho que es IDENTIFICADOR.	
		Parser_Identificador* id_clase = std::get<Parser_Identificador*>((*it)->_v1);

		bool clase_existe = false;
		int itr;

		for (itr = 0; itr < Parser::clases.size(); itr++)
		{
			if ((Parser::clases)[itr]->getNombre() == id_clase->nombre)
			{
				id_clase->index = itr;
				(*it)->is_Static = true;
				clase_existe = true;
				break;
			}
		}
		
		if(!clase_existe)
		{
			Errores::generarError(Errores::ERROR_VARIABLE_NO_EXISTE, &(*it)->parametros, id_clase->nombre);
			this->existenErrores = true;
			continue;
		}

		bool existe = false;

		std::visit(overloaded{
			[&](auto&)
			{
				Errores::generarError(Errores::ERROR_CLASE_STATIC_ACCESS, &(*it)->parametros, id_clase->nombre);
				this->existenErrores = true;
			},
			[&](Call_Value* a)
			{
				std::vector<int> staticIds;

				if ((Parser::clases)[itr]->core)
				{
					int pID = (Parser::clases)[itr]->findFuncionCore(a->ID->nombre);


					if ((Parser::clases)[itr]->getFuncionesCore()[pID].is_Static)
					{
						existe = true;
						staticIds.emplace_back(pID);
					}
				}
				else
				{
					std::vector<int>* pIds = (Parser::clases)[itr]->findFuncionBase(a->ID->nombre);
					//std::vector<int>* 
					for (std::vector<int>::iterator interno = pIds->begin(); interno != pIds->end(); ++interno)
					{
						if ((Parser::clases)[itr]->getFuncionesBase()[*interno]->is_Static && a->entradas.size() == (Parser::clases)[itr]->getFuncionesBase()[*interno]->entradas.size())
						{
							staticIds.emplace_back(*interno);
							existe = true;
						}
					}
				}

				if (staticIds.size() == 0)
				{
					Errores::generarError(Errores::ERROR_CLASE_STATIC_FUNCTION_NOT_FOUND, &(*it)->parametros, a->ID->nombre, id_clase->nombre);
					this->existenErrores = true;
				}
				else
				{
					deletePtr(a->inx_class);
					a->setFuncion();
					a->inx_funcion->funcionesItrData = staticIds;
					a->is_Static = true; // En caso de funciones, avisaremos también al objeto de llamada, para evitar colisiones con la llamada de función.
					existe = true;
				}
			},
			[&](Parser_Identificador* a)
			{
				std::unordered_map<std::string, int>::iterator got = (Parser::clases)[itr]->static_var_map.find(a->nombre);
				if (got != (Parser::clases)[itr]->static_var_map.end())
				{
					a->index = got->second;
					a->is_Static = true; // Avisamos que esta operación en concreto es estática, para el interprete.
					existe = true;
				}
				else
				{
					Errores::generarError(Errores::ERROR_CLASE_STATIC_VAR_NOT_FOUND, &(*it)->parametros, a->nombre, id_clase->nombre);
					this->existenErrores = true;
				}

			},
		}, (*it)->_v2);
	}
}

//Preload de funciones y clases
bool Parser::preloadCalls()
{
	for (std::vector<Call_Value*>::iterator it = this->valores_llamadas.begin(); it != this->valores_llamadas.end(); ++it)
	{
		if ((*it)->skip)
			continue;

		bool existe = false;

		if ((*it)->is_class) //Se trata del constructor de una clase
		{
			for (int itr = 0; itr < Parser::clases.size(); itr++)
			{
				//Debe coincidir el nombre de la misma.
				if ((Parser::clases)[itr]->getNombre() == (*it)->ID->nombre)
				{
					(*it)->setClass();

					if ((Parser::clases)[itr]->isCore)
					{
						(*it)->AddClass(itr, 0);
						existe = true;
						break;
					}

					bool is_ok = false;
					//Comprobamos si existe un constructor adecuado para la clase
					for (int inx_const = 0; inx_const < (Parser::clases)[itr]->getConstructores().size(); inx_const++)
					{
						if ((Parser::clases)[itr]->getConstructores()[inx_const]->entradas.size() / 2 != (*it)->entradas.size())
							continue;

						(*it)->AddClass(itr, inx_const);
						is_ok = true;
						existe = true;
						break;
					}

					if (is_ok)
						break;

					//Constructor predefinido si no existen constructores definidos por el usuario.
					if ((*it)->entradas.size() == 0 && (Parser::clases)[itr]->getConstructores().size() == 0)
					{
						(*it)->AddClass(itr, -1);
						existe = true;
						break;
					}
				}
			}

			if (!existe)
			{
				Errores::generarError(Errores::ERROR_CLASE_NO_DECLARADA, &(*it)->parametros, (*it)->ID->nombre);
				this->existenErrores = true;
				continue;
			}
		}
		else // Se trata de una función
		{
			if ((*it)->inside_class)
			{
				if ((*it)->inside_class->core)
				{
					for (int itr = 0; itr < (*it)->inside_class->getFuncionesCore().size(); itr++)
					{
						if ((*it)->inside_class->getFuncionesCore()[itr].nombre == (*it)->ID->nombre)
						{
							//Establece que es una función
							(*it)->setFuncion();

							//TODO: Entradas en core?
							//if ((*it)->inside_class->getFunciones<Core_Function*>()[itr]->entradas.size() != (*it)->entradas.size())
							//	continue;

							if ((*it)->function_parent_is_static && !(*it)->inside_class->getFuncionesCore()[itr].is_Static)
								continue;

							(*it)->AddFuncion(itr);
							(*it)->isInsideClass = true;
							existe = true;
						}
					}
				}
				else
				{
					for (int itr = 0; itr < (*it)->inside_class->getFuncionesBase().size(); itr++)
					{
						if ((*it)->inside_class->getFuncionesBase()[itr]->pID->nombre == (*it)->ID->nombre)
						{
							//Establece que es una función
							(*it)->setFuncion();

							if ((*it)->inside_class->getFuncionesBase()[itr]->entradas.size() != (*it)->entradas.size())
								continue;

							if ((*it)->function_parent_is_static && !(*it)->inside_class->getFuncionesBase()[itr]->is_Static)
								continue;

							(*it)->AddFuncion(itr);
							(*it)->isInsideClass = true;
							existe = true;
						}
					}
				}
			}

			if (existe)
				continue;

			for (int itr = 0; itr < Core::core_functions.size(); itr++)
			{
				if (Core::core_functions[itr]->nombre == (*it)->ID->nombre)
				{
					//Establece que es una función
					(*it)->setFuncion();
					(*it)->inx_funcion->isCore = true;
					//TODO: Entradas en core?
					//if (Core::core_functions[itr]->entradas.size() != (*it)->entradas.size())
					//	continue;

					(*it)->AddFuncion_Core(itr);
					existe = true;
				}
			}

			if (existe)
				continue;

			for (int itr = 0; itr < funciones.size(); itr++)
			{
				//Debe coincidir el nombre de la misma.
				if (funciones[itr]->pID->nombre == (*it)->ID->nombre)
				{
					(*it)->setFuncion();

					//si no tiene el mismo numero de entradas, saltamos, no es esta función.
					if (funciones[itr]->entradas.size() != (*it)->entradas.size())
						continue;

					(*it)->AddFuncion(itr);
					existe = true;
				}
			}

			if (existe)
				continue;

			Errores::generarError(Errores::ERROR_FUNCION_NO_DECLARADA, &(*it)->parametros, (*it)->ID->nombre);
			this->existenErrores = true;
			continue;
		}

	}
	PreloadStaticCalls();
	//this->valores_funciones.clear();
	//this->valores_funciones.shrink_to_fit();
	return true;
}





