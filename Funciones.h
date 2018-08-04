#ifndef FUNCIONES_H
#define FUNCIONES_H

#include <string>
#include <vector>
#include <sstream>
#include <deque>

#define deletePtr(x) {delete x; x = NULL;}

void ReplaceAll(std::string& str, const std::string& from, const std::string& to);
std::deque<std::string> split(const std::string& s, char delimiter);
std::deque<std::string> split_Str(std::string s, std::string delimiter);
std::deque<std::string> split_StrMult(std::string s, std::deque<std::string> delimiters);
void RetirarValoresNoUtiles(std::string& str);
bool nextStringLine(std::string& linea, std::string busqueda);
#endif