#include "Optimizer.h"
#include<queue>
#include<iostream>

static bool isVar(string name) {
	return name.size() > 0 && isalpha(name[0]);
}

static bool isNum(string name) {
	return name.size() > 0 && isdigit(name[0]);
}

NewLabeler::NewLabeler() :index(1), tmp(0)
{}

string NewLabeler::newLabel() {
	return string("Label") + to_string(index++);
}

string NewLabeler::newTmp() {
	return string("Tmp") + to_string(tmp++);
}

void Optimizer::divideBlocks(const parsing& pars, const proc_symbolTable* ptr) {
	vector<pair<int, string> > funcEnter = pars.p_symbolTable->getFuncEnter();
	this->code = pars.mid_code;

	for (vector<pair<int, string> >::iterator iter = funcEnter.begin(); iter != funcEnter.end(); iter++) { //每一个函数块
		vector<Block>blocks;
		priority_queue<int, vector<int>, greater<int> >block_enter; //所有基本块的入口位置
		block_enter.push(iter->first);

		int endIndex = (iter + 1 == funcEnter.end()) ? code.nextquad : (iter + 1)->first;
		for (int i = iter->first; i != endIndex; i++) {
			if (code.code[i - CODE_START_POS].Op == Oper::J || code.code[i - CODE_START_POS].Op == Oper::Jeq || code.code[i - CODE_START_POS].Op == Oper::Jge ||
				code.code[i - CODE_START_POS].Op == Oper::Jgt || code.code[i - CODE_START_POS].Op == Oper::Jle
				|| code.code[i - CODE_START_POS].Op == Oper::Jlt || code.code[i - CODE_START_POS].Op == Oper::Jne)
			{
				if (code.code[i - CODE_START_POS].Op == Oper::J)
				{
					block_enter.push(atoi(code.code[i - CODE_START_POS].result.c_str()));
				}
				else
				{
					if (i + 1 < endIndex) {
						block_enter.push(i + 1);
					}
					block_enter.push(atoi(code.code[i - CODE_START_POS].result.c_str()));
				}
			}
			else if (code.code[i - CODE_START_POS].Op == Oper::Return) {
				if (i + 1 < endIndex) {
					block_enter.push(i + 1);
				}
			}
		}

		//划分基本块	
		Block block;
		map<int, string>labelEnter; //入口点和标签的对应关系
		map<int, int>enter_block; //建立入口点和block的对应关系
		int firstFlag = true; //函数块第一块标记，该块命名为函数名
		int enter;
		int lastEnter = block_enter.top();
		block_enter.pop();
		while (!block_enter.empty()) {
			enter = block_enter.top();
			block_enter.pop();

			if (enter == lastEnter) {
				continue;
			}

			for (int i = lastEnter; i != enter; i++) {
				block.codes.push_back(code.code[i - CODE_START_POS]);
			}

			if (!firstFlag) { //该基本块不是函数块的第一块基本块
				block.name = label.newLabel();
				labelEnter[lastEnter] = block.name;
			}
			else { //该基本块是函数块的第一块基本块
				block.name = iter->second;
				firstFlag = false;
			}

			enter_block[lastEnter] = blocks.size();
			blocks.push_back(block);
			lastEnter = enter;
			block.codes.clear();
		}
		if (!firstFlag) { //该基本块不是函数块的第一块基本块
			block.name = label.newLabel();
			labelEnter[lastEnter] = block.name;
		}
		else { //该基本块是函数块的第一块基本块
			block.name = iter->second;
			firstFlag = false;
		}
		if (iter + 1 != funcEnter.end()) { //在两个函数的起点之间
			for (int i = lastEnter; i != (iter + 1)->first; i++) {
				block.codes.push_back(code.code[i - CODE_START_POS]);
			}
		}
		else {//在最后一个函数至中间代码末尾
			for (int i = lastEnter; i != code.nextquad; i++) {
				block.codes.push_back(code.code[i - CODE_START_POS]);
			}
		}
		enter_block[lastEnter] = blocks.size();
		blocks.push_back(block);

		int blockIndex = 0;
		for (vector<Block>::iterator bIter = blocks.begin(); bIter != blocks.end(); bIter++, blockIndex++) {
			vector<quadruple>::reverse_iterator lastCode = bIter->codes.rbegin();
			if (lastCode->Op == Oper::J || lastCode->Op == Oper::Jeq || lastCode->Op == Oper::Jge || lastCode->Op == Oper::Jgt
				|| lastCode->Op == Oper::Jle || lastCode->Op == Oper::Jlt || lastCode->Op == Oper::Jne)
			{
				if (lastCode->Op == Oper::J) {
					bIter->next1 = enter_block[atoi(lastCode->result.c_str())];
					bIter->next2 = -1;
				}
				else {
					bIter->next1 = blockIndex + 1;
					bIter->next2 = enter_block[atoi(lastCode->result.c_str())];
					bIter->next2 = bIter->next1 == bIter->next2 ? -1 : bIter->next2;
				}
				lastCode->result = labelEnter[atoi(lastCode->result.c_str())];
			}
			else if (lastCode->Op == Oper::Return) {
				bIter->next1 = bIter->next2 = -1;
			}
			else {
				bIter->next1 = blockIndex + 1;
				bIter->next2 = -1;
			}
		}
		funcBlocks[iter->second] = blocks;
	}

	for (map<string, vector<Block> >::iterator fbiter = funcBlocks.begin(); fbiter != funcBlocks.end(); fbiter++)
	{
		const std::vector<symbolTableItem>& parms = ptr->functionTable.at(fbiter->first)->parm;
		Block& block = fbiter->second[0];
		vector<quadruple> get_parm;

		if (fbiter->first != string("main"))
		{
			for (const auto& parm : parms)
			{
				if (parm.type != symbolType::Array)
					get_parm.push_back({ Oper::Get,string(""),string("") ,parm.gobalname });
			}
		}
		block.codes.insert(block.codes.begin(), get_parm.begin(), get_parm.end());
	}
}

