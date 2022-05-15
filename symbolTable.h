#pragma once
#include <string>
#include <vector>
#include <map>
using namespace std;

enum class symbolType : char
{
	Int,
	Real,
	Void,
	Array,
	None,
	Unknown
};

enum class procSymbolTableType : char
{
	function,
	block,
	unknow
};

const map< symbolType, int> symbolTypeOffset =
{
	{map< symbolType, int>::value_type(symbolType::Int, 4)},
	{map< symbolType, int>::value_type(symbolType::Real,4)},
	{map< symbolType, int>::value_type(symbolType::Void,0)},
	{map< symbolType, int>::value_type(symbolType::Array,4)},
	{map< symbolType, int>::value_type(symbolType::None,0)},
	{map< symbolType, int>::value_type(symbolType::Unknown,0)}
};

struct symbolTableItem
{
	symbolType type;
	std::string name;
	std::string gobalname;
	int offset; //偏移量
	vector<int> array;
	symbolTableItem() = default;
	symbolTableItem(symbolType, std::string, std::string, int);
	symbolTableItem(symbolType, std::string, std::string, int, vector<int>);
};

class Optimizer;
class ObjectCode;
class proc_symbolTable
{
	procSymbolTableType type; //种类
	int enter_quad; //入口地址
	std::string name;
	proc_symbolTable* returnAddr;      //返回地址
	std::map<std::string, proc_symbolTable*> functionTable;
	std::map<std::string, symbolTableItem> itemTable;
	int itemTable_offset; //当前符号表的offset

public:
	static int nexttmpname;
	static std::string newtemp();

	std::vector<symbolTableItem> parm; //参数列表
	symbolType return_type;

	proc_symbolTable();
	proc_symbolTable* create_function();
	proc_symbolTable* return_block(int enter_quad);
	void init_function(const std::vector<symbolTableItem>& parms, std::string name, symbolType return_type, int enter_quad);
	void insert_variable(const symbolTableItem&);

	const symbolTableItem& find_variable(std::string name);
	proc_symbolTable* find_function(const std::string& name) const;
	const proc_symbolTable* find_gobal() const;

public:
	int get_enterquad() const;
	int get_offset() const;
	vector<pair<int, string> >getFuncEnter() const;
	friend class Optimizer;
	friend class ObjectCode;
};