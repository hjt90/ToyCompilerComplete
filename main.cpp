#include <iostream>
#include "lexer.h"
using namespace std;

int main()
{
	ifstream fin("test.c"); //��ȡ�ļ������֣�������Ի����
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