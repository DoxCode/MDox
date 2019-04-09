/**
*
*	Primer paso del Interprete, encargado de cargar las variables que se van a usar antes de usarlas.
*	Esto aumentará el rendimiento, aunque bien producirá un leve aumento del consumo de ram, 
*	disminuirá notablemente el tiempo de ejecución.
*
**/

#include "Interprete.h"


Variable * Interprete::BusquedaVariable(std::string ID, std::vector<Variable> * variables)
{
	for (int itr = 0; itr < variables->size(); itr++)
	{
		if ((*variables)[itr].nombre == ID)
			return &((*variables)[itr]);
	}
	return NULL;
}

void Interprete::RemoverVariablesScope(int deep, std::vector<Variable> * variables)
{
	std::vector<Variable>::iterator it = variables->begin();

	for (; it != variables->end(); ) {
		if (it->deep > deep) 
			it = variables->erase(it);
		else {
			++it;
		}
	}
}

void Interprete::PreLoad_Identificador(Parser_Identificador * x, int* total_vars, std::vector<Variable> * variables, int deep, OutData_Parametros * node, bool add)
{
	Variable * v = BusquedaVariable(x->nombre, variables);

	int index = variables->size();

	if (add)
	{
		if (v == NULL)
		{
			variables->push_back(Variable(x->nombre, deep, index));
			(*total_vars)++;
		}
		else index = v->index;
	}
	else
	{
		if (v == NULL)
		{
			Errores::generarError(Errores::ERROR_VARIABLE_NO_EXISTE, node, x->nombre);
			this->PreloadError = true;
			variables->push_back(Variable(x->nombre, deep, index));
			(*total_vars)++;

		}
		else index = v->index;
	}

	x->index = index;

}

void Interprete::PreLoad()
{
	// TODO: Por aquí implementaríamos las variables globales, o externas al ámbito de la función.
	this->PreloadError = false;
	for (std::vector<Parser_Funcion*>::iterator it = funciones.begin(); it != funciones.end(); ++it)
	{
		PreLoad_Funcion(*it);
	}
}


void Interprete::PreLoad_Funcion(Parser_Funcion * funcion)
{
	std::vector<Variable> variables;
	int total_vars = 0;
	int deep = 0;

	//Valores de entrada de la funcion :: DEEP 0.
	for (std::vector<Funcion_ValorEntrada*>::iterator it = funcion->entradas.begin(); it != funcion->entradas.end(); ++it)
	{
		if ((*it)->tipo == ENTRADA_PARAM)
		{
			Parser_Parametro * x = static_cast<Parser_Parametro*>(*it);
			if (x->tipo == PRM_DECLARATIVO_ID)
			{
				Parametro_Declarativo_ID * pDId = static_cast<Parametro_Declarativo_ID*>(x);
				PreLoad_Identificador(pDId->pID, &total_vars, &variables, deep, &x->parametros, true);
				total_vars++;
			}
			else if (x->tipo == PRM_ID)
			{
				Parser_Identificador * pID = static_cast<Parser_Identificador*>(x);
				PreLoad_Identificador(pID, &total_vars,&variables, deep, &x->parametros, true);
				total_vars++;
			}
		}
	}

	//Sentencia de la funcion
	this->PreLoad_Sentencia(funcion->body, &total_vars, &variables, deep);
	funcion->preload_var = total_vars;


}

void Interprete::PreLoad_Valor(Parser_Valor * pV, int* total_vars, std::vector<Variable> * variables,  int deep, OutData_Parametros * data)
{
	if (pV->tipo == VAL_ID)
	{
		Parser_Identificador * x = static_cast<Parser_Identificador*>(pV);
		PreLoad_Identificador(x, total_vars, variables, deep, data);
	}

}

void Interprete::PreLoad_Param(Parser_Parametro * x, int* total_vars, std::vector<Variable> * variables, int deep)
{	
	switch (x->tipo)
	{
		case PRM_DECLARATIVO_ID:
		{
			Parametro_Declarativo_ID * x2 = static_cast<Parametro_Declarativo_ID*>(x);
			PreLoad_Identificador(x2->pID, total_vars, variables, deep, &x2->parametros, true);
			break;
		}
		case PRM_ID:
		{

			Parser_Identificador * x2 = static_cast<Parser_Identificador*>(x);
			PreLoad_Identificador(x2, total_vars, variables, deep, &x2->parametros, true);
			break;
		}
	}
}


