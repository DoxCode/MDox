#ifndef INTERPRETE_CORE_H
#define INTERPRETE_CORE_H

#include "Interprete.h"

// ----- FUNCIONES INTERPRETE
bool funcion_print(std::vector<Value>&);
bool funcion_now(std::vector<Value>&);
bool funcion_sleep(std::vector<Value>&);

// ----- FUNCIONES DE V ATOMICAS

// -------- STRING --------
bool funcion_string_find(Value, std::vector<Value>&);
bool funcion_string_substring(Value, std::vector<Value>&);
bool funcion_string_find(Value, std::vector<Value>&);
bool funcion_string_toUpper(Value, std::vector<Value>&);
bool funcion_string_toLower(Value, std::vector<Value>&);
bool funcion_string_Replace(Value, std::vector<Value>&);
bool funcion_string_split(Value, std::vector<Value>&);
bool funcion_string_lenght(Value, std::vector<Value>&);
bool funcion_string_match(Value, std::vector<Value>&);
bool funcion_string_startsWith(Value, std::vector<Value>&);

// -------- INT --------
bool funcion_int_lowestBit(Value, std::vector<Value>&);
bool funcion_int_toBinaryString(Value, std::vector<Value>&);
bool funcion_int_highestBit(Value, std::vector<Value>&);
bool funcion_int_bitCount(Value, std::vector<Value>&);
bool funcion_int_toOctalString(Value, std::vector<Value>&);
bool funcion_int_toHexString(Value, std::vector<Value>&);

// -------- LONG LONG --------
bool funcion_lint_lowestBit(Value, std::vector<Value>&);
bool funcion_lint_toBinaryString(Value, std::vector<Value>&);
bool funcion_lint_highestBit(Value, std::vector<Value>&);
bool funcion_lint_bitCount(Value, std::vector<Value>&);
bool funcion_lint_toOctalString(Value, std::vector<Value>&);
bool funcion_lint_toHexString(Value, std::vector<Value>&);


bool funcion_vector_lenght(Value, std::vector<Value>&);
bool funcion_vector_isEmpty(Value, std::vector<Value>&);
bool funcion_vector_contains(Value, std::vector<Value>&);
bool funcion_vector_indexOf(Value, std::vector<Value>&);
bool funcion_vector_replace(Value, std::vector<Value>&);
bool funcion_vector_remove(Value, std::vector<Value>&);
bool funcion_vector_pushBack(Value, std::vector<Value>&);
bool funcion_vector_pushFront(Value, std::vector<Value>&);
bool funcion_vector_popBack(Value, std::vector<Value>&);
bool funcion_vector_popFront(Value, std::vector<Value>&);
bool funcion_vector_insert(Value, std::vector<Value>&);
bool funcion_vector_containsAll(Value, std::vector<Value>&);



#endif
