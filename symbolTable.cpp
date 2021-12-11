#include "symbolTable.h"

int proc_symbolTable::nextname = 0;

const symbolTableItem &proc_symbolTable::find_variable(std::string name)
{
}

const symbolTableFunction &proc_symbolTable::find_function(std::string name)
{
}

void proc_symbolTable::make_function(const std::vector<symbolTableItem> &parms, std::string name, symbolType return_type)
{
}

void proc_symbolTable::insert_variable(const symbolTableItem &)
{
}

int proc_symbolTable::into_function(std::string)
{
}

int proc_symbolTable::return_function()
{
}