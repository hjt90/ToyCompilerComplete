#include <sstream>
#include <string>
#include <iostream>
#include <algorithm>
#include "Parsing.h"
using namespace std;

proc_symbolTable *beforeSymbolTable;

syntaxTreeNode::syntaxTreeNode(const pair<Token, string> &rhs) : index(0), parent(-1), productions(-1), type(0), inTree(false)
{
	this->type = int(rhs.first);
	this->val = rhs.second;
}

syntaxTreeNode::syntaxTreeNode() : index(0), parent(-1), productions(-1), type(0), inTree(false), quad(0), stype(symbolType::Unknown)
{
}

/*********
 * ���뵽 symbolTable �У�ͬʱ���� symbol2Index ��
 * ��������ھͲ��룬������ֱ�ӷ������
 * ********/
symbolTableIndex parsing::insertSymbol(symbolItem insrt)
{
	if (!this->symbol2Index.count(insrt)) // map�в�����
	{
		symbolTable.push_back(insrt);
		symbol2Index[insrt] = symbolTable.size() - 1;
	}
	return symbol2Index[insrt];
}

void parsing::initTerminalSymbol()
{
	insertSymbol("$ID");
	insertSymbol("$Int");
	insertSymbol("$Void");
	insertSymbol("$If");
	insertSymbol("$Else");
	insertSymbol("$While");
	insertSymbol("$Return");
	insertSymbol("$Plus");
	insertSymbol("$Minus");
	insertSymbol("$Multiply");
	insertSymbol("$Divide");
	insertSymbol("$Equal");
	insertSymbol("$Equal2");
	insertSymbol("$Bigger");
	insertSymbol("$BiggerEqual");
	insertSymbol("$Smaller");
	insertSymbol("$SmallerEqual");
	insertSymbol("$NotEqual");
	insertSymbol("$LeftArray");
	insertSymbol("$RightArray");
	insertSymbol("$Semi");
	insertSymbol("$Comma");
	insertSymbol("$LeftAnno");
	insertSymbol("$RightAnno");
	insertSymbol("$Anno");
	insertSymbol("$LeftBracket");
	insertSymbol("$RightBracket");
	insertSymbol("$LeftBrace");
	insertSymbol("$RightBrace");
	insertSymbol("$Number");
	insertSymbol("$End");
	insertSymbol("$Empty");
	insertSymbol("$Start0");
	insertSymbol("$Start");
	this->terminalSymbolMax = symbolTable.size() - 1 - 2;
	this->startIndex = this->symbol2Index["$Start0"];
	this->emptyIndex = this->symbol2Index["$Empty"];
}

/*********
 * ��ʼ�� symbolTable��terminalSymbolMax��startIndex
 * 		symbol2Index��syntaxTable
 * ********/
void parsing::initSymbolTable(ifstream &infile)
{
	initTerminalSymbol();
	int line = 0;
	char templine[1024]; //��һ��
	string tmpstr;
	while (infile.getline(templine, 1024))
	{
		line++;
		syntaxTableItem tmpSyntax;
		stringstream ss(templine);
		if (ss >> tmpstr)
		{
			if (!((tmpstr[0] == '<' && tmpstr.back() == '>') || tmpstr[0] == '$'))
				cout << "�﷨����������:"
					 << "�﷨�����" << line << "�У���಻�Ƿ��ս�� " << tmpstr << endl;
			tmpSyntax.lhs = insertSymbol(tmpstr);

			ss >> tmpstr;
			if (tmpstr != "::=")
				cout << "�﷨����������:"
					 << "�﷨�����" << line << "�У�δ�ҵ���ֵ��(::=)����Ϊ" << tmpstr << endl;
			while (ss >> tmpstr)
			{
				tmpSyntax.rhs.push_back(insertSymbol(tmpstr));
			}
			this->syntaxTable.push_back(tmpSyntax);
		}
	}

	this->syntaxTable.push_back({this->startIndex, {this->symbol2Index["$Start"]}}); //���S' ==> S
	searchSyntaxByLhs = vector<set<syntaxTableIndex>>(symbolTable.size());
	for (int i = 0; i < syntaxTable.size(); i++)
	{
		searchSyntaxByLhs[syntaxTable[i].lhs].insert(i);
	}
}

/*********
 * ��ʼ�� firstTable
 * ********/
