#include <sstream>
#include <string>
#include <iostream>
#include <algorithm>
#include "Parsing.h"
using namespace std;

syntaxTreeNode::syntaxTreeNode(const pair<Token, string>& rhs) : index(0), parent(-1), productions(-1), type(0), inTree(false)
{
	this->type = int(rhs.first);
	this->val = rhs.second;
}

syntaxTreeNode::syntaxTreeNode() : index(0), parent(-1), productions(-1), type(0), inTree(false)
{
}

/*********
 * ���뵽 symbolTable �У�ͬʱ���� symbol2Index ��
 * ��������ھͲ��룬������ֱ�ӷ������
 * ********/
symbolTableIndex parsing::insertSymbol(symbolItem insrt)
{
	if (!this->symbol2Index.count(insrt)) //map�в�����
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
void parsing::initSymbolTable(ifstream& infile)
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

	this->syntaxTable.push_back({ this->startIndex, {this->symbol2Index["$Start"]} }); //���S' ==> S
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
			if (symbolTmp <= this->terminalSymbolMax) //����ʽ�Ҳ�����Ϊ�ս��
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
				const vector<symbolTableIndex>& rhsTmp = this->syntaxTable[syntaxIndexTmp].rhs;
				for (int rhsIndex = 0; rhsIndex < rhsTmp.size(); rhsIndex++) //����������
				{
					const firstTableItem& firstSymbolSet = this->firstTable[rhsTmp[rhsIndex]];
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
		const firstTableItem& firstSymbolSet = this->firstTable[rhsTmp[rhsIndex]];
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
bool operator<(const DFA_item& A, const DFA_item& B)
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
bool operator==(const DFA_item& A, const DFA_item& B)
{
	if (A.lhs == B.lhs && A.rhs == B.rhs && A.pos == B.pos && A.forecast == B.forecast)
		return true;
	else
		return false;
}

/*********
 * ����closure
 * ********/
pair<int, bool> parsing::createClosure(DFA_status& sta)
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
	DFA_status temps, temptopstatus;
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
	while (!si.empty())
	{
		statusno = si.top();
		temptopstatus = DFA[statusno];
		si.pop();
		transflag.clear();
		for (auto it = temptopstatus.begin(); it != temptopstatus.end(); it++) //�ҵ�������������п���ת�Ƶ��ַ�
		{
			if ((*it).pos < (*it).rhs.size())
			{
				transflag.insert((*it).rhs[(*it).pos]);
			}
		}
		for (auto it = transflag.begin(); it != transflag.end(); it++) //����ÿ��������ת�Ƶ��ַ������ƽ�״̬
		{
			temps.clear();
			for (auto it2 = temptopstatus.begin(); it2 != temptopstatus.end(); it2++) //����ÿһ�����
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
		for (auto it = temptopstatus.begin(); it != temptopstatus.end(); it++) //�ҹ�Լ״̬
		{
			if ((*it).pos >= (*it).rhs.size())
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

void parsing::initSyntax(ifstream& fin)
{
	this->initSymbolTable(fin);
	this->initFirstTable();
	this->initAnalyseTable();
	this->debugdfa();
}

void parsing::analyze(const vector<pair<Token, string>>& lexs)
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
			const syntaxTableItem& useSyntax = this->syntaxTable[nextAction.second];
			vector<syntaxTreeNodeIndex> rhsTmp;
			for (int i = 0; i < useSyntax.rhs.size(); i++)
			{
				syntaxTreeNodeIndex tmp = this->analyseSymbolStack.top();
				this->analyseSymbolStack.pop();
				this->statusStack.pop();
				rhsTmp.push_back(tmp);
				syntaxTree[tmp].parent = syntaxTree.size();
			}
			syntaxTreeNode lhsTmp;
			lhsTmp.children = rhsTmp;
			lhsTmp.productions = nextAction.second;
			lhsTmp.type = useSyntax.lhs;
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

void parsing::outputStruction(ofstream& struction, syntaxTreeNodeIndex Node, int pad)
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

void parsing::outputDot(ofstream& graph, syntaxTreeNodeIndex Node)
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
			graph << "	" << "{" << endl << "		" << "rank = same;" << endl << "		";
			for (int i = this->syntaxTree[Node].children.size() - 1; i >= 0 ; i--)
			{
				graph
					<< "Node" << this->syntaxTree[Node].children[i];
				if (i != 0)
					graph << "->";
			}
			graph << "[color=white];" << endl << "		" << "rankdir=LR;" << endl << "	" << "}" << endl;
		}

		for (auto child : this->syntaxTree[Node].children)
		{
			this->outputDot(graph, child);
		}
	}
}

void parsing::output(ofstream& struction, ofstream& graph)
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