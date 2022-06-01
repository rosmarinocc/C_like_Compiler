#include "Semantic.h"

SemanticAnalyzer::SemanticAnalyzer()
{
    temp_counter = -1;
    last_table=NULL; //指向最后的表项
    global_table=NULL;//程序所有符号表 
}

SemanticAnalyzer::~SemanticAnalyzer()
{
    symbolTable* tp = last_table;
    symbolTable* ntp = NULL;
    while (tp != NULL)
    {
        ntp = tp->previous;
        free(tp);
        tp = ntp;
    }
}

void SemanticAnalyzer::printQuadruple(const string outpath)
{
	ofstream fout(outpath, ios::out);

    for (auto i = 0; i < this->Quadruple_Code.size(); i++)
    {
        fout << "(" << i << ")\t" << Quadruple_Code[i].op << '\t' << Quadruple_Code[i].arg1 << '\t' << Quadruple_Code[i].arg2 << '\t' << Quadruple_Code[i].res << endl;
    }
	fout.close();
}

string SemanticAnalyzer::newtemp()
{
	string temp_name = string("V") + to_string(offset_stack.back());
	table_stack.back()->enter(temp_counter--, INT, VAR, offset_stack.back());
	offset_stack.back() += 4;

	AddQCode("+", "$sp", to_string(4), "$sp");
	return temp_name;
}

string SemanticAnalyzer::lookup(int id)
{
	symbolTable* tp = table_stack.back();
	int offset;
	while (tp)
	{
		for (auto i = 0; i < tp->table.size(); i++)
		{
			if (tp->table[i].id == id)
			{
				offset = tp->table[i].offset;
				if (tp->table[i].k == VAR || tp->table[i].k == ARRAY)
				{
					if (tp->parent)
						return string("V") + to_string(offset);
					else
						return string("G") + to_string(offset);
				}
			}
		}
		tp = tp->parent;
	}
	return "";
}

symbolTableItem* SemanticAnalyzer::find(int id)
{
	symbolTable* tp = table_stack.back();
	int offset = -1;
	while (tp)
	{
		for (auto i = 0; i < tp->table.size(); i++)
		{
			if (tp->table[i].id == id)
			{
				return &(tp->table[i]);
			}
		}
		tp = tp->parent;
	}
	return NULL;
}

int SemanticAnalyzer::nextstat()
{
	return Quadruple_Code.size();
}

void SemanticAnalyzer::AddQCode(string op, string arg1, string arg2, string result)
{
//op :
//nop j jal break ret jnz j< j<= j> j>= j== j!= 
// 
//:= []= =[] + - & | ^ 

	Quadruple quad = { op,arg1,arg2,result };
	Quadruple_Code.push_back(quad);
}

