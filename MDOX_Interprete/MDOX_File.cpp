#include "Interprete_Core.h"


#define FILE_CLASS_NO_ABIERTO -1 // --
#define FILE_CLASS_LECTURA 0	 // r
#define FILE_CLASS_ESCRITURA 1   // w
#define FILE_CLASS_REMPLAZO 2    // x


int MDOX_File::ruta_fichero;
int MDOX_File::abierto_como;

void MDOX_File::MDOX_File_Start()
{
	MDOX_Class_File();

}

//Construye la clase core FILE al inicializar el programa.
void MDOX_File::MDOX_Class_File()
{
	Parser_Class* class_file = Clase_Core::crearClase("File");
	Clase_Core::addConstructor(class_file, &MDOX_Class_File_Constructor);

	ruta_fichero = Clase_Core::addVariable(class_file, "ruta_fichero", Value(std::monostate()));
	abierto_como = Clase_Core::addVariable(class_file, "abierto_como", Value(std::monostate()));


	Clase_Core::addFuncion(class_file, CoreClass_Function("readLine", &MDOX_Class_File_Function_getLine));
	Clase_Core::addFuncion(class_file, CoreClass_Function("readAll", &MDOX_Class_File_Function_getAll));
	Clase_Core::addFuncion(class_file, CoreClass_Function("readChar", &MDOX_Class_File_Function_getChar));
	Clase_Core::addFuncion(class_file, CoreClass_Function("isEOF", &MDOX_Class_File_Function_isEOF));
	Clase_Core::addFuncion(class_file, CoreClass_Function("close", &MDOX_Class_File_Function_close));
	Clase_Core::addFuncion(class_file, CoreClass_Function("isOpen", &MDOX_Class_File_Function_isOpen));
	Clase_Core::addFuncion(class_file, CoreClass_Function("open", &MDOX_Class_File_Function_open));
	Clase_Core::addFuncion(class_file, CoreClass_Function("getSize", &MDOX_Class_File_Function_getSize));
	Clase_Core::addFuncion(class_file, CoreClass_Function("write", &MDOX_Class_File_Function_write));
	Clase_Core::addFuncion(class_file, CoreClass_Function("setReadPosition", &MDOX_Class_File_Function_setReadPosition));
	Clase_Core::addFuncion(class_file, CoreClass_Function("getReadPosition", &MDOX_Class_File_Function_getReadPosition));
	Clase_Core::addFuncion(class_file, CoreClass_Function("setWritePosition", &MDOX_Class_File_Function_setWritePosition));
	Clase_Core::addFuncion(class_file, CoreClass_Function("getWritePosition", &MDOX_Class_File_Function_getWritePosition));
	Clase_Core::addFuncion(class_file, CoreClass_Function("createFile", &MDOX_Class_File_Function_Static_createFile, true));

	Clase_Core::endClassCore(class_file);
}

/*
CLASE File - FUNCION STATIC createFile.
Crea un fichero nuevo.
ENTRADA: <STRING>
SALIDA: <BOOL>
*/
bool  MDOX_File::MDOX_Class_File_Function_Static_createFile(std::shared_ptr<mdox_object> obj, std::vector<Value>& v)
{
	if (v.size() != 1)
	{
		Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "createFile", "createFile(<STRING>):<BOOL>");
		return false;
	}

	if (!v[0].Cast(PARAM_STRING))
	{
		Errores::generarError(Errores::ERROR_CLASE_CORE_CONSTRUCTOR, Errores::outData, "createFile", "createFile(<STRING>):<BOOL>");
		return false;
	}


	std::filesystem::path path = std::get<std::string>(v[0].value);
	if (!path.is_absolute())
	{
		path = Parser::mainPathProgram.parent_path() / std::get<std::string>(v[0].value);
	}

	std::fstream stream(path, std::ios::out);

	if (stream.is_open())
	{
		stream.close();
		Interprete::instance->setRetorno(Value(true));
	} else
		Interprete::instance->setRetorno(Value(false));

	return true;
}


