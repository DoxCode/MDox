
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
			long long value = std::stoll(token);
			if (negativo) value = -value;

			Value * LS;
			if(value > INT_MAX)
				LS = new Value_LINT(value);
			else
				LS = new Value_INT(value);

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
	else if (token == "lint")
	{
		Declarativo_SingleValue * p = new Declarativo_SingleValue(PARAM_LINT);
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
// ##################### OPERADORES  ########################## 
// ############################################################

Parser_Operador * Parser::getOperador(int& local_index)
{
	int index = local_index;
	std::string tkn = tokenizer.getTokenValue(index);
	OPERADORES acc;

	//Operadores aritméticos
	if (tkn == "+")
	{
		acc = OP_ARIT_SUMA;
	}
	else if (tkn == "-")
	{
		acc = OP_ARIT_RESTA;
	}
	else if (tkn == "/")
	{
		acc = OP_ARIT_DIV;
	}
	else if (tkn == "*")
	{
		acc = OP_ARIT_MULT;
	}
	else if (tkn == "%")
	{
		acc = OP_ARIT_MOD;
	}
	else if (tkn == "div")
	{
		acc = OP_ARIT_DIV_ENTERA;
	}
	// Operadores relacionales
	else if (tkn == "==")
	{
		acc = OP_REL_EQUAL;
	}
	else if (tkn == "!=")
	{
		acc = OP_REL_NOT_EQUAL;
	}
	else if (tkn == ">")
	{
		acc = OP_REL_MAJOR;
	}
	else if (tkn == "<")
	{
		acc = OP_REL_MINOR;
	}
	else if (tkn == "<=")
	{
		acc = OP_REL_MINOR_OR_EQUAL;
	}
	else if (tkn == ">=")
	{
		acc = OP_REL_MAJOR_OR_EQUAL;
	}
	// Operadores lógicos
	else if (tkn == "&&")
	{
		acc = OP_LOG_ADD;
	}
	else if (tkn == "||")
	{
		acc = OP_LOG_OR;
	}

	else // No se trata de un token de acción matemática. Puede tratarse del último valor de la serie.
	{
		return NULL;
	}

	Parser_Operacion * op = getOperacion(index);

	if (op)
	{
		local_index = index;
		return new Parser_Operador(acc, op);
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

	bool negado = false;
	//Comprobamos si se trata de una operación binaria con prioridades de paréntesis.
	if (token == "!")
	{
		negado = true;
		token = tokenizer.getTokenValue(index);
	}

	if (token == "(")
	{
		int l_index = index;
		Parser_Operacion * op1 = getOperacion(l_index);

		if (op1)
		{
			if (tokenizer.getTokenValue(l_index) == ")")
			{
				int l_index2 = l_index;
				Parser_Operador * op2 = getOperador(l_index2);

				if (op2)
				{
					local_index = l_index2;
					Operacion_Recursiva * sif = new Operacion_Recursiva(op1, op2);
					sif->generarPosicion(&tokenizer);
					sif->negado = negado;
					return sif;
				}
				else
				{
					//delete op2;
					local_index = l_index;
					Operacion_Recursiva * sif = new Operacion_Recursiva(op1);
					sif->generarPosicion(&tokenizer);
					sif->negado = negado;
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
			sif->ID->negado = negado;
			sif->generarPosicion(&tokenizer);
			return sif;
		}
		else if (token == "--")
		{
			local_index = index;
			Operacion_ID * sif = new Operacion_ID(id, ID_DECREMENTO);
			sif->ID->negado = negado;
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
			sif->ID->negado = negado;
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
			sif->ID->negado = negado;
			sif->generarPosicion(&tokenizer);
			return sif;
		}
	}
	// #######  EXPRESIONES MATEMÁTICAS -- Comenzando por igualdad #######
	index = local_index;
	Operacion_Igualdad* pOi = getIgualdad(index);
	
	if (pOi != NULL)
	{
		local_index = index;
		return pOi;
	}

	// #######  EXPRESIONES MATEMÁTICAS #######
	//Comprobando expresión matemática. -> <VALOR> [MATH]
	index = local_index;

	Parser_Valor* pValor = getValor(index);

	if (pValor)
	{
		int t_valor = index;
		Parser_Operador* pMath = getOperador(t_valor);

		if (pMath)
		{
			local_index = t_valor;
			Operacion_Operador * sif = new Operacion_Operador(pValor, pMath);
			sif->generarPosicion(&tokenizer);
			return sif;
		}
		//En el caso de que el operador no sea compatible matemático, por lo que
		// puede ser, o el final de un operador matematico o una igualdad paramétrica.
		else 	
		{
			int t_index = local_index;
	

			Operacion_Igualdad* oi = getIgualdad(t_index);

			if (oi != NULL)
			{
				local_index = t_index;
				delete pValor;
				return oi;
			}
		}

		local_index = index;
		Operacion_Operador * sif = new Operacion_Operador(pValor);
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

Operacion_Igualdad * Parser::getIgualdad(int& local_index)
{
	int index = local_index;
	Parser_Parametro * pPar = getParametro(index);

	if (pPar)
	{
		int p_index = index;
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
	/*	else if (token == ";")
		{
			local_index = p_index;
			Operacion_Igualdad * sif = new Operacion_Igualdad(pPar, NULL, IG_NONE);
			sif->generarPosicion(&tokenizer);
			return sif;
		}*/
		else 
		{
			deletePtr(pPar);
			return NULL;
		}


		Parser_Operacion * pCond = getOperacion(index);

		if (pCond)
		{
			local_index = index;
			Operacion_Igualdad * sif = new Operacion_Igualdad(pPar, pCond, tipo);
			sif->generarPosicion(&tokenizer);
			return sif;
		}
		

		deletePtr(pPar);
		return NULL;
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
			Parser_Operacion * pCond = getOperacion(index);

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
			Parser_Operacion * pCond = getOperacion(index);

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
			Operacion_Igualdad * pIg = getIgualdad(index);
			if (tokenizer.getTokenValue(index) == ";")
			{
				Parser_Operacion * pCond = getOperacion(index);

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
	if (tokenizer.getTokenValue(index) == "<::")
	{
		Parser_Operacion * pOp = getOperacion(index);
		if (pOp)
		{
				if (tokenizer.getTokenValue(index) == ";")
				{
					local_index = index;
					Sentencia_Print *sif = new Sentencia_Print(pOp);
					sif->generarPosicion(&tokenizer);
					return sif;
				}
			deletePtr(pOp);
		}
	}

	// #######  IGUALDAD ESPECIAL   #######
	//Igualdad de estilo especial x; int x; double y; es decir, PARAMETROS
	index = local_index;
	Parser_Parametro * pPar = getParametro(index);

	if (pPar)
	{
		if (tokenizer.getTokenValue(index) == ";")
		{
			local_index = index;
			Operacion_Igualdad * sif1 = new Operacion_Igualdad(pPar, NULL, IG_NONE);
			Sentencia_Operacional *sif = new Sentencia_Operacional(sif1);
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