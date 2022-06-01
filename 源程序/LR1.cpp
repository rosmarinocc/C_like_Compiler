#include "LR1.h"
#include <iomanip>
/******************LR1��******************/
int LR1::read_generators(string dat_path)
{

	stringstream sstr;
	ifstream fin(dat_path, ios::in | ios::binary);
	if (!fin.is_open())
		return -1;
	char ch;
	while (1) {
		fin.get(ch);
		if (fin.eof())
			break;
		sstr << ch;
	}
	fin.close();
	int prod_no = 0;
	while (1) {
		char buf[1000] = { 0 };
		sstr.getline(buf, 1000, '\n');
		if (buf[0] == '\0')
			break;
		/*������,ÿ�ִ���1��,����1-n��Production*/

		string s0 = buf;
		int pos_t = s0.find('\r');
		if (pos_t != -1)
			s0.erase(pos_t, 1);
		while ((pos_t = s0.find(' ')) != -1)
			s0.erase(pos_t, 1);
		int pos_arrow = s0.find("->");
		//����ȡ
		string left0 = s0.substr(0, pos_arrow);
		string left = left0.substr(left0.find('<'), (left0.find('>') - left0.find('<')+1));
		Symbol left_tag = Symbol(VN, left);
		vSet.insert(left_tag);
		vnSet.insert(left_tag);

		//�Ҳ���ȡ
		string right0 = s0.substr(pos_arrow + 2);
		string str_t;
		while (!right0.empty()) {
			string right_t;//ÿ�ִ����Ҳ�'|'�ָ���һ��
			Production p1;
			int pos_t = right0.find('|');
			if (pos_t == -1)
				right_t = right0;
			else
				right_t = right0.substr(0, pos_t);
			for (int i = 0; i < right_t.length(); i++) {//ÿ�ִ���һ������ʽ�ĸ�������
				if (right_t[i] == '<') {//���ս��
					int pos_rbracket = right_t.find_first_of('>', i);
					string vn_s = right_t.substr(i, pos_rbracket - i + 1);
					Symbol vn(VN, vn_s);  //VN��ʾ���ս����
					vSet.insert(vn);
					vnSet.insert(vn);
					p1.Right.push_back(vn);
					i = pos_rbracket;
				}
				else if (right_t[i] == '[') {//�ս��
					int pos_rbracket = right_t.find_first_of(']', i);
					string vt_s = right_t.substr(i, pos_rbracket - i + 1);
					Symbol vt(VT, vt_s);
					if (vt_s == "[z]")
						vt.symbol_type = EPSILON;
					vSet.insert(vt);
					if (vt.symbol_type == VT)  //VT���ս��
						vtSet.insert(vt);
					p1.Right.push_back(vt);
					i = pos_rbracket;
				}
			}
			p1.Left = left_tag;
			generator.push_back(p1);
			if (pos_t != -1)
				right0.erase(0, pos_t + 1);
			else
				break;
		}
	}
	return 1;
}
/******************************************************************************/
//��׼merge
bool mergeSet(set<Symbol>& dst, const set<Symbol>& src)
{
	//�ϲ�src��dst
	set<Symbol> tmp(src);
	int before = dst.size();
	set_union(begin(tmp), end(tmp), begin(dst), end(dst), inserter(dst, begin(dst)));
	//�ж�ǰ������,�жϺϲ��ɹ����
	return before < dst.size();
}
//��ǿռ��Ĳ���-{��}
bool mergeSetNoEpsilon(set<Symbol>& dst, const set<Symbol>& src)
{
	//�ϲ�src-{��}��dst
	Symbol z{ EPSILON,"[z]" };
	set<Symbol> tmp(src);
	int before = dst.size();
	if (tmp.find(z) != tmp.end())//ɾ����
		tmp.erase(z);	
	set_union(begin(tmp), end(tmp), begin(dst), end(dst), inserter(dst, begin(dst)));	
	//�ж�ǰ������,�жϺϲ��ɹ����
	return before < dst.size();
}
/******************************************************************************/
//��ʼ��first��,����VT��VN
void init_firstset(LR1& lr1)
{
	//��ʼ��firstset��map
	for (auto sym = lr1.vSet.begin(); sym != lr1.vSet.end(); ++sym) {
		set <Symbol> tmp{};
		if (sym->symbol_type == VT)  //�ַ������ս����first���Ͼ��Ǳ���
			tmp.insert(*sym);
		lr1.first_set.insert(make_pair(*sym, tmp));
	}
	return;
}


