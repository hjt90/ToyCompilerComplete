#include "quadruple.h"

IntermediateLanguage::IntermediateLanguage() :nextquad(CODE_START_POS)
{}

int IntermediateLanguage::emit_code(const quadruple& newqr)
{
	code.push_back(newqr);
	int quadnow = nextquad;
	nextquad++;
	return quadnow;
}

void IntermediateLanguage::back_patch(std::vector<quadrupleIndex> qrlist, int pos)
{
	for (auto it = qrlist.begin(); it != qrlist.end(); it++)
	{
		code[(*it) - CODE_START_POS].result = std::to_string(pos); //这里注意减去代码段的初始偏移
	}
	return;
}

string Oper2string(Oper op)
{
	switch (op)
	{
		case Oper::J:
			return "J";
			break;
		case Oper::Jeq:
			return "Jeq";
			break;
		case Oper::Jgt:
			return "Jgt";
			break;
		case Oper::Jge:
			return "Jge";
			break;
		case Oper::Jlt:
			return "Jlt";
			break;
		case Oper::Jle:
			return "Jle";
			break;
		case Oper::Jne:
			return "Jne";
			break;
		case Oper::Plus:
			return "Plus";
			break;
		case Oper::Minus:
			return "Minus";
			break;
		case Oper::Multiply:
			return "Multiply";
			break;
		case Oper::Divide:
			return "Divide";
			break;
		case Oper::Assign:
			return "Assign";
			break;
		case Oper::Parm:
			return "Parm";
			break;
		case Oper::Call:
			return "Call";
			break;
		case Oper::Return:
			return "Return";
			break;
		default:
			return "-1";
	}
}

void IntermediateLanguage::output(ofstream& midcode)
{
	int pos = CODE_START_POS;
	for (auto i : code)
	{
		midcode << pos++ << "," << Oper2string(i.Op) << "," << i.arg1 << "," << i.arg2 << "," << i.result << "," << endl;
	}
}

quadruple::quadruple(Oper Op, std::string arg1, std::string arg2, std::string result)
{
	this->Op = Op;
	this->arg1 = arg1;
	this->arg2 = arg2;
	this->result = result;
}

quadruple::quadruple() : Op(Oper::Assign)
{}