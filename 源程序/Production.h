#pragma once
#include <vector>
#include <set>

#include "Symbol.h"

/*产生式类*/
class Production {
public:
	Symbol Left; //左部
	vector<Symbol> Right; //右部
	friend ostream& operator<<(ostream& out, const Production& p);
	friend bool operator==(const Production& p1, const Production& p2);
	friend bool operator<(const Production& p1, const Production& p2);
};



/*项目类*/
//形如A->.Ba
class Event {
public:
	Production prod;//对应的production
	int dotPos;     //对应点的位置
	Symbol ahead;   //展望符号
	Event(const Production& p, const int& pos,const Symbol& a);
	friend ostream& operator<<(ostream& out, const Event& e);
	friend bool operator==(const Event& ev1, const Event& ev2);
	friend bool operator<(const Event& ev1, const Event& ev2);
};



class EventClosure
{
public:
	vector <Event> Es;//产生式不同,应该是set类型会正确，但是set类无法改变其中的元素，会使得插入的
	friend bool operator<(EventClosure& ev1, const EventClosure& ev2);

	friend bool operator==(const EventClosure& ev1, const EventClosure& ev2);
};


