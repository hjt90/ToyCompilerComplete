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
	if (varOffset.find(var) != varOffset.end()) {//如果已经为*iter分配好了存储空间
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

//为引用变量分配寄存器
string ObjectCode::allocateReg() {
	//如果有尚未分配的寄存器，则从中选取一个Ri为所需要的寄存器R
	string ret;
	if (freeReg.size()) {
		ret = freeReg.back();
		freeReg.pop_back();
		return ret;
	}

	/*
	从已分配的寄存器中选取一个Ri为所需要的寄存器R。最好使得Ri满足以下条件：
	占用Ri的变量的值也同时存放在该变量的贮存单元中
	或者在基本块中要在最远的将来才会引用到或不会引用到。
	*/

	const int inf = 1000000;
	int maxNextPos = 0;
	for (map<string, set<string> >::iterator iter = Rvalue.begin(); iter != Rvalue.end(); iter++) {//遍历所有的寄存器
		int nextpos = inf;
		for (set<string>::iterator viter = iter->second.begin(); viter != iter->second.end(); viter++) {//遍历寄存器中储存的变量
			bool inFlag = false;//变量已在其他地方存储的标志
			for (set<string>::iterator aiter = Avalue[*viter].begin(); aiter != Avalue[*viter].end(); aiter++) {//遍历变量的存储位置
				if (*aiter != iter->first) {//如果变量存储在其他地方
					inFlag = true;
					break;
				}
			}
			if (!inFlag) {//如果变量仅存储在寄存器中，就看未来在何处会引用该变量
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
		//对ret的寄存器中保存的变量*iter，他们都将不再存储在ret中
		Avalue[*iter].erase(ret);
		//如果V的地址描述数组AVALUE[V]说V还保存在R之外的其他地方，则不需要生成存数指令
		if (Avalue[*iter].size() > 0) {
			//pass
		}
		//如果V不会在此之后被使用，则不需要生成存数指令
		else {
			bool storeFlag = true;
			vector<QuaternaryWithInfo>::iterator cIter;
			for (cIter = nowQuatenary; cIter != nowIBlock->codes.end(); cIter++) {
				if (cIter->q.arg1 == *iter || cIter->q.arg2 == *iter) {//如果V在本基本块中被引用
					storeFlag = true;
					break;
				}
				if (cIter->q.result == *iter) {//如果V在本基本块中被赋值
					storeFlag = false;
					break;
				}
			}
			if (cIter == nowIBlock->codes.end()) {//如果V在本基本块中未被引用，且也没有被赋值
				int index = nowIBlock - funcIBlocks[nowFunc].begin();
				if (funcOUTL[nowFunc][index].count(*iter) == 1) {//如果此变量是出口之后的活跃变量
					storeFlag = true;
				}
				else {
					storeFlag = false;
				}
			}
			if (storeFlag) {//生成存数指令
				storeVar(ret, *iter);
			}
		}
	}
	Rvalue[ret].clear();//清空ret寄存器中保存的变量

	return ret;
}

//为引用变量分配寄存器
string ObjectCode::allocateReg(string var) {
	if (isNum(var)) {
		string ret = allocateReg();
		objectCodes.push_back(string("addi ") + ret + " $zero " + var);
		return ret;
	}

	for (set<string>::iterator iter = Avalue[var].begin(); iter != Avalue[var].end(); iter++) {
		if ((*iter)[0] == '$') {//如果变量已经保存在某个寄存器中
			return *iter;//直接返回该寄存器
		}
	}

	//如果该变量没有在某个寄存器中
	string ret = allocateReg();
	objectCodes.push_back(string("lw ") + ret + " " + to_string(varOffset[var]) + "($sp)");
	Avalue[var].insert(ret);
	Rvalue[ret].insert(var);
	return ret;
}

//为目标变量分配寄存器
string ObjectCode::getReg() {
	//i: A:=B op C
	//如果B的现行值在某个寄存器Ri中，RVALUE[Ri]中只包含B
	//此外，或者B与A是同一个标识符或者B的现行值在执行四元式A:=B op C之后不会再引用
	//则选取Ri为所需要的寄存器R

	//如果src1不是数字
	if (!isNum(nowQuatenary->q.arg1)) {
		//遍历src1所在的寄存器
		set<string>& src1pos = Avalue[nowQuatenary->q.arg1];
		for (set<string>::iterator iter = src1pos.begin(); iter != src1pos.end(); iter++) {
			if ((*iter)[0] == '$') {
				if (Rvalue[*iter].size() == 1) {//如果该寄存器中值仅仅存有src1
					if (nowQuatenary->q.result == nowQuatenary->q.arg1 || !nowQuatenary->info1.active) {//如果A,B是同一标识符或B以后不活跃
						Avalue[nowQuatenary->q.result].insert(*iter);
						Rvalue[*iter].insert(nowQuatenary->q.result);
						return *iter;
					}
				}
			}
		}
	}

	//为目标变量分配可能不正确
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

		vector<map<string, VarInfomation> > symTables;//每个基本块对应一张符号表
		//初始化符号表
		for (vector<Block>::iterator biter = blocks.begin(); biter != blocks.end(); biter++) {//遍历每一个基本块
			map<string, VarInfomation>symTable;
			for (vector<quadruple>::iterator citer = biter->codes.begin(); citer != biter->codes.end(); citer++) {//遍历基本块中的每个四元式
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
		for (vector<set<string> >::iterator iter = OUTL.begin(); iter != OUTL.end(); iter++, blockIndex++) {//遍历每个基本块的活跃变量表
			for (set<string>::iterator viter = iter->begin(); viter != iter->end(); viter++) {//遍历活跃变量表中的变量
				symTables[blockIndex][*viter] = VarInfomation{ -1,true };
			}

		}

		blockIndex = 0;
		//计算每个四元式的待用信息和活跃信息
		for (vector<IBlock>::iterator ibiter = iBlocks.begin(); ibiter != iBlocks.end(); ibiter++, blockIndex++) {//遍历每一个基本块
			int codeIndex = ibiter->codes.size() - 1;
			for (vector<QuaternaryWithInfo>::reverse_iterator citer = ibiter->codes.rbegin(); citer != ibiter->codes.rend(); citer++, codeIndex--) {//逆序遍历基本块中的代码
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

//基本块出口，将出口活跃变量保存在内存
void ObjectCode::storeOutLiveVar(set<string>& outl) {
	for (set<string>::iterator oiter = outl.begin(); oiter != outl.end(); oiter++) {
		string reg;//活跃变量所在的寄存器名称
		bool inFlag = false;//活跃变量在内存中的标志
		for (set<string>::iterator aiter = Avalue[*oiter].begin(); aiter != Avalue[*oiter].end(); aiter++) {
			if ((*aiter)[0] != '$') {//该活跃变量已经存储在内存中
				inFlag = true;
				break;
			}
			else {
				reg = *aiter;
			}
		}
		if (!inFlag) {//如果该活跃变量不在内存中，则将reg中的var变量存入内存
			storeVar(reg, *oiter);
		}
	}
}

void ObjectCode::generateCodeForQuatenary(int nowBaseBlockIndex, int& arg_num, int& par_num, list<pair<string, bool> >& par_list)
{
}

void ObjectCode::generateCodeForBaseBlocks(int nowBaseBlockIndex)
{
	int arg_num = 0;//par的实参个数
	int par_num = 0;//get的形参个数
	list<pair<string, bool> > par_list;//函数调用用到的实参集list<实参名,是否活跃>

	if (nowFunc == "program") {
		int a = 1;
	}

	Avalue.clear();
	Rvalue.clear();
	set<string>& inl = funcINL[nowFunc][nowBaseBlockIndex];
	for (set<string>::iterator iter = inl.begin(); iter != inl.end(); iter++) {
		Avalue[*iter].insert(*iter);
	}

	//初始化空闲寄存器
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
			objectCodes.push_back("sw $ra 4($sp)");//把返回地址压栈
			top = 8;
		}
	}

	for (vector<QuaternaryWithInfo>::iterator cIter = nowIBlock->codes.begin(); cIter != nowIBlock->codes.end(); cIter++) {//对基本块内的每一条语句
		nowQuatenary = cIter;
		//如果是基本块的最后一条语句
		if (cIter + 1 == nowIBlock->codes.end()) {
			//如果最后一条语句是控制语句，则先将出口活跃变量保存，再进行跳转(j,call,return)
			if (isControlOp(Oper2string(cIter->q.Op))) {
				storeOutLiveVar(funcOUTL[nowFunc][nowBaseBlockIndex]);
				generateCodeForQuatenary(nowBaseBlockIndex, arg_num, par_num, par_list);
			}
			//如果最后一条语句不是控制语句（是赋值语句），则先计算，再将出口活跃变量保存
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
	for (vector<IBlock>::iterator iter = iBlocks.begin(); iter != iBlocks.end(); iter++) {//对每一个基本块
		nowIBlock = iter;
		generateCodeForBaseBlocks(nowIBlock - iBlocks.begin());
	}
}

void ObjectCode::generateArrayData(const proc_symbolTable* ptr)
{
	objectCodes.push_back(".data");

	for (const auto& item : ptr->itemTable)	//全局数组
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

	for (const auto& func : ptr->functionTable)	//局部数组
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
	for (map<string, vector<IBlock> >::iterator fiter = funcIBlocks.begin(); fiter != funcIBlocks.end(); fiter++) {//对每一个函数块
		generateCodeForFuncBlocks(fiter);
	}
	objectCodes.push_back("end:");
}
