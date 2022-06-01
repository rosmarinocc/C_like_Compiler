# pragma once
# define _CRT_SECURE_NO_WARNINGS
# include <set>
# include <map>
# include <stack>
# include <vector>
# include <iostream>
# include <iomanip>
# include <string>
# include <fstream>
# include <sstream>
# include <algorithm>
# include <queue>

# include "Symbol.h"
# include "Production.h"
# include "Lex.h"
# include "Semantic.h"
# include "Optimizer.h"
# include "ObjectCodeGen.h"

using namespace std;


#define SHIFTIN   0//�ƽ�
#define REDUCE    1//��Լ
#define ACCEPT    2//����
#define ERROR     3//����


struct op {
	int action;
	int index;	//eg:r5--ʹ���ؿ��ķ�5��Լ;s6--�ƽ�ת�Ƶ�״̬6
};


/*SLR��*/
class LR1 {
public:
	string targetCode;//������Դ����

	LexAnalyzer lex;
	vector <Production> generator;//����ʽ����

	set <Symbol> vSet;//���еļ���,����first/follow��Ϣ
	set <Symbol> vnSet;//���з��ս���ļ���,�����������ȶ�,�����first/follow
	set <Symbol> vtSet;//�����ս������,�����������ȶ�,�����first/follow

	map <Symbol, set <Symbol>> first_set;//FIRST��
	map <Symbol, set <Symbol>> follow_set;//FOLLOW��

	vector<Event> items;//�ع��ķ���Ŀ����,�������λ���Լ�չ��������Ŀ���ϣ�S'->S.#,
	vector<EventClosure> eventclo;//������չ�����ŵ�event����items������Ӧ������˳���

	map<pair<int, Symbol>, int> GotoInfo;//Goto����Ϣ,��һ����ϢӦ����Eventclosure,����ע�⣬���е�һ��int����alleventlo���±��

	map <pair< int, Symbol >, op > GotoTable;//GOTO��:eventclo[int1]����Symbolת�Ƶ�eventclo[int2],ֻ����error��shift
	map<pair<int, Symbol>, op> ActionTable;//Action��:ACTION[i,a]=op

	SemanticAnalyzer SemanticAnalysis;
	int leafnum = 0;
	int maxtreelevel = 0;
	IdentNode* reductionTreeRoot = NULL;


	vector<Quadruple> OptIntermediateCode;//�Ż�����Ԫʽ�м����
	vector<blockItem> BlockGroup;//�����黮��

/************************************************************/

	//��ȡ����ʽ,�����ع��ķ�
	int read_generators(string dat_path);
	//first���Ĳ���
	void get_firstset_of_vn();
	//����ĳ���Ŵ���first��
	set<Symbol> get_firstset_of_string(const vector<Symbol>& str);

	//follow���Ĳ���
	//void get_followset_of_vn();

	//���ɱհ�
	void init_items();
	//����generator�����ع��ķ���items���ϣ�����չ������λ��
	//void get_table();//�õ�action���Լ�goto��
	//���ɱհ����ϣ�������
	EventClosure& GetEventClo(EventClosure& CloJ);
	EventClosure GetEventTO(EventClosure& CloI, const Symbol sym);
	/*************************************/
	void getTable();
	void printTable(const string file_path);
	void grammartree(const string filepath, queue<SymToken>& code);
	void Seman_analysis(queue<SymToken>& Code);
	void Seman_tree(IdentNode*thenode,int thelevel);




	/**************�������*********************/
	void LexGo();
	void SemanticGo();
};

