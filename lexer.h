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
	string translatetoken(Token i);        //翻译token以便输出
	void printresult();           //打印结果
	void analyze(ifstream& file); //分析
	const vector<pair<Token, string>>& output();
private:
	vector<string> keywordlist = { "int", "void", "if", "else", "while", "return" }; //保留字表（里面顺序必须严格和Token中顺序一样）
	vector<string> idtlist;                                                        //标识符表
	bool annflag = false;                                                          //控制注释开关，用来判断是否处于/**/内
	vector<pair<Token, string>> result;                                            //结果

	Ftype chtype(char ch);                               //查看字符是什么类型
	Status dealnumber(char* templine, int& cr, int len); //处理开头是数字的词
	Status dealalpha(char* templine, int& cr, int len);  //处理开头是字母的词
	Status dealsymbol(char* templine, int& cr, int len); //处理开头是符号的词
	void dealerror(int line, int cr);                    //输出错误
	Token search_keyword(string& s);                     //查询关键字表
	int search_identifier(string& s);                    //查标识符表
};

#endif