//��������vn��first����
void LR1::get_firstset_of_vn()
{
	init_firstset(*this);
	bool changed;
	while (1) {
		changed = false;
		//�������з��ս��
		for (auto vn = vnSet.begin(); vn != vnSet.end(); ++vn) {
			//�������в���ʽ
			//��ʱ���еĲ���ʽ�����ķ��ս������һ�£����ǣ��Ҳ����Ϊ�ս�������ս�����ַ�������[z]
			for (auto prod = generator.begin(); prod != generator.end(); ++prod) {
				if (prod->Left != *vn)
					continue;
				//�ҵ���Ӧ����ʽ,��ʼ�����Ҳ�����vector
				auto prod_r = prod->Right.begin();

				//�Ҳ��һ����vt or ��ֱ�Ӽ���first���ϣ�ֱ��������һ������ʽ
				//�����[z],�ȼ���first����
				if (prod_r->symbol_type == VT || prod_r->symbol_type == EPSILON) {
					changed = first_set[*vn].insert(*prod_r).second || changed;
					continue;
				}

				//�Ҳ���vn��ʼ
				bool eps = true;//���Ƶ��մ�
				for (; prod_r != prod->Right.end(); ++prod_r) {
					//��vt,ֹͣ,����VT��left��first����
					if (prod_r->symbol_type == VT) {
						changed = first_set[*vn].insert(*prod_r).second || changed;
						eps = false;
						break;
					}

					changed = mergeSetNoEpsilon(first_set[*vn], first_set[*prod_r]) || changed;
					//�����Ƶ����մ�
					eps = eps && first_set[*prod_r].count(symEps);
					//�����˳���ǰ����ʽ�Ĵ���
					if (!eps)
						break;
				}
				//�����Ҳ������Ƴ��մ�
				if (eps && (prod_r == prod->Right.end()))
					changed = first_set[*vn].insert(symEps).second || changed;
			}
		}
		if (!changed)
			break;
	}
}

//����������Ŵ���first��,����һ��set
set<Symbol> LR1::get_firstset_of_string(const vector<Symbol>& str)
{
	//set<Symbol>newSet;
	//if (str.empty())//�մ�
	//	return newSet;
	//auto s = str.begin();
	//for (; s != str.end(); ++s) {
	//	//�ս��
	//	if (s->symbol_type == VT) {
	//		mergeSetNoEpsilon(newSet, first_set[*s]);
	//		break;
	//	}
	//	//�մ�
	//	if (s->symbol_type == EPSILON) {
	//		//newSet.insert(*s);
	//		continue;
	//	}
	//	//���ս��
	//	mergeSetNoEpsilon(newSet, first_set[*s]);
	//	//��������Ƶ����մ�,�����ѭ��
	//	if (first_set[*s].count(symEps) == 0)
	//		break;
	//}
	////���ܲ����մ�,������
	//if (s == str.end())
	//	newSet.insert(symEps);
	//return newSet;
	set<Symbol> newSet;
	//strΪ��ֱ�ӷ���
	if (str.empty())
		return newSet;
	//epsilonIn�����жϿմ��Ƿ���Ҫ����
	bool EpsIn = true;

	for (auto it = str.begin(); it != str.end(); it++) {
		//������ս��
		if (it->symbol_type == VT)
		{
			mergeSetNoEpsilon(newSet, first_set[*it]);
			EpsIn = false;
			break;
		}
		//�ǿմ�
		if (it->symbol_type == EPSILON)
		{
			newSet.insert(*it);
			EpsIn = false;
			break;
		}
		//���ս�����ϲ�first����
		mergeSetNoEpsilon(newSet, first_set[*it]);
		//�����ǰ���ս�������Ƶ����մ��������ѭ��
		EpsIn = EpsIn && first_set[*it].count(symEps);
		if (!EpsIn)
			break;
	}
	//������еĶ����Բ����մ���first������մ�
	if (EpsIn)
		newSet.insert(symEps);
	return newSet;

}
/******************************************************************************/
bool deriveEps(LR1& lr1, const Symbol& s)
{
	if (s.symbol_type == EPSILON)
		return true;
	if (s.symbol_type == VT)
		return false;
	//���ս��
	bool dEps = false, one_dEps;
	for (auto prod = lr1.generator.begin(); prod != lr1.generator.end(); ++prod) {
		if (prod->Left == s) {//�ҵ�A->
			if (prod->Right[0] == symEps) {//A->��
				return true;
			}
			else {
				one_dEps = true;
				for (int i = 0; i < prod->Right.size(); ++i) {//�����ò���ʽ�����ҷ���
					if (prod->Right[i].symbol_type == VT) {//�Ҳ����ս��,����һ������ʽ
						one_dEps = false;
						break;
					}
					else if (prod->Right[i] != s && !deriveEps(lr1, prod->Right[i])) {
						one_dEps = false;
					}
					if (!one_dEps)
						break;
				}
				dEps = dEps || one_dEps;
			}
			if (dEps)
				break;
		}
	}
	return dEps;
}

