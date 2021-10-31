#include <iostream>
#include "lexer.h"
#include "Parsing.h"
using namespace std;

int main()
{
	ifstream fin("D:\\workspace\\GitHub\\ToyCompiler\\test.c");
	ifstream finSyntax("D:\\workspace\\GitHub\\ToyCompiler\\part_parser.cc");
	if (!fin)
	{
		cout << "open fail." << endl;
		exit(1);
	}
	if (!finSyntax)
	{
		cout << "open fail." << endl;
		exit(1);
	}

	Lexer lex;
	parsing parser;

	lex.analyze(fin);
	//lex.printresult();

	parser.initSyntax(finSyntax);
	return 0;
}