#include "objectCode.h"
#include<iostream>

static bool isVar(string name) {
	return isalpha(name[0]);
}

static bool isNum(string name) {
	return isdigit(name[0]);
}

static bool isControlOp(string op) {
	if (op[0] == 'J' || op == "Call" || op == "Return" || op == "Get") {
		return true;
	}
	return false;
}

VarInfomation::VarInfomation(int next, bool active) {
	this->next = next;
	this->active = active;
}

VarInfomation::VarInfomation(const VarInfomation& other) {
	this->active = other.active;
	this->next = other.next;
}

VarInfomation::VarInfomation() :next(0), active(false)
{

}

void VarInfomation::output(ostream& out) {
	out << "(";
	if (next == -1)
		out << "^";
	else
		out << next;
	out << ",";
	if (active)
		out << "y";
	else
		out << "^";

	out << ")";
}

QuaternaryWithInfo::QuaternaryWithInfo(quadruple q, VarInfomation info1, VarInfomation info2, VarInfomation info3) {
	this->q = q;
	this->info1 = info1;
	this->info2 = info2;
	this->info3 = info3;
}

void QuaternaryWithInfo::output(ostream& out) {
	out << "(" << Oper2string(q.Op) << "," << q.arg1 << "," << q.arg2 << "," << q.result << ")";
	info1.output(out);
	info2.output(out);
	info3.output(out);
}

ObjectCode::ObjectCode() :top(0)
{
}

void ObjectCode::storeVar(string reg, string var) {
	if (varOffset.find(var) != varOffset.end()) {//����Ѿ�Ϊ*iter������˴洢�ռ�
		objectCodes.push_back(string("sw ") + reg + " " + to_string(varOffset[var]) + "($sp)");
	}
	else {
		varOffset[var] = top;
		top += 4;
		objectCodes.push_back(string("sw ") + reg + " " + to_string(varOffset[var]) + "($sp)");
	}
	Avalue[var].insert(var);
}

void ObjectCode::releaseVar(string var) {
	for (set<string>::iterator iter = Avalue[var].begin(); iter != Avalue[var].end(); iter++) {
		if ((*iter)[0] == '$') {
			Rvalue[*iter].erase(var);
			if (Rvalue[*iter].size() == 0 && (*iter)[1] == 's') {
				freeReg.push_back(*iter);
			}
		}
	}
	Avalue[var].clear();
}

//Ϊ���ñ�������Ĵ���
string ObjectCode::allocateReg() {
	//�������δ����ļĴ����������ѡȡһ��RiΪ����Ҫ�ļĴ���R
	string ret;
	if (freeReg.size()) {
		ret = freeReg.back();
		freeReg.pop_back();
		return ret;
	}

	/*
	���ѷ���ļĴ�����ѡȡһ��RiΪ����Ҫ�ļĴ���R�����ʹ��Ri��������������
	ռ��Ri�ı�����ֵҲͬʱ����ڸñ��������浥Ԫ��
	�����ڻ�������Ҫ����Զ�Ľ����Ż����õ��򲻻����õ���
	*/

	const int inf = 1000000;
	int maxNextPos = 0;
	for (map<string, set<string> >::iterator iter = Rvalue.begin(); iter != Rvalue.end(); iter++) {//�������еļĴ���
		int nextpos = inf;
		for (set<string>::iterator viter = iter->second.begin(); viter != iter->second.end(); viter++) {//�����Ĵ����д���ı���
			bool inFlag = false;//�������������ط��洢�ı�־
			for (set<string>::iterator aiter = Avalue[*viter].begin(); aiter != Avalue[*viter].end(); aiter++) {//���������Ĵ洢λ��
				if (*aiter != iter->first) {//��������洢�������ط�
					inFlag = true;
					break;
				}
			}
			if (!inFlag) {//����������洢�ڼĴ����У��Ϳ�δ���ںδ������øñ���
				for (vector<QuaternaryWithInfo>::iterator cIter = nowQuatenary; cIter != nowIBlock->codes.end(); cIter++) {
					if (*viter == cIter->q.arg1 || *viter == cIter->q.arg2) {
						nextpos = cIter - nowQuatenary;
					}
					else if (*viter == cIter->q.result) {
						break;
					}
				}
			}
		}
		if (nextpos == inf) {
			ret = iter->first;
			break;
		}
		else if (nextpos > maxNextPos) {
			maxNextPos = nextpos;
			ret = iter->first;
		}
	}

	for (set<string>::iterator iter = Rvalue[ret].begin(); iter != Rvalue[ret].end(); iter++) {
		//��ret�ļĴ����б���ı���*iter�����Ƕ������ٴ洢��ret��
		Avalue[*iter].erase(ret);
		//���V�ĵ�ַ��������AVALUE[V]˵V��������R֮��������ط�������Ҫ���ɴ���ָ��
		if (Avalue[*iter].size() > 0) {
			//pass
		}
		//���V�����ڴ�֮��ʹ�ã�����Ҫ���ɴ���ָ��
		else {
			bool storeFlag = true;
			vector<QuaternaryWithInfo>::iterator cIter;
			for (cIter = nowQuatenary; cIter != nowIBlock->codes.end(); cIter++) {
				if (cIter->q.arg1 == *iter || cIter->q.arg2 == *iter) {//���V�ڱ��������б�����
					storeFlag = true;
					break;
				}
				if (cIter->q.result == *iter) {//���V�ڱ��������б���ֵ
					storeFlag = false;
					break;
				}
			}
			if (cIter == nowIBlock->codes.end()) {//���V�ڱ���������δ�����ã���Ҳû�б���ֵ
				int index = nowIBlock - funcIBlocks[nowFunc].begin();
				if (funcOUTL[nowFunc][index].count(*iter) == 1) {//����˱����ǳ���֮��Ļ�Ծ����
					storeFlag = true;
				}
				else {
					storeFlag = false;
				}
			}
			if (storeFlag) {//���ɴ���ָ��
				storeVar(ret, *iter);
			}
		}
	}
	Rvalue[ret].clear();//���ret�Ĵ����б���ı���

	return ret;
}

