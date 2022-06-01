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


#define SHIFTIN   0//移进
#define REDUCE    1//归约
#define ACCEPT    2//接受
#define ERROR     3//出错


struct op {
	int action;
	int index;	//eg:r5--使用拓宽文法5归约;s6--移进转移到状态6
};


/*SLR类*/
class LR1 {
public:
	string targetCode;//待编译源程序

	LexAnalyzer lex;
	vector <Production> generator;//产生式集合

	set <Symbol> vSet;//所有的集合,包含first/follow信息
	set <Symbol> vnSet;//所有非终结符的集合,仅用作迭代比对,不存放first/follow
	set <Symbol> vtSet;//所有终结符集合,仅用作迭代比对,不存放first/follow

	map <Symbol, set <Symbol>> first_set;//FIRST集
	map <Symbol, set <Symbol>> follow_set;//FOLLOW集

	vector<Event> items;//拓广文法项目集合,加入插入位置以及展望符的项目集合，S'->S.#,
	vector<EventClosure> eventclo;//加入了展望符号的event集合items，这里应当是有顺序的

	map<pair<int, Symbol>, int> GotoInfo;//Goto的信息,第一个信息应当是Eventclosure,但是注意，其中第一个int是在alleventlo的下标号

	map <pair< int, Symbol >, op > GotoTable;//GOTO表:eventclo[int1]遇到Symbol转移到eventclo[int2],只包括error和shift
	map<pair<int, Symbol>, op> ActionTable;//Action表:ACTION[i,a]=op

	SemanticAnalyzer SemanticAnalysis;
	int leafnum = 0;
	int maxtreelevel = 0;
	IdentNode* reductionTreeRoot = NULL;


	vector<Quadruple> OptIntermediateCode;//优化后四元式中间代码
	vector<blockItem> BlockGroup;//基本块划分

/************************************************************/

	//读取产生式,生成拓广文法
	int read_generators(string dat_path);
	//first集的产生
	void get_firstset_of_vn();
	//返回某符号串的first集
	set<Symbol> get_firstset_of_string(const vector<Symbol>& str);

	//follow集的产生
	//void get_followset_of_vn();

	//生成闭包
	void init_items();
	//根据generator生成拓广文法的items集合，加上展望符号位置
	//void get_table();//得到action表以及goto表
	//生成闭包集合，包括了
	EventClosure& GetEventClo(EventClosure& CloJ);
	EventClosure GetEventTO(EventClosure& CloI, const Symbol sym);
	/*************************************/
	void getTable();
	void printTable(const string file_path);
	void grammartree(const string filepath, queue<SymToken>& code);
	void Seman_analysis(queue<SymToken>& Code);
	void Seman_tree(IdentNode*thenode,int thelevel);




	/**************打包函数*********************/
	void LexGo();
	void SemanticGo();
};

