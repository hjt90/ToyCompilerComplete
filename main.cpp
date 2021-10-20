#include<iostream>
#include<vector>
#include<fstream>
using namespace std;

enum class Token
{
	Id,
	Int, Void, If, Else, While, Return,
	Plus, Minus, Multiply, Divide, Equal, Equal2, Bigger, BiggerEqual, Smaller, SmallerEqual, NotEqual,
	Semi, Comma, LeftAnno, RightAnno, Anno, LeftBracket, RightBracket, LeftBrace, RightBrace,
	Number,
	End,
	Unknown
};
enum class Ftype
{
	Alpha, Number, Symbol, Blank
};
enum class Status
{
	Success, Fail
};
class Lexer
{
private:
	vector<string> keywordlist = { "int","void","if","else","while","return" };//保留字表（里面顺序必须严格和Token中顺序一样）
	vector<string> idtlist;//标识符表
	bool annflag = false;//控制注释开关，用来判断是否处于/**/内
	vector<pair<Token, string>> result;//结果

	void analyze(ifstream file);//分析
	Ftype chtype(char ch);//查看字符是什么类型
	Status dealnumber(char* templine, int& cr, int len);//处理开头是数字的词
	Status dealalpha(char* templine, int& cr, int len);//处理开头是字母的词
	Status dealsymbol(char* templine, int& cr, int len);//处理开头是符号的词
	void dealerror(int line, int cr);//输出错误
	Token search_keyword(string& s);//查询关键字表
	int search_identifier(string& s);//查标识符表
	void printresult();//打印结果
};

void Lexer::printresult()
{
	int len = this->result.size();
	for (int i = 0; i < len; i++)
	{
		cout << this->result[i].first << ' ' << this->result[i].second << endl;
	}
}

void Lexer::dealerror(int line, int cr)
{
	cout << "在第" << line << "行第" << cr << "个字符发现错误" << endl;
}

Token Lexer::search_keyword(string& s)
{
	for (int i = 0; i < this->keywordlist.size(); i++)
	{
		if (s == this->keywordlist[i])//如果查到是关键字
		{
			return Token::Int + i;//返回这个关键字的类别号
		}
	}
	return Token::Id;//是标识符不是关键字
}

int Lexer::search_identifier(string& s)
{
	for (int i = 0; i < this->idtlist.size(); i++)
	{
		if (s == this->idtlist[i])//如果查到是标识符
		{
			return i;//返回这个标识符的索引
		}
	}
	//没查到，新增
	this->idtlist.push_back(s);
	return i;//返回这个标识符的索引
}

