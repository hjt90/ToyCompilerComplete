#pragma once
#include <string>
#include <vector>
#include <map>

enum class symbolType : char
{
    Int,
    Real
};

struct symbolTableItem
{
    symbolType type;
    std::string name;
    int addr;
};

struct symbolTableFunction
{
    std::string name;
    symbolTable *addr;
};

class symbolTable
{
public:
    static int nextname;
    symbolTable *returnAddr;
    std::map<std::string, symbolTableItem> itemTable;
    std::map<std::string, symbolTableFunction> functionTable;
    symbolTable() = default;
};