void parsing::initFirstTable()
{
	this->firstTable = vector<firstTableItem>(symbolTable.size());
	//�ս�� FIRST��Ϊ������
	for (int i = 0; i <= this->terminalSymbolMax; i++)
	{
		this->firstTable[i].insert(i);
	}
	//���ս��FIRST���������ʽ�����
	for (int i = this->terminalSymbolMax + 1; i < this->symbolTable.size(); i++)
	{
		for (auto syntaxIndexTmp : this->searchSyntaxByLhs[i])
		{
			symbolTableIndex symbolTmp = this->syntaxTable[syntaxIndexTmp].rhs[0];
			if (symbolTmp < this->terminalSymbolMax) //����ʽ�Ҳ�����Ϊ�ս��,�ǿ�
				this->firstTable[i].insert(symbolTmp);
		}
	}
	//���ս��FIRST������
	bool inc = false;
	do
	{
		inc = false;
		for (int i = this->terminalSymbolMax + 1; i < this->symbolTable.size(); i++) //����ÿһ�����ս����
		{
			int cntTmp = this->firstTable[i].size();
			for (auto syntaxIndexTmp : this->searchSyntaxByLhs[i]) //������Ϊ����Ĳ���ʽ
			{
				const vector<symbolTableIndex> &rhsTmp = this->syntaxTable[syntaxIndexTmp].rhs;
				for (int rhsIndex = 0; rhsIndex < rhsTmp.size(); rhsIndex++) //����������
				{
					const firstTableItem &firstSymbolSet = this->firstTable[rhsTmp[rhsIndex]];
					if (rhsTmp[rhsIndex] <= this->terminalSymbolMax) //Ϊ�ս��
					{
						this->firstTable[i].insert(rhsTmp[rhsIndex]);
						break;
					}

					bool haveEmpty = this->firstTable[i].count(this->emptyIndex);
					this->firstTable[i].insert(firstSymbolSet.cbegin(), firstSymbolSet.cend());
					if (!haveEmpty && firstSymbolSet.count(this->emptyIndex))
						this->firstTable[i].erase(this->emptyIndex);

					if (!firstSymbolSet.count(this->emptyIndex)) //������
						break;
					if (rhsIndex == rhsTmp.size() - 1) //�������ʽ�Ҳ�ȫΪ�գ������
						this->firstTable[i].insert(this->emptyIndex);
				}
			}
			if (this->firstTable[i].size() > cntTmp)
				inc = true;
		}
	} while (inc); //ֱ����������
}

/*********
 * ������ӵ�FIRST��
 * ********/
set<symbolTableIndex> parsing::firstForPhrase(vector<symbolTableIndex> rhsTmp)
{
	set<symbolTableIndex> res;

	for (int rhsIndex = 0; rhsIndex < rhsTmp.size(); rhsIndex++) //����������
	{
		const firstTableItem &firstSymbolSet = this->firstTable[rhsTmp[rhsIndex]];
		if (rhsTmp[rhsIndex] <= this->terminalSymbolMax) //Ϊ�ս��
		{
			res.insert(rhsTmp[rhsIndex]);
			break;
		}

		bool haveEmpty = res.count(this->emptyIndex);
		res.insert(firstSymbolSet.cbegin(), firstSymbolSet.cend());
		if (!haveEmpty && firstSymbolSet.count(this->emptyIndex))
			res.erase(this->emptyIndex);

		if (!firstSymbolSet.count(this->emptyIndex)) //������
			break;
		if (rhsIndex == rhsTmp.size() - 1) //�������ʽ�Ҳ�ȫΪ�գ������
			res.insert(this->emptyIndex);
	}

	return res;
}

/*********
 * ����<���ڱȽ�DFA_item
 * ********/
bool operator<(const DFA_item &A, const DFA_item &B)
{
	if (A.lhs < B.lhs)
		return true;
	else if (A.lhs > B.lhs)
		return false;

	if (A.rhs.size() < B.rhs.size())
		return true;
	else if (A.rhs.size() > B.rhs.size())
		return false;
	else
	{
		int i;
		int mm = A.rhs.size();
		for (i = 0; i < mm; i++)
		{
			if (A.rhs[i] < B.rhs[i])
				return true;
			else if (A.rhs[i] > B.rhs[i])
				return false;
		}
	}

	if (A.pos < B.pos)
		return true;
	else if (A.pos > B.pos)
		return false;

	if (A.forecast < B.forecast)
		return true;
	else if (A.forecast > B.forecast)
		return false;

	return false;
}

/*********
 * ����==���ڱȽ�DFA_item
 * ********/
bool operator==(const DFA_item &A, const DFA_item &B)
{
	if (A.lhs == B.lhs && A.rhs == B.rhs && A.pos == B.pos && A.forecast == B.forecast)
		return true;
	else
		return false;
}

/*********
 * ����closure
 * ********/
pair<int, bool> parsing::createClosure(DFA_status &sta)
{
	set<int> tempfirst;
	vector<symbolTableIndex> restsentence;
	stack<DFA_item> sd;
	DFA_item temptop, tempd;
	//�Ȱ�������ջ
	for (auto it = sta.begin(); it != sta.end(); it++)
	{
		sd.push(*it);
	}
	while (!sd.empty())
	{
		temptop = sd.top();
		sd.pop();																			  //ջ��������ջ
		if (temptop.pos < temptop.rhs.size() && temptop.rhs[temptop.pos] > terminalSymbolMax) //������Ƿ��ս��
		{
			restsentence.clear();
			for (int i = temptop.pos + 1; i < temptop.rhs.size(); i++) //��ȡ��Ҫ��first�������
			{
				restsentence.push_back(temptop.rhs[i]);
			}
			restsentence.push_back(temptop.forecast);
			tempfirst = firstForPhrase(restsentence); //��first��
			//������first������ʼ�����µľ���
			for (auto it = tempfirst.begin(); it != tempfirst.end(); it++)
			{
				for (auto it2 = searchSyntaxByLhs[temptop.rhs[temptop.pos]].begin(); it2 != searchSyntaxByLhs[temptop.rhs[temptop.pos]].end(); it2++)
				{
					tempd.lhs = temptop.rhs[temptop.pos];
					tempd.rhs = syntaxTable[*it2].rhs;
					tempd.pos = 0;
					tempd.forecast = *it;
					if (sta.insert(tempd).second == true) //�ɹ�����
					{
						sd.push(tempd); //��Ϊһ���µ�ʽ����ջ
					}
				}
			}
		}
	}
	//����Ƿ�����״̬
	for (int i = 0; i < DFA.size(); i++)
	{
		if (DFA[i] == sta)
			return pair<int, bool>(i, false);
	}
	DFA.push_back(sta);
	return pair<int, bool>(DFA.size() - 1, true);
}

