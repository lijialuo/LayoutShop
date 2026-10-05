#pragma once
#include "CNode.h"
#include <string>
#include <unordered_map>
#include <queue>
#include <Qpainter>


class CLayoutTree
{
public:
	CLayoutTree();
	CLayoutTree(CNode* root);
	~CLayoutTree();

	void InsertNode(CNode* node, CNode* parent = nullptr);
	void DeleteNode(CNode* node);
	void DestroyAllData();
	void ReadFromFile(std::string fileName);
	void BreadthFirstSearch();
	void DrawLayout(QPainter& painter, double resizeCoef = 1.0);

	int StringToInteger(std::string str);

	CNode* GetRoot() { return root; }

public:
	std::unordered_map<CNode*, int> ptr_idx_map;
	std::unordered_map<int, CNode*> idx_ptr_map;

private:
	NODE_PRESENT_TYPE SetNodePresentType(int flag);
	void DrawLeafNode(QPainter& paint, CNode* node_, double& resizeCoef);

private:
	CNode* root;
};

