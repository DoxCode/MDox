
#include "Funciones.h"

/* Transforma de cadena a double, pasandole el formato de coma por locale.*/
double s2d(const std::string str, const std::locale &loc) {
	double val;
	std::stringstream ss(str);
	ss.imbue(loc);
	ss >> val;
	return val;
}

void ReplaceAll(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
}

/*Divide un string en partes por un delimitador de tipo caracter */
std::deque<std::string> split(const std::string& s, char delimiter)
{
	std::deque<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter))
	{
		tokens.push_back(token);
	}
	return tokens;
}

/*Divide un string en partes por un delimitador de tipo string */
std::deque<std::string> split_Str(std::string s, std::string delimiter)
{
	size_t pos = 0;
	std::deque<std::string> tokens;
	while ((pos = s.find(delimiter)) != std::string::npos) {
		tokens.push_back(s.substr(0, pos));	
		s.erase(0, pos + delimiter.length());
	}
	return tokens;
}

/*Divide un string en partes por una serie de delimitadores de tipo string*/
std::deque<std::string> split_StrMult(std::string s, std::deque<std::string> delimiters)
{
	size_t pos = 0;
	bool b = true;
	std::deque<std::string> tokens;

	while (true) {

		int temp;
		int min_pos = s.find(delimiters.front());
		int delimiter_length = delimiters.front().length();
		delimiters.pop_front();

		for (std::string delimiter : delimiters)
		{
			if (min_pos < (temp = s.find(delimiters.front())))
			{
				min_pos = temp;
				delimiter_length=delimiters.front().length();
			}
			delimiters.pop_front();
		}

		tokens.push_back(s.substr(0, pos));
		s.erase(0, pos + delimiter_length);

		if (s.length() == 0)
			break;
	}
	return tokens;
}

// Retira espacios y tabuladores antes de informacion útil
void RetirarValoresNoUtiles(std::string& str) {
	
	int num = 0;
	for (char& c : str)
	{
		if (c != ' ' && c != '\t')
		{
			break;
		}
		num++;
	}

	str = str.erase(0, num);
}


std::string getFileNameAndExt(std::string s)
{
	const size_t last_slash_idx = s.find_last_of("\\/");
	if (std::string::npos != last_slash_idx)
	{
		s.erase(0, last_slash_idx + 1);
	}

	return s;
}

//Transformar una ruta relativa a una Absoluta.
//Esto lógicamente dependerá del sistema operativo, por lo que el código será distinto dependiendo
//sobre que SO se está realizando la compilación.
std::string getAbsolutePathFromRelative(std::string path)
{
	int len = path.length();

	#ifdef _WIN32
		char full[_MAX_PATH];
		if (_fullpath(full, path.c_str(), _MAX_PATH) != NULL)
		{
			return std::string(full);
		}
		else
		{
			return path;
		}

	#elif linux	
		char * full = realpath(path.c_str(), NULL);

		if (full != NULL)
		{
			char f2[_MAX_PATH];
			strcpy(f2, full);
			std::string ret(f2);
			delete full;
			return ret;
		}
		else
		{
			return path;
		}
	#else
		return path; // Desconocida forma
	#endif
}





// Ejemplos:
//  double d = string_to<double>("1223.23");
//  int i = string_to<int>("1223");
//  bool b = string_to<bool>("1");
template<typename DataType>
DataType string_to(const std::string& s)
{
	DataType d;
	std::istringstream(s) >> d; // convierte de string al tipo de dato.
	return d;
}


bool nextStringLine(std::string& linea, std::string busqueda)
{
	if (linea.substr(0, busqueda.size()) == busqueda)
		return true;
	return false;
}

bool is_number(const std::string& s)
{
	std::string::const_iterator it = s.begin();
	while (it != s.end() && isdigit(*it)) ++it;
	return !s.empty() && it == s.end();
}

bool is_Identificador(const std::string & s)
{
	int n_digit = 0;
	int n_alph = 0;

	for (unsigned itr = 0; itr < s.size(); itr++)
	{
		if (isdigit(s.at(itr)))
		{
			n_digit++;
			continue;
		}
		else if (isalpha(s.at(itr)))
		{
			n_alph++;
			continue;
		}
		else if (s.at(itr) == '_')
		{
			continue;
		}
		else return false;
	}

	if (n_alph > 0)
		return true;


	return false;
}

//Para evitar el overflow en enteros por sumas
bool addOvf(int* result, int a, int b)
{
	if (a > 0 && b > 0 && *result < 0)
		return true;
	if (a < 0 && b < 0 && *result > 0)
		return true;
	return false;
}
//Para evitar el overflow en enteros por restas (a-b)
bool minOvf(int* result, int a, int b)
{	
	return addOvf(result, a, -b);
}

//Para evitar el overflow en enteros por sumas
bool multOvf(int* result, int a, int b)
{
	if (a > 0 && b > 0 && *result < 0)
		return true;
	if (a < 0 && b < 0 && *result < 0)
		return true;
	if (a < 0 && b > 0 && *result > 0)
		return true;
	if (a > 0 && b < 0 && *result > 0)
		return true;
	return false;
}