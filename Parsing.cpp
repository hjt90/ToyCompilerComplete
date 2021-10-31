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
 * 初始化 symbolTable、terminalSymbolMax、startIndex
 * 		symbol2Index、syntaxTable
 * ********/
void parsing::initSymbolTable(ifstream& infile)
{
	initTerminalSymbol();
	int line = 0;
	char templine[1024]; //存一行
	string tmpstr;
	while (infile.getline(templine, 1024))
	{
		line++;
		syntaxTableItem tmpSyntax;
		stringstream ss(templine);
		if (ss >> tmpstr)
		{
			if (!((tmpstr[0] == '<' && tmpstr.back() == '>') || tmpstr[0] == '$'))
				cout << "语法分析器错误:"
				<< "语法输入第" << line << "行，左侧不是非终结符 " << tmpstr << endl;
			tmpSyntax.lhs = insertSymbol(tmpstr);

			ss >> tmpstr;
			if (tmpstr != "::=")
				cout << "语法分析器错误:"
				<< "语法输入第" << line << "行，未找到赋值符(::=)，现为" << tmpstr << endl;
			while (ss >> tmpstr)
			{
				tmpSyntax.rhs.push_back(insertSymbol(tmpstr));
			}
			this->syntaxTable.push_back(tmpSyntax);
		}
	}

	this->syntaxTable.push_back({ this->startIndex,{this->symbol2Index["$Start"]} });	//添加S' ==> S
	searchSyntaxByLhs = vector<set<syntaxTableIndex>>(symbolTable.size());
	for (int i = 0; i < syntaxTable.size(); i++)
	{
		searchSyntaxByLhs[syntaxTable[i].lhs].insert(i);
	}
}

/*********
 * 初始化 firstTable
 * ********/
void parsing::initFirstTable()
{
	this->firstTable = vector<firstTableItem>(symbolTable.size());
	//终结符 FIRST集为其自身
	for (int i = 0; i <= this->terminalSymbolMax; i++)
	{
		this->firstTable[i].insert(i);
	}
	//非终结符FIRST集加入产生式的左侧
	for (int i = this->terminalSymbolMax + 1; i < this->symbolTable.size(); i++)
	{
		for (auto syntaxIndexTmp : this->searchSyntaxByLhs[i])
		{
			symbolTableIndex symbolTmp = this->syntaxTable[syntaxIndexTmp].rhs[0];
			if (symbolTmp <= this->terminalSymbolMax)	//产生式右侧首项为终结符
				this->firstTable[i].insert(symbolTmp);
		}
	}
	//非终结符FIRST集互推
	bool inc = false;
	do
	{
		inc = false;
		for (int i = this->terminalSymbolMax + 1; i < this->symbolTable.size(); i++)	//对于每一个非终结符集
		{
			int cntTmp = this->firstTable[i].size();
			for (auto syntaxIndexTmp : this->searchSyntaxByLhs[i])	//对于其为左项的产生式
			{
				const vector<symbolTableIndex>& rhsTmp = this->syntaxTable[syntaxIndexTmp].rhs;
				for (int rhsIndex = 0; rhsIndex < rhsTmp.size(); rhsIndex++)	//遍历其右项
				{
					const firstTableItem& firstSymbolSet = this->firstTable[rhsTmp[rhsIndex]];
					if (rhsTmp[rhsIndex] <= this->terminalSymbolMax)	//为终结符
					{
						this->firstTable[i].insert(rhsTmp[rhsIndex]);
						break;
					}

					bool haveEmpty = this->firstTable[i].count(this->emptyIndex);
					this->firstTable[i].insert(firstSymbolSet.cbegin(), firstSymbolSet.cend());
					if (!haveEmpty && firstSymbolSet.count(this->emptyIndex))
						this->firstTable[i].erase(this->emptyIndex);

					if (!firstSymbolSet.count(this->emptyIndex))	//不含空
						break;
					if (rhsIndex == rhsTmp.size() - 1)	//如果产生式右侧全为空，加入空
						this->firstTable[i].insert(this->emptyIndex);
				}
			}
			if (this->firstTable[i].size() > cntTmp)
				inc = true;
		}
	} while (inc);	//直到不再增长
}

/*********
 * 计算句子的FIRST集
 * ********/
set<symbolTableIndex> parsing::firstForPhrase(vector<symbolTableIndex> rhsTmp)
{
	set<symbolTableIndex> res;

	for (int rhsIndex = 0; rhsIndex < rhsTmp.size(); rhsIndex++)	//遍历其右项
	{
		const firstTableItem& firstSymbolSet = this->firstTable[rhsTmp[rhsIndex]];
		if (rhsTmp[rhsIndex] <= this->terminalSymbolMax)	//为终结符
		{
			res.insert(rhsTmp[rhsIndex]);
			break;
		}

		bool haveEmpty = res.count(this->emptyIndex);
		res.insert(firstSymbolSet.cbegin(), firstSymbolSet.cend());
		if (!haveEmpty && firstSymbolSet.count(this->emptyIndex))
			res.erase(this->emptyIndex);

		if (!firstSymbolSet.count(this->emptyIndex))	//不含空
			break;
		if (rhsIndex == rhsTmp.size() - 1)	//如果产生式右侧全为空，加入空
			res.insert(this->emptyIndex);
	}

	return res;
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