Ftype Lexer::chtype(char ch)
{
	if (ch == ' ' || ch == '\t')
		return Ftype::Blank;
	if (ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z')
		return Ftype::Alpha;
	else if (ch >= '0' && ch <= '9')
		return Ftype::Number;
	else
		return Ftype::Symbol;
}

Status Lexer::dealnumber(char* templine, int& cr, int len)
{
	string tempstr = "";
	while (cr < len && chtype(templine[cr]) == Ftype::Number)//一直往后读字符，读到不是数字为止
	{
		tempstr += templine[cr];
		cr++;
	}
	this->result.push_back(pair<Token::Number, tempstr>);//把这一个数字的结果加入result
	return Status::Success;//成功
}

Status Lexer::dealalpha(char* templine, int& cr, int len)
{
	string tempstr = "";
	Ftype chtp;//为了避免重复调用chtype函数，记录一下
	while (cr < len)//一直往后读字符，读到不是数字或字母为止
	{
		//为了效率代替原来的while循环语句
		chtp = chtype(templine[cr]);
		if (chtp != Ftype::Alpha && chtp != Ftype::Number)
			break;
		//循环内的内容
		tempstr += templine[cr];
		cr++;
	}
	Token tk;
	tk = search_keyword(tempstr);//查是不是关键字
	if (tk != Token::Id)//是关键字
		this->result.push_back(pair<tk, "">);
	else//是标识符
	{
		int index = search_identifier(tempstr);//查有没有登记，如果没有自动创建
		this->result.push_back(pair<Token::Id, to_string(index)>);//这里我返回的值是它的索引而不是名字，可以再改
	}
	return Status::Success;
}

Status Lexer::dealsymbol(char* templine, int& cr, int len)
{
	if (templine[cr] == '+')
	{
		this->result.push_back(pair<Token::Plus, "">);
		cr++;
		return Status::Success;
	}
	else if (templine[cr] == '-')
	{
		this->result.push_back(pair<Token::Minus, "">);
		cr++;
		return Status::Success;
	}
	else if (templine[cr] == '*')
	{
		this->result.push_back(pair<Token::Multiply, "">);
		cr++;
		return Status::Success;
	}
	else if (templine[cr] == '/')
	{
		this->result.push_back(pair<Token::Divide, "">);
		cr++;
		return Status::Success;
	}
	else if (templine[cr] == '=')
	{
		if (cr + 1 < len && chtype(templine[cr + 1]) == '=')
		{
			this->result.push_back(pair<Token::Equal2, "">);
			cr += 2;
			return Status::Success;
		}
		else
		{
			this->result.push_back(pair<Token::Equal, "">);
			cr++;
			return Status::Success;
		}
	}
	else if (templine[cr] == '>')
	{
		if (cr + 1 < len && chtype(templine[cr + 1]) == '=')
		{
			this->result.push_back(pair<Token::BiggerEqual, "">);
			cr += 2;
			return Status::Success;
		}
		else
		{
			this->result.push_back(pair<Token::Bigger, "">);
			cr++;
			return Status::Success;
		}
	}
	else if (templine[cr] == '<')
	{
		if (cr + 1 < len && chtype(templine[cr + 1]) == '=')
		{
			this->result.push_back(pair<Token::SmallerEqual, "">);
			cr += 2;
			return Status::Success;
		}
		else
		{
			this->result.push_back(pair<Token::Smaller, "">);
			cr++;
			return Status::Success;
		}
	}
	else if (templine[cr] == '!')
	{
		if (cr + 1 < len && chtype(templine[cr + 1]) == '=')
		{
			this->result.push_back(pair<Token::NotEqual, "">);
			cr += 2;
			return Status::Success;
		}
		else
			return Status::Fail;
	}
	else if (templine[cr] == ',')
	{
		this->result.push_back(pair<Token::Comma, "">);
		cr++;
		return Status::Success;
	}
	else if (templine[cr] == ';')
	{
		this->result.push_back(pair<Token::Semi, "">);
		cr++;
		return Status::Success;
	}
	else if (templine[cr] == '/')//这个注释处理起来比较特别
	{
		if (cr + 1 >= len)
			return Status::Fail;
		else if (templine[cr + 1] == '/')//变成双斜杠注释，直接把指针移到这一行最末
		{
			cr = len;
			return Status::Success;
		}
		else if (templine[cr + 1] == '*')
		{
			this->annflag = true;//开启注释开关
			cr += 2;
			return Status::Success;
		}
		else
			return Status::Fail;
	}
	//特别需要注意，我们这里不判断*/，留到analyze里处理
	else if (templine[cr] == '(')
	{
		this->result.push_back(pair<Token::LeftBracket, "">);
		cr++;
		return Status::Success;
	}
	else if (templine[cr] == ')')
	{
		this->result.push_back(pair<Token::RightBracket, "">);
		cr++;
		return Status::Success;
	}
	else if (templine[cr] == '{')
	{
		this->result.push_back(pair<Token::LeftBrace, "">);
		cr++;
		return Status::Success;
	}
	else if (templine[cr] == '}')
	{

		this->result.push_back(pair<Token::RightBrace, "">);
		cr++;
		return Status::Success;
	}
	else if (templine[cr] == '#')
	{
		this->result.push_back(pair<Token::End, "">);
		cr++;
		return Status::Success;
	}
	else
	{
		return Status::Fail;
	}
}

void Lexer::analyze(ifstream file)
{
	int line = 0;//记录是哪一行
	char templine[1024];//存一行
	int cr;//i是指向当前行的字符的位置的指针，从templine[0]开始
	int len;//len记录templine中一行的长度
	Status res;//接受一些返回状态
	Ftype chtp;//记录类型
	while (file.getline(templine, 1024))
	{
		line++;
		len = strlen(templine);
		cr = 0;
		while (cr < len)
		{
			if (this->annflag == true)//当前处在/**/的注释中
			{
				while (cr < len)
				{
					if (templine[cr] == '*' && cr + 1 < len && templine[cr + 1] == '/')
					{
						cr += 2;
						this->annflag == false;
						break;
					}
					cr++;
				}
			}
			else//正常情况
			{
				chtp = chtype(templine[cr]);
				if (chtp == Ftype::Blank)//开头是tab或空格
					cr++;
				else if (chtp == Ftype::Number)//开头是数字
					res = dealnumber(templine, cr, len);
				else if (chtp == Ftype::Alpha)//开头是字母
					res = dealalpha(templine, cr, len);
				else if (chtp == Ftype::Symbol)//开头是符号
				{
					res = dealsymbol(templine, cr, len);
					if (res == Status::Fail)
						dealerror(line, cr);
				}
				else
					dealerror(line, cr);
			}
		}
	}
}