/*分析主体函数*/
void SemanticAnalyzer::seman_analysis(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	//<Program> -> <Pro_m> <DeclareString>
	if (pro.Left.content == "<Program>")
		SemanProd_Program(pro,root,nameTable);
	//<Pro_m> ->[z]
	else if (pro.Left.content == "<Pro_m>")
		SemanProd_Pro_m(pro,root,nameTable);
	//<DeclareString> -> <DeclareString - option> 
	//				   | <DeclareString - option> <DeclareString>
	else if (pro.Left.content == "<DeclareString>")
		SemanProd_DeclareString(pro,root,nameTable);
	//<DeclareString-option> -> [INT] [ID] <ParameterContent> [SEMI]
	//						  | <FunctionDeclare> <FunDec> <Block>
	else if (pro.Left.content == "<DeclareString-option>")
		SemanProd_DecOption(pro,root,nameTable);
	//<ParameterContent> -> <ArrayDeclare> | [z]
	else if (pro.Left.content == "<ParameterContent>")
		SemanProd_ParaContent(pro,root,nameTable);
	//<ArrayDeclare> -> [LeftSquareBracket] [NUM] [RightSquareBracket] <ArrayDeclare> | [LeftSquareBracket] [NUM] [RightSquareBracket] 
	else if (pro.Left.content == "<ArrayDeclare>")
		SemanProd_ArrayDeclare(pro,root,nameTable);
	/*<FunctionDeclare> ->[VOID] | [INT]*/
	else if (pro.Left.content == "<FunctionDeclare>")
		SemanProd_FunctionDeclare(pro,root,nameTable);
	//<FunDec> ->[ID] <CreateFunTable_m> [LPAREN] <VarContent> [RPAREN]
	else if (pro.Left.content == "<FunDec>")
		SemanProd_FunDec(pro,root,nameTable);
	/*<CreateFunTable_m> ->[z]*/
	else if (pro.Left.content == "<CreateFunTable_m>")
		SemanProd_CreateFunTable_m(pro,root,nameTable);
	//<VarContent> -> <VarList> | [VOID] | [z]
	else if (pro.Left.content == "<VarContent>")
		SemanProd_VarContent(pro,root,nameTable);
	//<VarList> -> [INT] [ID] [COMMA] <VarList> | [INT] [ID]
	else if (pro.Left.content == "<VarList>")
		SemanProd_VarList(pro,root,nameTable);
	//<Block> ->[LBBRACKET] <DefList> <StmtList>[RBBRACKET]
	else if (pro.Left.content == "<Block>")
		SemanProd_Block(pro,root,nameTable);
	//<DefList> -> <Def> [SEMI] <DefList> | [z]
	else if (pro.Left.content == "<DefList>")
		SemanProd_DefList(pro,root,nameTable);
	//<Def> ->[INT][ID] <ParameterContent>
	else if (pro.Left.content == "<Def>")
		SemanProd_Def(pro,root,nameTable);
	//<StmtList> -> <Stmt> <StmtList> | <Stmt> 
	else if (pro.Left.content == "<StmtList>")
		SemanProd_StmtList(pro,root,nameTable);
	//<Stmt> -> <AssignStmt>[SEMI] | <ReturnStmt>[SEMI] | <IfStmt> | <WhileStmt>
	else if (pro.Left.content == "<Stmt>")
		SemanProd_Stmt(pro,root,nameTable);
	//<AssignStmt> ->[ID][ASSIGN] <Exp> | <Array>[ASSIGN] <Exp>
	else if (pro.Left.content == "<AssignStmt>")
		SemanProd_AssignStmt(pro,root,nameTable);
	//<Exp> -> <ANDExp> | <ANDExp> [OR] <Exp>
	else if (pro.Left.content == "<Exp>")
		SemanProd_Exp(pro,root,nameTable);
	//<ANDExp> -> <NOTExp> | <NOTExp>[AND] <ANDExp>
	else if (pro.Left.content == "<ANDExp>")
		SemanProd_ANDExp(pro,root,nameTable);
	//<NOTExp> ->[NOT] <SubExp> | <SubExp>
	else if (pro.Left.content == "<NOTExp>")
		SemanProd_NOTExp(pro,root,nameTable);
	//<SubExp> -> <OPNUM> | <OPNUM> <Relop> <SubExp> 
	else if (pro.Left.content == "<SubExp>")
		SemanProd_SubExp(pro,root,nameTable);
	//<Relop> ->[GT] | [LT] | [GE] | [LE] | [EQ] | [NQ]
	else if (pro.Left.content == "<Relop>")
		SemanProd_Relop(pro,root,nameTable);
	//<OPNUM> -> <Item> | <Item> <OPERAT1> <OPNUM>
	else if (pro.Left.content == "<OPNUM>")
		SemanProd_OPNUM(pro,root,nameTable);
	//<Item> -> <Factor> | <Factor> <OPERAT2> <Item>
	else if (pro.Left.content == "<Item>")
		SemanProd_Item(pro,root,nameTable);
	//<OPERAT1> ->[PLUS] | [SUB] | [BinaryAnd] | [BinaryOr] | [BinaryXor]
	else if (pro.Left.content == "<OPERAT1>")
		SemanProd_OPERAT1(pro,root,nameTable);
	//<OPERAT2> -> [MUL] | [DIV] 
	else if (pro.Left.content == "<OPERAT2>")
		SemanProd_OPERAT2(pro,root,nameTable);
	//<Factor> -> [NUM] | [LPAREN] <Exp> [RPAREN] | [ID] | [ID] <CallStmt> | <Array> | [LPAREN] <AssignStmt> [RPAREN] 
	else if (pro.Left.content == "<Factor>")
		SemanProd_Factor(pro,root,nameTable);
	//<Array> -> [ID] [LeftSquareBracket] <Exp> [RightSquareBracket] | <Array> [LeftSquareBracket] <Exp> [RightSquareBracket] 
	else if (pro.Left.content == "<Array>")
		SemanProd_Array(pro,root,nameTable);
	//<CallStmt> -> [LPAREN] <CallFun> [RPAREN]
	else if (pro.Left.content == "<CallStmt>")
		SemanProd_CallStmt(pro,root,nameTable);
	//<CallFun> ->[z] | <Args>
	else if (pro.Left.content == "<CallFun>")
		SemanProd_CallFun(pro,root,nameTable);
	//<Args> -> <Exp>[COMMA] <Args> | <Exp>
	else if (pro.Left.content == "<Args>")
		SemanProd_Args(pro,root,nameTable);
	//<ReturnStmt> -> [RETURN] <Exp> | [RETURN]
	else if (pro.Left.content == "<ReturnStmt>")
		SemanProd_ReturnStmt(pro,root,nameTable);
	//<WhileStmt> -> <WhileStmt_m1> [WHILE] [LPAREN] <Exp> [RPAREN] <WhileStmt_m2> <Block>
	else if (pro.Left.content == "<WhileStmt>")
		SemanProd_WhileStmt(pro,root,nameTable);
	//<WhileStmt_m1> ->[z]
	else if (pro.Left.content == "<WhileStmt_m1>")
		SemanProd_WhileStmt_m1(pro,root,nameTable);
	//<WhileStmt_m2> ->[z]
	else if (pro.Left.content == "<WhileStmt_m2>")
		SemanProd_WhileStmt_m2(pro,root,nameTable);
	//<IfStmt> -> [IF] [LPAREN] <Exp_stmt> [RPAREN] <IfStmt_m> <Block> <IfNext>
	else if (pro.Left.content == "<IfStmt>")
		SemanProd_IfStmt(pro,root,nameTable);
	//<Exp_stmt> -> <Exp>
	else if (pro.Left.content == "<Exp_stmt>")
		SemanProd_Exp_stmt(pro, root, nameTable);
	//<IfStmt_m> -> [z]
	else if (pro.Left.content == "<IfStmt_m>")
		SemanProd_IfStmt_m1(pro,root,nameTable);
	//<IfNext> -> [z] | [ELSE] <IfStmt_next> <Block>
	else if (pro.Left.content == "<IfNext>")
		SemanProd_IfNext(pro,root,nameTable);
	//<IfStmt_next>-> [z]
	else if (pro.Left.content == "<IfStmt_next>")
		SemanProd_IfStmt_next(pro,root,nameTable);
	else 
	{
		cout << "There is no symbol like [" << pro.Left.content << "]" << endl;
	}
}

