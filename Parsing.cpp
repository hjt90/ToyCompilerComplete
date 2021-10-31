#include <sstream>
#include <string>
#include <iostream>
#include "Parsing.h"
using namespace std;

/*********
 * 插入到 symbolTable 中，同时更新 symbol2Index 表
 * 如果不存在就插入，存在则直接返回序号
 * ********/
symbolTableIndex parsing::insertSymbol(symbolItem insrt)
{
	if (!this->symbol2Index.count(insrt)) //map中不存在
	{
		symbolTable.push_back(insrt);
		symbol2Index[insrt] = symbolTable.size();
	}
	return symbol2Index[insrt];
}

/*********
 * 初始化 symbolTable、terminalSymbolMax、startIndex
 * 		symbol2Index、syntaxTable
 * ********/
void parsing::initSymbolTable(ifstream &infile)
{
	int line = 0;
	char templine[1024]; //存一行
	string tmpstr;
	while (infile.getline(templine, 1024))
	{
		line++;
		stringstream ss(templine);
		ss >> tmpstr;
		if (!((tmpstr[0] == '<' && tmpstr.back() == '>') || tmpstr[0] == '$'))
			cout << "语法分析器错误:"
				 << "语法输入第" << line << "行，左侧不是非终结符 " << tmpstr << endl;
		ss >> tmpstr;
		if (!(tmpstr != "::="))
			cout << "语法分析器错误:"
				 << "语法输入第" << line << "行，非找到赋值符(::=)，现为" << tmpstr << endl;
		while (ss >> tmpstr)
		{
		}
	}
}

/*********
 * 初始化 firstTable
 * ********/
void parsing::initFirstTable()
{
}

/*********
 * 初始化 DFA、analyseTable
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
