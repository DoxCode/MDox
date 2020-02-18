
#include "parser.h"
#include "../MDOX_Interprete/Interprete.h" //Requerido para acceder a los operadores de los objetos, es necesarios para procesar el arbol.
#include <algorithm>  


/**
	Usado como copia interna cuando se llama a un operador '@@'
*/
Value Value::copyIn()
{
	return std::visit(overloaded{
	[](std::shared_ptr<mdox_object> & a)->Value
	{
			return a->createCopyIn();
	},
	[](std::shared_ptr<mdox_vector> & a)->Value
	{
		return a->createCopyIn();
	},
	[&](auto&)->Value {return this->value; },

		}, value);
}

std::shared_ptr<mdox_vector> mdox_vector::createCopyIn()
{
	std::shared_ptr<mdox_vector> of = std::make_shared<mdox_vector>();
	//of->vector.reserve(vector.size());

	//	for (int itr = 0; itr < this->vector.size(); itr++)
	for (std::deque<Value>::iterator it = vector.begin(); it != vector.end(); ++it)
	{
		of->vector.emplace_back(it->copyIn());
	}

	return of;
}


template <typename T>
std::string Value::to_string_p(const T& a_value, const int n)
{
	std::ostringstream out;
	out.precision(n);
	out << std::fixed << a_value;

	//return out.str();
	//Borra ceros sobrantes en caso de un double: 5.43 sería = 5.4300000000 -> Lo transforma en 5.43
	if (std::is_same<T, double>::value)
	{
		std::string s2 = out.str().erase(out.str().find_last_not_of('0') + 1, std::string::npos);
		return s2.erase(s2.find_last_not_of('.') + 1, std::string::npos);
	}
	else return out.str();
}

Value Value::Suma(Value& v1, Value& v2)
{
	return std::visit(overloaded{


		//INTEGER .. XX
		[](const int& a, const int& b)->Value
		{
			int t = a + b;
			if (addOvf(&t, a, b))
				return (long long(a) + b);
			return t;
		},
		[](const int& a, const bool& b)->Value
		{
			int t = a + b;
			if (addOvf(&t, a, b))
				return (long long(a) + b);
			return (t);
		},
		[](const int& a, const double& b)->Value { return (a + b); },
		[](const int& a,  std::string & b)->Value { return (to_string_p(a) + b); },
		[](const int& a, const long long& b)->Value { return (a + b); }, // [](int x) {return x + 5; }

		//DOUBLE .. XX
		[](const double& a,const int& b)->Value { return (a + b); },
		[](const double& a,const bool& b)->Value { return (a + b); },
		[](const double& a, const double& b)->Value { return (a + b); },
		[](const double& a,  std::string & b)->Value { return (to_string_p(a) + b); },
		[](const double& a, const long long& b)->Value { return (a + b); },


			//LONGLONG .. XX
			[](const long long& a,const int& b)->Value { return (a + b); },
			[](const long long& a,const bool& b)->Value { return (a + b); },
			[](const long long& a, const double& b)->Value { return (a + b); },
			[](const long long& a,  std::string & b)->Value { return (to_string_p(a) + b); },
			[](const long long& a, const long long& b)->Value { return (a + b); },


			//STRING .. XX
				[]( std::string & a,const int& b)->Value { return (a + to_string_p(b)); },
				[]( std::string & a,const bool& b)->Value { return (a + to_string_p(b)); },
				[]( std::string & a,const double& b)->Value { return (a + to_string_p(b)); },
				[]( std::string & a, std::string & b)->Value { return (a + b); },
				[]( std::string & a,const long long& b)->Value { return (a + to_string_p(b)); },
				[](std::string & a,std::monostate)->Value { return a; },

			//BOOL .. XX
			[](const bool& a,const int& b)->Value
			{
				int t = a + b;
				if (addOvf(&t, a, b))
					return (a + long long(b));
				return (t);
			},
			[](const bool& a,const bool& b)->Value { return (a + b); },
			[](const bool& a, const double& b)->Value { return (a + b); },
			[](const bool& a, std::string & b)->Value { return (to_string_p(a) + b); },
			[](const bool& a, const long long& b)->Value { return (a + b); },


			[&](auto&, std::shared_ptr<mdox_vector> & b)->Value
			{		
				std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
//				r->vector.reserve(b->vector.size());

				for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
				{
					r->vector.emplace_back(Value::Suma(v1,*sss));
				}
				return std::move(r);
			},

			[&](std::shared_ptr<mdox_vector> & b, const bool&)->Value
			{
				std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
				//r->vector.reserve(b->vector.size());

				for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
				{
					r->vector.emplace_back(Value::Suma(*sss, v2));
				}
				return std::move(r);
			},
			[&](std::shared_ptr<mdox_vector> & b, const double&)->Value
			{
				std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
				//r->vector.reserve(b->vector.size());

				for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
				{
					r->vector.emplace_back(Value::Suma(*sss, v2));
				}
				return std::move(r);
			},
			[&](std::shared_ptr<mdox_vector> & b, std::string&)->Value
			{
				std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
				//r->vector.reserve(b->vector.size());

				for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
				{
					r->vector.emplace_back(Value::Suma(*sss, v2));
				}
				return std::move(r);
			},
			[&](std::shared_ptr<mdox_vector> & b, long long&)->Value
			{
				std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
				//r->vector.reserve(b->vector.size());

				for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
				{
					r->vector.emplace_back(Value::Suma(*sss, v2));
				}
				return std::move(r);
			},


			[&v2](std::shared_ptr<mdox_vector> & b, int&)->Value
			{
				std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
				//r->vector.reserve(b->vector.size());
				//r.resize(b.size());
				for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
				{
					r->vector.emplace_back(Value::Suma(*sss, v2));
				}
				return std::move(r);
			},

			[](std::shared_ptr<mdox_vector> & a, std::shared_ptr<mdox_vector> & b)->Value
			{
				if (a->vector.size() == b->vector.size() && !a->vector.empty())
				{
					std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
					//r->vector.reserve(a->vector.size());

					int itr = 0;
					for (std::deque<Value>::iterator sss = a->vector.begin(); sss != a->vector.end(); ++sss)
					{
						r->vector.emplace_back(Value::Suma(*sss, b->vector[itr]));
						itr++;
					}
					return std::move(r);
				}

				if (a->vector.empty() || b->vector.empty())
				{
					if (a->vector.empty())
						return b;
					else return a;
				}

				std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
				//r->vector.reserve(a->vector.size() > b->vector.size() ? a->vector.size() : b->vector.size());

				Value* a1 = &*a->vector.begin();
				Value* b1 = &*b->vector.begin();

				Value * a2 = &a->vector.back();
				Value * b2 = &b->vector.back();


				while (true)
				{
					if (a1 == a2)
					{
						r->vector.emplace_back(Value::Suma(*a1, *b1));

						if (b1 == b2)
							return r;

						b1++; 

						while (b1 != b2)
						{
							r->vector.emplace_back(*b1);
							b1++; 
						}

						r->vector.emplace_back(*b1);
						return r;
					}

					if (b1 == b2)
					{
						r->vector.emplace_back(Value::Suma(*a1, *b1));

						if (a1 == a2)
							return r;

						a1++; 

						while (a1 != a2)
						{
							r->vector.emplace_back(*a1);
							a1++; 
						}
						r->vector.emplace_back(*a1);
						return r;
					}

					r->vector.emplace_back(Value::Suma(*a1, *b1));
					a1++; b1++;
				}
			},
			[&v1](auto& a, std::shared_ptr<mdox_object>& b)->Value
			{

				if (b->clase->right_operators && b->clase->right_operators->OPERATOR_suma)
				{
					return Interprete::instance->ExecOperador(b->clase->right_operators->OPERATOR_suma, v1, b->variables_clase);
				}

				Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "+", b->clase->pID->nombre);
				return std::monostate();
			},

			[&v2](std::shared_ptr<mdox_object>& a, auto& b)->Value
			{
				if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_suma)
				{
					return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_suma, v2, a->variables_clase);
				}

				Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "+", a->clase->pID->nombre);
				return std::monostate();
			},

			[&](std::shared_ptr<mdox_object>& a, std::shared_ptr<mdox_object>& b)->Value
			{
				if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_suma)
				{
					return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_suma, v2, a->variables_clase);
				}

				if (b->clase->right_operators && b->clase->right_operators->OPERATOR_suma)
				{
					return Interprete::instance->ExecOperador(b->clase->right_operators->OPERATOR_suma, v1, b->variables_clase);
				}

				Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "+", a->clase->pID->nombre);
				return std::monostate();
			},

			[&v2](std::shared_ptr<mdox_object>& a, std::shared_ptr<mdox_vector>& b)->Value
			{
				if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_suma)
				{
					return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_suma, v2, a->variables_clase);
				}

				Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "+", a->clase->pID->nombre);
				return std::monostate();
			},

			//std::shared_ptr<mdox_object>

			[](auto,auto)->Value { return std::monostate(); },

		}, v1.value, v2.value);
}

Value Value::Resta(Value& v1, Value& v2)
{
	return std::visit(overloaded{

		//INTEGER .. XX
		[](const int& a,const int& b)->Value
		{
			int t = a - b;
			if (minOvf(&t, a, b))
				return (long long(a) - b);
			return (t);
		},
		[](const int& a, const bool& b)->Value
		{
			int t = a - b;
			if (minOvf(&t, a, b))
				return (long long(a) - b);
			return (t);
		},
		[](const int& a, const double& b)->Value { return (a - b); },
		[](const int& a, std::string & b)->Value
		{
			if (a < 0) return (b);
			if (a > b.length()) return (std::string(""));
			return (b.erase(0,a));
		},
		[](const int& a,const  long long& b)->Value { return (a - b); },

			//DOUBLE .. XX
			[](const double& a,const int& b)->Value { return (a - b); },
			[](const double& a,const bool& b)->Value { return (a - b); },
			[](const double& a, const double& b)->Value { return (a - b); },
			[](const double& a,  std::string & b)->Value { Errores::generarError(Errores::ERROR_MATH_MINUS_STRING, Errores::outData, "double"); return std::monostate(); },
			[](const double& a, const long long& b)->Value { return (a - b); },

			//LONGLONG .. XX
			[](const long long& a, const int& b)->Value { return (a - b); },
			[](const long long& a, const bool& b)->Value { return (a - b); },
			[](const long long& a, const double& b)->Value { return (a - b); },
			[](const long long& a, std::string & b)->Value
			{
				if (a < 0) return (b);
				if (a > b.length()) return (std::string(""));
				return (b.erase(0,a));
			},
			[](const long long& a, const long long& b)->Value { return (a - b); },

				//STRING .. XX
				[](std::string & a,const int& b)->Value
				{
					if (b < 0) return (a);
					if (b > a.length()) return "";
					return (a.erase(a.length() - (b), b));
				},
				[](std::string & a,const bool& b)->Value
				{
					if (b > a.length()) return "";
					if (b) return (a.erase(a.length() - 2, 1)); else return "";
				},
				[](std::string & a,const double& b)->Value { Errores::generarError(Errores::ERROR_MATH_MINUS_STRING, Errores::outData, "double"); return std::monostate(); },
				[](std::string & a, std::string & b)->Value //TODO: FALLA
				{

					if (a.length() < b.length())
						return a;

					int inx = a.length() - b.length();
					std::string tt = a.substr(inx, b.length());
					if (tt == b)
						return a.erase(inx, b.length());
					else return a;


				},
				[](std::string & a,const long long& b)->Value
				{
					if (b < 0) return (a);
					if (b > a.length()) return (std::string(""));
					return (a.erase(a.length() - (b + 1), b));
				},

					//BOOL .. XX
					[](const bool& a,const int& b)->Value { return (a - b); },
					[](const bool& a,const bool& b)->Value { return (a - b); },
					[](const bool& a, const double& b)->Value { return (a - b); },
					[](const bool& a, std::string & b)->Value
					{
						if (a < 0) return (b);
						if (a > b.length()) return (std::string(""));
						return (b.erase(0,a));
					},
					[](const bool& a, const long long& b)->Value { return (a - b); },


					[&](auto&, std::shared_ptr<mdox_vector> & b)->Value
					{
						std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
						//r->vector.reserve(b->vector.size());

						for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
						{
							r->vector.emplace_back(Value::Resta(v1, *sss));
						}
						return std::move(r);
					},

					[&](std::shared_ptr<mdox_vector> & b, const bool&)->Value
					{
						std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
						//r->vector.reserve(b->vector.size());

						for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
						{
							r->vector.emplace_back(Value::Resta(*sss, v2));
						}
						return std::move(r);
					},
					[&](std::shared_ptr<mdox_vector> & b, const double&)->Value
					{
						std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
						//r->vector.reserve(b->vector.size());

						for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
						{
							r->vector.emplace_back(Value::Resta(*sss, v2));
						}
						return std::move(r);
					},
					[&](std::shared_ptr<mdox_vector> & b, std::string&)->Value
					{
						std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
						//r->vector.reserve(b->vector.size());

						for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
						{
							r->vector.emplace_back(Value::Resta(*sss, v2));
						}
						return std::move(r);
					},
					[&](std::shared_ptr<mdox_vector> & b, long long&)->Value
					{
						std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
						//r->vector.reserve(b->vector.size());

						for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
						{
							r->vector.emplace_back(Value::Resta(*sss, v2));
						}
						return std::move(r);
					},
					[&](std::shared_ptr<mdox_vector> & b, int&)->Value
					{
						std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
						//r->vector.reserve(b->vector.size());

						for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
						{
							r->vector.emplace_back(Value::Resta(*sss, v2));
						}
						return std::move(r);
					},

					[&](std::shared_ptr<mdox_vector> & a, std::shared_ptr<mdox_vector> & b)->Value
					{
						if (a->vector.size() == b->vector.size() && !a->vector.empty())
						{
							std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
							//r->vector.reserve(a->vector.size());

							int itr = 0;
							for (std::deque<Value>::iterator sss = a->vector.begin(); sss != a->vector.end(); ++sss)
							{
								r->vector.emplace_back(Value::Resta(*sss, b->vector[itr]));
								itr++;
							}
							return std::move(r);
						}

						if (a->vector.empty() || b->vector.empty())
						{
							if (a->vector.empty())
								return b;
							else return a;
						}

						std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
						//r->vector.reserve(a->vector.size() > b->vector.size() ? a->vector.size() : b->vector.size());

						Value* a1 = &*a->vector.begin();
						Value* b1 = &*b->vector.begin();

						Value* a2 = &a->vector.back();
						Value* b2 = &b->vector.back();


						while (true)
						{
							if (a1 == a2)
							{
								r->vector.emplace_back(Value::Resta(*a1, *b1));

								if (b1 == b2)
									return r;

								b1++;

								OPERADORES op = ELEM_NEG_FIRST;
								while (b1 != b2)
								{
									r->vector.emplace_back(b1->operacion_Unitaria(op));
									b1++;
								}

								r->vector.emplace_back(b1->operacion_Unitaria(op));
								return r;
							}

							if (b1 == b2)
							{
								r->vector.emplace_back(Value::Resta(*a1, *b1));

								if (a1 == a2)
									return r;

								a1++;

								while (a1 != a2)
								{
									r->vector.emplace_back(*a1);
									a1++;
								}
								r->vector.emplace_back(*a1);
								return r;
							}

							r->vector.emplace_back(Value::Resta(*a1, *b1));
							a1++; b1++;
						}
					},

					[&v1](auto& a, std::shared_ptr<mdox_object>& b)->Value
					{
						if (b->clase->right_operators && b->clase->right_operators->OPERATOR_resta)
						{
							return Interprete::instance->ExecOperador(b->clase->right_operators->OPERATOR_resta, v1, b->variables_clase);
						}

						Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "-", b->clase->pID->nombre);
						return std::monostate();
					},

					[&v2](std::shared_ptr<mdox_object>& a, auto& b)->Value
					{
						if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_resta)
						{
							return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_resta, v2, a->variables_clase);
						}

						Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "-", a->clase->pID->nombre);
						return std::monostate();
					},

					[&](std::shared_ptr<mdox_object>& a, std::shared_ptr<mdox_object>& b)->Value
					{
						if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_resta)
						{
							return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_resta, v2, a->variables_clase);
						}
						if (b->clase->right_operators && b->clase->right_operators->OPERATOR_resta)
						{
							return Interprete::instance->ExecOperador(b->clase->right_operators->OPERATOR_resta, v1, b->variables_clase);
						}

						Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "-", a->clase->pID->nombre);
						return std::monostate();
					},

					[&v2](std::shared_ptr<mdox_object>& a, std::shared_ptr<mdox_vector>& b)->Value
					{
						if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_resta)
						{
							return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_resta, v2, a->variables_clase);
						}

						Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "-", a->clase->pID->nombre);
						return std::monostate();
					},

					[](auto,auto)->Value { return std::monostate(); },
		}, v1.value, v2.value);
}

