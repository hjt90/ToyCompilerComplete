#pragma once
#include <string>
#include <vector>

const int CODE_START_POS = 100;
typedef int quadrupleIndex;

enum class Oper : char
{
    J = 0,
    Jeq,
    Jgt,
    Jge,
    Jlt,
    Jle,
    Assign,
    Return
};

class quadruple
{
public:
    quadruple(Oper Op, std::string arg1, std::string arg2, std::string result);
    Oper Op;
    std::string arg1;
    std::string arg2;
    std::string result;
};

class IntermediateLanguage
{
    std::vector<quadruple> code;

public:
    static quadrupleIndex nextquad;
    int emit_code(const quadruple & newqr);
    void back_patch(std::vector<quadrupleIndex> qrlist, int pos);
    IntermediateLanguage() = default;
};