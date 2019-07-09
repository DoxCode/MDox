
#include "errores.h"


template <typename T>
std::string Value::to_string_p(const T a_value, const int n)
{
	std::ostringstream out;
	out.precision(n);
	out << std::fixed << a_value;

	//return out.str();
	//Borra ceros sobrantes en caso de un double: 5.43 sería = 5.4300000000 -> Lo transforma en 5.43
	if(std::is_same<T, double>::value) 
	{ 
		std::string s2 = out.str().erase(out.str().find_last_not_of('0') + 1, std::string::npos);
		return s2.erase(s2.find_last_not_of('.') + 1, std::string::npos); 
	} else return out.str();
}

Value  Value::operacion_Binaria(Value& v, const OPERADORES op)
{
	switch (op)
	{
		// --------------------------------------------------------------------
		// ---------------------- OPERACIÓN SUMA ------------------------------
		// --------------------------------------------------------------------
	case OP_ARIT_SUMA:
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
			[](const int& a, const long long& b)->Value { return (a + b); },

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
					[](const std::string & a,const int& b)->Value { return (a + to_string_p(b)); },
					[](const std::string & a,const bool& b)->Value { return (a + to_string_p(b)); },
					[](const std::string & a,const double& b)->Value { return (a + to_string_p(b)); },
					[](const std::string & a,const std::string & b)->Value { return (a + b); },
					[](const std::string & a,const long long& b)->Value { return (a + to_string_p(b)); },

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
				[](const bool& a, const std::string & b)->Value { return (to_string_p(a) + b); },
				[](const bool& a, const long long& b)->Value { return (a + b); },

				[](auto,auto)->Value { return std::monostate(); },

			}, value, v.value);
		break;
	}

	// --------------------------------------------------------------------
	// ---------------------- OPERACIÓN RESTA -----------------------------
	// --------------------------------------------------------------------
	case OP_ARIT_RESTA:
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
					[]( std::string & a,const double& b)->Value { Errores::generarError(Errores::ERROR_MATH_MINUS_STRING, Errores::outData, "double"); return std::monostate(); },
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
						[](auto,auto)->Value { return std::monostate(); },
			}, value, v.value);
		break;
	}
	// --------------------------------------------------------------------
	// ------------------ OPERACIÓN MULTIPLICACIÓN ------------------------
	// --------------------------------------------------------------------
	case OP_ARIT_MULT:
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
					[]( std::string & a,int& b)->Value {
						std::string ex = "";
						for (int itr = 0; itr < b; itr++)
							ex += a;
						return (ex);
					},
					[](std::string & a,const bool& b)->Value { if (b) return (a); else return ""; },
					[]( std::string & a,const double& b)->Value { Errores::generarError(Errores::ERROR_MATH_MULT_STRING, Errores::outData, "double"); return std::monostate(); },
					[]( std::string & a, std::string & b)->Value {/*abc ab = aa ab ba bb ca cb*/ return (false); },
					[]( std::string & a,const long long& b)->Value { return (a + to_string_p(b)); },

						//BOOL .. XX
						[](const bool& a,const int& b)->Value { return (a * b); },
						[](const bool& a,const bool& b)->Value { return (a * b); },
						[](const bool& a, const double& b)->Value { return (a * b); },
						[](const bool& a, std::string & b)->Value { if (a) return (b); else return ""; },
						[](const bool& a, const long long& b)->Value { return (a * b); },


						[](auto,auto)->Value { return std::monostate(); },

			}, value, v.value);
		break;
	}

	// --------------------------------------------------------------------
	// ---------------------- OPERACIÓN DIVISIÓN --------------------------
	// --------------------------------------------------------------------
	case OP_ARIT_DIV:
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
			[]( std::string & a,const int& b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "/"); return std::monostate(); },
			[]( std::string & a,const bool& b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "/"); return std::monostate(); },
			[]( std::string & a,const double& b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "/"); return std::monostate(); },
			[]( std::string & a, std::string & b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "/"); return std::monostate(); },
			[]( std::string & a,const long long& b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "/"); return std::monostate(); },

			//BOOL .. XX
			[](const bool& a,const int& b)->Value { return ((double)a / b); },
			[](const bool& a,const bool& b)->Value { return ((double)a / b); },
			[](const bool& a,const  double& b)->Value { return (a / b); },
			[](const bool& a,  std::string & b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "/"); return std::monostate(); },
			[](const bool& a, const long long& b)->Value { return ((double)a / b); },


			[](auto,auto)->Value { return std::monostate(); },

			}, value, v.value);
		break;
	}

	// --------------------------------------------------------------------
	// ------------------ OPERACIÓN DIVISIÓN ENTERA -----------------------
	// --------------------------------------------------------------------
	case OP_ARIT_DIV_ENTERA:
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
			[]( std::string & a,const int& b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "div"); return std::monostate(); },
			[]( std::string & a,const bool& b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "div"); return std::monostate(); },
			[]( std::string & a,const double& b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "div"); return std::monostate(); },
			[]( std::string & a, std::string & b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "div"); return std::monostate(); },
			[]( std::string & a,const long long& b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "div"); return std::monostate(); },

			//BOOL .. XX
			[](const bool& a,const int& b)->Value { return ((int)(a / b)); },
			[](const bool& a,const bool& b)->Value { return ((int)(a / b)); },
			[](const bool& a, const double& b)->Value { return ((int)(a / b)); },
			[](const bool& a,  std::string & b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "div"); return std::monostate(); },
			[](const bool& a, const long long& b)->Value { return ((int)(a / b)); },


			[](auto,auto)->Value { return std::monostate(); },

			}, value, v.value);
		break;
	}

	// --------------------------------------------------------------------
	// ------------------------- OPERACIÓN MOD ----------------------------
	// --------------------------------------------------------------------
	case OP_ARIT_MOD:
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
			[]( std::string & a,const int& b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "%"); return std::monostate(); },
			[]( std::string & a,const bool& b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "%"); return std::monostate(); },
			[]( std::string & a,const double& b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "%"); return std::monostate(); },
			[]( std::string & a, std::string & b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "%"); return std::monostate(); },
			[]( std::string & a,const long long& b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "%"); return std::monostate(); },

			//BOOL .. XX
			[](const bool& a, const int& b)->Value { return (a % b); },
			[](const bool& a, const bool& b)->Value { return (a % b); },
			[](const bool& a, const double& b)->Value { return (fmod(a,b)); },
			[](const bool& a,  std::string & b)->Value {Errores::generarError(Errores::ERROR_MATH_STRING,  Errores::outData, "%"); return std::monostate(); },
			[](const bool& a, const long long& b)->Value { return (a % b); },


			[](auto,auto)->Value { return std::monostate(); },

			}, value, v.value);
		break;
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
			[&v](const int& a,const std::string&)
			{
				return a && v.ValueToBool();
			},

			[](const long long& a, const long long& b) { return a && b; },
			[](const long long& a, const  bool& b) { return a && b; },
			[](const long long& a, const int& b) { return a && b; },
			[](const long long& a, const double& b) { return a && b; },
			[&v](const long long& a, const std::string&)
			{
				return a && v.ValueToBool();
			},

			[](const double& a, const double& b) { return a && b; },
			[](const double& a, const int& b) { return a && b; },
			[](const double& a, const long long& b) { return a && b; },
			[](const double& a, const bool& b) { return a && b; },
			[&v](const double& a, const std::string&)
			{
				return a && v.ValueToBool();
			},

			[](const bool& a, const double& b) { return a && b;  },
			[](const bool& a, const int& b) { return a && b; },
			[](const bool& a, const long long& b) { return a && b; },
			[](const bool& a, const bool& b) { return a && b;  },
			[&v](const bool& a, const std::string&)
			{
				return a && v.ValueToBool();
			},

			[this](const std::string & a, const double& b)
			{
				return this->ValueToBool() && b;
			},
			[this](const std::string & a, const int& b)
			{
				return this->ValueToBool() && b;
			},
			[this](const std::string & a, const long long& b)
			{
				return this->ValueToBool() && b;
			},
			[this](const std::string & a, const bool& b)
			{
				return this->ValueToBool() && b;
			},
			[&](const std::string & a, const std::string & b)
			{
				return this->ValueToBool() && v.ValueToBool();
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
			[&v](const int& a,const std::string&)
			{
				return a || v.ValueToBool();
			},

			[](const long long& a, const long long& b) { return a || b; },
			[](const long long& a, const  bool& b) { return a || b; },
			[](const long long& a, const int& b) { return a || b; },
			[](const long long& a, const double& b) { return a || b; },
			[&v](const long long& a, const std::string&)
			{
				return a || v.ValueToBool();
			},

			[](const double& a, const double& b) { return a || b; },
			[](const double& a, const int& b) { return a || b; },
			[](const double& a, const long long& b) { return a || b; },
			[](const double& a, const bool& b) { return a || b; },
			[&v](const double& a, const std::string&)
			{
				return a || v.ValueToBool();
			},

			[](const bool& a, const double& b) { return a || b;  },
			[](const bool& a, const int& b) { return a || b; },
			[](const bool& a, const long long& b) { return a || b; },
			[](const bool& a, const bool& b) { return a || b;  },
			[&v](const bool& a, const std::string&)
			{
				return a || v.ValueToBool();
			},

			[this](const std::string & a, const double& b)
			{
				return this->ValueToBool() || b;
			},
			[this](const std::string & a, const int& b)
			{
				return this->ValueToBool() || b;
			},
			[this](const std::string & a, const long long& b)
			{
				return this->ValueToBool() || b;
			},
			[this](const std::string & a, const bool& b)
			{
				return this->ValueToBool() || b;
			},
			[&](const std::string & a, const std::string & b)
			{
				return this->ValueToBool() || v.ValueToBool();
			},

			[](auto,auto) { return false; },

			}, value, v.value);
		break;
	}
	}
}

