#include "Production.h"
/******************Production类******************/
ostream& operator<<(ostream& out, const Production& p) {
	out << p.Left << " -> ";
	auto p_r = p.Right.begin();
	for (; p_r != p.Right.end(); ++p_r) {
		out << *p_r;
	}
	return out;
}

bool operator==(const Production& p1, const Production& p2) {
	if (p1.Left != p2.Left)
		return false;
	if (p1.Right.size() != p2.Right.size())
		return false;
	for (int it = 0; it < p1.Right.size(); it++)
	{
		if (p1.Right[it] != p2.Right[it])
			return false;
	}		
}

bool operator<(const Production& p1, const Production& p2) {
	if (p1.Left < p2.Left)
		return true;
	else if (p2.Left < p1.Left)
		return false;
	else {
		int i = 0;
		for (; i < int(min(p1.Right.size(), p2.Right.size())); ++i) {
			if (p1.Right[i] < p2.Right[i])
				return true;
			else if (p2.Right[i] < p1.Right[i])
				return false;
			else
				continue;
		}
		if (p1.Right.size() == p2.Right.size())
			return false;
		else {
			if (i == p1.Right.size())//p1右部比p2短
				return true;
		}
		return false;
	}
}

/******************Production类******************/


/******************Event类******************/
Event::Event(const Production& p, const int& pos, const Symbol& a)
{
	prod = p;
	dotPos = pos;
	ahead = a;
}

ostream& operator<<(ostream& out, const Event& e)
{
	out << e.prod.Left << " -> ";
	for (int i = 0; i < int(e.prod.Right.size()); ++i) {
		if (i == e.dotPos)
			cout << "·";
		cout << e.prod.Right[i];
	}
	if (e.dotPos == e.prod.Right.size())
		cout << "·";
	cout << " , " << e.ahead; //S -> S· ， #
	return out;
}

//判断两个项目是否相等,包含了项目以及其对应的展望符
bool operator==(const Event& ev1, const Event& ev2)
{
	return (ev1.ahead==ev2.ahead&&ev1.dotPos==ev2.dotPos&&ev1.prod==ev2.prod);
}
bool operator<(const Event& ev1, const Event& ev2)
{
	return (ev1.prod < ev2.prod ? true : ev1.dotPos < ev2.dotPos ? true : ev1.ahead < ev2.ahead ? true : false);
}
/******************Event类******************/




bool operator<(EventClosure& ev1, const EventClosure& ev2)
{
	int r = min(ev1.Es.size(), ev2.Es.size());
	for (int i = 0; i < r; i++)
	{
		if (!(ev1.Es[i] == ev2.Es[i]))
			return ev1.Es[i] < ev2.Es[i];
	}
	return ev1.Es.size() < ev2.Es.size();
}

/******************EventSet类*******************/
//evetset中，判断两种闭包是否一样
bool operator==(const EventClosure& ev1, const EventClosure& ev2)
{
	if (ev1.Es.size() != ev2.Es.size())
		return false;

	int count = 0;
	for (auto& tmp1 : ev1.Es) {
		for (auto& tmp2 : ev2.Es) {
			if (tmp1 == tmp2) {
				++count;
				break;
			}
		}
	}

	/*for (int i = 0; i < (int)(ev1.Es.size()); i++)
		if (!(ev1.Es[i] == ev2.Es[i]))
			return false;*/
	return count== ev1.Es.size();
}