/*********
 * ��ʼ�� DFA��analyseTable
 * ********/
void parsing::initAnalyseTable()
{
	DFA_status temps;
	DFA_item temptop, tempd;
	set<int> tempfirst;
	vector<symbolTableIndex> restsentence;
	pair<int, bool> gt;
	int statusno;
	stack<int> si;
	//���ֶ������ʼ״̬0
	//��һ��S'->.S,#
	tempd.lhs = symbol2Index["$Start0"];
	tempd.rhs.push_back(symbol2Index["$Start"]);
	tempd.pos = 0;
	tempd.forecast = symbol2Index["$End"];
	temps.insert(tempd);

	createClosure(temps); //����0��״̬

	si.push(0); //��״̬0��ջ

	//���˵�0��״̬�������
	//��������ʼ�Ƶ�ʣ��״̬
	set<int> transflag; //��¼����Щ���ſ�������ת��
	analyseTable.push_back(vector<analyseTableItem>(symbolTable.size(), pair<char, int>('\0', -1)));
	bool empty_flag = 0; //��¼�Ƿ����Ƴ���

	while (!si.empty())
	{
		empty_flag = 0;
		// debugdfa();//����
		statusno = si.top();
		// DFA_status& DFA[statusno] = DFA[statusno];
		si.pop();
		transflag.clear();
		for (auto it = DFA[statusno].begin(); it != DFA[statusno].end(); it++) //�ҵ�������������п���ת�Ƶ��ַ�
		{
			if ((*it).pos < (*it).rhs.size())
			{
				transflag.insert((*it).rhs[(*it).pos]);
			}
		}
		for (auto it = transflag.begin(); it != transflag.end(); it++) //����ÿ��������ת�Ƶ��ַ������ƽ�״̬
		{
			if (*it == emptyIndex) //���⴦������empty�����ݣ�����ԭ��״̬��дת�Ʊ�
			{
				// empty_flag = 1;//����Ƴ��գ�����ڴ���Ȼ����ѭ��
				/*
				for (auto it2 = DFA[statusno].begin(); it2 != DFA[statusno].end(); it2++) //����ÿһ�����
				{
					if ((*it2).pos < (*it2).rhs.size() && (*it2).rhs[(*it2).pos] == *it)
					{
						tempd.lhs = (*it2).lhs;
						tempd.rhs = (*it2).rhs;
						tempd.pos = (*it2).pos + 1;
						tempd.forecast = (*it2).forecast;
						DFA[statusno].insert(tempd);
					}
				}
				*/
				;
			}
			else //����ת��дת�Ʊ�
			{
				temps.clear();
				for (auto it2 = DFA[statusno].begin(); it2 != DFA[statusno].end(); it2++) //����ÿһ�����
				{
					if ((*it2).pos < (*it2).rhs.size() && (*it2).rhs[(*it2).pos] == *it)
					{
						tempd.lhs = (*it2).lhs;
						tempd.rhs = (*it2).rhs;
						tempd.pos = (*it2).pos + 1;
						tempd.forecast = (*it2).forecast;
						temps.insert(tempd);
					}
				}
				gt = createClosure(temps);
				analyseTable[statusno][*it] = pair<char, int>('s', gt.first);
				if (gt.second == true) //���µ�״̬
				{
					si.push(gt.first);
					analyseTable.push_back(vector<analyseTableItem>(symbolTable.size(), pair<char, int>('\0', -1)));
				}
			}
		}
		for (auto it = DFA[statusno].begin(); it != DFA[statusno].end(); it++) //�ҹ�Լ״̬
		{
			if ((*it).pos >= (*it).rhs.size() || (*it).rhs[0] == emptyIndex)
			{
				int synno;
				//�ҵ�����һ����Լ�ķ�
				for (auto it2 = searchSyntaxByLhs[(*it).lhs].begin(); it2 != searchSyntaxByLhs[(*it).lhs].end(); it2++)
				{
					if (syntaxTable[*it2].rhs == (*it).rhs)
					{
						synno = *it2;
						break;
					}
				}
				//���Լ��
				if (analyseTable[statusno][(*it).forecast].first != '\0')
					cout << "wrong" << endl;
				if ((*it).lhs == symbol2Index["$Start0"] && (*it).rhs[0] == symbol2Index["$Start"] && (*it).forecast == symbol2Index["$End"])
					analyseTable[statusno][(*it).forecast] = pair<char, int>('a', -1);
				else
					analyseTable[statusno][(*it).forecast] = pair<char, int>('r', synno);
			}
		}
	}
}

void parsing::debugdfa() //��������dfa
{
	ofstream of;
	of.open("dfadebug.txt");
	for (int i = 0; i < DFA.size(); i++)
	{
		of << "״̬" << i << endl;
		for (auto it = DFA[i].begin(); it != DFA[i].end(); it++)
		{
			of << symbolTable[(*it).lhs] << " ::= ";
			for (int j = 0; j < (*it).rhs.size(); j++)
			{
				if (j == (*it).pos)
					of << " dot ";
				of << symbolTable[(*it).rhs[j]] << ' ';
			}
			if ((*it).pos == (*it).rhs.size())
				of << " dot ";
			of << "," << symbolTable[(*it).forecast];
			of << endl;
		}
	}
	of.close();
}

