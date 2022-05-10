#include <iostream>
#include <fstream>
#include "lexer.h"
#include "Parsing.h"
using namespace std;

int main(int argc, char** argv)
{
	int option_index = 0;
	ifstream fin;
	ifstream finSyntax;
	ofstream struction;
	ofstream graph;
	ofstream midcode;

	fin.open("D:/workspace/GitHub/ToyCompiler/test.c");
	//fin.setstate(std::ios_base::badbit);
	finSyntax.open("D:/workspace/GitHub/ToyCompiler/part_parse.cc");
	struction.open("D:/workspace/GitHub/ToyCompiler/test.json");
	graph.open("D:/workspace/GitHub/ToyCompiler/test.dot");
	midcode.open("D:/workspace/GitHub/ToyCompiler/test.csv");

	Lexer lex;
	parsing parser;

	lex.analyze(fin);
	//lex.printresult();

	parser.initSyntax(finSyntax);
	parser.analyze(lex.output());
	parser.output(struction, graph);
	parser.outputMidcode(midcode);

	struction.close();
	graph.close();
	midcode.close();

	return 0;
}