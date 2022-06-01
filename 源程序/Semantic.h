#pragma once
#include <iostream>
#include <string>
#include "Lex.h"
#include"Production.h"
using namespace std;


enum SymType { VOID, INT };
enum SymKind { FUNC, VAR, ARRAY, TEMP, CONST, RET };


typedef SymType type;
typedef SymKind kind;

struct SemanticSymbol
{
	/*�����������*/
	TokSym toke; //int line ,string content,string type {lineΪtoken�е�Line��contentΪstring�����token�ľ������ݣ�����type��identifier��int ���ߺ�content������ͬ}
	int table_index;//��������table��index
	int index;//������table�ڲ���index
};

struct Quadruple
{
	/*��Ԫʽ*/
	//int index;//������
	string op;//�����
	string arg1;
	string arg2;
	string res;
};

struct IdentNode
{
	int level = 0;
	IdentNode* parent = NULL;
	vector <IdentNode*> children;
	pair<string, int> content;
	int type = INT;
	int kind = VAR;
	int n = 0;
	int width = 0;
	vector<int> dimension;  //�洢����/����ά��
	vector<string> params;  //��������
	string place;
	int quad;
	int true_list;
	int false_list;
	IdentNode() { clear(); }
	void clear()
	{
		level = -1;
		parent = NULL;
		children.clear();
		content = pair<string, int>("", -1);
	}
};

struct symbolTable;

struct symbolTableItem
{
	int id;
	type t;	//���ͣ�int or void
	kind k;	//���ࣺfunc/var/array/temp/const/ret
	int offset;
	vector<int> dimension;//����������
	symbolTable* proctable = NULL;
};

struct symbolTable
{
	//˫����
	symbolTable* previous = NULL;
	symbolTable* next = NULL;
	//����
	symbolTable* parent = NULL;
	int width = 0;
	vector<symbolTableItem> table;
	void clear()
	{
		previous = NULL;
		next = NULL;
		width = 0;
		table.clear();
	}
	void enter(int id, type t, kind k, int offset)
	{
		symbolTableItem e;
		e.id = id;
		e.t = t;
		e.k = k;
		e.offset = offset;
		for (auto i = 0; i < table.size(); i++)
		{
			if (table[i].id == id)
			{
				throw string("ERROR: ����������ű����:") + to_string(id) + string("�ض���\n");
			}
		}
		table.push_back(e);
	}
	void enterdimension(int id, vector<int> dimension)
	{
		for (auto i = 0; i < table.size(); i++)
		{
			if (table[i].id == id && ((table[i].k == ARRAY) || (table[i].k == FUNC)))
			{
				table[i].dimension = dimension;
				break;
			}
		}
	}
	void enterproc(int id, symbolTable* newtable)
	{
		for (auto i = 0; i < table.size(); i++)
		{
			if (table[i].id == id && table[i].k == FUNC)
			{
				table[i].proctable = newtable;
			}
		}
	}
};


