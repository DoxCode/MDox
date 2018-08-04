
#include "Tester.h"
#include <deque>
/*
Clase usada para testear distintas partes del sistema.
 Los métodos se guardaran por si hacen falta en un futuro.
*/

void debug_mostrarParametros(estructuraParametro* p, int index)
{
	switch (p->_getType)
	{
	case EP_BASE:
		std::cout << "BASE: " << " Index: " << index << " Tipo: " << p->tipo << "\n";
		break;
	case EP_VALOR:
	{
		estructuraParValor * eVPointer = static_cast<estructuraParValor*>(p);
		std::cout << "VALOR: " << " Index: " << index << " Tipo: " << eVPointer->tipo << " val: " << eVPointer->value << "\n";
		break;
	}
	case EP_SINGLE:
	{
		estructuraParSingle * eVPointer = static_cast<estructuraParSingle*>(p);
		std::cout << "SINGLE: " << " Index: " << index << " Tipo: " << eVPointer->tipo << "  -> ";
       
		debug_mostrarParametros(eVPointer->valor, index);
		break;
	}
	case EP_MULTIPLE:
	{
		estructuraParMultiple * eVPointer = static_cast<estructuraParMultiple*>(p);
		std::cout << "MULTIPLE: " <<" Index: " << index << " Tipo: " << eVPointer->tipo << "   -->  \n  ";

		for (std::vector<estructuraParametro*>::iterator it = eVPointer->valores.begin(); it != eVPointer->valores.end(); ++it)
		{
			debug_mostrarParametros((*it), index);
		}

		std::cout << "\n  <----\n";

		break;
	}
	}
}


int fff2(int a)
{
	
	std::deque<int> _v;
	_v.push_front(a);


    int _a;

	while (true)
	{
		if (_v.at(0) == 1)
		{
			break;
		}

		_v.push_front(_v.at(0) - 1);
	}

	while (true)
	{
		_a = 1 + _v.front();
		_v.pop_front();

		if (_v.size() == 0)
			break;
	}

	return _a;
}

int fff(int a)
{
	if (a == 1)
		return 1;

	return 1 + fff(a - 1);
}
