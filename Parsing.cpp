#include <sstream>
#include <string>
#include <iostream>
#include<algorithm>
#include "Parsing.h"
using namespace std;

syntaxTreeNode::syntaxTreeNode(const pair<Token, string>& rhs)
{
	this->type = int(rhs.first);
	this->val = rhs.second;
}

/*********
 * 插入到 symbolTable 中，同时更新 symbol2Index 表
 * 如果不存在就插入，存在则直接返回序号
 * ********/
symbolTableIndex parsing::insertSymbol(symbolItem insrt)
{
	if (!this->symbol2Index.count(insrt)) //map中不存在
	{
		symbolTable.push_back(insrt);
		symbol2Index[insrt] = symbolTable.size() - 1;
	}
	return symbol2Index[insrt];
}

void parsing::initTerminalSymbol()
{
	insertSymbol("$ID");
	insertSymbol("$Int");
	insertSymbol("$Void");
	insertSymbol("$If");
	insertSymbol("$Else");
	insertSymbol("$While");
	insertSymbol("$Return");
	insertSymbol("$Plus");
	insertSymbol("$Minus");
	insertSymbol("$Multiply");
	insertSymbol("$Divide");
	insertSymbol("$Equal");
	insertSymbol("$Equal2");
	insertSymbol("$Bigger");
	insertSymbol("$BiggerEqual");
	insertSymbol("$Smaller");
	insertSymbol("$SmallerEqual");
	insertSymbol("$NotEqual");
	insertSymbol("$Semi");
	insertSymbol("$Comma");
	insertSymbol("$LeftAnno");
	insertSymbol("$RightAnno");
	insertSymbol("$Anno");
	insertSymbol("$LeftBracket");
	insertSymbol("$RightBracket");
	insertSymbol("$LeftBrace");
	insertSymbol("$RightBrace");
	insertSymbol("$Number");
	insertSymbol("$End");
	insertSymbol("$Empty");
	insertSymbol("$Start0");
	insertSymbol("$Start");
	this->terminalSymbolMax = symbolTable.size() - 1 - 2;
	this->startIndex = this->symbol2Index["$Start0"];
	this->emptyIndex = this->symbol2Index["$Empty"];
}

/*********
 * 初始化 symbolTable、terminalSymbolMax、startIndex
 * 		symbol2Index、syntaxTable
 * ********/
void parsing::initSymbolTable(ifstream& infile)
{
	initTerminalSymbol();
	int line = 0;
	char templine[1024]; //存一行
	string tmpstr;
	while (infile.getline(templine, 1024))
	{
		line++;
		syntaxTableItem tmpSyntax;
		stringstream ss(templine);
		if (ss >> tmpstr)
		{
			if (!((tmpstr[0] == '<' && tmpstr.back() == '>') || tmpstr[0] == '$'))
				cout << "语法分析器错误:"
				<< "语法输入第" << line << "行，左侧不是非终结符 " << tmpstr << endl;
			tmpSyntax.lhs = insertSymbol(tmpstr);

			ss >> tmpstr;
			if (tmpstr != "::=")
				cout << "语法分析器错误:"
				<< "语法输入第" << line << "行，未找到赋值符(::=)，现为" << tmpstr << endl;
			while (ss >> tmpstr)
			{
				tmpSyntax.rhs.push_back(insertSymbol(tmpstr));
			}
			this->syntaxTable.push_back(tmpSyntax);
		}
	}

	this->syntaxTable.push_back({ this->startIndex,{this->symbol2Index["$Start"]} });	//添加S' ==> S
	searchSyntaxByLhs = vector<set<syntaxTableIndex>>(symbolTable.size());
	for (int i = 0; i < syntaxTable.size(); i++)
	{
		searchSyntaxByLhs[syntaxTable[i].lhs].insert(i);
	}
}

/*********
 * 初始化 firstTable
 * ********/
