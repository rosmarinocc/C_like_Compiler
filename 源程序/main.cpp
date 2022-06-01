#include "LR1.h"

static void global_init(LR1& lr1) {

	char temp;

	cout << "=========================================" << endl;
	cout << endl;
	cout << "类C编译器	by zhk" << endl;
	cout << endl;
	cout << "=========================================" << endl;
	cout << "初始化ing..." << endl;

	/*读入语法规则, 构建子集族*/
	cout << "正在读入语法规则,构建规范子集族..." << endl;
	//lr1.read_generators("./input/grammar.txt");
	lr1.read_generators("./input/grammarArray.txt");
	lr1.get_firstset_of_vn();
	lr1.init_items();
	lr1.getTable();

	cout << "【 LR1分析表已生成,是否输出文件?	Y:确认	其余:取消 】" << endl;
	cin >> temp;
	if (temp == 'Y' || temp == 'y') {
		lr1.printTable("./res/LR1Table.csv");
		cout << "LR1分析表已保存至 ./res/LR1Table.csv" << endl;
	}
	cout << endl;

	cout << "【 请输入待编译程序所在目录及文件名...(示例: ./input/array.cpp) 】" << endl;
	cin >> lr1.targetCode;
	cout << endl;
}


static void OptimizerGo(LR1& lr1) {

	char temp;

	cout << endl;
	cout << "=========================================" << endl;
	cout << "【PART 3】 DAG中间代码优化" << endl;
	cout << "=========================================" << endl;


	Optimizer opt(lr1.lex.NameTable, lr1.SemanticAnalysis.global_table, lr1.SemanticAnalysis.Quadruple_Code);

	cout << "优化中ing..." << endl;

	double opt_rate = opt.analysis();
	lr1.OptIntermediateCode = opt.intermediate_code;
	lr1.BlockGroup = opt.block_group;

	cout << "优化完成！优化率：" << opt_rate << '%' << endl;
	cout << "【 是否输出DAG优化后的中间代码到文件？		Y:确认	其余 : 取消 】" << endl;

	cin >> temp;
	if (temp == 'Y' || temp == 'y') {

		ofstream fout("./res/OptQuadrupleRes.txt", ios::out);
		int i = 0;
		for (auto it : opt.intermediate_code) {

			fout << "(" << i++ << ")\t" << it.op << '\t' << it.arg1 << '\t' << it.arg2 << '\t' << it.res << endl;
		}
		fout.close();
	}
	cout << "DAG优化后的四元式中间代码结果已保存至 ./res/OptQuadrupleRes.txt" << endl;


}

static string getMipsName(string path) {

	int ps = path.find_last_of("/");
	int pe = path.find_last_of(".");
	string pic_name = path.substr(ps + 1, pe - ps - 1);
	return "./res/" + pic_name + ".s";

}

static void MipsGenGo(LR1& lr1) {
	char temp;
	cout << endl;
	cout << "=========================================" << endl;
	cout << "【PART 4】 目标代码生成(MIPS)" << endl;
	cout << "=========================================" << endl;

	ObjectCodeGen mipsGen(lr1.OptIntermediateCode, lr1.BlockGroup, DEFAULT_STACK_SIZE);
	cout << "目标代码生成ing..." << endl;
	mipsGen.geneObjectCode();

	string mipsPath = getMipsName(lr1.targetCode);
	ofstream fout(mipsPath, ios::out);
	for (auto it : mipsGen.object_code)
		fout << it << endl;
	fout.close();
	cout << "目标代码已保存至 " << mipsPath << endl;
	cout << endl;
	cout << "【 是否需要生成待用活跃信息？	Y:确认	其余 : 取消 】" << endl;
	cin >> temp;
	if (temp == 'Y' || temp == 'y')
		mipsGen.showMessageTableHistory();

	cout << endl;

	cout << "【 是否需要生成寄存器分配信息？	Y:确认	其余 : 取消 】" << endl;

	cin >> temp;
	if (temp == 'Y' || temp == 'y')
		mipsGen.showAnalysisHistory();

	cout << endl;
	cout << "=========================================" << endl;
	cout << endl;
	cout << "编译完成！" << endl;
	cout << endl;
	cout << "=========================================" << endl;
	cout << endl;

}

int main() {

	/*全局初始化*/
	LR1 lr1;
	global_init(lr1);

	/*词法分析*/
	lr1.LexGo();

	/*语法&语义分析*/
	lr1.SemanticGo();

	/*中间代码优化*/
	OptimizerGo(lr1);

	/*目标代码生成*/
	MipsGenGo(lr1);

	return 0;
}