////��ʼ��follow��,������VN
//void init_followset(LR1& lr1)
//{
//	//��ʼ��followset��map
//	for (auto sym = lr1.vnSet.begin(); sym != lr1.vnSet.end(); ++sym) {
//		set<Symbol>tmp{};
//		if (sym->content == "<Program>")
//			tmp.insert(Symbol(VT, "[#]"));//�ķ���ʼ���ż���[#]
//		lr1.follow_set.insert(make_pair(*sym, tmp));
//	}
//	return;
//}
////��������vt��follow����
//void LR1::get_followset_of_vn()
//{
//	bool changed;
//	while (1) {
//		changed = false;
//		//�������з��ս��
//		for (auto vn = vnSet.begin(); vn != vnSet.end(); ++vn) {//A
//			for (auto prod = generator.begin(); prod != generator.end(); ++prod) {
//				for (auto prod_r = prod->Right.begin(); prod_r != prod->Right.end(); ++prod_r) {
//					if (*prod_r == *vn) {//�ҵ��˲���ʽ .->.A.
//						if (prod_r == prod->Right.end() - 1) //����ʽ .->.A
//							changed = mergeSet(follow_set[*vn], follow_set[prod->Left]) || changed;
//						else {//����ʽ .->.A.
//							changed = mergeSetNoEpsilon(follow_set[*vn], first_set[*(prod_r + 1)]) || changed;
//							if (deriveEps(*this,*(prod_r + 1)))
//								changed = mergeSet(follow_set[*vn], follow_set[prod->Left]) || changed;
//						}
//					}
//				}
//			}
//		}
//		if (!changed)
//			break;
//	}
//}

/******************************************************************************/
//�����ع��ķ�,���ż��м���<S0>,����ʽ���ϼ���<S0>-><Program>
void get_extension(LR1& slr)
{
	//S0����
	Symbol s0(VN, "<S0>");
	slr.vSet.insert(s0);
	slr.vnSet.insert(s0);
	slr.vSet.insert(symEnd);
	slr.vtSet.insert(symEnd);
	slr.first_set.insert(make_pair(symEnd, set<Symbol>{ symEnd }));
	//<S0>-><Program>
	Production s0_s;
	s0_s.Left = s0;
	s0_s.Right.push_back(Symbol(VN, "<Program>"));
	slr.generator.insert(slr.generator.begin(), s0_s);
	Event extS(s0_s, 0, symEnd); //���� S������Ŀ��
	EventClosure inieveclo;//����չ��������Ŀ�հ����ϣ�Ĭ�Ͽղι��캯��
	inieveclo.Es.push_back(extS);
	slr.eventclo.push_back(slr.GetEventClo(inieveclo));
}


