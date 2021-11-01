#include <iostream>
#include <fstream>
#include "lexer.h"
#include "Parsing.h"
using namespace std;

int main()
{
	ifstream fin("D:\\workspace\\GitHub\\ToyCompiler\\test.c");
	ifstream finSyntax("D:\\workspace\\GitHub\\ToyCompiler\\part_parser.cc");
	ofstream struction("D:\\workspace\\GitHub\\ToyCompiler\\test.out");
	ofstream graph("D:\\workspace\\GitHub\\ToyCompiler\\test.dot");
	if (!fin || !finSyntax || !struction || !graph)
	{
		cout << "open fail." << endl;
		exit(1);
	}

	Lexer lex;
	parsing parser;

	lex.analyze(fin);
	//lex.printresult();

	parser.initSyntax(finSyntax);
	parser.analyze(lex.output());
	parser.output(struction, graph);

	return 0;
}