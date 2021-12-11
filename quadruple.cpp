#include "quadruple.h"

int IntermediateLanguage::nextquad = CODE_START_POS;

int IntermediateLanguage::emit_code(const quadruple & newqr)
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