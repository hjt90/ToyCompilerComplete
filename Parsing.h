#pragma once
#include<vector>
#include<map>
#include<set>
#include<utility>
#include<stack>
#include<string>
#include <fstream>
#include "lexer.h"
using namespace std;

class DFA_item;
typedef int symbolTableIndex;
typedef int syntacticTableIndex;
typedef int syntaxTreeNodeIndex;
typedef int DFA_statusIndex;
typedef std::set<symbolTableIndex> firstTableItem;
typedef std::pair<char, int> analyseTableItem;
typedef std::string symbolItem;
typedef std::vector<DFA_item> DFA_status;

//��Ŀ
class DFA_item
{
public:
	symbolTableIndex lhs;
	vector<symbolTableIndex> rhs;
	int pos;
	vector<symbolTableIndex> forecast;
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
	syntaxTreeNodeIndex parent;
	vector<syntaxTreeNodeIndex> children;
	syntacticTableIndex productions;//����ʽ
	symbolTableIndex type;
	string val;
};

//�ķ�������
class parsing
{
	//���ű�
	vector<symbolItem> symbolTable;
	symbolTableIndex terminalSymbolMax; //�ս�������һ��
	symbolTableIndex startIndex;	//��ʼ��
	map<symbolItem, int>symbol2Index;
	//�ķ�
	vector<syntaxTableItem> syntaxTable;
	//first��
	vector<firstTableItem> firstTable;
	//��Ŀ��
	vector<DFA_status> DFA;
	//������
	vector <vector<analyseTableItem>> analyseTable;
	//��������
	vector<syntaxTreeNode> syntaxTree;//�﷨���ڵ�
	stack<DFA_statusIndex> statusStack;//����״̬ջ
	stack<syntaxTreeNodeIndex>analyseSymbolStack;//��������ջ
	stack<syntaxTreeNodeIndex>inputSymbolvector;//�������ջ

	void initSymbolTable();
	void initFirstTable();
	void initAnalyseTable();
public:
	void clear();
	void initSyntax(ifstream&);
	void analyze(vector<pair<Token, string>>&);
};