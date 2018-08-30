
#include "Parser.h"
#include "Funciones.h"
#include <string> 

// ############################################################
// ######################## LITERALES  ######################## 
// ############################################################

Parser_Literal * Parser::getLiteral(int& local_index)
{
	int index = local_index;
	
	//#########  Probando si puede tratarse de un STRING
	if (tokenizer.getToken(index) == "\"")
	{
		std::string value = tokenizer.getToken(index);

		if (value == "\"")
		{
			Value_STRING * LS = new Value_STRING("");
			local_index = index;
			return new Parser_Literal(LS);
		}

		if (tokenizer.getToken(index) == "\"")
		{
			Value_STRING * LS = new Value_STRING(value);
			local_index = index;
			return new Parser_Literal(LS);
		}
	}

	//######### Probando si puede tratarse de un FLOAT o un ENTERO
	index = local_index;
	std::string token = tokenizer.getToken(index);

	if (is_number(token))
	{
		int l_index = index;
		if (tokenizer.getToken(l_index) == ".")
		{
			std::string token2 = tokenizer.getToken(l_index);
			if (is_number(token2))
			{
				//Es un double
				// Siempre calcularemos doubles, si luego hay que reducirlo a float, se hará
				// en la creación del arbol de acción, nunca en el parser.
				std::string doub = token + "." + token2;
				double value = std::stod(doub);
				Value_DOUBLE * LS = new Value_DOUBLE(value);
				local_index = l_index;
				return new Parser_Literal(LS);
			}

			//Si llegamos aquí, no es un DOUBLE; pero tampoco un INT.
		}
		else
		{
			//Se trata de un ENTERO
			int value = std::stoi(token);
			Value_INT * LS = new Value_INT(value);
			local_index = index;
			return new Parser_Literal(LS);
		}
	}

	//######### Probando si puede tratarse de un booleano
	index = local_index;
	token = tokenizer.getToken(index);
	if (token == "true")
	{
		Value_BOOL * LB = new Value_BOOL(true);
		local_index = index;
		return new Parser_Literal(LB);
	}
	else if (token == "false")
	{
		Value_BOOL * LB = new Value_BOOL(false);
		local_index = index;
		return new Parser_Literal(LB);
	}


	return NULL;
}

// ############################################################
// ###################### DECLARATIVOS  ####################### 
// ############################################################

Parser_Declarativo * Parser::getDeclarativo(int& local_index)
{
	int index = local_index;

	std::string token = tokenizer.getToken(index); 
	

	if (token == "string")
	{
		Declarativo_SingleValue * p = new Declarativo_SingleValue(PARAM_STRING);
		local_index = index;
		return new Parser_Declarativo(DEC_SINGLE, p);
	}
	else if (token == "int")
	{
		Declarativo_SingleValue * p = new Declarativo_SingleValue(PARAM_INT);
		local_index = index;
		return new Parser_Declarativo(DEC_SINGLE, p);
	}
	else if (token == "double")
	{
		Declarativo_SingleValue * p = new Declarativo_SingleValue(PARAM_DOUBLE);
		local_index = index;
		return new Parser_Declarativo(DEC_SINGLE, p);
	}	
	else if (token == "bool")
	{
		Declarativo_SingleValue * p = new Declarativo_SingleValue(PARAM_BOOL);
		local_index = index;
		return new Parser_Declarativo(DEC_SINGLE, p);
	}

	//Probamos si puede tratarse de una tupla
	if (token == "(")
	{
		std::vector<Parser_Declarativo*> values;

		while (true)
		{
			int t_index = index;
			Parser_Declarativo * p = getDeclarativo(t_index);

			if (p)
			{
				values.push_back(p);
				std::string tk = tokenizer.getToken(t_index);
				index = t_index;

				if (tk == ",")
				{
					continue;
				}
				else if (tk == ")")
				{
					Declarativo_MultiValue * p2 = new Declarativo_MultiValue(PARAM_TUPLA, values);
					local_index = index;
					return new Parser_Declarativo(DEC_MULTI, p2);
				}
				else
				{
					for (std::vector<Parser_Declarativo*>::iterator it = values.begin(); it != values.end(); ++it)
					{
						delete (*it);
					}
					values.clear();
					break;
				}
			}
			else
			{
				for (std::vector<Parser_Declarativo*>::iterator it = values.begin(); it != values.end(); ++it)
				{
					delete (*it);
				}
				values.clear();
				return NULL;
			}
		}
	}


//	index = local_index;
	return NULL;
}