void Optimizer::outputBlocks(ostream& out)
{
	for (map<string, vector<Block> >::iterator iter = funcBlocks.begin(); iter != funcBlocks.end(); iter++) {
		out << "[" << iter->first << "]" << endl;
		for (vector<Block>::iterator bIter = iter->second.begin(); bIter != iter->second.end(); bIter++) {
			out << bIter->name << ":" << endl;
			for (vector<quadruple>::iterator cIter = bIter->codes.begin(); cIter != bIter->codes.end(); cIter++) {
				out << "    " << "(" << Oper2string(cIter->Op) << "," << cIter->arg1 << "," << cIter->arg2 << "," << cIter->result << ")" << endl;
			}
			out << "    " << "next1 = " << bIter->next1 << endl;
			out << "    " << "next2 = " << bIter->next2 << endl;
		}
		out << endl;
	}
}

/***
*     活跃变量分析
* 参考：https://blog.csdn.net/weixin_42815609/article/details/108022060
*/
void Optimizer::init_INOUTL()
{
	for (map<string, vector<Block> >::iterator fbiter = this->funcBlocks.begin(); fbiter != this->funcBlocks.end(); fbiter++) {
		vector<Block>& blocks = fbiter->second;
		vector<set<string> >INL, OUTL, DEF, USE;

		//活跃变量的数据流方程
		for (vector<Block>::iterator biter = blocks.begin(); biter != blocks.end(); biter++) {
			set<string>def, use;
			for (vector<quadruple>::iterator citer = biter->codes.begin(); citer != biter->codes.end(); citer++) {
				if (citer->Op == Oper::ArrayAssign) {
					if (isVar(citer->arg1) && def.count(citer->arg1) == 0) {//如果源操作数1还没有被定值
						use.insert(citer->arg1);
					}
					if (isVar(citer->arg2) && def.count(citer->arg2) == 0) {//如果源操作数2还没有被定值
						use.insert(citer->arg2);
					}
					if (isVar(citer->result) && use.count(citer->result) == 0) {//如果目的操作数还没有被引用
						def.insert(citer->result);
					}
				}
				else if (citer->Op == Oper::Get) {
					if (isVar(citer->result) && use.count(citer->result) == 0) {//如果目的操作数还没有被引用
						def.insert(citer->result);
					}
				}
				else if (citer->Op == Oper::AssignArray) {
					if (isVar(citer->arg2) && def.count(citer->arg2) == 0) {//如果源操作数2还没有被定值
						use.insert(citer->arg2);
					}
					if (isVar(citer->result) && use.count(citer->result) == 0) {//如果目的操作数还没有被引用
						def.insert(citer->result);
					}
				}
				else if (citer->Op == Oper::Call) {
					if (isVar(citer->result) && use.count(citer->result) == 0) {//如果目的操作数还没有被引用
						def.insert(citer->result);
					}
				}
				else if (citer->Op == Oper::Parm || citer->Op == Oper::Return) {
					if (isVar(citer->arg1) && def.count(citer->arg1) == 0) {//如果源操作数1还没有被定值
						use.insert(citer->arg1);
					}
				}
				else if (citer->Op == Oper::J)
				{
					//pass
				}
				else if (citer->Op == Oper::Jeq || citer->Op == Oper::Jge || citer->Op == Oper::Jgt || citer->Op == Oper::Jle
					|| citer->Op == Oper::Jlt || citer->Op == Oper::Jne)
				{
					if (isVar(citer->arg1) && def.count(citer->arg1) == 0) {//如果源操作数1还没有被定值
						use.insert(citer->arg1);
					}
					if (isVar(citer->arg2) && def.count(citer->arg2) == 0) {//如果源操作数2还没有被定值
						use.insert(citer->arg2);
					}
				}
				else {
					if (isVar(citer->arg1) && def.count(citer->arg1) == 0) {//如果源操作数1还没有被定值
						use.insert(citer->arg1);
					}
					if (isVar(citer->arg2) && def.count(citer->arg2) == 0) {//如果源操作数2还没有被定值
						use.insert(citer->arg2);
					}
					if (isVar(citer->result) && use.count(citer->result) == 0) {//如果目的操作数还没有被引用
						def.insert(citer->result);
					}
				}
			}
			INL.push_back(use);
			DEF.push_back(def);
			USE.push_back(use);
			if (biter->codes.back().Op == Oper::Return)
				OUTL.push_back(set<string>({ biter->codes.back().arg1 }));
			else
				OUTL.push_back(set<string>());
		}

		//确定INL，OUTL
		bool change = true;
		while (change) {
			change = false;
			int blockIndex = 0;
			for (vector<Block>::iterator biter = blocks.begin(); biter != blocks.end(); biter++, blockIndex++) {
				int next1 = biter->next1;
				int next2 = biter->next2;
				if (next1 != -1) {
					for (set<string>::iterator inlIter = INL[next1].begin(); inlIter != INL[next1].end(); inlIter++) {
						if (OUTL[blockIndex].insert(*inlIter).second == true) {
							if (DEF[blockIndex].count(*inlIter) == 0) {
								INL[blockIndex].insert(*inlIter);
							}
							change = true;
						}
					}
				}
				if (next2 != -1) {
					for (set<string>::iterator inlIter = INL[next2].begin(); inlIter != INL[next2].end(); inlIter++) {
						if (OUTL[blockIndex].insert(*inlIter).second == true) {
							if (DEF[blockIndex].count(*inlIter) == 0) {
								INL[blockIndex].insert(*inlIter);
							}
							change = true;
						}
					}
				}
			}
		}
		funcOUTL[fbiter->first] = OUTL;
		funcINL[fbiter->first] = INL;
	}
}

