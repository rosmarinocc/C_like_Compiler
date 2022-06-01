# include "ObjectCodeGen.h"

ObjectCodeGen::ObjectCodeGen(vector<Quadruple> ic, vector<blockItem> bg, int stack_size)
{
	intermediate_code = ic;
	block_group = bg;
	stack_buf_size = stack_size * 1024;
	data_buf_size = stack_size * 1024;
	temp_buf_size = stack_size * 1024;
}
vector<messageTableItem> ObjectCodeGen::geneMessageTable(int block_no)
{
	vector<messageTableItem> message_table;
	map<string, pair<int, bool>> message_link;
	for (auto pos = block_group[block_no].end; pos >= block_group[block_no].begin; pos--)
	{
		Quadruple TAS = intermediate_code[pos];
		messageTableItem new_table_item;
		new_table_item.no = pos;
		new_table_item.TAS = TAS;
		if (TAS.arg1[0] == 'G' || TAS.arg1[0] == 'V' || TAS.arg1[0] == 'T')
		{
			if (message_link.find(TAS.arg1) == message_link.end())
			{
				if (TAS.arg1[0] == 'G' || find(block_group[block_no].wait_variable.begin(), block_group[block_no].wait_variable.end(), TAS.arg1) != block_group[block_no].wait_variable.end())
				{
					message_link[TAS.arg1] = pair<int, bool>(INT_MAX, true);
				}
				else
				{
					message_link[TAS.arg1] = pair<int, bool>(0, false);
				}
			}
			new_table_item.arg1_tag = message_link[TAS.arg1];
			message_link[TAS.arg1] = pair<int, bool>(pos, true);
		}
		if (TAS.arg2[0] == 'G' || TAS.arg2[0] == 'V' || TAS.arg2[0] == 'T')
		{
			if (message_link.find(TAS.arg2) == message_link.end())
			{
				if (TAS.arg2[0] == 'G' || find(block_group[block_no].wait_variable.begin(), block_group[block_no].wait_variable.end(), TAS.arg2) != block_group[block_no].wait_variable.end())
				{
					message_link[TAS.arg2] = pair<int, bool>(INT_MAX, true);
				}
				else
				{
					message_link[TAS.arg2] = pair<int, bool>(0, false);
				}
			}
			new_table_item.arg2_tag = message_link[TAS.arg2];
			message_link[TAS.arg2] = pair<int, bool>(pos, true);
		}
		if (TAS.res[0] == 'G' || TAS.res[0] == 'V' || TAS.res[0] == 'T')
		{
			if (message_link.find(TAS.res) == message_link.end())
			{
				if (TAS.res[0] == 'G' || find(block_group[block_no].wait_variable.begin(), block_group[block_no].wait_variable.end(), TAS.res) != block_group[block_no].wait_variable.end())
				{
					message_link[TAS.res] = pair<int, bool>(INT_MAX, true);
				}
				else
				{
					message_link[TAS.res] = pair<int, bool>(0, false);
				}
			}
			new_table_item.result_tag = message_link[TAS.res];
			message_link[TAS.res] = pair<int, bool>(0, false);
		}
		message_table.push_back(new_table_item);
		messageTableHistory.push_back(new_table_item);
	}
	reverse(message_table.begin(), message_table.end());
	return message_table;
}

void ObjectCodeGen::EMIT(string code)
{
	object_code.push_back(code);
	new_code.push_back(code);
}

