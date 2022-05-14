#include <sstream>
#include <string>
#include <iostream>
#include <algorithm>
#include "Parsing.h"
using namespace std;

proc_symbolTable *beforeSymbolTable;

syntaxTreeNode::syntaxTreeNode(const pair<Token, string> &rhs) : index(0), parent(-1), productions(-1), type(0), inTree(false)
{
	this->type = int(rhs.first);
	this->val = rhs.second;
}

syntaxTreeNode::syntaxTreeNode() : index(0), parent(-1), productions(-1), type(0), inTree(false), quad(0), stype(symbolType::Unknown)
{
}

/*********
 * 插入到 symbolTable 中，同时更新 symbol2Index 表
 * 如果不存在就插入，存在则直接返回序号
 * ********/
symbolTableIndex parsing::insertSymbol(symbolItem insrt)
{
	if (!this->symbol2Index.count(insrt)) // map中不存在
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
	insertSymbol("$LeftArray");
	insertSymbol("$RightArray");
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
void parsing::initSymbolTable(ifstream &infile)
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

	this->syntaxTable.push_back({this->startIndex, {this->symbol2Index["$Start"]}}); //添加S' ==> S
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
			if (symbolTmp < this->terminalSymbolMax) //产生式右侧首项为终结符,非空
				this->firstTable[i].insert(symbolTmp);
		}
	}
	//非终结符FIRST集互推
	bool inc = false;
	do
	{
		inc = false;
		for (int i = this->terminalSymbolMax + 1; i < this->symbolTable.size(); i++) //对于每一个非终结符集
		{
			int cntTmp = this->firstTable[i].size();
			for (auto syntaxIndexTmp : this->searchSyntaxByLhs[i]) //对于其为左项的产生式
			{
				const vector<symbolTableIndex> &rhsTmp = this->syntaxTable[syntaxIndexTmp].rhs;
				for (int rhsIndex = 0; rhsIndex < rhsTmp.size(); rhsIndex++) //遍历其右项
				{
					const firstTableItem &firstSymbolSet = this->firstTable[rhsTmp[rhsIndex]];
					if (rhsTmp[rhsIndex] <= this->terminalSymbolMax) //为终结符
					{
						this->firstTable[i].insert(rhsTmp[rhsIndex]);
						break;
					}

					bool haveEmpty = this->firstTable[i].count(this->emptyIndex);
					this->firstTable[i].insert(firstSymbolSet.cbegin(), firstSymbolSet.cend());
					if (!haveEmpty && firstSymbolSet.count(this->emptyIndex))
						this->firstTable[i].erase(this->emptyIndex);

					if (!firstSymbolSet.count(this->emptyIndex)) //不含空
						break;
					if (rhsIndex == rhsTmp.size() - 1) //如果产生式右侧全为空，加入空
						this->firstTable[i].insert(this->emptyIndex);
				}
			}
			if (this->firstTable[i].size() > cntTmp)
				inc = true;
		}
	} while (inc); //直到不再增长
}

/*********
 * 计算句子的FIRST集
 * ********/
set<symbolTableIndex> parsing::firstForPhrase(vector<symbolTableIndex> rhsTmp)
{
	set<symbolTableIndex> res;

	for (int rhsIndex = 0; rhsIndex < rhsTmp.size(); rhsIndex++) //遍历其右项
	{
		const firstTableItem &firstSymbolSet = this->firstTable[rhsTmp[rhsIndex]];
		if (rhsTmp[rhsIndex] <= this->terminalSymbolMax) //为终结符
		{
			res.insert(rhsTmp[rhsIndex]);
			break;
		}

		bool haveEmpty = res.count(this->emptyIndex);
		res.insert(firstSymbolSet.cbegin(), firstSymbolSet.cend());
		if (!haveEmpty && firstSymbolSet.count(this->emptyIndex))
			res.erase(this->emptyIndex);

		if (!firstSymbolSet.count(this->emptyIndex)) //不含空
			break;
		if (rhsIndex == rhsTmp.size() - 1) //如果产生式右侧全为空，加入空
			res.insert(this->emptyIndex);
	}

	return res;
}

/*********
 * 重载<用于比较DFA_item
 * ********/
bool operator<(const DFA_item &A, const DFA_item &B)
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
 * 重载==用于比较DFA_item
 * ********/
bool operator==(const DFA_item &A, const DFA_item &B)
{
	if (A.lhs == B.lhs && A.rhs == B.rhs && A.pos == B.pos && A.forecast == B.forecast)
		return true;
	else
		return false;
}