Value Value::Multiplicacion(Value& v1, Value& v2)
{
	return std::visit(overloaded{

		//INTEGER .. XX
		[](const int& a,const int& b)->Value
		{
			int t = a * b;
			if (multOvf(&t, a, b))
				return (long long(a) * b);
			return (t);
		},
		[](const int& a, const bool& b)->Value { return (a * b);	},
		[](const int& a, const double& b)->Value { return (a * b); },
		[](const int& a, std::string & b)->Value
		{
			std::string ex = "";
			for (int itr = 0; itr < a; itr++)
				ex += b;
			return (ex);
		},
		[](const int& a, const long long& b)->Value { return (a * b); },

			//DOUBLE .. XX
			[](const double& a,const int& b)->Value { return (a * b); },
			[](const double& a,const bool& b)->Value { return (a * b); },
			[](const double& a, const double& b)->Value { return (a * b); },
			[](const double& a,  std::string & b)->Value { Errores::generarError(Errores::ERROR_MATH_MULT_STRING, Errores::outData, "double"); return std::monostate(); },
			[](const double& a, const long long& b)->Value { return (a * b); },

			//LONGLONG .. XX
			[](const long long& a, const int& b)->Value { return (a * b); },
			[](const long long& a, const bool& b)->Value { return (a * b); },
			[](const long long& a, const double& b)->Value { return (a * b); },
			[](const long long& a,  std::string & b)->Value
			{
				std::string ex = "";
				for (int itr = 0; itr < a; itr++)
					ex += b;
				return (ex);
			},
			[](const long long& a, const long long& b)->Value { return (a * b); },

				//STRING .. XX
				[](std::string & a,int& b)->Value {
					std::string ex = "";
					for (int itr = 0; itr < b; itr++)
						ex += a;
					return (ex);
				},
				[](std::string & a,const bool& b)->Value { if (b) return (a); else return ""; },
				[](std::string & a,const double& b)->Value { Errores::generarError(Errores::ERROR_MATH_MULT_STRING, Errores::outData, "double"); return std::monostate(); },
				[](std::string & a, std::string & b)->Value {/*abc ab = aa ab ba bb ca cb*/ return (false); },
				[](std::string & a,const long long& b)->Value { return (a + to_string_p(b)); },

					//BOOL .. XX
					[](const bool& a,const int& b)->Value { return (a * b); },
					[](const bool& a,const bool& b)->Value { return (a * b); },
					[](const bool& a, const double& b)->Value { return (a * b); },
					[](const bool& a, std::string & b)->Value { if (a) return (b); else return ""; },
					[](const bool& a, const long long& b)->Value { return (a * b); },


					[&](auto&, std::shared_ptr<mdox_vector> & b)->Value
					{
						std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
						//r->vector.reserve(b->vector.size());

						for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
						{
							r->vector.emplace_back(Value::Multiplicacion(v1, *sss));
						}
						return std::move(r);
					},

					[&](std::shared_ptr<mdox_vector> & b, const bool&)->Value
					{
						std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
						//r->vector.reserve(b->vector.size());

						for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
						{
							r->vector.emplace_back(Value::Multiplicacion(*sss, v2));
						}
						return std::move(r);
					},
					[&](std::shared_ptr<mdox_vector> & b, const double&)->Value
					{
						std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
						//r->vector.reserve(b->vector.size());

						for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
						{
							r->vector.emplace_back(Value::Multiplicacion(*sss, v2));
						}
						return std::move(r);
					},
					[&](std::shared_ptr<mdox_vector> & b, std::string&)->Value
					{
						std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
						//r->vector.reserve(b->vector.size());

						for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
						{
							r->vector.emplace_back(Value::Multiplicacion(*sss, v2));
						}
						return std::move(r);
					},
					[&](std::shared_ptr<mdox_vector> & b, long long&)->Value
					{
						std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
						//r->vector.reserve(b->vector.size());

						for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
						{
							r->vector.emplace_back(Value::Multiplicacion(*sss, v2));
						}
						return std::move(r);
					},

					[&](std::shared_ptr<mdox_vector> & b, int&)->Value
					{
						std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
						//r->vector.reserve(b->vector.size());

						for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
						{
							r->vector.emplace_back(Value::Multiplicacion(*sss, v2));
						}
						return std::move(r);
					},

					[&](std::shared_ptr<mdox_vector> & a, std::shared_ptr<mdox_vector> & b)->Value
					{
						if (a->vector.size() == b->vector.size() && !a->vector.empty())
						{
							std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
							//r->vector.reserve(a->vector.size());

							int itr = 0;
							for (std::deque<Value>::iterator sss = a->vector.begin(); sss != a->vector.end(); ++sss)
							{
								r->vector.emplace_back(Value::Multiplicacion(*sss, b->vector[itr]));
								itr++;
							}
							return std::move(r);
						}

						if (a->vector.empty() || b->vector.empty())
						{
							if (a->vector.empty())
								return b;
							else return a;
						}

						std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
						//r->vector.reserve(a->vector.size() > b->vector.size() ? a->vector.size() : b->vector.size());

						Value* a1 = &*a->vector.begin();
						Value* b1 = &*b->vector.begin();

						Value* a2 = &a->vector.back();
						Value* b2 = &b->vector.back();


						while (true)
						{
							if (a1 == a2)
							{
								r->vector.emplace_back(Value::Multiplicacion(*a1, *b1));

								if (b1 == b2)
									return r;

								b1++;

								while (b1 != b2)
								{
									r->vector.emplace_back(std::monostate());
									b1++;
								}

								r->vector.emplace_back(std::monostate());
								return r;
							}

							if (b1 == b2)
							{
								r->vector.emplace_back(Value::Multiplicacion(*a1, *b1));

								if (a1 == a2)
									return r;

								a1++;

								while (a1 != a2)
								{
									r->vector.emplace_back(std::monostate());
									a1++;
								}
								r->vector.emplace_back(std::monostate());
								return r;
							}

							r->vector.emplace_back(Value::Multiplicacion(*a1, *b1));
							a1++; b1++;
						}
					},


					[&v1](auto& a, std::shared_ptr<mdox_object>& b)->Value
					{
						if (b->clase->right_operators && b->clase->right_operators->OPERATOR_multiplicacion)
						{
							return Interprete::instance->ExecOperador(b->clase->right_operators->OPERATOR_multiplicacion, v1, b->variables_clase);
						}

						Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "*", b->clase->pID->nombre);
						return std::monostate();
					},

					[&v2](std::shared_ptr<mdox_object>& a, auto& b)->Value
					{
						if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_multiplicacion)
						{
							return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_multiplicacion, v2, a->variables_clase);
						}

						Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "*", a->clase->pID->nombre);
						return std::monostate();
					},

					[&](std::shared_ptr<mdox_object>& a, std::shared_ptr<mdox_object>& b)->Value
					{
						if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_multiplicacion)
						{
							return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_multiplicacion, v2, a->variables_clase);
						}
						if (b->clase->right_operators && b->clase->right_operators->OPERATOR_multiplicacion)
						{
							return Interprete::instance->ExecOperador(b->clase->right_operators->OPERATOR_multiplicacion, v1, b->variables_clase);
						}
						Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "*", a->clase->pID->nombre);
						return std::monostate();
					},

					[&v2](std::shared_ptr<mdox_object>& a, std::shared_ptr<mdox_vector>& b)->Value
					{
						if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_multiplicacion)
						{
							return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_multiplicacion, v2, a->variables_clase);
						}

						Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "*", a->clase->pID->nombre);
						return std::monostate();
					},

					[](auto,auto)->Value { return std::monostate(); },

		}, v1.value, v2.value);
}


Value Value::Div(Value& v1, Value& v2)
{
	return std::visit(overloaded{

		//INTEGER .. XX
		[](const int& a,const int& b)->Value { return (a / (double)b); },
		[](const int& a,const bool& b)->Value { return (a / (double)b); },
		[](const int& a,const double& b)->Value { return (a / b); },

		[](const int& a,  std::string & b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "/"); return std::monostate(); },
		[](const int& a, const long long& b)->Value { return ((double)a / b); },

		//DOUBLE .. XX
		[](const double& a, const int& b)->Value { return (a / b); },
		[](const double& a, const bool& b)->Value { return (a / b); },
		[](const double& a, const double& b)->Value { return (a / b); },
		[](const double& a,  std::string & b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "/"); return std::monostate(); },
		[](const double& a, const long long& b)->Value { return (a / b); },

		//LONGLONG .. XX
		[](const long long& a,const int& b)->Value { return (a / (double)b); },
		[](const long long& a,const bool& b)->Value { return (a / (double)b); },
		[](const long long& a, const double& b)->Value { return (a / b); },
		[](const long long& a,  std::string & b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "/"); return std::monostate(); },
		[](const long long& a,const  long long& b)->Value { return (a / (double)b); },

		//STRING .. XX
		[](std::string & a,const int& b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "/"); return std::monostate(); },
		[](std::string & a,const bool& b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "/"); return std::monostate(); },
		[](std::string & a,const double& b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "/"); return std::monostate(); },
		[](std::string & a, std::string & b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "/"); return std::monostate(); },
		[](std::string & a,const long long& b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "/"); return std::monostate(); },

		//BOOL .. XX
		[](const bool& a,const int& b)->Value { return ((double)a / b); },
		[](const bool& a,const bool& b)->Value { return ((double)a / b); },
		[](const bool& a,const  double& b)->Value { return (a / b); },
		[](const bool& a,  std::string & b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "/"); return std::monostate(); },
		[](const bool& a, const long long& b)->Value { return ((double)a / b); },

		[&](auto&, std::shared_ptr<mdox_vector> & b)->Value
		{
			std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
			//r->vector.reserve(b->vector.size());

				for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
				{
					r->vector.emplace_back(Value::Div(v1, *sss));
				}
				return std::move(r);
		},

		[&](std::shared_ptr<mdox_vector> & b, const bool&)->Value
		{
			std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
			//r->vector.reserve(b->vector.size());

				for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
				{
					r->vector.emplace_back(Value::Div(*sss, v2));
				}
				return std::move(r);
		},
		[&](std::shared_ptr<mdox_vector> & b, const double&)->Value
		{
			std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
			//r->vector.reserve(b->vector.size());

				for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
				{
					r->vector.emplace_back(Value::Div(*sss, v2));
				}
				return std::move(r);
		},
		[&](std::shared_ptr<mdox_vector> & b, std::string&)->Value
		{
			Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "/"); return std::monostate();
		},
		[&](std::shared_ptr<mdox_vector> & b, long long&)->Value
		{
			std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
			//r->vector.reserve(b->vector.size());

				for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
				{
					r->vector.emplace_back(Value::Div(*sss, v2));
				}
				return std::move(r);
		},
		[&](std::shared_ptr<mdox_vector> & b, int&)->Value
		{
			std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
			//r->vector.reserve(b->vector.size());

				for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
				{
					r->vector.emplace_back(Value::Div(*sss, v2));
				}
				return std::move(r);
		},

		[&](std::shared_ptr<mdox_vector> & a, std::shared_ptr<mdox_vector> & b)->Value
		{
			if (a->vector.size() == b->vector.size() && !a->vector.empty())
			{
				std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
				//r->vector.reserve(a->vector.size());

				int itr = 0;
				for (std::deque<Value>::iterator sss = a->vector.begin(); sss != a->vector.end(); ++sss)
				{
					r->vector.emplace_back(Value::Div(*sss, b->vector[itr]));
					itr++;
				}
				return std::move(r);
			}

			if (a->vector.empty() || b->vector.empty())
			{
				if (a->vector.empty())
					return b;
				else return a;
			}

			std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
			//r->vector.reserve(a->vector.size() > b->vector.size() ? a->vector.size() : b->vector.size());

			Value* a1 = &*a->vector.begin();
			Value* b1 = &*b->vector.begin();

			Value* a2 = &a->vector.back();
			Value* b2 = &b->vector.back();


			while (true)
			{
				if (a1 == a2)
				{
					r->vector.emplace_back(Value::Div(*a1, *b1));

					if (b1 == b2)
						return r;

					b1++;

					while (b1 != b2)
					{
						r->vector.emplace_back(std::monostate());
						b1++;
					}

					r->vector.emplace_back(std::monostate());
					return r;
				}

				if (b1 == b2)
				{
					r->vector.emplace_back(Value::Div(*a1, *b1));

					if (a1 == a2)
						return r;

					a1++;

					while (a1 != a2)
					{
						r->vector.emplace_back(std::monostate());
						a1++;
					}
					r->vector.emplace_back(std::monostate());
					return r;
				}

				r->vector.emplace_back(Value::Div(*a1, *b1));
				a1++; b1++;
			}
		},

		[&v1](auto& a, std::shared_ptr<mdox_object>& b)->Value
		{
			if (b->clase->right_operators && b->clase->right_operators->OPERATOR_div)
			{
				return Interprete::instance->ExecOperador(b->clase->right_operators->OPERATOR_div, v1, b->variables_clase);
			}

			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "/", b->clase->pID->nombre);
			return std::monostate();
		},

		[&v2](std::shared_ptr<mdox_object>& a, auto& b)->Value
		{
			if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_div)
			{
				return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_div, v2, a->variables_clase);
			}

			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "/", a->clase->pID->nombre);
			return std::monostate();
		},

		[&](std::shared_ptr<mdox_object>& a, std::shared_ptr<mdox_object>& b)->Value
		{
			if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_div)
			{
				return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_div, v2, a->variables_clase);
			}
			if (b->clase->right_operators && b->clase->right_operators->OPERATOR_div)
			{
				return Interprete::instance->ExecOperador(b->clase->right_operators->OPERATOR_div, v1, b->variables_clase);
			}
			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "/", a->clase->pID->nombre);
			return std::monostate();
		},

		[&v2](std::shared_ptr<mdox_object>& a, std::shared_ptr<mdox_vector>& b)->Value
		{
			if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_div)
			{
				return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_div, v2, a->variables_clase);
			}

			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "/", a->clase->pID->nombre);
			return std::monostate();
		},

		[](auto,auto)->Value { return std::monostate(); },

		}, v1.value, v2.value);
}


