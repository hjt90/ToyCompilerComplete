#include"Parsing.h"
using namespace std;

/*********
 * 初始化 symbolTable、terminalSymbolMax、startIndex
 * 		symbol2Index、syntaxTable
 * ********/
void parsing::initSymbolTable()
{

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

void parsing::initSyntax(ifstream&)
{

}

void parsing::analyze(const vector<pair<Token, string>>&)
{

}