void LR1::init_items()//����generator�����ع��ķ���items����
{
	get_extension(*this);
	for (int i = 0; i < eventclo.size(); i++)
	{
		for (set<Symbol>::iterator s = vSet.begin(); s != vSet.end(); s++)
		{
			//����Ϊ�ս������ս��
			if (!((*s).symbol_type == VT || (*s).symbol_type == VN))
				continue;
			//  1. I���κ���Ŀ������CLOSURE(I)��
			/*  2. ����Ŀ[A��alpha��B beta, a]����CLOSURE(I)��B�� Sigma
				��һ������ʽ�� ��ô�� ����FIRST(beta a) �е�ÿ
				���ս��b�� ���[B����Sigma, b]ԭ������
				CLOSURE(I)�У� ������ӽ�ȥ��
				3. �ظ�ִ�в���2�� ֱ��CLOSURE(I)��������
				Ϊֹ��*/

			//�õ�����VT/VN
			EventClosure toclosure = GetEventTO(eventclo[i],(*s));
			//��Ϊ��ʱֱ��continue
			if (toclosure.Es.empty())
				continue;
			int whereclo = -1;//�±�Ų�Ϊ0
			for (int j = 0; j < eventclo.size(); j++)
			{
				if (toclosure == eventclo[j])
				{
					whereclo = j;
					break;
				}
			}
			if (whereclo != -1)//˵���Ѿ����ڣ�ֱ�ӽ�����goto��ϵ
			{
				GotoInfo[{i, (*s)}] = whereclo;
			}
			else //�����������
			{
				eventclo.push_back(toclosure);//���뵽������λ��
				GotoInfo[{i, (*s)}] = eventclo.size() - 1;
			}
		}		
	}
}


EventClosure& LR1::GetEventClo(EventClosure& CloJ)
{
	//ע�����ﲻ��ʹ�õ�������������CloJ.Es��ֵ�ı������
	for (int it = 0; it < CloJ.Es.size(); it++)
	{
		Event nowit = CloJ.Es[it];
		//���������λ�õġ�û���ս��,������A->D��
		if (nowit.dotPos >= static_cast<int>(nowit.prod.Right.size()))
			continue;
		Symbol next_symbol = nowit.prod.Right[nowit.dotPos];//��֮��ķ�����ʲô
		if (next_symbol.symbol_type == VT) //������ս��
			continue;
		if (next_symbol.symbol_type == EPSILON)//�����epsilon��ִ�С�epsilon->epsilon��
		{
			CloJ.Es[it].dotPos++;
			continue;
		}
		//�������ͨ������ʽA->alpha B beta , a����beta a��first����
		//��dotPos=sizeʱ���������������������˲���Խ��

		vector<Symbol> ba((nowit.prod.Right.begin() + nowit.dotPos+1 ), nowit.prod.Right.end());
		ba.push_back(nowit.ahead);//����չ�������չ������

		set<Symbol> firstba = get_firstset_of_string(ba); //beta a��first����
		//for(auto i=firstba.begin();)

		//�ҵ�nextsymbol��ʼ�Ĳ���ʽ
		for (auto pro = generator.begin(); pro != generator.end(); pro++)
		{
			if (pro->Left != next_symbol)//������ǣ���ô�ͽ��ű���
				continue;
			//��������в���ʽ���ɿյ���ʽA->epsilon��
			//����ҵ�������CloJ
			for (auto beta = firstba.begin(); beta != firstba.end(); beta++)
			{
				//�������ʽ�Ҳ���epslon����ʽ,��ô���ٿ���epsilon��ת�ƣ�ֱ�Ӽ���epsilon������
				//[A->alpha B beta , a] ;b in first(beta a) ; B-> E; [B->E, b ]����ǰ׺��Ȼ��Ч
				//������Ч��[B->E, b ]��ȫ������
				if ((*pro).Right.front().symbol_type == EPSILON)
				{					
					Event tmp(*pro, 1, *beta);//��ʱ�������epsilon��������dotposֱ��ȡ1
					vector<Event>::iterator over;
					for (over = CloJ.Es.begin(); over != CloJ.Es.end(); over++)
					{
						if (*over == tmp)
							break;
					}
					if (over == CloJ.Es.end())
					{
						CloJ.Es.push_back(tmp);
						//cout << "Episi" << *beta << endl;
					}
				}
				else //����ʽ�������մ�
				{
					Event tmp(*pro, 0, *beta);//��ʱ�������epsilon��������dotposֱ��ȡ1
					vector<Event>::iterator over;
					for (over = CloJ.Es.begin(); over != CloJ.Es.end(); over++)
					{
						if (*over == tmp)
							break;
					}
					if (over == CloJ.Es.end())
					{
						CloJ.Es.push_back(tmp);
						//cout<< *beta << endl;
						
					}
				}
			}
		}

	}
	return CloJ;
}

