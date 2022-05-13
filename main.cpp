#include <iostream>
#include <fstream>
#include "lexer.h"
#include "Parsing.h"
#include "Optimizer.h"
using namespace std;

int main(int argc, char** argv)
{
	int option_index = 0;
	ifstream fin, finSyntax;
	ofstream struction, graph, midcode, midblock, optimizer_block;

	//fin.setstate(std::ios_base::badbit);
	fin.open("D:/workspace/GitHub/ToyCompiler/test.c");
	finSyntax.open("D:/workspace/GitHub/ToyCompiler/part_parse.cc");
	struction.open("D:/workspace/GitHub/ToyCompiler/test.json");
	graph.open("D:/workspace/GitHub/ToyCompiler/test.dot");
	midcode.open("D:/workspace/GitHub/ToyCompiler/test.csv");
	midblock.open("D:/workspace/GitHub/ToyCompiler/midblock.txt");
	optimizer_block.open("D:/workspace/GitHub/ToyCompiler/optimizer_block.txt");

	Lexer lex;
	parsing parser;
	Optimizer optimizer;

	lex.analyze(fin);
	//lex.printresult();

	parser.initSyntax(finSyntax);
	parser.analyze(lex.output());
	parser.output(struction, graph);
	parser.outputMidcode(midcode);

	optimizer.divideBlocks(parser);
	optimizer.outputBlocks(midblock);
	midcode.close();
	optimizer.optimizer();
	optimizer.outputBlocks(optimizer_block);

	fin.close();
	finSyntax.close();
	struction.close();
	graph.close();
	
	midblock.close();

	return 0;
}