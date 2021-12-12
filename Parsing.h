#pragma once
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

//��Ŀ
class DFA_item
{
public:
	symbolTableIndex lhs;
	vector<symbolTableIndex> rhs;
	int pos;
	symbolTableIndex forecast;
};

//�ķ�
class syntaxTableItem
{
public:
	symbolTableIndex lhs;
	vector<symbolTableIndex> rhs;
};

//�﷨���ڵ�
class syntaxTreeNode
{
public:
	syntaxTreeNodeIndex index;
	syntaxTreeNodeIndex parent;
	vector<syntaxTreeNodeIndex> children;
	syntaxTableIndex productions; //����ʽ
	symbolTableIndex type;
	string val;

	quadrupleIndex quad;
	symbolType stype;
	string place;
	vector<quadrupleIndex> truelist;
	vector<quadrupleIndex> falselist;
	vector<quadrupleIndex> nextlist;

	bool inTree;

	syntaxTreeNode();
	syntaxTreeNode(const pair<Token, string> &);
};

//�ķ�������
class parsing
{
	//���ű�
	vector<symbolItem> symbolTable;
	symbolTableIndex terminalSymbolMax; //�ս�������һ��
	symbolTableIndex startIndex;		//��ʼ��
	symbolTableIndex emptyIndex;		//��
	map<symbolItem, int> symbol2Index;
	//�ķ�
	vector<syntaxTableItem> syntaxTable;
	vector<set<syntaxTableIndex>> searchSyntaxByLhs; //ͨ���ķ�������Ҳ���ʽ
	//first��
	vector<firstTableItem> firstTable;
	//��Ŀ��
	vector<DFA_status> DFA;
	//������
	vector<vector<analyseTableItem>> analyseTable;
	//��������
	vector<syntaxTreeNode> syntaxTree;			   //�﷨���ڵ�
	stack<DFA_statusIndex> statusStack;			   //����״̬ջ
	stack<syntaxTreeNodeIndex> analyseSymbolStack; //��������ջ
	stack<syntaxTreeNodeIndex> inputSymbolvector;  //�������ջ
	syntaxTreeNodeIndex topNode;
	//�м�������ɲ���
	proc_symbolTable *p_symbolTable;
	IntermediateLanguage mid_code;

	void initSymbolTable(ifstream &);
	void initFirstTable();
	void initAnalyseTable();
	void initTerminalSymbol();
	void generate_midcode(syntaxTableIndex SyntaxIndex, syntaxTreeNode &lhs, vector<syntaxTreeNodeIndex> &rhs);
	symbolTableIndex insertSymbol(symbolItem);
	set<symbolTableIndex> firstForPhrase(vector<symbolTableIndex> p);
	pair<int, bool> createClosure(DFA_status &sta);
	void outputStruction(ofstream &, syntaxTreeNodeIndex, int);
	void outputDot(ofstream &, syntaxTreeNodeIndex);
	void debugdfa(); //��������dfa
	vector<quadrupleIndex> mergelist(vector<quadrupleIndex> &list1, vector<quadrupleIndex> &list2);
	vector<quadrupleIndex> mergelist(vector<quadrupleIndex> &list1, vector<quadrupleIndex> &list2, vector<quadrupleIndex> &list3);

public:
	parsing() = default;
	void clear();
	void initSyntax(ifstream &);
	void analyze(const vector<pair<Token, string>> &);
	void output(ofstream &struction, ofstream &graph);
};