void parsing::initFirstTable()
{
	this->firstTable = vector<firstTableItem>(symbolTable.size());
	//终结符 FIRST集为其自身
	for (int i = 0; i <= this->terminalSymbolMax; i++)
	{
		this->firstTable[i].insert(i);
	}
	//非终结符FIRST集加入产生式的左侧
	for (int i = this->terminalSymbolMax + 1; i < this->symbolTable.size(); i++)
	{
		for (auto syntaxIndexTmp : this->searchSyntaxByLhs[i])
		{
			symbolTableIndex symbolTmp = this->syntaxTable[syntaxIndexTmp].rhs[0];
			if (symbolTmp <= this->terminalSymbolMax)	//产生式右侧首项为终结符
				this->firstTable[i].insert(symbolTmp);
		}
	}
	//非终结符FIRST集互推
	bool inc = false;
	do
	{
		inc = false;
		for (int i = this->terminalSymbolMax + 1; i < this->symbolTable.size(); i++)	//对于每一个非终结符集
		{
			int cntTmp = this->firstTable[i].size();
			for (auto syntaxIndexTmp : this->searchSyntaxByLhs[i])	//对于其为左项的产生式
			{
				const vector<symbolTableIndex>& rhsTmp = this->syntaxTable[syntaxIndexTmp].rhs;
				for (int rhsIndex = 0; rhsIndex < rhsTmp.size(); rhsIndex++)	//遍历其右项
				{
					const firstTableItem& firstSymbolSet = this->firstTable[rhsTmp[rhsIndex]];
					if (rhsTmp[rhsIndex] <= this->terminalSymbolMax)	//为终结符
					{
						this->firstTable[i].insert(rhsTmp[rhsIndex]);
						break;
					}

					bool haveEmpty = this->firstTable[i].count(this->emptyIndex);
					this->firstTable[i].insert(firstSymbolSet.cbegin(), firstSymbolSet.cend());
					if (!haveEmpty && firstSymbolSet.count(this->emptyIndex))
						this->firstTable[i].erase(this->emptyIndex);

					if (!firstSymbolSet.count(this->emptyIndex))	//不含空
						break;
					if (rhsIndex == rhsTmp.size() - 1)	//如果产生式右侧全为空，加入空
						this->firstTable[i].insert(this->emptyIndex);
				}
			}
			if (this->firstTable[i].size() > cntTmp)
				inc = true;
		}
	} while (inc);	//直到不再增长
}

/*********
 * 计算句子的FIRST集
 * ********/
set<symbolTableIndex> parsing::firstForPhrase(vector<symbolTableIndex> rhsTmp)
{
	set<symbolTableIndex> res;

	for (int rhsIndex = 0; rhsIndex < rhsTmp.size(); rhsIndex++)	//遍历其右项
	{
		const firstTableItem& firstSymbolSet = this->firstTable[rhsTmp[rhsIndex]];
		if (rhsTmp[rhsIndex] <= this->terminalSymbolMax)	//为终结符
		{
			res.insert(rhsTmp[rhsIndex]);
			break;
		}

		bool haveEmpty = res.count(this->emptyIndex);
		res.insert(firstSymbolSet.cbegin(), firstSymbolSet.cend());
		if (!haveEmpty && firstSymbolSet.count(this->emptyIndex))
			res.erase(this->emptyIndex);

		if (!firstSymbolSet.count(this->emptyIndex))	//不含空
			break;
		if (rhsIndex == rhsTmp.size() - 1)	//如果产生式右侧全为空，加入空
			res.insert(this->emptyIndex);
	}

	return res;
}

/*********
 * 重载<用于比较DFA_item
 * ********/
bool operator<(const DFA_item& A, const DFA_item& B)
{
	if (A.lhs < B.lhs)
		return true;
	else if (A.lhs > B.lhs)
		return false;

	if (A.rhs.size() < B.rhs.size())
		return true;
	else if (A.rhs.size() > B.rhs.size())
		return false;
	else
	{
		int i;
		int mm = A.rhs.size();
		for (i = 0; i < mm; i++)
		{
			if (A.rhs[i] < B.rhs[i])
				return true;
			else if (A.rhs[i] > B.rhs[i])
				return false;
		}
	}

	if (A.pos < B.pos)
		return true;
	else if (A.pos > B.pos)
		return false;

	if (A.forecast < B.forecast)
		return true;
	else if (A.forecast > B.forecast)
		return false;

	return false;
}

