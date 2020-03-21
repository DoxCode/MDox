#ifndef INTERPRETE_H
#define INTERPRETE_H

//#include "../MDOX/Funciones.h"
//#include "Estructuras.h"
#include "Parser.h"
#include <array>
#include <iostream>
#include <math.h>  
#include <chrono>
#include <thread>
#include <regex>

class MDOX_Regex_Transform
{
public:
	 static constexpr const char REGEX_TRANSFORM_STRING[] = "(.*)";
	 static constexpr const char REGEX_TRANSFORM_DOUBLE[] = "\\+?(-?[0-9]+\\.?[0-9]+?)";
	 static constexpr const char REGEX_TRANSFORM_INT[] = "\\+?(-?[0-9]+)";
	 static constexpr const char REGEX_TRANSFORM_BOOL[] = "(true|false|0|1)";
};


class Variable_Runtime
{
public:
	Value value;
	tipos_parametros tipo; // Si no es void, no es fuerte.
	bool publica = true;

	Variable_Runtime() : tipo(PARAM_VOID) {}
	Variable_Runtime(Value a) : value(a), tipo(PARAM_VOID) {}
	Variable_Runtime(Value a, tipos_parametros b) : value(a), tipo(b) {}

	//Llamado al usar @@
	void copyIn(Variable_Runtime& vr)
	{
		vr.tipo = this->tipo;
		//vr.publica = publica;
		vr.value = value.copyIn();
	}
};


//Objeto producido por alguna clase
class mdox_object
{
public:

	//Clase de la cual proviene el objeto
	Parser_Class* clase;

	//Variable prefijadas, nos sirve para ganar en rendimiento, no obstante debido a que las clases son
	// prototipadas o dinámicas, únicamente se beneficiará de este cacheado las llamadas desde funciones internas de la clase.
	Variable_Runtime* variables_clase; 
	std::unordered_map<std::string, Variable_Runtime> _proto_variables; //Almacenamiento de las variables prototipo del objeto en particular.


	//Como buscamos las variables.
	//Únicamente usaremos este método, si NO disponemos de la ID directa de la variable.
	Variable_Runtime* findVariable(std::string& id)
	{
		//Primero buscamos la variable, entre las declaradas en la clase
		std::unordered_map<std::string, int>::const_iterator got = clase->_variables_map.find(id);

		if (got != clase->_variables_map.end())
			return &variables_clase[got->second];

		//Si no existe en la primera búsqueda lo busca en las variables dinámicamente agregadas
		std::unordered_map<std::string, Variable_Runtime>::iterator got2 = _proto_variables.find(id);

		if (got2 != _proto_variables.end())
			return &got2->second;

		return nullptr;
	}

	Variable_Runtime* AsignarVariable(std::string& id, Value& v)
	{
		Variable_Runtime* vr = findVariable(id);
		if (vr)
		{
			if (vr->publica)
			{
				vr->value.asignacion(v, vr->tipo);
				return vr;
			}
			else
			{
				Errores::generarError(Errores::ERROR_CLASE_VAR_PRIVATE, NULL, id);
				return nullptr;
			}
		}
		else
		{
			auto a = _proto_variables.emplace(id, Variable_Runtime(v, PARAM_VOID));
			if (a.second)
				return &a.first->second;
			return nullptr;
		}
	}

	//Llamado al usar el operador @
	std::shared_ptr<mdox_object> createCopy()
	{
		std::shared_ptr<mdox_object> of = std::make_shared<mdox_object>(this->clase);
		
		for (int itr = 0; itr < this->clase->preload_var; itr++)
		{
			of->variables_clase[itr] = this->variables_clase[itr];
		}

		return of;
	}

	// Llamado al usar el operador @@
	std::shared_ptr<mdox_object> createCopyIn()
	{
		std::shared_ptr<mdox_object> of = std::make_shared<mdox_object>(this->clase);

		for (int itr = 0; itr < this->clase->preload_var; itr++)
		{
			variables_clase[itr].copyIn(of->variables_clase[itr]);
		}

		return of;
	}

	mdox_object(Parser_Class* c) : clase(c) 
	{
		this->variables_clase = new Variable_Runtime[c->preload_var];
	};

	~mdox_object()
	{
		delete[] variables_clase;
	}
};


class Interprete {
private:
	bool return_activo = false;
	bool break_activo = false;
	bool continue_activo = false;
	bool ignore_activo = false;


	Value _retorno;
public:
	static Interprete* instance;

	bool varLoaded = false;

	Variable_Runtime* variablesMain = NULL;         //Variables del scope superior que llamaremos MAIN
	Variable_Runtime* variables_globales = NULL;    //Variable globales, aplicables en todo el entorno