void SemanticAnalyzer::SemanProd_Program(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	global_table = table_stack.back();
	table_stack.pop_back();
	global_table->width = offset_stack.back();
	offset_stack.pop_back();
}

void SemanticAnalyzer::SemanProd_Pro_m(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	root->quad = nextstat();
	symbolTable* t = new(nothrow)symbolTable;
	if (!t) { exit(-3); }
	if (last_table != NULL)
	{
		last_table->next = t;
		t->previous = last_table;
	}
	if (!table_stack.empty())
	{
		t->parent = table_stack.back();
	}
	last_table = t;
	table_stack.push_back(t);
	offset_stack.push_back(0);
}

void SemanticAnalyzer::SemanProd_DeclareString(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	//Is here still anything to be done?
}

void SemanticAnalyzer::SemanProd_DecOption(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	Symbol first_symmpro = pro.Right[0];
	//[INT] [ID] <ParameterDeclare> [SEMI]
	if (first_symmpro.content == "[INT]")
	{
		root->type = INT;
		root->kind = root->children[2]->kind;
		root->n = root->children[2]->n;
		root->width = 4 * root->n;
		table_stack.back()->enter(root->children[1]->content.second, type(root->type), kind(root->kind), offset_stack.back());
		if (root->kind == ARRAY)
		{
			root->dimension = root->children[2]->dimension;
			table_stack.back()->enterdimension(root->children[1]->content.second, root->dimension);
		}
		offset_stack.back() += root->width;
	}
	//<FunctionDeclare> <FunDec> <Block>
	else if (first_symmpro.content == "<FunctionDeclare>")
	{
		root->type = root->children[0]->type;
		root->kind = root->children[0]->kind;
		root->children[1]->type = root->children[0]->type;

		
		root->n = root->children[1]->n;
		root->width = root->children[1]->width;

		

		symbolTable* t = table_stack.back();
		table_stack.pop_back();
		t->width = t->table.empty() ? 0 : offset_stack.back() - t->table[0].offset;
		offset_stack.pop_back();
	}
	else
	{
		exit(-3);
	}
}

void SemanticAnalyzer::SemanProd_ParaContent(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	Symbol first_symmpro = pro.Right[0];
	//<ParameterContent> ->[z]
	if (first_symmpro.content == "[z]")
	{
		root->kind = VAR;
		root->n = 1;
	}
	//<ParameterContent> -> <ArrayDeclare>
	else if (first_symmpro.content == "<ArrayDeclare>")
	{
		root->kind = ARRAY;
		root->n = root->children[0]->n;
		root->dimension = root->children[0]->dimension;
		reverse(root->dimension.begin(), root->dimension.end());
	}
}

void SemanticAnalyzer::SemanProd_ArrayDeclare(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	int short_long = int(pro.Right.size());
	//[LeftSquareBracket] [NUM] [RightSquareBracket] 
	if (short_long == 3)
	{
		root->kind = ARRAY;
		root->n = root->children[1]->content.second;
		root->dimension.push_back(root->children[1]->content.second);
	}
	// [LeftSquareBracket] [NUM] [RightSquareBracket] <ArrayDeclare>
	else if(short_long == 4)
	{
		root->kind = ARRAY;
		root->n = root->children[1]->content.second * root->children[3]->n;
		root->dimension = root->children[3]->dimension;
		root->dimension.push_back(root->children[1]->content.second);
	}
}

void SemanticAnalyzer::SemanProd_FunctionDeclare(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	//Is here still anything to be done?
	Symbol first_symmpro = pro.Right[0];
	//<FunctionDeclare> ->[VOID] | [INT]
	if (first_symmpro.content == "[VOID]")
	{
		root->type = VOID;
		root->kind = FUNC;
	}
	else if (first_symmpro.content == "[INT]")
	{
		root->type = INT;
		root->kind = FUNC;
	}
}