/**
* Parametros que realizan la precarga de variables e información antes de comenzar con la interpretación.
*
**/
void Interprete::PreLoad_Sentencia(Parser_Sentencia * sentencia, int* total_vars, std::vector<Variable> * variables, int deep)
{
	if (sentencia == NULL)
		return;

	switch (sentencia->tipo)
	{
		case SENT_REC:
		{
			Sentencia_Recursiva * x = static_cast<Sentencia_Recursiva*>(sentencia);

			deep++;
			for (std::vector<Parser_Sentencia*>::iterator it = x->valor.begin(); it != x->valor.end(); ++it)
			{
				this->PreLoad_Sentencia((*it), total_vars, variables, deep);
			}
			deep--;
			RemoverVariablesScope(deep, variables);
			break;
		}
		case SENT_IF:
		{
			Sentencia_IF * x = static_cast<Sentencia_IF*>(sentencia);
			deep++;
			PreLoad_Operacion(x->pCond, total_vars, variables, deep);
			deep++;
			PreLoad_Sentencia(x->pS, total_vars, variables, deep);
			deep--;
			RemoverVariablesScope(deep, variables);
			deep++;
			PreLoad_Sentencia(x->pElse, total_vars, variables, deep);
			deep -= 2;
			RemoverVariablesScope(deep, variables);	
			break;
		}
		case SENT_WHILE:
		{
			Sentencia_WHILE * x = static_cast<Sentencia_WHILE*>(sentencia);
			deep++;
			PreLoad_Operacion(x->pCond, total_vars, variables, deep);
			deep++;
			PreLoad_Sentencia(x->pS, total_vars, variables, deep);
			deep -= 2;
			RemoverVariablesScope(deep, variables);
			break;
		}
		case SENT_FOR:
		{
			Sentencia_FOR * x = static_cast<Sentencia_FOR*>(sentencia);
			deep++;
			PreLoad_Operacion(x->pIguald, total_vars, variables, deep);
			PreLoad_Operacion(x->pCond, total_vars, variables, deep);
			PreLoad_Operacion(x->pOp, total_vars, variables, deep);
			deep++;
			PreLoad_Sentencia(x->pS, total_vars, variables, deep);
			deep -= 2;
			RemoverVariablesScope(deep, variables);
			break;
		}
		case SENT_RETURN:
		{
			Sentencia_Return * x = static_cast<Sentencia_Return*>(sentencia);
			PreLoad_Operacion(x->pOp, total_vars, variables, deep);
			break;
		}
		case SENT_PRINT:
		{
			Sentencia_Print * x = static_cast<Sentencia_Print*>(sentencia);
			PreLoad_Operacion(x->pOp, total_vars, variables, deep);
			break;
		}
		case SENT_OP:
		{
			Sentencia_Operacional * x = static_cast<Sentencia_Operacional*>(sentencia);
			PreLoad_Operacion(x->pOp, total_vars, variables, deep);
			break;
		}

	}
}

void Interprete::PreLoad_Operacion(Parser_Operacion * op, int* total_vars, std::vector<Variable> * variables, int deep)
{
	if (op == NULL)
		return;

	switch (op->tipo)
	{
		case OP_ID:
		{
			Operacion_ID * x = static_cast<Operacion_ID*>(op);
			PreLoad_Identificador(x->ID, total_vars, variables, deep, &op->parametros);
			break;
		}
		case OP_MATH:
		{
			Operacion_Operador * x = static_cast<Operacion_Operador*>(op);
			PreLoad_Valor(x->op1, total_vars, variables, deep, &x->parametros);

			if(x->op2)
				PreLoad_Operacion(x->op2->op, total_vars, variables, deep);

			break;
		}
		case OP_REC_OP:
		{
			Operacion_Recursiva * x = static_cast<Operacion_Recursiva*>(op);
			PreLoad_Operacion(x->op1, total_vars, variables, deep);
			if (x->op2 != NULL)
			PreLoad_Operacion(x->op2->op, total_vars, variables, deep);

			break;
		}
		case OP_IGUALDAD:
		{
			Operacion_Igualdad * x = static_cast<Operacion_Igualdad*>(op);
			PreLoad_Operacion(x->op, total_vars, variables, deep);
			PreLoad_Param(x->param, total_vars, variables, deep);

			break;
		}
	}
}