Value Value::DivEntera(Value& v1, Value& v2)
{
	return std::visit(overloaded{

		//INTEGER .. XX
		[](const int& a,const int& b)->Value { return ((int)(a / b)); },
		[](const int& a,const bool& b)->Value { return ((int)(a / b)); },
		[](const int& a, const double& b)->Value { return ((int)(a / b)); },

		[](const int& a,  std::string & b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "div"); return std::monostate(); },
		[](const int& a, const long long& b)->Value { return ((int)(a / b)); },

		//DOUBLE .. XX
		[](const double& a,const int& b)->Value { return ((int)(a / b)); },
		[](const double& a,const bool& b)->Value { return ((int)(a / b)); },
		[](const double& a,const double& b)->Value { return (a / b); },
		[](const double& a, std::string & b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "div"); return std::monostate(); },
		[](const double& a, const long long& b)->Value { return ((int)(a / b)); },

		//LONGLONG .. XX
		[](const long long& a,const int& b)->Value { return ((int)(a / b)); },
		[](const long long& a,const bool& b)->Value { return ((int)(a / b)); },
		[](const long long& a, const double& b)->Value { return ((int)(a / b)); },
		[](const long long& a,  std::string & b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "div"); return std::monostate(); },
		[](const long long& a, const long long& b)->Value { return ((int)(a / b)); },

		//STRING .. XX
		[](std::string & a,const int& b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "div"); return std::monostate(); },
		[](std::string & a,const bool& b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "div"); return std::monostate(); },
		[](std::string & a,const double& b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "div"); return std::monostate(); },
		[](std::string & a, std::string & b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "div"); return std::monostate(); },
		[](std::string & a,const long long& b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "div"); return std::monostate(); },

		//BOOL .. XX
		[](const bool& a,const int& b)->Value { return ((int)(a / b)); },
		[](const bool& a,const bool& b)->Value { return ((int)(a / b)); },
		[](const bool& a, const double& b)->Value { return ((int)(a / b)); },
		[](const bool& a,  std::string & b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "div"); return std::monostate(); },
		[](const bool& a, const long long& b)->Value { return ((int)(a / b)); },

		[&](auto&, std::shared_ptr<mdox_vector> & b)->Value
		{
			std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
			//r->vector.reserve(b->vector.size());

				for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
				{
					r->vector.emplace_back(Value::DivEntera(v1, *sss));
				}
				return std::move(r);
		},

		[&](std::shared_ptr<mdox_vector> & b, const bool&)->Value
		{
			std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
			//r->vector.reserve(b->vector.size());

				for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
				{
					r->vector.emplace_back(Value::DivEntera(*sss,v2));
				}
				return std::move(r);
		},
		[&](std::shared_ptr<mdox_vector> & b, const double&)->Value
		{
			std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
			//r->vector.reserve(b->vector.size());

				for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
				{
					r->vector.emplace_back(Value::DivEntera(*sss,v2));
				}
				return std::move(r);
		},
		[&](std::shared_ptr<mdox_vector> & b, std::string&)->Value
		{
			Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "div"); return std::monostate();
		},
		[&](std::shared_ptr<mdox_vector> & b, long long&)->Value
		{
			std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
			//r->vector.reserve(b->vector.size());

				for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
				{
					r->vector.emplace_back(Value::DivEntera(*sss,v2));
				}
				return std::move(r);
		},
		[&](std::shared_ptr<mdox_vector> & b, int&)->Value
		{
			std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
			//r->vector.reserve(b->vector.size());

				for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
				{
					r->vector.emplace_back(Value::DivEntera(*sss,v2));
				}
				return std::move(r);
		},

		[&](std::shared_ptr<mdox_vector> & a, std::shared_ptr<mdox_vector> & b)->Value
		{
			if (a->vector.size() == b->vector.size() && !a->vector.empty())
			{
				std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
				//r->vector.reserve(a->vector.size());

				int itr = 0;
				for (std::deque<Value>::iterator sss = a->vector.begin(); sss != a->vector.end(); ++sss)
				{
					r->vector.emplace_back(Value::DivEntera(*sss, b->vector[itr]));
					itr++;
				}
				return std::move(r);
			}

			if (a->vector.empty() || b->vector.empty())
			{
				if (a->vector.empty())
					return b;
				else return a;
			}

			std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
			//	r->vector.reserve(a->vector.size() > b->vector.size() ? a->vector.size() : b->vector.size());

			Value* a1 = &*a->vector.begin();
			Value* b1 = &*b->vector.begin();

			Value* a2 = &a->vector.back();
			Value* b2 = &b->vector.back();


			while (true)
			{
				if (a1 == a2)
				{
					r->vector.emplace_back(Value::DivEntera(*a1, *b1));

					if (b1 == b2)
						return r;

					b1++;

					while (b1 != b2)
					{
						r->vector.emplace_back(std::monostate());
						b1++;
					}

					r->vector.emplace_back(std::monostate());
					return r;
				}

				if (b1 == b2)
				{
					r->vector.emplace_back(Value::DivEntera(*a1, *b1));

					if (a1 == a2)
						return r;

					a1++;

					while (a1 != a2)
					{
						r->vector.emplace_back(std::monostate());
						a1++;
					}
					r->vector.emplace_back(std::monostate());
					return r;
				}

				r->vector.emplace_back(Value::DivEntera(*a1, *b1));
				a1++; b1++;
			}
		},

		[&v1](auto& a, std::shared_ptr<mdox_object>& b)->Value
		{
			if (b->clase->right_operators && b->clase->right_operators->OPERATOR_divEntera)
			{
				return Interprete::instance->ExecOperador(b->clase->right_operators->OPERATOR_divEntera, v1, b->variables_clase);
			}

			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "div", b->clase->pID->nombre);
			return std::monostate();
		},

		[&v2](std::shared_ptr<mdox_object>& a, auto& b)->Value
		{
			if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_divEntera)
			{
				return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_divEntera, v2, a->variables_clase);
			}

			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "div", a->clase->pID->nombre);
			return std::monostate();
		},

		[&](std::shared_ptr<mdox_object>& a, std::shared_ptr<mdox_object>& b)->Value
		{
			if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_divEntera)
			{
				return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_divEntera, v2, a->variables_clase);
			}
			if (b->clase->right_operators && b->clase->right_operators->OPERATOR_divEntera)
			{
				return Interprete::instance->ExecOperador(b->clase->right_operators->OPERATOR_divEntera, v1, b->variables_clase);
			}
			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "div", a->clase->pID->nombre);
			return std::monostate();
		},

		[&v2](std::shared_ptr<mdox_object>& a, std::shared_ptr<mdox_vector>& b)->Value
		{
			if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_divEntera)
			{
				return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_divEntera, v2, a->variables_clase);
			}

			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "div", a->clase->pID->nombre);
			return std::monostate();
		},

		[](auto,auto)->Value { return std::monostate(); },

		}, v1.value, v2.value);
}

Value Value::Mod(Value& v1, Value& v2)
{
	return std::visit(overloaded{

		//INTEGER .. XX
		[](const int& a,const int& b)->Value { return (a % b); },
		[](const int& a,const bool& b)->Value { return (a % b); },
		[](const int& a, const double& b)->Value { return (fmod(a,b)); },
		[](const int& a,  std::string & b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "%"); return std::monostate(); },
		[](const int& a, const long long& b)->Value { return (a % b); },

		//DOUBLE .. XX
		[](const double& a, const int& b)->Value { return (fmod(a,b)); },
		[](const double& a, const bool& b)->Value { return (fmod(a,b)); },
		[](const double& a, const double& b)->Value { return (fmod(a,b)); },
		[](const double& a,  std::string & b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "%"); return std::monostate(); },
		[](const double& a, const long long& b)->Value { return (fmod(a,b)); },

		//LONGLONG .. XX
		[](const long long& a,const int& b)->Value { return (a % b); },
		[](const long long& a,const bool& b)->Value { return (a % b); },
		[](const long long& a,const double& b)->Value { return (fmod(a,b)); },
		[](const long long& a,  std::string & b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "%"); return std::monostate(); },
		[](const long long& a,const long long& b)->Value { return (a % b); },

		//STRING .. XX
		[](std::string & a,const int& b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "%"); return std::monostate(); },
		[](std::string & a,const bool& b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "%"); return std::monostate(); },
		[](std::string & a,const double& b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "%"); return std::monostate(); },
		[](std::string & a, std::string & b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "%"); return std::monostate(); },
		[](std::string & a,const long long& b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "%"); return std::monostate(); },

		//BOOL .. XX
		[](const bool& a, const int& b)->Value { return (a % b); },
		[](const bool& a, const bool& b)->Value { return (a % b); },
		[](const bool& a, const double& b)->Value { return (fmod(a,b)); },
		[](const bool& a,  std::string & b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "%"); return std::monostate(); },
		[](const bool& a, const long long& b)->Value { return (a % b); },

		[&](auto&, std::shared_ptr<mdox_vector> & b)->Value
		{
			std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
			//r->vector.reserve(b->vector.size());

				for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
				{
					r->vector.emplace_back(Value::Mod(v1,*sss));
				}
				return std::move(r);
		},

		[&](std::shared_ptr<mdox_vector> & b, const bool&)->Value
		{
			std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
			//r->vector.reserve(b->vector.size());

				for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
				{
					r->vector.emplace_back(Value::Mod(*sss,v2));
				}
				return std::move(r);
		},
		[&](std::shared_ptr<mdox_vector> & b, const double&)->Value
		{
			std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
			//r->vector.reserve(b->vector.size());

				for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
				{
					r->vector.emplace_back(Value::Mod(*sss,v2));
				}
				return std::move(r);
		},
		[&](std::shared_ptr<mdox_vector> & b, std::string&)->Value
		{
			std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
			//r->vector.reserve(b->vector.size());

				for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
				{
					r->vector.emplace_back(Value::Mod(*sss,v2));
				}
				return std::move(r);
		},
		[&](std::shared_ptr<mdox_vector> & b, long long&)->Value
		{
			std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
			//r->vector.reserve(b->vector.size());

				for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
				{
					r->vector.emplace_back(Value::Mod(*sss,v2));
				}
				return std::move(r);
		},
		[&](std::shared_ptr<mdox_vector> & b, int&)->Value
		{
			std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
			//r->vector.reserve(b->vector.size());

				for (std::deque<Value>::iterator sss = b->vector.begin(); sss != b->vector.end(); ++sss)
				{
					r->vector.emplace_back(Value::Mod(*sss,v2));
				}
				return std::move(r);
		},


		[&](std::shared_ptr<mdox_vector> & a, std::shared_ptr<mdox_vector> & b)->Value
		{
			if (a->vector.size() == b->vector.size() && !a->vector.empty())
			{
				std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
				//r->vector.reserve(a->vector.size());

				int itr = 0;
				for (std::deque<Value>::iterator sss = a->vector.begin(); sss != a->vector.end(); ++sss)
				{
					r->vector.emplace_back(Value::Mod(*sss, b->vector[itr]));
					itr++;
				}
				return std::move(r);
			}

			if (a->vector.empty() || b->vector.empty())
			{
				if (a->vector.empty())
					return b;
				else return a;
			}

			std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
			//r->vector.reserve(a->vector.size() > b->vector.size() ? a->vector.size() : b->vector.size());

			Value* a1 = &*a->vector.begin();
			Value* b1 = &*b->vector.begin();

			Value* a2 = &a->vector.back();
			Value* b2 = &b->vector.back();


			while (true)
			{
				if (a1 == a2)
				{
					r->vector.emplace_back(Value::Mod(*a1, *b1));

					if (b1 == b2)
						return r;

					b1++;

					while (b1 != b2)
					{
						r->vector.emplace_back(std::monostate());
						b1++;
					}

					r->vector.emplace_back(std::monostate());
					return r;
				}

				if (b1 == b2)
				{
					r->vector.emplace_back(Value::Mod(*a1, *b1));

					if (a1 == a2)
						return r;

					a1++;

					while (a1 != a2)
					{
						r->vector.emplace_back(std::monostate());
						a1++;
					}
					r->vector.emplace_back(std::monostate());
					return r;
				}

				r->vector.emplace_back(Value::Mod(*a1, *b1));
				a1++; b1++;
			}
		},

		[&v1](auto& a, std::shared_ptr<mdox_object>& b)->Value
		{
			if (b->clase->right_operators && b->clase->right_operators->OPERATOR_mod)
			{
				return Interprete::instance->ExecOperador(b->clase->right_operators->OPERATOR_mod, v1, b->variables_clase);
			}

			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "%", b->clase->pID->nombre);
			return std::monostate();
		},

		[&v2](std::shared_ptr<mdox_object>& a, auto& b)->Value
		{
			if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_mod)
			{
				return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_mod, v2, a->variables_clase);
			}

			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "%", a->clase->pID->nombre);
			return std::monostate();
		},

		[&](std::shared_ptr<mdox_object>& a, std::shared_ptr<mdox_object>& b)->Value
		{
			if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_mod)
			{
				return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_mod, v2, a->variables_clase);
			}
			if (b->clase->right_operators && b->clase->right_operators->OPERATOR_mod)
			{
				return Interprete::instance->ExecOperador(b->clase->right_operators->OPERATOR_mod, v1, b->variables_clase);
			}
			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "%", a->clase->pID->nombre);
			return std::monostate();
		},

		[&v2](std::shared_ptr<mdox_object>& a, std::shared_ptr<mdox_vector>& b)->Value
		{
			if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_mod)
			{
				return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_mod, v2, a->variables_clase);
			}

			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "%", a->clase->pID->nombre);
			return std::monostate();
		},

		[](auto,auto)->Value { return std::monostate(); },

		}, v1.value, v2.value);
}

