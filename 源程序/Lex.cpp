#include"Lex.h"
#include<iomanip>
using namespace std;
map<string, LexComponent> LexMap;
const int symbolNum = 86;
const char* ComponentString[] = {
	//default
	"\0",
	//comment marks
	"/*",
	"*/",
	"//",
	//brackets
	"{",
	"}",
	"[",
	"]",
	"(",
	")",
	//binary operators
	"+",
	"-",
	"*",
	"/",
	"%",
	"==",
	"!=",
	">",
	"<",
	">=",
	"<=",
	"&&",
	"||",
	"&",
	"|",
	"^",
	//unary operator
	"~",
	"!",
	"++",
	"--",
	//assignment
	"=",
	"+=",
	"-=",
	"*=",
	"/=",
	"%=",
	"&=",
	"|=",
	"^=",
	//special
	".",
	",",
	";",
	":",
	"#",
	"?",
	"<<",
	">>",
	//keywords
	"auto",
	"break",
	"bool",
	"case",
	"char",
	"const",
	"continue",
	"default",
	"do",
	"double",
	"else",
	"enum",
	"extern",
	"float",
	"for",
	"goto",
	"if",
	"int",
	"long",
	"register",
	"return",
	"short",
	"signed",
	"sizeof",
	"static",
	"struct",
	"switch",
	"typedef",
	"union",
	"unsigned",
	"void",
	"volatile",
	"while",
	//ID
	"\0\0\0\0\0",
	//constants
	"true",
	"false",
	"\0\0",
	"\0\0\0",
	"\0\0\0\0"
};

const char* ComponentName[] = {
	//default
	"Undefined",
	//comment marks
	"MultipleLineCommentLeft",
	"MultipleLineCommentRight",
	"SingleLineComment",
	//brackets
	//"LeftBrace",
	"[LBBRACKET]",
	//"RightBrace",
	"[RBBRACKET]",
	"[LeftSquareBracket]",
	"[RightSquareBracket]",
	//"LeftBracket",
	"[LPAREN]",
	//"RightBracket",
	"[RPAREN]",
	//binary operators
	//"Plus",
	"[PLUS]",
	//"Minus",
	"[SUB]",
	//"Multiply",
	"[MUL]",
	//"Divide",
	"[DIV]",
	"Modulus",
	//"Equal",
	"[EQ]",
	//"NotEqual",
	"[NE]",
	//"Greater",
	"[GT]",
	//"Less",
	"[LT]",
	//"GreaterEqual",
	"[GE]",
	//"LessEqual",
	"[LE]",
	"LogicalAnd",
	"LogicalOr",
	"[BinaryAnd]",
	"[BinaryOr]",
	"[BinaryXor]",
	//unary operators
	"BinaryNot",
	"LogicalNot",
	"Increment",
	"Decrement",
	//assignment
	//"Assignment",
	"[ASSIGN]",
	"AddAssignment",
	"SubtractAssignment",
	"MultiplyAssignment",
	"DivideAssignment",
	"ModulusAssignment",
	"BinaryAndAssignment",
	"BinaryOrAssignment",
	"BinaryXorAssignment",
	//special
	"Dot",
	//"Comma",
	"[COMMA]",
	//"Semicolon",
	"[SEMI]",
	"Colon",
	"NumberSign",
	"QuestionMark",
	"LeftShift",
	"RightShift",
	//keywords
	"Auto",
	"Break",
	"Bool",
	"Case",
	"Char",
	"Const",
	"Continue",
	"Default",
	"Do",
	"Double",
	//"Else",
	"[ELSE]",
	"Enum",
	"Extern",
	"Float",
	"For",
	"Goto",
	//"If",
	"[IF]",
	//"Int",
	"[INT]",
	"Long",
	"Register",
	//"Return",
	"[RETURN]",
	"Short",
	"Signed",
	"Sizeof",
	"Static",
	"Struct",
	"Switch",
	"Typedef",
	"Union",
	"Unsigned",
	//"Void",
	"[VOID]",
	"Volatile",
	//"While",
	"[WHILE]",
	//ID
	//"ID",
	"[ID]",
	//constants
	"True",
	"False",
	"ConstString",
	"ConstChar",
	//"Digit"
	"[NUM]"
};