/*********
 * 构造closure
 * ********/
pair<int, bool>  parsing::createClosure(DFA_status& sta)
{
	vector<symbolTableIndex> restsentence;
	stack<DFA_item> sd;
	DFA_item temptop, tempd;
	//先把所有入栈
	for (auto it = sta.begin(); it != sta.end(); it++)
	{
		sd.push(*it);
	}
	while (!sd.empty())
	{
		temptop = sd.top();
		sd.pop();//栈顶的语句出栈
		if (temptop.pos<temptop.rhs.size() && temptop.rhs[temptop.pos]>terminalSymbolMax)//点后面是非终结符
		{
			restsentence.clear();
			for (int i = temptop.pos + 1; i < temptop.rhs.size(); i++)//提取需要找first集的语句
			{
				restsentence.push_back(temptop.rhs[i]);
			}
			restsentence.push_back(temptop.forecast);
			tempfirst = firstForPhrase(restsentence);//查first集
			//查完了first集，开始构造新的句子
			for (auto it = tempfirst.begin(); it != tempfirst.end(); it++)
			{
				for (auto it2 = searchSyntaxByLhs[temptop.rhs[temptop.pos]].begin(); it2 != searchSyntaxByLhs[temptop.rhs[temptop.pos]].end(); it2++)
				{
					tempd.lhs = temptop.rhs[temptop.pos];
					tempd.rhs = syntaxTable[*it2].rhs;
					tempd.pos = 0;
					tempd.forecast = *it;
					if (sta.insert(tempd).second == true)//成功插入
					{
						sd.push(tempd);//作为一个新的式子入栈
					}
				}
			}
		}
	}
	//检查是否是新状态
	for (int i = 0; i < DFA.size(); i++)
	{
		if (DFA[i] == sta)
			return <i, false>;
	}
	DFA.push_back(sta);
	return <DFA.size() - 1, true>;
}
/*********
 * 初始化 DFA、analyseTable
 * ********/
void parsing::initAnalyseTable()
{
	DFA_status temps, temptopstatus;
	DFA_item temptop, tempd;
	set<int> tempfirst;
	vector<symbolTableIndex> restsentence;
	pair<int, bool>  gt;
	int statusno;
	stack<int> si;
	//先手动构造初始状态0
	//第一条S'->.S,#
	tempd.lhs = symbol2Index["$Start0"];
	tempd.rhs.push_back(symbol2Index["$Start"]);
	tempd.pos = 0;
	tempd.forecast = symbol2Index["$End"];
	temps.insert(tempd);

	createClosure(temps);//创建0号状态

	si.push(0);//把状态0入栈

	//至此第0号状态构建完成
	//接下来开始推导剩余状态
	set<int>transflag;//记录有哪些符号可以用来转移

	while (!si.empty())
	{
		temps.clear();
		statusno = si.top();
		temptopstatus = DFA[statusno];
		si.pop();
		transflag.clear();
		for (auto it = temptopstatus.begin(); it != temptopstatus.end(); it++)//找到这个集合中所有可以转移的字符
		{
			if ((*it).pos < (*it).rhs.size())
			{
				transflag.insert((*it).rhs[(*it).pos]);
			}
		}
		for (auto it = transflag.begin(); it != transflag.end(); it++)//对于每个可引发转移的字符，找移进状态
		{
			for (auto it2 = temptopstatus.begin(); it2 != temptopstatus.end(); it2++)//对于每一条语句
			{
				if ((*it2).pos < (*it2).rhs.size() && (*it2).rhs[(*it2).pos] == *it)
				{
					tempd.lhs = (*it2).lhs;
					tempd.rhs = (*it2).rhs;
					tempd.pos = (*it2).pos + 1;
					tempd.forecast = (*it2).forecast;
					temps.insert(tempd);
				}
			}
			gt = createClosure(temps);
			analyseTable[statusno][*it] = pair<char, int>('s', gt.first);
			if (gt.second == ture)//是新的状态
			{
				si.push(gt.first);
			}
		}
		for (auto it = temptopstatus.begin(); it != temptopstatus.end(); it++)//找规约状态
		{
			if ((*it).pos >= (*it).rhs.size())
			{
				int synno;
				//找到是哪一个规约文法
				for (auto it2 = searchSyntaxByLhs[(*it).lhs].begin(); it2 != searchSyntaxByLhs[(*it).lhs].end(); it2++)
				{
					if (syntaxTable[*it2].rhs == (*it).rhs)
					{
						synno = *it2;
						break;
					}
				}
				//填规约表
				if ((*it).lhs == symbol2Index["$Start0"] && (*it).rhs[0] == symbol2Index["$Start"] && (*it).forecast == symbol2Index["$End"])
					analyseTable[statusno][(*it).forecast] = pair<char, int>('a', -1);
				else
					analyseTable[statusno][(*it).forecast] = pair<char, int>('r', synno);
			}
		}
	}

}

