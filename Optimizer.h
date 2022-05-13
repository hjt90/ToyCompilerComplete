#pragma once
#include "quadruple.h"
#include "Parsing.h"

class NewLabeler {
private:
	int index;
	int tmp;
public:
	NewLabeler();
	string newLabel();
	string newTmp();
};

struct DAGitem
{
	bool useful = false;
	bool isleaf;
	string value;
	string op;
	vector<string> label;
	int parent = -1;
	int left_child = -1;
	int right_child = -1;
	int tri_child = -1;
	bool isremain = false;
	quadruple code;
	bool operator== (DAGitem b)
	{
		bool f1 = this->isleaf == b.isleaf;
		bool f2 = this->value == b.value;
		bool f3 = this->op == b.op;
		bool f4 = this->label.size() == b.label.size();
		bool f5 = this->parent == b.parent;
		bool f6 = this->left_child == b.left_child;
		bool f7 = this->right_child == b.right_child;
		bool f8 = true;
		for (auto i = 0; i < this->label.size() && i < b.label.size(); i++)
		{
			if (this->label[i] != b.label[i])
			{
				f8 = false;
				break;
			}
		}
		return f1 && f2 && f3 && f4 && f5 && f6 && f7 && f8;
	}
};

typedef vector<DAGitem> Block_DAG;

class Optimizer {
private:
	IntermediateLanguage code;
	map<string, vector<Block> >funcBlocks;
	map<string, vector<Block_DAG> >DAGBlocks;  //��������DAGͼ
	map<string, vector<set<string> > >funcOUTL; //���������л�����ĳ��ڻ�Ծ������
	map<string, vector<set<string> > >funcINL; //���������л��������ڻ�Ծ������
	NewLabeler label;

	void preparing(Block&);
	void init_INOUTL(); //��ʼ������ڻ�Ծ������
	void init_DAGs();
	void optimizer_Blocks();

	vector<DAGitem> geneDAG(const Block&); //��������תΪDAGͼ
	Block DAG2block(vector<DAGitem>&, const Block&, const set<string>&); //��DAGͼתΪ������


public:
	Optimizer() = default;
	void outputBlocks(ostream& out);
	void divideBlocks(const parsing&);
	void optimizer();
};