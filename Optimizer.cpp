#include "Optimizer.h"
#include<queue>

NewLabeler::NewLabeler() :index(1)
{}

string NewLabeler::newLabel() {
	return string("Label") + to_string(index++);
}

void Optimizer::divideBlocks(const parsing& pars) {
	vector<pair<int, string> > funcEnter = pars.p_symbolTable->getFuncEnter();
	this->code = pars.mid_code;

	for (vector<pair<int, string> >::iterator iter = funcEnter.begin(); iter != funcEnter.end(); iter++) { //ÿһ��������
		vector<Block>blocks;
		priority_queue<int, vector<int>, greater<int> >block_enter; //���л���������λ��
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
			else if (code.code[i - CODE_START_POS].Op == Oper::Return || code.code[i - CODE_START_POS].Op == Oper::Call) {
				if (i + 1 < endIndex) {
					block_enter.push(i + 1);
				}
			}
		}

		//���ֻ�����	
		Block block;
		map<int, string>labelEnter; //��ڵ�ͱ�ǩ�Ķ�Ӧ��ϵ
		map<int, int>enter_block; //������ڵ��block�Ķ�Ӧ��ϵ
		int firstFlag = true; //�������һ���ǣ��ÿ�����Ϊ������
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

			if (!firstFlag) { //�û����鲻�Ǻ�����ĵ�һ�������
				block.name = label.newLabel();
				labelEnter[lastEnter] = block.name;
			}
			else { //�û������Ǻ�����ĵ�һ�������
				block.name = iter->second;
				firstFlag = false;
			}

			enter_block[lastEnter] = blocks.size();
			blocks.push_back(block);
			lastEnter = enter;
			block.codes.clear();
		}
		if (!firstFlag) { //�û����鲻�Ǻ�����ĵ�һ�������
			block.name = label.newLabel();
			labelEnter[lastEnter] = block.name;
		}
		else { //�û������Ǻ�����ĵ�һ�������
			block.name = iter->second;
			firstFlag = false;
		}
		if (iter + 1 != funcEnter.end()) { //���������������֮��
			for (int i = lastEnter; i != (iter + 1)->first; i++) {
				block.codes.push_back(code.code[i - CODE_START_POS]);
			}
		}
		else {//�����һ���������м����ĩβ
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