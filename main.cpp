#include <iostream>
#include "lexer.h"
using namespace std;

int main()
{
	ifstream fin("test.c"); //读取文件的名字，可以相对或绝对
	if (!fin)
	{
		cout << "open fail." << endl;
		exit(1);
	}
	Lexer lex;
	lex.analyze(fin);
	lex.printresult();
	return 0;
}