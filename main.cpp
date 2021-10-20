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
	vector<string> keywordlist = { "int","void","if","else","while","return" };//�����ֱ�����˳������ϸ��Token��˳��һ����
	vector<string> idtlist;//��ʶ����
	bool annflag = false;//����ע�Ϳ��أ������ж��Ƿ���/**/��
	vector<pair<Token, string>> result;//���

	void analyze(ifstream file);//����
	Ftype chtype(char ch);//�鿴�ַ���ʲô����
	Status dealnumber(char* templine, int& cr, int len);//����ͷ�����ֵĴ�
	Status dealalpha(char* templine, int& cr, int len);//����ͷ����ĸ�Ĵ�
	Status dealsymbol(char* templine, int& cr, int len);//����ͷ�Ƿ��ŵĴ�
	void dealerror(int line, int cr);//�������
	Token search_keyword(string& s);//��ѯ�ؼ��ֱ�
	int search_identifier(string& s);//���ʶ����
	void printresult();//��ӡ���
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
	cout << "�ڵ�" << line << "�е�" << cr << "���ַ����ִ���" << endl;
}

Token Lexer::search_keyword(string& s)
{
	for (int i = 0; i < this->keywordlist.size(); i++)
	{
		if (s == this->keywordlist[i])//����鵽�ǹؼ���
		{
			return Token::Int + i;//��������ؼ��ֵ�����
		}
	}
	return Token::Id;//�Ǳ�ʶ�����ǹؼ���
}

int Lexer::search_identifier(string& s)
{
	for (int i = 0; i < this->idtlist.size(); i++)
	{
		if (s == this->idtlist[i])//����鵽�Ǳ�ʶ��
		{
			return i;//���������ʶ��������
		}
	}
	//û�鵽������
	this->idtlist.push_back(s);
	return i;//���������ʶ��������
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
	while (cr < len && chtype(templine[cr]) == Ftype::Number)//һֱ������ַ���������������Ϊֹ
	{
		tempstr += templine[cr];
		cr++;
	}
	this->result.push_back(pair<Token::Number, tempstr>);//����һ�����ֵĽ������result
	return Status::Success;//�ɹ�
}

Status Lexer::dealalpha(char* templine, int& cr, int len)
{
	string tempstr = "";
	Ftype chtp;//Ϊ�˱����ظ�����chtype��������¼һ��
	while (cr < len)//һֱ������ַ��������������ֻ���ĸΪֹ
	{
		//Ϊ��Ч�ʴ���ԭ����whileѭ�����
		chtp = chtype(templine[cr]);
		if (chtp != Ftype::Alpha && chtp != Ftype::Number)
			break;
		//ѭ���ڵ�����
		tempstr += templine[cr];
		cr++;
	}
	Token tk;
	tk = search_keyword(tempstr);//���ǲ��ǹؼ���
	if (tk != Token::Id)//�ǹؼ���
		this->result.push_back(pair<tk, "">);
	else//�Ǳ�ʶ��
	{
		int index = search_identifier(tempstr);//����û�еǼǣ����û���Զ�����
		this->result.push_back(pair<Token::Id, to_string(index)>);//�����ҷ��ص�ֵ�������������������֣������ٸ�
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
	else if (templine[cr] == '/')//���ע�ʹ��������Ƚ��ر�
	{
		if (cr + 1 >= len)
			return Status::Fail;
		else if (templine[cr + 1] == '/')//���˫б��ע�ͣ�ֱ�Ӱ�ָ���Ƶ���һ����ĩ
		{
			cr = len;
			return Status::Success;
		}
		else if (templine[cr + 1] == '*')
		{
			this->annflag = true;//����ע�Ϳ���
			cr += 2;
			return Status::Success;
		}
		else
			return Status::Fail;
	}
	//�ر���Ҫע�⣬�������ﲻ�ж�*/������analyze�ﴦ��
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
	int line = 0;//��¼����һ��
	char templine[1024];//��һ��
	int cr;//i��ָ��ǰ�е��ַ���λ�õ�ָ�룬��templine[0]��ʼ
	int len;//len��¼templine��һ�еĳ���
	Status res;//����һЩ����״̬
	Ftype chtp;//��¼����
	while (file.getline(templine, 1024))
	{
		line++;
		len = strlen(templine);
		cr = 0;
		while (cr < len)
		{
			if (this->annflag == true)//��ǰ����/**/��ע����
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
			else//�������
			{
				chtp = chtype(templine[cr]);
				if (chtp == Ftype::Blank)//��ͷ��tab��ո�
					cr++;
				else if (chtp == Ftype::Number)//��ͷ������
					res = dealnumber(templine, cr, len);
				else if (chtp == Ftype::Alpha)//��ͷ����ĸ
					res = dealalpha(templine, cr, len);
				else if (chtp == Ftype::Symbol)//��ͷ�Ƿ���
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
