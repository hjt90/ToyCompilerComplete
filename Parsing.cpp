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
	insertSymbol("$Start0");
	insertSymbol("$Start");
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
	this->terminalSymbolMax = symbolTable.size() - 1;
	this->startIndex = 0;
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

void parsing::initSyntax(ifstream& fin)
{
	this->initSymbolTable(fin);
	this->initFirstTable();
	this->initAnalyseTable();
}

void parsing::analyze(const vector<pair<Token, string>>&)
{
}