Value Value::Offset(Value& v1, Value& v2)
{
	return std::visit(overloaded{
		
		[&](std::shared_ptr<mdox_vector> & a, int& b)
		{
			if (b > a->vector.size() || b < 0)
			{
				Errores::generarError(Errores::ERROR_OFFSET_INVALIDO, Errores::outData, std::to_string(b));
				return Value();
			}
			return a->vector[b];
		},

		[&](std::shared_ptr<mdox_vector> & a, long long& b)
		{
			if (b > a->vector.size() || b < 0)
			{
				Errores::generarError(Errores::ERROR_OFFSET_INVALIDO, Errores::outData, std::to_string(b));
				return Value();
			}
			return a->vector[b];
		},

		[&](std::shared_ptr<mdox_vector> & a, auto&)
		{
			if (v2.Cast(PARAM_LINT))
			{
				long long tr = std::get<long long>(v2.value);
				if (tr > a->vector.size() || tr < 0)
				{
					Errores::generarError(Errores::ERROR_OFFSET_INVALIDO, Errores::outData, std::to_string(tr));
					return Value();
				}
				return a->vector[tr];
			}
			else
			{
				 Errores::generarError(Errores::ERROR_OPERADOR_INVALIDO,  Errores::outData, "[]"); 
				 return Value();
			}
		},
		[&v1](std::shared_ptr<mdox_vector>& a, std::shared_ptr<mdox_object>& b)
		{
			if (b->clase->right_operators && b->clase->right_operators->OPERATOR_brack)
			{
				return Interprete::instance->ExecOperador(b->clase->right_operators->OPERATOR_brack, v1, b->variables_clase);
			}

			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "[]", b->clase->pID->nombre);
			return Value();
		},

		[&v1](auto & a, std::shared_ptr<mdox_object> & b)
		{
			if (b->clase->right_operators && b->clase->right_operators->OPERATOR_brack)
			{
				return Interprete::instance->ExecOperador(b->clase->right_operators->OPERATOR_brack, v1, b->variables_clase);
			}

			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "[]", b->clase->pID->nombre);
			return Value();
		},



		[&v2](std::shared_ptr<mdox_object> & a, auto & b)
		{
			if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_brack)
			{
				return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_brack, v2, a->variables_clase);
			}

			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "[]", a->clase->pID->nombre);
			return Value();
		},

		[&v2](std::shared_ptr<mdox_object> & a, std::shared_ptr<mdox_object> & b)
		{
			if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_brack)
			{
				return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_brack, v2, a->variables_clase);
			}

			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "[]", b->clase->pID->nombre);
			return Value();
		},

		[&v2](std::shared_ptr<mdox_object> & a, std::shared_ptr<mdox_vector> & b)
		{
			if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_brack)
			{
				return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_brack, v2, a->variables_clase);
			}

			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "[]", a->clase->pID->nombre);
			return Value();
		},

		[](auto&,auto&) {  Errores::generarError(Errores::ERROR_OPERADOR_INVALIDO,  Errores::outData, "[]"); return Value(); },

		}, v1.value, v2.value);
}

Value Value::ClassAccess(Parser_Identificador* v2, Call_Value* call, Variable_Runtime* variables, Variable_Runtime * var_class)
{
	Interprete::instance->getRealValueFromValueWrapper(*this);

	return std::visit(overloaded{

		[&](std::shared_ptr<mdox_object> & a)->Value 
		{ 
			if (call)
				return Interprete::instance->ExecFuncion(call, Interprete::instance->transformarEntradasCall(call, variables, var_class), a->variables_clase, a->clase);
			else
			{
				auto ret = a->findVariable(v2->nombre);//wrapper_object_call(a,v2);
				if (ret == nullptr)
				{
					Errores::generarError(Errores::ERROR_CLASE_VAR_NOT_EXIST, Errores::outData, v2->nombre);
					return std::monostate();
				}
				return ret;
			}
		},
		[](auto)->Value {  Errores::generarError(Errores::ERROR_OPERADOR_INVALIDO,  Errores::outData, "."); return std::monostate(); },

		}, value);
}

Value Value::operacion_Binaria(Value & v, const OPERADORES op)
{

	Interprete::instance->getRealValueFromValueWrapper(*this);
	Interprete::instance->getRealValueFromValueWrapper(v);

	switch (op)
	{

	// --------------------------------------------------------------------
	// ---------------------- OPERACIÓN SUMA ------------------------------
	// --------------------------------------------------------------------
	case OP_ARIT_SUMA:
	{
		return Suma(*this, v);
	}

	// --------------------------------------------------------------------
	// ---------------------- OPERACIÓN RESTA -----------------------------
	// --------------------------------------------------------------------
	case OP_ARIT_RESTA:
	{
		return Resta(*this, v);
	}
	// --------------------------------------------------------------------
	// ------------------ OPERACIÓN MULTIPLICACIÓN ------------------------
	// --------------------------------------------------------------------
	case OP_ARIT_MULT:
	{
		return Multiplicacion(*this, v);
	}

	// --------------------------------------------------------------------
	// ---------------------- OPERACIÓN DIVISIÓN --------------------------
	// --------------------------------------------------------------------
	case OP_ARIT_DIV:
	{
		return Div(*this, v);
	}

	// --------------------------------------------------------------------
	// ------------------ OPERACIÓN DIVISIÓN ENTERA -----------------------
	// --------------------------------------------------------------------
	case OP_ARIT_DIV_ENTERA:
	{
		return DivEntera(*this, v);
	}

	// --------------------------------------------------------------------
	// ------------------------- OPERACIÓN MOD ----------------------------
	// --------------------------------------------------------------------
	case OP_ARIT_MOD:
	{
		return Mod(*this, v);
	}

	// --------------------------------------------------------------------
	// ------------------------- OPERACIÓN OFFSET ----------------------------
	// --------------------------------------------------------------------
	case OP_BRACKET_LEFT:
	{
		return Offset(*this, v);
	}

	// --------------------------------------------------------------------
	// -------------------- OPERACIÓNES CONDICIONALES ---------------------
	// --------------------------------------------------------------------

	case OPERADORES::OP_REL_EQUAL:
	{
		return this->igualdad_Condicional(v);
	}

	case OPERADORES::OP_REL_NOT_EQUAL:
	{
		return !this->igualdad_Condicional(v);
	}

	case OPERADORES::OP_REL_MINOR:
	{
		return this->menorQue_Condicional(v);
	}

	case OPERADORES::OP_REL_MAJOR:
	{
		return this->mayorQue_Condicional(v);
	}

	case OPERADORES::OP_REL_MAJOR_OR_EQUAL:
	{
		return this->mayorIgualQue_Condicional(v);
	}

	case OPERADORES::OP_REL_MINOR_OR_EQUAL:
	{
		return this->menorIgualQue_Condicional(v);
	}


	// --------------------------------------------------------------------
	// ---------------------- OPERACIÓNES LÓGICAS -------------------------
	// --------------------------------------------------------------------

	case OPERADORES::OP_LOG_ADD:
	{
		return std::visit(overloaded{

			//INTEGER .. XX
			[](const int& a,const int& b) { return a && b; },
			[](const int& a,const bool& b) { return a && b; },
			[](const int& a,const long long& b) { return a && b; },
			[](const int& a,const double& b) { return a && b; },
			[&v](const int& a, std::string&)
			{
				return a && v.ValueToBool();
			},
			[](const int& a, std::shared_ptr<mdox_vector> & b) { return a && !b->vector.empty(); },

			[](const long long& a, const long long& b) { return a && b; },
			[](const long long& a, const  bool& b) { return a && b; },
			[](const long long& a, const int& b) { return a && b; },
			[](const long long& a, const double& b) { return a && b; },
			[&v](const long long& a,  std::string&)
			{
				return a && v.ValueToBool();
			},
			[](const long long& a, std::shared_ptr<mdox_vector> & b) { return a && !b->vector.empty(); },

			[](const double& a, const double& b) { return a && b; },
			[](const double& a, const int& b) { return a && b; },
			[](const double& a, const long long& b) { return a && b; },
			[](const double& a, const bool& b) { return a && b; },
			[&v](const double& a,  std::string&)
			{
				return a && v.ValueToBool();
			},
			[](const double& a, std::shared_ptr<mdox_vector> & b) { return a && !b->vector.empty(); },

			[](const bool& a, const double& b) { return a && b;  },
			[](const bool& a, const int& b) { return a && b; },
			[](const bool& a, const long long& b) { return a && b; },
			[](const bool& a, const bool& b) { return a && b;  },
			[&v](const bool& a,  std::string&)
			{
				return a && v.ValueToBool();
			},
			[](const bool& a, std::shared_ptr<mdox_vector> & b) { return a && !b->vector.empty(); },

			[this]( std::string & a, const double& b)
			{
				return this->ValueToBool() && b;
			},
			[this]( std::string & a, const int& b)
			{
				return this->ValueToBool() && b;
			},
			[this]( std::string & a, const long long& b)
			{
				return this->ValueToBool() && b;
			},
			[this]( std::string & a, const bool& b)
			{
				return this->ValueToBool() && b;
			},
			[&]( std::string & a,  std::string & b)
			{
				return this->ValueToBool() && v.ValueToBool();
			},
			[&]( std::string & a, std::shared_ptr<mdox_vector> & b) { return this->ValueToBool() && !b->vector.empty(); },


			[](std::shared_ptr<mdox_vector> & a, const double& b) { return !a->vector.empty() && b; },
			[](std::shared_ptr<mdox_vector> & a, const int& b) { return !a->vector.empty() && b; },
			[](std::shared_ptr<mdox_vector> & a, const long long& b) { return !a->vector.empty() && b; },
			[](std::shared_ptr<mdox_vector> & a, const bool& b) { return !a->vector.empty() && b; },
			[&v](std::shared_ptr<mdox_vector> & a,  std::string&)
			{
				return !a->vector.empty() && v.ValueToBool();
			},
			[](std::shared_ptr<mdox_vector> & a, std::shared_ptr<mdox_vector> & b) { return !a->vector.empty() && !b->vector.empty(); },

			[&](auto& a, std::shared_ptr<mdox_object>& b)
			{
				if (b->clase->right_operators && b->clase->right_operators->OPERATOR_log_and)
				{
					return Interprete::instance->ExecOperador(b->clase->right_operators->OPERATOR_log_and, *this, b->variables_clase).ValueToBool();
				}

				Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "&&", b->clase->pID->nombre);
				return false;
			},

			[&v](std::shared_ptr<mdox_object>& a, auto& b)
			{
				if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_log_and)
				{
					return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_log_and, v, a->variables_clase).ValueToBool();
				}

				Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "&&", a->clase->pID->nombre);
				return false;
			},

			[&](std::shared_ptr<mdox_object>& a, std::shared_ptr<mdox_object>& b)
			{
				if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_log_and)
				{
					return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_log_and, v, a->variables_clase).ValueToBool();
				}
				if (b->clase->right_operators && b->clase->right_operators->OPERATOR_log_and)
				{
					return Interprete::instance->ExecOperador(b->clase->right_operators->OPERATOR_log_and, *this, b->variables_clase).ValueToBool();
				}

				Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "&&", a->clase->pID->nombre);
				return false;
			},

			[](auto,auto) { return false; },

			}, value, v.value);
		break;
	}

	case OPERADORES::OP_LOG_OR:
	{
		return std::visit(overloaded{

			//INTEGER .. XX
			[](const int& a,const int& b) { return a || b; },
			[](const int& a,const bool& b) { return a || b; },
			[](const int& a,const long long& b) { return a || b; },
			[](const int& a,const double& b) { return a || b; },
			[&v](const int& a, std::string&)
			{
				return a || v.ValueToBool();
			},
			[](const int& a, std::shared_ptr<mdox_vector> & b) { return a || !b->vector.empty(); },

			[](const long long& a, const long long& b) { return a || b; },
			[](const long long& a, const  bool& b) { return a || b; },
			[](const long long& a, const int& b) { return a || b; },
			[](const long long& a, const double& b) { return a || b; },
			[&v](const long long& a,  std::string&)
			{
				return a || v.ValueToBool();
			},
			[](const long long& a, std::shared_ptr<mdox_vector> & b) { return a || !b->vector.empty(); },

			[](const double& a, const double& b) { return a || b; },
			[](const double& a, const int& b) { return a || b; },
			[](const double& a, const long long& b) { return a || b; },
			[](const double& a, const bool& b) { return a || b; },
			[&v](const double& a,  std::string&)
			{
				return a || v.ValueToBool();
			},
			[](const double& a, std::shared_ptr<mdox_vector> & b) { return a || !b->vector.empty(); },

			[](const bool& a, const double& b) { return a || b;  },
			[](const bool& a, const int& b) { return a || b; },
			[](const bool& a, const long long& b) { return a || b; },
			[](const bool& a, const bool& b) { return a || b;  },
			[&v](const bool& a,  std::string&)
			{
				return a || v.ValueToBool();
			},
			[](const bool& a, std::shared_ptr<mdox_vector> & b) { return a || !b->vector.empty(); },

			[this]( std::string & a, const double& b)
			{
				return this->ValueToBool() || b;
			},
			[this]( std::string & a, const int& b)
			{
				return this->ValueToBool() || b;
			},
			[this]( std::string & a, const long long& b)
			{
				return this->ValueToBool() || b;
			},
			[this]( std::string & a, const bool& b)
			{
				return this->ValueToBool() || b;
			},
			[&]( std::string & a,  std::string & b)
			{
				return this->ValueToBool() || v.ValueToBool();
			},
			[&]( std::string & a, std::shared_ptr<mdox_vector> & b) { return this->ValueToBool() || !b->vector.empty(); },


			[](std::shared_ptr<mdox_vector> & a, const double& b) { return !a->vector.empty() || b; },
			[](std::shared_ptr<mdox_vector> & a, const int& b) { return !a->vector.empty() || b; },
			[](std::shared_ptr<mdox_vector> & a, const long long& b) { return !a->vector.empty() || b; },
			[](std::shared_ptr<mdox_vector> & a, const bool& b) { return !a->vector.empty() || b; },
			[&v](std::shared_ptr<mdox_vector> & a,  std::string&)
			{
				return !a->vector.empty() || v.ValueToBool();
			},
			[](std::shared_ptr<mdox_vector> & a, std::shared_ptr<mdox_vector> & b) { return !a->vector.empty() || !b->vector.empty(); },

			[&](auto& a, std::shared_ptr<mdox_object>& b)
			{
				if (b->clase->right_operators && b->clase->right_operators->OPERATOR_log_or)
				{
					return Interprete::instance->ExecOperador(b->clase->right_operators->OPERATOR_log_or, *this, b->variables_clase).ValueToBool();
				}

				Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "||", b->clase->pID->nombre);
				return false;
			},

			[&v](std::shared_ptr<mdox_object>& a, auto& b)
			{
				if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_log_or)
				{
					return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_log_or, v, a->variables_clase).ValueToBool();
				}

				Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "||", a->clase->pID->nombre);
				return false;
			},

			[&](std::shared_ptr<mdox_object>& a, std::shared_ptr<mdox_object>& b)
			{
				if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_log_or)
				{
					return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_log_or, v, a->variables_clase).ValueToBool();
				}
				if (b->clase->right_operators && b->clase->right_operators->OPERATOR_log_or)
				{
					return Interprete::instance->ExecOperador(b->clase->right_operators->OPERATOR_log_or, *this, b->variables_clase).ValueToBool();
				}
				Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "||", a->clase->pID->nombre);
				return false;
			},

			[](auto,auto) { return false; },


			}, value, v.value);
		break;
	}
	}
	return std::monostate();
}

