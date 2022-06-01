#include "LR1.h"

static void global_init(LR1& lr1) {

	char temp;

	cout << "=========================================" << endl;
	cout << endl;
	cout << "��C������	by zhk" << endl;
	cout << endl;
	cout << "=========================================" << endl;
	cout << "��ʼ��ing..." << endl;

	/*�����﷨����, �����Ӽ���*/
	cout << "���ڶ����﷨����,�����淶�Ӽ���..." << endl;
	//lr1.read_generators("./input/grammar.txt");
	lr1.read_generators("./input/grammarArray.txt");
	lr1.get_firstset_of_vn();
	lr1.init_items();
	lr1.getTable();

	cout << "�� LR1������������,�Ƿ�����ļ�?	Y:ȷ��	����:ȡ�� ��" << endl;
	cin >> temp;
	if (temp == 'Y' || temp == 'y') {
		lr1.printTable("./res/LR1Table.csv");
		cout << "LR1�������ѱ����� ./res/LR1Table.csv" << endl;
	}
	cout << endl;

	cout << "�� ������������������Ŀ¼���ļ���...(ʾ��: ./input/array.cpp) ��" << endl;
	cin >> lr1.targetCode;
	cout << endl;
}


static void OptimizerGo(LR1& lr1) {

	char temp;

	cout << endl;
	cout << "=========================================" << endl;
	cout << "��PART 3�� DAG�м�����Ż�" << endl;
	cout << "=========================================" << endl;


	Optimizer opt(lr1.lex.NameTable, lr1.SemanticAnalysis.global_table, lr1.SemanticAnalysis.Quadruple_Code);

	cout << "�Ż���ing..." << endl;

	double opt_rate = opt.analysis();
	lr1.OptIntermediateCode = opt.intermediate_code;
	lr1.BlockGroup = opt.block_group;

	cout << "�Ż���ɣ��Ż��ʣ�" << opt_rate << '%' << endl;
	cout << "�� �Ƿ����DAG�Ż�����м���뵽�ļ���		Y:ȷ��	���� : ȡ�� ��" << endl;

	cin >> temp;
	if (temp == 'Y' || temp == 'y') {

		ofstream fout("./res/OptQuadrupleRes.txt", ios::out);
		int i = 0;
		for (auto it : opt.intermediate_code) {

			fout << "(" << i++ << ")\t" << it.op << '\t' << it.arg1 << '\t' << it.arg2 << '\t' << it.res << endl;
		}
		fout.close();
	}
	cout << "DAG�Ż������Ԫʽ�м�������ѱ����� ./res/OptQuadrupleRes.txt" << endl;


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
	cout << "��PART 4�� Ŀ���������(MIPS)" << endl;
	cout << "=========================================" << endl;

	ObjectCodeGen mipsGen(lr1.OptIntermediateCode, lr1.BlockGroup, DEFAULT_STACK_SIZE);
	cout << "Ŀ���������ing..." << endl;
	mipsGen.geneObjectCode();

	string mipsPath = getMipsName(lr1.targetCode);
	ofstream fout(mipsPath, ios::out);
	for (auto it : mipsGen.object_code)
		fout << it << endl;
	fout.close();
	cout << "Ŀ������ѱ����� " << mipsPath << endl;
	cout << endl;
	cout << "�� �Ƿ���Ҫ���ɴ��û�Ծ��Ϣ��	Y:ȷ��	���� : ȡ�� ��" << endl;
	cin >> temp;
	if (temp == 'Y' || temp == 'y')
		mipsGen.showMessageTableHistory();

	cout << endl;

	cout << "�� �Ƿ���Ҫ���ɼĴ���������Ϣ��	Y:ȷ��	���� : ȡ�� ��" << endl;

	cin >> temp;
	if (temp == 'Y' || temp == 'y')
		mipsGen.showAnalysisHistory();

	cout << endl;
	cout << "=========================================" << endl;
	cout << endl;
	cout << "������ɣ�" << endl;
	cout << endl;
	cout << "=========================================" << endl;
	cout << endl;

}

int main() {

	/*ȫ�ֳ�ʼ��*/
	LR1 lr1;
	global_init(lr1);

	/*�ʷ�����*/
	lr1.LexGo();

	/*�﷨&�������*/
	lr1.SemanticGo();

	/*�м�����Ż�*/
	OptimizerGo(lr1);

	/*Ŀ���������*/
	MipsGenGo(lr1);

	return 0;
}