/*
CLASE File - FUNCION setWritePosition.
Establece el indice de escritura si se introduce solo lint.
En el caso que se introduzca acompañado de POS, el primer valor será el offset el segundo: 0->inicio fichero 1-> posicion actual, 2-> fin fichero
ENTRADA: <LINT> <LINT, POS>
SALIDA: <>
*/
bool MDOX_File::MDOX_Class_File_Function_setWritePosition(std::shared_ptr<mdox_object> obj, std::vector<Value>& v)
{

	if (std::get<int>(obj->variables_clase[abierto_como].value.value) != FILE_CLASS_ESCRITURA)
	{
		Errores::generarError(Errores::ERROR_CLASE_CORE_FILE_NOT_WRITE, Errores::outData, "setWritePosition");
		return false;
	}

	if (v.size() == 1)
	{
		if (!v[0].Cast(PARAM_LINT))
		{
			Errores::generarError(Errores::ERROR_CLASE_CORE_CONSTRUCTOR, Errores::outData, "setWritePosition", "setWritePosition(<LINT>):<VOID>, setWritePosition(<LINT>(Offset), POS (0->beginning of the file, 1->current position, 2-> End of file)):<VOID>");
			return false;
		}

		std::fstream* stream = &std::get<Stream<std::fstream>>(obj->especial_value).stream;

		long long nVal = std::get<long long>(v[0].value);

		stream->seekp(0, std::fstream::end); //Establecemos puntero al final del fichero para buscar el size final

		if (nVal <= stream->tellp() && nVal > 0)
			stream->seekp(nVal, std::fstream::beg);
		else
		{
			Errores::generarError(Errores::ERROR_CLASE_CORE_FILE_OUT_INDEX, Errores::outData, nVal + "", stream->tellg() + "");
			return false;
		}
		return true;
	}

	if (v.size() == 2)
	{
		if (!v[0].Cast(PARAM_LINT))
		{
			Errores::generarError(Errores::ERROR_CLASE_CORE_CONSTRUCTOR, Errores::outData, "setReadPosition", "setReadPosition(<LINT>):<VOID>, setReadPosition(<LINT>(Offset), POS (0->beginning of the file, 1->current position, 2-> End of file)):<VOID>");
			return false;
		}

		if (!v[1].Cast(PARAM_INT))
		{
			Errores::generarError(Errores::ERROR_CLASE_CORE_CONSTRUCTOR, Errores::outData, "setReadPosition", "setReadPosition(<LINT>):<VOID>, setReadPosition(<LINT>(Offset), POS (0->beginning of the file, 1->current position, 2-> End of file)):<VOID>");
			return false;
		}

		std::fstream* stream = &std::get<Stream<std::fstream>>(obj->especial_value).stream;

		long long nVal = std::get<long long>(v[0].value);
		int posic = std::get<int>(v[1].value);

		if (posic == 0)
		{
			stream->seekp(nVal, std::fstream::beg);
			return true;
		}
		else if (posic == 1)
		{
			stream->seekp(nVal, std::fstream::cur);
			return true;
		}
		else
		{
			stream->seekp(nVal, std::fstream::end);
			return true;
		}

	}

	Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "setReadPosition", "setReadPosition(<LINT>):<VOID>, setReadPosition(<LINT>(Offset), POS (0->beginning of the file, 1->current position, 2-> End of file)):<VOID>");
	return false;
}


/*
CLASE File - FUNCION getWritePosition.
Obtiene el índece de escritura actual.
ENTRADA: <>
SALIDA: <LINT>
*/
bool  MDOX_File::MDOX_Class_File_Function_getWritePosition(std::shared_ptr<mdox_object> obj, std::vector<Value>& v)
{
	if (v.size() != 0)
	{
		Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "getWritePosition", "getWritePosition():<LINT>");
		return false;
	}

	if (std::get<int>(obj->variables_clase[abierto_como].value.value) != FILE_CLASS_ESCRITURA)
	{
		Errores::generarError(Errores::ERROR_CLASE_CORE_FILE_NOT_WRITE, Errores::outData, "getWritePosition");
		return false;
	}

	std::fstream* stream = &std::get<Stream<std::fstream>>(obj->especial_value).stream;

	Interprete::instance->setRetorno(Value(stream->tellp()));
	return true;
}