EventClosure LR1::GetEventTO(EventClosure& CloI, const Symbol sym)
{
	// TODO: �ڴ˴����� return ���
	EventClosure toclo;
	////��������Ѿ���VT��VN,������Ҫ�󷵻ؿ�
	//if (!(sym.symbol_type == VT || sym.symbol_type == VN))
	//	return toclo;

	for (auto it = CloI.Es.begin(); it != CloI.Es.end(); it++)
	{
		//�ж�it�����һ���ַ��ǡ�������ֱ������
		if ((*it).dotPos >= (*it).prod.Right.size())
			continue;

		//���һ���ַ����� present_symbol�������ж�
		if ((*it).prod.Right[(*it).dotPos] != sym)
			continue;
		// A.BC������B ,.aB,����a
		toclo.Es.push_back({ (*it).prod,(*it).dotPos + 1,(*it).ahead });		
	}

	//�����˴�clo(I)����VT/VN�õ���ת���������û��������FIRST(beta a)���ս���Ĳ���
	//��ʱ�õ�����ֱ�ӿ���һ���õ��Ľ��
	//�±���First(beta a) 
	return GetEventClo(toclo);
}


static int findindex(const Production& prod,const LR1 &lr1)
{
	for (int i = 0; i < lr1.generator.size(); ++i) {
		if (prod == lr1.generator[i])
			return i;
	}

	return -1;//�������Ҳ���,���ǻ���д������
}

/****************���ϲ�********************/
//����Table
void LR1::getTable()
{
	for (int clo_index = 0; clo_index < eventclo.size(); ++clo_index) {
		for (int it_index = 0; it_index != eventclo[clo_index].Es.size(); ++it_index) {
			Event current = eventclo[clo_index].Es[it_index];

			//����ĩβ
			if (current.dotPos >= current.prod.Right.size()) {
				//�Ƿ�Ϊ��չ��ʼ����?��,���й�Լ
				if (current.prod.Left != symS0)
					ActionTable[{clo_index, current.ahead}] = { REDUCE, findindex(current.prod,*this) };
				else
					ActionTable[{clo_index, current.ahead}] = { ACCEPT,-1 };
			}
			//dot����ĩβ
			else {
				Symbol SymAfterDot = current.prod.Right[current.dotPos];
				//���ս��
				if (SymAfterDot.symbol_type == VN) {
					auto it = GotoInfo.find({ clo_index, SymAfterDot });
					if (it != GotoInfo.end()) //�ҵ�
						GotoTable[{clo_index, SymAfterDot}] = { SHIFTIN, it->second};
				}
				else if (SymAfterDot.symbol_type == VT) {
					auto it = GotoInfo.find({ clo_index, SymAfterDot });
					if (it != GotoInfo.end()) //�ҵ�
						ActionTable[{clo_index, SymAfterDot}] = { SHIFTIN, it->second };
				}
			}
		}
	}
}


