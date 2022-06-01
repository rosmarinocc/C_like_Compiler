#pragma once
#ifndef _LEX_H
#define _LEX_H
#include"Symbol.h"
#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include<map>
#include<vector>
#include<queue>
using namespace std;

enum class LexComponent :int {
	//default 0
	Undefined,
	//comment marks 1~3
	MultipleLineCommentLeft,
	MultipleLineCommentRight,
	SingleLineComment,
	//brackets 4~9
	LeftBrace,
	RightBrace,
	LeftSquareBracket,
	RightSquareBracket,
	LeftBracket,
	RightBracket,
	//binary operators 10~25
	Plus,
	Minus,
	Multiply,
	Divide,
	Modulus,
	Equal,
	NotEqual,
	Greater,
	Less,
	GreaterEqual,
	LessEqual,
	LogicalAnd,
	LogicalOr,
	BinaryAnd,
	BinaryOr,
	BinaryXor,
	//unary operators 26~29
	BinaryNot,
	LogicalNot,
	Increment,
	Decrement,
	//assignment 30~38
	Assignment,
	AddAssignment,
	SubtractAssignment,
	MultiplyAssignment,
	DivideAssignment,
	ModulusAssignment,
	BinaryAndAssignment,
	BinaryOrAssignment,
	BinaryXorAssignment,
	//special  39~46
	Dot,
	Comma,
	Semicolon,
	Colon,
	NumberSign,
	QuestionMark,
	LeftShift,
	RightShift,
	//keywords 47~79
	Auto,
	Break,
	Bool,
	Case,
	Char,
	Const,
	Continue,
	Default,
	Do,
	Double,
	Else,
	Enum,
	Extern,
	Float,
	For,
	Goto,
	If,
	Int,
	Long,
	Register,
	Return,
	Short,
	Signed,
	Sizeof,
	Static,
	Struct,
	Switch,
	Typedef,
	Union,
	Unsigned,
	Void,
	Volatile,
	While,
	//ID 79
	ID,
	//constants 81~85
	True,
	False,
	ConstString,//双引号开头结尾的字符串常量
	ConstChar,//单引号开头结尾的单个字符常量，不再单独处理单双引号
	Digit,
	Start,
	End
};
bool is_undefined(LexComponent i);
bool is_comment_mark(LexComponent i);
bool is_bracket(LexComponent i);
bool is_binary_operator(LexComponent i);
bool is_unary_operator(LexComponent i);
bool is_assignment(LexComponent i);
bool is_id(LexComponent i);
bool is_constant(LexComponent i);

extern const char* ComponentString[];
extern const char* ComponentName[];
extern map<string, LexComponent> LexMap;
extern const int symbolNum;
void init_map();

//用于中间代码生成
typedef struct {
	int line;
	string content;
	string type;
}TokSym;

typedef struct {
	int line;
	string content;
	LexComponent type;
} Token;

//为了将行数记录进入
typedef struct {
	Token tok;
	Symbol sym;
}SymToken;


class LexAnalyzer {
private:
	int symbol_count; //已经分析道的符号表，计数 num([ID])
public:
	LexAnalyzer() { init(); }
	int resultNum;//分析结果vector的元素数量
	//Token get_rslt_elem(int index) { return index < LexResult.size() ? LexResult[index] : Token{ -1,"",LexComponent::Undefined }; }//获取结果中第index个单词
	map<int, string> NameTable; //符号表
	void init() { symbol_count = 0;  resultNum = 0; curLine = 1; LexResult.clear(); }//重置
	bool lex_analyze(string inFile);
	void output_result(string outFile);
	void getResult();//LexResult->Result
	int getcount(const string elem);
	int curLine;//当前扫描的行数
	queue<SymToken>Result;//输出给语法分析
	
	vector<Token>LexResult;//存储词法分析结果
};

#endif