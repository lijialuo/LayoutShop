#pragma once

#include "CNode.h"
#include <vector>


class CGroupTreeNode
{
public:
	//															初始先都设置为叶子结点，插入时做修改
	CGroupTreeNode() : tree0_node(nullptr), tree1_node(nullptr), node_type(NODE_TYPE::LEAF), parent(nullptr), tree0_node_id(-10000), tree1_node_id(-10000) {};
	~CGroupTreeNode() {};


public:
	int index;
	CNode* tree0_node;
	CNode* tree1_node;

	int tree0_node_id;
	int tree1_node_id;

	NODE_TYPE node_type;

	CGroupTreeNode* parent;
	std::vector<CGroupTreeNode*> children;

};