void parsing::clear()
{
	symbolTable.clear();
	symbol2Index.clear();
	syntaxTable.clear();
	searchSyntaxByLhs.clear();
	firstTable.clear();
	DFA.clear();
	analyseTable.clear();
	syntaxTree.clear();
	statusStack = stack<DFA_statusIndex>();
	analyseSymbolStack = stack<syntaxTreeNodeIndex>();
	inputSymbolvector = stack<syntaxTreeNodeIndex>();
}

void parsing::initSyntax(ifstream& fin)
{
	this->initSymbolTable(fin);
	this->initFirstTable();
	this->initAnalyseTable();
}

void parsing::analyze(const vector<pair<Token, string>>& lexs)
{
	//初始化输入符号栈
	this->syntaxTree.push_back(pair<Token, string>{Token::End, ""});
	this->syntaxTree.back().index = syntaxTree.size() - 1;
	this->inputSymbolvector.push(syntaxTree.size() - 1);
	for (int i = lexs.size() - 1; i >= 0; i--)
	{
		this->syntaxTree.push_back(lexs[i]);
		syntaxTreeNodeIndex index = syntaxTree.size() - 1;
		this->syntaxTree.back().index = index;
		this->inputSymbolvector.push(index);
	}
	//初始化分析状态栈
	this->statusStack.push(0);
	//初始化分析符号栈
	this->syntaxTree.push_back(pair<Token, string>{Token::End, ""});
	this->syntaxTree.back().index = syntaxTree.size() - 1;
	this->analyseSymbolStack.push(syntaxTree.size() - 1);

	while (1)
	{
		analyseTableItem nextAction = this->analyseTable[this->statusStack.top()][this->syntaxTree[this->inputSymbolvector.top()].type];
		if (nextAction.first == 'a')	//成功
			break;
		else if (nextAction.first == 's')	//移进
		{
			this->statusStack.push(nextAction.second);
			syntaxTreeNodeIndex tmp = this->inputSymbolvector.top();
			this->inputSymbolvector.pop();
			this->analyseSymbolStack.push(tmp);
		}
		else if (nextAction.first == 'r')	//归约
		{
			const syntaxTableItem& useSyntax = this->syntaxTable[nextAction.second];
			vector<syntaxTreeNodeIndex> rhsTmp;
			for (int i = 0; i < useSyntax.rhs.size(); i++)
			{
				syntaxTreeNodeIndex tmp = this->analyseSymbolStack.top();
				this->analyseSymbolStack.pop();
				this->statusStack.pop();
				rhsTmp.push_back(tmp);
				syntaxTree[tmp].parent = syntaxTree.size();
			}
			syntaxTreeNode lhsTmp;
			lhsTmp.children = rhsTmp;
			lhsTmp.productions = nextAction.second;
			lhsTmp.type = useSyntax.lhs;
			syntaxTree.push_back(lhsTmp);
			this->syntaxTree.back().index = syntaxTree.size() - 1;
			this->analyseSymbolStack.push(syntaxTree.size() - 1);

			nextAction = this->analyseTable[this->statusStack.top()][this->syntaxTree[this->analyseSymbolStack.top()].type];
			if (nextAction.first == 's')
			{
				this->statusStack.push(nextAction.second);
			}
			else
			{
				cout << "语法分析出错" << endl;
				break;
			}
		}
		else								//错误
		{
			cout << "语法分析出错" << endl;
			break;
		}
	}
}