void SemanticAnalyzer::SemanProd_FunDec(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	root->kind = FUNC;
	root->n = 1;
	root->width = -1 * root->n;

	symbolTable* new_table = table_stack.back();
	table_stack.pop_back();
	int new_offset = offset_stack.back();
	offset_stack.pop_back();

	table_stack.back()->enter(root->children[0]->content.second, type(root->type), kind(root->kind), root->children[1]->quad);
	offset_stack.back() += 0;
	table_stack.back()->enterproc(root->children[0]->content.second, new_table);
	table_stack.back()->enterdimension(root->children[0]->content.second, root->children[3]->dimension);

	table_stack.push_back(new_table);
	offset_stack.push_back(new_offset);
}


void SemanticAnalyzer::SemanProd_CreateFunTable_m(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	this->SemanProd_Pro_m(pro,root,nameTable);
}

void SemanticAnalyzer::SemanProd_VarContent(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	Symbol first_symmpro = pro.Right[0];
	//<VarList> | [VOID] | [z]
	if (first_symmpro.content == "[VOID]")
	{
		root->dimension.push_back(0);
	}
	else if (first_symmpro.content == "[z]")
	{
		root->dimension.push_back(0);
	}
	else if (first_symmpro.content == "<VarList>")
	{
		root->dimension = root->children[0]->dimension;
	}
}

void SemanticAnalyzer::SemanProd_VarList(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	int short_long = int(pro.Right.size());
	//[INT] [ID]
	if (short_long == 2)
	{
		root->type = INT;
		root->kind = VAR;
		root->n = 1;
		root->width = 4 * root->n;
		root->dimension.push_back(1);
		table_stack.back()->enter(root->children[1]->content.second, type(root->type), kind(root->kind), offset_stack.back());
		offset_stack.back() += root->width;
	}
	// [INT] [ID] [COMMA] <VarList> 
	else if (short_long == 4)
	{
		root->type = INT;
		root->kind = VAR;
		root->n = 1;
		root->width = 4 * root->n;
		root->dimension = root->children[3]->dimension;
		root->dimension[0] += 1;
		table_stack.back()->enter(root->children[1]->content.second, type(root->type), kind(root->kind), offset_stack.back());
		offset_stack.back() += root->width;
	}
}
//
//void SemanticAnalyzer::SemanProd_ParamDec(const Production& pro, IdentNode* root, map<int, string> nameTable)
//{
//	root->type = INT;
//	root->kind = VAR;
//	root->n = 1;
//	root->width = 4 * root->n;
//	table_stack.back()->enter(root->children[1]->content.second, root->type, root->kind, offset_stack.back());
//	offset_stack.back() += root->width;
//	EMIT("+", "$sp", to_string(root->width), "$sp");
//}

void SemanticAnalyzer::SemanProd_Block(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	//Is here still anything to be done?
}

void SemanticAnalyzer::SemanProd_DefList(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	//Is here still anything to be done?
}

void SemanticAnalyzer::SemanProd_Def(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	root->type = INT;
	root->kind = root->children[2]->kind;
	root->n = root->children[2]->n;
	root->width = 4 * root->n;
	root->dimension = root->children[2]->dimension;
	reverse(root->dimension.begin(), root->dimension.end());
	table_stack.back()->enter(root->children[1]->content.second, type(root->type), kind(root->kind), offset_stack.back());
	table_stack.back()->enterdimension(root->children[1]->content.second, root->dimension);
	offset_stack.back() += root->width;
	AddQCode("+", "$sp", to_string(root->width), "$sp");
}

void SemanticAnalyzer::SemanProd_StmtList(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	//Is here still anything to be done?
}

void SemanticAnalyzer::SemanProd_Stmt(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	//Is here still anything to be done?
}

void SemanticAnalyzer::SemanProd_AssignStmt(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	Symbol first_symmpro = pro.Right[0];
	//[ID][ASSIGN] <Exp> | <Array>[ASSIGN] <Exp>
	if (first_symmpro.content == "[ID]")
	{
		string p = lookup(root->children[0]->content.second);
		if (p == "")
		{
			throw string("ERROR: 语义分析器错误:") + nameTable[root->children[0]->content.second] + string("未定义\n");
		}
		else
		{
			AddQCode(":=", root->children[2]->place, "", p);
			root->place = newtemp();
			AddQCode(":=", root->children[2]->place, "", root->place);
		}
	}
	else if (first_symmpro.content == "<Array>")
	{
		if (root->children[0]->dimension.size() != 1)
		{
			throw string("ERROR: 语义分析器错误:遇到不完整的数组索引\n");
		}
		string p = lookup(root->children[0]->content.second);
		if (p == "")
		{

			throw string("ERROR: 语义分析器错误:") + nameTable[root->children[0]->content.second] + string("未定义\n");
		}
		else
		{
			AddQCode("[]=", root->children[2]->place, root->children[0]->place, p);
			root->place = newtemp();
			AddQCode(":=", root->children[2]->place, "", root->place);
		}
	}
}