/*
CLASE File - FUNCION write.
Escribe en el indice marcado del fichero.
ENTRADA: <STRING>
SALIDA: <>
*/
bool MDOX_File::MDOX_Class_File_Function_write(std::shared_ptr<mdox_object> obj, std::vector<Value>& v)
{
	if (v.size() != 1)
	{
		Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "write", "write(<STRING>):<VOID>");
		return false;
	}

	if (!v[0].Cast(PARAM_STRING))
	{
		Errores::generarError(Errores::ERROR_CLASE_CORE_CONSTRUCTOR, Errores::outData, "write", "write(<STRING>):<VOID>");
		return false;
	}

	if (std::get<int>(obj->variables_clase[abierto_como].value.value) != FILE_CLASS_ESCRITURA)
	{
		Errores::generarError(Errores::ERROR_CLASE_CORE_FILE_NOT_WRITE, Errores::outData, "write");
		return false;
	}

	std::fstream* stream = &std::get<Stream<std::fstream>>(obj->especial_value).stream;

	*stream << std::get<std::string>(v[0].value);

	if (stream->fail())
	{
		stream->flush();
		return true;
	}	

	return true;
}



/*
CLASE File - FUNCION setReadPosition.
Establece el indice de lectura si se introduce solo lint.
En el caso que se introduzca acompañado de POS, el primer valor será el offset el segundo: 0->inicio fichero 1-> posicion actual, 2-> fin fichero
ENTRADA: <LINT> <LINT, POS>
SALIDA: <>
*/
bool MDOX_File::MDOX_Class_File_Function_setReadPosition(std::shared_ptr<mdox_object> obj, std::vector<Value>& v)
{

	if (std::get<int>(obj->variables_clase[abierto_como].value.value) != FILE_CLASS_LECTURA)
	{
		Errores::generarError(Errores::ERROR_CLASE_CORE_FILE_NOT_READ, Errores::outData, "setReadPosition");
		return false;
	}

	if (v.size() == 1)
	{
		if (!v[0].Cast(PARAM_LINT))
		{
			Errores::generarError(Errores::ERROR_CLASE_CORE_CONSTRUCTOR, Errores::outData, "setReadPosition", "setReadPosition(<LINT>):<VOID>, setReadPosition(<LINT>(Offset), POS (0->beginning of the file, 1->current position, 2-> End of file)):<VOID>");
			return false;
		}

		std::fstream* stream = &std::get<Stream<std::fstream>>(obj->especial_value).stream;

		long long nVal = std::get<long long>(v[0].value);

		stream->seekg(0, std::fstream::end); //Establecemos puntero al final del fichero para buscar el size final

		if (nVal <= stream->tellg() && nVal > 0)
			stream->seekg(nVal, std::fstream::beg);
		else
		{
			Errores::generarError(Errores::ERROR_CLASE_CORE_FILE_OUT_INDEX, Errores::outData, nVal+"", stream->tellg()+"");			
			return false;
		}
		return true;
	}

	if (v.size() == 2)
	{
		if (!v[0].Cast(PARAM_LINT))
		{
			Errores::generarError(Errores::ERROR_CLASE_CORE_CONSTRUCTOR, Errores::outData, "setReadPosition", "setReadPosition(<LINT>):<VOID>, setReadPosition(<LINT>(Offset), POS (0->beginning of the file, 1->current position, 2-> End of file)):<VOID>");
			return false;
		}

		if (!v[1].Cast(PARAM_INT))
		{
			Errores::generarError(Errores::ERROR_CLASE_CORE_CONSTRUCTOR, Errores::outData, "setReadPosition", "setReadPosition(<LINT>):<VOID>, setReadPosition(<LINT>(Offset), POS (0->beginning of the file, 1->current position, 2-> End of file)):<VOID>");
			return false;
		}

		std::fstream* stream = &std::get<Stream<std::fstream>>(obj->especial_value).stream;

		long long nVal = std::get<long long>(v[0].value);
		int posic = std::get<int>(v[1].value);

		if (posic == 0)
		{
			stream->seekg(nVal, std::fstream::beg);
			return true;
		}
		else if (posic == 1)
		{
			stream->seekg(nVal, std::fstream::cur);
			return true;
		}
		else 
		{
			stream->seekg(nVal, std::fstream::end);
			return true;
		}

	}

	Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "setReadPosition", "setReadPosition(<LINT>):<VOID>, setReadPosition(<LINT>(Offset), POS (0->beginning of the file, 1->current position, 2-> End of file)):<VOID>");
	return false;
}


