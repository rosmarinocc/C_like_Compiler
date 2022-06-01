#pragma once
#include <vector>
#include <set>

#include "Symbol.h"

/*����ʽ��*/
class Production {
public:
	Symbol Left; //��
	vector<Symbol> Right; //�Ҳ�
	friend ostream& operator<<(ostream& out, const Production& p);
	friend bool operator==(const Production& p1, const Production& p2);
	friend bool operator<(const Production& p1, const Production& p2);
};



/*��Ŀ��*/
//����A->.Ba
class Event {
public:
	Production prod;//��Ӧ��production
	int dotPos;     //��Ӧ���λ��
	Symbol ahead;   //չ������
	Event(const Production& p, const int& pos,const Symbol& a);
	friend ostream& operator<<(ostream& out, const Event& e);
	friend bool operator==(const Event& ev1, const Event& ev2);
	friend bool operator<(const Event& ev1, const Event& ev2);
};



class EventClosure
{
public:
	vector <Event> Es;//����ʽ��ͬ,Ӧ����set���ͻ���ȷ������set���޷��ı����е�Ԫ�أ���ʹ�ò����
	friend bool operator<(EventClosure& ev1, const EventClosure& ev2);

	friend bool operator==(const EventClosure& ev1, const EventClosure& ev2);
};


