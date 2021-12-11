#include "quadruple.h"

int IntermediateLanguage::nextquad = CODE_START_POS;

int IntermediateLanguage::emit_code(const quadruple &)
{
    return;
}

void IntermediateLanguage::back_patch(std::vector<quadrupleIndex>, int)
{
    return;
}