/*f1 indica si el valor de la izquierda es fuerte, f2 indica si lo es el de la derecha*/
bool Value::OperadoresEspeciales_Check(Value* v, int index/*, Parser_Identificador* f1, Parser_Identificador* f2*/)
{
	tipos_parametros tipo_1 = PARAM_VOID, tipo_2 = PARAM_VOID;

	Value* _this = &(*this);
	Interprete::instance->getRealValueFromValueWrapperRef(&_this, &tipo_1);
	Interprete::instance->getRealValueFromValueWrapperRef(&v, &tipo_2);


	if (index < 0)
	{
		return std::visit(overloaded{
			[&](std::shared_ptr<mdox_vector> & a, std::monostate)->bool
			{
				if (a->vector.size() > 0)
				{
					if (tipo_2 != PARAM_VOID)
					{
						v->inicializacion(tipo_2);

						v->asignacion(a->vector.back(), true);
					}
					else
					{
						v->asignacion(a->vector.back(), false);
					}
					return true;
				}
				else return false;
			},
			[&](std::monostate, std::shared_ptr<mdox_vector> & a)->bool
			{
				if (a->vector.size() > 0)
				{
					if (tipo_1 != PARAM_VOID)
					{
						_this->inicializacion(tipo_1);
						_this->asignacion(a->vector.front(), true);
					}
					else
					{
						_this->asignacion(a->vector.front(), false);
					}
					return true;
				}
				else return false;
			},
		//	[&](std::shared_ptr<mdox_vector> & a, auto&)->Value { return a->vector.back().igualdad_Condicional(v); },
		//	[&](auto&, std::shared_ptr<mdox_vector>& a)->Value {  return a->vector.front().igualdad_Condicional(*this); },

			[&](std::shared_ptr<mdox_vector> & a, std::shared_ptr<mdox_vector>&)->bool { if (a->vector.size() <= 0) return false; return a->vector.back().igualdad_Condicional(*v); },
			[&](std::shared_ptr<mdox_vector> & a, int&)->bool { if (a->vector.size() <= 0) return false; return a->vector.back().igualdad_Condicional(*v); },
			[&](std::shared_ptr<mdox_vector> & a, std::string&)->bool {if (a->vector.size() <= 0) return false; return a->vector.back().igualdad_Condicional(*v); },
			[&](std::shared_ptr<mdox_vector> & a, bool&)->bool {if (a->vector.size() <= 0) return false; return a->vector.back().igualdad_Condicional(*v); },
			[&](std::shared_ptr<mdox_vector> & a, double&)->bool {if (a->vector.size() <= 0) return false; return a->vector.back().igualdad_Condicional(*v); },
			[&](std::shared_ptr<mdox_vector> & a, long long&)->bool {if (a->vector.size() <= 0) return false; return a->vector.back().igualdad_Condicional(*v); },

			[&](int&, std::shared_ptr<mdox_vector> & a)->bool { if (a->vector.size() <= 0) return false; return a->vector.front().igualdad_Condicional(*_this); },
			[&](std::string&, std::shared_ptr<mdox_vector> & a)->bool { if (a->vector.size() <=  0) return false; return a->vector.front().igualdad_Condicional(*_this); },
			[&](bool&, std::shared_ptr<mdox_vector> & a)->bool { if (a->vector.size() <= 0) return false; return a->vector.front().igualdad_Condicional(*_this); },
			[&](double&, std::shared_ptr<mdox_vector> & a)->bool { if (a->vector.size() <= 0) return false; return a->vector.front().igualdad_Condicional(*_this); },
			[&](long long&, std::shared_ptr<mdox_vector> & a)->bool { if (a->vector.size() <= 0) return false; return a->vector.front().igualdad_Condicional(*_this); },
			//[&](std::shared_ptr<mdox_vector>&, std::shared_ptr<mdox_vector>& a) {  return a->vector.front().igualdad_Condicional(*this); },

			[](auto&,auto&)->bool { Errores::generarError(Errores::ERROR_OPERADOR_INVALIDO,  Errores::outData, "::"); return false; },
			}, _this->value, v->value);
	}
	else
	{
		return std::visit(overloaded{

			[&](std::shared_ptr<mdox_vector> & a, std::monostate)->bool 
			{ 
				int f_index = (a->vector.size() - index) - 1;
				if (a->vector.size() > f_index)
				{  
					if (tipo_2 != PARAM_VOID)
					{
						v->inicializacion(tipo_2);
						v->asignacion(a->vector[f_index], true);
					}
					else
					{
						v->asignacion(a->vector[f_index], false);
					}
					return true; 
				} 
				else return false; 
			},
			[&](std::monostate, std::shared_ptr<mdox_vector> & a)->bool 
			{ 
				if (a->vector.size() > index) 
				{ 
					if (tipo_1 != PARAM_VOID)
					{
						_this->inicializacion(tipo_1);
						_this->asignacion(a->vector[index], true);
					}
					else
					{
						_this->asignacion(a->vector[index], false);
					}
					return true; 
				}
				else return false; 
			},

		//	[&](std::shared_ptr<mdox_vector> & a, auto&)->Value { return a->vector.back().igualdad_Condicional(v); },
		//	[&](auto&, std::shared_ptr<mdox_vector>& a)->Value {  return a->vector.front().igualdad_Condicional(*this); },

			[&](std::shared_ptr<mdox_vector> & a, std::shared_ptr<mdox_vector>&)->bool { int f_index = (a->vector.size() - index) - 1;  if (a->vector.size() <= f_index) return false; return a->vector[f_index].igualdad_Condicional(*v); },
			[&](std::shared_ptr<mdox_vector> & a, int&)->bool {int f_index = (a->vector.size() - index) -1; if (a->vector.size() <= f_index) return false; return a->vector[f_index].igualdad_Condicional(*v); },
			[&](std::shared_ptr<mdox_vector> & a, std::string&)->bool {int f_index = (a->vector.size() - index) - 1; if (a->vector.size() <= f_index) return false; return a->vector[f_index].igualdad_Condicional(*v); },
			[&](std::shared_ptr<mdox_vector> & a, bool&)->bool {int f_index = (a->vector.size() - index) - 1; if (a->vector.size() <= f_index) return false; return a->vector[f_index].igualdad_Condicional(*v); },
			[&](std::shared_ptr<mdox_vector> & a, double&)->bool {int f_index = (a->vector.size() - index) - 1; if (a->vector.size() <= f_index) return false; return a->vector[f_index].igualdad_Condicional(*v); },
			[&](std::shared_ptr<mdox_vector> & a, long long&)->bool {int f_index = (a->vector.size() - index) - 1; if (a->vector.size() <= f_index) return false; return a->vector[index].igualdad_Condicional(*v); },

			[&](int&, std::shared_ptr<mdox_vector> & a)->bool { if (a->vector.size() <= index) return false; return a->vector[index].igualdad_Condicional(*_this); },
			[&](std::string&, std::shared_ptr<mdox_vector> & a)->bool { if (a->vector.size() <= index) return false; return a->vector[index].igualdad_Condicional(*_this); },
			[&](bool&, std::shared_ptr<mdox_vector> & a)->bool { if (a->vector.size() <= index) return false; return a->vector[index].igualdad_Condicional(*_this); },
			[&](double&, std::shared_ptr<mdox_vector> & a)->bool { if (a->vector.size() <= index) return false; return a->vector[index].igualdad_Condicional(*_this); },
			[&](long long&, std::shared_ptr<mdox_vector> & a)->bool { if (a->vector.size() <= index) return false; return a->vector[index].igualdad_Condicional(*_this); },
			//[&](std::shared_ptr<mdox_vector>&, std::shared_ptr<mdox_vector>& a) {  return a->vector.front().igualdad_Condicional(*this); },

			[](auto&,auto&)->bool { Errores::generarError(Errores::ERROR_OPERADOR_INVALIDO,  Errores::outData, "::"); return false; },
			}, _this->value, v->value);
	}
}

short int Value::OperadoresEspeciales_Pop(Value* v, bool& left)
{
	tipos_parametros tipo_1 = PARAM_VOID, tipo_2 = PARAM_VOID;

	Value* _this = &(*this);
	Interprete::instance->getRealValueFromValueWrapperRef(&_this, &tipo_1);
	Interprete::instance->getRealValueFromValueWrapperRef(&v, &tipo_2);

	return std::visit(overloaded{
		[&](std::shared_ptr<mdox_vector> & a, std::monostate)->short int
		{
			left = true;
			if (a->vector.size() == 0) return 2;
			if (tipo_2 != PARAM_VOID)
			{
				v->inicializacion(tipo_2);
				v->asignacion(a->vector.back(), true);
				a->vector.pop_back();
				return true;
			}
			v->asignacion(a->vector.back(), false);
			a->vector.pop_back();
			return true;
		},
		[&](std::monostate, std::shared_ptr<mdox_vector> & a)->short int
		{ 
			if (a->vector.size() == 0) return 2;
			if (tipo_1 != PARAM_VOID)
			{
				_this->inicializacion(tipo_1);
				_this->asignacion(a->vector.front(),true);
				a->vector.erase(a->vector.begin());
				return true;
			}

			_this->asignacion(a->vector.front(), false);
			a->vector.erase(a->vector.begin());
			return true;
		}, //NO recomendado su uso.

		//[&](std::shared_ptr<mdox_vector>& a, auto&) {  a->vector.emplace_back(v); return v; },
		[&](std::shared_ptr<mdox_vector> & a, int&)->short int { a->vector.emplace_back(*v); left = true; return true; },
		[&](std::shared_ptr<mdox_vector> & a, std::string&)->short int {  a->vector.emplace_back(*v); left = true; return true; },
		[&](std::shared_ptr<mdox_vector> & a, bool&)->short int {  a->vector.emplace_back(*v); left = true; return true; },
		[&](std::shared_ptr<mdox_vector> & a, long long&)->short int { a->vector.emplace_back(*v); left = true; return true; },
		[&](std::shared_ptr<mdox_vector> & a, double&)->short int {  a->vector.emplace_back(*v); left = true; return true; },
		[&](std::shared_ptr<mdox_vector> & a, std::shared_ptr<mdox_vector>&)->short int {a->vector.emplace_back(*v); left = true; return true; },

		//[&](auto&, std::shared_ptr<mdox_vector>& a) {  a->vector.emplace_back(*this); std::rotate(a->vector.rbegin(), a->vector.rbegin() + 1, a->vector.rend()); return *this; },
		[&](std::string&, std::shared_ptr<mdox_vector> & a)->short int {a->vector.emplace_front(*_this); return true; },
		[&](int&, std::shared_ptr<mdox_vector> & a)->short int { a->vector.emplace_front(*_this); return true; },
		[&](double&, std::shared_ptr<mdox_vector> & a)->short int { a->vector.emplace_front(*_this); return true; },
		[&](long long&, std::shared_ptr<mdox_vector> & a)->short int { a->vector.emplace_front(*_this); return true; },
		[&](bool&, std::shared_ptr<mdox_vector> & a)->short int { a->vector.emplace_front(*_this); return true; },
		//[&](std::shared_ptr<mdox_vector>& a, std::shared_ptr<mdox_vector>& b) {  a->vector.emplace_back(*this); std::rotate(a->vector.rbegin(), a->vector.rbegin() + 1, a->vector.rend()); return *this; },


		[](auto,auto)->short int { Errores::generarError(Errores::ERROR_OPERADOR_INVALIDO,  Errores::outData, ":"); return false; },
		}, _this->value, v->value);
	
}



bool Value::OperacionRelacional(Value & v, const OPERADORES op)
{
	Interprete::instance->getRealValueFromValueWrapper(*this);
	Interprete::instance->getRealValueFromValueWrapper(v);

	switch (op)
	{

		// --------------------------------------------------------------------
		// -------------------- OPERACIÓNES CONDICIONALES ---------------------
		// --------------------------------------------------------------------

	case OPERADORES::OP_REL_EQUAL:
	{
		return this->igualdad_Condicional(v);
	}

	case OPERADORES::OP_REL_NOT_EQUAL:
	{
		return !this->igualdad_Condicional(v);
	}

	case OPERADORES::OP_REL_MINOR:
	{
		return this->menorQue_Condicional(v);
	}

	case OPERADORES::OP_REL_MAJOR:
	{
		return this->mayorQue_Condicional(v);
	}

	case OPERADORES::OP_REL_MAJOR_OR_EQUAL:
	{
		return this->mayorIgualQue_Condicional(v);
	}

	case OPERADORES::OP_REL_MINOR_OR_EQUAL:
	{
		return this->menorIgualQue_Condicional(v);
	}
	}
	return false;
}

void Value::inicializacion(Parser_Declarativo * tipo)
{
	inicializacion(tipo->value);
}

