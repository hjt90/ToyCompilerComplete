#include"Parsing.h"
using namespace std;

/*********
 * ��ʼ�� symbolTable��terminalSymbolMax��startIndex
 * 		symbol2Index��syntaxTable
 * ********/
void parsing::initSymbolTable()
{

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

void parsing::initSyntax(ifstream&)
{

}

void parsing::analyze(const vector<pair<Token, string>>&)
{

}
