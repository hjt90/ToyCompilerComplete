#include "quadruple.h"
int IntermediateLanguage::nextquad = CODE_START_POS;

int IntermediateLanguage::emit_code(const quadruple & newqr)
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
        code[(*it)- CODE_START_POS].result =std::to_string(pos);//这里注意减去代码段的初始偏移
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