bool is_undefined(LexComponent i) { return i == LexComponent::Undefined; }
bool is_comment_mark(LexComponent i) { return i >= LexComponent::MultipleLineCommentLeft && i <= LexComponent::SingleLineComment; }
bool is_bracket(LexComponent i) { return i >= LexComponent::LeftBrace && i <= LexComponent::RightBracket; }
bool is_binary_operator(LexComponent i) { return i >= LexComponent::Plus && i <= LexComponent::BinaryXor; }
bool is_unary_operator(LexComponent i) { return i >= LexComponent::BinaryNot && i <= LexComponent::Decrement; }
bool is_assignment(LexComponent i) { return i >= LexComponent::Assignment && i <= LexComponent::BinaryXorAssignment; }
bool is_id(LexComponent i) { return i == LexComponent::ID; }
bool is_constant(LexComponent i) { return i >= LexComponent::True && i <= LexComponent::Digit; }

void init_map()
{
	for (int i = 0; i < symbolNum; i++)
	{
		LexMap[(string)ComponentString[i]] = (LexComponent)i;
	}
}

bool LexAnalyzer::lex_analyze(string inFile)
{
	ifstream f(inFile, ios::in);
	if (f.fail())
	{
		cout<< "Error: Fail to open file " << inFile << " !" << endl;
		exit(-1);
		return false;
	}
	string cur_elem = "";
	char c;
	while (f.peek() != EOF)
	{
		c = char(f.get());
		switch (c) {
			case(' '):
			case('\t'):
				while (f.peek() == ' ' || f.peek() == '\t')
					c = char(f.get());
				break;
			case('\n'):
				curLine++;
				break;
			case('{'):
				LexResult.push_back({ curLine,"{",LexComponent::LeftBrace });
				break;
			case('}'):
				LexResult.push_back({ curLine,"}",LexComponent::RightBrace });
				break;
			case('('):
				LexResult.push_back({ curLine,"(",LexComponent::LeftBracket });
				break;
			case(')'):
				LexResult.push_back({ curLine,")",LexComponent::RightBracket });
				break;
			case('['):
				LexResult.push_back({ curLine,"[",LexComponent::LeftSquareBracket });
				break;
			case(']'):
				LexResult.push_back({ curLine,"]",LexComponent::RightSquareBracket });
				break;
			case('+'):
				if (f.peek() == '=')
				{
					f.get();
					LexResult.push_back({ curLine,"+=",LexComponent::AddAssignment });
				}
				else if (f.peek() == '+')
				{
					f.get();
					LexResult.push_back({ curLine,"++",LexComponent::Increment });
				}
				else
					LexResult.push_back({ curLine,"+",LexComponent::Plus });
				break;
			case('-'):
				if (f.peek() == '=')
				{
					f.get();
					LexResult.push_back({ curLine,"-=",LexComponent::SubtractAssignment });
				}
				if (f.peek() == '-')
				{
					f.get();
					LexResult.push_back({ curLine,"--",LexComponent::Decrement });
				}
				else
					LexResult.push_back({ curLine,"-",LexComponent::Minus });
				break;
			case('*'):
				if (f.peek() == '=')
				{
					f.get();
					LexResult.push_back({ curLine,"*=",LexComponent::MultiplyAssignment });
				}
				else
					LexResult.push_back({ curLine,"*",LexComponent::Multiply });
				break;
			case('/'):
				if (f.peek() == '=')
				{
					f.get();
					LexResult.push_back({ curLine,"/=",LexComponent::DivideAssignment });
				}
				else if (f.peek() == '/')//单行注释
				{
					while (f.peek() != EOF && f.peek() != '\n')
						f.get();
				}
				else if (f.peek() == '*')//多行注释
				{
					f.get();
					int close = 0;
					while (f.peek() != EOF)
					{
						c = (char)f.get();
						if (c == '*' && f.peek() == '/')
						{
							f.get();
							close = 1;
							break;
						}
					}
					if (!close)
					{
						cout << "Error in line " << curLine << " : Unclosed comment block!" << endl;
						exit(-1);
						return false;
					}
				}
				else
					LexResult.push_back({ curLine,"/",LexComponent::Divide });
				break;
			case('%'):
				if (f.peek() == '=')
				{
					f.get();
					LexResult.push_back({ curLine,"%",LexComponent::ModulusAssignment });
				}
				else
					LexResult.push_back({ curLine,"%=",LexComponent::Modulus });
				break;
			case('&'):
				if (f.peek() == '=')
				{
					f.get();
					LexResult.push_back({ curLine,"&=",LexComponent::BinaryAndAssignment });
				}
				else if (f.peek() == '&')
				{
					f.get();
					LexResult.push_back({ curLine,"&&",LexComponent::LogicalAnd });
				}
				else
					LexResult.push_back({ curLine,"&",LexComponent::BinaryAnd });
				break;
			case('|'):
				if (f.peek() == '=')
				{
					f.get();
					LexResult.push_back({ curLine,"|=",LexComponent::BinaryOrAssignment });
				}
				else if (f.peek() == '|')
				{
					f.get();
					LexResult.push_back({ curLine,"||",LexComponent::LogicalOr });
				}
				else
					LexResult.push_back({ curLine,"|",LexComponent::BinaryOr });
				break;
			case('^'):
				if (f.peek() == '=')
				{
					f.get();
					LexResult.push_back({ curLine,"^=",LexComponent::BinaryXor });
				}
				else
					LexResult.push_back({ curLine,"^",LexComponent::BinaryXorAssignment });
				break;
			case('~'):
				LexResult.push_back({ curLine,"~",LexComponent::BinaryNot });
				break;
			case('!'):
				if (f.peek() == '=')
				{
					f.get();
					LexResult.push_back({ curLine,"!=",LexComponent::NotEqual });
				}
				else
					LexResult.push_back({ curLine,"!",LexComponent::LogicalNot });
				break;
			case('='):
				if (f.peek() == '=')
				{
					f.get();
					LexResult.push_back({ curLine,"==",LexComponent::Equal });
				}
				else
					LexResult.push_back({ curLine,"=",LexComponent::Assignment });
				break;
			case('>'):
				if (f.peek() == '=')
				{
					f.get();
					LexResult.push_back({ curLine,">=",LexComponent::GreaterEqual });
				}
				else if (f.peek() == '>')
				{
					f.get();
					LexResult.push_back({ curLine,">>",LexComponent::RightShift });
				}
				else
					LexResult.push_back({ curLine,">",LexComponent::Greater });
				break;
			case('<'):
				if (f.peek() == '=')
				{
					f.get();
					LexResult.push_back({ curLine,"<=",LexComponent::LessEqual });
				}
				else if (f.peek() == '<')
				{
					f.get();
					LexResult.push_back({ curLine,"<<",LexComponent::LeftShift });
				}
				else
					LexResult.push_back({ curLine,"<",LexComponent::Less });
				break;
			case('.'):
				LexResult.push_back({ curLine,".",LexComponent::Dot });
				break;
			case(','):
				LexResult.push_back({ curLine,",",LexComponent::Comma });
				break;
			case(';'):
				LexResult.push_back({ curLine,";",LexComponent::Semicolon });
				break;
			case('#'):
				LexResult.push_back({ curLine,"#",LexComponent::NumberSign });
				break;
			case('?'):
				LexResult.push_back({ curLine,"?",LexComponent::QuestionMark });
				break;
			case(':'):
				LexResult.push_back({ curLine,":",LexComponent::Colon });
				break;
			case('\'')://const char
				cur_elem.clear();
				cur_elem += '\'';
				c = (char)f.get();
				cur_elem += c;
				if (c != '\\' && f.peek() == '\'')
				{
					cur_elem += (char)f.get();
					LexResult.push_back({ curLine,cur_elem,LexComponent::ConstChar });
				}
				else
				{
					if (c == '\\' && (f.peek() == 'n' || f.peek() == 't' || f.peek() == '\'' || f.peek() == '\"'
						|| f.peek() == '\?' || f.peek() == '0' || f.peek() == '\\'))
					{
						cur_elem += (char)f.get();
						if (f.peek() == '\'')
						{
							cur_elem += (char)f.get();
							LexResult.push_back({ curLine,cur_elem,LexComponent::ConstChar });
						}
						else
						{
							if (f.peek() == EOF)
								cout << "Error in line " << curLine << "Unclosed punctuation mark!" << endl;
							else
								cout << "Error in line " << curLine << " : More than one character between punctuation marks!" << endl;
							exit(-1);
							return false;
						}
					}
					else
					{
						if (f.peek() == EOF)
							cout << "Error in line " << curLine << "Unclosed punctuation mark!" << endl;
						else
							cout << "Error in line " << curLine << " : More than one character between punctuation marks!" << endl;
						exit(-1);
						return false;
					}
				}
				break;
			case('\"'):
				cur_elem.clear();
				c = '\"';
				while (f.peek() != EOF && f.peek() != '\n' && (!(c != '\\' && f.peek() == '\"')))
				{
					cur_elem += c;
					c = (char)f.get();
				}

				if (f.peek() == EOF || f.peek() == '\n')
				{
					cout << "Error in line " << curLine << "Unclosed Quotation mark!" << endl;
					exit(-1);
					return false;
				}
				else
				{
					cur_elem += c;
					cur_elem += (char)f.get();
					LexResult.push_back({ curLine,cur_elem,LexComponent::ConstString });
				}
				break;
			default:
				if (c < 0 || c>255)
				{
					cout << "Error in line " << curLine << "Undefined char!" << endl;
					exit(-1);
					return false;
				}
				if (isdigit(c))
				{
					cur_elem.clear();
					cur_elem += c;
					while (isdigit((char)f.peek()))
					{
						cur_elem += (char)f.get();
					}
					LexResult.push_back({ curLine,cur_elem,LexComponent::Digit });
				}
				else if (isalpha(c) || c == '_')
				{
					cur_elem.clear();
					cur_elem += c;
					while (isalnum((char)f.peek()) || (char)f.peek() == '_')
					{
						cur_elem += (char)f.get();
					}
					if (LexMap.find(cur_elem) != LexMap.end())
						LexResult.push_back({ curLine,cur_elem,LexMap[cur_elem.c_str()] });
					else //说明不属于已有的函数或其他命名，是自定义的名字
					{
						bool flag = false; // 表明在符号表中是否存在
						for (map<int, string>::iterator it = this->NameTable.begin(); it != this->NameTable.end(); it++)
						{
							if (it->second == cur_elem)
							{
								flag = true; //表明已经存在
								break;
							}
						}
						if (!flag)
						{
							NameTable.insert(pair<int, string>(this->symbol_count++, cur_elem));
						}
						LexResult.push_back({ curLine,cur_elem,LexComponent::ID });
					}
				}
		}
	}
	resultNum = LexResult.size();
	getResult();
	return true;
}