//��ӡTable
void LR1::printTable(const string file_path)
{
	ofstream fout(file_path, ios::out);

	fout << " ,";
	for (auto vt : vtSet)
		fout << vt << ",";
	fout << "||,";
	for (auto vn : vnSet)
		if (vn != symS0)
			fout << vn << ",";

	fout << endl;
	for (int i = 0; i < eventclo.size(); ++i) {
		fout << i << ",";
		for (auto vt : vtSet) {
			auto current = ActionTable.find({ i, vt });
			if (current == ActionTable.end())
				fout << " ,";
			else {
				if (current->second.action == ACCEPT)
					fout << "acc,";
				else if (current->second.action == SHIFTIN)
					fout << "s" << current->second.index << ",";
				else if (current->second.action == REDUCE)
					fout << "r" << current->second.index << ",";
				else
					fout << ",";
			}

		}
		fout << "||,";
		for (auto vn : vnSet) {
			if (vn == symS0)
				continue;

			auto current = GotoTable.find({ i, vn });
			if (current == GotoTable.end())
				fout << " ,";
			else
				fout << current->second.index << ",";
		}
		fout << endl;
	}
	fout.close();
}

void LR1::grammartree(const string filepath, queue<SymToken>& Code)
{
	///*********************************************************************/
	////��ʼ�� semantic ��
	//SemanticAnalysis = SemanticAnalyzer();
	///*********************************************************************/

	vector<Symbol> Symstack; //����ջ
	vector<int>Statestack;  //״̬ջ
	vector<int> printsymstac; //��¼��ӡ����ջ
	int prt = 0;// ��ӡʱ�ı��
	stringstream sstr;
	//�����ļ��򿪣�.dot
	fstream fout;
	fout.open(filepath, ios::out);
	if (fout.is_open() == false)
	{
		cout << "Dot File open Failed" << endl;
	}
	sstr << "digraph tree { " << endl;
	Statestack.push_back(0);
	Symstack.push_back({symEnd});
	SymToken edsy = {{-1,strend,LexComponent::End},symEnd};//
	Code.push(edsy);//

	stack<IdentNode*> treeNodeStack;
	IdentNode* tp;

	/*********************************************************************/

	//�� Start ���� semantic ����������б�
	//SemanticAnalysis.insertSymbol( {{-1,"",strstart},-1,- 1 } );

	/*********************************************************************/

	//int pos;
	while (true&&!Code.empty())
	{
		Symbol first = Code.front().sym;
		Token firtok = Code.front().tok;


		auto current = ActionTable.find({ Statestack.back(),first });		
		//���1��ERROR
		if (current == ActionTable.end()) {
			cout << "ACTIONERROR : Symbol "<<firtok.content <<"can't find, error row is "<< firtok.line << endl;
			exit(-1);
			break;
		}
		//���2��ACCEPT
		else if (current->second.action == ACCEPT) {
			cout << "ACCEPT" << endl;
			sstr << "}" << endl;
			Seman_tree(reductionTreeRoot, 0);
			break;
		}
		//���3��SHIFTIN
		else if (current->second.action == SHIFTIN) {
			Symstack.push_back(first);
			Statestack.push_back(current->second.index);			
			Code.pop();
			printsymstac.push_back(prt);
			sstr << "v" << prt++ << "[label=\"" << first << "\",fillcolor=paleturquoise,shape=doublecircle,style=filled];" << endl;

			tp = new(nothrow)IdentNode;
			if (!tp) { exit(-2); }

			tp->content = pair<string, int>(first.content, this->lex.getcount(firtok.content));
			treeNodeStack.push(tp);


		}
		//���4����Լ
		else if (current->second.action == REDUCE) {
			//GotoTable
			//�ҵ�generator�еĹ�Լ״̬
			Production now = generator[current->second.index];
			int Rightnum = int(now.Right.size());

			vector<int>printsym;//Ϊ��ӡ��׼��
			if (now.Right.front().symbol_type != EPSILON)
			{
				while (Rightnum--)
				{
					Symstack.pop_back();
					Statestack.pop_back();
					printsym.push_back(printsymstac.back());
					printsymstac.pop_back();//�Լ���ӡ��ջpop					
				}
			}
			else //��ǰ��ԼʽΪ�չ�Լʽ
			{
				tp = new(nothrow)IdentNode;
				if (!tp) { exit(-2); }

				tp->content = pair<string, int>("[z]", -1);
				treeNodeStack.push(tp);
			}
			tp = new(nothrow)IdentNode;
			if (!tp) { exit(-2); }
			tp->content = pair<string, int>(now.Left.content, -1);

			for (auto it = now.Right.begin(); it != now.Right.end(); it++)
			{
				treeNodeStack.top()->parent = tp;// wrong
				tp->children.push_back(treeNodeStack.top());
				treeNodeStack.pop();
			}
			reverse(tp->children.begin(), tp->children.end());
			reductionTreeRoot = tp;
			treeNodeStack.push(tp);

			SemanticAnalysis.seman_analysis(now, tp, lex.NameTable);

			Symbol syleft = now.Left;
			//״̬վ��Goto��
			Symstack.push_back(syleft);

			int state = Statestack.back();
			Symbol codestart = Code.front().sym;
			auto goto_id = GotoTable.find({ state, syleft });
			if (goto_id == GotoTable.end())
			{
				cout << "GOTOERROR" << endl;
				exit(-1);
				break;
			}
			Statestack.push_back(goto_id->second.index);

			printsymstac.push_back(prt);
			sstr << "v" << prt++ << "[label=\"" << syleft << "\""<<",fillcolor=mistyrose,shape=doubleoctagon,style=filled" <<"];" << endl;
			if (printsym.size() != 0)
			{
				for (auto t = printsym.begin(); t != printsym.end(); t++)
					sstr << "v" << prt - 1 << " -> " << "v" << *t << ";" << endl;
			}
			else //��ʾ�ǿմ�
			{
				sstr << "e" << prt << "[label=\"[Z]\"];" << endl;
				sstr << "v" << prt - 1 << "-> e" << prt << ";" << endl;
			}

			/*********************************************************************/

			//�����������
			//�������������ʱ��JUJU������ֻ�õ��˲���ʽ��ߺ��ұߵ��ַ���������ȫ������

			//SemanticAnalysis.semanticANL(now);

			/*********************************************************************/

		}
	}
	fout << sstr.str();
	fout.close();
}



