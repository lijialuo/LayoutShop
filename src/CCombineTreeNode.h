#pragma once
#include "CNode.h"
#include "CGroupTreeNode.h"
#include <queue>

enum OPERSTATE { CORRESED, DELETE };
enum NODE_CORRES_TYPE { T_ABC, T_AB, T_AC, T_BC, A, B, C };	//A:first tree   B:second_tree   C:third_tree

class CCombineTreeNode
{
public:
	CCombineTreeNode() : tree0_node(nullptr), tree1_node(nullptr), tree2_node(nullptr), group_tree_node(nullptr), node_type(NODE_TYPE::LEAF), parent(nullptr),
		tree0_node_id(-20000), tree1_node_id(-20000), tree2_node_id(-20000), node_relation(NODE_RELATION::NONE) {};
	~CCombineTreeNode() {};

	void DeleteNode(CCombineTreeNode* node);

	static void DeepDestroy(CCombineTreeNode* aNode);
	CCombineTreeNode* DeepCopy();

public:
    enum ALIGN_TYPE {H, V};
    typedef struct AlignPair
    {
        CCombineTreeNode* source_node;
        CCombineTreeNode* target_node;
        int               source_point_idx, target_point_idx;
        ALIGN_TYPE align_type;
    } AlignPair;


	int index;

	CNode* tree0_node;
	CNode* tree1_node;
	CNode* tree2_node;
	CGroupTreeNode* group_tree_node;

	int tree0_node_id;
	int tree1_node_id;
	int tree2_node_id;
	int group_tree_node_id;

	NODE_TYPE node_type;
	NODE_CORRES_TYPE node_corres_type;
	NODE_PRESENT_TYPE node_present_type;
	NODE_RELATION node_relation;

	OPERSTATE operation_state;
	double operation_value;

	double x, y, width, height;
    double fix_x, fix_y, fix_width, fix_height;
	int corres_img_idx;
    int text_idx = -1;
    bool picture_lock = false;
    int fix_geometry_level = -1;
	double v_padding;
	double h_padding;
	std::pair<int, int> grid_template;
	std::set<std::pair<int, int>> equal_size_node;
	std::vector<int> equal_space_node;
	std::vector<int> left_align_node;
	std::vector<int> right_align_node;
	std::vector<int> top_align_node;
	std::vector<int> bottom_align_node;
    std::vector<AlignPair*> align_pair_list;
	CCombineTreeNode* parent;
	std::vector<CCombineTreeNode*> children;

	double img_score = 0.0;
    double structure_score = 0.0;
    double total_score = 0.0;

    int group_idx_ = -1;

};