bool Value::OperacionRelacional(const Value& v, const OPERADORES op)
{
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
}

void Value::inicializacion(tipos_parametros& tipo)
{
	switch(tipo)
	{
		case tipos_parametros::PARAM_INT: value = (int)0; break;
		case tipos_parametros::PARAM_LINT: value = (long long)0; break;
		case tipos_parametros::PARAM_DOUBLE: value = (double)0; break;
		case tipos_parametros::PARAM_BOOL: value = (bool)false; break;
		case tipos_parametros::PARAM_STRING: value = (std::string)""; break;
		default: value = std::monostate(); break;
	}
}

bool Value::asignacion(Value & v, bool& fuerte)
{
	if (!fuerte)
	{
		*this = v;
		return true;
	}
	else
	{
		return std::visit(overloaded{

			//INTEGER .. XX
			[&]( int&, int&) { *this = v; return true; },
			[&]( int&, bool& b) { *this = Value((int)b); return true; },
			[&]( int&, long long& b) { *this = Value((int)b); return true;  },
			[&]( int&, double& b) { *this = Value((int)b); return true;  },
			[&]( int&, std::string & b)
			{
				char* endptr = NULL;
				auto number = strtol(b.c_str(), &endptr, 10);
				if (b.c_str() == endptr)
				{
					std::cout << "--TTTTT22--";
					Errores::generarError(Errores::ERROR_CONVERSION_VARIABLE_INT, Errores::outData);
					return false;
				}
				*this = Value((int)number);
				return true;
			},
			[&]( int&, std::monostate&) { *this = std::monostate();  return true; },


			[&]( long long&, long long& b) { *this = v;  return true;  },
			[&]( long long&,  bool& b) { *this = Value((long long)b);  return true; },
			[&]( long long&,  int& b) { *this = Value((long long)b); return true; },
			[&]( long long&,  double& b) { *this = Value((int)b); return true; },
			[&]( long long&,  std::string & b)
			{
				char* endptr = NULL;
				long long number = strtoll(b.c_str(), &endptr, 10);
				if (b.c_str() == endptr)
				{
					Errores::generarError(Errores::ERROR_CONVERSION_VARIABLE_LONG, Errores::outData);
					return false; 
				}

				*this = Value(number);
				return true;
			},
			[&]( long long&, std::monostate&) { *this = std::monostate();  return true; },

			[&]( double&,  double& b) { *this = v;   return true; },
			[&]( double&,  int& b) { *this = Value((double)b);  return true;  },
			[&]( double&,  long long& b) { *this = Value((double)b);  return true;  },
			[&]( double&,  bool& b) { *this = Value((double)b); return true;  },
			[&]( double&,  std::string & b)
			{
				char* endptr = NULL;
				double number = strtod(b.c_str(), &endptr);
				if (b.c_str() == endptr)
				{
					Errores::generarError(Errores::ERROR_CONVERSION_VARIABLE_REAL, Errores::outData);
					return false;
				}

				*this = Value(number);
				return true;
			},
			[&]( double&, std::monostate&) { *this = std::monostate();  return true; },

			[&]( bool&,  double& b) { *this = Value((bool)b);   return true; },
			[&]( bool&,  int& b) { *this = Value((bool)b);  return true; },
			[&]( bool&,  long long& b) { *this = Value((bool)b); return true;  },
			[&]( bool&,  bool& b) { *this = v; return true;   },
			[&]( bool&,  std::string & b)
			{
				if (b == "" || b == "0" || b == "false")
					* this = Value(false);
				else
					*this = Value(true);

				return true;
			},
			[&]( bool&, std::monostate&) { *this = std::monostate();  return true; },

			[&]( std::string&,  double& b) { *this = Value(to_string_p(b));  return true;  },
			[&]( std::string&,  int& b) { *this = Value(to_string_p(b));  return true; },
			[&]( std::string&,  long long& b) { *this = Value(to_string_p(b));   return true; },
			[&]( std::string&,  bool& b) { *this = Value(to_string_p(b));  return true;  },
			[&]( std::string&,  std::string & b) { *this = v;   return true; },
			[&]( std::string&, std::monostate&) { *this = std::monostate();  return true; },


			[&](std::monostate&, auto&) { *this = v;  return true; },
			

			[](auto&,auto&) { Errores::generarError(Errores::ERROR_CONVERSION_DESCONOCIDA, Errores::outData);  return false; },

			}, value, v.value);
	}


}