void Value::inicializacion(tipos_parametros tipo)
{
	Value* _this = &(*this);
	Interprete::instance->getRealValueFromValueWrapperRef(&_this);

	switch (tipo)
	{
	case tipos_parametros::PARAM_INT: _this->value = (int)0; break;
	case tipos_parametros::PARAM_LINT: _this->value = (long long)0; break;
	case tipos_parametros::PARAM_DOUBLE: _this->value = (double)0; break;
	case tipos_parametros::PARAM_BOOL: _this->value = (bool)false; break;
	case tipos_parametros::PARAM_STRING: _this->value = (std::string)""; break;
	case tipos_parametros::PARAM_VECTOR: _this->value = std::make_shared<mdox_vector>(); break;
	default: _this->value = std::monostate(); break;
	}
}

bool Value::asignacion(Value & v, bool fuerte)
{
	tipos_parametros tipo = PARAM_NULL;
	Value* _this = &(*this);
	Interprete::instance->getRealValueFromValueWrapperRef(&_this, &tipo);
	Interprete::instance->getRealValueFromValueWrapper(v);

	if (tipo != PARAM_NULL)
	{
		if (tipo != PARAM_VOID)
			fuerte = true;
		else fuerte = false;
	}

	if (!fuerte)
	{
		//TODO PERFORMANCE
		*_this = Value(v.value);
		return true;
	}
	else
	{
		return std::visit(overloaded{

			//INTEGER .. XX
			[&](int&, int&) { *_this = v; return true; },
			[&](int&, bool& b) { *_this = Value((int)b); return true; },
			[&](int&, long long& b) { *_this = Value((int)b); return true;  },
			[&](int&, double& b) { *_this = Value((int)b); return true;  },
			[&](int&, std::string & b)
			{
				char* endptr = NULL;
				auto number = strtol(b.c_str(), &endptr, 10);
				if (b.c_str() == endptr)
				{
					Errores::generarError(Errores::ERROR_CONVERSION_VARIABLE_INT, Errores::outData);
					return false;
				}
				*_this = Value((int)number);
				return true;
			},
			[&](int&, std::monostate&) { Errores::generarError(Errores::ERROR_ASIGNACION_VALOR_VOID, Errores::outData, "int"); return false; },


			[&](long long&, long long& b) { *_this = v;  return true;  },
			[&](long long&,  bool& b) { *_this = Value((long long)b);  return true; },
			[&](long long&,  int& b) { *_this = Value((long long)b); return true; },
			[&](long long&,  double& b) { *_this = Value((int)b); return true; },
			[&](long long&,  std::string & b)
			{
				char* endptr = NULL;
				long long number = strtoll(b.c_str(), &endptr, 10);
				if (b.c_str() == endptr)
				{
					Errores::generarError(Errores::ERROR_CONVERSION_VARIABLE_LONG, Errores::outData);
					return false;
				}

				*_this = Value(number);
				return true;
			},
			[&](long long&, std::monostate&) { Errores::generarError(Errores::ERROR_ASIGNACION_VALOR_VOID, Errores::outData, "lint"); return false; },

			[&](double&,  double& b) { *_this = v;   return true; },
			[&](double&,  int& b) { *_this = Value((double)b);  return true;  },
			[&](double&,  long long& b) { *_this = Value((double)b);  return true;  },
			[&](double&,  bool& b) { *_this = Value((double)b); return true;  },
			[&](double&,  std::string & b)
			{
				char* endptr = NULL;
				double number = strtod(b.c_str(), &endptr);
				if (b.c_str() == endptr)
				{
					Errores::generarError(Errores::ERROR_CONVERSION_VARIABLE_REAL, Errores::outData);
					return false;
				}

				*_this = Value(number);
				return true;
			},
			[&](double&, std::monostate&) { Errores::generarError(Errores::ERROR_ASIGNACION_VALOR_VOID, Errores::outData, "double"); return false; },

			[&](bool&,  double& b) { *_this = Value((bool)b);   return true; },
			[&](bool&,  int& b) { *_this = Value((bool)b);  return true; },
			[&](bool&,  long long& b) { *_this = Value((bool)b); return true;  },
			[&](bool&,  bool& b) { *_this = v; return true;   },
			[&](bool&,  std::string & b)
			{
				if (b == "" || b == "0" || b == "false")
					* _this = Value(false);
				else
					*_this = Value(true);

				return true;
			},
			[&](bool&, std::monostate&) { Errores::generarError(Errores::ERROR_ASIGNACION_VALOR_VOID, Errores::outData, "bool"); return false; },

			[&](std::string&,  double& b) { *_this = Value(to_string_p(b));  return true;  },
			[&](std::string&,  int& b) { *_this = Value(to_string_p(b));  return true; },
			[&](std::string&,  long long& b) { *_this = Value(to_string_p(b));   return true; },
			[&](std::string&,  bool& b) { *_this = Value(to_string_p(b));  return true;  },
			[&](std::string&,  std::string & b) { *_this = v;   return true; },
			[&](std::string&, std::monostate&) { Errores::generarError(Errores::ERROR_ASIGNACION_VALOR_VOID, Errores::outData, "string"); return false; },

			[&](std::monostate&, auto&) { *_this = v;  return true; },


			[&](std::shared_ptr<mdox_vector>&, std::shared_ptr<mdox_vector> & b)
			{
				*_this = b;
				return true;
			},

			[&](auto & a, std::shared_ptr<mdox_object> & b)
			{
				if (b->clase->right_operators && b->clase->right_operators->OPERATOR_asignacion)
				{
					return Interprete::instance->ExecOperador(b->clase->right_operators->OPERATOR_asignacion, *_this, b->variables_clase).ValueToBool();
				}

				Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "=", b->clase->pID->nombre);
				return false;
			},

			[&](std::monostate&, std::shared_ptr<mdox_object>& b)
			{
				if (b->clase->right_operators && b->clase->right_operators->OPERATOR_asignacion)
				{
					return Interprete::instance->ExecOperador(b->clase->right_operators->OPERATOR_asignacion, *_this, b->variables_clase).ValueToBool();
				}

				Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "=", b->clase->pID->nombre);
				return false;
			},

			[&v](std::shared_ptr<mdox_object> & a, auto & b)
			{
				if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_asignacion)
				{
					return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_asignacion, v, a->variables_clase).ValueToBool();
				}

				Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "=", a->clase->pID->nombre);
				return false;
			},


			[&v](std::shared_ptr<mdox_object> & a, std::shared_ptr<mdox_object> & b)
			{
				if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_asignacion)
				{
					return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_asignacion, v, a->variables_clase).ValueToBool();
				}

				Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "=", a->clase->pID->nombre);
				return false;
			},


			[](auto&,auto&) { Errores::generarError(Errores::ERROR_CONVERSION_DESCONOCIDA, Errores::outData);  return false; },

			}, _this->value, v.value);
	}


}

bool Value::Cast(Parser_Declarativo * pDec)
{
	return Cast(pDec->value);
}

bool Value::Cast(const tipos_parametros tipo)
{
	Value* _this = &(*this);
	Interprete::instance->getRealValueFromValueWrapperRef(&_this);

	switch (tipo)
	{
	case PARAM_VOID:
	{
		return true;
	}
	case PARAM_INT:
	{
		return std::visit(overloaded{
			[&](std::string & a)
			{
				char* endptr = NULL;
				auto number = strtol(a.c_str(), &endptr, 10);
				if (a.c_str() == endptr)
				{
					Errores::generarError(Errores::ERROR_CONVERSION_VARIABLE_INT, Errores::outData);
					return false;
				}
				*_this = Value((int)number);
				return true;
			},
			[&](bool& a) { *_this = Value((int)a); return true; },
			[](int& a) { return true; },
			[](long long& a) { return true; },
			[&](double& a) { *_this = Value((int)a); return true; },
			[](auto&) { Errores::generarError(Errores::ERROR_CONVERSION_DESCONOCIDA, Errores::outData);  return false; },
			}, _this->value);
	}
	case PARAM_LINT:
	{
		return std::visit(overloaded{
			[&](std::string & a)
			{
				char* endptr = NULL;
				auto number = strtoll(a.c_str(), &endptr, 10);
				if (a.c_str() == endptr)
				{
					Errores::generarError(Errores::ERROR_CONVERSION_VARIABLE_LONG, Errores::outData);
					return false;
				}
				*_this = Value((long long)number);
				return true;
			},
			[&]( bool& a) { *_this = Value((long long)a); return true; },
			[]( int& a) { return true; },
			[]( long long& a) { return true; },
			[&]( double& a) { *_this = Value((long long)a); return true; },
			[](auto&) { Errores::generarError(Errores::ERROR_CONVERSION_DESCONOCIDA, Errores::outData);  return false; },
			}, _this->value);
	}

	case PARAM_BOOL:
	{
		return std::visit(overloaded{
			[&](std::string & b)
			{
				if (b == "" || b == "0" || b == "false")
					* _this = Value(false);
				else
					*_this = Value(true);

				return true;
			},
			[]( bool& a) { return true; },
			[&]( int& a) { *_this = Value((bool)a);  return true; },
			[&]( long long& a) { *_this = Value((bool)a);  return true; },
			[&]( double& a) { *_this = Value((bool)a); return true; },
			[](auto&) { Errores::generarError(Errores::ERROR_CONVERSION_DESCONOCIDA, Errores::outData); return false; },
			}, _this->value);
	}

	case PARAM_STRING:
	{
		return std::visit(overloaded{
			[](std::string & a) { return true; },
			[&]( bool& a) {  *_this = Value(to_string_p(a)); return true;  },
			[&]( int& a) {  *_this = Value(to_string_p(a));  return true; },
			[&]( long long& a) {  *_this = Value(to_string_p(a));  return true; },
			[&]( double& a) { *_this = Value(to_string_p(a)); return true; },
			[](auto&) { Errores::generarError(Errores::ERROR_CONVERSION_DESCONOCIDA, Errores::outData); return false; },
			}, _this->value);
	}

	case PARAM_DOUBLE:
	{
		return std::visit(overloaded{
			[&](std::string & b)
			{
				char* endptr = NULL;
				double number = strtod(b.c_str(), &endptr);
				if (b.c_str() == endptr)
				{
					Errores::generarError(Errores::ERROR_CONVERSION_VARIABLE_REAL, Errores::outData);
					return false;
				}

				*_this = Value(number);
				return true;
			},
			[&]( bool& a) {  *_this = Value(double(a)); return true; },
			[&]( int& a) {  *_this = Value(double(a)); return true; },
			[&]( long long& a) {  *_this = Value(double(a));  return true; },
			[]( double& a) { return true; },
			[](auto&) { Errores::generarError(Errores::ERROR_CONVERSION_DESCONOCIDA, Errores::outData);  return false; },
			}, _this->value);
	}
	case PARAM_VECTOR:
	{
		return std::visit(overloaded{
			[&](std::string & b) { *_this = Value(std::make_shared<mdox_vector>(std::deque<Value>{b})); return true; },
			[&](bool& a) {  *_this = Value(std::make_shared<mdox_vector>(std::deque<Value>{a}));  return true; },
			[&](int& a) {  *_this = Value(std::make_shared<mdox_vector>(std::deque<Value>{a})); return true; },
			[&](long long& a) {  *_this = Value(std::make_shared<mdox_vector>(std::deque<Value>{a}));  return true; },
			[&](double& a) { *_this = Value(std::make_shared<mdox_vector>(std::deque<Value>{a}));  return true; },
			[](std::shared_ptr<mdox_vector>&) { return true; },
			[](auto&) { Errores::generarError(Errores::ERROR_CONVERSION_DESCONOCIDA, Errores::outData);  return false; },
			}, _this->value);
	}

	default: return false;
	}
}

bool Value::ValueToBool()
{
	return std::visit(overloaded{
		[](std::string & a)
		{
			if (a == "" || a == "0" || a == "false")
				return false;
			else
				return true;
		},
		[](bool& a) { return a; },
		[](int& a) { return (bool)a; },
		[](long long& a) { return (bool)a; },
		[](double& a) { return (bool)a; },
		[](std::shared_ptr<mdox_vector> & a) { return !a->vector.empty(); },

		[](auto&) { return false; },
		}, value);
}

bool Value::igualdad_CondicionalFuncion(Value& v)
{
	Interprete::instance->getRealValueFromValueWrapper(*this);
	Interprete::instance->getRealValueFromValueWrapper(v);
	return this->igualdad_Condicional(v);
}

bool Value::igualdad_Condicional(Value & v)
{
	return std::visit(overloaded{

		//INTEGER .. XX
		[]( int& a, int& b) { return a == b; },
		[]( int& a, bool& b) { return a == b; },
		[]( int& a, long long& b) { return a == b; },
		[]( int& a, double& b) { return a == b; },
		[]( int& a, std::string & b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 int  number = strtol(b.c_str(), &endptr, 10);

			 if (errno == 0 && b.c_str() && !*endptr)
				 return a == number;
			 else
				 return false;
		},

		[]( long long& a,  long long& b) { return a == b; },
		[]( long long& a,   bool& b) { return a == b; },
		[]( long long& a,  int& b) { return a == b; },
		[]( long long& a,  double& b) { return a == b; },
		[]( long long& a,  std::string & b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 long long number = strtoll(b.c_str(), &endptr,10);

			 if (errno == 0 && b.c_str() && !*endptr)
				 return a == number;
			 else
				 return false;
		},

		[]( double& a,  double& b) { return a == b; },
		[]( double& a,  int& b) { return a == b; },
		[]( double& a,  long long& b) { return a == b; },
		[]( double& a,  bool& b) { return a == b; },
		[]( double& a,  std::string & b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 double number = strtod(b.c_str(), &endptr);

			 if (errno == 0 && b.c_str() && !*endptr)
				 return a == number;
			 else
				 return false;
		},

		[]( bool& a,  double& b) { return a == b;  },
		[]( bool& a,  int& b) { return a == b; },
		[]( bool& a,  long long& b) { return a == b; },
		[]( bool& a,  bool& b) { return a == b;  },
		[]( bool& a,  std::string & b)
		{
		   if (b == "" || b == "0" || b == "false")
				return (false == a);
			else
				return (true == a);
		},

		[](std::string & a,  double& b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 double number = strtod(a.c_str(), &endptr);

			 if (errno == 0 && a.c_str() && !*endptr)
				 return b == number;
			 else
				 return false;
		},
		[](std::string & a,  int& b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 int number = strtol(a.c_str(), &endptr, 10);

			 if (errno == 0 && a.c_str() && !*endptr)
				 return b == number;
			 else
				 return false;
		},
		[](std::string & a,  long long& b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 long long number = strtoll(a.c_str(), &endptr,10);

			 if (errno == 0 && a.c_str() && !*endptr)
				 return b == number;
			 else
				 return false;
		},
		[](std::string & a,  bool& b)
		{
		   if (a == "" || a == "0" || a == "false")
				return (false == b);
			else
				return (true == b);
		},
		[](std::string & a,  std::string & b) { return a == b; },

		[](std::shared_ptr<mdox_vector> & a,  std::shared_ptr<mdox_vector> & b) { return distance(a->vector.begin(), a->vector.end()) == distance(b->vector.begin(), b->vector.end()) && equal(a->vector.begin(), a->vector.end(), b->vector.begin()); },


		[&](auto& a, std::shared_ptr<mdox_object>& b)
		{
			if (b->clase->right_operators && b->clase->right_operators->OPERATOR_rel_igualdad)
			{
				return Interprete::instance->ExecOperador(b->clase->right_operators->OPERATOR_rel_igualdad, *this, b->variables_clase).ValueToBool();
			}

			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "==", b->clase->pID->nombre);
			return false;
		},

		[&v](std::shared_ptr<mdox_object>& a, auto& b)
		{
			if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_rel_igualdad)
			{
				return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_rel_igualdad, v, a->variables_clase).ValueToBool();
			}

			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "==", a->clase->pID->nombre);
			return false;
		},


		[&](std::shared_ptr<mdox_object>& a, std::shared_ptr<mdox_object>& b)
		{
			if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_rel_igualdad)
			{
				return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_rel_igualdad, v, a->variables_clase).ValueToBool();
			}
			if (b->clase->right_operators && b->clase->right_operators->OPERATOR_rel_igualdad)
			{
				return Interprete::instance->ExecOperador(b->clase->right_operators->OPERATOR_rel_igualdad, *this, b->variables_clase).ValueToBool();
			}
			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "==", a->clase->pID->nombre);
			return false;
		},

		[](auto&,auto&) { return false; },

		}, value, v.value);
}