string ObjectCodeGen::getREG(string result)
{
	string R;
	bool has_R = false;
	//result本身有寄存器
	if (AVALUE.find(result) != AVALUE.end() && AVALUE[result].size() > 0)
	{
		for (auto i = 0; i < AVALUE[result].size(); i++)
		{
			if (AVALUE[result][i] != "M")
			{
				R = AVALUE[result][i];
				has_R = true;
				break;
			}
		}
	}
	if (!has_R)
	{
		for (map<string, vector<pair<string, int>>>::iterator iter = RVALUE.begin(); iter != RVALUE.end(); iter++)
		{
			if (iter->second.size() == 0)
			{
				R = iter->first;
				return R;
			}
		}
		//choose R which will be used in longest time
		int farthest_R = -1;
		for (map<string, vector<pair<string, int>>>::iterator iter = RVALUE.begin(); iter != RVALUE.end(); iter++)
		{
			int closest_V = INT_MAX;
			for (auto i = 0; i < iter->second.size(); i++)
			{
				if (iter->second[i].second < closest_V)
					closest_V = iter->second[i].second;
			}
			if (closest_V > farthest_R)
			{
				farthest_R = closest_V;
				R = iter->first;
			}
		}
	}
	for (auto i = 0; i < RVALUE[R].size(); i++)
	{
		string V = RVALUE[R][i].first;
		if (AVALUE[V].size() == 1 && AVALUE[V][0] == R)
		{
			//save variable V
			if (V[0] == 'G')
				EMIT("sw " + R + "," + DATA + "+" + to_string(stoi(V.substr(1))));
			else if (V[0] == 'V')
				EMIT("sw " + R + "," + STACK + "+" + to_string(4 + stoi(V.substr(1))) + "($fp)");
			else if (V[0] == 'T')
				EMIT("sw " + R + "," + TEMP + "+" + to_string(4 * stoi(V.substr(1))));
			else
			{
				//cerr << "ERROR: AVALUE中出现意外的变量名:" << V << endl;
				//exit(-1);
				throw string("ERROR: 目标代码生成器错误:AVALUE中出现意外的变量名:") + V + string("\n");
			}
		}
		//delete R from AVALUE
		vector<string>::iterator Ritor = find(AVALUE[V].begin(), AVALUE[V].end(), R);
		AVALUE[V].erase(Ritor);
		//add memroy address to AVALUE
		if (find(AVALUE[V].begin(), AVALUE[V].end(), "M") == AVALUE[V].end())
			AVALUE[V].push_back("M");
	}
	//delete all V from RVALUE
	RVALUE[R].clear();
	return R;
}
void ObjectCodeGen::freshRA(pair<int, bool> tag, string R, string V, bool value_changed)
{
	if (value_changed || !tag.second)
	{
		for (auto i = 0; i < AVALUE[V].size(); i++)
		{
			if (RVALUE.find(AVALUE[V][i]) != RVALUE.end())
			{
				string opR = AVALUE[V][i];
				for (auto j = 0; j < RVALUE[opR].size(); j++)
				{
					if (RVALUE[opR][j].first == V)
					{
						vector<pair<string, int>>::iterator iter = find(RVALUE[opR].begin(), RVALUE[opR].end(), RVALUE[opR][j]);
						RVALUE[opR].erase(iter);
						break;
					}
				}
			}
		}
		if (tag.second)
		{
			AVALUE[V] = vector<string>{ R };
			RVALUE[R].push_back(pair<string, int>(V, tag.first));
		}
		else
		{
			AVALUE.erase(V);
		}
	}
	else
	{
		bool is_find = false;
		//在R的记录中寻找V
		for (auto i = 0; i < RVALUE[R].size(); i++)
		{
			if (RVALUE[R][i].first == V)
			{
				//找到V后更新V
				is_find = true;
				RVALUE[R][i].second = tag.first;
				break;
			}
		}
		//没找到V添加V
		if (!is_find)
			RVALUE[R].push_back(pair<string, int>(V, tag.first));

		//V是否存在于AVALUE
		if (AVALUE.find(V) == AVALUE.end())
		{
			//V不存在于AVALUE中则新建V
			AVALUE[V] = vector<string>{ R };
		}
		else
		{
			//V在AVALUE中
			if (find(AVALUE[V].begin(), AVALUE[V].end(), R) == AVALUE[V].end())
				//V的记录中没有R则添加R
				AVALUE[V].push_back(R);
		}
	}
}
void ObjectCodeGen::endBlock()
{
	//基本块处理结束
	for (map<string, vector<string>>::iterator iter = AVALUE.begin(); iter != AVALUE.end(); iter++)
	{
		string V = iter->first;
		//尚未存入内存的变量
		if (find(AVALUE[V].begin(), AVALUE[V].end(), "M") == AVALUE[V].end())
		{
			string R;
			for (auto i = 0; i < AVALUE[V].size(); i++)
			{
				if (AVALUE[V][i] != "M")

				{
					R = AVALUE[V][i];
					break;
				}
			}
			if (V[0] == 'G')
				EMIT("sw " + R + "," + DATA + "+" + to_string(stoi(V.substr(1))));
			else if (V[0] == 'V')
				EMIT("sw " + R + "," + STACK + "+" + to_string(4 + stoi(V.substr(1))) + "($fp)");
			else if (V[0] == 'T')
				EMIT("sw " + R + "," + TEMP + "+" + to_string(4 * stoi(V.substr(1))));
			else
			{
				//cerr << "ERROR: AVALUE中出现意外的变量名:" << V << endl;
				//exit(-1);
				throw string("ERROR: 目标代码生成器错误:AVALUE中出现意外的变量名:") + V + string("\n");
			}
			//AVALUE[V].push_back("M");
		}
	}
	for (map<string, vector<pair<string, int>>>::iterator iter = RVALUE.begin(); iter != RVALUE.end(); iter++)
	{
		iter->second.clear();
	}
	AVALUE.clear();
}
void ObjectCodeGen::geneObjectCode()
{
	EMIT(".data");
	EMIT(DATA + ":.space " + to_string(data_buf_size));
	EMIT(STACK + ":.space " + to_string(stack_buf_size));
	EMIT(TEMP + ":.space " + to_string(temp_buf_size));
	EMIT(".text");
	EMIT("j B0");
	EMIT("B1:");
	EMIT("nop");
	EMIT("j B1");
	EMIT("nop");
	EMIT("B2:");
	EMIT("jal Fmain");
	EMIT("nop");
	EMIT("break");
	EMIT("B0:");
	EMIT("addi $gp,$zero,0");
	EMIT("addi $fp,$zero,0");
	EMIT("addi $sp,$zero,4");
	EMIT("j B2");
	EMIT("nop");

	for (auto block_no = 0; block_no < block_group.size(); block_no++)
	{
		vector<messageTableItem> MessageTable = geneMessageTable(block_no);
		bool j_end = false;
		for (auto i = 0; i < MessageTable.size(); i++)
		{
			new_code.clear();
			Quadruple TAS = MessageTable[i].TAS;
			string Reg_arg1, Reg_arg2;
			if (TAS.arg1 == "" || TAS.op == "=[]")
				Reg_arg1 = "";
			else if (TAS.arg1[0] == '$')
				Reg_arg1 = TAS.arg1;
			else if (TAS.arg1 == "[$sp]")
				Reg_arg1 = STACK + "($sp)";
			else if (is_num(TAS.arg1))
			{
				if (TAS.op == "+")
					Reg_arg1 = TAS.arg1;
				else
				{
					EMIT("addi $t8,$zero," + TAS.arg1);
					Reg_arg1 = "$t8";
				}
			}
			else if (TAS.arg1[0] == 'G')
			{
				if (AVALUE.find(TAS.arg1) == AVALUE.end())
				{
					AVALUE[TAS.arg1] = vector<string>{ "M" };
				}
				else if (AVALUE[TAS.arg1].size() == 0)
				{
					//cerr << "ERROR: 找不到" << TAS.arg1 << "的地址\n";
					//exit(-1);
					throw string("ERROR: 目标代码生成器错误:找不到") + TAS.arg1 + string("的地址\n");
				}

				if (AVALUE[TAS.arg1].size() == 1 && AVALUE[TAS.arg1][0] == "M")
				{
					EMIT("lw $t8," + DATA + "+" + to_string(stoi(TAS.arg1.substr(1))));
					Reg_arg1 = "$t8";
				}
				else
				{
					for (auto i = 0; i < AVALUE[TAS.arg1].size(); i++)
					{
						if (AVALUE[TAS.arg1][i] != "M")
							Reg_arg1 = AVALUE[TAS.arg1][i];
					}
				}
			}
			else if (TAS.arg1[0] == 'V')
			{
				if (AVALUE.find(TAS.arg1) == AVALUE.end())
				{
					AVALUE[TAS.arg1] = vector<string>{ "M" };
				}
				else if (AVALUE[TAS.arg1].size() == 0)
				{
					//cerr << "ERROR: 找不到" << TAS.arg1 << "的地址\n";
					//exit(-1);
					throw string("ERROR: 目标代码生成器错误:找不到") + TAS.arg1 + string("的地址\n");
				}

				if (AVALUE[TAS.arg1].size() == 1 && AVALUE[TAS.arg1][0] == "M")
				{
					EMIT("lw $t8," + STACK + "+" + to_string(4 + stoi(TAS.arg1.substr(1))) + "($fp)");
					Reg_arg1 = "$t8";
				}
				else
				{
					for (auto i = 0; i < AVALUE[TAS.arg1].size(); i++)
					{
						if (AVALUE[TAS.arg1][i] != "M")
							Reg_arg1 = AVALUE[TAS.arg1][i];
					}
				}
			}
			else if (TAS.arg1[0] == 'T')
			{
				if (AVALUE.find(TAS.arg1) == AVALUE.end())
				{
					AVALUE[TAS.arg1] = vector<string>{ "M" };
				}
				else if (AVALUE[TAS.arg1].size() == 0)
				{
					//cerr << "ERROR: 找不到" << TAS.arg1 << "的地址\n";
					//exit(-1);
					throw string("ERROR: 目标代码生成器错误:找不到") + TAS.arg1 + string("的地址\n");
				}

				if (AVALUE[TAS.arg1].size() == 1 && AVALUE[TAS.arg1][0] == "M")
				{
					EMIT("lw $t8," + TEMP + "+" + to_string(4 * stoi(TAS.arg1.substr(1))));
					Reg_arg1 = "$t8";
				}
				else
				{
					for (auto i = 0; i < AVALUE[TAS.arg1].size(); i++)
					{
						if (AVALUE[TAS.arg1][i] != "M")
							Reg_arg1 = AVALUE[TAS.arg1][i];
					}
				}
			}

			if (TAS.arg2 == "")
				Reg_arg2 = "";
			else if (TAS.arg2[0] == '$')
				Reg_arg2 = TAS.arg2;
			else if (TAS.arg2 == "[$sp]")
				Reg_arg2 = STACK + "($sp)";
			else if (is_num(TAS.arg2))
			{
				if (TAS.op == "+" && !is_num(Reg_arg1))//不能有两个立即数
					Reg_arg2 = TAS.arg2;
				else
				{
					EMIT("addi $t9,$zero," + TAS.arg2);
					Reg_arg2 = "$t9";
				}
			}
			else if (TAS.arg2[0] == 'G')
			{
				if (AVALUE.find(TAS.arg2) == AVALUE.end())
				{
					AVALUE[TAS.arg2] = vector<string>{ "M" };
				}
				else if (AVALUE[TAS.arg2].size() == 0)
				{
					//cerr << "ERROR: 找不到" << TAS.arg2 << "的地址\n";
					//exit(-1);
					throw string("ERROR: 目标代码生成器错误:找不到") + TAS.arg2 + string("的地址\n");
				}

				if (AVALUE[TAS.arg2].size() == 1 && AVALUE[TAS.arg2][0] == "M")
				{
					EMIT("lw $t9," + DATA + "+" + to_string(stoi(TAS.arg2.substr(1))));
					Reg_arg2 = "$t9";
				}
				else
				{
					for (auto i = 0; i < AVALUE[TAS.arg2].size(); i++)
					{
						if (AVALUE[TAS.arg2][i] != "M")
							Reg_arg2 = AVALUE[TAS.arg2][i];
					}
				}
			}
			else if (TAS.arg2[0] == 'V')
			{
				if (AVALUE.find(TAS.arg2) == AVALUE.end())
				{
					AVALUE[TAS.arg2] = vector<string>{ "M" };
				}
				else if (AVALUE[TAS.arg2].size() == 0)
				{
					//cerr << "ERROR: 找不到" << TAS.arg2 << "的地址\n";
					//exit(-1);
					throw string("ERROR: 目标代码生成器错误:找不到") + TAS.arg2 + string("的地址\n");
				}

				if (AVALUE[TAS.arg2].size() == 1 && AVALUE[TAS.arg2][0] == "M")
				{
					EMIT("lw $t9," + STACK + "+" + to_string(4 + stoi(TAS.arg2.substr(1))) + "($fp)");
					Reg_arg2 = "$t9";
				}
				else
				{
					for (auto i = 0; i < AVALUE[TAS.arg2].size(); i++)
					{
						if (AVALUE[TAS.arg2][i] != "M")
							Reg_arg2 = AVALUE[TAS.arg2][i];
					}
				}
			}
			else if (TAS.arg2[0] == 'T')
			{
				if (AVALUE.find(TAS.arg2) == AVALUE.end())
				{
					AVALUE[TAS.arg2] = vector<string>{ "M" };
				}
				else if (AVALUE[TAS.arg2].size() == 0)
				{
					//cerr << "ERROR: 找不到" << TAS.arg2 << "的地址\n";
					//exit(-1);
					throw string("ERROR: 目标代码生成器错误:找不到") + TAS.arg2 + string("的地址\n");
				}

				if (AVALUE[TAS.arg2].size() == 1 && AVALUE[TAS.arg2][0] == "M")
				{
					EMIT("lw $t9," + TEMP + "+" + to_string(4 * stoi(TAS.arg2.substr(1))));
					Reg_arg2 = "$t9";
				}
				else
				{
					for (auto i = 0; i < AVALUE[TAS.arg2].size(); i++)
					{
						if (AVALUE[TAS.arg2][i] != "M")
							Reg_arg2 = AVALUE[TAS.arg2][i];
					}
				}
			}

			if (TAS.op[0] == 'F' || TAS.op[0] == 'L')
			{
				EMIT(TAS.op + ":");
			}
			else if (TAS.op == "nop")
			{
				EMIT("nop");
			}
			else if (TAS.op == "j")
			{
				j_end = true;
				endBlock();
				EMIT("j " + TAS.res);
			}
			else if (TAS.op == "jal")
			{
				j_end = true;
				//endBlock();
				EMIT("jal " + TAS.res);
			}
			else if (TAS.op == "break")
			{
				j_end = true;
				endBlock();
				EMIT("break");
			}
			else if (TAS.op == "ret")
			{
				j_end = true;
				endBlock();
				EMIT("jr $ra");
			}
			else if (TAS.op == "jnz")
			{
				j_end = true;
				if (RVALUE.find(Reg_arg1) != RVALUE.end())
					freshRA(MessageTable[i].arg1_tag, Reg_arg1, TAS.arg1, false);
				endBlock();
				EMIT("bne " + Reg_arg1 + ",$zero," + TAS.res);
			}
			else if (TAS.op == "j<")
			{
				j_end = true;
				EMIT("addi $t8," + Reg_arg1 + ",1");
				EMIT("sub $t9," + Reg_arg2 + ",$t8");
				if (RVALUE.find(Reg_arg1) != RVALUE.end())
					freshRA(MessageTable[i].arg1_tag, Reg_arg1, TAS.arg1, false);
				if (RVALUE.find(Reg_arg2) != RVALUE.end())
					freshRA(MessageTable[i].arg2_tag, Reg_arg2, TAS.arg2, false);
				endBlock();
				EMIT("bgez $t9," + TAS.res);
			}
			else if (TAS.op == "j<=")
			{
				j_end = true;
				EMIT("sub $t9," + Reg_arg2 + "," + Reg_arg1);
				if (RVALUE.find(Reg_arg1) != RVALUE.end())
					freshRA(MessageTable[i].arg1_tag, Reg_arg1, TAS.arg1, false);
				if (RVALUE.find(Reg_arg2) != RVALUE.end())
					freshRA(MessageTable[i].arg2_tag, Reg_arg2, TAS.arg2, false);
				endBlock();
				EMIT("bgez $t9," + TAS.res);
			}
			else if (TAS.op == "j>")
			{
				j_end = true;
				EMIT("addi $t9," + Reg_arg2 + ",1");
				EMIT("sub  $t8," + Reg_arg1 + ",$t9");
				if (RVALUE.find(Reg_arg1) != RVALUE.end())
					freshRA(MessageTable[i].arg1_tag, Reg_arg1, TAS.arg1, false);
				if (RVALUE.find(Reg_arg2) != RVALUE.end())
					freshRA(MessageTable[i].arg2_tag, Reg_arg2, TAS.arg2, false);
				endBlock();
				EMIT("bgez $t8," + TAS.res);
			}
			else if (TAS.op == "j>=")
			{
				j_end = true;
				EMIT("sub $t8," + Reg_arg1 + "," + Reg_arg2);
				if (RVALUE.find(Reg_arg1) != RVALUE.end())
					freshRA(MessageTable[i].arg1_tag, Reg_arg1, TAS.arg1, false);
				if (RVALUE.find(Reg_arg2) != RVALUE.end())
					freshRA(MessageTable[i].arg2_tag, Reg_arg2, TAS.arg2, false);
				endBlock();
				EMIT("bgez $t8," + TAS.res);
			}
			else if (TAS.op == "j==")
			{
				j_end = true;
				if (RVALUE.find(Reg_arg1) != RVALUE.end())
					freshRA(MessageTable[i].arg1_tag, Reg_arg1, TAS.arg1, false);
				if (RVALUE.find(Reg_arg2) != RVALUE.end())
					freshRA(MessageTable[i].arg2_tag, Reg_arg2, TAS.arg2, false);
				endBlock();
				EMIT("beq " + Reg_arg1 + "," + Reg_arg2 + "," + TAS.res);
			}
			else if (TAS.op == "j!=")
			{
				j_end = true;
				if (RVALUE.find(Reg_arg1) != RVALUE.end())
					freshRA(MessageTable[i].arg1_tag, Reg_arg1, TAS.arg1, false);
				if (RVALUE.find(Reg_arg2) != RVALUE.end())
					freshRA(MessageTable[i].arg2_tag, Reg_arg2, TAS.arg2, false);
				endBlock();
				EMIT("bne " + Reg_arg1 + "," + Reg_arg2 + "," + TAS.res);
			}
			else if (TAS.op == ":=")
			{
				string R;
				if (TAS.res[0] == '$')
				{
					R = TAS.res;
					if (TAS.arg1 == "[$sp]")
					{
						EMIT("lw " + R + "," + Reg_arg1);
					}
					else
					{
						EMIT("add " + R + ",$zero," + Reg_arg1);
					}
				}
				else if (TAS.res == "[$sp]")
				{
					R = STACK + "($sp)";
					if (TAS.arg1 == "[$sp]")
					{
						//cerr << "ERROR: 从[$sp]到[$sp]的赋值\n";
						//exit(-1);
						throw string("ERROR: 目标代码生成器错误:发生从[$sp]到[$sp]的赋值\n");
					}
					else
					{
						EMIT("sw " + Reg_arg1 + "," + R);
					}
				}
				else
				{
					R = getREG(TAS.res);
					if (TAS.arg1 == "[$sp]")
					{
						EMIT("lw " + R + "," + Reg_arg1);
					}
					else
					{
						EMIT("add " + R + ",$zero," + Reg_arg1);
					}
				}
				if (RVALUE.find(R) != RVALUE.end())
					freshRA(MessageTable[i].result_tag, R, TAS.res, true);
				if (RVALUE.find(Reg_arg1) != RVALUE.end())
					freshRA(MessageTable[i].arg1_tag, Reg_arg1, TAS.arg1, false);
			}
			else if (TAS.op == "[]=")
			{
				string base = TAS.res;
				if (TAS.res[0] == 'G')
				{
					EMIT("sll $t9," + Reg_arg2 + ",2");
					EMIT("addi $t9,$t9," + base.substr(1));
					EMIT("sw " + Reg_arg1 + "," + DATA + "($t9)");
				}
				else if (TAS.res[0] == 'V')
				{
					EMIT("sll $t9," + Reg_arg2 + ",2");
					EMIT("addi $t9,$t9," + base.substr(1));
					EMIT("addi $t9,$t9,4");
					EMIT("add $t9,$t9,$fp");
					EMIT("sw " + Reg_arg1 + "," + STACK + "($t9)");
				}
				else if (TAS.res[0] == 'T')
				{
					EMIT("addi $t9," + Reg_arg2 + "," + base.substr(1));
					EMIT("sll $t9,$t9,2");
					EMIT("sw " + Reg_arg1 + "," + TEMP + "($t9)");
				}
				else
				{
					//cerr << "ERROR: []=的左部result标识符不合法\n";
					//exit(-1);
					throw string("ERROR: 目标代码生成器错误:[]=的左部result标识符不合法\n");
				}
				if (RVALUE.find(Reg_arg1) != RVALUE.end())
					freshRA(MessageTable[i].arg1_tag, Reg_arg1, TAS.arg1, false);
				if (RVALUE.find(Reg_arg2) != RVALUE.end())
					freshRA(MessageTable[i].arg2_tag, Reg_arg2, TAS.arg2, false);
			}
			else if (TAS.op == "=[]")
			{
				string R;
				if (TAS.res[0] == '$')
					R = TAS.res;
				else if (TAS.res == "[$sp]")
					R = STACK + "($sp)";
				else
					R = getREG(TAS.res);
				if (TAS.arg1[0] == 'G')
				{
					EMIT("sll $t9," + Reg_arg2 + ",2");
					EMIT("addi $t9,$t9," + TAS.arg1.substr(1));
					EMIT("lw " + R + "," + DATA + "($t9)");
				}
				else if (TAS.arg1[0] == 'V')
				{
					EMIT("sll $t9," + Reg_arg2 + ",2");
					EMIT("addi $t9,$t9," + TAS.arg1.substr(1));
					EMIT("addi $t9,$t9,4");
					EMIT("add $t9,$t9,$fp");
					EMIT("lw " + R + "," + STACK + "($t9)");
				}
				else if (TAS.arg1[0] == 'T')
				{
					EMIT("addi $t9," + Reg_arg2 + "," + TAS.arg1.substr(1));
					EMIT("sll $t9,$t9,2");
					EMIT("lw " + R + "," + TEMP + "($t9)");
				}
				else
				{
					//cerr << "ERROR: =[]的右部arg1标识符不合法\n";
					//exit(-1);
					throw string("ERROR: 目标代码生成器错误:=[]的右部arg1标识符不合法\n");
				}
				if (RVALUE.find(R) != RVALUE.end())
					freshRA(MessageTable[i].result_tag, R, TAS.res, true);
				if (RVALUE.find(Reg_arg2) != RVALUE.end())
					freshRA(MessageTable[i].arg2_tag, Reg_arg2, TAS.arg2, false);
			}
			else if (TAS.op == "+")
			{
				string R;
				if (TAS.res[0] == '$')
					R = TAS.res;
				else if (TAS.res == "[$sp]")
					R = STACK + "($sp)";
				else
					R = getREG(TAS.res);

				if (is_num(Reg_arg1))
					EMIT("addi " + R + "," + Reg_arg2 + "," + Reg_arg1);
				else if (is_num(Reg_arg2))
					EMIT("addi " + R + "," + Reg_arg1 + "," + Reg_arg2);
				else
					EMIT("add " + R + "," + Reg_arg1 + "," + Reg_arg2);
				if (RVALUE.find(R) != RVALUE.end())
					freshRA(MessageTable[i].result_tag, R, TAS.res, true);
				if (RVALUE.find(Reg_arg1) != RVALUE.end())
					freshRA(MessageTable[i].arg1_tag, Reg_arg1, TAS.arg1, false);
				if (RVALUE.find(Reg_arg2) != RVALUE.end())
					freshRA(MessageTable[i].arg2_tag, Reg_arg2, TAS.arg2, false);
			}
			else if (TAS.op == "-")
			{
				string R;
				if (TAS.res[0] == '$')
					R = TAS.res;
				else if (TAS.res == "[$sp]")
					R = STACK + "($sp)";
				else
					R = getREG(TAS.res);
				EMIT("sub " + R + "," + Reg_arg1 + "," + Reg_arg2);
				if (RVALUE.find(R) != RVALUE.end())
					freshRA(MessageTable[i].result_tag, R, TAS.res, true);
				if (RVALUE.find(Reg_arg1) != RVALUE.end())
					freshRA(MessageTable[i].arg1_tag, Reg_arg1, TAS.arg1, false);
				if (RVALUE.find(Reg_arg2) != RVALUE.end())
					freshRA(MessageTable[i].arg2_tag, Reg_arg2, TAS.arg2, false);
			}
			else if (TAS.op == "&")
			{
				string R;
				if (TAS.res[0] == '$')
					R = TAS.res;
				else if (TAS.res == "[$sp]")
					R = STACK + "($sp)";
				else
					R = getREG(TAS.res);
				EMIT("and " + R + "," + Reg_arg1 + "," + Reg_arg2);
				if (RVALUE.find(R) != RVALUE.end())
					freshRA(MessageTable[i].result_tag, R, TAS.res, true);
				if (RVALUE.find(Reg_arg1) != RVALUE.end())
					freshRA(MessageTable[i].arg1_tag, Reg_arg1, TAS.arg1, false);
				if (RVALUE.find(Reg_arg2) != RVALUE.end())
					freshRA(MessageTable[i].arg2_tag, Reg_arg2, TAS.arg2, false);
			}
			else if (TAS.op == "|")
			{
				string R;
				if (TAS.res[0] == '$')
					R = TAS.res;
				else if (TAS.res == "[$sp]")
					R = STACK + "($sp)";
				else
					R = getREG(TAS.res);
				EMIT("or " + R + "," + Reg_arg1 + "," + Reg_arg2);
				if (RVALUE.find(R) != RVALUE.end())
					freshRA(MessageTable[i].result_tag, R, TAS.res, true);
				if (RVALUE.find(Reg_arg1) != RVALUE.end())
					freshRA(MessageTable[i].arg1_tag, Reg_arg1, TAS.arg1, false);
				if (RVALUE.find(Reg_arg2) != RVALUE.end())
					freshRA(MessageTable[i].arg2_tag, Reg_arg2, TAS.arg2, false);
			}
			else if (TAS.op == "^")
			{
				string R;
				if (TAS.res[0] == '$')
					R = TAS.res;
				else if (TAS.res == "[$sp]")
					R = STACK + "($sp)";
				else
					R = getREG(TAS.res);
				EMIT("xor " + R + "," + Reg_arg1 + "," + Reg_arg2);
				if (RVALUE.find(R) != RVALUE.end())
					freshRA(MessageTable[i].result_tag, R, TAS.res, true);
				if (RVALUE.find(Reg_arg1) != RVALUE.end())
					freshRA(MessageTable[i].arg1_tag, Reg_arg1, TAS.arg1, false);
				if (RVALUE.find(Reg_arg2) != RVALUE.end())
					freshRA(MessageTable[i].arg2_tag, Reg_arg2, TAS.arg2, false);
			}
			else if (TAS.op == "*")
			{
				string R;
				if (TAS.res[0] == '$')
					R = TAS.res;
				else if (TAS.res == "[$sp]")
					R = STACK + "($sp)";
				else
					R = getREG(TAS.res);
				EMIT("mul " + R + "," + Reg_arg1 + "," + Reg_arg2);
				if (RVALUE.find(R) != RVALUE.end())
					freshRA(MessageTable[i].result_tag, R, TAS.res, true);
				if (RVALUE.find(Reg_arg1) != RVALUE.end())
					freshRA(MessageTable[i].arg1_tag, Reg_arg1, TAS.arg1, false);
				if (RVALUE.find(Reg_arg2) != RVALUE.end())
					freshRA(MessageTable[i].arg2_tag, Reg_arg2, TAS.arg2, false);
			}
			else if (TAS.op == "/")
			{
				string R;
				if (TAS.res[0] == '$')
					R = TAS.res;
				else if (TAS.res == "[$sp]")
					R = STACK + "($sp)";
				else
					R = getREG(TAS.res);
				EMIT("div " + Reg_arg1 + "," + Reg_arg2);
				EMIT("amflo " + R);//Quotient in $lo
				if (RVALUE.find(R) != RVALUE.end())
					freshRA(MessageTable[i].result_tag, R, TAS.res, true);
				if (RVALUE.find(Reg_arg1) != RVALUE.end())
					freshRA(MessageTable[i].arg1_tag, Reg_arg1, TAS.arg1, false);
				if (RVALUE.find(Reg_arg2) != RVALUE.end())
					freshRA(MessageTable[i].arg2_tag, Reg_arg2, TAS.arg2, false);
			}
			else
			{
				throw string("ERROR: 目标代码生成器错误:非法的中间代码") + TAS.op + string(" ") + TAS.arg1 + string(" ") + TAS.arg2 + string(" ") + TAS.res + string("\n");
			}
			analysisHistory.push_back({ TAS,new_code,RVALUE,AVALUE });
		}
		if (!j_end)
			endBlock();
	}
}
void ObjectCodeGen::showMessageTableHistory()
{
	ofstream fout("./res/MessggeTable.log", ios::out);
	for (auto it : messageTableHistory) {
		fout << "(" << it.no << ")\t" << it.TAS.op << ' ' << it.TAS.arg1 << ' ' << it.TAS.arg2 << ' ' << it.TAS.res;
		fout << '\n';

		string l_value, l_arg, r_arg;

		if (it.result_tag.second) {
			if (it.result_tag.first == INT_MAX)
				l_value = string("(^,y)");
			else
				l_value = string("(" + to_string(it.result_tag.first) + ",y)");

		}
		else
			l_value = string("(^,^)");

		if (it.arg1_tag.second) {
			if (it.arg1_tag.first == INT_MAX)
				l_arg = string("(^,y)");
			else
				l_arg = string("(" + to_string(it.arg1_tag.first) + ",y)");
		}
		else
			l_arg = string("(^,^)");

		if (it.arg2_tag.second) {

			if (it.arg2_tag.first == INT_MAX)
				r_arg = string("(^,y)");
			else
				r_arg = string("(" + to_string(it.arg2_tag.first) + ",y)");
		}
		else
			r_arg = string("(^,^)");

		fout << '\t' << l_value << ' ' << l_arg << ' ' << r_arg << endl;

	}
	fout.close();
	cout << "待用活跃信息已保存至 ./res/MessageTable.log" << endl;
}