void SemanticAnalyzer::SemanProd_Exp(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	int short_long = int(pro.Right.size());
	// <ANDExp>
	if (short_long == 1)
	{
		root->place = root->children[0]->place;
	}
	// <ANDExp> [OR] <Exp>
	else if (short_long == 3)
	{
		root->place = newtemp();
		AddQCode("jnz", root->children[0]->place, "", to_string(nextstat() + 4));
		AddQCode("jnz", root->children[2]->place, "", to_string(nextstat() + 3));
		AddQCode(":=", to_string(0), "", root->place);
		AddQCode("j", "", "", to_string(nextstat() + 2));
		AddQCode(":=", to_string(1), "", root->place);
	}
}

void SemanticAnalyzer::SemanProd_ANDExp(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	int short_long = int(pro.Right.size());
	// <NOTExp>
	if (short_long == 1)
	{
		root->place = root->children[0]->place;
	}
	// <NOTExp>[AND] <ANDExp>
	else if (short_long == 3)
	{
		root->place = newtemp();
		AddQCode("jnz", root->children[0]->place, "", to_string(nextstat() + 2));
		AddQCode("j", "", "", to_string(nextstat() + 2));
		AddQCode("jnz", root->children[2]->place, "", to_string(nextstat() + 3));
		AddQCode(":=", to_string(0), "", root->place);
		AddQCode("j", "", "", to_string(nextstat() + 2));
		AddQCode(":=", to_string(1), "", root->place);
	}
}

void SemanticAnalyzer::SemanProd_NOTExp(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	int short_long = int(pro.Right.size());
	// <SubExp>
	if (short_long == 1)
	{
		root->place = root->children[0]->place;
	}
	//[NOT] <SubExp>
	else if (short_long == 2)
	{
		root->place = newtemp();
		AddQCode("jnz", root->children[1]->place, "", to_string(nextstat() + 3));
		AddQCode(":=", to_string(1), "", root->place);
		AddQCode("j", "", "", to_string(nextstat() + 2));
		AddQCode(":=", to_string(0), "", root->place);
	}
}

void SemanticAnalyzer::SemanProd_SubExp(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	int short_long = int(pro.Right.size());
	//  <OPNUM> | 
	if (short_long == 1)
	{
		root->place = root->children[0]->place;
	}
	//<OPNUM> <Relop> <SubExp> 
	else if (short_long == 3)
	{
		root->place = newtemp();
		string relop = root->children[1]->content.first;
		//[GT] | [LT] | [GE] | [LE] | [EQ] | [NQ]
		if (relop == "[LT]")
		{
			AddQCode("j<", root->children[0]->place, root->children[2]->place, to_string(nextstat() + 3)); 
		}
		else if (relop == "[LE]")
		{
			AddQCode("j<=", root->children[0]->place, root->children[2]->place, to_string(nextstat() + 3));
		}
		else if (relop == "[GT]")
		{
			AddQCode("j>", root->children[0]->place, root->children[2]->place, to_string(nextstat() + 3)); 
		}
		else if (relop == "[GE]")
		{
			AddQCode("j>=", root->children[0]->place, root->children[2]->place, to_string(nextstat() + 3));
		}
		else if (relop == "[EQ]")
		{
			AddQCode("j==", root->children[0]->place, root->children[2]->place, to_string(nextstat() + 3));
		}
		else if (relop == "[NQ]")
		{
			AddQCode("j!=", root->children[0]->place, root->children[2]->place, to_string(nextstat() + 3));
		}
		AddQCode(":=", to_string(0), "", root->place);
		AddQCode("j", "", "", to_string(nextstat() + 2));
		AddQCode(":=", to_string(1), "", root->place);
	}
}

void SemanticAnalyzer::SemanProd_Relop(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	//<relop> -> [GT] | [LT] | [GE] | [LE] | [EQ] | [NQ]
	root->content = root->children[0]->content;
}

void SemanticAnalyzer::SemanProd_OPNUM(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	//[PLUS] | [SUB] | [BinaryAnd] | [BinaryOr] | [BinaryXor]
	int short_long = int(pro.Right.size());
	// <Item>
	if (short_long == 1)
	{
		root->place = root->children[0]->place;
	}
	// <Item> <OPERAT1> <OPNUM>
	//[PLUS] | [SUB] | [BinaryAnd] | [BinaryOr] | [BinaryXor]
	else if (short_long == 3)
	{
		root->place = newtemp();
		string op1 = root->children[1]->content.first;
		if (op1 == "[PLUS]")
		{
			AddQCode("+", root->children[0]->place, root->children[2]->place, root->place);
		}
		else if (op1 == "[SUB]")
		{
			AddQCode("-", root->children[0]->place, root->children[2]->place, root->place);
		}
		else if (op1 == "[BinaryAnd]")
		{
			AddQCode("&", root->children[0]->place, root->children[2]->place, root->place);
		}
		else if (op1 == "[BinaryXor]")
		{
			AddQCode("|", root->children[0]->place, root->children[2]->place, root->place);
		}
		else if (op1 == "[BinaryOr]")
		{
			AddQCode("^", root->children[0]->place, root->children[2]->place, root->place);
		}
	}
}

