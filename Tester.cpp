
#include "Tester.h"
#include "Parser.h"
#include <deque>
/*
Clase usada para testear distintas partes del sistema.
 Los métodos se guardaran por si hacen falta en un futuro.
*/


void debug_mostrar(Parser_Literal* p, int tb)
{
	if (!p)
	{
		std::cout << "NULO \n";
		return;
	}

		if (p->tipo == PARAM_INT)
		{
			Value_INT* x = static_cast<Value_INT *>(p->value);
			std::cout << tabulacion(tb) << "LITERAL INT: " << x->value;
		}
		else if (p->tipo == PARAM_DOUBLE)
		{
			Value_DOUBLE* x = static_cast<Value_DOUBLE *>(p->value);
			std::cout << tabulacion(tb) << "LITERAL DOUBLE: " << x->value;
		}
		else if (p->tipo == PARAM_STRING)
		{
			Value_STRING* x = static_cast<Value_STRING *>(p->value);
			std::cout << tabulacion(tb) << "LITERAL STRING: " << x->value;
		}

		std::cout << "\n";
}

void debug_mostrarDec(Parser_Declarativo* p, int tb)
{
	if (!p)
	{
		std::cout << "NULO \n";
		return;
	}

	if (p->tipo == DEC_SINGLE)
	{
		Declarativo_SingleValue* x = static_cast<Declarativo_SingleValue *>(p->value);
		std::cout << tabulacion(tb) << "DECLARATIVO: VALUE: "  << NameParameter(x->value);
	}
	else
	{
		Declarativo_MultiValue* x = static_cast<Declarativo_MultiValue *>(p->value);

		std::cout << tabulacion(tb) << "DECLARATIVO: " << NameParameter(x->tipo) << ": { \n";

		for (unsigned itr = 0; itr < x->value.size(); itr++)
		{
			debug_mostrarDec(x->value.at(itr), tb+1);
		}
		std::cout << tabulacion(tb) << " } \n";
	}
	std::cout << "\n";
}

void debug_mostrarID(Parser_Identificador* p, int tb)
{
	if (!p)
	{
		std::cout << "NULO \n";
		return;
	}
	std::cout << tabulacion(tb) << "ID: " << p->nombre << "\n";
}


void debug_mostrarValor(Parser_Valor* p, int tb)
{
	if (!p)
	{
		std::cout << "NULO \n";
		return;
	}

	if (p->tipo == VAL_ID)
	{
		Parser_Identificador* x = static_cast<Parser_Identificador *>(p);
		debug_mostrarID(x,tb);
	}
	else if (p->tipo == VAL_LIT)
	{
		Parser_Literal* x = static_cast<Parser_Literal *>(p);
		debug_mostrar(x, tb);
	}
	else if (p->tipo == VAL_FUNC)
	{
		Valor_Funcion* x = static_cast<Valor_Funcion *>(p);
	
		std::cout << tabulacion(tb) << "FUNCION: ID: " << x->ID->nombre << " Entradas: \n";

		for (unsigned itr = 0; itr < x->entradas.size(); itr++)
		{
			debug_mostrarOperaciones(x->entradas.at(itr), tb + 1);
		}

		std::cout << "\n";
	}
}


void debug_mostrarMath(Parser_Math* p,int tb)
{
	if (!p)
	{
		std::cout << "NULO \n";
		return;
	}

	std::cout << tabulacion(tb) << "MATH_EXP: [";
	
	switch (p->accion)
	{
		case MATH_SUMA: std::cout << " + "; break;
		case MATH_MULT:std::cout << " * "; break;
		case MATH_DIV:std::cout << " / "; break;
		case MATH_DIV_ENTERA:std::cout << " div "; break;
		case MATH_RESTA:std::cout << " - "; break;
		case MATH_EXP:std::cout << " ^ "; break;
		case MATH_MOD:std::cout << " % "; break;
	}
	std::cout << "] \n";
	debug_mostrarOperaciones(p->op, tb + 1);

}

