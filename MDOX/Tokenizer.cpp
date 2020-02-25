/***************************
* La clase Tokenizer sería lo que en muchos lenguajes de programación se llamaría LEXER.
*
* Es el encargador de tomar el código en raw y generar una serie de tokens, que serán los que se analizarán con el PARSER.
*
*
*/
#include "Tokenizer.h"

void Tokenizer::generarTokens(std::vector<Linea*> str)
{	
	std::string val = "";

	bool _cadena = false;
	bool _comillaSimple = false;
	bool anteriorLinea = false;
	bool continue_line = false;

	for (std::vector<Linea*>::iterator it = str.begin(); it != str.end(); ++it)
	{
		anteriorLinea = true;
		bool ignore_next = false;


		for (unsigned itr = 0; itr < (*it)->val.size(); itr++)
		{
			const char c = (*it)->val.at(itr);

			if (_cadena)
			{
				if (c == '\\' && !ignore_next)
				{
					if ((*it)->val.size() == itr + 1)
					{
						continue_line = true;
						continue;
					}

					ignore_next = true;
					continue;
				}

				if (ignore_next)
				{
					switch (c)
					{
						case '\\': val += '\\'; break;
						case '"': val += '"'; break;
						case 'n': val += '\n'; break;
						case 't': val += '\t'; break;
						case 'r': val += '\r'; break;
						case '?': val += '\?'; break;
						case 'a': val += '\a'; break;
						case 'b': val += '\b'; break;
						case 'f': val += '\f'; break;
						case 'v': val += '\v'; break;
						case '0': val += '\0'; break;
						case '\'': val += "'"; break;
						default: val += c;
					}

					if ((*it)->val.size() == itr + 1)
					{
						continue_line = true;
						continue;
					}

					ignore_next = false;
					continue;
				}

				if ((c == '"' && !_comillaSimple) || (c == '\'' && _comillaSimple))
				{
					//if (val.size() > 0)
					//{
						tokens.push_back(new Token((*it)->linea,itr,val, anteriorLinea));
						val = "";
				//	}

					std::string chr(1, c);
					tokens.push_back(new Token((*it)->linea, itr, chr, anteriorLinea));
					_cadena = false;
				}
				else
				{
					//Llega al final de linea sin haber encontrado fin de string, lo sigue buscando en posteriores.
					if ((*it)->val.size() == itr + 1)
					{
						continue_line = true;
						val += c;
						
						continue;
					}

					val += c;

				}

				continue_line = false;
				continue;
			}


			const char* p = strchr(" \t\n", c);

			if (p)
			{
				if (val.size() > 0)
				{
					tokens.push_back(new Token((*it)->linea, itr, val, anteriorLinea));
					val = "";
				}
				continue;
			}

			//TOKENIZER:: operaciones con 3 caracteres
			if (itr + 2 < (*it)->val.size())
			{
				const char c2 = (*it)->val.at(itr + 1);
				const char c3 = (*it)->val.at(itr + 2);

				if (
					(c == '<' && c2 == ':' && c3 == ':') ||
					(c == ':' && c2 == ':' && c3 == '>')
					)
				{
		
					if (val.size() > 0)
					{
						tokens.push_back(new Token((*it)->linea, itr, val, anteriorLinea));
						val = "";
					}

					std::string chr(1, c);
					chr += c2;
					chr += c3;
					tokens.push_back(new Token((*it)->linea, itr, chr, anteriorLinea));
					itr += 2;
					continue;

				}
			}

			//TOKENIZER:: operaciones con 2 caracteres
			if (itr + (long long)1 < (*it)->val.size())
			{
				const char c2 = (*it)->val.at(itr+(long long)1);

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
					(c == '@' && c2 == '@') || 
					(c == '|' && c2 == '|')
					)
				{
					if (val.size() > 0)
					{
						tokens.push_back(new Token((*it)->linea, itr, val, anteriorLinea));
						val = "";
					}

					std::string chr(1, c);
					chr += c2;
					tokens.push_back(new Token((*it)->linea, itr, chr, anteriorLinea));
					itr++;
					continue;

				}
			}

			p = strchr("()[]{}:,*+-^\"'<>;=!%/.@", c);

			if (p)
			{
				if (val.size() > 0)
				{
					tokens.push_back(new Token((*it)->linea, itr, val, anteriorLinea));
					val = "";
				}

				std::string chr(1, c);
				tokens.push_back(new Token((*it)->linea, itr, chr, anteriorLinea));

				if (c == '"')
				{
					_cadena = true;
					_comillaSimple = false;
				}
				else if (c == '\'')
				{
					_cadena = true;
					_comillaSimple = true;
				}

				continue;
			}

			val += c;
		}

		if (continue_line)
		{
			val += '\n';
			continue;
		}

		if (val.size() > 0)
		{
			tokens.push_back(new Token((*it)->linea, (*it)->val.size(), val, anteriorLinea));
			val = "";
		}

	}

	for (std::vector<Linea*>::iterator it = str.begin(); it != str.end(); ++it)
	{
		delete (*it);
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
	std::fstream ifs(ruta);
	std::string temp_line;

	std::vector<Linea*> raw_string;

	if (ifs.fail()) {
		return false;
	}

	bool comentario_bloque = false;



	int line = 0;
	while (std::getline(ifs, temp_line))
	{
		line++;

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

					raw_string.push_back(new Linea(line, temp_line));
				}
				break;
			}

			if (((offset2 == -1) && (-1 < offset)) || ((-1 < offset2) && (-1 < offset) && (offset < offset2))) // Existe un comentario de lina '//'
			{
				temp_line = temp_line.substr(0, offset);

				if (temp_line.size() > 0)
				{
					raw_string.push_back(new Linea(line, temp_line));
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
						raw_string.push_back(new Linea(line, temp_line));
					}
					comentario_bloque = true;
				}
				else
				{
					//	std::cout << "\n -- offset : offset2 : endl " << offset << " : " << offset2 << " : " << endl << "--- \n";
					std::string head = temp_line.substr(0, offset2);
					std::string tail = temp_line.erase(0, endl + 2);

					temp_line = head + tail;

					raw_string.push_back(new Linea(line, temp_line));
					continue;
				}
				break;
			}
			break;
		}
	}

	fichero = new Fichero(ruta);
	generarTokens(raw_string);

	return true;
}