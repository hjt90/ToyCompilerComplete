#pragma once
#include <vector>
#include <map>
#include <set>
#include <utility>
#include <stack>
#include <string>
#include <fstream>
#include "lexer.h"
using namespace std;

class DFA_item;
typedef int symbolTableIndex;
typedef int syntaxTableIndex;
typedef int syntaxTreeNodeIndex;
typedef int DFA_statusIndex;
typedef std::set<symbolTableIndex> firstTableItem;
typedef std::pair<char, int> analyseTableItem;
typedef std::string symbolItem;
typedef std::set<DFA_item> DFA_status;

//项目
class DFA_item
{
public:
	symbolTableIndex lhs;
	vector<symbolTableIndex> rhs;
	int pos;
	symbolTableIndex forecast;
};

//文法
class syntaxTableItem
{
public:
	symbolTableIndex lhs;
	vector<symbolTableIndex> rhs;
};

//语法树节点
class syntaxTreeNode
{
public:
	syntaxTreeNodeIndex parent;
	vector<syntaxTreeNodeIndex> children;
	syntaxTableIndex productions; //产生式
	symbolTableIndex type;
	string val;
};

//文法分析器
class parsing
{
	//符号表
	vector<symbolItem> symbolTable;
	symbolTableIndex terminalSymbolMax; //终结符的最后一个
	symbolTableIndex startIndex;		//开始符
	map<symbolItem, int> symbol2Index;
	//文法
	vector<syntaxTableItem> syntaxTable;
	//first表
	vector<firstTableItem> firstTable;
	//项目集
	vector<DFA_status> DFA;
	//分析表
	vector<vector<analyseTableItem>> analyseTable;
	//分析过程
	vector<syntaxTreeNode> syntaxTree;			   //语法树节点
	stack<DFA_statusIndex> statusStack;			   //分析状态栈
	stack<syntaxTreeNodeIndex> analyseSymbolStack; //分析符号栈
	stack<syntaxTreeNodeIndex> inputSymbolvector;  //输入符号栈

	void initSymbolTable(ifstream &);
	void initFirstTable();
	void initAnalyseTable();
	symbolTableIndex insertSymbol(symbolItem);

public:
	void clear();
	void initSyntax(ifstream &);
	void analyze(const vector<pair<Token, string>> &);
};