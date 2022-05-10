#include "symbolTable.h"
using namespace std;

int proc_symbolTable::nexttmpname = 0;

proc_symbolTable::proc_symbolTable() : enter_quad(-1), return_type(symbolType::Unknown), returnAddr(NULL), itemTable_offset(0), type(procSymbolTableType::unknow)
{
}

proc_symbolTable* proc_symbolTable::create_function()
{
	proc_symbolTable* func = new proc_symbolTable();
	func->returnAddr = this;
	return func;
}

void proc_symbolTable::insert_variable(const symbolTableItem& item)
{
	if (this->itemTable.find(item.name) == this->itemTable.end()) //Ã»ÓÐÖØ¸´
	{
		this->itemTable.insert({ item.name, item });
		this->itemTable_offset += symbolTypeOffset.find(item.type)->second;
	}
	else
	{
		;//throw error
	}
}

proc_symbolTable* proc_symbolTable::find_function(const std::string& name) const
{
	const proc_symbolTable* gobal = this->find_gobal();
	if (gobal->functionTable.find(name) != gobal->functionTable.end())
	{
		return gobal->functionTable.find(name)->second;
	}
	return NULL;
}

const symbolTableItem& proc_symbolTable::find_variable(std::string name)
{
	proc_symbolTable* tmp = this;
	while (tmp->returnAddr != NULL)
	{
		if (tmp->itemTable.find(name) != tmp->itemTable.end())
		{
			return tmp->itemTable[name];
		}
		tmp = tmp->returnAddr;
	}
	if (tmp->itemTable.find(name) != tmp->itemTable.end())
	{
		return tmp->itemTable[name];
	}
	symbolTableItem tmpItem;
	tmpItem.type = symbolType::None;
	return tmpItem;
}

const proc_symbolTable* proc_symbolTable::find_gobal() const
{
	const proc_symbolTable* tmp = this;
	while (tmp->returnAddr != NULL)
	{
		tmp = tmp->returnAddr;
	}
	return tmp;

}

void proc_symbolTable::init_function(const std::vector<symbolTableItem>& parms, std::string name, symbolType return_type, int enter_quad)
{
	this->name = name;
	this->return_type = return_type;
	this->parm = parms;
	this->enter_quad = enter_quad;
	this->returnAddr->functionTable.insert({ name, this });
	this->type = procSymbolTableType::function;
	return;
}

proc_symbolTable* proc_symbolTable::return_block(int enter_quad)
{
	return this->returnAddr;
}

string proc_symbolTable::newtemp()
{
	string temp = "T";
	temp += std::to_string(proc_symbolTable::nexttmpname);
	proc_symbolTable::nexttmpname++;
	return temp;
}

int proc_symbolTable::get_enterquad() const
{
	return this->enter_quad;
}

int proc_symbolTable::get_offset() const
{
	return this->itemTable_offset;
}