/*
CLASE File - FUNCION getReadPosition.
Obtiene el índece de lectura actual.
ENTRADA: <>
SALIDA: <LINT>
*/
bool MDOX_File::MDOX_Class_File_Function_getReadPosition(std::shared_ptr<mdox_object> obj, std::vector<Value>& v)
{
	if (v.size() != 0)
	{
		Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "getReadPosition", "getReadPosition():<LINT>");
		return false;
	}

	if (std::get<int>(obj->variables_clase[abierto_como].value.value) != FILE_CLASS_LECTURA)
	{
		Errores::generarError(Errores::ERROR_CLASE_CORE_FILE_NOT_READ, Errores::outData, "getReadPosition");
		return false;
	}

	std::fstream* stream = &std::get<Stream<std::fstream>>(obj->especial_value).stream;

	Interprete::instance->setRetorno(Value(stream->tellg()));
	return true;
}


/*
	CLASE File - FUNCION getSize.
	Devuelve la longitud del fichero
	ENTRADA: <>
	SALIDA: <LINT>
*/
bool MDOX_File::MDOX_Class_File_Function_getSize(std::shared_ptr<mdox_object> obj, std::vector<Value>& v)
{
	if (v.size() != 0)
	{
		Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "getSize", "getSize():<LINT>");
		return false;
	}

	std::ifstream in(std::get<std::string>(obj->variables_clase[ruta_fichero].value.value), std::ifstream::ate | std::ifstream::binary);
	long long lng = in.tellg();
	in.close();

	Interprete::instance->setRetorno(Value(lng));
	return true;
}




#define FILE_CLASS_FUNCTION_OPEN "open(<STRING>, <STRING>: r=read, w=write, x=replace):<BOOL>"
/*
	CLASE File - FUNCION open.
	Abre un fichero determinado, dependiendo de la ruta introducida. Devuelve true si se pudo abrir correctamente.
	ENTRADA: <STRING>
	SALIDA: <BOOL>
*/
bool MDOX_File::MDOX_Class_File_Function_open(std::shared_ptr<mdox_object> obj, std::vector<Value>& v)
{

	if (v.size() == 2)
	{
		if (!v[0].Cast(PARAM_STRING))
		{
			Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "open", FILE_CLASS_FUNCTION_OPEN);
			return false;
		}

		if (!v[1].Cast(PARAM_STRING))
		{
			Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "open", FILE_CLASS_FUNCTION_OPEN);
			return false;
		}

		std::string tipo = std::get<std::string>(v[1].value);
		if(tipo.size() != 1)
		{
			Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "open", FILE_CLASS_FUNCTION_OPEN);
			return false;
		}

		int tipo_apertura = FILE_CLASS_NO_ABIERTO;

		switch (tipo[0])
		{
		case 'r': tipo_apertura = FILE_CLASS_LECTURA; break;
		case 'w': tipo_apertura = FILE_CLASS_ESCRITURA; break;
		case 'x': tipo_apertura = FILE_CLASS_REMPLAZO; break;
		default:
			Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "open", FILE_CLASS_FUNCTION_OPEN);
			return false;
		}

		std::fstream* stream = &std::get<Stream<std::fstream>>(obj->especial_value).stream;

		if (stream->is_open())
			stream->close();

		std::filesystem::path path = std::get<std::string>(v[0].value);
		if (!path.is_absolute())
		{
			path = Parser::mainPathProgram.parent_path() / std::get<std::string>(v[0].value);
		}

		if(tipo_apertura == FILE_CLASS_LECTURA)
			stream->open(path, std::ios::in);
		else if(tipo_apertura == FILE_CLASS_ESCRITURA)
			stream->open(path, std::ios::app);
		else if (tipo_apertura == FILE_CLASS_REMPLAZO)
			stream->open(path, std::ios::trunc);

		obj->variables_clase[abierto_como].value = tipo_apertura == FILE_CLASS_LECTURA ? FILE_CLASS_LECTURA : FILE_CLASS_ESCRITURA;

		Interprete::instance->setRetorno(Value(stream->is_open()));
		obj->variables_clase[ruta_fichero].value = path.string();
		return true;		
	}

	Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "open", FILE_CLASS_FUNCTION_OPEN);
	return false;

}