bool Value::Cast(Parser_Declarativo* pDec)
{
	if (pDec->_tipo == DEC_SINGLE)
	{
		Declarativo_SingleValue* x = static_cast<Declarativo_SingleValue*>(pDec);
		return Cast(x->value);
	}
	else
	{
		//Por ahora no hay valores de tipo multiple.
		return false;
	}
}

bool Value::Cast(const tipos_parametros tipo)
{
	switch (tipo)
	{
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
					*this = Value((int)number);
					return true;
				},
				[&](const bool& a) { *this = Value((int)a); return true; },
				[](const int& a) { return true; },
				[](const long long& a) { return true; },
				[&](const double& a) { *this = Value((int)a); return true; },
				[](auto) { Errores::generarError(Errores::ERROR_CONVERSION_DESCONOCIDA, Errores::outData);  return false; },
				}, value);
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
					*this = Value((long long)number);
					return true;
				},
				[&](const bool& a) { *this = Value((long long)a); return true; },
				[](const int& a) { return true; },
				[](const long long& a) { return true; },
				[&](const double& a) { *this = Value((long long)a); return true; },
				[](auto) { Errores::generarError(Errores::ERROR_CONVERSION_DESCONOCIDA, Errores::outData);  return false; },
			}, value);
		}

		case PARAM_BOOL:
		{
			return std::visit(overloaded{
				[&](std::string & b)
				{
					if (b == "" || b == "0" || b == "false")
						*this = Value(false);
					else
						*this = Value(true);

					return true;
				},
				[](const bool& a) { return true; },
				[&](const int& a) { *this = Value((bool)a);  return true; },
				[&](const long long& a) { *this = Value((bool)a);  return true; },
				[&](const double& a) { *this = Value((bool)a); return true; },
				[](auto) { Errores::generarError(Errores::ERROR_CONVERSION_DESCONOCIDA, Errores::outData); return false; },
			}, value);
		}

		case PARAM_STRING:
		{
			return std::visit(overloaded{
				[](std::string & a){ return true; },
				[&](const bool& a) {  *this = Value(to_string_p(a)); return true;  },
				[&](const int& a) {  *this = Value(to_string_p(a));  return true; },
				[&](const long long& a) {  *this = Value(to_string_p(a));  return true; },
				[&](const double& a) { *this = Value(to_string_p(a)); return true; },
				[](auto) { Errores::generarError(Errores::ERROR_CONVERSION_DESCONOCIDA, Errores::outData); return false; },
				}, value);
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

					*this = Value(number);
					return true;
				},
				[&](const bool& a) {  *this = Value(double(a)); return true; },
				[&](const int& a) {  *this = Value(double(a)); return true; },
				[&](const long long& a) {  *this = Value(double(a));  return true; },
				[](const double& a) { return true; },
				[](auto) { Errores::generarError(Errores::ERROR_CONVERSION_DESCONOCIDA, Errores::outData);  return false; },
				}, value);
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
		[]( bool& a) { return a; },
		[]( int& a) { return (bool)a; },
		[]( long long& a) { return (bool)a; },
		[]( double& a) { return (bool)a; },
		[](auto&) { return false; },
		}, value);
}