// ############################################################
// ##################### IDENTIFICADOR  ####################### 
// ############################################################

Parser_Identificador * Parser::getIdentificador(int& local_index)
{
	int index = local_index;

	Parser_Declarativo* d = getDeclarativo(index);

	if (d)
	{
		delete d;
		return NULL;
	}

	index = local_index;

	Parser_Literal* l = getLiteral(index);

	if (l)
	{
		delete l;
		return NULL;
	}

	//Habría que comprobar también que no se trate de una función específica del tipo:
	// return, break, if, else, while ... 

	index = local_index;
	std::string token = tokenizer.getToken(index);

	if (is_Identificador(token))
	{
		local_index = index;
		return new Parser_Identificador(token);
	}

	return NULL;

}

// ############################################################
// ######################### VALOR  ########################### 
// ############################################################

Parser_Valor * Parser::getValor(int& local_index)
{
	int index = local_index;
	bool negado = false;

	if (tokenizer.getToken(index) == "!")
	{
		negado = true;
	}
	else index = local_index;


	//Comprobamos que sea un literal.
	Parser_Literal * l = getLiteral(index);

	if (l)
	{
		local_index = index;
		l->negado = negado;
		return l;
	}

	//En caso de no ser un literal, podrá ser o una llamada a una función o un identificador.
	index = local_index;
	Parser_Identificador * i  = getIdentificador(index);
	
	if (i)
	{
		int i_index = index;
		if (tokenizer.getToken(i_index) == "(") // Identificador (x1,x2,x3...) -> LLamada  a una función. Entradas = parámetros
		{
			std::vector<Parser_Operacion*> entradas;

			while (true)
			{
				int t_index = i_index;
				Parser_Operacion * p = getOperacion(t_index);

				if (p)
				{
					entradas.push_back(p);
					std::string token = tokenizer.getToken(t_index);
					i_index = t_index;

					if (token == ",")
					{
						continue;
					}
					else if (token == ")")
					{
						Valor_Funcion * v = new Valor_Funcion(i, entradas);
						local_index = i_index;
						v->negado = negado;
						return v;
					}
					else
					{
						for (std::vector<Parser_Operacion*>::iterator it = entradas.begin(); it != entradas.end(); ++it)
						{
							delete (*it);
						}
						delete i;
						break;
					}
				}
				else
				{
					for (std::vector<Parser_Operacion*>::iterator it = entradas.begin(); it != entradas.end(); ++it)
					{
						delete (*it);
					}
					delete i;
					break;
				}
			}
		}
		else //Es un IDENTIFICADOR
		{
			local_index = index;
			i->negado = negado;
			return i;
		}
	}


	index = local_index;

	return NULL;
}

// ############################################################
// ######################### MATH  ############################ 
// ############################################################

Parser_Math * Parser::getMath(int& local_index)
{
	int index = local_index;
	std::string tkn = tokenizer.getToken(index);
	MATH_ACCION acc;

	if (tkn == "+")
	{
		acc = MATH_SUMA;
	}
	else if (tkn == "-")
	{
		acc = MATH_RESTA;
	}
	else if (tkn == "/")
	{
		acc = MATH_DIV;
	}
	else if (tkn == "*")
	{
		acc = MATH_MULT;
	}
	else if (tkn == "^")
	{
		acc = MATH_EXP;
	}
	else if (tkn == "%")
	{
		acc = MATH_MOD;
	}
	else if (tkn == "div")
	{
		acc = MATH_DIV_ENTERA;
	}
	else // No se trata de un token de acción matemática. Puede tratarse del último valor de la serie.
	{
		return NULL;
	}

	Parser_Operacion * op = getOperacion(index);

	if (op)
	{
		local_index = index;
		return new Parser_Math(acc, op);
	}

	return NULL;
}


// ############################################################
// ####################### OPERACIÓN  #########################
// ############################################################