void parsing::clear()
{
	symbolTable.clear();
	symbol2Index.clear();
	syntaxTable.clear();
	searchSyntaxByLhs.clear();
	firstTable.clear();
	DFA.clear();
	analyseTable.clear();
	syntaxTree.clear();
	statusStack = stack<DFA_statusIndex>();
	analyseSymbolStack = stack<syntaxTreeNodeIndex>();
	inputSymbolvector = stack<syntaxTreeNodeIndex>();
}

void parsing::initSyntax(ifstream &fin)
{
	this->initSymbolTable(fin);
	this->initFirstTable();
	this->initAnalyseTable();
	this->debugdfa();
}

void parsing::analyze(const vector<pair<Token, string>> &lexs)
{
	//��ʼ���������ջ
	this->syntaxTree.push_back(pair<Token, string>{Token::End, ""});
	this->syntaxTree.back().index = syntaxTree.size() - 1;
	this->inputSymbolvector.push(syntaxTree.size() - 1);
	for (int i = lexs.size() - 1; i >= 0; i--)
	{
		this->syntaxTree.push_back(lexs[i]);
		syntaxTreeNodeIndex index = syntaxTree.size() - 1;
		this->syntaxTree.back().index = index;
		this->inputSymbolvector.push(index);
	}
	//��ʼ������״̬ջ
	this->statusStack.push(0);
	//��ʼ����������ջ
	this->syntaxTree.push_back(pair<Token, string>{Token::End, ""});
	this->syntaxTree.back().index = syntaxTree.size() - 1;
	this->analyseSymbolStack.push(syntaxTree.size() - 1);
	this->p_symbolTable = new proc_symbolTable();

	while (1)
	{
		if (this->syntaxTree[this->analyseSymbolStack.top()].type == this->symbol2Index["$Start"] && this->syntaxTree[this->inputSymbolvector.top()].type == this->symbol2Index["$End"])
			break;
		analyseTableItem nextAction = this->analyseTable[this->statusStack.top()][this->syntaxTree[this->inputSymbolvector.top()].type];
		if (nextAction.first == 'a') //�ɹ�
			break;
		else if (nextAction.first == 's') //�ƽ�
		{
			this->statusStack.push(nextAction.second);
			syntaxTreeNodeIndex tmp = this->inputSymbolvector.top();
			this->inputSymbolvector.pop();
			this->analyseSymbolStack.push(tmp);
		}
		else if (nextAction.first == 'r') //��Լ
		{
			const syntaxTableItem &useSyntax = this->syntaxTable[nextAction.second];
			vector<syntaxTreeNodeIndex> rhsTmp;
			if (useSyntax.rhs[0] != this->emptyIndex)
			{
				for (int i = 0; i < useSyntax.rhs.size(); i++)
				{
					syntaxTreeNodeIndex tmp = this->analyseSymbolStack.top();
					this->analyseSymbolStack.pop();
					this->statusStack.pop();
					rhsTmp.push_back(tmp);
					syntaxTree[tmp].parent = syntaxTree.size();
				}
			}
			syntaxTreeNode lhsTmp;
			reverse(rhsTmp.begin(), rhsTmp.end());
			lhsTmp.children = rhsTmp;
			lhsTmp.productions = nextAction.second;
			lhsTmp.type = useSyntax.lhs;
			this->generate_midcode(nextAction.second, lhsTmp, rhsTmp);
			syntaxTree.push_back(lhsTmp);
			this->syntaxTree.back().index = syntaxTree.size() - 1;
			this->analyseSymbolStack.push(syntaxTree.size() - 1);
			this->topNode = syntaxTree.size() - 1;

			nextAction = this->analyseTable[this->statusStack.top()][this->syntaxTree[this->analyseSymbolStack.top()].type];
			if (nextAction.first == 's')
			{
				this->statusStack.push(nextAction.second);
			}
			else
			{
				cout << "�﷨��������" << endl;
				break;
			}
		}
		else //����
		{
			cout << "�﷨��������" << endl;
			break;
		}
	}
}