void SemanticAnalyzer::SemanProd_Item(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	int short_long = int(pro.Right.size());
	// <Factor> 
	if (short_long == 1)
	{
		root->place = root->children[0]->place;
	}
	// <Factor> <OPERAT2> <Item>
	// [MUL] | [DIV] 
	else if (short_long == 3)
	{
		root->place = newtemp();
		string op2 = root->children[1]->content.first;
		if (op2 == "[MUL]")
		{
			AddQCode("*", root->children[0]->place, root->children[2]->place, root->place);
		}
		else if (op2 == "[DIV]")
		{
			AddQCode("/", root->children[0]->place, root->children[2]->place, root->place);
		}
	}
}

void SemanticAnalyzer::SemanProd_OPERAT1(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	//[PLUS] | [SUB] | [BinaryAnd] | [BinaryOr] | [BinaryXor]
	root->content = root->children[0]->content;
}

void SemanticAnalyzer::SemanProd_OPERAT2(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	// [MUL] | [DIV] 
	root->content = root->children[0]->content;
}

void SemanticAnalyzer::SemanProd_Factor(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	int short_long = int(pro.Right.size());
	string first_sym = pro.Right[0].content;
	switch (short_long)
	{
		//[NUM] | [ID] | <Array>
		case 1:
		{
			if (first_sym == "[NUM]")
			{
				root->place = newtemp();
				AddQCode(":=", to_string(root->children[0]->content.second), "", root->place);
			}
			else if (first_sym == "[ID]")
			{
				string p = lookup(root->children[0]->content.second);
				if (p == "")
				{
					throw string("ERROR: 语义分析器错误:") + nameTable[root->children[0]->content.second] + string("未定义\n");
				}
				else
				{
					root->place = p;
				}
			}
			else if (first_sym == "<Array>")
			{
				if (root->children[0]->dimension.size() != 1)
				{
					throw string("ERROR: 语义分析器错误:遇到不完整的数组索引\n");
				}
				string p = lookup(root->children[0]->content.second);
				if (p == "")
				{
					throw string("ERROR: 语义分析器错误:") + nameTable[root->children[0]->content.second] + string("未定义\n");
				}
				else
				{
					root->place = newtemp();
					AddQCode("=[]", p, root->children[0]->place, root->place);
				}
			}

			break;
		}
		//[ID] <CallStmt>
		case 2:
		{
			symbolTableItem* f = find(root->children[0]->content.second);
			if (f == NULL)
			{
				throw string("ERROR: 语义分析器错误:") + nameTable[root->children[0]->content.second] + string("未定义\n");
			}
			if (f->dimension[0] != root->children[1]->params.size())
			{
				throw string("ERROR: 语义分析器错误:调用过程 ") + nameTable[f->id] + string(" 需要实参: ") + to_string(f->dimension[0]) + string(" 个, 实际给出: ") + to_string(root->children[1]->params.size()) + string("个\n");
			}

			AddQCode(":=", "$ra", "", "[$sp]");
			AddQCode("+", "$sp", to_string(4), "$sp");
			AddQCode(":=", "$t0", "", "[$sp]");
			AddQCode("+", "$sp", to_string(4), "$sp");
			AddQCode(":=", "$t1", "", "[$sp]");
			AddQCode("+", "$sp", to_string(4), "$sp");
			AddQCode(":=", "$t2", "", "[$sp]");
			AddQCode("+", "$sp", to_string(4), "$sp");
			AddQCode(":=", "$t3", "", "[$sp]");
			AddQCode("+", "$sp", to_string(4), "$sp");
			AddQCode(":=", "$t4", "", "[$sp]");
			AddQCode("+", "$sp", to_string(4), "$sp");
			AddQCode(":=", "$t5", "", "[$sp]");
			AddQCode("+", "$sp", to_string(4), "$sp");
			AddQCode(":=", "$t6", "", "[$sp]");
			AddQCode("+", "$sp", to_string(4), "$sp");
			AddQCode(":=", "$t7", "", "[$sp]");
			AddQCode("+", "$sp", to_string(4), "$sp");

			//string t = newtemp();
			AddQCode(":=", "$sp", "", "$s0");
			AddQCode(":=", "$fp", "", "[$sp]");
			AddQCode("+", "$sp", to_string(4), "$sp");
			AddQCode(":=", "$s0", "", "$fp");

			for (auto i = 0; i < root->children[1]->params.size(); i++)
			{
				AddQCode(":=", root->children[1]->params[i], "", "[$sp]");
				AddQCode("+", "$sp", to_string(4), "$sp");
			}
			AddQCode("jal", "", "", to_string(f->offset));
			AddQCode(":=", "$fp", "", "$sp");
			AddQCode(":=", "[$sp]", "", "$fp");

			AddQCode("-", "$sp", to_string(4), "$sp");
			AddQCode(":=", "[$sp]", "", "$t7");
			AddQCode("-", "$sp", to_string(4), "$sp");
			AddQCode(":=", "[$sp]", "", "$t6");
			AddQCode("-", "$sp", to_string(4), "$sp");
			AddQCode(":=", "[$sp]", "", "$t5");
			AddQCode("-", "$sp", to_string(4), "$sp");
			AddQCode(":=", "[$sp]", "", "$t4");
			AddQCode("-", "$sp", to_string(4), "$sp");
			AddQCode(":=", "[$sp]", "", "$t3");
			AddQCode("-", "$sp", to_string(4), "$sp");
			AddQCode(":=", "[$sp]", "", "$t2");
			AddQCode("-", "$sp", to_string(4), "$sp");
			AddQCode(":=", "[$sp]", "", "$t1");
			AddQCode("-", "$sp", to_string(4), "$sp");
			AddQCode(":=", "[$sp]", "", "$t0");
			AddQCode("-", "$sp", to_string(4), "$sp");
			AddQCode(":=", "[$sp]", "", "$ra");

			root->place = newtemp();
			AddQCode(":=", "$v0", "", root->place);
			break;
		}
		// [LPAREN] <Exp> [RPAREN] | [LPAREN] <AssignStmt> [RPAREN] 
		case 3:
		{
			root->place = root->children[1]->place;
			break;
		}
		default:
		{
			break;
		}
	}

}