bool Value::igualdad_Condicional(const Value & v)
{
	return std::visit(overloaded{

		//INTEGER .. XX
		[](const int& a,const int& b) { return a == b; },
		[](const int& a,const bool& b) { return a == b; },
		[](const int& a,const long long& b) { return a == b; },
		[](const int& a,const double& b) { return a == b; },
		[](const int& a,const std::string & b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 int  number = strtol(b.c_str(), &endptr, 10);

			 if (errno == 0 && b.c_str() && !*endptr)
				 return a == number;
			 else
				 return false;
		},

		[](const long long& a, const long long& b) { return a == b; },
		[](const long long& a, const  bool& b) { return a == b; },
		[](const long long& a, const int& b) { return a == b; },
		[](const long long& a, const double& b) { return a == b; },
		[](const long long& a, const std::string & b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 long long number = strtoll(b.c_str(), &endptr,10);

			 if (errno == 0 && b.c_str() && !*endptr)
				 return a == number;
			 else
				 return false;
		},

		[](const double& a, const double& b) { return a == b; },
		[](const double& a, const int& b) { return a == b; },
		[](const double& a, const long long& b) { return a == b; },
		[](const double& a, const bool& b) { return a == b; },
		[](const double& a, const std::string & b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 double number = strtod(b.c_str(), &endptr);

			 if (errno == 0 && b.c_str() && !*endptr)
				 return a == number;
			 else
				 return false;
		},

		[](const bool& a, const double& b) { return a == b;  },
		[](const bool& a, const int& b) { return a == b; },
		[](const bool& a, const long long& b) { return a == b; },
		[](const bool& a, const bool& b) { return a == b;  },
		[](const bool& a, const std::string & b)
		{
		   if (b == "" || b == "0" || b == "false")
				return (false == a);
			else
				return (true == a);
		},

		[](const std::string & a, const double& b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 double number = strtod(a.c_str(), &endptr);

			 if (errno == 0 && a.c_str() && !*endptr)
				 return b == number;
			 else
				 return false;
		},
		[](const std::string & a, const int& b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 int number = strtol(a.c_str(), &endptr, 10);

			 if (errno == 0 && a.c_str() && !*endptr)
				 return b == number;
			 else
				 return false;
		},
		[](const std::string & a, const long long& b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 long long number = strtoll(a.c_str(), &endptr,10);

			 if (errno == 0 && a.c_str() && !*endptr)
				 return b == number;
			 else
				 return false;
		},
		[](const std::string & a, const bool& b)
		{
		   if (a == "" || a == "0" || a == "false")
				return (false == b);
			else
				return (true == b);
		},
		[](const std::string & a, const std::string & b) { return a == b; },


		[](auto,auto) { return false; },

		}, value, v.value);
}


