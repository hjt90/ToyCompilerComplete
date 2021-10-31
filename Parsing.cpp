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
		symbol2Index[insrt] = symbolTable.size();
	}
	return symbol2Index[insrt];
}

/*********
 * ��ʼ�� symbolTable��terminalSymbolMax��startIndex
 * 		symbol2Index��syntaxTable
 * ********/
void parsing::initSymbolTable(ifstream &infile)
{
	int line = 0;
	char templine[1024]; //��һ��
	string tmpstr;
	while (infile.getline(templine, 1024))
	{
		line++;
		stringstream ss(templine);
		ss >> tmpstr;
		if (!((tmpstr[0] == '<' && tmpstr.back() == '>') || tmpstr[0] == '$'))
			cout << "�﷨����������:"
				 << "�﷨�����" << line << "�У���಻�Ƿ��ս�� " << tmpstr << endl;
		ss >> tmpstr;
		if (!(tmpstr != "::="))
			cout << "�﷨����������:"
				 << "�﷨�����" << line << "�У����ҵ���ֵ��(::=)����Ϊ" << tmpstr << endl;
		while (ss >> tmpstr)
		{
		}
	}
}

/*********
 * ��ʼ�� firstTable
 * ********/
void parsing::initFirstTable()
{
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
}

void parsing::analyze(const vector<pair<Token, string>> &)
{
}