//Ϊ���ñ�������Ĵ���
string ObjectCode::allocateReg(string var) {
	if (isNum(var)) {
		string ret = allocateReg();
		objectCodes.push_back(string("addi ") + ret + " $zero " + var);
		return ret;
	}

	for (set<string>::iterator iter = Avalue[var].begin(); iter != Avalue[var].end(); iter++) {
		if ((*iter)[0] == '$') {//��������Ѿ�������ĳ���Ĵ�����
			return *iter;//ֱ�ӷ��ظüĴ���
		}
	}

	//����ñ���û����ĳ���Ĵ�����
	string ret = allocateReg();
	objectCodes.push_back(string("lw ") + ret + " " + to_string(varOffset[var]) + "($sp)");
	Avalue[var].insert(ret);
	Rvalue[ret].insert(var);
	return ret;
}

//ΪĿ���������Ĵ���
string ObjectCode::getReg() {
	//i: A:=B op C
	//���B������ֵ��ĳ���Ĵ���Ri�У�RVALUE[Ri]��ֻ����B
	//���⣬����B��A��ͬһ����ʶ������B������ֵ��ִ����ԪʽA:=B op C֮�󲻻�������
	//��ѡȡRiΪ����Ҫ�ļĴ���R

	//���src1��������
	if (!isNum(nowQuatenary->q.arg1)) {
		//����src1���ڵļĴ���
		set<string>& src1pos = Avalue[nowQuatenary->q.arg1];
		for (set<string>::iterator iter = src1pos.begin(); iter != src1pos.end(); iter++) {
			if ((*iter)[0] == '$') {
				if (Rvalue[*iter].size() == 1) {//����üĴ�����ֵ��������src1
					if (nowQuatenary->q.result == nowQuatenary->q.arg1 || !nowQuatenary->info1.active) {//���A,B��ͬһ��ʶ����B�Ժ󲻻�Ծ
						Avalue[nowQuatenary->q.result].insert(*iter);
						Rvalue[*iter].insert(nowQuatenary->q.result);
						return *iter;
					}
				}
			}
		}
	}

	//ΪĿ�����������ܲ���ȷ
	//return allocateReg(nowQuatenary->q.des);
	string ret = allocateReg();
	Avalue[nowQuatenary->q.result].insert(ret);
	Rvalue[ret].insert(nowQuatenary->q.result);
	return ret;
}