void Optimizer::init_DAGs()
{
	for (auto& func : this->funcBlocks)
	{
		vector<Block_DAG> DAGs = vector<Block_DAG>();
		for (auto& block : func.second)
		{
			this->preparing(block);
			DAGs.push_back(this->geneDAG(block));
		}
		this->DAGBlocks[func.first] = DAGs;
	}
}

void Optimizer::optimizer_Blocks()
{
	for (auto& func : this->funcBlocks)
	{
		for (int i = 0; i < func.second.size(); i++)
		{
			func.second[i] = this->DAG2block(this->DAGBlocks[func.first][i], this->funcBlocks[func.first][i], this->funcOUTL[func.first][i]);
		}
	}
}

void Optimizer::preparing(Block& block)
{
	int index = 0;
	for (auto& code : block.codes)
	{
		if (code.Op == Oper::Parm)
			code.result = code.result + to_string(index++);
	}
}

vector<DAGitem> Optimizer::geneDAG(const Block& block)
{
	vector<DAGitem> DAG;
	int ArrayAss = 0;
	for (const auto& code : block.codes)
	{
		int oper_type;
		if (code.Op == Oper::J || code.Op == Oper::Get)
			oper_type = -1;
		else if (code.Op == Oper::Parm || code.Op == Oper::Call || code.Op == Oper::Return) //Call应该是一个多子节点的数组
			oper_type = 1;
		else if (code.Op == Oper::Jeq || code.Op == Oper::Jge || code.Op == Oper::Jgt || code.Op == Oper::AssignArray
			|| code.Op == Oper::Jle || code.Op == Oper::Jlt || code.Op == Oper::Jne || code.Op == Oper::Plus ||
			code.Op == Oper::Divide || code.Op == Oper::Multiply || code.Op == Oper::Minus)
			oper_type = 2;
		else if (code.Op == Oper::Assign)
			oper_type = 0;
		else if (code.Op == Oper::ArrayAssign)
			oper_type = 3;
		else
			oper_type = 2;

		string B = code.arg1;
		string C = code.arg2;
		string A = code.result;

		//不做DAG转化的中间代码
		if (oper_type == -1)
		{
			DAGitem newDAG;
			newDAG.isremain = true;
			newDAG.isleaf = false;
			newDAG.code = code;
			newDAG.op = Oper2string(code.Op);
			DAG.push_back(newDAG);
			continue;
		}

		//对该中间代码生成DAG
		int state = 1;
		int n;
		int A_no;
		bool new_A;
		int B_no;
		bool new_B;
		int C_no;
		bool new_C;
		while (state > 0)
		{
			switch (state)
			{
				case 1:
				{
					//在已有DAG节点中寻找B
					B_no = -1;
					for (auto i = 0; i < DAG.size(); i++)
					{
						if ((DAG[i].isleaf && DAG[i].value == B) || find(DAG[i].label.begin(), DAG[i].label.end(), B) != DAG[i].label.end())
						{
							B_no = i;
							new_B = false;
							break;
						}
					}
					//已有DAG中没有B则新建B的DAG节点
					if (B_no == -1)
					{
						DAGitem newDAG;
						newDAG.isleaf = true;
						newDAG.value = B;
						newDAG.code = code;
						B_no = DAG.size();
						new_B = true;
						DAG.push_back(newDAG);
					}
					if (oper_type == 0)
					{
						n = B_no;
						state = 4;
					}
					else if (oper_type == 1)
					{
						state = 21;
					}
					else if (oper_type == 2)
					{
						//在已有DAG节点中寻找C
						C_no = -1;
						for (auto i = 0; i < DAG.size(); i++)
						{
							if ((DAG[i].isleaf && DAG[i].value == C) || find(DAG[i].label.begin(), DAG[i].label.end(), C) != DAG[i].label.end())
							{
								C_no = i;
								new_C = false;
								break;
							}
						}
						//已有DAG中没有C则新建C的DAG节点
						if (C_no == -1)
						{
							DAGitem newDAG;
							newDAG.isleaf = true;
							newDAG.value = C;
							newDAG.code = code;
							C_no = DAG.size();
							new_C = true;
							DAG.push_back(newDAG);
						}
						state = 22;
					}
					else if (oper_type == 3)
					{
						//在已有DAG节点中寻找C
						C_no = -1;
						for (auto i = 0; i < DAG.size(); i++)
						{
							if ((DAG[i].isleaf && DAG[i].value == C) || find(DAG[i].label.begin(), DAG[i].label.end(), C) != DAG[i].label.end())
							{
								C_no = i;
								new_C = false;
								break;
							}
						}
						//已有DAG中没有C则新建C的DAG节点
						if (C_no == -1)
						{
							DAGitem newDAG;
							newDAG.isleaf = true;
							newDAG.value = C;
							newDAG.code = code;
							C_no = DAG.size();
							new_C = true;
							DAG.push_back(newDAG);
						}
						//在已有DAG节点中寻找A
						A_no = -1;
						for (auto i = 0; i < DAG.size(); i++)
						{
							if ((DAG[i].isleaf && DAG[i].value == A) || find(DAG[i].label.begin(), DAG[i].label.end(), A) != DAG[i].label.end())
							{
								A_no = i;
								new_A = false;
								break;
							}
						}
						//已有DAG中没有A则新建A的DAG节点
						if (A_no == -1)
						{
							DAGitem newDAG;
							newDAG.isleaf = true;
							newDAG.value = A;
							newDAG.code = code;
							A_no = DAG.size();
							new_A = true;
							DAG.push_back(newDAG);
						}
						DAGitem newDAG;
						newDAG.isremain = true;
						newDAG.isleaf = false;
						newDAG.op = Oper2string(code.Op);
						newDAG.left_child = B_no;
						newDAG.right_child = C_no;
						newDAG.tri_child = A_no;
						newDAG.code = code;
						newDAG.label.push_back(string("ArrayAssigned") + to_string(ArrayAss++));
						n = DAG.size();
						DAG.push_back(newDAG);
						DAG[B_no].parent = n;
						DAG[C_no].parent = n;
						DAG[A_no].parent = n;
						//其他值为该数组中任意元素的叶结点失效
						for (auto i = 0; i < DAG.size(); i++)
						{
							if (DAG[i].isleaf && DAG[i].value == A)
							{
								DAG[i].value = "-" + A;
								break;
							}
						}
						state = -1;
					}
					else
					{
						state = -1;
					}
					break;
				}
				case 21:
				{
					if (DAG[B_no].isleaf && isNum(DAG[B_no].value))
					{
						//B是立即数
						state = 23;
					}
					else
					{
						state = 31;
					}
					break;
				}
				case 22:
				{
					if ((DAG[B_no].isleaf && isNum(DAG[B_no].value)) && (DAG[C_no].isleaf && isNum(DAG[C_no].value)))
					{
						//B和C是立即数
						state = 24;
					}
					else
					{
						state = 32;
					}
					break;
				}
				case 23:
				{
					//Return Parm
					int B = stoi(DAG[B_no].value);
					int P;
					if (code.Op == Oper::Return)
					{
						P = B;
					}
					else if (code.Op == Oper::Parm)
					{
						P = B;
					}
					DAGitem tmpB = DAG[B_no];
					//如果B是新建的则无需新建B的DAG节点
					if (new_B)
					{
						vector<DAGitem>::iterator i;
						i = find(DAG.begin(), DAG.end(), tmpB);
						DAG.erase(i);
					}
					//寻找计算结果是否已经有DAG节点

					//否则新建计算结果的叶节点
					DAGitem newDAG;
					newDAG.isleaf = true;
					newDAG.value = to_string(P);
					newDAG.code = code;
					n = DAG.size();
					DAG.push_back(newDAG);
					state = 4;
					break;
				}
				case 24:
				{
					int B = stoi(DAG[B_no].value);
					int C = stoi(DAG[C_no].value);
					int P;
					if (code.Op == Oper::Plus)
					{
						P = B + C;
					}
					else if (code.Op == Oper::Minus)
					{
						P = B - C;
					}
					else if (code.Op == Oper::Multiply)
					{
						P = B * C;
					}
					else if (code.Op == Oper::Divide)
					{
						P = B / C;
					}
					DAGitem tmpB = DAG[B_no], tmpC = DAG[C_no];
					//如果B是新建的则无需新建B的DAG节点
					if (new_B)
					{
						vector<DAGitem>::iterator i;
						i = find(DAG.begin(), DAG.end(), tmpB);
						DAG.erase(i);
					}
					//如果C是新建的则无需新建C的DAG节点
					if (new_C)
					{
						vector<DAGitem>::iterator i;
						i = find(DAG.begin(), DAG.end(), tmpC);
						DAG.erase(i);
					}
					//寻找计算结果是否已经有DAG节点
					n = -1;
					for (auto i = 0; i < DAG.size(); i++)
					{
						if ((DAG[i].isleaf && DAG[i].value == to_string(P)) || find(DAG[i].label.begin(), DAG[i].label.end(), to_string(P)) != DAG[i].label.end())
						{
							n = i;
							break;
						}
					}
					//否则新建计算结果的叶节点
					if (n == -1)
					{
						DAGitem newDAG;
						newDAG.isleaf = true;
						newDAG.value = to_string(P);
						newDAG.code = code;
						n = DAG.size();
						DAG.push_back(newDAG);
					}
					state = 4;
					break;
				}
				case 31:
				{
					//寻找是否有相同运算的DAG
					n = -1;
					for (auto i = 0; i < DAG.size(); i++)
					{
						if (!DAG[i].isleaf && DAG[i].left_child == B_no && DAG[i].op == Oper2string(code.Op))
						{
							n = i;
							break;
						}
					}
					if (code.Op == Oper::Parm || code.Op == Oper::Call) //不许优化
						n = -1;
					//没有则新建根节点
					if (n == -1)
					{
						DAGitem newDAG;
						newDAG.isleaf = false;
						newDAG.op = Oper2string(code.Op);
						newDAG.left_child = B_no;
						newDAG.code = code;
						n = DAG.size();
						DAG.push_back(newDAG);
						DAG[B_no].parent = n;
					}
					state = 4;
					break;
				}
				case 32:
				{
					//寻找是否有相同运算的DAG
					n = -1;
					for (auto i = 0; i < DAG.size(); i++)
					{
						if (!DAG[i].isleaf && DAG[i].left_child == B_no && DAG[i].right_child == C_no && DAG[i].op == Oper2string(code.Op))
						{
							n = i;
							break;
						}
					}
					//没有则新建根节点
					if (n == -1)
					{
						DAGitem newDAG;
						newDAG.isleaf = false;
						newDAG.op = Oper2string(code.Op);
						newDAG.code = code;
						newDAG.left_child = B_no;
						newDAG.right_child = C_no;
						n = DAG.size();
						DAG.push_back(newDAG);
						DAG[B_no].parent = n;
						DAG[C_no].parent = n;
					}
					state = 4;
					break;
				}
				case 4:
				{
					//如果A已经有DAG节点则从这些节点中去除A,但要保证每一个叶结点都有值，过期的值前面加-，防止再次被选为源操作数
					for (auto i = 0; i < DAG.size(); i++)
					{
						if (DAG[i].isleaf && DAG[i].value == A)
						{
							DAG[i].value = "-" + A;
							break;
						}
						else if (find(DAG[i].label.begin(), DAG[i].label.end(), A) != DAG[i].label.end())
						{
							vector<string>::iterator iter;
							iter = find(DAG[i].label.begin(), DAG[i].label.end(), A);
							DAG[i].label.erase(iter);
							break;
						}
					}
					DAG[n].label.push_back(A);
					state = -1;
					break;
				}
				default:
					break;
			}
		}
	}
	return DAG;
}

