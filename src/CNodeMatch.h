#pragma once

#include "CHungarianAlgorithm.h"
#include "CLayoutTree.h"
#include "CGroupTreeNode.h"
#include "CCombineTreeNode.h"
#include <iostream>
#include <algorithm>
#include <set>
#include <vector>
#include <queue>
#include <unordered_map>

class pairhash {
public:
	template <typename T, typename U>
	std::size_t operator()(const std::pair<T, U>& x) const
	{
		return std::hash<T>()(x.first) ^ std::hash<U>()(x.second);
	}
};

class CNodeMatch
{
public:
	CNodeMatch();
	~CNodeMatch();

	int HungaryCost(std::vector<CNode*>& nodeSet1, std::vector<CNode*>& nodeSet2, std::set<std::pair<int, int>>& crtCsp);
	int HungaryGrpThdCost(std::vector<CGroupTreeNode*>& nodeSet1, std::vector<CNode*>& nodeSet2, std::set<std::pair<int, int>>& crtCsp);

	int ComputeCost(CNode* n1, CNode* n2);
	void ComputeCsp(CNode* root1, CNode* root2);
	int getCost() { return two_tree_cost; }

	void CreateGroupTree1();		//生成group tree
	void CreateGroupTree();		//生成group tree，上面那种生成group tree的方法有问题，不能处理多次下沉



	void CreateCombineTree1();		//生成combine tree
	void CreateCombineTree();	//另一种生成combine tree的方法，上面那种方法有问题

	void GetCrpInFirstTwoTree();		//指定combine tree node在头两个tree 中的对应。

	void AssignGroupTreeIndex();	//给group tree的节点赋予index
	void AssignCombineTreeIndex();	//给combine tree的节点赋予index



	void SetThirdTree(CNode* tree_root);

	void ComputeGrpThdCsp();	//计算group tree与third tree的对应
	int CalGrpThdCost(CGroupTreeNode* n1, CNode* n2);	//计算group tree节点与普通节点之间的cost

	CCombineTreeNode* GetCombineTreeRoot();

private:
	int LeafToNULLCost(CNode* node);
	int LeafToLeafCost(CNode* n1, CNode* n2);

	int GroupLeafToLeafCost(CGroupTreeNode* n1, CNode* n2);

	bool PairExistedInCombineTree(std::pair<int, int>& thisPair, CCombineTreeNode* combine_tree_root_);
	bool OnlyPairFirstExistedInCombineTree(std::pair<int, int>& thisPair, CCombineTreeNode* combine_tree_root_);
	bool PairDoNotExistedInCombineTree(std::pair<int, int>& thisPair, CCombineTreeNode* combine_tree_root_);
	bool OnlyPairSecondExistedInCombineTree(std::pair<int, int>& thisPair, CCombineTreeNode* combine_tree_root_);

	bool PairExistedInGroupTree(std::pair<int, int>& thisPair, CGroupTreeNode* group_tree_root_);
	bool OnlyPairFirstExistedInGroupTree(std::pair<int, int>& thisPair, CGroupTreeNode* group_tree_root_);
	bool PairDoNotExistedInGroupTree(std::pair<int, int>& thisPair, CGroupTreeNode* group_tree_root_);
	bool OnlyPairSecondExistedInGroupTree(std::pair<int, int>& thisPair, CGroupTreeNode* group_tree_root_);

	CCombineTreeNode* ReturnParentNode(int index1, int index2);
	CCombineTreeNode* ReturnRightParentNode(int index2);
	CCombineTreeNode* ReturnLeftParentNode(int index1);

	CGroupTreeNode* ReturnParentGroupNode(int index1, int index2);
	CGroupTreeNode* ReturnRightParentGroupNode(int index2);
	CGroupTreeNode* ReturnLeftParentGroupNode(int index1);

private:
	CNode* first_tree_root;
	CNode* second_tree_root;
	CNode* third_tree_root;
	CGroupTreeNode* group_tree_root;
	CCombineTreeNode* combine_tree_root;

	int two_tree_cost;		//记录两个input layout的deformation cost

	std::unordered_map<std::pair<int, int>, std::set<std::pair<int, int>>, pairhash> recordCorrespondence;
	std::unordered_map<std::pair<int, int>, int, pairhash> recordCost;

	std::unordered_map<std::pair<int, int>, std::set<std::pair<int, int>>, pairhash> recordGrpThdCrp;	//记录group tree与third layout之间的对应
	std::unordered_map<std::pair<int, int>, int, pairhash> recordGrpThdCost;	//记录group tree与third layout之间的cost

};