/***************************
* La clase Tokenizer sería lo que en muchos lenguajes de programación se llamaría LEXER.
*
* Es el encargador de tomar el código en raw y generar una serie de tokens, que serán los que se analizarán con el PARSER.
*
*
*/


#include "Tokenizer.h"


void Tokenizer::generarTokens(std::string str)
{	
	std::string val = "";


	for (unsigned itr = 0; itr < str.size(); itr++) 
	{
		const char c = str.at(itr);

		const char* p = strchr(" \t\n", c);

		if (p)
		{
			if (val.size() > 0)
			{
				tokens.push_back(val);
				val = "";
			}
			continue;
		}

		if (str_compare(str, itr, "true"))
		{
			if (val.size() > 0)
			{
				tokens.push_back(val);
				val = "";
			}

			tokens.push_back("true");
			continue;
		}

		if (str_compare(str, itr, "false"))
		{
			if (val.size() > 0)
			{
				tokens.push_back(val);
				val = "";
			}

			tokens.push_back("false");
			continue;
		}


		const char c2 = str.at(itr + 1);

		if (
			(c == '+' && c2 == '+') ||
			(c == '-' && c2 == '-') ||
			(c == ':' && c2 == ':') ||
			(c == '+' && c2 == '=') ||
			(c == '-' && c2 == '=') ||
			(c == '=' && c2 == '=') ||
			(c == '!' && c2 == '=') ||
			(c == '<' && c2 == '=') ||
			(c == '>' && c2 == '=') ||
			(c == '&' && c2 == '&') ||
			(c == '|' && c2 == '|')
			) 
		{
			if (val.size() > 0)
			{
				tokens.push_back(val);
				val = "";
			}

			std::string chr(1, c);
			chr += c2;
			tokens.push_back(chr);
			continue;

		}

		p = strchr("()[]{}:,*+-^'\"<>;!%/", c);

		if (p)
		{
			if (val.size() > 0)
			{
				tokens.push_back(val);
				val = "";
			}

			std::string chr(1, c);
			tokens.push_back(chr);
			continue;
		}

	}

}


bool str_compare(std::string str, int itr,  std::string busq)
{
	if (str.size() - itr < busq.size())
	{
		return false;
	}

	std::string temp = "";

	for (unsigned i = 0; i < str.size(); i++)
	{
		if (i >= busq.size())
			break;

		temp += str.at(itr + i);
	}

	return temp == busq;
}


bool Tokenizer::GenerarTokenizerDesdeFichero(std::string ruta)
{
	std::ifstream ifs(ruta);
	std::string temp_line;

	std::string raw_string = "";
	std::vector<int> iLines;

	if (ifs.fail()) {
		return false;
	}

	bool comentario_bloque = false;

	while (std::getline(ifs, temp_line))
	{
		while (true)
		{
			if (comentario_bloque)
			{
				int endl = temp_line.find("*/");

				if (endl == -1)
				{
					break;
				}
				else
				{
					temp_line = temp_line.erase(0, endl + 2);
					comentario_bloque = false;
					continue;
				}

			}

			int offset = temp_line.find("//");
			int offset2 = temp_line.find("/*");

			if (offset == -1 && offset2 == -1) //No hay comentarios.
			{
				if (temp_line.size() > 0)
				{
					int n = std::count(temp_line.begin(), temp_line.end(), ' ');
					n += std::count(temp_line.begin(), temp_line.end(), '\t');
					n += std::count(temp_line.begin(), temp_line.end(), '\n');
					n = temp_line.length() - n;

					iLines.push_back(n);
					raw_string += temp_line;
				}
				break;
			}

			if (((offset2 == -1) && (-1 < offset)) || ((-1 < offset2) && (-1 < offset) && (offset < offset2))) // Existe un comentario de lina '//'
			{
				temp_line = temp_line.substr(0, offset);

				if (temp_line.size() > 0)
				{
					int n = std::count(temp_line.begin(), temp_line.end(), ' ');
					n += std::count(temp_line.begin(), temp_line.end(), '\t');
					n += std::count(temp_line.begin(), temp_line.end(), '\n');
					n = temp_line.length() - n;

					iLines.push_back(n);
					raw_string += temp_line;
				}
				break;
			}

			if (((-1 == offset) && (-1 < offset2)) || ((-1 < offset2) && (-1 < offset) && (offset2 < offset))) // Existe un comentario de bloque '/*'
			{
				int endl = temp_line.find("*/");

				if (endl == -1)
				{
					temp_line = temp_line.substr(0, offset2);

					if (temp_line.size() > 0)
					{
						int n = std::count(temp_line.begin(), temp_line.end(), ' ');
						n += std::count(temp_line.begin(), temp_line.end(), '\t');
						n += std::count(temp_line.begin(), temp_line.end(), '\n');
						n = temp_line.length() - n;

						iLines.push_back(n);
						raw_string += temp_line;
					}
					comentario_bloque = true;
				}
				else
				{
					//	std::cout << "\n -- offset : offset2 : endl " << offset << " : " << offset2 << " : " << endl << "--- \n";
					std::string head = temp_line.substr(0, offset2);
					std::string tail = temp_line.erase(0, endl + 2);


					temp_line = head + tail;

					int n = std::count(temp_line.begin(), temp_line.end(), ' ');
					n += std::count(temp_line.begin(), temp_line.end(), '\t');
					n += std::count(temp_line.begin(), temp_line.end(), '\n');
					n = temp_line.length() - n;

					iLines.push_back(n);
					raw_string += temp_line;
					continue;
				}
				break;
			}
			break;
		}
	}

	num_Lines = iLines;
//	generarTokens(raw_string);

	return true;
}