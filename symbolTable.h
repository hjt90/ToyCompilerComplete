#pragma once
#include <string>
#include <vector>
#include <map>

enum class symbolType : char
{
    Int,
    Real,
    Void,
    None
};

struct symbolTableItem
{
    symbolType type;
    std::string name;
};

class proc_symbolTable;
struct symbolTableFunction
{
    int enter_quad;
    std::string name;
    symbolType return_type;
    proc_symbolTable *addr;
    std::vector<symbolTableItem> parm;
};

class proc_symbolTable
{
    proc_symbolTable *returnAddr;
    std::map<std::string, symbolTableItem> itemTable;
    std::map<std::string, symbolTableFunction> functionTable;

public:
    static int nextname;
    static int nexttmpname;
    proc_symbolTable() = default;
    const symbolTableItem &find_variable(std::string name);
    const symbolTableFunction &find_function(std::string name);
    void make_function(const std::vector<symbolTableItem> &parms, std::string name, symbolType return_type, int enter_quad);
    void insert_variable(const symbolTableItem &);
    proc_symbolTable *into_function(std::string);
    proc_symbolTable *return_function();
    string newtemp();
};