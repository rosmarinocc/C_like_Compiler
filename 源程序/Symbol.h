#pragma once
#include <iostream>
#include <string>
using namespace std;
/*�ķ�������*/
#define VN   0
#define VT   1
#define EPSILON   2
#define END  3
#define symEps Symbol(EPSILON,"[z]")
#define symEnd Symbol(END,"[#]")
#define symS0 Symbol(VN,"<S0>")
#define strstart string("Program")
#define strend string("[#]")

class Symbol {
public:
	int symbol_type;//��������
	string content;//�������
	friend bool operator==(const Symbol& s1, const Symbol& s2);
	friend bool operator!=(const Symbol& s1, const Symbol& s2);
	friend bool operator < (const Symbol& s1, const Symbol& s2);
	friend ostream& operator<<(ostream& out, const Symbol& s);

	Symbol();
	Symbol(const int& st, const string& c);
	Symbol(const Symbol& s);
};



