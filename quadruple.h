#pragma once
#include <string>
#include <vector>

const int CODE_START_POS = 100;

enum class Oper : char
{
    J = 0,
    Jeq,
    Jgt,
    Jge,
    Jlt,
    Jle,
    Assign
};

struct quadruple
{
    Oper Op;
    int arg1;
    int arg2;
    std::string result;
};

class IntermediateLanguage
{
    std::vector<quadruple> code;

public:
    static int nextquad;
    IntermediateLanguage() = default;
};