void parsing::generate_midcode(syntaxTableIndex SyntaxIndex, syntaxTreeNode &lhs, vector<syntaxTreeNodeIndex> &rhs)
{
	proc_symbolTable *functmp;
	switch (SyntaxIndex)
	{
	case 0: //$Start ::= <N> <������>
		mid_code.back_patch(syntaxTree[rhs[0]].nextlist, this->p_symbolTable->find_function("main")->get_enterquad());
		break;
	case 1: //<A> ::= $Empty
		lhs.quad = mid_code.nextquad;
		p_symbolTable = p_symbolTable->create_function(); //���뺯��
		break;
	case 2: //<N> ::= $Empty
		lhs.nextlist.push_back(mid_code.nextquad);
		mid_code.emit_code(quadruple(Oper::J, string(""), string(""), string("")));
		break;
	case 3: //<M> ::= $Empty
		lhs.quad = mid_code.nextquad;
		break;
	case 4: //<������> ::= <����>
		break;
	case 5: //<������> ::= <������> <����>
		break;
	case 6: //<����> ::= $Int $ID <��������>
		p_symbolTable->insert_variable({symbolType::Int, syntaxTree[rhs[1]].val, proc_symbolTable::newtemp(), p_symbolTable->get_offset()});
		break;
	case 7: //<����> ::= $Void $ID <M> <A> <��������>
		beforeSymbolTable->init_function(syntaxTree[rhs[4]].plist, syntaxTree[rhs[1]].val, symbolType::Void, syntaxTree[rhs[2]].quad);
		break;
	case 8: //<����> ::= $Int $ID <M> <A> <��������>
		beforeSymbolTable->init_function(syntaxTree[rhs[4]].plist, syntaxTree[rhs[1]].val, symbolType::Int, syntaxTree[rhs[2]].quad);
		break;
	case 9: //<��������> ::=  $Semi
		break;
	case 10: //<��������> ::= $LeftBracket <�β�> $RightBracket <����>
		lhs.plist = syntaxTree[rhs[1]].plist;
		break;
	case 11: //<�β�> ::= <�����б�>
		lhs.plist = syntaxTree[rhs[0]].plist;
		break;
	case 12: //<�β�> ::= $Void
		lhs.plist = vector<symbolTableItem>();
		break;
	case 13: //<�����б�> ::= <����>
		lhs.plist.push_back(syntaxTree[rhs[0]].plist[0]);
		p_symbolTable->insert_variable({syntaxTree[rhs[0]].plist[0].type, syntaxTree[rhs[0]].plist[0].name, proc_symbolTable::newtemp(), p_symbolTable->get_offset()});
		break;
	case 14: //<�����б�> ::= <����> $Comma <�����б�>
		lhs.plist.push_back(syntaxTree[rhs[0]].plist[0]);
		p_symbolTable->insert_variable({syntaxTree[rhs[0]].plist[0].type, syntaxTree[rhs[0]].plist[0].name, proc_symbolTable::newtemp(), p_symbolTable->get_offset()});
		lhs.plist.insert(lhs.plist.end(), syntaxTree[rhs[2]].plist.begin(), syntaxTree[rhs[2]].plist.end());
		break;
	case 15: //<����> ::= $Int $ID
		lhs.plist.push_back({symbolType::Int, syntaxTree[rhs[1]].val, proc_symbolTable::newtemp(), 0});
		break;
	case 16: //<����> ::= $LeftBrace <�ڲ�����> <��䴮> $RightBrace
		lhs.nextlist = syntaxTree[rhs[2]].nextlist;
		beforeSymbolTable = p_symbolTable;
		p_symbolTable = p_symbolTable->return_block(mid_code.nextquad);
		break;
	case 17: //<�ڲ�����> ::= $Empty
		break;
	case 18: //<�ڲ�����> ::= <�ڲ���������> <�ڲ�����>
		break;
	case 19: //<�ڲ���������> ::= $Int $ID $Semi
		p_symbolTable->insert_variable({symbolType::Int, syntaxTree[rhs[1]].val, proc_symbolTable::newtemp(), p_symbolTable->get_offset()});
		break;
	case 20: //<��䴮> ::= <���>
		lhs.nextlist = syntaxTree[rhs[0]].nextlist;
		break;
	case 21: //<��䴮> ::= <���> <M> <��䴮>
		lhs.nextlist = syntaxTree[rhs[2]].nextlist;
		mid_code.back_patch(syntaxTree[rhs[0]].nextlist, syntaxTree[rhs[1]].quad);
		break;
	case 22: //<���> ::= <if���>
		lhs.nextlist = syntaxTree[rhs[0]].nextlist;
		break;
	case 23: //<���> :: = <while���>
		lhs.nextlist = syntaxTree[rhs[0]].nextlist;
		break;
	case 24: //<���> ::= <return���>
		break;
	case 25: //<���> ::= <assign���>
		break;
	case 26: //<assign���> ::= $ID $Equal <���ʽ> $Semi
		if (p_symbolTable->find_variable(syntaxTree[rhs[0]].val).type == symbolType::None)
			cout << syntaxTree[rhs[0]].val << "���ڷ��ű���" << endl;
		else
			mid_code.emit_code(quadruple(Oper::Assign, syntaxTree[rhs[2]].place, string(""), p_symbolTable->find_variable(syntaxTree[rhs[0]].val).gobalname));
		break;
	case 27: //<return���> ::= $Return $Semi
		mid_code.emit_code(quadruple(Oper::Return, string(""), string(""), string("")));
		break;
	case 28: //<return���> ::= $Return <���ʽ> $Semi
		mid_code.emit_code(quadruple(Oper::Return, syntaxTree[rhs[1]].place, string(""), string("ReturnValue")));
		break;
	case 29: //<while���> ::= $While <M> $LeftBracket <���ʽ> $RightBracket <A> <����>
		mid_code.back_patch(syntaxTree[rhs[6]].nextlist, syntaxTree[rhs[1]].quad);
		mid_code.back_patch(syntaxTree[rhs[3]].truelist, syntaxTree[rhs[5]].quad);
		lhs.nextlist = syntaxTree[rhs[3]].falselist;
		mid_code.emit_code(quadruple(Oper::J, string(""), string(""), to_string(syntaxTree[rhs[1]].quad)));
		break;
	case 30: //<if���> ::= $If $LeftBracket <���ʽ> $RightBracket <A> <����>
		mid_code.back_patch(syntaxTree[rhs[2]].truelist, syntaxTree[rhs[4]].quad);
		lhs.nextlist = mergelist(syntaxTree[rhs[2]].falselist, syntaxTree[rhs[4]].nextlist);
		break;
	case 31: //<if���> ::= $If $LeftBracket <���ʽ> $RightBracket <A> <����> <N> $Else <M> <A> <����>
		mid_code.back_patch(syntaxTree[rhs[2]].truelist, syntaxTree[rhs[4]].quad);
		mid_code.back_patch(syntaxTree[rhs[2]].falselist, syntaxTree[rhs[8]].quad);
		lhs.nextlist = mergelist(syntaxTree[rhs[5]].nextlist, syntaxTree[rhs[6]].nextlist, syntaxTree[rhs[10]].nextlist);
		break;
	case 32: //<���ʽ> ::= <�ӷ����ʽ>
		lhs.place = syntaxTree[rhs[0]].place;
		break;
	case 33: //<���ʽ> ::= <���ʽ> <�Ƚ������> <�ӷ����ʽ>
		lhs.truelist.push_back(mid_code.nextquad);
		lhs.falselist.push_back(mid_code.nextquad + 1);
		if (syntaxTree[rhs[1]].place == "$Smaller")
			mid_code.emit_code(quadruple(Oper::Jlt, syntaxTree[rhs[0]].place, syntaxTree[rhs[2]].place, string("0")));
		else if (syntaxTree[rhs[1]].place == "$SmallerEqual")
			mid_code.emit_code(quadruple(Oper::Jle, syntaxTree[rhs[0]].place, syntaxTree[rhs[2]].place, string("0")));
		else if (syntaxTree[rhs[1]].place == "$Bigger")
			mid_code.emit_code(quadruple(Oper::Jgt, syntaxTree[rhs[0]].place, syntaxTree[rhs[2]].place, string("0")));
		else if (syntaxTree[rhs[1]].place == "$BiggerEqual")
			mid_code.emit_code(quadruple(Oper::Jge, syntaxTree[rhs[0]].place, syntaxTree[rhs[2]].place, string("0")));
		else if (syntaxTree[rhs[1]].place == "$Equal2")
			mid_code.emit_code(quadruple(Oper::Jeq, syntaxTree[rhs[0]].place, syntaxTree[rhs[2]].place, string("0")));
		else if (syntaxTree[rhs[1]].place == "$NotEqual")
			mid_code.emit_code(quadruple(Oper::Jne, syntaxTree[rhs[0]].place, syntaxTree[rhs[2]].place, string("0")));
		else
			cout << "wrong comparation" << endl;

		mid_code.emit_code(quadruple(Oper::J, string(""), string(""), string("0")));
		break;
	case 34: //<�Ƚ������> ::= $Smaller
		lhs.place = "$Smaller";
		break;
	case 35: //<�Ƚ������> ::= $SmallerEqual
		lhs.place = "$SmallerEqual";
		break;
	case 36: //<�Ƚ������> ::= $Bigger
		lhs.place = "$Bigger";
		break;
	case 37: //<�Ƚ������> ::= $BiggerEqual
		lhs.place = "$BiggerEqual";
		break;
	case 38: //<�Ƚ������> ::= $Equal2
		lhs.place = "$Equal2";
		break;
	case 39: //<�Ƚ������> ::= $NotEqual
		lhs.place = "$NotEqual";
		break;
	case 40: //<�ӷ����ʽ> ::= <��>
		lhs.place = syntaxTree[rhs[0]].place;
		break;
	case 41: //<�ӷ����ʽ> ::= <��> $Plus <�ӷ����ʽ>
		lhs.place = proc_symbolTable::newtemp();
		mid_code.emit_code(quadruple(Oper::Plus, syntaxTree[rhs[0]].place, syntaxTree[rhs[2]].place, lhs.place));
		break;
	case 42: //<�ӷ����ʽ> ::= <��> $Minus <�ӷ����ʽ>
		lhs.place = proc_symbolTable::newtemp();
		mid_code.emit_code(quadruple(Oper::Minus, syntaxTree[rhs[0]].place, syntaxTree[rhs[2]].place, lhs.place));
		break;
	case 43: //<��> ::= <����>
		lhs.place = syntaxTree[rhs[0]].place;
		break;
	case 44: //<��> ::= <����> $Multiply <��>
		lhs.place = proc_symbolTable::newtemp();
		mid_code.emit_code(quadruple(Oper::Multiply, syntaxTree[rhs[0]].place, syntaxTree[rhs[2]].place, lhs.place));
		break;
	case 45: //<��> ::= <����> $Divide <��>
		lhs.place = proc_symbolTable::newtemp();
		mid_code.emit_code(quadruple(Oper::Divide, syntaxTree[rhs[0]].place, syntaxTree[rhs[2]].place, lhs.place));
		break;
	case 46: //<����> ::= $Number
		lhs.place = syntaxTree[rhs[0]].val;
		break;
	case 47: //<����> ::= $LeftBracket <���ʽ> $RightBracket
		lhs.place = syntaxTree[rhs[1]].place;
		break;
	case 48: //<����> ::= $ID $LeftBracket <ʵ���б�> $RightBracket
		functmp = p_symbolTable->find_function(syntaxTree[rhs[0]].val);
		if (functmp == NULL)
			cout << syntaxTree[rhs[0]].val << "���ں�������" << endl;
		else if (functmp->parm.size() != syntaxTree[rhs[2]].plist.size())
			cout << syntaxTree[rhs[0]].val << "����ʵ���б���" << endl;
		else
		{
			for (const auto &i : syntaxTree[rhs[2]].plist)
			{
				mid_code.emit_code(quadruple(Oper::Parm, i.gobalname, string(""), syntaxTree[rhs[0]].val));
			}
			if (functmp->return_type == symbolType::Int)
			{
				lhs.place = p_symbolTable->newtemp();
				mid_code.emit_code(quadruple(Oper::Call, syntaxTree[rhs[0]].val, string(""), lhs.place));
			}
			else
			{
				mid_code.emit_code(quadruple(Oper::Call, syntaxTree[rhs[0]].val, string(""), string("")));
			}
		}
		break;
	case 49: //<����> ::= $ID
		if (p_symbolTable->find_variable(syntaxTree[rhs[0]].val).type == symbolType::None)
			cout << syntaxTree[rhs[0]].val << "���ڷ��ű���" << endl;
		else
			lhs.place = p_symbolTable->find_variable(syntaxTree[rhs[0]].val).gobalname;
		break;
	case 50: //<ʵ���б�> ::= $Empty
		break;
	case 51: //<ʵ���б�> ::= <���ʽ>

		lhs.plist.push_back({symbolType::Unknown, syntaxTree[rhs[0]].place, syntaxTree[rhs[0]].place, 0});
		break;
	case 52: //<ʵ���б�> ::= <���ʽ> $Comma <ʵ���б�>
		lhs.plist.push_back({symbolType::Unknown, syntaxTree[rhs[0]].place, syntaxTree[rhs[0]].place, 0});
		lhs.plist.insert(lhs.plist.end(), syntaxTree[rhs[2]].plist.begin(), syntaxTree[rhs[2]].plist.end());
		break;
	case 53: //<����> ::= <����>
		{
			symbolTableItem variable = p_symbolTable->find_variable(syntaxTree[rhs[0]].place);
			if(variable.array.size() != syntaxTree[rhs[0]].array_vec.size())
			{
				cout << syntaxTree[rhs[0]].place << "����ά�Ȳ���Ӧ"  << variable.array.size() << syntaxTree[rhs[0]].array_vec.size() << endl;
				break;
			}
			string this_place, before_place;
			int len = syntaxTree[rhs[0]].array_vec.size();
			for(int i = 0 ; i < len; i++)
			{
				this_place = proc_symbolTable::newtemp();
				if (i == 0)
				{
					mid_code.emit_code(quadruple(Oper::Plus, syntaxTree[rhs[0]].array_vec[i], to_string(0), this_place));
				}
				else
				{
					mid_code.emit_code(quadruple(Oper::Multiply, before_place, to_string(variable.array[i]), this_place));
					mid_code.emit_code(quadruple(Oper::Plus, this_place, syntaxTree[rhs[0]].array_vec[i], this_place));
				}
				before_place = this_place;
			}
			lhs.place = proc_symbolTable::newtemp();
			mid_code.emit_code(quadruple(Oper::AssignArray, variable.gobalname, this_place, lhs.place));
			break;
		}
	case 54: //<����> ::= $ID $LeftArray <���ʽ> $RightArray
		if (p_symbolTable->find_variable(syntaxTree[rhs[0]].val).type == symbolType::None)
			cout << syntaxTree[rhs[0]].val << "���ڷ��ű���" << endl;
		else
		{
			symbolTableItem variable = p_symbolTable->find_variable(syntaxTree[rhs[0]].val);
			if( variable.type != symbolType::Array )
				cout << syntaxTree[rhs[0]].val << "��������" << endl;
			else
			{
				lhs.place = syntaxTree[rhs[0]].val;
				lhs.array_vec.push_back(syntaxTree[rhs[2]].place);
			}
		}	
		break;
	case 55: //<����> ::= <����> $LeftArray <���ʽ> $RightArray
		lhs.place = syntaxTree[rhs[0]].place;
		lhs.array_vec = syntaxTree[rhs[0]].array_vec;
		lhs.array_vec.push_back(syntaxTree[rhs[2]].place);
		break;
	case 56: //<assign���> ::= <����> $Equal <���ʽ> $Semi
		{
			symbolTableItem variable = p_symbolTable->find_variable(syntaxTree[rhs[0]].place);
			if(variable.array.size() != syntaxTree[rhs[0]].array_vec.size())
			{
				cout << syntaxTree[rhs[0]].place << "����ά�Ȳ���Ӧ" << endl;
				break;
			}
			string this_place, before_place;
			int len = syntaxTree[rhs[0]].array_vec.size();
			for(int i = 0 ; i < len; i++)
			{
				this_place = proc_symbolTable::newtemp();
				if (i == 0)
				{
					mid_code.emit_code(quadruple(Oper::Plus, syntaxTree[rhs[0]].array_vec[i], to_string(0), this_place));
				}
				else
				{
					mid_code.emit_code(quadruple(Oper::Multiply, before_place, to_string(variable.array[i]), this_place));
					mid_code.emit_code(quadruple(Oper::Plus, this_place, syntaxTree[rhs[0]].array_vec[i], this_place));
				}
				before_place = this_place;
			}
			lhs.place = proc_symbolTable::newtemp();
			mid_code.emit_code(quadruple(Oper::ArrayAssign, syntaxTree[rhs[2]].place, variable.gobalname,this_place ));
			break;
		}
		break;
	case 57: //<����> ::= $Int $ID <��������> $Semi
		{
			vector<int> vec;
			for(const auto &i : syntaxTree[rhs[2]].array_vec)
				vec.push_back(stoi(i));
			p_symbolTable->insert_variable({symbolType::Array, syntaxTree[rhs[1]].val, proc_symbolTable::newtemp(), p_symbolTable->get_offset(), vec});
			break;
		}
	case 58: //<��������> ::= $LeftArray $Number $RightArray
		lhs.array_vec.push_back(syntaxTree[rhs[1]].val);
		break;
	case 59: //<��������> ::= <��������> $LeftArray $Number $RightArray
		lhs.array_vec = syntaxTree[rhs[0]].array_vec;
		lhs.array_vec.push_back(syntaxTree[rhs[2]].val);
		break;
	case 60: //<�ڲ���������> ::= $Int $ID <��������> $Semi
		{
			vector<int> vec;
			for(const auto &i : syntaxTree[rhs[2]].array_vec)
				vec.push_back(stoi(i));
			p_symbolTable->insert_variable({symbolType::Array, syntaxTree[rhs[1]].val, proc_symbolTable::newtemp(), p_symbolTable->get_offset(), vec});
			break;
		}
	default:
		break;
	}
	return;
}