/*
CLASE File - FUNCION isOpen.
Comprueba si existe un fichero abierto en el objeto asignado.
ENTRADA: <>
SALIDA: <BOOL>
*/
bool MDOX_File::MDOX_Class_File_Function_isOpen(std::shared_ptr<mdox_object> obj, std::vector<Value>& v)
{
	if (v.size() != 0)
	{
		Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "isOpen", "isOpen():<BOOL>");
		return false;
	}

	Interprete::instance->setRetorno(Value(std::get<Stream<std::fstream>>(obj->especial_value).stream.is_open()));
	return true;
}

/*
CLASE File - FUNCION close.
Cierra el archivo asociado a la clase.
ENTRADA: <>
SALIDA: <BOOL>
*/
bool MDOX_File::MDOX_Class_File_Function_close(std::shared_ptr<mdox_object> obj, std::vector<Value>& v)
{
	if (v.size() != 0)
	{
		Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "close", "close():<VOID>");
		return false;
	}
	std::fstream* stream = &std::get<Stream<std::fstream>>(obj->especial_value).stream;

	if (stream->is_open())
	{
		stream->close();
	}

	return true;
}

/*
CLASE File - FUNCION readChar.
Obtiene el siguiente caracter del fichero.
ENTRADA: <>
SALIDA: <STRING>
*/

bool MDOX_File::MDOX_Class_File_Function_getChar(std::shared_ptr<mdox_object> obj, std::vector<Value>& v)
{
	if (v.size() != 0)
	{
		Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "readChar", "readChar():<STRING>");
		return false;
	}


	if (std::get<int>(obj->variables_clase[abierto_como].value.value) != FILE_CLASS_LECTURA)
	{
		Errores::generarError(Errores::ERROR_CLASE_CORE_FILE_NOT_READ, Errores::outData, "readChar");
		return false;
	}

	std::fstream* stream = &std::get<Stream<std::fstream>>(obj->especial_value).stream;

	char c;
	if (stream->get(c))
	{
		Interprete::instance->setRetorno(Value(std::string(1, c)));
		return true;
	}

	Interprete::instance->setRetorno(Value(""));
	return true;
}

/*
CLASE File - FUNCION isEOF.
Devuelve true si ha llegado al final de la lectura del fichero.
ENTRADA: <>
SALIDA: <BOOL>
*/
bool MDOX_File::MDOX_File::MDOX_Class_File_Function_isEOF(std::shared_ptr<mdox_object> obj, std::vector<Value>& v)
{
	if (v.size() != 0)
	{
		Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "isEOF", "isEOF():<BOOL>");
		return false;
	}
	std::fstream* stream = &std::get<Stream<std::fstream>>(obj->especial_value).stream;

	if (stream->eof())
	{
		Interprete::instance->setRetorno(Value(true));
		return true;
	}

	Interprete::instance->setRetorno(Value(false));
	return true;
}


/*
CLASE File - FUNCION readLine.
Lee y devuelve la siguiente linea del fichero.
ENTRADA: <>
SALIDA: <STRING>
*/
bool MDOX_File::MDOX_File::MDOX_Class_File_Function_getLine(std::shared_ptr<mdox_object> obj, std::vector<Value>& v)
{
	if (v.size() != 0)
	{
		Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "readLine", "readLine():<STRING>");
		return false;
	}

	std::fstream* stream = &std::get<Stream<std::fstream>>(obj->especial_value).stream;

	if (!stream->is_open())
	{
		Errores::generarError(Errores::ERROR_CLASE_CORE_FILE_NOT_OPENED, Errores::outData);
		return false;
	}

	if (stream->fail())
	{
		Errores::generarError(Errores::ERROR_CLASE_CORE_FILE_NOT_OPENED, Errores::outData);
		stream->clear();
		return false;
	}

	if (std::get<int>(obj->variables_clase[abierto_como].value.value) != FILE_CLASS_LECTURA)
	{
		Errores::generarError(Errores::ERROR_CLASE_CORE_FILE_NOT_READ, Errores::outData, "readLine");
		return false;
	}

	std::string ss;
	if (std::getline(*stream, ss))
	{
		Interprete::instance->setRetorno(Value(ss));
		return true;
	}

	Interprete::instance->setRetorno(Value(""));
	return true;
}