void ObjectCode::analyseBlock(Optimizer& optim) {
	map<string, vector<Block> >& funcBlocks = optim.funcBlocks;
	const map<string, vector<set<string> > >& funcOUTL = optim.funcOUTL;

	for (map<string, vector<Block> >::iterator fbiter = funcBlocks.begin(); fbiter != funcBlocks.end(); fbiter++) {
		vector<IBlock> iBlocks;
		vector<Block>& blocks = fbiter->second;
		vector<set<string> >OUTL = funcOUTL.at(fbiter->first);

		for (vector<Block>::iterator iter = blocks.begin(); iter != blocks.end(); iter++) {
			IBlock iBlock;
			iBlock.next1 = iter->next1;
			iBlock.next2 = iter->next2;
			iBlock.name = iter->name;
			for (vector<quadruple>::iterator qIter = iter->codes.begin(); qIter != iter->codes.end(); qIter++) {
				iBlock.codes.push_back(QuaternaryWithInfo(*qIter, VarInfomation(-1, false), VarInfomation(-1, false), VarInfomation(-1, false)));
			}
			iBlocks.push_back(iBlock);
		}

		vector<map<string, VarInfomation> > symTables;//ÿ���������Ӧһ�ŷ��ű�
		//��ʼ�����ű�
		for (vector<Block>::iterator biter = blocks.begin(); biter != blocks.end(); biter++) {//����ÿһ��������
			map<string, VarInfomation>symTable;
			for (vector<quadruple>::iterator citer = biter->codes.begin(); citer != biter->codes.end(); citer++) {//�����������е�ÿ����Ԫʽ
				if (citer->Op == Oper::J) {
					//pass
				}
				else if (citer->Op == Oper::Call)
				{
					if (isVar(citer->result)) {
						symTable[citer->result] = VarInfomation{ -1,false };
					}
				}
				else if (citer->Op == Oper::Jeq || citer->Op == Oper::Jge || citer->Op == Oper::Jgt || citer->Op == Oper::Jle
					|| citer->Op == Oper::Jlt || citer->Op == Oper::Jne)
				{
					if (isVar(citer->arg1)) {
						symTable[citer->arg1] = VarInfomation{ -1,false };
					}
					if (isVar(citer->arg2)) {
						symTable[citer->arg2] = VarInfomation{ -1,false };
					}
				}
				else if (citer->Op == Oper::Return)
				{
					if (isVar(citer->arg1)) {
						symTable[citer->arg1] = VarInfomation{ -1,false };
					}
				}
				else if (citer->Op == Oper::ArrayAssign)
				{
					if (isVar(citer->arg1)) {
						symTable[citer->arg1] = VarInfomation{ -1,false };
					}
					if (isVar(citer->result)) {
						symTable[citer->result] = VarInfomation{ -1,false };
					}
				}
				else if (citer->Op == Oper::AssignArray)
				{
					if (isVar(citer->arg2)) {
						symTable[citer->arg2] = VarInfomation{ -1,false };
					}
					if (isVar(citer->result)) {
						symTable[citer->result] = VarInfomation{ -1,false };
					}
				}
				else {
					if (isVar(citer->arg1)) {
						symTable[citer->arg1] = VarInfomation{ -1,false };
					}
					if (isVar(citer->arg2)) {
						symTable[citer->arg2] = VarInfomation{ -1,false };
					}
					if (isVar(citer->result)) {
						symTable[citer->result] = VarInfomation{ -1,false };
					}
				}
			}
			symTables.push_back(symTable);
		}
		int blockIndex = 0;
		for (vector<set<string> >::iterator iter = OUTL.begin(); iter != OUTL.end(); iter++, blockIndex++) {//����ÿ��������Ļ�Ծ������
			for (set<string>::iterator viter = iter->begin(); viter != iter->end(); viter++) {//������Ծ�������еı���
				symTables[blockIndex][*viter] = VarInfomation{ -1,true };
			}

		}

		blockIndex = 0;
		//����ÿ����Ԫʽ�Ĵ�����Ϣ�ͻ�Ծ��Ϣ
		for (vector<IBlock>::iterator ibiter = iBlocks.begin(); ibiter != iBlocks.end(); ibiter++, blockIndex++) {//����ÿһ��������
			int codeIndex = ibiter->codes.size() - 1;
			for (vector<QuaternaryWithInfo>::reverse_iterator citer = ibiter->codes.rbegin(); citer != ibiter->codes.rend(); citer++, codeIndex--) {//��������������еĴ���
				if (citer->q.Op == Oper::J) {
					//pass
				}
				else if (citer->q.Op == Oper::Call)
				{
					if (isVar(citer->q.result)) {
						citer->info3 = symTables[blockIndex][citer->q.result];
						symTables[blockIndex][citer->q.result] = VarInfomation{ -1,false };
					}
				}
				else if (citer->q.Op == Oper::Jeq || citer->q.Op == Oper::Jge || citer->q.Op == Oper::Jgt || citer->q.Op == Oper::Jle
					|| citer->q.Op == Oper::Jlt || citer->q.Op == Oper::Jne)
				{
					if (isVar(citer->q.arg1)) {
						citer->info1 = symTables[blockIndex][citer->q.arg1];
						symTables[blockIndex][citer->q.arg1] = VarInfomation{ codeIndex,true };
					}
					if (isVar(citer->q.arg2)) {
						citer->info2 = symTables[blockIndex][citer->q.arg2];
						symTables[blockIndex][citer->q.arg2] = VarInfomation{ codeIndex,true };
					}
				}
				else if (citer->q.Op == Oper::Return)
				{
					if (isVar(citer->q.arg1)) {
						citer->info1 = symTables[blockIndex][citer->q.arg1];
						symTables[blockIndex][citer->q.arg1] = VarInfomation{ codeIndex,true };
					}
				}
				else if (citer->q.Op == Oper::ArrayAssign)
				{
					if (isVar(citer->q.arg1)) {
						citer->info1 = symTables[blockIndex][citer->q.arg1];
						symTables[blockIndex][citer->q.arg1] = VarInfomation{ codeIndex,true };
					}
					if (isVar(citer->q.result)) {
						citer->info3 = symTables[blockIndex][citer->q.result];
						symTables[blockIndex][citer->q.result] = VarInfomation{ -1,false };
					}
				}
				else if (citer->q.Op == Oper::AssignArray)
				{
					if (isVar(citer->q.arg2)) {
						citer->info2 = symTables[blockIndex][citer->q.arg2];
						symTables[blockIndex][citer->q.arg2] = VarInfomation{ codeIndex,true };
					}
					if (isVar(citer->q.result)) {
						citer->info3 = symTables[blockIndex][citer->q.result];
						symTables[blockIndex][citer->q.result] = VarInfomation{ -1,false };
					}
				}
				else {
					if (isVar(citer->q.arg1)) {
						citer->info1 = symTables[blockIndex][citer->q.arg1];
						symTables[blockIndex][citer->q.arg1] = VarInfomation{ codeIndex,true };
					}
					if (isVar(citer->q.arg2)) {
						citer->info2 = symTables[blockIndex][citer->q.arg2];
						symTables[blockIndex][citer->q.arg2] = VarInfomation{ codeIndex,true };
					}
					if (isVar(citer->q.result)) {
						citer->info3 = symTables[blockIndex][citer->q.result];
						symTables[blockIndex][citer->q.result] = VarInfomation{ -1,false };
					}
				}
			}
		}

		funcIBlocks[fbiter->first] = iBlocks;
	}
}

