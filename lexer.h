#ifndef LEXER_H
#define LEXER_H
#include <vector>
#include <fstream>
#include <string>
#include "Token.h"
using namespace std;

enum class Ftype
{
	Alpha,
	Number,
	Symbol,
	Blank
};
enum class Status
{
	Success,
	Fail
};
class Lexer
{
public:
	Lexer() = default;
	string translatetoken(Token i);        //����token�Ա����
	void printresult();           //��ӡ���
	void analyze(ifstream& file); //����
	const vector<pair<Token, string>>& output();
private:
	vector<string> keywordlist = { "int", "void", "if", "else", "while", "return" }; //�����ֱ�����˳������ϸ��Token��˳��һ����
	vector<string> idtlist;                                                        //��ʶ����
	bool annflag = false;                                                          //����ע�Ϳ��أ������ж��Ƿ���/**/��
	vector<pair<Token, string>> result;                                            //���

	Ftype chtype(char ch);                               //�鿴�ַ���ʲô����
	Status dealnumber(char* templine, int& cr, int len); //����ͷ�����ֵĴ�
	Status dealalpha(char* templine, int& cr, int len);  //����ͷ����ĸ�Ĵ�
	Status dealsymbol(char* templine, int& cr, int len); //����ͷ�Ƿ��ŵĴ�
	void dealerror(int line, int cr);                    //�������
	Token search_keyword(string& s);                     //��ѯ�ؼ��ֱ�
	int search_identifier(string& s);                    //���ʶ����
};

#endif