bool Value::mayorQue_Condicional(Value & v)
{
	return std::visit(overloaded{

		//INTEGER .. XX
		[](const int& a,const int& b)
		{
			return a > b;
		},
		[](const int& a,const bool& b) { return a > b; },
		[](const int& a,const long long& b) { return a > b; },
		[](const int& a,const double& b) { return a > b; },
		[](const int& a, std::string & b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 int  number = strtol(b.c_str(), &endptr, 10);

			 if (errno == 0 && b.c_str() && !*endptr)
				 return a > number;
			 else
				 return false;
		},

		[](const long long& a, const long long& b) { return a > b; },
		[](const long long& a, const  bool& b) { return a > b; },
		[](const long long& a, const int& b) { return a > b; },
		[](const long long& a, const double& b) { return a > b; },
		[](const long long& a,  std::string & b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 long long number = strtoll(b.c_str(), &endptr,10);

			 if (errno == 0 && b.c_str() && !*endptr)
				 return a > number;
			 else
				 return false;
		},

		[](const double& a, const double& b) { return a > b; },
		[](const double& a, const int& b) { return a > b; },
		[](const double& a, const long long& b) { return a > b; },
		[](const double& a, const bool& b) { return a > b; },
		[](const double& a,  std::string & b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 double number = strtod(b.c_str(), &endptr);

			 if (errno == 0 && b.c_str() && !*endptr)
				 return a > number;
			 else
				 return false;
		},

		[](const bool& a, const double& b) { return a > b;  },
		[](const bool& a, const int& b) { return a > b; },
		[](const bool& a, const long long& b) { return a > b; },
		[](const bool& a, const bool& b) { return a > b;  },
		[](const bool& a,  std::string & b)
		{
		   if (b == "" || b == "0" || b == "false")
				return (false > a);
			else
				return (true > a);
		},

		[](std::string & a, const double& b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 double number = strtod(a.c_str(), &endptr);

			 if (errno == 0 && a.c_str() && !*endptr)
				 return b > number;
			 else
				 return false;
		},
		[](std::string & a, const int& b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 int number = strtol(a.c_str(), &endptr, 10);

			 if (errno == 0 && a.c_str() && !*endptr)
				 return b > number;
			 else
				 return false;
		},
		[](std::string & a, const long long& b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 long long number = strtoll(a.c_str(), &endptr,10);

			 if (errno == 0 && a.c_str() && !*endptr)
				 return b > number;
			 else
				 return false;
		},
		[](std::string & a, const bool& b)
		{
		   if (a == "" || a == "0" || a == "false")
				return (false > b);
			else
				return (true > b);
		},
		[](std::string & a,  std::string & b) { return a > b; },

		[](std::shared_ptr<mdox_vector> & a,  std::shared_ptr<mdox_vector> & b)
		{
			if ((a->vector.empty() && b->vector.empty()) || (a->vector.empty() && !b->vector.empty()))
				return false;

			if (b->vector.empty())
				return true;

			Value * v1 = &*a->vector.begin();
			Value * v1_end = &a->vector.back();

			Value * v2 = &*b->vector.begin();
			Value * v2_end = &b->vector.back();


			while (true)
			{
				if (v1 == v1_end)
				{
					if (*v1 == *v2)
					{
						if (v2 == v2_end)
						{
							return false; // Son iguales
						}
						else return false; //V2 tiene mas valores, ende es mayor
					}
					return v1->mayorQue_Condicional(*v2); //Dependera de que valor es mayor.
				}
				else if (v2 == v2_end)
				{
					if (*v1 == *v2)
					{
						if (v1 == v1_end)
						{
							return false; // Son iguales
						}
						else return true; //V1 tiene mas valores, ende es mayor
					}
					return v1->mayorQue_Condicional(*v2); //Dependera de que valor es mayor.
				}
				else
				{
					if (*v1 == *v2)
					{
						v1++; v2++;
						continue;
					}
					return v1->mayorQue_Condicional(*v2); //Dependera de que valor es mayor.
				}
			}
		},

		[&](auto& a, std::shared_ptr<mdox_object>& b)
		{
			if (b->clase->right_operators && b->clase->right_operators->OPERATOR_rel_mayor)
			{
				return Interprete::instance->ExecOperador(b->clase->right_operators->OPERATOR_rel_mayor, *this, b->variables_clase).ValueToBool();
			}

			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, ">", b->clase->pID->nombre);
			return false;
		},

		[&v](std::shared_ptr<mdox_object>& a, auto& b)
		{
			if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_rel_mayor)
			{
				return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_rel_mayor, v, a->variables_clase).ValueToBool();
			}

			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, ">", a->clase->pID->nombre);
			return false;
		},

		[&](std::shared_ptr<mdox_object>& a, std::shared_ptr<mdox_object>& b)
		{
			if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_rel_mayor)
			{
				return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_rel_mayor, v, a->variables_clase).ValueToBool();
			}
			if (b->clase->right_operators && b->clase->right_operators->OPERATOR_rel_mayor)
			{
				return Interprete::instance->ExecOperador(b->clase->right_operators->OPERATOR_rel_mayor, *this, b->variables_clase).ValueToBool();
			}
			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, ">", a->clase->pID->nombre);
			return false;
		},

		[](auto,auto) { return false; },

		}, value, v.value);
}

bool Value::menorQue_Condicional(Value & v)
{
	return std::visit(overloaded{

		//INTEGER .. XX
		[](const int& a,const int& b) { return a < b; },
		[](const int& a,const bool& b) { return a < b; },
		[](const int& a,const long long& b) { return a < b; },
		[](const int& a,const double& b) { return a < b; },
		[](const int& a, std::string & b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 int  number = strtol(b.c_str(), &endptr, 10);

			 if (errno == 0 && b.c_str() && !*endptr)
				 return a < number;
			 else
				 return false;
		},

		[](const long long& a, const long long& b) { return a < b; },
		[](const long long& a, const  bool& b) { return a < b; },
		[](const long long& a, const int& b) { return a < b; },
		[](const long long& a, const double& b) { return a < b; },
		[](const long long& a,  std::string & b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 long long number = strtoll(b.c_str(), &endptr,10);

			 if (errno == 0 && b.c_str() && !*endptr)
				 return a < number;
			 else
				 return false;
		},

		[](const double& a, const double& b) { return a < b; },
		[](const double& a, const int& b) { return a < b; },
		[](const double& a, const long long& b) { return a < b; },
		[](const double& a, const bool& b) { return a < b; },
		[](const double& a,  std::string & b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 double number = strtod(b.c_str(), &endptr);

			 if (errno == 0 && b.c_str() && !*endptr)
				 return a < number;
			 else
				 return false;
		},

		[](const bool& a, const double& b) { return a < b;  },
		[](const bool& a, const int& b) { return a < b; },
		[](const bool& a, const long long& b) { return a < b; },
		[](const bool& a, const bool& b) { return a < b;  },
		[](const bool& a,  std::string & b)
		{
		   if (b == "" || b == "0" || b == "false")
				return (false < a);
			else
				return (true < a);
		},

		[](std::string & a, const double& b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 double number = strtod(a.c_str(), &endptr);

			 if (errno == 0 && a.c_str() && !*endptr)
				 return b < number;
			 else
				 return false;
		},
		[](std::string & a, const int& b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 int number = strtol(a.c_str(), &endptr, 10);

			 if (errno == 0 && a.c_str() && !*endptr)
				 return b < number;
			 else
				 return false;
		},
		[](std::string & a, const long long& b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 long long number = strtoll(a.c_str(), &endptr,10);

			 if (errno == 0 && a.c_str() && !*endptr)
				 return b < number;
			 else
				 return false;
		},
		[](std::string & a, const bool& b)
		{
		   if (a == "" || a == "0" || a == "false")
				return (false < b);
			else
				return (true < b);
		},
		[](std::string & a,  std::string & b) { return a < b; },

		[](std::shared_ptr<mdox_vector> & a,  std::shared_ptr<mdox_vector> & b)
		{
			if ((a->vector.empty() && b->vector.empty()) || (!a->vector.empty() && b->vector.empty()))
				return false;

			if (a->vector.empty())
				return true;

			Value * v1 = &*a->vector.begin();
			Value * v1_end = &a->vector.back();

			Value * v2 = &*b->vector.begin();
			Value * v2_end = &b->vector.back();

			while (true)
			{
				if (v1 == v1_end)
				{
					if (*v1 == *v2)
					{
						if (v2 == v2_end)
						{
							return false; // Son iguales
						}
						else return true; //V2 tiene mas valores, ende es mayor
					}
					return v1->menorQue_Condicional(*v2); //Dependera de que valor es mayor.
				}
				else if (v2 == v2_end)
				{
					if (*v1 == *v2)
					{
						if (v1 == v1_end)
						{
							return false; // Son iguales
						}
						else return false; //V1 tiene mas valores, ende es mayor
					}
					return v1->menorQue_Condicional(*v2); //Dependera de que valor es mayor.
				}
				else
				{
					if (*v1 == *v2)
					{
						v1++; v2++;
						continue;
					}
					return v1->menorQue_Condicional(*v2); //Dependera de que valor es mayor.
				}
			}

		},

		[&](auto& a, std::shared_ptr<mdox_object>& b)
		{
			if (b->clase->right_operators && b->clase->right_operators->OPERATOR_rel_menor)
			{
				return Interprete::instance->ExecOperador(b->clase->right_operators->OPERATOR_rel_menor, *this, b->variables_clase).ValueToBool();
			}

			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "<", b->clase->pID->nombre);
			return false;
		},

		[&v](std::shared_ptr<mdox_object>& a, auto& b)
		{
			if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_rel_menor)
			{
				return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_rel_menor, v, a->variables_clase).ValueToBool();
			}

			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "<", a->clase->pID->nombre);
			return false;
		},

		[&](std::shared_ptr<mdox_object>& a, std::shared_ptr<mdox_object>& b)
		{
			if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_rel_menor)
			{
				return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_rel_menor, v, a->variables_clase).ValueToBool();
			}
			if (b->clase->right_operators && b->clase->right_operators->OPERATOR_rel_menor)
			{
				return Interprete::instance->ExecOperador(b->clase->right_operators->OPERATOR_rel_menor, *this, b->variables_clase).ValueToBool();
			}
			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "<", a->clase->pID->nombre);
			return false;
		},

		[](auto,auto) { return false; },

		}, value, v.value);
}

