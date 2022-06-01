#include "Symbol.h"
/******************ÎÄ·¨·ûºÅÀà******************/
Symbol::Symbol()
{
	symbol_type = VT;
	content = "";
	return;
}

Symbol::Symbol(const int& st, const string& c)
{
	symbol_type = st;
	content = c;
	return;
}

Symbol::Symbol(const Symbol& s)
{
	symbol_type = s.symbol_type;
	content = s.content;
	return;
}


bool operator==(const Symbol& s1, const Symbol& s2)
{
	if (s1.symbol_type == s2.symbol_type && s1.content == s2.content)
		return true;
	return false;
}
bool operator < (const Symbol& s1, const Symbol& s2)
{

	if (s1.symbol_type < s2.symbol_type)
		return true;
	else if (s1.symbol_type > s2.symbol_type)
		return false;
	else
		return (s1.content < s2.content);
}
bool operator!=(const Symbol& s1, const Symbol& s2) {
	if (s1.content == s2.content && s1.symbol_type == s2.symbol_type)
		return false;
	return true;
}

ostream& operator<<(ostream& out, const Symbol& s)
{
	out << s.content;
	return out;
}