bool Value::mayorQue_Condicional(const Value & v)
{
	return std::visit(overloaded{

		//INTEGER .. XX
		[](const int& a,const int& b) { return a > b; },
		[](const int& a,const bool& b) { return a > b; },
		[](const int& a,const long long& b) { return a > b; },
		[](const int& a,const double& b) { return a > b; },
		[](const int& a,const std::string & b)
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
		[](const long long& a, const std::string & b)
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
		[](const double& a, const std::string & b)
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
		[](const bool& a, const std::string & b)
		{
		   if (b == "" || b == "0" || b == "false")
				return (false > a);
			else
				return (true > a);
		},

		[](const std::string & a, const double& b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 double number = strtod(a.c_str(), &endptr);

			 if (errno == 0 && a.c_str() && !*endptr)
				 return b > number;
			 else
				 return false;
		},
		[](const std::string & a, const int& b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 int number = strtol(a.c_str(), &endptr, 10);

			 if (errno == 0 && a.c_str() && !*endptr)
				 return b > number;
			 else
				 return false;
		},
		[](const std::string & a, const long long& b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 long long number = strtoll(a.c_str(), &endptr,10);

			 if (errno == 0 && a.c_str() && !*endptr)
				 return b > number;
			 else
				 return false;
		},
		[](const std::string & a, const bool& b)
		{
		   if (a == "" || a == "0" || a == "false")
				return (false > b);
			else
				return (true > b);
		},
		[](const std::string & a, const std::string & b) { return a > b; },


		[](auto,auto) { return false; },

		}, value, v.value);
}

