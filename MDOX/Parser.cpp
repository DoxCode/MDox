
#include "Parser.h"
#include "Funciones.h"
#include <string>
#include <iomanip>

// ############################################################
// ######################## LITERALES  ######################## 
// ############################################################

Parser_Literal * Parser::getLiteral(int& local_index)
{
	int index = local_index;
	
	//#########  Probando si puede tratarse de un STRING
	if (tokenizer.getTokenValue(index) == "\"")
	{
		std::string value = tokenizer.getTokenValue(index);

		if (value == "\"")
		{
			Value_STRING * LS = new Value_STRING("");
			local_index = index;
			Parser_Literal * sif = new Parser_Literal(LS);
			sif->generarPosicion(&tokenizer);
			return sif;
		}

		if (tokenizer.getTokenValue(index) == "\"")
		{
			Value_STRING * LS = new Value_STRING(value);
			local_index = index;
			Parser_Literal * sif = new Parser_Literal(LS);
			sif->generarPosicion(&tokenizer);
			return sif;
		}
	}

	//######### Probando si puede tratarse de un FLOAT o un ENTERO
	index = local_index;
	std::string token = tokenizer.getTokenValue(index);

	bool negativo = false;

	if (token == "-")
	{
		negativo = true;
		token = tokenizer.getTokenValue(index);
	}

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

				double value = s2d(doub,std::locale::classic());
				//double value = std::stold(doub);

				if (negativo) value = -value;

				Value_DOUBLE * LS = new Value_DOUBLE(value);
				local_index = l_index;

				Parser_Literal * sif = new Parser_Literal(LS);
				sif->generarPosicion(&tokenizer);
				return sif;
			}

			//Si llegamos aquí, no es un DOUBLE; pero tampoco un INT.
		}
		else
		{
			//Se trata de un ENTERO
			int value = std::stoi(token);
			if (negativo) value = -value;
			Value_INT * LS = new Value_INT(value);
			local_index = index;
			Parser_Literal * sif = new Parser_Literal(LS);
			sif->generarPosicion(&tokenizer);
			return sif;
		}
	}

	//######### Probando si puede tratarse de un booleano
	index = local_index;
	token = tokenizer.getTokenValue(index);
	if (token == "true")
	{
		Value_BOOL * LB = new Value_BOOL(true);
		local_index = index;
		Parser_Literal * sif = new Parser_Literal(LB);
		sif->generarPosicion(&tokenizer);
		return sif;
	}
	else if (token == "false")
	{
		Value_BOOL * LB = new Value_BOOL(false);
		local_index = index;
		Parser_Literal * sif = new Parser_Literal(LB);
		sif->generarPosicion(&tokenizer);
		return sif;
	}


	return NULL;
}

// ############################################################
// ###################### DECLARATIVOS  ####################### 
// ############################################################