Block Optimizer::DAG2block(vector<DAGitem>& DAGs, const Block& block, const set<string>& block_OutVariable)
{
	Block res = Block();
	set<string> activate_variable = block_OutVariable;
	res.name = block.name;
	res.next1 = block.next1;
	res.next2 = block.next2;

	Oper lastOp = block.codes.back().Op;
	if (lastOp == Oper::Jeq || lastOp == Oper::Jge || lastOp == Oper::Jgt || lastOp == Oper::Jle || lastOp == Oper::Jlt || lastOp == Oper::Jne)
	{
		activate_variable.insert(block.codes.back().arg1);
		activate_variable.insert(block.codes.back().arg2);
		activate_variable.insert(block.codes.back().result);
		DAGs.back().isremain = true;
	}

	int ArrayAss = 0;
	for (const auto& code : block.codes)
	{
		if (code.Op == Oper::Call)
		{
			activate_variable.insert(code.arg1);
		}
		if (code.Op == Oper::Parm)
		{
			activate_variable.insert(code.result);
		}
		if (code.Op == Oper::Return)
		{
			activate_variable.insert("ReturnValue");
		}
		if (code.Op == Oper::ArrayAssign)
		{
			activate_variable.insert(string("ArrayAssigned") + to_string(ArrayAss++));
		}
	}

	int len = 0;
	while (len < activate_variable.size())
	{
		len = activate_variable.size();
		for (const auto& vari : activate_variable) // 活跃变量的子节点为活跃
		{
			// 找到值为活跃变量的DAG块
			int n = -1;
			for (auto i = 0; i < DAGs.size(); i++)
			{
				if ((DAGs[i].isleaf && DAGs[i].value == vari) || find(DAGs[i].label.begin(), DAGs[i].label.end(), vari) != DAGs[i].label.end())
				{
					n = i;
					break;
				}
			}
			if (n == -1)
			{
				continue;
			}
			// 将其所有子节点记录为useful
			queue<int> que;
			que.push(n);
			while (!que.empty())
			{
				int par = que.front();
				que.pop();
				if (DAGs[par].useful == true)
					continue;
				DAGs[par].useful = true;
				if (DAGs[par].isleaf)
				{
					// 只处理多元赋值
					if (isVar(DAGs[par].value))
						activate_variable.insert(DAGs[par].value);
					continue;
				}
				if (DAGs[par].code.Op != Oper::Call)
				{
					if (DAGs[par].code.arg1 != string(""))
						activate_variable.insert(DAGs[par].code.arg1);
					if (DAGs[par].code.arg2 != string(""))
						activate_variable.insert(DAGs[par].code.arg2);
				}

				if (DAGs[par].left_child != -1)
					que.push(DAGs[par].left_child);
				if (DAGs[par].right_child != -1)
					que.push(DAGs[par].right_child);
				if (DAGs[par].tri_child != -1)
					que.push(DAGs[par].tri_child);
			}
		}

	}

	for (auto& DAG : DAGs) //按顺序恢复DAG图
	{
		if (DAG.isremain || DAG.useful)
		{
			if (DAG.isleaf) //叶子节点只处理赋值
			{
				if (DAG.value.size() >= 1 && DAG.value[0] == '-')
					DAG.value = DAG.value.substr(1, DAG.value.size() - 1);
				string val = string("");
				vector<string> labels;
				if (isNum(DAG.value))
					val = DAG.value;
				else
				{
					if (find(activate_variable.begin(), activate_variable.end(), DAG.value) != activate_variable.end())
						labels.push_back(DAG.value);
				}
				for (auto& i : DAG.label)
				{
					if (isNum(i))
						val = i;
					else
					{
						if (find(activate_variable.begin(), activate_variable.end(), i) != activate_variable.end()
							&& find(labels.begin(), labels.end(), i) == labels.end())
							labels.push_back(i);
					}
				}
				DAG.label = labels;
				DAG.value = val;
				if (labels.size() == 0)
				{
					DAG.label.push_back(val);
				}
				else if (labels.size() == 1)
				{
					if (isNum(val))
					{
						if (find(block_OutVariable.begin(), block_OutVariable.end(), labels[0]) != block_OutVariable.end())
							res.codes.push_back({ Oper::Assign,val,string(""),labels[0] });
					}
					else if (isVar(val))
						res.codes.push_back({ Oper::Assign,val,string(""),labels[0] });
				}
				else
				{
					if (isNum(val))
					{
						for (int i = 0; i < labels.size(); i++)
							if (find(block_OutVariable.begin(), block_OutVariable.end(), labels[i]) != block_OutVariable.end())
								res.codes.push_back({ Oper::Assign,val,string(""),labels[i] });
					}
					else if (isVar(val))
					{
						for (int i = 0; i < labels.size(); i++)
							res.codes.push_back({ Oper::Assign,val,string(""),labels[i] });
					}
					else
					{
						for (int i = 1; i < labels.size(); i++)
							res.codes.push_back({ Oper::Assign,labels[0],string(""),labels[i] });
					}
				}
				if (isNum(val))
					DAG.label.insert(DAG.label.begin(), val);
			}
			else
			{
				vector<string> labels;
				for (auto& i : DAG.label)
				{
					if (find(activate_variable.begin(), activate_variable.end(), i) != activate_variable.end()
						&& find(labels.begin(), labels.end(), i) == labels.end())
						labels.push_back(i);
				}
				if (labels.size() == 0)
					labels.push_back(label.newTmp());
				DAG.label = labels;

				if (DAG.code.Op == Oper::J)
					res.codes.push_back(DAG.code);
				else if (DAG.code.Op == Oper::Get)
				{
					string parm = DAG.code.result;
					for (const auto DAG : DAGs)
					{
						if ((DAG.isremain || DAG.useful) && DAG.op != string("Get") && (DAG.isleaf && DAG.value == parm) || find(DAG.label.begin(), DAG.label.end(), parm) != DAG.label.end())
						{
							if (DAG.label.size() == 0)
							{
								parm = DAG.value;
							}
							else
							{
								vector<string> labels = DAG.label;
								labels.push_back(DAG.value);
								for (auto& i : labels)
								{
									if (isNum(i))
										continue;
									else
									{
										if (find(activate_variable.begin(), activate_variable.end(), i) != activate_variable.end())
										{
											parm = i;
											break;
										}
									}
								}
							}
							break;
						}
					}
					res.codes.push_back({ DAG.code.Op,DAG.value,DAG.code.arg2,parm });
				}
				else if (DAG.code.Op == Oper::Return || DAG.code.Op == Oper::Parm)
					if (DAG.isleaf)
						res.codes.push_back({ DAG.code.Op,DAG.value,DAG.code.arg2,DAG.code.result });
					else
						res.codes.push_back({ DAG.code.Op,DAGs[DAG.left_child].label[0],DAG.code.arg2,DAG.code.result });
				else if (DAG.code.Op == Oper::Call)
					res.codes.push_back(DAG.code);
				else if (DAG.code.Op == Oper::Jeq || DAG.code.Op == Oper::Jge || DAG.code.Op == Oper::Jgt
					|| DAG.code.Op == Oper::Jle || DAG.code.Op == Oper::Jlt || DAG.code.Op == Oper::Jne || DAG.code.Op == Oper::Plus ||
					DAG.code.Op == Oper::Divide || DAG.code.Op == Oper::Multiply || DAG.code.Op == Oper::Minus)
				{
					if (labels.size() == 1)
					{
						res.codes.push_back({ DAG.code.Op,DAGs[DAG.left_child].label[0],DAGs[DAG.right_child].label[0],labels[0] });
					}
					else
					{
						res.codes.push_back({ DAG.code.Op,DAGs[DAG.left_child].label[0],DAGs[DAG.right_child].label[0],labels[0] });
						for (int i = 1; i < labels.size(); i++)
							res.codes.push_back({ Oper::Assign,labels[0],string(""),labels[i] });
					}
				}
				else if (DAG.code.Op == Oper::ArrayAssign)
				{
					res.codes.push_back({ DAG.code.Op,DAG.code.arg1,DAGs[DAG.right_child].label[0] ,DAGs[DAG.tri_child].label[0] });
				}
				else if (DAG.code.Op == Oper::AssignArray)
				{
					res.codes.push_back({ DAG.code.Op,DAG.code.arg1,DAGs[DAG.right_child].label[0],labels[0] });
				}
			}
		}
	}
	return res;
}

void Optimizer::optimizer()
{
	this->init_INOUTL();
	this->init_DAGs();
	this->optimizer_Blocks();
}