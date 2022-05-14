#include <iostream>
#include <fstream>
#include "lexer.h"
#include "Parsing.h"
#include "Optimizer.h"
#include "objectCode.h"
using namespace std;

int main(int argc, char** argv)
{
	int option_index = 0;
	ifstream fin, finSyntax;
	ofstream struction, graph, midcode, midblock, optimizer_block, object_Iblocks, object_code;

	//fin.setstate(std::ios_base::badbit);
	fin.open("D:/workspace/GitHub/ToyCompiler/test.c");
	finSyntax.open("D:/workspace/GitHub/ToyCompiler/part_parse.cc");
	struction.open("D:/workspace/GitHub/ToyCompiler/test.json");
	graph.open("D:/workspace/GitHub/ToyCompiler/test.dot");
	midcode.open("D:/workspace/GitHub/ToyCompiler/test.csv");
	midblock.open("D:/workspace/GitHub/ToyCompiler/midblock.txt");
	optimizer_block.open("D:/workspace/GitHub/ToyCompiler/optimizer_block.txt");
	object_Iblocks.open("D:/workspace/GitHub/ToyCompiler/object_Iblocks.txt");
	object_code.open("D:/workspace/GitHub/ToyCompiler/object_code.txt");

	Lexer lex;
	parsing parser;
	Optimizer optimizer;
	ObjectCode objectcode;

	lex.analyze(fin);
	// lex.printresult();

	parser.initSyntax(finSyntax);
	parser.analyze(lex.output());
	parser.output(struction, graph);
	parser.outputMidcode(midcode);

	optimizer.divideBlocks(parser);
	optimizer.outputBlocks(midblock);
	optimizer.optimizer();
	optimizer.outputBlocks(optimizer_block);

	objectcode.analyseBlock(optimizer);
	objectcode.outputIBlocks(object_Iblocks);

	fin.close();
	finSyntax.close();
	struction.close();
	graph.close();
	midcode.close();
	midblock.close();
	object_Iblocks.close();
	object_code.close();

	return 0;
}