void LexAnalyzer::getResult()
{
	for (int i = 0; i < LexResult.size(); i++)
	{
		Token tk = LexResult[i];
		Symbol sy;
		sy.symbol_type = VT;
		sy.content = string(ComponentName[(int)LexResult[i].type]);
		Result.push({ tk,sy });
	}
}

int LexAnalyzer::getcount(const string elem)
{

	int ret = -1;
	for (map<int, string>::iterator it = this->NameTable.begin(); it != this->NameTable.end(); it++)
	{
		if (it->second == elem)
		{
			ret = it->first; //表明已经存在
			return ret;
		}
	}
	//不存在，说明不是ID符号，
	//是否是数字,如果是，则ret为数字
	if (elem.find_first_not_of("0123456789") == string::npos)
		ret = stoi(elem);

	//直接返回-1
	return ret;
}

void LexAnalyzer::output_result(string outFile)
{
	ofstream f(outFile, ios::out);
	if (f.fail())
	{
		cout << "Error: Fail to open file " << outFile << " !" << endl;
		exit(-1);
		return;
	}
	f << setw(45) << "TOKEN STREAM" << endl;
	f << "-------------------------------------------------------------------" << endl;
	f << "Line";
	f << "\t";
	f << "Content";
	f << "\t\t\t";
	f << "Type";
	f << endl;

	for (int i = 0; i < resultNum; i++) {
		f << LexResult[i].line;
		f << "\t";
		f << LexResult[i].content;
		f << "\t\t\t";
		f << ComponentName[(int)LexResult[i].type];
		f << endl;
	}
}