Parser_Operacion * Parser::getOperacion(int& local_index)
{
	int index = local_index;

	std::string token = tokenizer.getToken(index);

	//Comprobamos si se trata de una operación binaria con prioridades de paréntesis.
	if (token == "(")
	{
		int l_index = index;
		Parser_Operacion * op1 = getOperacion(l_index);

		if (op1)
		{
			if (tokenizer.getToken(l_index) == ")")
			{
				int l_index2 = l_index;
				Parser_Math * op2 = getMath(l_index2);

				if (op2)
				{
					local_index = l_index2;
					return new Operacion_Recursiva(op1, op2);
				}
				else
				{
					//delete op2;
					local_index = l_index;
					return new Operacion_Recursiva(op1);
				}

			}
			else // Si no encuentra el paréntesis de cierre, habrá algo mal en la sintaxis.
			{
				delete op1;
				return NULL;
			}
		}
	}

	// #######  EXPRESIONES CON IDENTIFICADORES  #######
	//Comprobando expresiones de identificadores
	//Comprobando si primero se encuentra el identificador
	index = local_index;

	Parser_Identificador * id = getIdentificador(index);

	// El primer elmento es un identificador.
	if (id)
	{
		std::string token = tokenizer.getToken(index);

		if (token == "++")
		{
			local_index = index;
			return new Operacion_ID(id, ID_INCREMENTO);
		}
		else if (token == "--")
		{
			local_index = index;
			return new Operacion_ID(id, ID_DECREMENTO);
		}
		delete id;
	}

	// El primer elemento NO es un identificador
	index = local_index;

	std::string vl_token = tokenizer.getToken(index);

	if (vl_token == "++")
	{
		Parser_Identificador * i2 = getIdentificador(index);

		if (i2)
		{
			local_index = index;
			return new Operacion_ID(i2, ID_INCREMENTO);
		}
	}
	else if (vl_token == "--")
	{
		Parser_Identificador * i2 = getIdentificador(index);

		if (i2)
		{
			local_index = index;
			return new Operacion_ID(i2, ID_DECREMENTO);
		}
	}

	// #######  EXPRESIONES MATEMÁTICAS #######
	//Comprobando expresión matemática. -> <VALOR> [MATH]
	index = local_index;

	Parser_Valor* pValor = getValor(index);

	if (pValor)
	{
		int t_valor = index;
		Parser_Math* pMath = getMath(t_valor);

		if (pMath)
		{
			local_index = t_valor;
			return new Operacion_Math(pValor, pMath);
		}

		local_index = index;
		return new Operacion_Math(pValor);
	}
	return NULL;
}

// ############################################################
// ####################### PARAMETRO  #########################
// ############################################################


Parser_Parametro * Parser::getParametro(int& local_index)
{

	// ###### Comprobamos si se trata de un declarativo `+ Identificador ######
	// Declarativo + Identificados -> int varA
	int index = local_index;

	Parser_Declarativo* pD1 = getDeclarativo(index);
	 
	//Si el primer valor es un declarativo, puede tratarse de un declarativo + identificador
	// o de algún tipo de puntero. <<Punteros aún no implementados.>>
	if (pD1)
	{
		int index2 = index;

		Parser_Identificador * pID = getIdentificador(index2);

		if (pID)
		{
			local_index = index2;
			return new Parametro_Declarativo_ID(pD1, pID);
		}
		
		delete pID;
		//Comprobar PUNTEROS
	}


	// ###### Comprobamos si es solamente un identificador ######
	 index = local_index;

	Parser_Identificador * pID = getIdentificador(index);

	if (pID)
	{
		local_index = index;
		return pID;
	}

	// ###### Comprobamos si se trata de una tupla ######

	 index = local_index;

	if (tokenizer.getToken(index) == "(")
	{
		std::vector<Parser_Operacion*> valor;
		int t_index = index;

		if (tokenizer.getToken(t_index) == ")")
		{
			local_index = t_index;
			return new Parametro_Tupla(valor);
		}

		t_index = index;

		while (true)
		{
			Parser_Operacion * pOp = getOperacion(t_index);

			if (pOp)
			{
				valor.push_back(pOp);
				std::string token = tokenizer.getToken(t_index);

				if (token == ",")
				{
					continue;
				}
				else if (token == ")")
				{
					local_index = t_index;
					return new Parametro_Tupla(valor);
				}
				else
				{
					for (std::vector<Parser_Operacion*>::iterator it = valor.begin(); it != valor.end(); ++it)
					{
						delete (*it);
					}

					return NULL;
				}
			}

			for (std::vector<Parser_Operacion*>::iterator it = valor.begin(); it != valor.end(); ++it)
			{
				delete (*it);
			}

			return NULL;
		}
	}

	//int index = local_index;
	return NULL;
}


// ############################################################
// ####################### IGUALDAD  ##########################
// ############################################################