void SemanticAnalyzer::SemanProd_Array(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	string first_sym = pro.Right[0].content;
	//[ID] [LeftSquareBracket] <Exp> [RightSquareBracket] 
	if (first_sym == "[ID]")
	{
		symbolTableItem* e = find(root->children[0]->content.second);
		if (e == NULL)
		{
			throw string("ERROR: 语义分析器错误:") + nameTable[root->children[0]->content.second] + string(" 未定义\n");
		}
		root->content = root->children[0]->content;
		root->kind = ARRAY;
		root->dimension = e->dimension;

		if (root->dimension.size() == 0)
		{
			throw string("ERROR: 语义分析器错误:数组下标错误");
		}
		else if (root->dimension.size() == 1)
		{
			root->place = root->children[2]->place;
		}
		else
		{
			int dim_len = root->dimension[1];
			for (auto i = 2; i < root->dimension.size(); i++)
			{
				dim_len *= root->dimension[i];
			}
			string p = newtemp();
			AddQCode(":=", to_string(dim_len), "", p);
			root->place = newtemp();
			AddQCode("*", p, root->children[2]->place, root->place);
		}
	}
	//| <Array> [LeftSquareBracket] <Exp> [RightSquareBracket] 
	else if (first_sym == "<Array>")
	{
		root->content = root->children[0]->content;
		root->kind = ARRAY;
		root->dimension = root->children[0]->dimension;
		root->dimension.erase(root->dimension.begin());

		if (root->dimension.size() == 0)
		{
			throw string("ERROR: 语义分析器错误:数组下标错误");
		}
		else if (root->dimension.size() == 1)
		{
			root->place = newtemp();
			AddQCode("+", root->children[0]->place, root->children[2]->place, root->place);
		}
		else
		{
			int dim_len = root->dimension[1];
			for (auto i = 2; i < root->dimension.size(); i++)
			{
				dim_len *= root->dimension[i];
			}
			string p1 = newtemp();
			AddQCode(":=", to_string(dim_len), "", p1);
			string p2 = newtemp();
			AddQCode("*", p1, root->children[2]->place, p2);
			root->place = newtemp();
			AddQCode("+", root->children[0]->place, p2, root->place);
		}
	}
}

void SemanticAnalyzer::SemanProd_CallStmt(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	//[LPAREN] <CallFun>[RPAREN]
	root->params = root->children[1]->params;
}

void SemanticAnalyzer::SemanProd_CallFun(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	string first_sym = pro.Right[0].content;
	if (first_sym == "[z]")
	{
		// nothing to do yet
		return;
	}
	else if (first_sym == "<Args>")
	{
		root->params = root->children[0]->params;
	}
}

void SemanticAnalyzer::SemanProd_Args(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	int first_len = int(pro.Right.size());
	//<Exp>
	if (first_len == 1)
	{
		root->params.push_back(root->children[0]->place);
	}
	//<Exp>[COMMA] <Args>
	else if (first_len == 3)
	{
		root->params = root->children[2]->params;
		root->params.push_back(root->children[0]->place);
	}
}

