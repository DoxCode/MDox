#ifndef FUNCIONES_H
#define FUNCIONES_H

#include <string>
#include <vector>
#include <sstream>
#include <deque>
#include <stdlib.h>


#define deletePtr(x) {if(x){ delete x; x = NULL;}}

double s2d(const std::string str, const std::locale &loc);
void ReplaceAll(std::string& str, const std::string& from, const std::string& to);
std::deque<std::string> split(const std::string& s, char delimiter);
std::deque<std::string> split_Str(std::string s, std::string delimiter);
std::deque<std::string> split_StrMult(std::string s, std::deque<std::string> delimiters);
void RetirarValoresNoUtiles(std::string& str);
bool nextStringLine(std::string& linea, std::string busqueda);
bool is_number(const std::string& s);
bool is_Identificador(const std::string & s);
std::string getAbsolutePathFromRelative(std::string path);
std::string getFileNameAndExt(std::string s);
bool addOvf(int*, int, int);
bool minOvf(int*, int, int);
bool multOvf(int*, int, int);



#endif