bool Value::menorQue_Condicional(const Value & v)
{
	return std::visit(overloaded{

		//INTEGER .. XX
		[](const int& a,const int& b) { return a < b; },
		[](const int& a,const bool& b) { return a < b; },
		[](const int& a,const long long& b) { return a < b; },
		[](const int& a,const double& b) { return a < b; },
		[](const int& a,const std::string & b)
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
		[](const long long& a, const std::string & b)
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
		[](const double& a, const std::string & b)
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
		[](const bool& a, const std::string & b)
		{
		   if (b == "" || b == "0" || b == "false")
				return (false < a);
			else
				return (true < a);
		},

		[](const std::string & a, const double& b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 double number = strtod(a.c_str(), &endptr);

			 if (errno == 0 && a.c_str() && !*endptr)
				 return b < number;
			 else
				 return false;
		},
		[](const std::string & a, const int& b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 int number = strtol(a.c_str(), &endptr, 10);

			 if (errno == 0 && a.c_str() && !*endptr)
				 return b < number;
			 else
				 return false;
		},
		[](const std::string & a, const long long& b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 long long number = strtoll(a.c_str(), &endptr,10);

			 if (errno == 0 && a.c_str() && !*endptr)
				 return b < number;
			 else
				 return false;
		},
		[](const std::string & a, const bool& b)
		{
		   if (a == "" || a == "0" || a == "false")
				return (false < b);
			else
				return (true < b);
		},
		[](const std::string & a, const std::string & b) { return a < b; },


		[](auto,auto) { return false; },

		}, value, v.value);
}

bool Value::menorIgualQue_Condicional(const Value & v)
{
	return std::visit(overloaded{

		//INTEGER .. XX
		[](const int& a,const int& b) { return a <= b; },
		[](const int& a,const bool& b) { return a <= b; },
		[](const int& a,const long long& b) { return a <= b; },
		[](const int& a,const double& b) { return a <= b; },
		[](const int& a,const std::string & b)
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
		[](const long long& a, const std::string & b)
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
		[](const double& a, const std::string & b)
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
		[](const bool& a, const std::string & b)
		{
		   if (b == "" || b == "0" || b == "false")
				return (false <= a);
			else
				return (true <= a);
		},

		[](const std::string & a, const double& b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 double number = strtod(a.c_str(), &endptr);

			 if (errno == 0 && a.c_str() && !*endptr)
				 return b <= number;
			 else
				 return false;
		},
		[](const std::string & a, const int& b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 int number = strtol(a.c_str(), &endptr, 10);

			 if (errno == 0 && a.c_str() && !*endptr)
				 return b <= number;
			 else
				 return false;
		},
		[](const std::string & a, const long long& b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 long long number = strtoll(a.c_str(), &endptr,10);

			 if (errno == 0 && a.c_str() && !*endptr)
				 return b <= number;
			 else
				 return false;
		},
		[](const std::string & a, const bool& b)
		{
		   if (a == "" || a == "0" || a == "false")
				return (false <= b);
			else
				return (true <= b);
		},
		[](const std::string & a, const std::string & b) { return a <= b; },


		[](auto,auto) { return false; },

		}, value, v.value);
}