Parser_Igualdad * Parser::getIgualdad(int& local_index)
{
	int index = local_index;
	Parser_Parametro * pPar = getParametro(index);

	if (pPar)
	{
		std::string token = tokenizer.getToken(index);
		IgualdadType tipo;

		if (token == "=")
		{
			tipo = IG_EQUAL;
		}
		else if (token == "+=")
		{
			tipo = IG_ADD_EQ;
		}
		else if (token == "-=")
		{
			tipo = IG_SUB_EQ;
		}
		else
		{
			delete pPar;
			return NULL;
		}

/*		int index2 = index;
		Parser_Operacion * pOp = getOperacion(index2);

		if (pOp)
		{
			local_index = index2;
			return new Parser_Igualdad(pPar, pOp, tipo);
		}
*/
		Parser_Condicional * pCond = getCondicional(index);

		if (pCond)
		{
			local_index = index;
			return new Parser_Igualdad(pPar, pCond, tipo);
		}
		

		delete pPar;
		return NULL;
	}

	return NULL;
}


// ############################################################
// ##################### CONDICIONAL  #########################
// ############################################################

Condicional_Agregada * Parser::getCondAgregada(int& local_index)
{
	int index = local_index;

	std::string token = tokenizer.getToken(index);
	CondicionalAgregadoType act;

	if (token == "&&")
	{
		act = COND_AG_AND;
	}
	else if (token == "||")
	{
		act = COND_AG_OR;
	}
	else
	{
		return NULL;
	}

	Parser_Condicional * pCond = getCondicional(index);

	if (pCond)
	{
		local_index = index;
		return new Condicional_Agregada(act, pCond);
	}

	return NULL;
}

Condicional_Agregada_Operacional* Parser::getCondAgregadaOperacional(int& local_index)
{
	int index = local_index;

	std::string token = tokenizer.getToken(index);
	CondicionalAccionType act;

	if (token == "==")
	{
		act = COND_ACC_EQUAL;
	}
	else if (token == "!=")
	{
		act = COND_ACC_NOT_EQUAL;
	}
	else if (token == ">")
	{
		act = COND_ACC_MAJOR;
	}
	else if (token == "<")
	{
		act = COND_ACC_MINOR;
	}
	else if (token == "<=")
	{
		act = COND_ACC_MINOR_OR_EQUAL;
	}
	else if (token == ">=")
	{
		act = COND_ACC_MAJOR_OR_EQUAL;
	}
	else
		return NULL;

	Parser_Operacion * pOp = getOperacion(index);

	if (pOp)
	{
		local_index = index;
		return new Condicional_Agregada_Operacional(act, pOp);
	}

	return NULL;
}

Parser_Condicional * Parser::getCondicional(int& local_index)
{
	//Comprobando condicional OPERACIONAL
	int index = local_index;

	Parser_Operacion * pOp = getOperacion(index);

	if (pOp)
	{
		std::vector<Condicional_Agregada_Operacional*>* valor = new std::vector<Condicional_Agregada_Operacional*>();
		int t_index = index;

		while (true)
		{
			int t2_index = t_index;
			Condicional_Agregada_Operacional * pCao = getCondAgregadaOperacional(t2_index);

			if (pCao)
			{
				valor->push_back(pCao);
				t_index = t2_index;
				continue;
			}
			else
				break;
		}

		/*if (valor->size() == 0)
		{
		local_index = index;
		return new Condicional_Operacion(pOp);
		}*/

		int t2_index = t_index;
		Condicional_Agregada * pCa = getCondAgregada(t2_index);

		if (pCa)
		{
			local_index = t2_index;
			if (valor->size() == 0)
				return new Condicional_Operacion(pOp, NULL, pCa);
			else
				return new Condicional_Operacion(pOp, valor, pCa);
		}
		else
		{
			local_index = t_index;
			if (valor->size() == 0)
				return new Condicional_Operacion(pOp);
			else
				return new Condicional_Operacion(pOp, valor);
		}
	}


	//Comprobando RECURSIVA condicional
	index = local_index;
	std::string tk1 = tokenizer.getToken(index);

	bool called = false;
	bool Negado = false;

	if (tk1 == "!")
	{
		Negado = true;

		if (tokenizer.getToken(index) == "(")
		{
			called = true;
		}
	}
	else if (tk1 == "(")
	{
		called = true;
	}


	if (called)
	{
		Parser_Condicional * pCond = getCondicional(index);

		if (pCond)
		{
			if (tokenizer.getToken(index) == ")")
			{
				int t_index = index;
				Condicional_Agregada * ca = getCondAgregada(t_index);
				
				if (ca)
				{
					local_index = t_index;
					return new Condicional_Recursiva(Negado, pCond, ca);
				}
				else
				{
					local_index = index;
					return new Condicional_Recursiva(Negado, pCond);
				}
			}

			delete pCond;
		}
	}

	


	return NULL;
}

