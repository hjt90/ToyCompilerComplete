#include "quadruple.h"

int IntermediateLanguage::nextquad = CODE_START_POS;

void IntermediateLanguage::emit_code(const quadruple & newqr)
{
    code.push_back(newqr);
    nextquad++;
    return;
}

void IntermediateLanguage::back_patch(std::vector<quadrupleIndex> qrlist, int pos)
{
    for (auto it = qrlist.begin(); it != qrlist.end(); it++)
    {
        code[*it].result = pos;
    }
    return;
}

quadruple::quadruple(Oper Op, std::string arg1, std::string arg2, std::string result)
{
    this->Op = Op;
    this->arg1 = arg1;
    this->arg2 = arg2;
    this->result = result;
}