	//Retorno continue
	bool ignoreCalled() { if (ignore_activo) { ignore_activo = false; return true; } return false; }
	//Retorno continue
	bool continueCalled() { if (continue_activo) { continue_activo = false; return true; } return false; }
	//Retorno break
	bool breakCalled() { if (break_activo) { break_activo = false; return true; } return false; }
	//Variable de retorno actual
	void setRetorno(Value& v) { _retorno = v; return_activo = true; }
	bool returnCalled() { if (return_activo) { return_activo = false; return true; } return false; }
	void retornoSinValor() { _retorno = std::monostate(); return_activo = true; }
	Value getRetorno() { return _retorno; }
	Value getRetornoAndEnd() { return_activo = false; return _retorno; }

	//void IniciateStaticClassValues(Parser_Class* pClase);
	ValueOrMulti getValueOrMulti(tipoValor& a, Variable_Runtime* variables, Variable_Runtime* var_clase);
	//bool OperacionOperadoresVectores(multi_value*, multi_value*, OPERADORES& op, Variable_Runtime* variables);
	short int OperacionOperadoresVectores(Value*, multi_value*, OPERADORES& operador, Variable_Runtime* variables, Variable_Runtime* var_clase, bool& isPop, bool& left);
	short int OperacionOperadoresVectores(multi_value*, Value*, OPERADORES& operador, Variable_Runtime* variables, Variable_Runtime* var_clase, bool& isPop, bool& left);
	short int OperacionOperadoresVectores(Value*, Value*, OPERADORES& operador, bool& isPop, bool& left);

	Value lectura_arbol_CallValue(arbol_operacional* node, Variable_Runtime* variables, Variable_Runtime* var_class);
	Value lectura_arbol_MultiValue_ref(arbol_operacional* node, Variable_Runtime* variables, Variable_Runtime* var_clase);
	Value lectura_arbol_operacional(arbol_operacional* node, Variable_Runtime* variables, Variable_Runtime* var_clase);
	//void setRetorno(Value * v) { delete _retorno; _retorno = v; }
	//Value * getRetorno() { Value * t = _retorno; _retorno = NULL; return t; }
	//Value * viewRetorno() { return _retorno; }
	//void nullRetorno() { if (_retorno != NULL) { delete _retorno;  _retorno = NULL; } }

	void String_to_MultiValue(std::string&, multi_value*, Variable_Runtime*, Variable_Runtime*);

	Value TratarMultiplesValores(multi_value* arr, Variable_Runtime* variables, Variable_Runtime* var_clase);
	
	bool Interprete::Interpretar(Parser* parser);
	//bool CargarDatos(Parser* parser);
	//void Interpretar(Parser* parser);
	bool Interprete_Sentencia(Parser_Sentencia* sentencia, Variable_Runtime* variables, Variable_Runtime* var_clase);

	//VariablePreloaded * Interprete_NuevaVariable(Parser_Parametro * par, VariablePreloaded * variables);
	
	void getRealValueFromValueWrapperRef(Value** v, tipos_parametros* tipo = nullptr);
	void getRealValueFromValueWrapper(Value& v, tipos_parametros* = nullptr);
	std::vector<Value> transformarEntradasCall(Call_Value* vF, Variable_Runtime* variables, Variable_Runtime* var_clase);
	bool Relacional_rec_arbol(arbol_operacional* node, Variable_Runtime* variables, Variable_Runtime* var_clase, Value& val_r);

	Value ExecOperador(Operator_Class* oc, Value& v, Variable_Runtime* var_class);
	Value ExecOperador(Operator_Class* oc, Variable_Runtime* var_class);
	Value ExecClass(Call_Value* vf, std::vector<Value>& entradas);
	Value ExecFuncion(Call_Value* vf, std::vector<Value>& entradas, Variable_Runtime* var_class, Parser_Class* pClass=NULL);
	bool FuncionCore(Call_Value* vf, std::vector<Value>& entradas);


	// =========================== Interprete interno ==============================
	void MDOX_StringFormat(std::string, arbol_operacional*, Variable_Runtime*, Variable_Runtime*);
	void InputSystem(Sentencia_Input*, Variable_Runtime*, Variable_Runtime*);

	Interprete()
	{
		Interprete::instance = this;
	}

	~Interprete()
	{

	};
};

class Core_Function_Interprete : public Core_Function
{
public:
	bool (*funcion_exec)(std::vector<Value>&);

	Core_Function_Interprete(std::string a, std::vector<tipos_parametros> b, tipos_parametros c) : Core_Function(a, b, c) {}

	~Core_Function_Interprete() {};
};




//Interprete * Interprete::instance;

#endif