bool Value::menorIgualQue_Condicional(Value & v)
{
	return std::visit(overloaded{

		//INTEGER .. XX
		[](const int& a,const int& b) { return a <= b; },
		[](const int& a,const bool& b) { return a <= b; },
		[](const int& a,const long long& b) { return a <= b; },
		[](const int& a,const double& b) { return a <= b; },
		[](const int& a, std::string & b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 int  number = strtol(b.c_str(), &endptr, 10);

			 if (errno == 0 && b.c_str() && !*endptr)
				 return a <= number;
			 else
				 return false;
		},

		[](const long long& a, const long long& b) { return a <= b; },
		[](const long long& a, const  bool& b) { return a <= b; },
		[](const long long& a, const int& b) { return a <= b; },
		[](const long long& a, const double& b) { return a <= b; },
		[](const long long& a,  std::string & b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 long long number = strtoll(b.c_str(), &endptr,10);

			 if (errno == 0 && b.c_str() && !*endptr)
				 return a <= number;
			 else
				 return false;
		},

		[](const double& a, const double& b) { return a <= b; },
		[](const double& a, const int& b) { return a <= b; },
		[](const double& a, const long long& b) { return a <= b; },
		[](const double& a, const bool& b) { return a <= b; },
		[](const double& a,  std::string & b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 double number = strtod(b.c_str(), &endptr);

			 if (errno == 0 && b.c_str() && !*endptr)
				 return a <= number;
			 else
				 return false;
		},

		[](const bool& a, const double& b) { return a <= b;  },
		[](const bool& a, const int& b) { return a <= b; },
		[](const bool& a, const long long& b) { return a <= b; },
		[](const bool& a, const bool& b) { return a <= b;  },
		[](const bool& a,  std::string & b)
		{
		   if (b == "" || b == "0" || b == "false")
				return (false <= a);
			else
				return (true <= a);
		},

		[]( std::string & a, const double& b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 double number = strtod(a.c_str(), &endptr);

			 if (errno == 0 && a.c_str() && !*endptr)
				 return b <= number;
			 else
				 return false;
		},
		[](std::string & a, const int& b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 int number = strtol(a.c_str(), &endptr, 10);

			 if (errno == 0 && a.c_str() && !*endptr)
				 return b <= number;
			 else
				 return false;
		},
		[](std::string & a, const long long& b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 long long number = strtoll(a.c_str(), &endptr,10);

			 if (errno == 0 && a.c_str() && !*endptr)
				 return b <= number;
			 else
				 return false;
		},
		[](std::string & a, const bool& b)
		{
		   if (a == "" || a == "0" || a == "false")
				return (false <= b);
			else
				return (true <= b);
		},
		[](std::string & a,  std::string & b) { return a <= b; },

		[](std::shared_ptr<mdox_vector> & a,  std::shared_ptr<mdox_vector> & b)
		{
			if ((!a->vector.empty() && b->vector.empty()))
				return false;

			if (a->vector.empty() || (a->vector.empty() && b->vector.empty()))
				return true;

			Value * v1 = &*a->vector.begin();
			Value * v1_end = &a->vector.back();

			Value * v2 = &*b->vector.begin();
			Value * v2_end = &b->vector.back();

			while (true)
			{
				if (v1 == v1_end)
				{
					if (*v1 == *v2)
					{
						if (v2 == v2_end)
						{
							return true; // Son iguales
						}
						else return true; //V2 tiene mas valores, ende es mayor
					}
					return v1->menorIgualQue_Condicional(*v2); //Dependera de que valor es mayor.
				}
				else if (v2 == v2_end)
				{
					if (*v1 == *v2)
					{
						if (v1 == v1_end)
						{
							return true; // Son iguales
						}
						else return false; //V1 tiene mas valores, ende es mayor
					}
					return v1->menorIgualQue_Condicional(*v2); //Dependera de que valor es mayor.
				}
				else
				{
					if (*v1 == *v2)
					{
						v1++; v2++;
						continue;
					}
					return v1->menorIgualQue_Condicional(*v2); //Dependera de que valor es mayor.
				}
			}
		},
		[&](auto& a, std::shared_ptr<mdox_object>& b)
		{
			if (b->clase->right_operators && b->clase->right_operators->OPERATOR_rel_menor_igual)
			{
				return Interprete::instance->ExecOperador(b->clase->right_operators->OPERATOR_rel_menor_igual, *this, b->variables_clase).ValueToBool();
			}

			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "<=", b->clase->pID->nombre);
			return false;
		},

		[&v](std::shared_ptr<mdox_object>& a, auto& b)
		{
			if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_rel_menor_igual)
			{
				return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_rel_menor_igual, v, a->variables_clase).ValueToBool();
			}

			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "<=", a->clase->pID->nombre);
			return false;
		},

		[&](std::shared_ptr<mdox_object>& a, std::shared_ptr<mdox_object>& b)
		{
			if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_rel_menor_igual)
			{
				return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_rel_menor_igual, v, a->variables_clase).ValueToBool();
			}
			if (b->clase->right_operators && b->clase->right_operators->OPERATOR_rel_menor_igual)
			{
				return Interprete::instance->ExecOperador(b->clase->right_operators->OPERATOR_rel_menor_igual, *this, b->variables_clase).ValueToBool();
			}
			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "<=", a->clase->pID->nombre);
			return false;
		},


		[](auto,auto) { return false; },

		}, value, v.value);
}

bool Value::mayorIgualQue_Condicional(Value & v)
{
	return std::visit(overloaded{

		//INTEGER .. XX
		[](const int& a,const int& b) { return a >= b; },
		[](const int& a,const bool& b) { return a >= b; },
		[](const int& a,const long long& b) { return a >= b; },
		[](const int& a,const double& b) { return a >= b; },
		[](const int& a, std::string & b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 int  number = strtol(b.c_str(), &endptr, 10);

			 if (errno == 0 && b.c_str() && !*endptr)
				 return a >= number;
			 else
				 return false;
		},

		[](const long long& a, const long long& b) { return a >= b; },
		[](const long long& a, const  bool& b) { return a >= b; },
		[](const long long& a, const int& b) { return a >= b; },
		[](const long long& a, const double& b) { return a >= b; },
		[](const long long& a,  std::string & b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 long long number = strtoll(b.c_str(), &endptr,10);

			 if (errno == 0 && b.c_str() && !*endptr)
				 return a >= number;
			 else
				 return false;
		},

		[](const double& a, const double& b) { return a >= b; },
		[](const double& a, const int& b) { return a >= b; },
		[](const double& a, const long long& b) { return a >= b; },
		[](const double& a, const bool& b) { return a >= b; },
		[](const double& a,  std::string & b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 double number = strtod(b.c_str(), &endptr);

			 if (errno == 0 && b.c_str() && !*endptr)
				 return a >= number;
			 else
				 return false;
		},

		[](const bool& a, const double& b) { return a >= b;  },
		[](const bool& a, const int& b) { return a >= b; },
		[](const bool& a, const long long& b) { return a >= b; },
		[](const bool& a, const bool& b) { return a >= b;  },
		[](const bool& a,  std::string & b)
		{
		   if (b == "" || b == "0" || b == "false")
				return (false >= a);
			else
				return (true >= a);
		},

		[](std::string & a, const double& b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 double number = strtod(a.c_str(), &endptr);

			 if (errno == 0 && a.c_str() && !*endptr)
				 return b >= number;
			 else
				 return false;
		},
		[](std::string & a, const int& b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 int number = strtol(a.c_str(), &endptr, 10);

			 if (errno == 0 && a.c_str() && !*endptr)
				 return b >= number;
			 else
				 return false;
		},
		[](std::string & a, const long long& b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 long long number = strtoll(a.c_str(), &endptr,10);

			 if (errno == 0 && a.c_str() && !*endptr)
				 return b >= number;
			 else
				 return false;
		},
		[](std::string & a, const bool& b)
		{
		   if (a == "" || a == "0" || a == "false")
				return (false >= b);
			else
				return (true >= b);
		},
		[](std::string & a,  std::string & b) { return a >= b; },

		[](std::shared_ptr<mdox_vector> & a,  std::shared_ptr<mdox_vector> & b)
		{
			if (a->vector.empty() && !b->vector.empty())
				return false;

			if (b->vector.empty() || (a->vector.empty() && b->vector.empty()))
				return true;

			Value * v1 = &*a->vector.begin();
			Value * v1_end = &a->vector.back();

			Value * v2 = &*b->vector.begin();
			Value * v2_end = &b->vector.back();


			while (true)
			{
				if (v1 == v1_end)
				{
					if (*v1 == *v2)
					{
						if (v2 == v2_end)
						{
							return true; // Son iguales
						}
						else return false; //V2 tiene mas valores, ende es mayor
					}
					return v1->mayorIgualQue_Condicional(*v2); //Dependera de que valor es mayor.
				}
				else if (v2 == v2_end)
				{
					if (*v1 == *v2)
					{
						if (v1 == v1_end)
						{
							return true; // Son iguales
						}
						else return true; //V1 tiene mas valores, ende es mayor
					}
					return v1->mayorIgualQue_Condicional(*v2); //Dependera de que valor es mayor.
				}
				else
				{
					if (*v1 == *v2)
					{
						v1++; v2++;
						continue;
					}
					return v1->mayorIgualQue_Condicional(*v2); //Dependera de que valor es mayor.
				}
			}
		},
		[&](auto& a, std::shared_ptr<mdox_object>& b)
		{
			if (b->clase->right_operators && b->clase->right_operators->OPERATOR_rel_mayor_igual)
			{
				return Interprete::instance->ExecOperador(b->clase->right_operators->OPERATOR_rel_mayor_igual, *this, b->variables_clase).ValueToBool();
			}

			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, ">=", b->clase->pID->nombre);
			return false;
		},

		[&v](std::shared_ptr<mdox_object>& a, auto& b)
		{
			if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_rel_mayor_igual)
			{
				return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_rel_mayor_igual, v, a->variables_clase).ValueToBool();
			}

			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, ">=", a->clase->pID->nombre);
			return false;
		},

		[&](std::shared_ptr<mdox_object>& a, std::shared_ptr<mdox_object>& b)
		{
			if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_rel_mayor_igual)
			{
				return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_rel_mayor_igual, v, a->variables_clase).ValueToBool();
			}
			if (b->clase->right_operators && b->clase->right_operators->OPERATOR_rel_mayor_igual)
			{
				return Interprete::instance->ExecOperador(b->clase->right_operators->OPERATOR_rel_mayor_igual, *this, b->variables_clase).ValueToBool();
			}
			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, ">=", a->clase->pID->nombre);
			return false;
		},

		[](auto,auto) { return false; },

		}, value, v.value);
}

bool Value::operacion_Asignacion(Value & v, OPERADORES & op, bool fuerte)
{
	switch (op)
	{
	case OPERADORES::OP_IG_EQUAL:
	{
		return this->asignacion(v, fuerte);
		break;
	}

	case OPERADORES::OP_IG_EQUAL_SUM:
	{
		return this->asignacion(Suma(*this,v), fuerte);
		break;
	}

	case OPERADORES::OP_IG_EQUAL_MIN:
	{
		return this->asignacion(Resta(*this,v), fuerte);
		break;
	}

	case OPERADORES::OP_IG_EQUAL_MULT:
	{
		return this->asignacion(Multiplicacion(*this, v), fuerte);
		break;
	}

	case OPERADORES::OP_IG_EQUAL_DIV:
	{
		return this->asignacion(Div(*this, v), fuerte);
		break;
	}

	case OPERADORES::OP_IG_EQUAL_MOD:
	{
		return this->asignacion(Mod(*this, v), fuerte);
		break;
	}
	}
	return false;
}

Value Value::operacion_Unitaria(OPERADORES & op)
{
	Interprete::instance->getRealValueFromValueWrapper(*this);
	switch (op)
	{
	case OP_COPY:
	{
		return std::visit(overloaded{
		[](std::shared_ptr<mdox_object> & a)->Value
		{
			return a->createCopy();
		},
		[](std::shared_ptr<mdox_vector> & a)->Value
		{
			return std::make_shared<mdox_vector>(*a);
		},
		[&](auto&)->Value { return value; },
			}, value);
	}
	case OP_IN_COPY:
	{
		return std::visit(overloaded{
		[](std::shared_ptr<mdox_object> & a)->Value
		{
			return a->createCopyIn();
		},
		[](std::shared_ptr<mdox_vector> & a)->Value
		{
			return a->createCopyIn();
		},
		[&](auto&)->Value { return value; },
			}, value);
	}
	case OP_NEGADO:
	{
		return std::visit(overloaded{
		[](std::shared_ptr<mdox_object> & a)->Value
		{
			if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_negado)
			{
				return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_negado, a->variables_clase);
			}

			Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "!", a->clase->pID->nombre);
			return false;
		},
		[&]( auto&)->Value { return !this->ValueToBool(); },
			}, value);
	}
	case ELEM_NEG_FIRST:
	{
		return std::visit(overloaded{
		[](int& val)->Value {return -val; },
		[](double& val)->Value {return -val; },
		[](long long& val)->Value {return -val; },
		[](bool& val)->Value {return -val; },
		[&](std::string & val)->Value {Errores::generarError(Errores::ERROR_MATH_STRING, Errores::outData, "-"); return std::monostate(); },
		[&](std::shared_ptr<mdox_vector> & a)->Value
		{
			std::shared_ptr<mdox_vector> r = std::make_shared<mdox_vector>();
			r->vector.resize(a->vector.size());
			std::transform(a->vector.begin(), a->vector.end(), r->vector.begin(), [](Value & x) { OPERADORES o = ELEM_NEG_FIRST; return x.operacion_Unitaria(o); });
			return r;
		},
		[](std::shared_ptr<mdox_object>& a)->Value
			{
				if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_negado_first)
				{
					return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_negado_first, a->variables_clase);
				}

				Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "-", a->clase->pID->nombre);
				return false;
			},
		[]( auto&)->Value {return std::monostate(); },
			},value);
	}
	case OPERADORES::OP_ITR_MIN:
	{
		return std::visit(overloaded{
			[](int& val)->Value {return val - 1; },
			[](double& val)->Value {return val - 1; },
			[](long long& val)->Value {return val - 1; },
			[](bool& val)->Value {return val - 1; },
			[](std::string & val)->Value {Errores::generarError(Errores::ERROR_MATH_STRING, Errores::outData, "--"); return std::monostate(); },
			[](std::shared_ptr<mdox_object>& a)->Value 
			{ 
				if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_min)
				{
					return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_min, a->variables_clase);
				}

				Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "--", a->clase->pID->nombre);
				return false;
			},
			[](auto&)->Value {Errores::generarError(Errores::ERROR_OPERADOR_INVALIDO, Errores::outData, "--");  return std::monostate(); },
			
			}, value);
	}

	case OPERADORES::OP_ITR_PLUS:
	{
		return std::visit(overloaded{
			[]( int& val)->Value {return val + 1; },
			[]( double& val)->Value {return val + 1; },
			[]( long long& val)->Value {return val + 1; },
			[]( bool& val)->Value {return val + 1; },
			[](std::shared_ptr<mdox_object> & a)->Value
			{
				if (a->clase->normal_operators && a->clase->normal_operators->OPERATOR_plus)
				{
					return Interprete::instance->ExecOperador(a->clase->normal_operators->OPERATOR_plus, a->variables_clase);
				}

				Errores::generarError(Errores::ERROR_CLASE_OPERADOR_NO_DECLARADO, NULL, "++", a->clase->pID->nombre);
				return false;
			},
			[]( std::string & val)->Value {Errores::generarError(Errores::ERROR_MATH_STRING, Errores::outData, "++"); return std::monostate(); },
			[]( auto&)->Value {Errores::generarError(Errores::ERROR_OPERADOR_INVALIDO, Errores::outData, "++");  return std::monostate(); },
			}, value);
	}

	}
	return std::monostate();
}

// --------------------------------------------------------------
// --------------------------  strict  --------------------------
// --------------------------------------------------------------

//reservado para elementos estrictos.
// Ejemplo:   void funcionPrueba(strict int a, int b);
// para poder acceder a la función se debe cumplir que:
// a deberá ser entero, y sólamente entero.
// b podrá ser otro valor transformado, por ejemplo: un booleano true, devolverá b = 1 o un double 5.32 devolverá b = 5.
// inclusive un string con un valor numérico como "743j", dará como resultado b=743
void Value::print()
{
	Interprete::instance->getRealValueFromValueWrapper(*this);

	std::visit(overloaded{
		[&](std::monostate) { std::cout << "<void>"; },
		[&](std::shared_ptr<mdox_vector> & a) {
			if (a->vector.empty())
			{
				std::cout << "[]";
				return;
			}
			std::cout << "[";
			std::for_each(a->vector.begin(), a->vector.end(), [](Value & x) { x.print(); std::cout << ", "; });
			std::cout << "\b\b]";
			},
		[&](auto & a) { std::cout << a; },
		}, value);
}