Parser_Declarativo * Parser::getDeclarativo(int& local_index)
{
	int index = local_index;

	std::string token = tokenizer.getTokenValue(index); 
	

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
	else if (token == "void")
	{
		Declarativo_SingleValue * p = new Declarativo_SingleValue(PARAM_VOID);
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
				std::string tk = tokenizer.getTokenValue(t_index);
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
						deletePtr(*it);
					}
					values.clear();
					break;
				}
			}
			else
			{
				for (std::vector<Parser_Declarativo*>::iterator it = values.begin(); it != values.end(); ++it)
				{
					deletePtr(*it);
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
		deletePtr(d);
		return NULL;
	}

	index = local_index;

	Parser_Literal* l = getLiteral(index);

	if (l)
	{
		deletePtr(l);
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
		Parser_Identificador * sif = new Parser_Identificador(token);
		sif->generarPosicion(&tokenizer);
		return sif;
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

	if (tokenizer.getTokenValue(index) == "!")
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
		if (tokenizer.getTokenValue(i_index) == "(") // Identificador (x1,x2,x3...) -> LLamada  a una función. Entradas = parámetros
		{
			std::vector<Parser_Operacion*> entradas;

			while (true)
			{
				int t_index = i_index;
				Parser_Operacion * p = getOperacion(t_index);

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
						Valor_Funcion * v = new Valor_Funcion(i, entradas);
						local_index = i_index;
						v->negado = negado;
						v->generarPosicion(&tokenizer);
						return v;
					}
					else
					{
						for (std::vector<Parser_Operacion*>::iterator it = entradas.begin(); it != entradas.end(); ++it)
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
						Valor_Funcion * v = new Valor_Funcion(i, entradas);
						local_index = t_index;
						v->negado = negado;
						v->generarPosicion(&tokenizer);
						return v;
					}

					for (std::vector<Parser_Operacion*>::iterator it = entradas.begin(); it != entradas.end(); ++it)
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
	std::string tkn = tokenizer.getTokenValue(index);
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

	std::string token = tokenizer.getTokenValue(index);

	//Comprobamos si se trata de una operación binaria con prioridades de paréntesis.
	if (token == "(")
	{
		int l_index = index;
		Parser_Operacion * op1 = getOperacion(l_index);

		if (op1)
		{
			if (tokenizer.getTokenValue(l_index) == ")")
			{
				int l_index2 = l_index;
				Parser_Math * op2 = getMath(l_index2);

				if (op2)
				{
					local_index = l_index2;
					Operacion_Recursiva * sif = new Operacion_Recursiva(op1, op2);
					sif->generarPosicion(&tokenizer);
					return sif;
				}
				else
				{
					//delete op2;
					local_index = l_index;
					Operacion_Recursiva * sif = new Operacion_Recursiva(op1);
					sif->generarPosicion(&tokenizer);
					return sif;
				}

			}
			else // Si no encuentra el paréntesis de cierre, habrá algo mal en la sintaxis.
			{
				deletePtr(op1);
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
		std::string token = tokenizer.getTokenValue(index);

		if (token == "++")
		{
			local_index = index;
			Operacion_ID * sif = new Operacion_ID(id, ID_INCREMENTO);
			sif->generarPosicion(&tokenizer);
			return sif;
		}
		else if (token == "--")
		{
			local_index = index;
			Operacion_ID * sif = new Operacion_ID(id, ID_DECREMENTO);
			sif->generarPosicion(&tokenizer);
			return sif;
		}
		deletePtr(id);
	}

	// El primer elemento NO es un identificador
	index = local_index;

	std::string vl_token = tokenizer.getTokenValue(index);

	if (vl_token == "++")
	{
		Parser_Identificador * i2 = getIdentificador(index);

		if (i2)
		{
			local_index = index;
			Operacion_ID * sif = new Operacion_ID(i2, ID_INCREMENTO);
			sif->generarPosicion(&tokenizer);
			return sif;
		}
	}
	else if (vl_token == "--")
	{
		Parser_Identificador * i2 = getIdentificador(index);

		if (i2)
		{
			local_index = index;
			Operacion_ID * sif = new Operacion_ID(i2, ID_DECREMENTO);
			sif->generarPosicion(&tokenizer);
			return sif;
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
			Operacion_Math * sif = new Operacion_Math(pValor, pMath);
			sif->generarPosicion(&tokenizer);
			return sif;
		}

		local_index = index;
		Operacion_Math * sif = new Operacion_Math(pValor);
		sif->generarPosicion(&tokenizer);
		return sif;
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

			Parametro_Declarativo_ID * sif = new Parametro_Declarativo_ID(pD1, pID);
			sif->generarPosicion(&tokenizer);
			return sif;
		}
		
		deletePtr(pID);
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

	if (tokenizer.getTokenValue(index) == "(")
	{
		std::vector<Parser_Operacion*> valor;
		int t_index = index;

		if (tokenizer.getTokenValue(t_index) == ")")
		{
			local_index = t_index;
			Parametro_Tupla * sif = new Parametro_Tupla(valor);
			sif->generarPosicion(&tokenizer);
			return sif;
		}

		t_index = index;

		while (true)
		{
			Parser_Operacion * pOp = getOperacion(t_index);

			if (pOp)
			{
				valor.push_back(pOp);
				std::string token = tokenizer.getTokenValue(t_index);

				if (token == ",")
				{
					continue;
				}
				else if (token == ")")
				{
					local_index = t_index;
					Parametro_Tupla * sif = new Parametro_Tupla(valor);
					sif->generarPosicion(&tokenizer);
					return sif;
				}
				else
				{
					for (std::vector<Parser_Operacion*>::iterator it = valor.begin(); it != valor.end(); ++it)
					{
						deletePtr(*it);
					}

					return NULL;
				}
			}

			for (std::vector<Parser_Operacion*>::iterator it = valor.begin(); it != valor.end(); ++it)
			{
				deletePtr(*it);
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
		std::string token = tokenizer.getTokenValue(index);
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
			deletePtr(pPar);
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
			Parser_Igualdad * sif = new Parser_Igualdad(pPar, pCond, tipo);
			sif->generarPosicion(&tokenizer);
			return sif;
		}
		

		deletePtr(pPar);
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

	std::string token = tokenizer.getTokenValue(index);
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

	std::string token = tokenizer.getTokenValue(index);
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
			{
				for (std::vector<Condicional_Agregada_Operacional*>::iterator it = valor->begin(); it != valor->end(); ++it)
				{
					deletePtr(*it);
				}
				deletePtr(valor);
				Condicional_Operacion * sif = new Condicional_Operacion(pOp, NULL, pCa);
				sif->generarPosicion(&tokenizer);
				return sif; 
			}
			else
			{
				Condicional_Operacion * sif = new Condicional_Operacion(pOp, valor, pCa);
				sif->generarPosicion(&tokenizer);
				return sif;
			}
		}
		else
		{
			local_index = t_index;
			if (valor->size() == 0)
			{
				for (std::vector<Condicional_Agregada_Operacional*>::iterator it = valor->begin(); it != valor->end(); ++it)
				{
					deletePtr(*it);
				}
				deletePtr(valor);
				Condicional_Operacion * sif = new Condicional_Operacion(pOp);
				sif->generarPosicion(&tokenizer);
				return sif;
			}
			else
			{
				Condicional_Operacion * sif = new Condicional_Operacion(pOp, valor);
				sif->generarPosicion(&tokenizer);
				return sif;
			}
		}
	}


	//Comprobando RECURSIVA condicional
	index = local_index;
	std::string tk1 = tokenizer.getTokenValue(index);

	bool called = false;
	bool Negado = false;

	if (tk1 == "!")
	{
		Negado = true;

		if (tokenizer.getTokenValue(index) == "(")
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
			if (tokenizer.getTokenValue(index) == ")")
			{
				int t_index = index;
				Condicional_Agregada * ca = getCondAgregada(t_index);
				
				if (ca)
				{
					local_index = t_index;
					Condicional_Recursiva * sif = new Condicional_Recursiva(Negado, pCond, ca);
					sif->generarPosicion(&tokenizer);
					return sif;
				}
				else
				{
					local_index = index;
					Condicional_Recursiva * sif = new Condicional_Recursiva(Negado, pCond);
					sif->generarPosicion(&tokenizer);
					return sif;
				}
			}

			deletePtr(pCond);
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

	if (tokenizer.getTokenValue(index) == "{")
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

				if (tokenizer.getTokenValue(t_index) == "}")
				{
					local_index = t_index;
					Sentencia_Recursiva * sif = new Sentencia_Recursiva(valor);
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
			Parser_Condicional * pCond = getCondicional(index);

			if (pCond)
			{
				if (tokenizer.getTokenValue(index) == ")")
				{
					Parser_Sentencia * pSent = getSentencia(index);

					if (pSent)
					{
						int t_index = index;

						if (tokenizer.getTokenValue(t_index) == "else")
						{
							Parser_Sentencia * pElse = getSentencia(t_index);
							
							if (pElse)
							{
								local_index = t_index;
								Sentencia_IF *sif = new Sentencia_IF(pCond, pSent, pElse);
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
							Sentencia_IF *sif = new Sentencia_IF(pCond, pSent);
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
			Parser_Condicional * pCond = getCondicional(index);

			if (pCond)
			{
				if (tokenizer.getTokenValue(index) == ")")
				{
					Parser_Sentencia * pSent = getSentencia(index);

					if (pSent)
					{
						local_index = index;
						Sentencia_WHILE *sif = new Sentencia_WHILE(pCond, pSent);
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
			Parser_Igualdad * pIg = getIgualdad(index);
			if (tokenizer.getTokenValue(index) == ";")
			{
				Parser_Condicional * pCond = getCondicional(index);

				if (tokenizer.getTokenValue(index) == ";")
				{
					Parser_Operacion * pOp = getOperacion(index);

						if (tokenizer.getTokenValue(index) == ")")
						{
							Parser_Sentencia * pSent = getSentencia(index);

							if (pSent)
							{
								local_index = index;
								Sentencia_FOR *sif = new Sentencia_FOR(pIg, pCond, pOp, pSent);
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
		Parser_Operacion * pOp = getOperacion(index);

		if (pOp)
		{
			if (tokenizer.getTokenValue(index) == ";")
			{
				local_index = index;
				Sentencia_Return *sif = new Sentencia_Return(pOp);
				sif->generarPosicion(&tokenizer);
				return sif;
			}
			deletePtr(pOp);
		}
		else
		{
			if (tokenizer.getTokenValue(index) == ";")
			{
				Sentencia_Return *sif = new Sentencia_Return(NULL);
				sif->generarPosicion(&tokenizer);
				return sif;
			}
		}
	}

	//##########   -- SENTENCIA PRINT --   ##########
	index = local_index;

	if (tokenizer.getTokenValue(index) == "print")
	{
		if (tokenizer.getTokenValue(index) == "(")
		{
			Parser_Operacion * pOp = getOperacion(index);

			if (pOp)
			{
				if (tokenizer.getTokenValue(index) == ")")
				{
					if (tokenizer.getTokenValue(index) == ";")
					{
						local_index = index;
						Sentencia_Print *sif = new Sentencia_Print(pOp);
						sif->generarPosicion(&tokenizer);
						return sif;
					}
				}
				deletePtr(pOp);
			}
		}
	}

	//##########   -- SENTENCIA VARIABLE INIT --   ##########
	index = local_index;

	Parser_Parametro * pPar = getParametro(index);

	if (pPar)
	{

		if (tokenizer.getTokenValue(index) == ";")
		{

			local_index = index;
			Sentencia_Parametro *sif = new Sentencia_Parametro(pPar);
			sif->generarPosicion(&tokenizer);
			return sif;
		}
		deletePtr(pPar);
	}

	//##########   -- SENTENCIA OPERACIONAL --   ##########
	 index = local_index;

	Parser_Operacion * pOp = getOperacion(index);

	if (pOp)
	{
		if (tokenizer.getTokenValue(index) == ";")
		{
			local_index = index;
			Sentencia_Operacional *sif = new Sentencia_Operacional(pOp);
			sif->generarPosicion(&tokenizer);
			return sif;
		}
		deletePtr(pOp);
	}


	//##########   -- SENTENCIA Igualdad --   ##########
	 index = local_index;

	Parser_Igualdad * pIg = getIgualdad(index);

	if (pIg)
	{
		if (tokenizer.getTokenValue(index) == ";")
		{
			local_index = index;
			Sentencia_Igualdad *sif = new Sentencia_Igualdad(pIg);
			sif->generarPosicion(&tokenizer);
			return sif;
		}
		deletePtr(pIg);
	}

	index = local_index;
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

	Parser_Operacion * pLit = getOperacion(index);

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


	if (tokenizer.getTokenValue(index) == "function")
	{
		Parser_Identificador * pID = getIdentificador(index);

		if (pID)
		{
			if (tokenizer.getTokenValue(index) == "(")
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

							for (std::vector<Funcion_ValorEntrada*>::iterator it = entradas.begin(); it != entradas.end(); ++it)
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

							for (std::vector<Funcion_ValorEntrada*>::iterator it = entradas.begin(); it != entradas.end(); ++it)
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
					Parser_Declarativo * pDecl = getDeclarativo(t3_index);

					if (pDecl)
					{
						Parser_Sentencia* pSent = getSentencia(t3_index);

						if (pSent)
						{
							local_index = t3_index;
							Parser_Funcion *sif = new Parser_Funcion(pID, entradas, pSent, pDecl);
							sif->generarPosicion(&tokenizer);
							return sif;
						}
						else
						{
							deletePtr(pDecl);
							deletePtr(pID);

							for (std::vector<Funcion_ValorEntrada*>::iterator it = entradas.begin(); it != entradas.end(); ++it)
							{
								deletePtr(*it);
							}
							return NULL;
						}
					}
					else
					{
						deletePtr(pID);

						for (std::vector<Funcion_ValorEntrada*>::iterator it = entradas.begin(); it != entradas.end(); ++it)
						{
							deletePtr(*it);
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
						Parser_Funcion *sif = new  Parser_Funcion(pID, entradas, pSent);
						sif->generarPosicion(&tokenizer);
						return sif;
					}
					else
					{
						deletePtr(pID);

						for (std::vector<Funcion_ValorEntrada*>::iterator it = entradas.begin(); it != entradas.end(); ++it)
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