/*
CLASE File - FUNCION getAll.
Lee y devuelve el contenido completo del fichero leído
ENTRADA: <>
SALIDA: <STRING>
*/
bool MDOX_File::MDOX_File::MDOX_Class_File_Function_getAll(std::shared_ptr<mdox_object> obj, std::vector<Value>& v)
{
	if (v.size() != 0)
	{
		Errores::generarError(Errores::ERROR_CLASE_ATOMIC_FUNCTION_PARAMETER, Errores::outData, "readAll", "readAll():<STRING>");
		return false;
	}

	std::fstream* stream = &std::get<Stream<std::fstream>>(obj->especial_value).stream;

	if (!stream->is_open())
	{
		Errores::generarError(Errores::ERROR_CLASE_CORE_FILE_NOT_OPENED, Errores::outData);
		return false;
	}

	if (std::get<int>(obj->variables_clase[abierto_como].value.value) != FILE_CLASS_LECTURA)
	{
		Errores::generarError(Errores::ERROR_CLASE_CORE_FILE_NOT_READ, Errores::outData, "readAll");
		return false;
	}

	std::string total = "";
	std::string ss;
	while (std::getline(*stream, ss))
	{
		total += ss + "\n";
	}

	Interprete::instance->setRetorno(Value(total));
	return true;
}

#define FILE_CLASS_CONSTRUCTOR "File(), File(<STRING>, <STRING>: r=read, w=write, x=replace)"
//Constructor de la clase FILE
bool MDOX_File::MDOX_File::MDOX_Class_File_Constructor(std::shared_ptr<mdox_object> obj, std::vector<Value>& v)
{

	if (v.size() == 0)
	{
		obj->especial_value = Stream<std::fstream >();
		obj->variables_clase[ruta_fichero].publica = false;
		obj->variables_clase[abierto_como].publica = false;
		return true;
	}

	if (v.size() == 2)
	{
		if (!v[0].Cast(PARAM_STRING))
		{
			Errores::generarError(Errores::ERROR_CLASE_CORE_CONSTRUCTOR, Errores::outData, "File", FILE_CLASS_CONSTRUCTOR);
			return false;
		}

		if (!v[1].Cast(PARAM_STRING))
		{
			Errores::generarError(Errores::ERROR_CLASE_CORE_CONSTRUCTOR, Errores::outData, "File", FILE_CLASS_CONSTRUCTOR);
			return false;
		}

		obj->variables_clase[ruta_fichero].publica = false;
		obj->variables_clase[abierto_como].publica = false;

		std::string tipo = std::get<std::string>(v[1].value);
		if (tipo.size() != 1)
		{
			Errores::generarError(Errores::ERROR_CLASE_CORE_CONSTRUCTOR, Errores::outData, "File", FILE_CLASS_CONSTRUCTOR);
			return false;
		}

		int tipo_apertura = FILE_CLASS_NO_ABIERTO;

		switch (tipo[0])
		{
		case 'r': tipo_apertura = FILE_CLASS_LECTURA; break;
		case 'w': tipo_apertura = FILE_CLASS_ESCRITURA; break;
		case 'x': tipo_apertura = FILE_CLASS_REMPLAZO; break;
		default:
			Errores::generarError(Errores::ERROR_CLASE_CORE_CONSTRUCTOR, Errores::outData, "File", FILE_CLASS_CONSTRUCTOR);
			return false;
		}

		obj->especial_value = Stream<std::fstream >();
		std::fstream* stream = &std::get<Stream<std::fstream>>(obj->especial_value).stream;

		std::filesystem::path path = std::get<std::string>(v[0].value);
		if (!path.is_absolute())
		{
			path = Parser::mainPathProgram.parent_path() / std::get<std::string>(v[0].value);
		}
		//Parser::mainPathProgram.parent_path()

		if (tipo_apertura == FILE_CLASS_LECTURA)
			stream->open(path, std::ios::in);
		else if (tipo_apertura == FILE_CLASS_ESCRITURA)
			stream->open(path, std::ios_base::app);
		else if (tipo_apertura == FILE_CLASS_REMPLAZO)
			stream->open(path, std::ios::trunc | std::ios::out);


		obj->variables_clase[abierto_como].value = tipo_apertura == FILE_CLASS_LECTURA ? FILE_CLASS_LECTURA : FILE_CLASS_ESCRITURA;
		obj->variables_clase[ruta_fichero].value = path.string();
		return true;
	}

	Errores::generarError(Errores::ERROR_CLASE_CORE_CONSTRUCTOR, Errores::outData, "File", FILE_CLASS_CONSTRUCTOR);
	return false;
}