void parsing::outputStruction(ofstream &struction, syntaxTreeNodeIndex Node, int pad)
{
	this->syntaxTree[Node].inTree = true;
	struction << string(pad, '	') << "{" << endl;
	struction << string(pad + 1, '	') << "\"kind\": \"" << this->symbolTable[this->syntaxTree[Node].type] << "\"";
	if (!this->syntaxTree[Node].val.empty() || this->syntaxTree[Node].productions != -1 || !this->syntaxTree[Node].children.empty())
		struction << ",";
	struction << endl;

	if (!this->syntaxTree[Node].val.empty())
	{
		struction << string(pad + 1, '	') << "\"val\": \"" << this->syntaxTree[Node].val << "\"";
		if (this->syntaxTree[Node].productions != -1 || !this->syntaxTree[Node].children.empty())
			struction << ",";
		struction << endl;
	}

	if (this->syntaxTree[Node].productions != -1)
	{
		struction << string(pad + 1, '	') << "\"type\": \"";
		for (auto rhs : this->syntaxTable[this->syntaxTree[Node].productions].rhs)
			struction << this->symbolTable[rhs] << " ";
		struction << "\"";
		if (!this->syntaxTree[Node].children.empty())
			struction << ",";
		struction << endl;
	}

	if (!this->syntaxTree[Node].children.empty())
	{
		struction << string(pad + 1, '	') << "\"inner\": [" << endl;
		for (int i = 0; i < this->syntaxTree[Node].children.size(); i++)
		{
			this->outputStruction(struction, syntaxTree[Node].children[i], pad + 2);
			if (i != this->syntaxTree[Node].children.size() - 1)
				struction << "," << endl;
		}
		struction << endl
				  << string(pad + 1, '	') << "]" << endl;
	}
	struction << string(pad, '	') << "}";
}