// ############################################################
// ####################### SENTENCIA  #########################
// ############################################################

Parser_Sentencia * Parser::getSentencia(int& local_index)
{
	//##########   -- SENTENCIA RECURSIVA --   ##########
	int index = local_index;

	if (tokenizer.getToken(index) == "{")
	{
		std::vector<Parser_Sentencia*> valor;

		while (true)
		{
			int t_index = index;
			Parser_Sentencia * pS = getSentencia(t_index);

			if (pS)
			{
				valor.push_back(pS);

				index = t_index;

				if (tokenizer.getToken(t_index) == "}")
				{
					local_index = t_index;
					return new Sentencia_Recursiva(valor);
				}
			}
			else
			{
				local_index = index;
				return new Sentencia_Recursiva(valor);
			}
		}
	}

	//##########   -- SENTENCIA IF --   ##########
	 index = local_index;

	if (tokenizer.getToken(index) == "if")
	{
		if (tokenizer.getToken(index) == "(")
		{
			Parser_Condicional * pCond = getCondicional(index);

			if (pCond)
			{
				if (tokenizer.getToken(index) == ")")
				{
					Parser_Sentencia * pSent = getSentencia(index);

					if (pSent)
					{
						int t_index = index;

						if (tokenizer.getToken(t_index) == "else")
						{
							Parser_Sentencia * pElse = getSentencia(t_index);
							
							if (pElse)
							{
								local_index = t_index;
								return new Sentencia_IF(pCond, pSent, pElse);
							}
							else
							{
								delete pSent;
								delete pCond;
								return NULL;
							}
						}
						else
						{
							local_index = index;
							return new Sentencia_IF(pCond, pSent);
						}

						delete pSent;
					}
				}

				delete pCond;
			}
			else/*else condicional*/ return NULL;
		}
	}

	//##########   -- SENTENCIA WHILE --   ##########
	 index = local_index;

	if (tokenizer.getToken(index) == "while")
	{
		if (tokenizer.getToken(index) == "(")
		{
			Parser_Condicional * pCond = getCondicional(index);

			if (pCond)
			{
				if (tokenizer.getToken(index) == ")")
				{
					Parser_Sentencia * pSent = getSentencia(index);

					if (pSent)
					{
						local_index = index;
						return new Sentencia_WHILE(pCond, pSent);
					}
					else
					{
						delete pCond;
						return NULL;
					}
				}

				delete pCond;
			}
			else return NULL;
		}
	}

	//##########   -- SENTENCIA FOR --   ##########
	 index = local_index;

	if (tokenizer.getToken(index) == "for")
	{
		if (tokenizer.getToken(index) == "(")
		{
			Parser_Igualdad * pIg = getIgualdad(index);

			if (pIg)
			{
				if (tokenizer.getToken(index) == ";")
				{
					Parser_Condicional * pCond = getCondicional(index);

					if (pCond)
					{
						if (tokenizer.getToken(index) == ";")
						{
							Parser_Operacion * pOp = getOperacion(index);

							if (pOp)
							{
								if (tokenizer.getToken(index) == ")")
								{
									Parser_Sentencia * pSent = getSentencia(index);

									if (pSent)
									{
										local_index = index;
										return new Sentencia_FOR(pIg, pCond, pOp, pSent);
									}
									else
									{
										delete pOp;
										delete pCond;
										delete pIg;
										return NULL;
									}
								}
								delete pOp;
							}
							else 
							{
								delete pCond;
								delete pIg;
								return NULL;
							}
						}

						delete pCond;
					}
					else 
					{
						delete pIg;
						return NULL;
					}
				}

				delete pIg;
			}
			else return NULL;
		}
	}

	//##########   -- SENTENCIA RETURN --   ##########
	index = local_index;

	if (tokenizer.getToken(index) == "return")
	{
		Parser_Operacion * pOp = getOperacion(index);

		if (pOp)
		{
			if (tokenizer.getToken(index) == ";")
			{
				local_index = index;
				return new Sentencia_Return(pOp);
			}
			delete pOp;
		}
	}

	//##########   -- SENTENCIA PRINT --   ##########
	index = local_index;

	if (tokenizer.getToken(index) == "print")
	{
		if (tokenizer.getToken(index) == "(")
		{
			Parser_Operacion * pOp = getOperacion(index);

			if (pOp)
			{
				if (tokenizer.getToken(index) == ")")
				{
					if (tokenizer.getToken(index) == ";")
					{
						local_index = index;
						return new Sentencia_Print(pOp);
					}
				}
				delete pOp;
			}
		}
	}

	//##########   -- SENTENCIA VARIABLE INIT --   ##########
	index = local_index;

	Parser_Parametro * pPar = getParametro(index);

	if (pPar)
	{

		if (tokenizer.getToken(index) == ";")
		{

			local_index = index;
			return new Sentencia_Parametro(pPar);
		}
		delete pPar;
	}

	//##########   -- SENTENCIA OPERACIONAL --   ##########
	 index = local_index;

	Parser_Operacion * pOp = getOperacion(index);

	if (pOp)
	{
		if (tokenizer.getToken(index) == ";")
		{
			local_index = index;
			return new Sentencia_Operacional(pOp);
		}
		delete pOp;
	}


	//##########   -- SENTENCIA Igualdad --   ##########
	 index = local_index;

	Parser_Igualdad * pIg = getIgualdad(index);

	if (pIg)
	{
		if (tokenizer.getToken(index) == ";")
		{
			local_index = index;
			return new Sentencia_Igualdad(pIg);
		}
		delete pIg;
	}






	return NULL;
}

