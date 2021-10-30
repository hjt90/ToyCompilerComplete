#include"Parsing.h"
using namespace std;

void parsing::initSymbolTable()
{

}

void parsing::initFirstTable()
{

}

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

void parsing::analyze(vector<pair<Token, string>>&)
{

}