class SemanticAnalyzer
{
	/*��������࣬public*/
public:
	symbolTable* last_table; //ָ�����ı���
	symbolTable* global_table;//�������з��ű� 
	vector<Quadruple> Quadruple_Code;//��Ԫʽ��
	vector<int> cur_tableStack;//��ǰ�������Ӧ�ķ��ű�����ջ
public:
	//����
	SemanticAnalyzer();
	//��������
	~SemanticAnalyzer();
	////�������
	//void insertSymbol(const SemanticSymbol& sym);//�����еķ�����Ϣ����
	//�����������
	void seman_analysis(const Production& pro, IdentNode* root, map<int, string> nameTable);//�������Ϊ����ʽ������
	//��Ԫʽ�����
	void printQuadruple(const string outpath);

private:
	vector<symbolTable*> table_stack;
	vector<int> offset_stack;
	int temp_counter = -1;
	string newtemp();
	string lookup(int id);
	symbolTableItem* find(int id);
	int nextstat();
	void AddQCode(string op, string arg1, string arg2, string result);
private:
	//<Program> -> <Pro_m> <DeclareString>
	void SemanProd_Program(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<Pro_m> ->[z]
	void SemanProd_Pro_m(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<DeclareString> -> <DeclareString - option> | <DeclareString - option> <DeclareString>
	void SemanProd_DeclareString(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<DeclareString-option> ->[INT] [ID] <ParameterContent> [SEMI] 
	//| <FunctionDeclare> <FunDec> <Block>
	void SemanProd_DecOption(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<ParameterContent> -> <ArrayDeclare> | [z]
	void SemanProd_ParaContent(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<ArrayDeclare> -> [LeftSquareBracket] [NUM] [RightSquareBracket] 
	//| [LeftSquareBracket] [NUM] [RightSquareBracket] <ArrayDeclare>
	void SemanProd_ArrayDeclare(const Production& pro, IdentNode* root, map<int, string> nameTable);
	/*<FunctionDeclare> ->[VOID] | [INT]*/
	void SemanProd_FunctionDeclare(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<FunDec> ->[ID] <CreateFunTable_m> [LPAREN] <VarContent> [RPAREN]
	void SemanProd_FunDec(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<CreateFunTable_m> ->[z]
	void SemanProd_CreateFunTable_m(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<VarContent> -> <VarList> | [VOID] | [z]
	void SemanProd_VarContent(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<VarList> -> [INT] [ID] [COMMA] <VarList> | [INT] [ID]
	void SemanProd_VarList(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<Block> ->[LBBRACKET] <DefList> <StmtList>[RBBRACKET]
	void SemanProd_Block(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<DefList> -> <Def> [SEMI] <DefList> | [z]
	void SemanProd_DefList(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<Def> ->[INT][ID] <ParameterContent>
	void SemanProd_Def(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<StmtList> -> <Stmt> <StmtList> | <Stmt> 
	void SemanProd_StmtList(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<Stmt> -> <AssignStmt>[SEMI] | <ReturnStmt>[SEMI] | <IfStmt> | <WhileStmt>
	void SemanProd_Stmt(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<AssignStmt> ->[ID][ASSIGN] <Exp> | <Array>[ASSIGN] <Exp>
	void SemanProd_AssignStmt(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<Exp> -> <ANDExp> | <ANDExp> [OR] <Exp>
	void SemanProd_Exp(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<ANDExp> -> <NOTExp> | <NOTExp>[AND] <ANDExp>
	void SemanProd_ANDExp(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<NOTExp> ->[NOT] <SubExp> | <SubExp>
	void SemanProd_NOTExp(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<SubExp> -> <OPNUM> | <OPNUM> <Relop> <SubExp> 
	void SemanProd_SubExp(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<Relop> ->[GT] | [LT] | [GE] | [LE] | [EQ] | [NQ]
	void SemanProd_Relop(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<OPNUM> -> <Item> | <Item> <OPERAT1> <OPNUM>
	void SemanProd_OPNUM(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<Item> -> <Factor> | <Factor> <OPERAT2> <Item>
	void SemanProd_Item(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<OPERAT1> ->[PLUS] | [SUB] | [BinaryAnd] | [BinaryOr] | [BinaryXor]
	void SemanProd_OPERAT1(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<OPERAT2> -> [MUL] | [DIV] 
	void SemanProd_OPERAT2(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<Factor> -> [NUM] | [ID] | <Array> | [ID] <CallStmt> | [LPAREN] <Exp> [RPAREN] | [LPAREN] <AssignStmt> [RPAREN] 
	void SemanProd_Factor(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<Array> -> [ID] [LeftSquareBracket] <Exp> [RightSquareBracket] | <Array> [LeftSquareBracket] <Exp> [RightSquareBracket] 
	void SemanProd_Array(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<CallStmt> -> [LPAREN] <CallFun> [RPAREN]
	void SemanProd_CallStmt(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<CallFun> ->[z] | <Args>
	void SemanProd_CallFun(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<Args> -> <Exp>[COMMA] <Args> | <Exp>
	void SemanProd_Args(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<ReturnStmt> -> [RETURN] <Exp> | [RETURN]
	void SemanProd_ReturnStmt(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<WhileStmt> -> <WhileStmt_m1> [WHILE] [LPAREN] <Exp> [RPAREN] <WhileStmt_m2> <Block>
	void SemanProd_WhileStmt(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<WhileStmt_m1> ->[z]
	void SemanProd_WhileStmt_m1(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<WhileStmt_m2> ->[z]
	void SemanProd_WhileStmt_m2(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<IfStmt> -> [IF] [LPAREN] <Exp_stmt> [RPAREN] <IfStmt_m2> <Block> <IfNext>
	void SemanProd_IfStmt(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<Exp_stmt>  -> <Exp>
	void SemanProd_Exp_stmt(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<IfStmt_m> -> [z]
	void SemanProd_IfStmt_m1(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<IfNext> -> [z] | [ELSE] <IfStmt_next> <Block>
	void SemanProd_IfNext(const Production& pro, IdentNode* root, map<int, string> nameTable);
	//<IfStmt_next>-> [z]
	void SemanProd_IfStmt_next(const Production& pro, IdentNode* root, map<int, string> nameTable);

};

