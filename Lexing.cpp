
#include "Lexing.h"

const Objeto_estructura_abstract * estructura_interna[] = {
	
	new Objeto_estructura_estatico("IF"),
	
 };


// COMANDO_EJEC <- <FUNCION>, <ASIGNACION>, <VARIABLE>


// FUNCION -> <ID>(<PARAMETRO>, <PARAMETRO>, ...) <COMANDO_EJEC>

// ASIGNACION -> <VARIABLE:X> = <UNION:X> ;
// ASIGNACION -> <ID> = <UNION:X> ;
/* Asignacion es VARIABLE si VARIABLE:X, no existe. */
// VARIABLE -> <ENUM> <ID> [= <UNION:X>] ;

// UNION:X  <- <ELEMS:X> {SIGNOS PERMITIDOS POR X}

// ELEM:X <- <FUNCION:X>, <VARIABLE:X>, <VALOR:X>

const std::string Fns[] =
{
	"IF ( <CONDICION> ) <COMANDO_EJEC> [ELSE <COMANDO_EJEC>]",
	"SI <CONDICION> ENTONCES <COMANDOS_EJEC> [SINO <COMANDOS_EJEC>] FIN-SI",
	"IF PRUEBA TEST"

};


const std::string op_si = "SI <CONDICION> ENTONCES <COMANDOS_EJEC> [SINO <COMANDOS_EJEC>] FIN-SI";
const std::string op_para = "PARA <VARIABLE:INT> DESDE <PARAMETRO:INT> HASTA <PARAMETRO:INT> [INCREMENTO <PARAMETRO:INT>] <COMANDOS_EJEC> FIN-PARA";


const std::string op_if = "IF ( <CONDICION> ) <COMANDO_EJEC> [ELSE <COMANDO_EJEC>]";
const std::string op_if2 = "<CONDICION>: <COMANDOS_EJEC> [| <COMANDOS_EJEC>] .";





void Lexing(std::string temporal_cache, std::string getWr)
{

	if (getWr.length() == 0)
	{
		funcionT(temporal_cache, getWr);
		return;
	}

	    char c = getWr.at(0);

		switch (c)
		{
		case ' ':
			getWr.erase(0, 1);

			if (funcionT(temporal_cache, getWr))
				return;

			break;
		case '(':
			if (funcionT(temporal_cache, getWr))
				/*ERROR*/return;

			break;

		default:
			temporal_cache += c;
			getWr.erase(0, 1);
		}

		Lexing(temporal_cache, getWr);
}


bool funcionT(std::string& ID, std::string getWr)
{
	if (ID == "")
		return false;

//	ReplaceAll(getWr, " ", "");

	for (std::string ssf : Fns)
	{
		std::deque<std::string> vcd = split(ssf, ' ');

		if (vcd.front() == ID)
		{
			vcd.pop_front();

			// Si la accion resulta ser correcta, no necesita buscar mas estructuras.
			// De no serlo, continuará buscando una correspondencia.
			if (AccionGrt(getWr, vcd))
				break;
		}
	}

	std::cout << "DEVOLVIENDO: '" << ID << "' \n";
	ID = "";
	return true;
}


bool AccionGrt(std::string getWr, std::deque<std::string> funcion)
{
	if (funcion.empty())
		return true;

	std::string p = funcion.front();

	if (p.empty())
		return false;


	std::deque<std::string> delimiters;
	delimiters.push_back("||");
	delimiters.push_back("&&");

	std::deque<std::string> sl = split_StrMult(getWr, delimiters);
	/*
	for (std::string ss : sl)
	{
		std::cout << ss << "\n";
	}
	*/

	return false;
   
	if (p.at(0) == '<') // VALOR ESPECIAL ADICIONAL OBLIGATORIO
	{
		funcion.pop_front();
		std::string p2 = funcion.front();



		if (p == "<CONDICION>")
		{
			boolVal ret = Busqueda_Elemento_Medio(p2, getWr);
			std::string valor = ModificarValoresBusqueda(getWr, ret, p2);

			std::cout << "VALOR DE: " << p << " P2 " << p2 << " ES '" << valor << "' QUEDA: " << getWr << " \n";

			if (valor.empty()) return false;


		}
	}
	else if (p.at(0) == '[') // VALOR ESPECIAL OPCIONAL
	{

	}
	else //CUERPO
	{
		if (getWr.length() >= p.length())
		{	
			if (getWr.substr(0, p.length()) == p)
			{
				std::cout << "CUERPO: " << p << " COINCIDE \n";

				getWr.erase(0, p.length());
				funcion.pop_front();
				return AccionGrt(getWr, funcion);
			}
		}
	}

	return false;
}

std::string ModificarValoresBusqueda(std::string& getWr, boolVal ret, const std::string& p2)
{
	if (std::get<0>(ret))
	{
		int offset = std::get<1>(ret);
		std::cout << "GET: true," << offset << " \n";
		std::string val = getWr.substr(0, offset);
		getWr.erase(0, offset + p2.length());
		return val;
	}
	return "";
}

bool esUnaCondicion(std::string condicion)
{
	std::deque<std::string> delimiters;
	delimiters.push_back("||");
	delimiters.push_back("&&");

	std::deque<std::string> sl = split_StrMult(condicion, delimiters);

	return true;
}


//Es la búsqueda de un elemento que está entre dos.
//Ejemplo: IF ( <CONDICION> ), <CONDICION> estará entre la posición 0 y la posición de ')'
boolVal Busqueda_Elemento_Medio(std::string& p, std::string getWr)
{
	if (getWr.length() >= p.length())
	{
		if (getWr.substr(0, p.length()) == p)
			return std::make_tuple(true, 0);

		getWr.erase(0, 1);
		std::tuple<bool, int> ret = Busqueda_Elemento_Medio(p, getWr);
		return std::make_tuple(std::get<0>(ret), std::get<1>(ret) + 1);
	}
	std::make_tuple(false, 0);
}