void LR1::Seman_analysis(queue<SymToken>& Code)
{
	vector<Symbol> Symstack; //����ջ
	vector<int>Statestack;  //״̬ջ
	Statestack.push_back(0);
	Symstack.push_back({ symEnd });
	SymToken epsilon_next_lexis;
	SymToken edsy = { {-1,strend,LexComponent::End},symEnd };//
	Code.push(edsy);//

	stack<IdentNode*> treeNodeStack;
	IdentNode* tp;

	/*********************************************************************/
	//�� Start ���� semantic ����������б�
	//SemanticAnalysis.insertSymbol( {{-1,"",strstart},-1,- 1 } );

	/*********************************************************************/

	//int pos;
	while (true)
	{
		Symbol first = Code.front().sym;
		Token firtok = Code.front().tok;

		auto current = ActionTable.find({ Statestack.back(),first });
		//���1��ERROR
		if (current == ActionTable.end()) {
			cout << "ACTIONERROR : Symbol " << firtok.content << "can't find, error row is " << firtok.line << endl;
			exit(-1);
			break;
		}
		//���2��ACCEPT
		else if (current->second.action == ACCEPT) {
			cout << "ACCEPT" << endl;
			Seman_tree(reductionTreeRoot,0);
			break;
		}
		//���3��SHIFTIN
		else if (current->second.action == SHIFTIN) {
			Symstack.push_back(first);
			Statestack.push_back(current->second.index);
			Code.pop();
			/*********************************************************************/
			/*string tmp_in;
			if (firtok.type == LexComponent::ID)
				tmp_in = "[ID]";
			else if (firtok.type == LexComponent::Digit)
				tmp_in = "[INT]";
			else
				tmp_in = firtok.content;*/
			//�� token ���� semantic ����������б�
			//SemanticAnalysis.insertSymbol({{ firtok.line,firtok.content,tmp_in } , -1,- 1});
			/*********************************************************************/
			tp = new(nothrow)IdentNode;
			if (!tp) { exit(-2); }

			tp->content = pair<string,int>(first.content, this->lex.getcount(firtok.content));
			treeNodeStack.push(tp);

		}
		//���4����Լ
		else if (current->second.action == REDUCE) {
			//GotoTable
			//�ҵ�generator�еĹ�Լ״̬
			Production now = generator[current->second.index];
			int Rightnum = int(now.Right.size());

			if (now.Right.front().symbol_type != EPSILON)
			{
				while (Rightnum--)
				{
					Symstack.pop_back();
					Statestack.pop_back();
				}
			}
			else //��ǰ��ԼʽΪ�չ�Լʽ
			{
				tp = new(nothrow)IdentNode;
				if (!tp) { exit(-2); }

				tp->content = pair<string, int>("[z]",-1);
				treeNodeStack.push(tp);

				//Symstack.push_back(symEps); //
				//Statestack.push_back(current->second.index);
			}



			tp = new(nothrow)IdentNode;
			if (!tp) { exit(-2); }
			tp->content = pair<string, int>(now.Left.content, -1);

			for (auto it = now.Right.begin(); it != now.Right.end(); it++)
			{
				treeNodeStack.top()->parent = tp;// wrong
				tp->children.push_back(treeNodeStack.top());
				treeNodeStack.pop();
			}
			reverse(tp->children.begin(), tp->children.end());
			reductionTreeRoot = tp;
			treeNodeStack.push(tp);

			SemanticAnalysis.seman_analysis(now,tp,lex.NameTable);

			Symbol syleft = now.Left;
			//״̬ջ��Goto��
			Symstack.push_back(syleft);

			int state = Statestack.back();

			Symbol codestart = Code.front().sym;
			auto goto_id = GotoTable.find({ state, syleft });
			if (goto_id == GotoTable.end())
			{
				cout << "GOTOERROR,"<< syleft <<"not find" << endl;
				exit(-1);
				break;
			}
			Statestack.push_back(goto_id->second.index);


		}
	}
}