bool Value::mayorIgualQue_Condicional(const Value & v)
{
	return std::visit(overloaded{

		//INTEGER .. XX
		[](const int& a,const int& b) { return a >= b; },
		[](const int& a,const bool& b) { return a >= b; },
		[](const int& a,const long long& b) { return a >= b; },
		[](const int& a,const double& b) { return a >= b; },
		[](const int& a,const std::string & b)
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
		[](const long long& a, const std::string & b)
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
		[](const double& a, const std::string & b)
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
		[](const bool& a, const std::string & b)
		{
		   if (b == "" || b == "0" || b == "false")
				return (false >= a);
			else
				return (true >= a);
		},

		[](const std::string & a, const double& b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 double number = strtod(a.c_str(), &endptr);

			 if (errno == 0 && a.c_str() && !*endptr)
				 return b >= number;
			 else
				 return false;
		},
		[](const std::string & a, const int& b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 int number = strtol(a.c_str(), &endptr, 10);

			 if (errno == 0 && a.c_str() && !*endptr)
				 return b >= number;
			 else
				 return false;
		},
		[](const std::string & a, const long long& b)
		{
			 char* endptr = NULL;
			 errno = 0;
			 long long number = strtoll(a.c_str(), &endptr,10);

			 if (errno == 0 && a.c_str() && !*endptr)
				 return b >= number;
			 else
				 return false;
		},
		[](const std::string & a, const bool& b)
		{
		   if (a == "" || a == "0" || a == "false")
				return (false >= b);
			else
				return (true >= b);
		},
		[](const std::string & a, const std::string & b) { return a >= b; },


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
		return this->asignacion(this->operacion_Binaria(v, OPERADORES::OP_ARIT_SUMA), fuerte);
		break;
	}

	case OPERADORES::OP_IG_EQUAL_MIN:
	{
		return this->asignacion(this->operacion_Binaria(v, OPERADORES::OP_ARIT_RESTA), fuerte);
		break;
	}

	case OPERADORES::OP_IG_EQUAL_MULT:
	{
		return this->asignacion(this->operacion_Binaria(v, OPERADORES::OP_ARIT_MULT), fuerte);
		break;
	}

	case OPERADORES::OP_IG_EQUAL_DIV:
	{
		return this->asignacion(this->operacion_Binaria(v, OPERADORES::OP_ARIT_DIV), fuerte);
		break;
	}

	case OPERADORES::OP_IG_EQUAL_MOD:
	{
		return this->asignacion(this->operacion_Binaria(v, OPERADORES::OP_ARIT_MOD), fuerte);
		break;
	}
	}
}

Value Value::operacion_Unitaria(OPERADORES & op)
{
	switch (op)
	{
	case OP_NEGADO: return !this->ValueToBool();
	case ELEM_NEG_FIRST:
	{
		return std::visit(overloaded{
		[](const int& val)->Value {return -val; },
		[](const double& val)->Value {return -val; },
		[](const long long& val)->Value {return -val; },
		[](const bool& val)->Value {return -val; },
		[&](const std::string & val)->Value {Errores::generarError(Errores::ERROR_MATH_STRING, Errores::outData, "-"); return std::monostate(); },
		[](const auto&)->Value {return std::monostate(); },
			}, value);
	}
	case OPERADORES::OP_ITR_MIN:
	{
		return std::visit(overloaded{
			[](const int& val)->Value {return val - 1; },
			[](const double& val)->Value {return val - 1; },
			[](const long long& val)->Value {return val - 1; },
			[](const bool& val)->Value {return val - 1; },
			[&](const std::string & val)->Value {Errores::generarError(Errores::ERROR_MATH_STRING, Errores::outData, "--"); return std::monostate(); },
			[](const auto&)->Value {return std::monostate(); },
			}, value);
	}

	case OPERADORES::OP_ITR_PLUS:
	{
		return std::visit(overloaded{
			[](const int& val)->Value {return val + 1; },
			[](const double& val)->Value {return val + 1; },
			[](const long long& val)->Value {return val + 1; },
			[](const bool& val)->Value {return val + 1; },
			[&](const std::string & val)->Value {Errores::generarError(Errores::ERROR_MATH_STRING, Errores::outData, "++"); return std::monostate(); },
			[](const auto&)->Value {return std::monostate(); },
			}, value);
	}

	}
}

// --------------------------------------------------------------
// --------------------------  strict  --------------------------
// --------------------------------------------------------------

//reservado para elementos estrictos.
// Ejemplo:   void funcionPrueba(strict int a, int b);
// para poder acceder a la función se debe cumplir que:
// a deberá ser entero, y sólamente entero.
// b podrá ser otro valor transformado, por ejemplo: un booleano true, devolverá b = 1 o un double 5.32 devolverá b = 5.
// inclusive un string con un valor numérico como "743", dará como resultado b=743


void Value::print()
{
	std::visit(overloaded{
		[&](std::monostate a) { return; },
		[&](auto& a) { std::cout << a; },
		//	[&](int& val) { std::cout },
		//	[&](double& val) {return Value(-val); },
		//	[&](long long& val) {return Value(-val); },
		//	[&](bool& val) {return Value(-val); },
		//	[&](std::string & val) {Errores::generarError(Errores::ERROR_MATH_STRING, Errores::outData, "-"); return Value(); },
		}, value);
}