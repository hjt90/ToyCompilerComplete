#pragma once
#include "quadruple.h"
#include "Optimizer.h"

class VarInfomation {
public:
	int next;//������Ϣ
	bool active;//��Ծ��Ϣ

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
	map<string, int>varOffset;//�������Ĵ洢λ��
	int top;//��ǰջ��
	list<string>freeReg;//���еļĴ������
	map<string, vector<set<string> > >funcOUTL;//���������л�����ĳ��ڻ�Ծ������
	map<string, vector<set<string> > >funcINL;//���������л��������ڻ�Ծ������
	string nowFunc;//��ǰ�����ĺ���
	vector<IBlock>::iterator nowIBlock;//��ǰ�����Ļ�����
	vector<QuaternaryWithInfo>::iterator nowQuatenary;//��ǰ��������Ԫʽ
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
};