void ObjectCodeGen::showAnalysisHistory()
{
	ofstream fout("./res/RegAllocTable.log", ios::out);
	for (auto ino = 0; ino < analysisHistory.size(); ino++)
	{
		analysisHistoryItem e = analysisHistory[ino];
		fout << "\n[四元式]\t(" << e.TAS.op << "," << e.TAS.arg1 << "," << e.TAS.arg2 << "," << e.TAS.res << ")\n";
		for (auto i = 0; i < e.object_codes.size(); i++)
		{
			if (i == 0)
				fout << "[汇编码]\t";
			else
				fout << "        \t";

			fout << e.object_codes[i] << '\n';
		}
		fout << "---------------------" << endl;

		string RVALUE = "", AVALUE = "";
		for (map<string, vector<pair<string, int>>>::iterator iter = analysisHistory[ino].RVALUE.begin(); iter != analysisHistory[ino].RVALUE.end(); iter++)
		{
			RVALUE += iter->first + ": ";
			for (auto k = 0; k < iter->second.size(); k++)
			{
				RVALUE += iter->second[k].first + " ";
			}
			map<string, vector<pair<string, int>>>::iterator olditer = iter;
			iter++;
			RVALUE += (iter == analysisHistory[ino].RVALUE.end() ? "" : "\n");
			iter = olditer;
		}
		for (map<string, vector<string>>::iterator iter = analysisHistory[ino].AVALUE.begin(); iter != analysisHistory[ino].AVALUE.end(); iter++)
		{
			AVALUE += iter->first + ": ";
			for (auto k = 0; k < iter->second.size(); k++)
			{
				AVALUE += iter->second[k] + " ";
			}
			map<string, vector<string>>::iterator olditer = iter;
			iter++;
			AVALUE += (iter == analysisHistory[ino].AVALUE.end() ? "" : "\n");
			iter = olditer;
		}



		fout << "\n[RVALUE]\n";
		fout << RVALUE << endl;
		fout << "---------------------" << endl;

		fout << "\n[AVALUE]\n";
		fout << AVALUE << endl;
		fout << "=========================================" << endl;
	}
	cout << "寄存器分配信息已保存至 ./res/RegAllocTable.log" << endl;
}
void ObjectCodeGen::showObjectCode()
{
	for (auto i = 0; i < object_code.size(); i++)
		cout << "(" << i << ")\t" << object_code[i] << endl;
}