void parsing::outputDot(ofstream &graph, syntaxTreeNodeIndex Node)
{
	if (!this->syntaxTree[Node].children.empty())
	{
		for (auto child : this->syntaxTree[Node].children)
		{
			graph << "	"
				  << "Node" << Node << "->"
				  << "Node" << child << endl;
		}

		if (this->syntaxTree[Node].children.size() > 1)
		{
			graph << "	"
				  << "{" << endl
				  << "		"
				  << "rank = same;" << endl
				  << "		";
			for (int i = 0; i < this->syntaxTree[Node].children.size(); i++)
			{
				graph
					<< "Node" << this->syntaxTree[Node].children[i];
				if (i < this->syntaxTree[Node].children.size() - 1)
					graph << "->";
			}
			graph << "[color=white];" << endl
				  << "		"
				  << "rankdir=LR;" << endl
				  << "	"
				  << "}" << endl;
		}

		for (auto child : this->syntaxTree[Node].children)
		{
			this->outputDot(graph, child);
		}
	}
}

void parsing::output(ofstream &struction, ofstream &graph)
{
	this->outputStruction(struction, this->topNode, 0);
	graph << "#@startdot" << endl
		  << endl;
	graph << "digraph demo {" << endl
		  << "node [fontname=\"Fangsong\" shape=plaintext]" << endl
		  << endl;

	for (int i = 0; i < syntaxTree.size(); i++)
		if (this->syntaxTree[i].inTree)
			graph << "	"
				  << "Node" << i << "[label=\"" << this->symbolTable[this->syntaxTree[i].type] << "\", shape=\"box\"]" << endl;

	graph << endl;
	this->outputDot(graph, this->topNode);

	graph << endl
		  << "}" << endl
		  << endl;
	graph << "#@enddot" << endl;
}

void parsing::outputMidcode(ofstream &midcode)
{
	this->mid_code.output(midcode);
}

vector<quadrupleIndex> parsing::mergelist(vector<quadrupleIndex> &list1, vector<quadrupleIndex> &list2)
{
	vector<quadrupleIndex> temp;
	temp.insert(temp.end(), list1.begin(), list1.end());
	temp.insert(temp.end(), list2.begin(), list2.end());
	return temp;
}

vector<quadrupleIndex> parsing::mergelist(vector<quadrupleIndex> &list1, vector<quadrupleIndex> &list2, vector<quadrupleIndex> &list3)
{
	vector<quadrupleIndex> temp;
	temp.insert(temp.end(), list1.begin(), list1.end());
	temp.insert(temp.end(), list2.begin(), list2.end());
	temp.insert(temp.end(), list3.begin(), list3.end());
	return temp;
}
