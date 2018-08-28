
#include "Comandos.h"


// Mapeado de correspondencias entre valores escritos y comandos a los cuales
// hace referencia.
std::map<std::string, comandos_enum> RespCom = {

	{ ":h"         , AYUDA },
	{ ":?"         , AYUDA },
	{ ":l"         , CARGAR_FICHERO },
	{ ":load"      , CARGAR_FICHERO },
	{ "load"       , CARGAR_FICHERO },
	{ ":exit"      , SALIR },
	{ ":e"         , SALIR },
	{ "exit"       , SALIR },
};

bool Comandos(std::string comando, Interprete * interprete)
{
	comandos_enum e_com = RespCom[comando];

	//Ayuda especializada de comando.
	if (e_com == NULO)
	{
		auto dis = comandoAyuda(comando);
		if (std::get<0>(dis))
		{
			if (std::get<1>(dis) == NULO)
			{		
				std::cout << " El comando sobre el que intenta buscar informacion no existe. \n";
				return false;
			}
			else
			{
				EscribirAyudaComando(std::get<1>(dis));
				return false;
			}
		}
	}

	//Acciones de los comandos ejecutados:
	switch (e_com)
	{
		// Muestra un resumen con todos los comandos disponibles.
	case AYUDA:
		std::cout << "Lista de los comandos existentes actualmente para MDOX interpretado: \n";
		std::cout << "  :h, :? -> Muestra todos los comandos disponibles \n";
		std::cout << "  :h [X], :? [X] -> Muestra la ayuda avanzada del comando [X] \n";
		std::cout << "  :load [X], :l [X] -> Interpreta el archivo en la ruta [X] \n";
		std::cout << "  :exit, :e -> Finaliza el interprete MDOX \n";
		return false;
	case CARGAR_FICHERO:
		AccionesComandosIndividuales(e_com);
		return false;
	case SALIR:
		return true;
	}


	std::deque<std::string> comando_rec = split(comando, ' ');
	comandos_enum base = RespCom[comando_rec.at(0)];

	switch (base)
	{
		case CARGAR_FICHERO:
		{	
			if (comando_rec.size() < 2)
			{
				AccionesComandosIndividuales(base);
				return false;
			}

			std::string ruta = "";
			int puntos = 1;

			// Si el primer valor en una comilla doble, implica que puede haber una ruta con espacios.
			if (comando_rec.at(1).at(0) == '"')
			{
				// Por lo cual concatenamos todos los valores posteriores
				for (puntos = 1; puntos < comando_rec.size(); puntos++)
				{
					ruta = ruta + " " + comando_rec.at(puntos);

					//Hasta que  encuentra un nuevo '"' al final de un ultimo caracter de una de las cadenas.
					if (comando_rec.at(puntos).back() == '"')
						break;
				}

				//En caso de que el for anterior termine y no sea un '"', implica malformación de las cadenas.
				if (comando_rec.at(puntos).back() != '"')
				{
					AccionesComandosIndividuales(CARGAR_FICHERO_MALFORMACION);
					return false;
				}

				ruta = ruta.substr(2, ruta.size() - 3);

			}
			else
			{
				ruta = comando_rec.at(puntos);
			}

		//	std::cout << "Ruta: <" << ruta << ">";

			//CAMBIAR- EN caso de que se acepten más de dos valores en el comando de load.
			// En el caso de tener algo como:    :l "C:/Test/cd ds/as.mdox" 2
			// El dos final al ser un valor adicional no contemplado, excede el size, de puntos, que seria "ds/as.mdox"", ende, dará error.
			if (comando_rec.size() > (puntos+1))
			{
				AccionesComandosIndividuales(base);
				return false;
			}

			//Creamos la clase Parser, ella requerirá de la clase Tokenizerm que podrá ser llamada y creada desde si misma.
			Parser parser = Parser();

			//Desde el parser, accedemos al tokenizer, desde el mismo podremos generarlo a través del fichero.
			bool correcto = parser.tokenizer.GenerarTokenizerDesdeFichero(ruta);

			if (!correcto)
			{
				std::cout << "ERROR: " << "Ruta del fichero no encontrada." ;
				return false;
			}

			if (interprete->CargarDatos(&parser))
			{
				std::cout << "Sintaxis del fichero correcta, cargado en memoria. \n";
			}
			else
			{
				std::cout << "Se ha producido un error en la lectura del fichero. \n";
			}

			return false;
		}
	}


	std::cout << " Comando invalido. Escriba ':?' para obtener mas informacion sobre los comandos disponibles. \n";
	return false;
}


void AccionesComandosIndividuales(comandos_enum e_com)
{
	//Acciones de los comandos ejecutados:
	switch (e_com)
	{
		//Muestra un error por poner valores incorrectos en las entradas
	case CARGAR_FICHERO:
		std::cout << "  Detectado numero erroneo de parámetros. \n";
		EscribirAyudaComando(CARGAR_FICHERO);
		break;
	case CARGAR_FICHERO_MALFORMACION:
		std::cout << "  Malformacion a la hora de construir el comando, compruebe la ruta del fichero. \n";
		break;
	}
}

/* Muestra la ayuda adicional sobre un comando en concreto.
    A esta ayuda se accederá mediante :? [COMANDO]
*/
void EscribirAyudaComando(comandos_enum com)
{
	switch (com)
	{
		case CARGAR_FICHERO:
			std::cout << "AYUDA DEL COMANDO LOAD o CARGAR FICHERO. \n";
			break;
		case SALIR:
			std::cout << " Finaliza el interprete de MDOX. \n";
			break;
	}
}


std::tuple<bool, comandos_enum> comandoAyuda(std::string comando)
{
	if (comando.find(":h") == 0)
	{
		limpiarComando(":h", comando);
		comandos_enum e_com = RespCom[comando];
		return std::make_tuple(true, e_com);
	}

	if (comando.find(":?") == 0)
	{
		limpiarComando(":?", comando);
		comandos_enum e_com = RespCom[comando];
		return std::make_tuple(true, e_com);
	}

	return std::make_tuple(false, NULO);
}

void limpiarComando(std::string head, std::string& comando)
{
	ReplaceAll(comando, head, "");
	ReplaceAll(comando, " ", "");
}