void ObjectCode::outputIBlocks(ostream& out) {
	for (map<string, vector<IBlock> >::iterator iter = funcIBlocks.begin(); iter != funcIBlocks.end(); iter++) {
		out << "[" << iter->first << "]" << endl;
		for (vector<IBlock>::iterator bIter = iter->second.begin(); bIter != iter->second.end(); bIter++) {
			out << bIter->name << ":" << endl;
			for (vector<QuaternaryWithInfo>::iterator cIter = bIter->codes.begin(); cIter != bIter->codes.end(); cIter++) {
				out << "    ";
				cIter->output(out);
				out << endl;
			}
			out << "    " << "next1 = " << bIter->next1 << endl;
			out << "    " << "next2 = " << bIter->next2 << endl;
		}
		out << endl;
	}
}

void ObjectCode::outputObjectCode(ostream& out) {
	for (vector<string>::iterator iter = objectCodes.begin(); iter != objectCodes.end(); iter++) {
		out << *iter << endl;
	}
}

//��������ڣ������ڻ�Ծ�����������ڴ�
void ObjectCode::storeOutLiveVar(set<string>& outl) {
	for (set<string>::iterator oiter = outl.begin(); oiter != outl.end(); oiter++) {
		string reg;//��Ծ�������ڵļĴ�������
		bool inFlag = false;//��Ծ�������ڴ��еı�־
		for (set<string>::iterator aiter = Avalue[*oiter].begin(); aiter != Avalue[*oiter].end(); aiter++) {
			if ((*aiter)[0] != '$') {//�û�Ծ�����Ѿ��洢���ڴ���
				inFlag = true;
				break;
			}
			else {
				reg = *aiter;
			}
		}
		if (!inFlag) {//����û�Ծ���������ڴ��У���reg�е�var���������ڴ�
			storeVar(reg, *oiter);
		}
	}
}

