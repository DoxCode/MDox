
#include "Tipos.h";

tipos_parametros DeclarativoToParamType(Parser_Declarativo * dec)
{
	switch (dec->tipo)
	{
		case DEC_SINGLE:
		{
			Declarativo_SingleValue * x2 = static_cast<Declarativo_SingleValue*>(dec->value);
			return x2->value;	
		}
		case DEC_MULTI:
		{
			Declarativo_MultiValue * x2 = static_cast<Declarativo_MultiValue*>(dec->value);
			return x2->tipo;

		}
	}
	return PARAM_VOID;
}