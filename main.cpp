#include <iostream>
#include <fstream>
#ifdef _MSC_VER
#include "getopt.h"
#else
#include <getopt.h>
#endif
#include "lexer.h"
#include "Parsing.h"
using namespace std;

void usage()
{
	cout << "usage: toycc --parser path_name --input pathname [--odot path_name] [--ojson path_name] [--omidcode pathname]" << endl;
}

int main(int argc, char** argv)
{
	int opt;
	int option_index = 0;
	ifstream fin;
	ifstream finSyntax;
	ofstream struction;
	ofstream graph;
	ofstream midcode;

	fin.setstate(std::ios_base::badbit);
	finSyntax.setstate(std::ios_base::badbit);
	struction.setstate(std::ios_base::badbit);
	graph.setstate(std::ios_base::badbit);
	midcode.setstate(std::ios_base::badbit);

	static struct option long_options[] =
	{
		{"odot", required_argument, NULL, 'p'},
		{"ojson", required_argument, NULL, 'd'},
		{"omidcode", required_argument, NULL, 'm'},
		{"parser", required_argument, NULL, 's'},
		{"input", required_argument, NULL, 'i'},
		{NULL, 0, NULL, 0} };

	while ((opt = getopt_long(argc, argv, "", long_options, &option_index)) != -1)
	{
		switch (opt)
		{
			case 'p':
				graph.clear();
				graph.open(optarg);
				if (!graph)
				{
					cout << "WARNING: open dot file fail" << endl;
				}
				break;
			case 'm':
				midcode.clear();
				midcode.open(optarg);
				if (!midcode)
				{
					cout << "WARNING: open midcode file fail" << endl;
				}
				break;
			case 'd':
				struction.clear();
				struction.open(optarg);
				if (!struction)
				{
					cout << "WARNING: open json file fail" << endl;
				}
				break;
			case 's':
				finSyntax.clear();
				finSyntax.open(optarg);
				if (!finSyntax)
				{
					cout << "WARNING: open syntax file fail" << endl;
				}
				break;
			case 'i':
				fin.clear();
				fin.open(optarg);
				if (!fin)
				{
					cout << "WARNING: open infile fail" << endl;
				}
				break;
			default: /* '?' */
				usage();
				exit(EXIT_FAILURE);
		}
	}

	if (!fin || !finSyntax)
	{
		usage();
		exit(EXIT_FAILURE);
	}

	//------------------------------------------------------------------------------------//

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