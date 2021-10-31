#include <sstream>
#include <string>
#include <iostream>
#include "Parsing.h"
using namespace std;

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
	insertSymbol("$End");
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

	this->syntaxTable.push_back({ this->startIndex,{this->symbol2Index["$Start"]} });	//���S' ==> S
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
			if (symbolTmp <= this->terminalSymbolMax)	//����ʽ�Ҳ�����Ϊ�ս��
				this->firstTable[i].insert(symbolTmp);
		}
	}
	//���ս��FIRST������
	bool inc = false;
	do
	{
		inc = false;
		for (int i = this->terminalSymbolMax + 1; i < this->symbolTable.size(); i++)	//����ÿһ�����ս����
		{
			int cntTmp = this->firstTable[i].size();
			for (auto syntaxIndexTmp : this->searchSyntaxByLhs[i])	//������Ϊ����Ĳ���ʽ
			{
				const vector<symbolTableIndex>& rhsTmp = this->syntaxTable[syntaxIndexTmp].rhs;
				for (int rhsIndex = 0; rhsIndex < rhsTmp.size(); rhsIndex++)	//����������
				{
					const firstTableItem& firstSymbolSet = this->firstTable[rhsTmp[rhsIndex]];
					if (rhsTmp[rhsIndex] <= this->terminalSymbolMax)	//Ϊ�ս��
					{
						this->firstTable[i].insert(rhsTmp[rhsIndex]);
						break;
					}

					bool haveEmpty = this->firstTable[i].count(this->emptyIndex);
					this->firstTable[i].insert(firstSymbolSet.cbegin(), firstSymbolSet.cend());
					if (!haveEmpty && firstSymbolSet.count(this->emptyIndex))
						this->firstTable[i].erase(this->emptyIndex);

					if (!firstSymbolSet.count(this->emptyIndex))	//������
						break;
					if (rhsIndex == rhsTmp.size() - 1)	//�������ʽ�Ҳ�ȫΪ�գ������
						this->firstTable[i].insert(this->emptyIndex);
				}
			}
			if (this->firstTable[i].size() > cntTmp)
				inc = true;
		}
	} while (inc);	//ֱ����������
}

/*********
 * ������ӵ�FIRST��
 * ********/
set<symbolTableIndex> parsing::firstForPhrase(vector<symbolTableIndex> rhsTmp)
{
	set<symbolTableIndex> res;

	for (int rhsIndex = 0; rhsIndex < rhsTmp.size(); rhsIndex++)	//����������
	{
		const firstTableItem& firstSymbolSet = this->firstTable[rhsTmp[rhsIndex]];
		if (rhsTmp[rhsIndex] <= this->terminalSymbolMax)	//Ϊ�ս��
		{
			res.insert(rhsTmp[rhsIndex]);
			break;
		}

		bool haveEmpty = res.count(this->emptyIndex);
		res.insert(firstSymbolSet.cbegin(), firstSymbolSet.cend());
		if (!haveEmpty && firstSymbolSet.count(this->emptyIndex))
			res.erase(this->emptyIndex);

		if (!firstSymbolSet.count(this->emptyIndex))	//������
			break;
		if (rhsIndex == rhsTmp.size() - 1)	//�������ʽ�Ҳ�ȫΪ�գ������
			res.insert(this->emptyIndex);
	}

	return res;
}

/*********
 * ��ʼ�� DFA��analyseTable
 * ********/
void parsing::initAnalyseTable()
{
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
}

void parsing::analyze(const vector<pair<Token, string>>&)
{
}