void debug_mostrarOperaciones(Parser_Operacion* p,int tb)
{
	if (!p)
	{
		std::cout << "NULO \n";
		return;
	}

	if (p->tipo == OP_REC_OP)
	{
		std::cout << tabulacion(tb) << "OPERACION: \n";
		Operacion_Recursiva * x = static_cast<Operacion_Recursiva *>(p);

		debug_mostrarOperaciones(x->op1, tb + 1);

		if(x->op2)
			debug_mostrarMath(x->op2, tb + 1);

	}
	else if (p->tipo == OP_MATH)
	{
		std::cout << tabulacion(tb) << "OPERACION: \n";
		Operacion_Math * x = static_cast<Operacion_Math *>(p);

		debug_mostrarValor(x->op1, tb + 1);

		if (x->op2)
			debug_mostrarMath(x->op2, tb + 1);
	}
	else if (p->tipo == OP_ID)
	{
		std::cout << tabulacion(tb) << "OPERACION DDD: \n";
		Operacion_ID * x = static_cast<Operacion_ID *>(p);
		debug_mostrarID(x->ID, tb + 1);
		
		switch (x->accion)
		{
			case ID_INCREMENTO: std::cout << tabulacion(tb) << " ++ "; break;
			case ID_DECREMENTO: std::cout << tabulacion(tb) << " -- "; break;
			case ID_NEGACION: std::cout << tabulacion(tb) << " ! "; break;
		}
	}
}

void debug_mostrarCondicional(Parser_Condicional* p, int tb)
{
	if (!p)
	{
		std::cout << "NULO \n";
		return;
	}

	if (p->tipo == COND_REC)
	{
		Condicional_Recursiva * x = static_cast<Condicional_Recursiva*>(p);
		if(x->negada)
			std::cout << tabulacion(tb) << "CONDICION NEGADA: \n";
		else
			std::cout << tabulacion(tb) << "CONDICION: \n";

		debug_mostrarCondicional(x->c1, tb + 1);

		if (x->ca)
		{
			if (x->ca->accion == COND_AG_AND)
			{
				std::cout << tabulacion(tb) << "&&\n";
			}
			else if (x->ca->accion == COND_AG_OR)
			{
				std::cout << tabulacion(tb) << "||\n";
			}

			debug_mostrarCondicional(x->ca->cond, tb + 1);
		}

	}
	else if (p->tipo == COND_OP)
	{
		std::cout << tabulacion(tb) << "CONDICION OP: \n";
		Condicional_Operacion * x = static_cast<Condicional_Operacion *>(p);

		debug_mostrarOperaciones(x->op1, tb + 1);

	

		if (x->adicional_ops)
		{
			for (unsigned itr = 0; itr < x->adicional_ops->size(); itr++)
			{
				Condicional_Agregada_Operacional * y = x->adicional_ops->at(itr);
				std::cout << tabulacion(tb) << "OP: [" << NameConditions(y->AccType) << "]\n";
				debug_mostrarOperaciones(y->op, tb + 1);
			}
		}

		if (x->ca)
		{
			if (x->ca->accion == COND_AG_AND)
			{
				std::cout << tabulacion(tb) << "&&\n";
			}
			else if (x->ca->accion == COND_AG_OR)
			{
				std::cout << tabulacion(tb) << "||\n";
			}

			debug_mostrarCondicional(x->ca->cond, tb + 1);
		}
	}
}

