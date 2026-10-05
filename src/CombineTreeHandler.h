#pragma once
#include "CCombineTreeNode.h"
#include "CLayoutTree.h"
#include "CustomGraphicsScene.h"


class CombineTreeHandler
{
public:
	CombineTreeHandler();
	~CombineTreeHandler();
	CCombineTreeNode* GenerateCombineTree(std::vector<CLayoutTree*> m_uploadLayout);
	std::vector<CCombineTreeNode*> GetCombineTreeCandidate(CCombineTreeNode* combine_tree, int img_num, int text_num);
	std::vector<std::vector<CCombineTreeNode*>> GetCombineTreeCandidateMultiPage(CCombineTreeNode* combine_tree, int img_num);
	std::vector<std::vector<CCombineTreeNode*>> GetCombineTreeCandidateMultiPage2(CCombineTreeNode* combine_tree, int img_num, int target_page);
    std::vector<std::vector<CCombineTreeNode*>> GetCombineTreeCandidateMultiPage3(CCombineTreeNode* combine_tree, int img_num, int target_page, std::vector<std::pair<int, double>> ref_idx_ratio_pair_list);
    std::vector<CCombineTreeNode*> GetTreeCandidateByRelations(CCombineTreeNode* combine_tree_root);

	int TreeColNum(CCombineTreeNode* tree);

	int CountDistanceBetweenTwoTrees(CCombineTreeNode* tree1, CCombineTreeNode* tree2);
	void TraverseTree(CCombineTreeNode* tree, std::vector<CCombineTreeNode*>& traverse_list, std::vector<int>& level_list,int level);
	void FindLeftNodes(std::vector<CCombineTreeNode*>& traverse_list, std::vector<CCombineTreeNode*>& left_node_list);
	int FindMaxLevel(CCombineTreeNode* tree);

	void MultiPageFindSolutions(std::vector<int>& limit, std::vector<int>& solution, std::vector<std::vector<int>>& solution_list, int img_num, int img_left);
	//void FindKeyRoots();
	void CutTitleFromATree(CCombineTreeNode* tree);

	int GetPicNode(CCombineTreeNode* combine_tree_root);
	int GetTitleNode(CCombineTreeNode* combine_tree_root);
	int GetTextNode(CCombineTreeNode* combine_tree_root);
	int GetPaddingNode(CCombineTreeNode* combine_tree_root);

	void DfsPic(int parent_list_idx, int delete_num);
	void DfsTitle(int parent_list_idx, int delete_num);
	void DfsText(int parent_list_idx, int delete_num);
	void DfsPadding(int parent_list_idx, int delete_num);

	int GetTextNodeWithVoid(CCombineTreeNode* combine_tree_root);

	void GetParentNode(CCombineTreeNode* combine_tree_root);
	void DeterminOrder(std::vector<CCombineTreeNode*>& tree);
	static void MergeTextNode(CCombineTreeNode* tree);
    static void MergePaddingNode(CCombineTreeNode* tree);
	static void MergeRelationNode(CCombineTreeNode* tree);
	static void OutputStructure(CCombineTreeNode* combine_tree);
	static void CutTreeNode(CCombineTreeNode* tree_node);
	static void MergeExtraParent(CCombineTreeNode* tree);
	void CutExtraTree(std::vector<CCombineTreeNode*>& tree_list);
	void SortTreeForDuplicate(CCombineTreeNode* tree);

	static bool CombineNodeCompareX(CCombineTreeNode* node_one, CCombineTreeNode* node_two);
	static bool CombineNodeCompareY(CCombineTreeNode* a, CCombineTreeNode* b);

	static int CombineNodeCompareDuplicateInt(CCombineTreeNode* node_one, CCombineTreeNode* node_two);
	static bool CombineNodeCompareDuplicateBool(CCombineTreeNode* node_one, CCombineTreeNode* node_two);
	static CCombineTreeNode* AddANodeToATree(CCombineTreeNode* source, CCombineTreeNode* target, CustomGraphicsScene::INSERT_DIRECTION direction);
	static int GenerateNewIndex(CCombineTreeNode* node);
private:
	std::vector<CCombineTreeNode*> parent_list;

	std::map<int,std::vector<int>> pic_node_list;
	std::vector<std::set<int>> pic_node_delete_list;
	std::set<int> pic_node_delete_temp;

	std::map<int, std::vector<int>> text_node_list;
	std::vector<std::set<int>> text_node_delete_list;
	std::set<int> text_node_delete_temp;

	//std::vector<int>txt_node_list;
	//std::vector<std::vector<int> >txt_node_delete_list;
	//std::vector<int>txt_node_delete_temp;

	std::map<int, std::vector<int>> title_node_list;
	std::vector<std::set<int>> title_node_delete_list;
	std::set<int> title_node_delete_temp;

	std::map<int, std::vector<int>> padding_node_list;
	std::vector<std::set<int>> padding_node_delete_list;
	std::set<int> padding_node_delete_temp;

    bool DivideLayoutTreeInGroups(CCombineTreeNode *tree, std::vector<std::pair<int, int>> group_need);
};