void LR1::Seman_tree(IdentNode* thenode, int thelevel)
{
	if (thelevel > maxtreelevel)
		maxtreelevel = thelevel;
	if (thenode->children.size() == 0)
		leafnum++;
	thenode->level = thelevel;
	for (int i = 0; i < thenode->children.size(); i++)
	{
		Seman_tree(thenode->children[i], thelevel + 1);
	}
}



/*************�������******************/
void LR1::LexGo() {
	char temp;
	cout << "=========================================" << endl;
	cout << "��PART 1�� �ʷ�����" << endl;
	cout << "=========================================" << endl;

	init_map();
	this->lex.lex_analyze(this->targetCode);

	cout << "�ʷ��������������,�Ƿ�����ļ�?	Y:ȷ��	����:ȡ��" << endl;
	cin >> temp;
	if (temp == 'Y' || temp == 'y') {
		this->lex.output_result("./res/LexOutput.txt");
		cout << "�ʷ���������ѱ����� ./res/LexOutput.txt" << endl;
	}
	cout << endl;
}

void LR1::SemanticGo() {

	char temp;
	char* ptemp;
	cout << "=========================================" << endl;
	cout << "��PART2�� �﷨ & ������� & �м��������" << endl;
	cout << "=========================================" << endl;

	cout << "���ڴ�ӡ�﷨��ing..." << endl;
	this->grammartree("./res/tree.dot", this->lex.Result);
	cout << "�﷨���ѱ����� ./res/tree.dot" << endl;

	ptemp = getenv("PATH");
	if (ptemp && strstr(ptemp, "Graphviz")) {
		cout << endl;
		cout << "�� ��⵽ϵͳ������������ Graphviz,�Ƿ���Ҫ�����﷨��ͼƬ?	Y:ȷ��	����:ȡ�� ��" << endl;
		cin >> temp;
		if (temp == 'Y' || temp == 'y') {
			system("dot -Tpng ./res/tree.dot -o ./res/tree.png");
			cout << "�﷨��ͼƬ(.png)�ѱ����� ./res/tree.png" << endl;
		}

	}

	cout << endl;

	cout << "���ڴ�ӡ��Ԫʽ�м����ing..." << endl;
	this->SemanticAnalysis.printQuadruple("./res/QuadrupleRes.txt");
	cout << "��Ԫʽ�м�������ѱ����� ./res/QuadrupleRes.txt" << endl;

	cout << endl;

}