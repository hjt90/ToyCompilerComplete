#pragma once
#pragma execution_character_set("utf-8")
#include "quadruple.h"
#include "Optimizer.h"

class VarInfomation {
public:
	int next;//待用信息
	bool active;//活跃信息

	VarInfomation(int next, bool active);
	VarInfomation(const VarInfomation& other);
	VarInfomation();
	void output(ostream& out);
};

class QuaternaryWithInfo {
public:
	quadruple q;
	VarInfomation info1;
	VarInfomation info2;
	VarInfomation info3;

	QuaternaryWithInfo(quadruple q, VarInfomation info1, VarInfomation info2, VarInfomation info3);
	void output(ostream& out);
};

struct IBlock {
	string name;
	vector<QuaternaryWithInfo> codes;
	int next1;
	int next2;
};

class ObjectCode {
private:
	map<string, vector<IBlock> >funcIBlocks;
	map<string, set<string> >Avalue;
	map<string, set<string> >Rvalue;
	map<string, int>varOffset;//各变量的存储位置
	int top;//当前栈顶
	list<string>freeReg;//空闲的寄存器编号
	map<string, vector<set<string> > >funcOUTL;//各函数块中基本块的出口活跃变量集
	map<string, vector<set<string> > >funcINL;//各函数块中基本块的入口活跃变量集
	string nowFunc;//当前分析的函数
	vector<IBlock>::iterator nowIBlock;//当前分析的基本块
	vector<QuaternaryWithInfo>::iterator nowQuatenary;//当前分析的四元式
	vector<string>objectCodes;

	void storeVar(string reg, string var);
	void storeOutLiveVar(set<string>& outl);
	void releaseVar(string var);
	string getReg();
	string getArrayReg();
	string allocateReg();
	string allocateReg(string var);

	void generateArrayData(const proc_symbolTable*);
	void generateCodeForFuncBlocks(map<string, vector<IBlock> >::iterator& fiter);
	void generateCodeForBaseBlocks(int nowBaseBlockIndex);
	void generateCodeForQuatenary(int nowBaseBlockIndex, int& arg_num, int& par_num, list<pair<string, bool> >& par_list);
public:
	ObjectCode();
	void generateCode(const proc_symbolTable*);
	void analyseBlock(Optimizer&);
	void outputIBlocks(ostream& out);
	void outputObjectCode(ostream& out);
	void clear();
};
