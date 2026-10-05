#pragma once
#include <vector>
#include <set>

enum NODE_TYPE { LEAF, NONLEAF };
enum NODE_PRESENT_TYPE { TITLE,  TEXT, PICTURE, PADDING, NONLABEL };
enum NODE_RELATION {NONE, HORIZONTAL, VERTICAL};

class CNode
{
public:
	CNode() {};
	CNode(int index_, NODE_TYPE node_type_, int x_, int y_, int width_, int height_) : index(index_), node_type(node_type_),
		x(x_), y(y_), width(width_), height(height_), parent(nullptr) {};

public:
	int index;
	NODE_TYPE node_type;
	int x;
	int y;
	int width;
	int height;

	NODE_PRESENT_TYPE node_present_type;
    NODE_RELATION node_relation;

	CNode* parent;
	std::vector<CNode*> children;

	std::set<std::pair<int, int>> equal_size_node;
	std::vector<int> equal_space_node;
	std::vector<int> left_align_node;
	std::vector<int> right_align_node;
	std::vector<int> top_align_node;
	std::vector<int> bottom_align_node;
};