#include "symbolTable.h"
using namespace std;

int proc_symbolTable::nextname = 0;
int proc_symbolTable::nexttmpname = 0;

const symbolTableItem& proc_symbolTable::find_variable(std::string name)
{
	if (itemTable.find(name) != itemTable.end())
	{
		return itemTable[name];
	}
	symbolTableItem tmp;
	tmp.type = symbolType::None;
	return tmp;
}

const symbolTableFunction& proc_symbolTable::find_function(std::string name)
{
	if (functionTable.find(name) != functionTable.end())
	{
		return functionTable[name];
	}
	symbolTableFunction tmp;
	tmp.return_type = symbolType::None;
	return tmp;
}

void proc_symbolTable::make_function(const std::vector<symbolTableItem>& parms, std::string name, symbolType return_type, int enter_quad)
{
	symbolTableFunction func;
	func.name = name;
	func.return_type = return_type;
	func.parm = parms;
	func.enter_quad = enter_quad;
	func.addr = new proc_symbolTable();
	func.addr->returnAddr = this;

	this->functionTable.insert({ name, func });
	return;
}

void proc_symbolTable::insert_variable(const symbolTableItem& item)
{
	this->itemTable.insert({ item.name, item });
}

proc_symbolTable* proc_symbolTable::into_function(std::string name)
{
	return this->find_function(name).addr;
}

proc_symbolTable* proc_symbolTable::return_function()
{
	return this->returnAddr;
}

string proc_symbolTable::newtemp()
{
	string temp = "T";
	temp += std::to_string(nexttmpname);
	nexttmpname++;
	return temp;
}