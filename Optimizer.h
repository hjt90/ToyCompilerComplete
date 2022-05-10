#pragma once
#include "quadruple.h"
#include "Parsing.h"

class NewLabeler {
private:
	int index;
public:
	NewLabeler();
	string newLabel();
};

class Optimizer {
private:
	IntermediateLanguage code;
	map<string, vector<Block> >funcBlocks;
	NewLabeler label;

public:
	Optimizer() = default;
	void outputBlocks(ostream& out);
	void divideBlocks(const parsing&);
};