// ############################################################
// ######################## FUNCION  ##########################
// ############################################################

Funcion_ValorEntrada * Parser::getEntrada(int& local_index)
{
	int index = local_index;

	Parser_Parametro * pPar = getParametro(index);
	
	if (pPar)
	{
		local_index = index;
		return pPar;
	}
	
	Parser_Literal * pLit = getLiteral(index);

	if (pLit)
	{
		local_index = index;
		return pLit;
	}

	return NULL;
}


Parser_Funcion * Parser::getFuncion(int& local_index)
{
	int index = local_index;


	if (tokenizer.getToken(index) == "function")
	{
		Parser_Identificador * pID = getIdentificador(index);

		if (pID)
		{
			if (tokenizer.getToken(index) == "(")
			{

				std::vector<Funcion_ValorEntrada*> entradas;
				
				int t_index = index;
				while (true)
				{
					int t2_index = t_index;
					Funcion_ValorEntrada * pFve = getEntrada(t2_index);

					if (pFve)
					{
						entradas.push_back(pFve);

						std::string token = tokenizer.getToken(t2_index);

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
							delete pID;

							for (std::vector<Funcion_ValorEntrada*>::iterator it = entradas.begin(); it != entradas.end(); ++it)
							{
								delete (*it);
							}

							return NULL;
						}
					}
					else 
					{
						if (tokenizer.getToken(t_index) != ")")
						{
							delete pID;

							for (std::vector<Funcion_ValorEntrada*>::iterator it = entradas.begin(); it != entradas.end(); ++it)
							{
								delete (*it);
							}

							return NULL;
						}
					}
				}

				int t3_index = index;

				if (tokenizer.getToken(t3_index) == "::")
				{
					Parser_Declarativo * pDecl = getDeclarativo(t3_index);

					if (pDecl)
					{
						Parser_Sentencia* pSent = getSentencia(t3_index);

						if (pSent)
						{
							local_index = t3_index;
							return new Parser_Funcion(pID, entradas, pSent, pDecl);
						}
						else
						{
							delete pDecl;
							delete pID;

							for (std::vector<Funcion_ValorEntrada*>::iterator it = entradas.begin(); it != entradas.end(); ++it)
							{
								delete (*it);
							}
							return NULL;
						}
					}
					else
					{
						delete pID;

						for (std::vector<Funcion_ValorEntrada*>::iterator it = entradas.begin(); it != entradas.end(); ++it)
						{
							delete (*it);
						}
						return NULL;
					}
				}
				else
				{
					Parser_Sentencia* pSent = getSentencia(index);
					if (pSent)
					{
						local_index = index;
						return new Parser_Funcion(pID, entradas, pSent);
					}
					else
					{
						delete pID;

						for (std::vector<Funcion_ValorEntrada*>::iterator it = entradas.begin(); it != entradas.end(); ++it)
						{
							delete (*it);
						}
						return NULL;
					}
				}

			}
		
			delete pID;
		}
	}

	return NULL;
}