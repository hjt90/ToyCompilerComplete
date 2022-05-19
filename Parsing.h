#pragma once
#pragma execution_character_set("utf-8")
#include <vector>
#include <map>
#include <set>
#include <utility>
#include <stack>
#include <string>
#include <fstream>
#include "lexer.h"
#include "symbolTable.h"
#include "quadruple.h"
using namespace std;

class DFA_item;
typedef int symbolTableIndex;
typedef int syntaxTableIndex;
typedef int syntaxTreeNodeIndex;
typedef int DFA_statusIndex;
typedef int quadrupleIndex;
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
	syntaxTreeNodeIndex index;
	syntaxTreeNodeIndex parent;
	vector<syntaxTreeNodeIndex> children;
	syntaxTableIndex productions; //产生式
	symbolTableIndex type;
	string val;

	quadrupleIndex quad;
	symbolType stype;
	string place;
	vector<quadrupleIndex> truelist;
	vector<quadrupleIndex> falselist;
	vector<quadrupleIndex> nextlist;
	std::vector<symbolTableItem> plist;

	vector<string> array_vec;
	string array_place;

	bool inTree;

	syntaxTreeNode();
	syntaxTreeNode(const pair<Token, string>&);
};

class Optimizer;
//文法分析器
class parsing
{
	//符号表
	vector<symbolItem> symbolTable;
	symbolTableIndex terminalSymbolMax; //终结符的最后一个
	symbolTableIndex startIndex;		//开始符
	symbolTableIndex emptyIndex;		//空
	map<symbolItem, int> symbol2Index;
	//文法
	vector<syntaxTableItem> syntaxTable;
	vector<set<syntaxTableIndex>> searchSyntaxByLhs; //通过文法的左侧找产生式
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
	syntaxTreeNodeIndex topNode;
	//中间代码生成部分
	proc_symbolTable* p_symbolTable;
	IntermediateLanguage mid_code;

	void initSymbolTable(istream&);
	void initFirstTable();
	void initAnalyseTable();
	void initTerminalSymbol();
	void generate_midcode(syntaxTableIndex SyntaxIndex, syntaxTreeNode& lhs, vector<syntaxTreeNodeIndex>& rhs);
	symbolTableIndex insertSymbol(symbolItem);
	set<symbolTableIndex> firstForPhrase(vector<symbolTableIndex> p);
	pair<int, bool> createClosure(DFA_status& sta);
    void outputStruction(ostream&, syntaxTreeNodeIndex, int);
    void outputDot(ostream&, syntaxTreeNodeIndex);
	vector<quadrupleIndex> mergelist(vector<quadrupleIndex>& list1, vector<quadrupleIndex>& list2);
	vector<quadrupleIndex> mergelist(vector<quadrupleIndex>& list1, vector<quadrupleIndex>& list2, vector<quadrupleIndex>& list3);

public:
	parsing() = default;
	void clear();
	void initSyntax(istream&);
	void analyze(const vector<pair<Token, string>>&);
    void output(ostream& struction, ostream& graph);
    void outputMidcode(ostream& midcode);
	proc_symbolTable* get_proc_symbolTable()const;
	friend class Optimizer;
};