/*********
 * 构造closure
 * ********/
pair<int, bool> parsing::createClosure(DFA_status &sta)
{
	set<int> tempfirst;
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
		sd.pop();																			  //栈顶的语句出栈
		if (temptop.pos < temptop.rhs.size() && temptop.rhs[temptop.pos] > terminalSymbolMax) //点后面是非终结符
		{
			restsentence.clear();
			for (int i = temptop.pos + 1; i < temptop.rhs.size(); i++) //提取需要找first集的语句
			{
				restsentence.push_back(temptop.rhs[i]);
			}
			restsentence.push_back(temptop.forecast);
			tempfirst = firstForPhrase(restsentence); //查first集
			//查完了first集，开始构造新的句子
			for (auto it = tempfirst.begin(); it != tempfirst.end(); it++)
			{
				for (auto it2 = searchSyntaxByLhs[temptop.rhs[temptop.pos]].begin(); it2 != searchSyntaxByLhs[temptop.rhs[temptop.pos]].end(); it2++)
				{
					tempd.lhs = temptop.rhs[temptop.pos];
					tempd.rhs = syntaxTable[*it2].rhs;
					tempd.pos = 0;
					tempd.forecast = *it;
					if (sta.insert(tempd).second == true) //成功插入
					{
						sd.push(tempd); //作为一个新的式子入栈
					}
				}
			}
		}
	}
	//检查是否是新状态
	for (int i = 0; i < DFA.size(); i++)
	{
		if (DFA[i] == sta)
			return pair<int, bool>(i, false);
	}
	DFA.push_back(sta);
	return pair<int, bool>(DFA.size() - 1, true);
}

/*********
 * 初始化 DFA、analyseTable
 * ********/
void parsing::initAnalyseTable()
{
	DFA_status temps;
	DFA_item temptop, tempd;
	set<int> tempfirst;
	vector<symbolTableIndex> restsentence;
	pair<int, bool> gt;
	int statusno;
	stack<int> si;
	//先手动构造初始状态0
	//第一条S'->.S,#
	tempd.lhs = symbol2Index["$Start0"];
	tempd.rhs.push_back(symbol2Index["$Start"]);
	tempd.pos = 0;
	tempd.forecast = symbol2Index["$End"];
	temps.insert(tempd);

	createClosure(temps); //创建0号状态

	si.push(0); //把状态0入栈

	//至此第0号状态构建完成
	//接下来开始推导剩余状态
	set<int> transflag; //记录有哪些符号可以用来转移
	analyseTable.push_back(vector<analyseTableItem>(symbolTable.size(), pair<char, int>('\0', -1)));
	bool empty_flag = 0; //记录是否有推出空

	while (!si.empty())
	{
		empty_flag = 0;
		// debugdfa();//调试
		statusno = si.top();
		// DFA_status& DFA[statusno] = DFA[statusno];
		si.pop();
		transflag.clear();
		for (auto it = DFA[statusno].begin(); it != DFA[statusno].end(); it++) //找到这个集合中所有可以转移的字符
		{
			if ((*it).pos < (*it).rhs.size())
			{
				transflag.insert((*it).rhs[(*it).pos]);
			}
		}
		for (auto it = transflag.begin(); it != transflag.end(); it++) //对于每个可引发转移的字符，找移进状态
		{
			if (*it == emptyIndex) //特殊处理生成empty的内容，加入原本状态不写转移表
			{
				// empty_flag = 1;//标记推出空，最后在处理不然会死循环
				/*
				for (auto it2 = DFA[statusno].begin(); it2 != DFA[statusno].end(); it2++) //对于每一条语句
				{
					if ((*it2).pos < (*it2).rhs.size() && (*it2).rhs[(*it2).pos] == *it)
					{
						tempd.lhs = (*it2).lhs;
						tempd.rhs = (*it2).rhs;
						tempd.pos = (*it2).pos + 1;
						tempd.forecast = (*it2).forecast;
						DFA[statusno].insert(tempd);
					}
				}
				*/
				;
			}
			else //正常转移写转移表
			{
				temps.clear();
				for (auto it2 = DFA[statusno].begin(); it2 != DFA[statusno].end(); it2++) //对于每一条语句
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
				if (gt.second == true) //是新的状态
				{
					si.push(gt.first);
					analyseTable.push_back(vector<analyseTableItem>(symbolTable.size(), pair<char, int>('\0', -1)));
				}
			}
		}
		for (auto it = DFA[statusno].begin(); it != DFA[statusno].end(); it++) //找规约状态
		{
			if ((*it).pos >= (*it).rhs.size() || (*it).rhs[0] == emptyIndex)
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
				if (analyseTable[statusno][(*it).forecast].first != '\0')
					cout << "wrong" << endl;
				if ((*it).lhs == symbol2Index["$Start0"] && (*it).rhs[0] == symbol2Index["$Start"] && (*it).forecast == symbol2Index["$End"])
					analyseTable[statusno][(*it).forecast] = pair<char, int>('a', -1);
				else
					analyseTable[statusno][(*it).forecast] = pair<char, int>('r', synno);
			}
		}
	}
}