void ObjectCode::generateCodeForQuatenary(int nowBaseBlockIndex, int& arg_num, int& par_num, list<pair<string, bool> >& par_list)
{
}

void ObjectCode::generateCodeForBaseBlocks(int nowBaseBlockIndex)
{
	int arg_num = 0;//par��ʵ�θ���
	int par_num = 0;//get���βθ���
	list<pair<string, bool> > par_list;//���������õ���ʵ�μ�list<ʵ����,�Ƿ��Ծ>

	if (nowFunc == "program") {
		int a = 1;
	}

	Avalue.clear();
	Rvalue.clear();
	set<string>& inl = funcINL[nowFunc][nowBaseBlockIndex];
	for (set<string>::iterator iter = inl.begin(); iter != inl.end(); iter++) {
		Avalue[*iter].insert(*iter);
	}

	//��ʼ�����мĴ���
	freeReg.clear();
	for (int i = 0; i <= 7; i++) {
		freeReg.push_back(string("$s") + to_string(i));
	}

	objectCodes.push_back(nowIBlock->name + ":");
	if (nowBaseBlockIndex == 0) {
		if (nowFunc == "main") {
			top = 8;
		}
		else {
			objectCodes.push_back("sw $ra 4($sp)");//�ѷ��ص�ַѹջ
			top = 8;
		}
	}

	for (vector<QuaternaryWithInfo>::iterator cIter = nowIBlock->codes.begin(); cIter != nowIBlock->codes.end(); cIter++) {//�Ի������ڵ�ÿһ�����
		nowQuatenary = cIter;
		//����ǻ���������һ�����
		if (cIter + 1 == nowIBlock->codes.end()) {
			//������һ������ǿ�����䣬���Ƚ����ڻ�Ծ�������棬�ٽ�����ת(j,call,return)
			if (isControlOp(Oper2string(cIter->q.Op))) {
				storeOutLiveVar(funcOUTL[nowFunc][nowBaseBlockIndex]);
				generateCodeForQuatenary(nowBaseBlockIndex, arg_num, par_num, par_list);
			}
			//������һ����䲻�ǿ�����䣨�Ǹ�ֵ��䣩�����ȼ��㣬�ٽ����ڻ�Ծ��������
			else {
				generateCodeForQuatenary(nowBaseBlockIndex, arg_num, par_num, par_list);
				storeOutLiveVar(funcOUTL[nowFunc][nowBaseBlockIndex]);
			}
		}
		else {
			generateCodeForQuatenary(nowBaseBlockIndex, arg_num, par_num, par_list);
		}

	}
}

void ObjectCode::generateCodeForFuncBlocks(map<string, vector<IBlock> >::iterator& fiter)
{
	varOffset.clear();
	nowFunc = fiter->first;
	vector<IBlock>& iBlocks = fiter->second;
	for (vector<IBlock>::iterator iter = iBlocks.begin(); iter != iBlocks.end(); iter++) {//��ÿһ��������
		nowIBlock = iter;
		generateCodeForBaseBlocks(nowIBlock - iBlocks.begin());
	}
}

void ObjectCode::generateArrayData(const proc_symbolTable* ptr)
{
	objectCodes.push_back(".data");

	for (const auto& item : ptr->itemTable)	//ȫ������
	{
		if (item.second.type == symbolType::Array)
		{
			objectCodes.push_back(item.second.gobalname + string(":"));
			int len = 4;
			for (const auto& arr : item.second.array)
				len *= arr;
			objectCodes.push_back(string(".space ") + to_string(len));
		}
	}

	for (const auto& func : ptr->functionTable)	//�ֲ�����
	{
		for (const auto& item : func.second->itemTable)
		{
			if (item.second.type == symbolType::Array)
			{
				objectCodes.push_back(item.second.gobalname + string(":"));
				int len = 4;
				for (const auto& arr : item.second.array)
					len *= arr;
				objectCodes.push_back(string(".space ") + to_string(len));
			}
		}
	}
}

void ObjectCode::generateCode(const proc_symbolTable* ptr)
{
	this->generateArrayData(ptr);
	objectCodes.push_back(".text");
	objectCodes.push_back("lui $sp,0x1001");
	objectCodes.push_back("j main");
	for (map<string, vector<IBlock> >::iterator fiter = funcIBlocks.begin(); fiter != funcIBlocks.end(); fiter++) {//��ÿһ��������
		generateCodeForFuncBlocks(fiter);
	}
	objectCodes.push_back("end:");
}