void debug_mostrarParametro(Parser_Parametro* p, int tb)
{
	if (!p)
	{
		std::cout << "NULO \n";
		return;
	}

	if (p->tipo == PRM_DECLARATIVO_ID)
	{
		Parametro_Declarativo_ID * x = static_cast<Parametro_Declarativo_ID*>(p);
		std::cout << tabulacion(tb) << "PARAMETRO: \n";
		debug_mostrarDec(x->pDec, tb + 1);
		debug_mostrarID(x->pID, tb + 1);
	}
	else if (p->tipo == PRM_ID)
	{
		Parser_Identificador * x = static_cast<Parser_Identificador*>(p);
		std::cout << tabulacion(tb) << "PARAMETRO: \n";
		debug_mostrarID(x, tb + 1);
	}
	else if (p->tipo == PRM_TUPLA)
	{
		Parametro_Tupla * x = static_cast<Parametro_Tupla*>(p);
		std::cout << tabulacion(tb) << "PARAMETRO TUPLA: \n";
		
		for (unsigned itr = 0; itr < x->value.size(); itr++)
		{
			debug_mostrarOperaciones(x->value.at(itr), tb + 1);
		}
	}
}

void debug_mostrarIgualdad(Parser_Igualdad* p, int tb)
{
	if (!p)
	{
		std::cout << "NULO \n";
		return;
	}
	std::cout << tabulacion(tb) << "IGUALDAD: \n";
	debug_mostrarParametro(p->param, tb + 1);
	
	switch (p->valor)
	{
	case IG_EQUAL: std::cout << tabulacion(tb) << "[ = ]\n";break;
	case IG_ADD_EQ: std::cout << tabulacion(tb) << "[ += ]\n"; break;
	case IG_SUB_EQ: std::cout << tabulacion(tb) << "[ -= ]\n"; break;
	}

	debug_mostrarOperaciones(p->op, tb + 1);
}

void debug_mostrarSentencia(Parser_Sentencia* p, int tb)
{
	if (!p)
	{
		std::cout << "NULO \n";
		return;
	}
	
	if (p->tipo == SENT_REC)
	{

		Sentencia_Recursiva * x = static_cast<Sentencia_Recursiva*>(p);
		std::cout << tabulacion(tb) << "SENTENCIA <" << x->valor.size() << ">: \n";
	

		if (x->valor.size() == 0)
		{
			std::cout << tabulacion(tb) << "NULL \n";
			return;
		}

		for (unsigned itr = 0; itr < x->valor.size(); itr++)
		{
			debug_mostrarSentencia(x->valor.at(itr), tb + 1);
		}
	}
	else if (p->tipo == SENT_IF)
	{
		std::cout << tabulacion(tb) << "IF: \n";
		Sentencia_IF * x = static_cast<Sentencia_IF*>(p);

		std::cout << tabulacion(tb+1) << "CONDICION: \n";
		debug_mostrarCondicional(x->pCond, tb + 2);

		std::cout << tabulacion(tb+1) << "ACCION: \n";
		debug_mostrarSentencia(x->pS, tb + 2);

		if (x->pElse)
		{
			std::cout << tabulacion(tb+1) << "ELSE: \n";
			debug_mostrarSentencia(x->pElse, tb + 2);
		}
	}
	else if (p->tipo == SENT_WHILE)
	{
		std::cout << tabulacion(tb) << "WHILE: \n";
		Sentencia_WHILE * x = static_cast<Sentencia_WHILE*>(p);

		std::cout << tabulacion(tb+2) << "CONDICION: \n";
		debug_mostrarCondicional(x->pCond, tb + 1);

		std::cout << tabulacion(tb+1) << "REPETICION: \n";
		debug_mostrarSentencia(x->pS, tb + 2);
	}
	else if (p->tipo == SENT_FOR)
	{
		std::cout << tabulacion(tb) << "FOR: \n";
		Sentencia_FOR * x = static_cast<Sentencia_FOR*>(p);
		std::cout << tabulacion(tb + 1) << "PAR1: \n";
		debug_mostrarIgualdad(x->pIguald, tb + 2);
		std::cout << tabulacion(tb + 1) << "PAR2: \n";
		debug_mostrarCondicional(x->pCond, tb + 2);
		std::cout << tabulacion(tb + 1) << "PAR3: \n";
		debug_mostrarOperaciones(x->pOp, tb + 2);

		std::cout << tabulacion(tb + 1) << "REPETICION: \n";
		debug_mostrarSentencia(x->pS, tb + 2);
	}
	else if (p->tipo == SENT_RETURN)
	{
		Sentencia_Return * x = static_cast<Sentencia_Return*>(p);
		std::cout << tabulacion(tb) << "RETURN: \n";
		debug_mostrarOperaciones(x->pOp, tb + 1);
	}
	else if (p->tipo == SENT_OP)
	{
		Sentencia_Operacional * x = static_cast<Sentencia_Operacional*>(p);
		std::cout << tabulacion(tb) << "SENTENCIA: \n";
		debug_mostrarOperaciones(x->pOp, tb + 1);
	}
	else if (p->tipo == SENT_IGU)
	{
		Sentencia_Igualdad * x = static_cast<Sentencia_Igualdad*>(p);
		std::cout << tabulacion(tb) << "SENTENCIA: \n";
		debug_mostrarIgualdad(x->pIg, tb + 1);
	}
	else if (p->tipo == SENT_VAR_INI)
	{
		Sentencia_Parametro * x = static_cast<Sentencia_Parametro*>(p);
		std::cout << tabulacion(tb) << "SENTENCIA: \n";
		debug_mostrarParametro(x->pPar, tb + 1);
	}

}