void parsing::debugdfa() //用来调试dfa
{
	ofstream of;
	of.open("dfadebug.txt");
	for (int i = 0; i < DFA.size(); i++)
	{
		of << "状态" << i << endl;
		for (auto it = DFA[i].begin(); it != DFA[i].end(); it++)
		{
			of << symbolTable[(*it).lhs] << " ::= ";
			for (int j = 0; j < (*it).rhs.size(); j++)
			{
				if (j == (*it).pos)
					of << " dot ";
				of << symbolTable[(*it).rhs[j]] << ' ';
			}
			if ((*it).pos == (*it).rhs.size())
				of << " dot ";
			of << "," << symbolTable[(*it).forecast];
			of << endl;
		}
	}
	of.close();
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

void parsing::initSyntax(ifstream &fin)
{
	this->initSymbolTable(fin);
	this->initFirstTable();
	this->initAnalyseTable();
	this->debugdfa();
}

void parsing::analyze(const vector<pair<Token, string>> &lexs)
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
	this->p_symbolTable = new proc_symbolTable();

	while (1)
	{
		if (this->syntaxTree[this->analyseSymbolStack.top()].type == this->symbol2Index["$Start"] && this->syntaxTree[this->inputSymbolvector.top()].type == this->symbol2Index["$End"])
			break;
		analyseTableItem nextAction = this->analyseTable[this->statusStack.top()][this->syntaxTree[this->inputSymbolvector.top()].type];
		if (nextAction.first == 'a') //成功
			break;
		else if (nextAction.first == 's') //移进
		{
			this->statusStack.push(nextAction.second);
			syntaxTreeNodeIndex tmp = this->inputSymbolvector.top();
			this->inputSymbolvector.pop();
			this->analyseSymbolStack.push(tmp);
		}
		else if (nextAction.first == 'r') //归约
		{
			const syntaxTableItem &useSyntax = this->syntaxTable[nextAction.second];
			vector<syntaxTreeNodeIndex> rhsTmp;
			if (useSyntax.rhs[0] != this->emptyIndex)
			{
				for (int i = 0; i < useSyntax.rhs.size(); i++)
				{
					syntaxTreeNodeIndex tmp = this->analyseSymbolStack.top();
					this->analyseSymbolStack.pop();
					this->statusStack.pop();
					rhsTmp.push_back(tmp);
					syntaxTree[tmp].parent = syntaxTree.size();
				}
			}
			syntaxTreeNode lhsTmp;
			reverse(rhsTmp.begin(), rhsTmp.end());
			lhsTmp.children = rhsTmp;
			lhsTmp.productions = nextAction.second;
			lhsTmp.type = useSyntax.lhs;
			this->generate_midcode(nextAction.second, lhsTmp, rhsTmp);
			syntaxTree.push_back(lhsTmp);
			this->syntaxTree.back().index = syntaxTree.size() - 1;
			this->analyseSymbolStack.push(syntaxTree.size() - 1);
			this->topNode = syntaxTree.size() - 1;

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
		else //错误
		{
			cout << "语法分析出错" << endl;
			break;
		}
	}
}

void parsing::generate_midcode(syntaxTableIndex SyntaxIndex, syntaxTreeNode &lhs, vector<syntaxTreeNodeIndex> &rhs)
{
	proc_symbolTable *functmp;
	switch (SyntaxIndex)
	{
	case 0: //$Start ::= <N> <声明串>
		mid_code.back_patch(syntaxTree[rhs[0]].nextlist, this->p_symbolTable->find_function("main")->get_enterquad());
		break;
	case 1: //<A> ::= $Empty
		lhs.quad = mid_code.nextquad;
		p_symbolTable = p_symbolTable->create_function(); //进入函数
		break;
	case 2: //<N> ::= $Empty
		lhs.nextlist.push_back(mid_code.nextquad);
		mid_code.emit_code(quadruple(Oper::J, string(""), string(""), string("")));
		break;
	case 3: //<M> ::= $Empty
		lhs.quad = mid_code.nextquad;
		break;
	case 4: //<声明串> ::= <声明>
		break;
	case 5: //<声明串> ::= <声明串> <声明>
		break;
	case 6: //<声明> ::= $Int $ID <声明类型>
		p_symbolTable->insert_variable({symbolType::Int, syntaxTree[rhs[1]].val, proc_symbolTable::newtemp(), p_symbolTable->get_offset()});
		break;
	case 7: //<声明> ::= $Void $ID <M> <A> <函数声明>
		beforeSymbolTable->init_function(syntaxTree[rhs[4]].plist, syntaxTree[rhs[1]].val, symbolType::Void, syntaxTree[rhs[2]].quad);
		break;
	case 8: //<声明> ::= $Int $ID <M> <A> <函数声明>
		beforeSymbolTable->init_function(syntaxTree[rhs[4]].plist, syntaxTree[rhs[1]].val, symbolType::Int, syntaxTree[rhs[2]].quad);
		break;
	case 9: //<声明类型> ::=  $Semi
		break;
	case 10: //<函数声明> ::= $LeftBracket <形参> $RightBracket <语句块>
		lhs.plist = syntaxTree[rhs[1]].plist;
		break;
	case 11: //<形参> ::= <参数列表>
		lhs.plist = syntaxTree[rhs[0]].plist;
		break;
	case 12: //<形参> ::= $Void
		lhs.plist = vector<symbolTableItem>();
		break;
	case 13: //<参数列表> ::= <参数>
		lhs.plist.push_back(syntaxTree[rhs[0]].plist[0]);
		p_symbolTable->insert_variable({syntaxTree[rhs[0]].plist[0].type, syntaxTree[rhs[0]].plist[0].name, proc_symbolTable::newtemp(), p_symbolTable->get_offset()});
		break;
	case 14: //<参数列表> ::= <参数> $Comma <参数列表>
		lhs.plist.push_back(syntaxTree[rhs[0]].plist[0]);
		p_symbolTable->insert_variable({syntaxTree[rhs[0]].plist[0].type, syntaxTree[rhs[0]].plist[0].name, proc_symbolTable::newtemp(), p_symbolTable->get_offset()});
		lhs.plist.insert(lhs.plist.end(), syntaxTree[rhs[2]].plist.begin(), syntaxTree[rhs[2]].plist.end());
		break;
	case 15: //<参数> ::= $Int $ID
		lhs.plist.push_back({symbolType::Int, syntaxTree[rhs[1]].val, proc_symbolTable::newtemp(), 0});
		break;
	case 16: //<语句块> ::= $LeftBrace <内部声明> <语句串> $RightBrace
		lhs.nextlist = syntaxTree[rhs[2]].nextlist;
		beforeSymbolTable = p_symbolTable;
		p_symbolTable = p_symbolTable->return_block(mid_code.nextquad);
		break;
	case 17: //<内部声明> ::= $Empty
		break;
	case 18: //<内部声明> ::= <内部变量声明> <内部声明>
		break;
	case 19: //<内部变量声明> ::= $Int $ID $Semi
		p_symbolTable->insert_variable({symbolType::Int, syntaxTree[rhs[1]].val, proc_symbolTable::newtemp(), p_symbolTable->get_offset()});
		break;
	case 20: //<语句串> ::= <语句>
		lhs.nextlist = syntaxTree[rhs[0]].nextlist;
		break;
	case 21: //<语句串> ::= <语句> <M> <语句串>
		lhs.nextlist = syntaxTree[rhs[2]].nextlist;
		mid_code.back_patch(syntaxTree[rhs[0]].nextlist, syntaxTree[rhs[1]].quad);
		break;
	case 22: //<语句> ::= <if语句>
		lhs.nextlist = syntaxTree[rhs[0]].nextlist;
		break;
	case 23: //<语句> :: = <while语句>
		lhs.nextlist = syntaxTree[rhs[0]].nextlist;
		break;
	case 24: //<语句> ::= <return语句>
		break;
	case 25: //<语句> ::= <assign语句>
		break;
	case 26: //<assign语句> ::= $ID $Equal <表达式> $Semi
		if (p_symbolTable->find_variable(syntaxTree[rhs[0]].val).type == symbolType::None)
			cout << syntaxTree[rhs[0]].val << "不在符号表中" << endl;
		else
			mid_code.emit_code(quadruple(Oper::Assign, syntaxTree[rhs[2]].place, string(""), p_symbolTable->find_variable(syntaxTree[rhs[0]].val).gobalname));
		break;
	case 27: //<return语句> ::= $Return $Semi
		mid_code.emit_code(quadruple(Oper::Return, string(""), string(""), string("")));
		break;
	case 28: //<return语句> ::= $Return <表达式> $Semi
		mid_code.emit_code(quadruple(Oper::Return, syntaxTree[rhs[1]].place, string(""), string("ReturnValue")));
		break;
	case 29: //<while语句> ::= $While <M> $LeftBracket <表达式> $RightBracket <A> <语句块>
		mid_code.back_patch(syntaxTree[rhs[6]].nextlist, syntaxTree[rhs[1]].quad);
		mid_code.back_patch(syntaxTree[rhs[3]].truelist, syntaxTree[rhs[5]].quad);
		lhs.nextlist = syntaxTree[rhs[3]].falselist;
		mid_code.emit_code(quadruple(Oper::J, string(""), string(""), to_string(syntaxTree[rhs[1]].quad)));
		break;
	case 30: //<if语句> ::= $If $LeftBracket <表达式> $RightBracket <A> <语句块>
		mid_code.back_patch(syntaxTree[rhs[2]].truelist, syntaxTree[rhs[4]].quad);
		lhs.nextlist = mergelist(syntaxTree[rhs[2]].falselist, syntaxTree[rhs[4]].nextlist);
		break;
	case 31: //<if语句> ::= $If $LeftBracket <表达式> $RightBracket <A> <语句块> <N> $Else <M> <A> <语句块>
		mid_code.back_patch(syntaxTree[rhs[2]].truelist, syntaxTree[rhs[4]].quad);
		mid_code.back_patch(syntaxTree[rhs[2]].falselist, syntaxTree[rhs[8]].quad);
		lhs.nextlist = mergelist(syntaxTree[rhs[5]].nextlist, syntaxTree[rhs[6]].nextlist, syntaxTree[rhs[10]].nextlist);
		break;
	case 32: //<表达式> ::= <加法表达式>
		lhs.place = syntaxTree[rhs[0]].place;
		break;
	case 33: //<表达式> ::= <表达式> <比较运算符> <加法表达式>
		lhs.truelist.push_back(mid_code.nextquad);
		lhs.falselist.push_back(mid_code.nextquad + 1);
		if (syntaxTree[rhs[1]].place == "$Smaller")
			mid_code.emit_code(quadruple(Oper::Jlt, syntaxTree[rhs[0]].place, syntaxTree[rhs[2]].place, string("0")));
		else if (syntaxTree[rhs[1]].place == "$SmallerEqual")
			mid_code.emit_code(quadruple(Oper::Jle, syntaxTree[rhs[0]].place, syntaxTree[rhs[2]].place, string("0")));
		else if (syntaxTree[rhs[1]].place == "$Bigger")
			mid_code.emit_code(quadruple(Oper::Jgt, syntaxTree[rhs[0]].place, syntaxTree[rhs[2]].place, string("0")));
		else if (syntaxTree[rhs[1]].place == "$BiggerEqual")
			mid_code.emit_code(quadruple(Oper::Jge, syntaxTree[rhs[0]].place, syntaxTree[rhs[2]].place, string("0")));
		else if (syntaxTree[rhs[1]].place == "$Equal2")
			mid_code.emit_code(quadruple(Oper::Jeq, syntaxTree[rhs[0]].place, syntaxTree[rhs[2]].place, string("0")));
		else if (syntaxTree[rhs[1]].place == "$NotEqual")
			mid_code.emit_code(quadruple(Oper::Jne, syntaxTree[rhs[0]].place, syntaxTree[rhs[2]].place, string("0")));
		else
			cout << "wrong comparation" << endl;

		mid_code.emit_code(quadruple(Oper::J, string(""), string(""), string("0")));
		break;
	case 34: //<比较运算符> ::= $Smaller
		lhs.place = "$Smaller";
		break;
	case 35: //<比较运算符> ::= $SmallerEqual
		lhs.place = "$SmallerEqual";
		break;
	case 36: //<比较运算符> ::= $Bigger
		lhs.place = "$Bigger";
		break;
	case 37: //<比较运算符> ::= $BiggerEqual
		lhs.place = "$BiggerEqual";
		break;
	case 38: //<比较运算符> ::= $Equal2
		lhs.place = "$Equal2";
		break;
	case 39: //<比较运算符> ::= $NotEqual
		lhs.place = "$NotEqual";
		break;
	case 40: //<加法表达式> ::= <项>
		lhs.place = syntaxTree[rhs[0]].place;
		break;
	case 41: //<加法表达式> ::= <项> $Plus <加法表达式>
		lhs.place = proc_symbolTable::newtemp();
		mid_code.emit_code(quadruple(Oper::Plus, syntaxTree[rhs[0]].place, syntaxTree[rhs[2]].place, lhs.place));
		break;
	case 42: //<加法表达式> ::= <项> $Minus <加法表达式>
		lhs.place = proc_symbolTable::newtemp();
		mid_code.emit_code(quadruple(Oper::Minus, syntaxTree[rhs[0]].place, syntaxTree[rhs[2]].place, lhs.place));
		break;
	case 43: //<项> ::= <因子>
		lhs.place = syntaxTree[rhs[0]].place;
		break;
	case 44: //<项> ::= <因子> $Multiply <项>
		lhs.place = proc_symbolTable::newtemp();
		mid_code.emit_code(quadruple(Oper::Multiply, syntaxTree[rhs[0]].place, syntaxTree[rhs[2]].place, lhs.place));
		break;
	case 45: //<项> ::= <因子> $Divide <项>
		lhs.place = proc_symbolTable::newtemp();
		mid_code.emit_code(quadruple(Oper::Divide, syntaxTree[rhs[0]].place, syntaxTree[rhs[2]].place, lhs.place));
		break;
	case 46: //<因子> ::= $Number
		lhs.place = syntaxTree[rhs[0]].val;
		break;
	case 47: //<因子> ::= $LeftBracket <表达式> $RightBracket
		lhs.place = syntaxTree[rhs[1]].place;
		break;
	case 48: //<因子> ::= $ID $LeftBracket <实参列表> $RightBracket
		functmp = p_symbolTable->find_function(syntaxTree[rhs[0]].val);
		if (functmp == NULL)
			cout << syntaxTree[rhs[0]].val << "不在函数表中" << endl;
		else if (functmp->parm.size() != syntaxTree[rhs[2]].plist.size())
			cout << syntaxTree[rhs[0]].val << "函数实参列表不符" << endl;
		else
		{
			for (const auto &i : syntaxTree[rhs[2]].plist)
			{
				mid_code.emit_code(quadruple(Oper::Parm, i.gobalname, string(""), syntaxTree[rhs[0]].val));
			}
			if (functmp->return_type == symbolType::Int)
			{
				lhs.place = p_symbolTable->newtemp();
				mid_code.emit_code(quadruple(Oper::Call, syntaxTree[rhs[0]].val, string(""), lhs.place));
			}
			else
			{
				mid_code.emit_code(quadruple(Oper::Call, syntaxTree[rhs[0]].val, string(""), string("")));
			}
		}
		break;
	case 49: //<因子> ::= $ID
		if (p_symbolTable->find_variable(syntaxTree[rhs[0]].val).type == symbolType::None)
			cout << syntaxTree[rhs[0]].val << "不在符号表中" << endl;
		else
			lhs.place = p_symbolTable->find_variable(syntaxTree[rhs[0]].val).gobalname;
		break;
	case 50: //<实参列表> ::= $Empty
		break;
	case 51: //<实参列表> ::= <表达式>

		lhs.plist.push_back({symbolType::Unknown, syntaxTree[rhs[0]].place, syntaxTree[rhs[0]].place, 0});
		break;
	case 52: //<实参列表> ::= <表达式> $Comma <实参列表>
		lhs.plist.push_back({symbolType::Unknown, syntaxTree[rhs[0]].place, syntaxTree[rhs[0]].place, 0});
		lhs.plist.insert(lhs.plist.end(), syntaxTree[rhs[2]].plist.begin(), syntaxTree[rhs[2]].plist.end());
		break;
	case 53: //<因子> ::= <数组>
		{
			symbolTableItem variable = p_symbolTable->find_variable(syntaxTree[rhs[0]].place);
			if(variable.array.size() != syntaxTree[rhs[0]].array_vec.size())
			{
				cout << syntaxTree[rhs[0]].place << "数组维度不对应"  << variable.array.size() << syntaxTree[rhs[0]].array_vec.size() << endl;
				break;
			}
			string this_place, before_place;
			int len = syntaxTree[rhs[0]].array_vec.size();
			for(int i = 0 ; i < len; i++)
			{
				this_place = proc_symbolTable::newtemp();
				if (i == 0)
				{
					mid_code.emit_code(quadruple(Oper::Plus, syntaxTree[rhs[0]].array_vec[i], to_string(0), this_place));
				}
				else
				{
					mid_code.emit_code(quadruple(Oper::Multiply, before_place, to_string(variable.array[i]), this_place));
					mid_code.emit_code(quadruple(Oper::Plus, this_place, syntaxTree[rhs[0]].array_vec[i], this_place));
				}
				before_place = this_place;
			}
			lhs.place = proc_symbolTable::newtemp();
			mid_code.emit_code(quadruple(Oper::AssignArray, variable.gobalname, this_place, lhs.place));
			break;
		}
	case 54: //<数组> ::= $ID $LeftArray <表达式> $RightArray
		if (p_symbolTable->find_variable(syntaxTree[rhs[0]].val).type == symbolType::None)
			cout << syntaxTree[rhs[0]].val << "不在符号表中" << endl;
		else
		{
			symbolTableItem variable = p_symbolTable->find_variable(syntaxTree[rhs[0]].val);
			if( variable.type != symbolType::Array )
				cout << syntaxTree[rhs[0]].val << "不是数组" << endl;
			else
			{
				lhs.place = syntaxTree[rhs[0]].val;
				lhs.array_vec.push_back(syntaxTree[rhs[2]].place);
			}
		}	
		break;
	case 55: //<数组> ::= <数组> $LeftArray <表达式> $RightArray
		lhs.place = syntaxTree[rhs[0]].place;
		lhs.array_vec = syntaxTree[rhs[0]].array_vec;
		lhs.array_vec.push_back(syntaxTree[rhs[2]].place);
		break;
	case 56: //<assign语句> ::= <数组> $Equal <表达式> $Semi
		{
			symbolTableItem variable = p_symbolTable->find_variable(syntaxTree[rhs[0]].place);
			if(variable.array.size() != syntaxTree[rhs[0]].array_vec.size())
			{
				cout << syntaxTree[rhs[0]].place << "数组维度不对应" << endl;
				break;
			}
			string this_place, before_place;
			int len = syntaxTree[rhs[0]].array_vec.size();
			for(int i = 0 ; i < len; i++)
			{
				this_place = proc_symbolTable::newtemp();
				if (i == 0)
				{
					mid_code.emit_code(quadruple(Oper::Plus, syntaxTree[rhs[0]].array_vec[i], to_string(0), this_place));
				}
				else
				{
					mid_code.emit_code(quadruple(Oper::Multiply, before_place, to_string(variable.array[i]), this_place));
					mid_code.emit_code(quadruple(Oper::Plus, this_place, syntaxTree[rhs[0]].array_vec[i], this_place));
				}
				before_place = this_place;
			}
			lhs.place = proc_symbolTable::newtemp();
			mid_code.emit_code(quadruple(Oper::ArrayAssign, syntaxTree[rhs[2]].place, variable.gobalname,this_place ));
			break;
		}
		break;
	case 57: //<声明> ::= $Int $ID <数组声明> $Semi
		{
			vector<int> vec;
			for(const auto &i : syntaxTree[rhs[2]].array_vec)
				vec.push_back(stoi(i));
			p_symbolTable->insert_variable({symbolType::Array, syntaxTree[rhs[1]].val, proc_symbolTable::newtemp(), p_symbolTable->get_offset(), vec});
			break;
		}
	case 58: //<数组声明> ::= $LeftArray $Number $RightArray
		lhs.array_vec.push_back(syntaxTree[rhs[1]].val);
		break;
	case 59: //<数组声明> ::= <数组声明> $LeftArray $Number $RightArray
		lhs.array_vec = syntaxTree[rhs[0]].array_vec;
		lhs.array_vec.push_back(syntaxTree[rhs[2]].val);
		break;
	case 60: //<内部变量声明> ::= $Int $ID <数组声明> $Semi
		{
			vector<int> vec;
			for(const auto &i : syntaxTree[rhs[2]].array_vec)
				vec.push_back(stoi(i));
			p_symbolTable->insert_variable({symbolType::Array, syntaxTree[rhs[1]].val, proc_symbolTable::newtemp(), p_symbolTable->get_offset(), vec});
			break;
		}
	default:
		break;
	}
	return;
}

void parsing::outputStruction(ofstream &struction, syntaxTreeNodeIndex Node, int pad)
{
	this->syntaxTree[Node].inTree = true;
	struction << string(pad, '	') << "{" << endl;
	struction << string(pad + 1, '	') << "\"kind\": \"" << this->symbolTable[this->syntaxTree[Node].type] << "\"";
	if (!this->syntaxTree[Node].val.empty() || this->syntaxTree[Node].productions != -1 || !this->syntaxTree[Node].children.empty())
		struction << ",";
	struction << endl;

	if (!this->syntaxTree[Node].val.empty())
	{
		struction << string(pad + 1, '	') << "\"val\": \"" << this->syntaxTree[Node].val << "\"";
		if (this->syntaxTree[Node].productions != -1 || !this->syntaxTree[Node].children.empty())
			struction << ",";
		struction << endl;
	}

	if (this->syntaxTree[Node].productions != -1)
	{
		struction << string(pad + 1, '	') << "\"type\": \"";
		for (auto rhs : this->syntaxTable[this->syntaxTree[Node].productions].rhs)
			struction << this->symbolTable[rhs] << " ";
		struction << "\"";
		if (!this->syntaxTree[Node].children.empty())
			struction << ",";
		struction << endl;
	}

	if (!this->syntaxTree[Node].children.empty())
	{
		struction << string(pad + 1, '	') << "\"inner\": [" << endl;
		for (int i = 0; i < this->syntaxTree[Node].children.size(); i++)
		{
			this->outputStruction(struction, syntaxTree[Node].children[i], pad + 2);
			if (i != this->syntaxTree[Node].children.size() - 1)
				struction << "," << endl;
		}
		struction << endl
				  << string(pad + 1, '	') << "]" << endl;
	}
	struction << string(pad, '	') << "}";
}

void parsing::outputDot(ofstream &graph, syntaxTreeNodeIndex Node)
{
	if (!this->syntaxTree[Node].children.empty())
	{
		for (auto child : this->syntaxTree[Node].children)
		{
			graph << "	"
				  << "Node" << Node << "->"
				  << "Node" << child << endl;
		}

		if (this->syntaxTree[Node].children.size() > 1)
		{
			graph << "	"
				  << "{" << endl
				  << "		"
				  << "rank = same;" << endl
				  << "		";
			for (int i = 0; i < this->syntaxTree[Node].children.size(); i++)
			{
				graph
					<< "Node" << this->syntaxTree[Node].children[i];
				if (i < this->syntaxTree[Node].children.size() - 1)
					graph << "->";
			}
			graph << "[color=white];" << endl
				  << "		"
				  << "rankdir=LR;" << endl
				  << "	"
				  << "}" << endl;
		}

		for (auto child : this->syntaxTree[Node].children)
		{
			this->outputDot(graph, child);
		}
	}
}

void parsing::output(ofstream &struction, ofstream &graph)
{
	this->outputStruction(struction, this->topNode, 0);
	graph << "#@startdot" << endl
		  << endl;
	graph << "digraph demo {" << endl
		  << "node [fontname=\"Fangsong\" shape=plaintext]" << endl
		  << endl;

	for (int i = 0; i < syntaxTree.size(); i++)
		if (this->syntaxTree[i].inTree)
			graph << "	"
				  << "Node" << i << "[label=\"" << this->symbolTable[this->syntaxTree[i].type] << "\", shape=\"box\"]" << endl;

	graph << endl;
	this->outputDot(graph, this->topNode);

	graph << endl
		  << "}" << endl
		  << endl;
	graph << "#@enddot" << endl;
}

void parsing::outputMidcode(ofstream &midcode)
{
	this->mid_code.output(midcode);
}

vector<quadrupleIndex> parsing::mergelist(vector<quadrupleIndex> &list1, vector<quadrupleIndex> &list2)
{
	vector<quadrupleIndex> temp;
	temp.insert(temp.end(), list1.begin(), list1.end());
	temp.insert(temp.end(), list2.begin(), list2.end());
	return temp;
}

vector<quadrupleIndex> parsing::mergelist(vector<quadrupleIndex> &list1, vector<quadrupleIndex> &list2, vector<quadrupleIndex> &list3)
{
	vector<quadrupleIndex> temp;
	temp.insert(temp.end(), list1.begin(), list1.end());
	temp.insert(temp.end(), list2.begin(), list2.end());
	temp.insert(temp.end(), list3.begin(), list3.end());
	return temp;
}