void SemanticAnalyzer::SemanProd_ReturnStmt(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	int short_long = int(pro.Right.size());
	//[RETURN]
	if (short_long==1)
	{
		AddQCode(":=", to_string(0), "", "$v0");
		AddQCode("ret", "", "", "");
	}
	//[RETURN] <Exp> | 
	else if (short_long == 2)
	{
		AddQCode(":=", root->children[1]->place, "", "$v0");
		AddQCode("ret", "", "", "");
	}
}

void SemanticAnalyzer::SemanProd_WhileStmt(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	//<WhileStmt_m1> [WHILE] [LPAREN] <Exp_stmt> [RPAREN] <WhileStmt_m2> <Block>
	symbolTable* t = table_stack.back();
	table_stack.pop_back();
	t->width = t->table.empty() ? 0 : offset_stack.back() - t->table[0].offset;
	offset_stack.pop_back();

	AddQCode("-", "$sp", to_string(t->width), "$sp");

	AddQCode("j", "", "", to_string(root->children[0]->quad));
	Quadruple_Code[root->children[3]->true_list].res = to_string(root->children[5]->quad);
	Quadruple_Code[root->children[3]->false_list].res = to_string(nextstat());
}



void SemanticAnalyzer::SemanProd_WhileStmt_m1(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	root->quad = nextstat();
}

void SemanticAnalyzer::SemanProd_WhileStmt_m2(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	root->quad = nextstat();
	symbolTable* t = new(nothrow)symbolTable;
	if (!t) {throw string("ERROR: 内部错误:bad_alloc创建符号表失败\n"); }
	if (last_table)
	{
		last_table->next = t;
		t->previous = last_table;
	}
	if (!table_stack.empty())
	{
		t->parent = table_stack.back();
	}
	last_table = t;
	table_stack.push_back(t);
	if (offset_stack.empty())
	{
		offset_stack.push_back(0);
	}
	else
	{
		int back_offset = offset_stack.back();
		offset_stack.push_back(back_offset);
	}
}

//<IfStmt> -> [IF] [LPAREN] <Exp_stmt> [RPAREN] <IfStmt_m1> <Block> <IfNext>
void SemanticAnalyzer::SemanProd_IfStmt(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	symbolTable* t = table_stack.back();
	table_stack.pop_back();
	t->width = t->table.empty() ? 0 : offset_stack.back() - t->table[0].offset;
	offset_stack.pop_back();

	AddQCode("-", "$sp", to_string(t->width), "$sp");

	Quadruple_Code[root->children[2]->true_list].res = to_string(root->children[4]->quad);
	
	if (root->children[6]->quad != NULL && root->children[6]->true_list != NULL)
	{
		Quadruple_Code[root->children[2]->false_list].res = to_string(root->children[6]->quad);
		Quadruple_Code[root->children[6]->true_list].res = to_string(nextstat());
	}
	else
	{
		Quadruple_Code[root->children[2]->false_list].res = to_string(nextstat());		
	}
}

//<IfStmt_m1> -> [z]
void SemanticAnalyzer::SemanProd_IfStmt_m1(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	SemanProd_WhileStmt_m2(pro,root,nameTable);
}

//<IfNext> -> [z] | [ELSE] <IfStmt_next> <Block>
void SemanticAnalyzer::SemanProd_IfNext(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	int short_long = int(pro.Right.size());
	// [ELSE] <IfStmt_next> <Block>
	if (short_long ==3)
	{
		root->true_list = root->children[1]->true_list;
		root->quad = root->children[1]->quad;
	}
	// [z]
	else if (short_long == 1)
	{
		root->true_list = NULL;
		root->quad = NULL;
	}
}
void SemanticAnalyzer::SemanProd_Exp_stmt(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	root->true_list = nextstat();
	AddQCode("jnz", root->children[0]->place, "", to_string(0));
	root->false_list = nextstat();
	AddQCode("j", "", "", to_string(0));
}
//<IfStmt_next>-> [z]
void SemanticAnalyzer::SemanProd_IfStmt_next(const Production& pro, IdentNode* root, map<int, string> nameTable)
{
	symbolTable* t = table_stack.back();
	table_stack.pop_back();
	t->width = t->table.empty() ? 0 : offset_stack.back() - t->table[0].offset;
	offset_stack.pop_back();

	AddQCode("-", "$sp", to_string(t->width), "$sp");

	t = new(nothrow)symbolTable;
	if (!t) { throw string("ERROR: 内部错误:bad_alloc创建符号表失败\n"); }
	if (last_table)
	{
		last_table->next = t;
		t->previous = last_table;
	}
	if (!table_stack.empty())
	{
		t->parent = table_stack.back();
	}
	last_table = t;
	table_stack.push_back(t);
	if (offset_stack.empty())
	{
		offset_stack.push_back(0);
	}
	else
	{
		int back_offset = offset_stack.back();
		offset_stack.push_back(back_offset);
	}
	root->true_list = nextstat();
	AddQCode("j", "", "", to_string(0));
	root->quad = nextstat();
}