void debug_mostrarFuncion(Parser_Funcion* p, int tb)
{
	if (!p)
	{
		std::cout << "NULO \n";
		return;
	}

	std::cout << tabulacion(tb) << "DECLARACION FUNCION: "<< " Nombre: " << p->pID->nombre <<"\n";
	std::cout << tabulacion(tb) << "PARAMETROS ENTRADA: \n";

	for (unsigned itr = 0; itr < p->entradas.size(); itr++)
	{
		if (p->entradas.at(itr)->tipo == ENTRADA_VALOR)
		{
			Parser_Valor * x = static_cast<Parser_Valor*>(p->entradas.at(itr));
			debug_mostrarValor(x, tb + 1);
		}
		else if (p->entradas.at(itr)->tipo == ENTRADA_PARAM)
		{
			Parser_Parametro * x = static_cast<Parser_Parametro*>(p->entradas.at(itr));
			debug_mostrarParametro(x, tb + 1);
		}
	}

	if (p->salida)
	{
		std::cout << tabulacion(tb) << "VALOR DE SALIDA: \n";
		debug_mostrarDec(p->salida,tb + 1);
	}
	
	std::cout << tabulacion(tb) << "CODIGO: \n";
	debug_mostrarSentencia(p->body, tb + 1);

}


std::string NameParameter(tipos_parametros a)
{
	switch (a)
	{
	case PARAM_INT: return "INT";
	case PARAM_CHAR:return "CHAR";
	case PARAM_FLOAT:return "FLOAT";
	case PARAM_DOUBLE:return "DOUBLE";
	case PARAM_BOOL:return "BOOL";
	case PARAM_STRUCT:return "STRUCT";
	case PARAM_ENUM:return "ENUM";
	case PARAM_STRING:return "STRING";
	case PARAM_LIST:return "LIST";
	case PARAM_VECTOR:return "VECTOR";
	case PARAM_DEQUE:return "DEQUE";
	case PARAM_TUPLA:return "TUPLA";
	}
}

std::string NameConditions(CondicionalAccionType a)
{
	switch (a)
	{
	case COND_ACC_EQUAL: return "==";
	case COND_ACC_NOT_EQUAL:return "!=";
	case COND_ACC_MINOR:return "<";
	case COND_ACC_MAJOR:return ">";
	case COND_ACC_MINOR_OR_EQUAL:return "<=";
	case COND_ACC_MAJOR_OR_EQUAL:return ">=";
	}
}


std::string tabulacion(int n)
{
	std::string res = "";
	for (unsigned t = 0; t < n; t++)
	{
		res += "  ";
	}
	return res;
}
