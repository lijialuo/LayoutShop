#include "CombineTreeHandler.h"

#include <stack>

#include "CCombineTreeProcessor.h"
#include <Eigen/Dense>
#include <algorithm>
#include <random>

CombineTreeHandler::CombineTreeHandler()
= default;

CombineTreeHandler::~CombineTreeHandler()
= default;

bool CombineTreeHandler::CombineNodeCompareX(CCombineTreeNode* node_one, CCombineTreeNode* node_two)
{
	return node_one->x < node_two->x;
}

bool CombineTreeHandler::CombineNodeCompareY(CCombineTreeNode* a, CCombineTreeNode* b)
{
	return a->y < b->y;
}

int CombineTreeHandler::CombineNodeCompareDuplicateInt(CCombineTreeNode* node_one, CCombineTreeNode* node_two)
{
	NODE_PRESENT_TYPE type_one = node_one->node_present_type;
	NODE_PRESENT_TYPE type_two = node_two->node_present_type;
	if(type_one != NODE_PRESENT_TYPE::NONLABEL || type_two != NODE_PRESENT_TYPE::NONLABEL)	return type_one - type_two;

	if (node_one->node_relation != node_two->node_relation) return node_one->node_relation - node_two->node_relation;

	int idx_one = 0, idx_two = 0;
	for (; idx_one < node_one->children.size() && idx_two < node_two->children.size(); ++idx_one, ++idx_two)
	{
		bool sub_cmp = CombineNodeCompareDuplicateInt(node_one->children[idx_one], node_two->children[idx_two]);
		if (sub_cmp != 0)
			return sub_cmp;
	}
	if (idx_one < node_one->children.size()) return 1;
	if (idx_two < node_two->children.size()) return  -1;
	return 0;
}

bool CombineTreeHandler::CombineNodeCompareDuplicateBool(CCombineTreeNode* node_one, CCombineTreeNode* node_two)
{
	NODE_PRESENT_TYPE type_one = node_one->node_present_type;
	NODE_PRESENT_TYPE type_two = node_two->node_present_type;
	if (type_one != NODE_PRESENT_TYPE::NONLABEL || type_two != NODE_PRESENT_TYPE::NONLABEL)	return type_one < type_two;

	if (node_one->node_relation != node_two->node_relation) return node_one->node_relation < node_two->node_relation;

	int idx_one = 0, idx_two = 0;
	for (; idx_one < node_one->children.size() && idx_two < node_two->children.size(); ++idx_one, ++idx_two)
	{
		int sub_cmp = CombineNodeCompareDuplicateInt(node_one->children[idx_one], node_two->children[idx_two]);
		if (sub_cmp > 0) return false;
		if (sub_cmp < 0) return true;
	}
	if (idx_two < node_two->children.size()) return  true;
	return false;
}

CCombineTreeNode* CombineTreeHandler::AddANodeToATree(CCombineTreeNode* source, CCombineTreeNode* target,
	CustomGraphicsScene::INSERT_DIRECTION direction)
{
	if(target->parent == nullptr)
	{
		int new_root_index = GenerateNewIndex(target) + 1;
		auto* new_root = new CCombineTreeNode();
		new_root->parent = nullptr;
		new_root->index = new_root_index;
		new_root->node_type = NONLEAF;
		new_root->node_present_type = NONLABEL;
		if (direction == CustomGraphicsScene::LEFT || direction == CustomGraphicsScene::RIGHT)
		{
			new_root->node_relation = HORIZONTAL;
		}
		else if (direction == CustomGraphicsScene::UP || direction == CustomGraphicsScene::BOTTOM)
		{
			new_root->node_relation = VERTICAL;
		}
		if (direction == CustomGraphicsScene::LEFT || direction == CustomGraphicsScene::UP)
		{
			new_root->children = { source, target };
		}
		else if (direction == CustomGraphicsScene::RIGHT || direction == CustomGraphicsScene::BOTTOM)
		{
			new_root->children = {  target, source };
		}
		source->parent = new_root;
		target->parent = new_root;
	}
	else if(direction == CustomGraphicsScene::LEFT || direction == CustomGraphicsScene::RIGHT)
	{
		auto parent_node = target->parent;
		int target_idx;
		for (target_idx = 0; target_idx < parent_node->children.size(); ++target_idx)
		{
			if (parent_node->children[target_idx] == target) break;
		}

		if(parent_node->node_relation == HORIZONTAL)
		{
			if(direction == CustomGraphicsScene::LEFT)
			{
				parent_node->children.insert(parent_node->children.begin() + target_idx, source);
			}
			else if(direction == CustomGraphicsScene::RIGHT)
			{
				parent_node->children.insert(parent_node->children.begin() + target_idx + 1, source);
			}
			source->parent = parent_node;
		}
		else if(parent_node->node_relation == VERTICAL)
		{
			int new_parent_index = GenerateNewIndex(target) + 1;
			auto* new_parent = new CCombineTreeNode();
			new_parent->parent = parent_node;
			new_parent->index = new_parent_index;
			new_parent->node_type = NONLEAF;
			new_parent->node_present_type = NONLABEL;
			new_parent->node_relation = HORIZONTAL;
			parent_node->children.insert(parent_node->children.begin() + target_idx, new_parent);
			parent_node->children.erase(parent_node->children.begin() + target_idx + 1);
			target->parent = new_parent;
			source->parent = new_parent;
			if (direction == CustomGraphicsScene::LEFT )
			{
				new_parent->children = { source, target };
			}
			else if (direction == CustomGraphicsScene::RIGHT)
			{
				new_parent->children = { target, source };
			}
		}
	}
	else if (direction == CustomGraphicsScene::UP || direction == CustomGraphicsScene::BOTTOM)
	{
		auto parent_node = target->parent;
		int target_idx;
		for (target_idx = 0; target_idx < parent_node->children.size(); ++target_idx)
		{
			if (parent_node->children[target_idx] == target) break;
		}

		if (parent_node->node_relation == VERTICAL)
		{
			if (direction == CustomGraphicsScene::UP)
			{
				parent_node->children.insert(parent_node->children.begin() + target_idx, source);
			}
			else if (direction == CustomGraphicsScene::BOTTOM)
			{
				parent_node->children.insert(parent_node->children.begin() + target_idx + 1, source);
			}
			source->parent = parent_node;
		}
		else if (parent_node->node_relation == HORIZONTAL)
		{
			int new_parent_index = GenerateNewIndex(target) + 1;
			auto* new_parent = new CCombineTreeNode();
			new_parent->parent = parent_node;
			new_parent->index = new_parent_index;
			new_parent->node_type = NONLEAF;
			new_parent->node_present_type = NONLABEL;
			new_parent->node_relation = VERTICAL;
			parent_node->children.insert(parent_node->children.begin() + target_idx, new_parent);
			parent_node->children.erase(parent_node->children.begin() + target_idx + 1);
			target->parent = new_parent;
			source->parent = new_parent;
			if (direction == CustomGraphicsScene::UP)
			{
				new_parent->children = { source, target };
			}
			else if (direction == CustomGraphicsScene::BOTTOM)
			{
				new_parent->children = { target, source };
			}
		}
	}
	
	while (target->parent != nullptr) target = target->parent;
	//MergeRelationNode(target);
	//MergeTextNode(target);
    //MergePaddingNode(target);
	//MergeExtraParent(target);
	//OutputStructure(target);
	return target;
}

int CombineTreeHandler::GenerateNewIndex(CCombineTreeNode* node)
{
	int res = 0;
	while(node->parent != nullptr)
	{
		node = node->parent;
	}
	std::queue<CCombineTreeNode*> queue;
	queue.push(node);
	while(!queue.empty())
	{
		auto node = queue.front();
		queue.pop();
		res = std::max(res, node->index);
		for(auto child : node->children)
		{
			queue.push(child);
		}
	}
	return res + 1;
}

CCombineTreeNode* CombineTreeHandler::GenerateCombineTree(std::vector<CLayoutTree*> m_uploadLayout)
{
	CCombineTreeProcessor combine_tree_processor;
	combine_tree_processor.m_allLayout = m_uploadLayout;

	for(auto layout:m_uploadLayout)
	{
		std::queue<CNode*> temp_queue;
		temp_queue.push(layout->GetRoot());
		int this_layer = 1;
		int next_layer = 0;
		std::cout << "tree structure:" << std::endl;
		while (!temp_queue.empty())
		{
			CNode* node = temp_queue.front();
			temp_queue.pop();
			if(node->parent != nullptr) std::cout << node->index << "," << node->node_present_type << "," << node->parent->index << " ";
			else std::cout << node->index << "," << node->node_present_type << "," <<  - node->index << " ";
			
			for (auto iter = node->children.begin(); iter != node->children.end(); iter++)
			{
				temp_queue.push(*iter);
				next_layer++;
			}
			this_layer--;
			if (this_layer == 0)
			{
				this_layer = next_layer;
				next_layer = 0;
				std::cout << std::endl;
			}
		}
		std::cout << std::endl;
	}

	combine_tree_processor.AllFunction();

	std::queue<CCombineTreeNode*> temp_queue;
	temp_queue.push(combine_tree_processor.combine_tree_root);

	while (!temp_queue.empty())
	{
		CCombineTreeNode* node = temp_queue.front();
		temp_queue.pop();
		if (node->tree0_node != nullptr && node->tree1_node != nullptr && node->tree2_node != nullptr)
		{
			node->node_corres_type = NODE_CORRES_TYPE::T_ABC;
			if (node->children.size() == 0)
			{
				if (node->tree0_node->node_present_type == node->tree1_node->node_present_type
					&& node->tree0_node->node_present_type == node->tree2_node->node_present_type)
				{
					//假设这3个节点语义信息相同
					node->node_present_type = node->tree0_node->node_present_type;
				}
				else
				{
					std::cout << "这3节点的语义信息不都相同" << std::endl;
					std::cout << "node index = " << node->index << "    ";
					std::cout << "tree0 label = " << node->tree0_node->node_present_type << "  tree1 label = " << node->tree1_node->node_present_type << "  tree2 label = " << node->tree2_node->node_present_type << std::endl;
				}
			}
			else node->node_present_type = NODE_PRESENT_TYPE::NONLABEL;

		}
		else if (node->tree0_node == nullptr && node->tree1_node != nullptr && node->tree2_node != nullptr)
		{
			node->node_corres_type = NODE_CORRES_TYPE::T_BC;
			if (node->children.size() == 0)
			{
				if (node->tree1_node->node_present_type == node->tree2_node->node_present_type)
				{
					//假设这2个节点语义信息相同
					node->node_present_type = node->tree1_node->node_present_type;
				}
				else
				{
					std::cout << "这2节点的语义信息不都相同" << std::endl;
					std::cout << "node index = " << node->index << "    ";
					std::cout << "tree1 label = " << node->tree1_node->node_present_type << "  tree2 label = " << node->tree2_node->node_present_type << std::endl;
				}
			}
			else node->node_present_type = NODE_PRESENT_TYPE::NONLABEL;
		}
		else if (node->tree0_node != nullptr && node->tree1_node == nullptr && node->tree2_node != nullptr)
		{
			node->node_corres_type = NODE_CORRES_TYPE::T_AC;
			if (node->children.size() == 0)
			{
				if (node->tree0_node->node_present_type == node->tree2_node->node_present_type)
				{
					//假设这2个节点语义信息相同
					node->node_present_type = node->tree0_node->node_present_type;
				}
				else
				{
					std::cout << "这2节点的语义信息不都相同" << std::endl;
					std::cout << "node index = " << node->index << "    ";
					std::cout << "tree0 label = " << node->tree0_node->node_present_type << "  tree2 label = " << node->tree2_node->node_present_type << std::endl;
				}
			}
			else node->node_present_type = NODE_PRESENT_TYPE::NONLABEL;

		}
		else if (node->tree0_node != nullptr && node->tree1_node != nullptr && node->tree2_node == nullptr)
		{
			node->node_corres_type = NODE_CORRES_TYPE::T_AB;
			if (node->children.size() == 0)
			{
				if (node->tree0_node->node_present_type == node->tree1_node->node_present_type)
				{
					//假设这2个节点语义信息相同
					node->node_present_type = node->tree0_node->node_present_type;
				}
				else
				{
					std::cout << "这2节点的语义信息不都相同" << std::endl;
					std::cout << "node index = " << node->index << "    ";
					std::cout << "tree0 label = " << node->tree0_node->node_present_type << "  tree1 label = " << node->tree1_node->node_present_type << std::endl;
				}
			}
			else node->node_present_type = NODE_PRESENT_TYPE::NONLABEL;
		}
		else if (node->tree0_node != nullptr && node->tree1_node == nullptr && node->tree2_node == nullptr)
		{
			node->node_corres_type = NODE_CORRES_TYPE::A;
			if (node->children.size() == 0) node->node_present_type = node->tree0_node->node_present_type;
			else node->node_present_type = NODE_PRESENT_TYPE::NONLABEL;
		}
		else if (node->tree0_node == nullptr && node->tree1_node != nullptr && node->tree2_node == nullptr)
		{
			node->node_corres_type = NODE_CORRES_TYPE::B;
			if (node->children.size() == 0) node->node_present_type = node->tree1_node->node_present_type;
			else node->node_present_type = NODE_PRESENT_TYPE::NONLABEL;
		}
		else if (node->tree0_node == nullptr && node->tree1_node == nullptr && node->tree2_node != nullptr)
		{
			node->node_corres_type = NODE_CORRES_TYPE::C;
			if (node->children.size() == 0) node->node_present_type = node->tree2_node->node_present_type;
			else node->node_present_type = NODE_PRESENT_TYPE::NONLABEL;
		}
		for (auto iter = node->children.begin(); iter != node->children.end(); iter++)
		{
			temp_queue.push(*iter);
		}
	}

	std::cout << "combine tree structure:" << std::endl;
	OutputStructure(combine_tree_processor.combine_tree_root);

	return combine_tree_processor.combine_tree_root;
}

std::vector<CCombineTreeNode*> GetTreeCandidateByRelationsDfs(std::vector<CCombineTreeNode*> &parent_node_list, int index)
{
	std::vector<CCombineTreeNode*> res_list;
	if(index == parent_node_list.size())
	{
		res_list.push_back(parent_node_list[0]->DeepCopy());
	}
	else
	{
		CCombineTreeNode* node = parent_node_list[index];
		bool horizon = false, vertical = false;
		if (node->tree0_node!=nullptr && node->tree0_node->children.size() > 0)
		{
			if (node->tree0_node->top_align_node.size() > 0 && node->tree0_node->bottom_align_node.size() > 0)
			{
				horizon = true;
			}
			else if (node->tree0_node->left_align_node.size() > 0 && node->tree0_node->right_align_node.size() > 0)
			{
				vertical = true;
			}
		}
		if (node->tree1_node != nullptr && node->tree1_node->children.size() > 0)
		{
			if (node->tree1_node->top_align_node.size() > 0 && node->tree1_node->bottom_align_node.size() > 0)
			{
				horizon = true;
			}
			else if (node->tree1_node->left_align_node.size() > 0 && node->tree1_node->right_align_node.size() > 0)
			{
				vertical = true;
			}
		}
		if (node->tree2_node != nullptr && node->tree2_node->children.size() > 0)
		{
			if (node->tree2_node->top_align_node.size() > 0 && node->tree2_node->bottom_align_node.size() > 0)
			{
				horizon = true;
			}
			else if (node->tree2_node->left_align_node.size() > 0 && node->tree2_node->right_align_node.size() > 0)
			{
				vertical = true;
			}
		}
		if (horizon)
		{
			/*for (auto child : node->children)
			{
				node->top_align_node.push_back(child->index);
				node->bottom_align_node.push_back(child->index);
			}*/
			node->node_relation = NODE_RELATION::HORIZONTAL;
			std::vector<CCombineTreeNode*> horizon_list = GetTreeCandidateByRelationsDfs(parent_node_list,index + 1);
			for(auto item:horizon_list)
			{
				res_list.push_back(item);
			}
			node->node_relation = NODE_RELATION::NONE;
			/*node->left_align_node.clear();
			node->right_align_node.clear();*/
		}
		if (vertical)
		{
			/*for (auto child : node->children)
			{
				node->left_align_node.push_back(child->index);
				node->right_align_node.push_back(child->index);
			}*/
			node->node_relation = NODE_RELATION::VERTICAL;
			std::vector<CCombineTreeNode*> vertical_list = GetTreeCandidateByRelationsDfs(parent_node_list, index + 1);
			for (auto item : vertical_list)
			{
				res_list.push_back(item);
			}
			node->node_relation = NODE_RELATION::NONE;
			/*node->left_align_node.clear();
			node->right_align_node.clear();*/
		}
	}
	return res_list;
}

std::vector<CCombineTreeNode*> CombineTreeHandler::GetTreeCandidateByRelations(CCombineTreeNode* combine_tree_root)
{
	std::queue<CCombineTreeNode*> temp_queue;
	std::vector<CCombineTreeNode*> parent_node_list;

	temp_queue.push(combine_tree_root);
	while (!temp_queue.empty())
	{
		CCombineTreeNode* node = temp_queue.front();
		temp_queue.pop();
		if(node->children.size()>0)	parent_node_list.push_back(node);
		for (auto iter = node->children.begin(); iter != node->children.end(); iter++)	temp_queue.push(*iter);
	}
	return GetTreeCandidateByRelationsDfs(parent_node_list, 0);
}

int CombineTreeHandler::TreeColNum(CCombineTreeNode* tree)
{
	if(tree->node_present_type != NODE_PRESENT_TYPE::NONLABEL)
	{
		return 1;
	}
	int res = 0;
	if(tree->node_relation == NODE_RELATION::HORIZONTAL)
	{
		for (int i = 0; i < tree->children.size(); ++i)
		{
			res += TreeColNum(tree->children[i]);
		}
	}
	else if(tree->node_relation == NODE_RELATION::VERTICAL)
	{
		for (int i = 0; i < tree->children.size(); ++i)
		{
			res = std::max(res, TreeColNum(tree->children[i]));
		}
	}
	return res;
}

int CombineTreeHandler::CountDistanceBetweenTwoTrees(CCombineTreeNode* tree1, CCombineTreeNode* tree2)
{
	std::vector<CCombineTreeNode*> traverse_list1, traverse_list2;
	int max_level1 = FindMaxLevel(tree1);
	int max_level2 = FindMaxLevel(tree2);
	std::vector<int> level_list1;
	std::vector<int> level_list2;
	TraverseTree(tree1, traverse_list1, level_list1, max_level1);
	TraverseTree(tree2, traverse_list2, level_list2, max_level2);
	std::vector<CCombineTreeNode*> left_node_list1;
	std::vector<CCombineTreeNode*> left_node_list2;
	FindLeftNodes(traverse_list1, left_node_list1);
	FindLeftNodes(traverse_list2, left_node_list2);
	
	std::vector<std::vector<int>> tree_dist(traverse_list1.size(), std::vector<int>(traverse_list2.size(), 0));
	std::vector<std::vector<std::pair<CCombineTreeNode*, int>>> key_roots1, key_roots2;


	std::unordered_map<CCombineTreeNode*, int> key_root_idx;
	for(int i=0;i<left_node_list1.size();++i)
	{
		CCombineTreeNode* left_node= left_node_list1[i];
		if(key_root_idx.count(left_node) != 0)
		{
			int idx = key_root_idx[left_node];
			if(level_list1[idx] < level_list1[i])
			{
				key_root_idx[left_node] = i;
			}
		}
		else
		{
			key_root_idx[left_node] = i;
		}
	}


	OutputStructure(tree1);
	for (auto item : traverse_list1) std::cout << item->index << " ";
	std::cout << std::endl;
	for (auto item : level_list1) std::cout << item << " ";
	std::cout << std::endl;
	for (auto item : left_node_list1) std::cout << item->index<< " ";
	std::cout << std::endl;
	for (auto item : key_root_idx) std::cout << item.second <<" "<<item.first->index << std::endl;

	return 0;
	
}

void CombineTreeHandler::TraverseTree(CCombineTreeNode* tree, std::vector<CCombineTreeNode*>& traverse_list, std::vector<int>& level_list, int level)
{
	for(int i=0; i < tree->children.size();++i)
	{
		TraverseTree(tree->children[i], traverse_list,level_list,level - 1);
	}
	traverse_list.push_back(tree);
	level_list.push_back(level);
}

void CombineTreeHandler::FindLeftNodes(std::vector<CCombineTreeNode*>& traverse_list,
	std::vector<CCombineTreeNode*>& left_node_list)
{
	for(int i = 0; i < traverse_list.size(); ++i)
	{
		CCombineTreeNode* node = traverse_list[i];
		while(node->node_present_type == NODE_PRESENT_TYPE::NONLABEL)
		{
			CCombineTreeNode* first_non_leaf = nullptr;
			for(int j = 0; j < node->children.size(); ++j)
			{
				if (node->children[j]->node_present_type == NODE_PRESENT_TYPE::NONLABEL)
				{
					first_non_leaf = node->children[j];
					break;
				}
			}
			if (first_non_leaf != nullptr) node = first_non_leaf;
			else node = node->children[0];
		}
		left_node_list.push_back(node);
	}
}


int CombineTreeHandler::FindMaxLevel(CCombineTreeNode* tree)
{
	std::queue<CCombineTreeNode*> temp_queue;
	int max_level = 0;
	temp_queue.push(tree);
	int children_num = 1;
	int next_children_num = 0;
	while(!temp_queue.empty())
	{
		auto node = temp_queue.front();
		temp_queue.pop();
		for(int  i= 0;i < node->children.size();++i)
		{
			temp_queue.push(node->children[i]);
			next_children_num++;
		}
		children_num--;
		if (children_num == 0)
		{
			children_num = next_children_num;
			next_children_num = 0;
			max_level++;
		}
	}
	return max_level;
}

void CombineTreeHandler::MultiPageFindSolutions(std::vector<int>& limit, std::vector<int>& solution, std::vector<std::vector<int>>& solution_list, int img_num,
	int img_left)
{
	if(img_left == 0 && img_num == limit.size())
	{
		solution_list.push_back(solution);
	}
	if (img_num == limit.size()) return;
	for(int i = 0; i <= limit[img_num];++i)
	{
		int new_img_left = img_left - i * img_num;
		if (new_img_left >= 0)
		{
			solution[img_num] = i;
			MultiPageFindSolutions(limit, solution, solution_list, img_num + 1, new_img_left);
		}
		else
		{		
			solution[img_num] = 0;
			MultiPageFindSolutions(limit, solution, solution_list, img_num + 1, img_left);
			break;	
		}
	}

}

void CombineTreeHandler::CutTitleFromATree(CCombineTreeNode* tree)
{
	std::queue<CCombineTreeNode*> queue;
	queue.push(tree);
	while(!queue.empty())
	{
		CCombineTreeNode* node = queue.front();
		queue.pop();

		if (node->node_present_type == NODE_PRESENT_TYPE::TITLE)
		{
			CutTreeNode(node);
			break;
		}
		for(int i=0; i < node->children.size();++i)
		{
			queue.push(node->children[i]);
		}
	}
	while (!queue.empty()) queue.pop();
}


void CombineTreeHandler::DeterminOrder(std::vector<CCombineTreeNode*> &tree_list)
{
	for (auto tree : tree_list)
	{
		const int min_height = 50;
		const int min_width = 50;
		double alpha, beta, gamma;
		alpha = 1.0 / 3.0;
		beta = 1.0 / 3.0;
		gamma = 1.0 - alpha - beta;
		double root_x = tree->tree0_node->x * alpha + tree->tree1_node->x * beta + tree->tree2_node->x * gamma;
		double root_y = tree->tree0_node->y * alpha + tree->tree1_node->y * beta + tree->tree2_node->y * gamma;
		double root_width = tree->tree0_node->width * alpha + tree->tree1_node->width * beta + tree->tree2_node->width * gamma;
		double root_height = tree->tree0_node->height * alpha + tree->tree1_node->height * beta + tree->tree2_node->height * gamma;

		//supposed that BFS put the nodes in.
		std::vector<CCombineTreeNode*> allnodes;
		std::queue<CCombineTreeNode*> combine_q;
		std::unordered_map<CCombineTreeNode*, int> ptr_idx_map;
		std::unordered_map<int, CCombineTreeNode*> idx_ptr_map;

		/*int counts_left_align_node = 0;
		int counts_right_align_node = 0;
		int counts_top_align_node = 0;
		int counts_bottom_align_node = 0;*/
		int counts_horizontal_align = 0;
		int counts_vertical_align = 0;

		combine_q.push(tree);
		while (!combine_q.empty())
		{
			//注意：我的combine tree index从1开始
			idx_ptr_map[combine_q.front()->index] = combine_q.front();
			ptr_idx_map[combine_q.front()] = allnodes.size();	//根据ptr映射出在allnodes中的order;

			allnodes.push_back(combine_q.front());
			combine_q.pop();
			////std::cout << allnodes.back()->index << std::endl;
			for (auto ptr : allnodes.back()->children)
			{
				combine_q.push(ptr);
			}
			if (allnodes.back()->node_relation == NODE_RELATION::HORIZONTAL)
			{
				counts_horizontal_align += allnodes.back()->children.size() - 1;
			}
			else if(allnodes.back()->node_relation == NODE_RELATION::VERTICAL)
			{
				counts_vertical_align += allnodes.back()->children.size() - 1;
			}
			/*if (allnodes.back()->left_align_node.size() > 0)
			{
				counts_left_align_node += allnodes.back()->left_align_node.size() - 1;
			}
			if (allnodes.back()->right_align_node.size() > 0)
			{
				counts_right_align_node += allnodes.back()->right_align_node.size() - 1;
			}
			if (allnodes.back()->top_align_node.size() > 0)
			{
				counts_top_align_node += allnodes.back()->top_align_node.size() - 1;
			}
			if (allnodes.back()->bottom_align_node.size() > 0)
			{
				counts_bottom_align_node += allnodes.back()->bottom_align_node.size() - 1;
			}*/
		}

		int constraint_num = counts_horizontal_align * 2 + counts_vertical_align * 2 + 4; 		//4 这个硬约束是为了实现最外层的bounding_box直接插值出来

		Eigen::MatrixXd H(allnodes.size() * 4, allnodes.size() * 4);
		H.setIdentity();

		for (int i = 0; i < allnodes.size(); i++)
		{
			if (allnodes[i]->node_corres_type == NODE_CORRES_TYPE::T_ABC)
			{
				H(i * 4, i * 4) = 2 * H(i * 4, i * 4);
				H(i * 4 + 1, i * 4 + 1) = 2 * H(i * 4 + 1, i * 4 + 1);
				H(i * 4 + 2, i * 4 + 2) = 2 * H(i * 4 + 2, i * 4 + 2);
				H(i * 4 + 3, i * 4 + 3) = 2 * H(i * 4 + 3, i * 4 + 3);
			}
			else if (allnodes[i]->node_corres_type == NODE_CORRES_TYPE::T_AB)
			{
				H(i * 4, i * 4) = 2 * (alpha + beta) * H(i * 4, i * 4);
				H(i * 4 + 1, i * 4 + 1) = 2 * (alpha + beta) * H(i * 4 + 1, i * 4 + 1);
				H(i * 4 + 2, i * 4 + 2) = 2 * H(i * 4 + 2, i * 4 + 2);
				H(i * 4 + 3, i * 4 + 3) = 2 * H(i * 4 + 3, i * 4 + 3);
			}
			else if (allnodes[i]->node_corres_type == NODE_CORRES_TYPE::T_AC)
			{
				H(i * 4, i * 4) = 2 * (alpha + gamma) * H(i * 4, i * 4);
				H(i * 4 + 1, i * 4 + 1) = 2 * (alpha + gamma) * H(i * 4 + 1, i * 4 + 1);
				H(i * 4 + 2, i * 4 + 2) = 2 * H(i * 4 + 2, i * 4 + 2);
				H(i * 4 + 3, i * 4 + 3) = 2 * H(i * 4 + 3, i * 4 + 3);
			}
			else if (allnodes[i]->node_corres_type == NODE_CORRES_TYPE::T_BC)
			{
				H(i * 4, i * 4) = 2 * (beta + gamma) * H(i * 4, i * 4);
				H(i * 4 + 1, i * 4 + 1) = 2 * (beta + gamma) * H(i * 4 + 1, i * 4 + 1);
				H(i * 4 + 2, i * 4 + 2) = 2 * H(i * 4 + 2, i * 4 + 2);
				H(i * 4 + 3, i * 4 + 3) = 2 * H(i * 4 + 3, i * 4 + 3);
			}
			else if (allnodes[i]->node_corres_type == NODE_CORRES_TYPE::A)
			{
				H(i * 4, i * 4) = 2 * alpha * H(i * 4, i * 4);
				H(i * 4 + 1, i * 4 + 1) = 2 * alpha * H(i * 4 + 1, i * 4 + 1);
				H(i * 4 + 2, i * 4 + 2) = 2 * H(i * 4 + 2, i * 4 + 2);
				H(i * 4 + 3, i * 4 + 3) = 2 * H(i * 4 + 3, i * 4 + 3);
			}
			else if (allnodes[i]->node_corres_type == NODE_CORRES_TYPE::B)
			{
				H(i * 4, i * 4) = 2 * beta * H(i * 4, i * 4);
				H(i * 4 + 1, i * 4 + 1) = 2 * beta * H(i * 4 + 1, i * 4 + 1);
				H(i * 4 + 2, i * 4 + 2) = 2 * H(i * 4 + 2, i * 4 + 2);
				H(i * 4 + 3, i * 4 + 3) = 2 * H(i * 4 + 3, i * 4 + 3);
			}
			else if (allnodes[i]->node_corres_type == NODE_CORRES_TYPE::C)
			{
				H(i * 4, i * 4) = 2 * gamma * H(i * 4, i * 4);
				H(i * 4 + 1, i * 4 + 1) = 2 * gamma * H(i * 4 + 1, i * 4 + 1);
				H(i * 4 + 2, i * 4 + 2) = 2 * H(i * 4 + 2, i * 4 + 2);
				H(i * 4 + 3, i * 4 + 3) = 2 * H(i * 4 + 3, i * 4 + 3);
			}

		}

		Eigen::VectorXd b(constraint_num);
		b.setZero();

		Eigen::MatrixXd A(constraint_num, 4 * allnodes.size());
		A.setZero();

		int row_count = 0;

		//添加直接插值的硬约束
		int root_order = ptr_idx_map[idx_ptr_map[tree->index]];
		int x_location_root = root_order * 4 + 1 - 1;
		int y_location_root = root_order * 4 + 2 - 1;
		int width_location_root = root_order * 4 + 3 - 1;
		int height_location_root = root_order * 4 + 4 - 1;

		A(row_count, x_location_root) = 1;
		row_count++;
		b(0) = root_x;

		A(row_count, y_location_root) = 1;
		row_count++;
		b(1) = root_y;

		A(row_count, width_location_root) = 1;
		row_count++;
		b(2) = root_width;

		A(row_count, height_location_root) = 1;
		row_count++;
		b(3) = root_height;

		//horizontal
		for (int i = 0; i < allnodes.size(); i++)
		{
			if (allnodes[i]->node_relation == NODE_RELATION::HORIZONTAL)
			{
				for (int j = 0; j < allnodes[i]->children.size() - 1; j++)
				{
					int order_first = ptr_idx_map[idx_ptr_map[allnodes[i]->children[j]->index]];
					int order_second = ptr_idx_map[idx_ptr_map[allnodes[i]->children[j + 1]->index]];

					int y_location_first = order_first * 4 + 2 - 1;
					int y_location_second = order_second * 4 + 2 - 1;
					int h_location_first = order_first * 4 + 4 - 1;
					int h_location_second = order_second * 4 + 4 - 1;

					A(row_count, y_location_first) = 1;
					A(row_count, y_location_second) = -1;
					row_count++;

					A(row_count, y_location_first) = 1;
					A(row_count, y_location_second) = -1;
					A(row_count, h_location_first) = 1;
					A(row_count, h_location_second) = -1;
					row_count++;

				}
			}
		}

		//vertical
		for (int i = 0; i < allnodes.size(); i++)
		{
			if (allnodes[i]->node_relation == NODE_RELATION::VERTICAL)
			{
				for (int j = 0; j < allnodes[i]->children.size() - 1; j++)
				{
					int order_first = ptr_idx_map[idx_ptr_map[allnodes[i]->children[j]->index]];
					int order_second = ptr_idx_map[idx_ptr_map[allnodes[i]->children[j + 1]->index]];
					int x_location_first = order_first * 4 + 1 - 1;
					int x_location_second = order_second * 4 + 1 - 1;
					int width_location_first = order_first * 4 + 3 - 1;
					int width_location_second = order_second * 4 + 3 - 1;

					A(row_count, x_location_first) = 1;
					A(row_count, x_location_second) = -1;
					row_count++;

					A(row_count, x_location_first) = 1;
					A(row_count, x_location_second) = -1;
					A(row_count, width_location_first) = 1;
					A(row_count, width_location_second) = -1;
					row_count++;
				}
			}
		}

		Eigen::VectorXd c(4 * allnodes.size());

		for (int i = 0; i < allnodes.size(); i++)
		{
			std::vector<int> position_size_one;		//存放位置和大小信息
			std::vector<int> position_size_two;
			std::vector<int> position_size_three;

			if (allnodes[i]->node_corres_type == NODE_CORRES_TYPE::T_ABC)
			{
				position_size_one.push_back(allnodes[i]->tree0_node->x);
				position_size_one.push_back(allnodes[i]->tree0_node->y);
				position_size_one.push_back(allnodes[i]->tree0_node->width);
				position_size_one.push_back(allnodes[i]->tree0_node->height);

				position_size_two.push_back(allnodes[i]->tree1_node->x);
				position_size_two.push_back(allnodes[i]->tree1_node->y);
				position_size_two.push_back(allnodes[i]->tree1_node->width);
				position_size_two.push_back(allnodes[i]->tree1_node->height);

				position_size_three.push_back(allnodes[i]->tree2_node->x);
				position_size_three.push_back(allnodes[i]->tree2_node->y);
				position_size_three.push_back(allnodes[i]->tree2_node->width);
				position_size_three.push_back(allnodes[i]->tree2_node->height);
			}
			else if (allnodes[i]->node_corres_type == NODE_CORRES_TYPE::T_AB)
			{
				position_size_one.push_back(allnodes[i]->tree0_node->x);
				position_size_one.push_back(allnodes[i]->tree0_node->y);
				position_size_one.push_back(allnodes[i]->tree0_node->width);
				position_size_one.push_back(allnodes[i]->tree0_node->height);

				position_size_two.push_back(allnodes[i]->tree1_node->x);
				position_size_two.push_back(allnodes[i]->tree1_node->y);
				position_size_two.push_back(allnodes[i]->tree1_node->width);
				position_size_two.push_back(allnodes[i]->tree1_node->height);

				position_size_three.push_back(0);
				position_size_three.push_back(0);
				position_size_three.push_back(min_width);
				position_size_three.push_back(min_height);
			}
			else if (allnodes[i]->node_corres_type == NODE_CORRES_TYPE::T_AC)
			{
				position_size_one.push_back(allnodes[i]->tree0_node->x);
				position_size_one.push_back(allnodes[i]->tree0_node->y);
				position_size_one.push_back(allnodes[i]->tree0_node->width);
				position_size_one.push_back(allnodes[i]->tree0_node->height);

				position_size_two.push_back(0);
				position_size_two.push_back(0);
				position_size_two.push_back(min_width);
				position_size_two.push_back(min_height);

				position_size_three.push_back(allnodes[i]->tree2_node->x);
				position_size_three.push_back(allnodes[i]->tree2_node->y);
				position_size_three.push_back(allnodes[i]->tree2_node->width);
				position_size_three.push_back(allnodes[i]->tree2_node->height);
			}
			else if (allnodes[i]->node_corres_type == NODE_CORRES_TYPE::T_BC)
			{
				position_size_one.push_back(0);
				position_size_one.push_back(0);
				position_size_one.push_back(min_width);
				position_size_one.push_back(min_height);

				position_size_two.push_back(allnodes[i]->tree1_node->x);
				position_size_two.push_back(allnodes[i]->tree1_node->y);
				position_size_two.push_back(allnodes[i]->tree1_node->width);
				position_size_two.push_back(allnodes[i]->tree1_node->height);

				position_size_three.push_back(allnodes[i]->tree2_node->x);
				position_size_three.push_back(allnodes[i]->tree2_node->y);
				position_size_three.push_back(allnodes[i]->tree2_node->width);
				position_size_three.push_back(allnodes[i]->tree2_node->height);
			}
			else if (allnodes[i]->node_corres_type == NODE_CORRES_TYPE::A)
			{
				position_size_one.push_back(allnodes[i]->tree0_node->x);
				position_size_one.push_back(allnodes[i]->tree0_node->y);
				position_size_one.push_back(allnodes[i]->tree0_node->width);
				position_size_one.push_back(allnodes[i]->tree0_node->height);

				position_size_two.push_back(0);
				position_size_two.push_back(0);
				position_size_two.push_back(min_width);
				position_size_two.push_back(min_height);

				position_size_three.push_back(0);
				position_size_three.push_back(0);
				position_size_three.push_back(min_width);
				position_size_three.push_back(min_height);
			}
			else if (allnodes[i]->node_corres_type == NODE_CORRES_TYPE::B)
			{
				position_size_one.push_back(0);
				position_size_one.push_back(0);
				position_size_one.push_back(min_width);
				position_size_one.push_back(min_height);

				position_size_two.push_back(allnodes[i]->tree1_node->x);
				position_size_two.push_back(allnodes[i]->tree1_node->y);
				position_size_two.push_back(allnodes[i]->tree1_node->width);
				position_size_two.push_back(allnodes[i]->tree1_node->height);

				position_size_three.push_back(0);
				position_size_three.push_back(0);
				position_size_three.push_back(min_width);
				position_size_three.push_back(min_height);
			}
			else if (allnodes[i]->node_corres_type == NODE_CORRES_TYPE::C)
			{
				// C
				position_size_one.push_back(0);
				position_size_one.push_back(0);
				position_size_one.push_back(min_width);
				position_size_one.push_back(min_height);

				position_size_two.push_back(0);
				position_size_two.push_back(0);
				position_size_two.push_back(min_width);
				position_size_two.push_back(min_height);

				position_size_three.push_back(allnodes[i]->tree2_node->x);
				position_size_three.push_back(allnodes[i]->tree2_node->y);
				position_size_three.push_back(allnodes[i]->tree2_node->width);
				position_size_three.push_back(allnodes[i]->tree2_node->height);
			}

			if (allnodes[i]->node_corres_type == NODE_CORRES_TYPE::T_ABC)
			{
				for (int j = 0; j < 4; j++)
				{
					c(4 * i + j) = -2 * alpha * position_size_one[j] - 2 * beta * position_size_two[j] - 2 * gamma * position_size_three[j];
				}
			}
			else if (allnodes[i]->node_corres_type == NODE_CORRES_TYPE::T_AB)
			{
				for (int j = 0; j < 4; j++)
				{
					if (j < 2)
					{
						c(4 * i + j) = -2 * alpha * position_size_one[j] - 2 * beta * position_size_two[j];
					}
					else
					{
						c(4 * i + j) = -2 * alpha * position_size_one[j] - 2 * beta * position_size_two[j] - 2 * gamma * position_size_three[j];
					}
				}
			}
			else if (allnodes[i]->node_corres_type == NODE_CORRES_TYPE::T_AC)
			{
				for (int j = 0; j < 4; j++)
				{
					if (j < 2)
					{
						c(4 * i + j) = -2 * alpha * position_size_one[j] - 2 * gamma * position_size_three[j];
					}
					else
					{
						c(4 * i + j) = -2 * alpha * position_size_one[j] - 2 * beta * position_size_two[j] - 2 * gamma * position_size_three[j];
					}
				}
			}
			else if (allnodes[i]->node_corres_type == NODE_CORRES_TYPE::T_BC)
			{
				for (int j = 0; j < 4; j++)
				{
					if (j < 2)
					{
						c(4 * i + j) = -2 * beta * position_size_two[j] - 2 * gamma * position_size_three[j];
					}
					else
					{
						c(4 * i + j) = -2 * alpha * position_size_one[j] - 2 * beta * position_size_two[j] - 2 * gamma * position_size_three[j];
					}
				}
			}
			else if (allnodes[i]->node_corres_type == NODE_CORRES_TYPE::A)
			{
				for (int j = 0; j < 4; j++)
				{
					if (j < 2)
					{
						c(4 * i + j) = -2 * alpha * position_size_one[j];
					}
					else
					{
						c(4 * i + j) = -2 * alpha * position_size_one[j] - 2 * beta * position_size_two[j] - 2 * gamma * position_size_three[j];
					}
				}
			}
			else if (allnodes[i]->node_corres_type == NODE_CORRES_TYPE::B)
			{
				for (int j = 0; j < 4; j++)
				{
					if (j < 2)
					{
						c(4 * i + j) = -2 * beta * position_size_two[j];
					}
					else
					{
						c(4 * i + j) = -2 * alpha * position_size_one[j] - 2 * beta * position_size_two[j] - 2 * gamma * position_size_three[j];
					}
				}
			}
			else
			{
				for (int j = 0; j < 4; j++)
				{
					if (j < 2)
					{
						c(4 * i + j) = -2 * gamma * position_size_three[j];
					}
					else
					{
						c(4 * i + j) = -2 * alpha * position_size_one[j] - 2 * beta * position_size_two[j] - 2 * gamma * position_size_three[j];
					}
				}
			}

		}

		Eigen::MatrixXd H_Inverse, A_HInver_ATrans, Inverse_A_HInver_ATrans, A_HInver, G, B, C;
		Eigen::VectorXd result, lam, fval;

		H_Inverse = H.inverse();

		A_HInver_ATrans = A * H_Inverse * A.transpose();

		//求广义逆
		//(A_HInver_ATrans)不满秩时，无法直接求逆，用广义逆来解决。
		double epsilon = std::numeric_limits<double>::epsilon();
		Eigen::JacobiSVD< Eigen::MatrixXd > svd(A_HInver_ATrans, Eigen::ComputeFullU | Eigen::ComputeFullV);
		double tolerance = epsilon * std::max(A_HInver_ATrans.cols(), A_HInver_ATrans.rows()) * svd.singularValues().array().abs()(0);
		Inverse_A_HInver_ATrans = svd.matrixV() * (svd.singularValues().array().abs() > tolerance).select(svd.singularValues().array().inverse(), 0).matrix().asDiagonal() * svd.matrixU().adjoint();


		A_HInver = A * H_Inverse;

		C = -(Inverse_A_HInver_ATrans);

		G = H_Inverse - H_Inverse * A.transpose() * Inverse_A_HInver_ATrans * A_HInver;
		B = Inverse_A_HInver_ATrans * A_HInver;

		result = B.transpose() * b - G * c;

		for (int i = 0; i < result.rows() / 4; i++)
		{
			allnodes[i]->x = result(4 * i);
			allnodes[i]->y = result(4 * i + 1);
			allnodes[i]->width = result(4 * i + 2);
			allnodes[i]->height = result(4 * i + 3);
		}
		std::queue<CCombineTreeNode*> temp_queue;
		temp_queue.push(tree);
		while (!temp_queue.empty())
		{
			auto node = temp_queue.front();
			temp_queue.pop();
			if (node->children.size() > 0)
			{
				if (node->node_relation == NODE_RELATION::HORIZONTAL)
					std::sort(node->children.begin(), node->children.end(), CombineNodeCompareX);
				else if (node->node_relation == NODE_RELATION::VERTICAL)
					std::sort(node->children.begin(), node->children.end(), CombineNodeCompareY);
			}
			for (auto iter = node->children.begin(); iter != node->children.end(); iter++)
			{
				temp_queue.push(*iter);
			}
		}
	}
}

void CombineTreeHandler::MergeTextNode(CCombineTreeNode* tree)
{
	std::queue<CCombineTreeNode*> temp_queue;
	temp_queue.push(tree);
	while (!temp_queue.empty())
	{
		auto node = temp_queue.front();
		temp_queue.pop();
		if (node->node_relation == NODE_RELATION::VERTICAL)
		{
			std::vector<CCombineTreeNode*> new_children = {};
			if (node->children.size() > 0) new_children.push_back(node->children[0]);
			for (int i = 1; i < node->children.size(); ++i)
			{
				auto child = node->children[i];
				auto pre_child = new_children.back();
				if (pre_child->node_present_type == NODE_PRESENT_TYPE::TEXT &&
					child->node_present_type == NODE_PRESENT_TYPE::TEXT)
				{
					continue;
				}
				new_children.push_back(child);
			}
			node->children = new_children;
			//node->left_align_node.clear();
			//node->right_align_node.clear();
			/*for(auto new_child : new_children)
			{
				node->left_align_node.push_back(new_child->index);
				node->right_align_node.push_back(new_child->index);
			}*/
		}
		for (auto child : node->children)	temp_queue.push(child);
	}
}

void CombineTreeHandler::MergePaddingNode(CCombineTreeNode* tree)
{
    std::queue<CCombineTreeNode*> temp_queue;
    temp_queue.push(tree);
    while (!temp_queue.empty())
    {
        auto node = temp_queue.front();
        temp_queue.pop();
        if (node->node_present_type == NODE_PRESENT_TYPE::NONLABEL)
        {
            std::vector<CCombineTreeNode*> new_children = {};
            if (node->children.size() > 0) new_children.push_back(node->children[0]);
            for (int i = 1; i < node->children.size(); ++i)
            {
                auto child = node->children[i];
                auto pre_child = new_children.back();
                if (pre_child->node_present_type == NODE_PRESENT_TYPE::PADDING &&
                    child->node_present_type == NODE_PRESENT_TYPE::PADDING)
                {
                    continue;
                }
                new_children.push_back(child);
            }
            node->children = new_children;
        }
        for (auto child : node->children)	temp_queue.push(child);
    }
}


void CombineTreeHandler::MergeRelationNode(CCombineTreeNode* tree)
{
	std::queue<CCombineTreeNode*> temp_queue;
	temp_queue.push(tree);
	while (!temp_queue.empty())
	{
		auto node = temp_queue.front();
		temp_queue.pop();
		NODE_RELATION parent_relation = node->node_relation;
		bool cut_child_exist = true;
		while(cut_child_exist)
		{
			int cut_child_index = -1;
			for (int i = 0; i < node->children.size(); ++i)
			{
				if(node->children[i]->node_relation == parent_relation)
				{
					cut_child_exist = true;
					cut_child_index = i;
					break;
				}
			}
			if(cut_child_index == -1)
			{
				cut_child_exist = false;
			}
			else
			{
				int insert_index = cut_child_index;
				CCombineTreeNode* cut_child = node->children[cut_child_index];
				node->children.erase(node->children.begin() + cut_child_index);
				for(auto new_child : cut_child->children)
				{
					node->children.emplace(node->children.begin() + insert_index, new_child);
					new_child->parent = node;
					insert_index++;
				}
			}
		}
		for (auto child : node->children)
		{
			if(child->children.size() > 0) temp_queue.push(child);
		}
	}
}

void CombineTreeHandler::GetParentNode(CCombineTreeNode* combine_tree_root)
{
	if (combine_tree_root)
	{
		std::queue<CCombineTreeNode*> temp_queue;
		temp_queue.push(combine_tree_root);
		while (!temp_queue.empty())
		{
			CCombineTreeNode* node = temp_queue.front();
			parent_list.push_back(node);
			temp_queue.pop();
			for (auto child : node->children)
			{
				if (child->children.size() > 0)
				{
					temp_queue.push(child);
				}
			}
		}
	}
}

std::vector<CCombineTreeNode*> CombineTreeHandler::GetCombineTreeCandidate(CCombineTreeNode* combine_tree, int img_num, int text_num_needed)
{
	std::vector<CCombineTreeNode*> relations_candidates  = GetTreeCandidateByRelations(combine_tree);
	DeterminOrder(relations_candidates);
	std::cout <<"由关系决定的结构数量：" << relations_candidates.size() << std::endl;
	for (auto candidate : relations_candidates)
	{
		MergeExtraParent(candidate);
		MergeRelationNode(candidate);
		//OutputStructure(candidate);
	}
	/*pic_node_list.clear();
	parent_list.clear();
	pic_node_delete_list.clear();
	pic_node_delete_temp.clear();
	GetParentNode(combine_tree);
	int pic_node = GetPicNode(combine_tree);
	std::cout << "原树的图片节点数量：" << pic_node << " " << "输入的图片元素数量：" << img_num << std::endl;*/
	//for (auto tree : relations_candidates) OutputStructure(tree);
	std::vector<CCombineTreeNode*> combine_tree_candidate_cut_pic;
	//统计原tree的picture node个数
	for (auto tree_root : relations_candidates)
	{
		pic_node_list.clear();
		parent_list.clear();
		pic_node_delete_list.clear();
		pic_node_delete_temp.clear();
		GetParentNode(tree_root);
		int pic_node = GetPicNode(tree_root);
		if (pic_node == img_num)//原树就是唯一的候选
		{
			combine_tree_candidate_cut_pic.push_back(tree_root);
		}
		else if (pic_node < img_num)
		{
			std::cout << "too much img in content." << std::endl;
		}
		else DfsPic(0, pic_node - img_num);
		for (int i = 0; i < pic_node_delete_list.size(); i++)//第i棵候选子树
		{
			CCombineTreeNode* root_candi = tree_root->DeepCopy();
			combine_tree_candidate_cut_pic.push_back(root_candi);
			std::queue<CCombineTreeNode*> temp_queue;
			temp_queue.push(root_candi);
			while (!temp_queue.empty())
			{
				CCombineTreeNode* node = temp_queue.front();
				temp_queue.pop();
				if (pic_node_delete_list[i].count(node->index) == 1)
				{
					CutTreeNode(node);
				}
				for (auto iter = node->children.begin(); iter != node->children.end(); iter++)
					temp_queue.push(*iter);
			}
		}
	}

	for(auto candidate:combine_tree_candidate_cut_pic)
	{
        MergeTextNode(candidate);
        MergePaddingNode(candidate);
		MergeExtraParent(candidate);
	}
	CutExtraTree(combine_tree_candidate_cut_pic);
	std::cout << "删去图片后的结构数量：" << combine_tree_candidate_cut_pic.size() << std::endl;

	int title_num = 1;//remove title here fucker
	std::vector<CCombineTreeNode*> combine_tree_candidate_cut_title;
	for (auto tree_root : combine_tree_candidate_cut_pic)
	{
		title_node_list.clear();
		parent_list.clear();
		title_node_delete_list.clear();
		title_node_delete_temp.clear();
		GetParentNode(tree_root);
		int title_node = GetTitleNode(tree_root);
		if (title_node == title_num)//原树就是唯一的候选
		{
			combine_tree_candidate_cut_title.push_back(tree_root);
		}
		else if (title_node < title_num)
		{
			std::cout << "too much title in content." << std::endl;
		}
		else DfsTitle(0, title_node - title_num);
		for (int i = 0; i < title_node_delete_list.size(); i++)//第i棵候选子树
		{
			CCombineTreeNode* root_candi = tree_root->DeepCopy();
			combine_tree_candidate_cut_title.push_back(root_candi);
			std::queue<CCombineTreeNode*> temp_queue;
			temp_queue.push(root_candi);
			while (!temp_queue.empty())
			{
				CCombineTreeNode* node = temp_queue.front();
				temp_queue.pop();
				if (title_node_delete_list[i].count(node->index) == 1)
				{
					CutTreeNode(node);
				}
				for (auto iter = node->children.begin(); iter != node->children.end(); iter++)
					temp_queue.push(*iter);
			}
		}
	}

	for (auto candidate : combine_tree_candidate_cut_title)
	{
		MergeTextNode(candidate);
        MergePaddingNode(candidate);
        MergeExtraParent(candidate);
	}
	CutExtraTree(combine_tree_candidate_cut_title);
	std::cout << "删去标题后的结构数量：" << combine_tree_candidate_cut_title.size() << std::endl;

	//int text_num;
	//text_node_delete_list.clear();
	//text_node_delete_temp.clear();
	//int text_node = GetTextNode(combine_tree);
	//std::cout << "原树的文本节点数量：" << text_node << std::endl;
	//for (auto tree : relations_candidates) OutputStructure(tree);
	std::vector<CCombineTreeNode*> combine_tree_candidate_cut_text;


	//for(int i = 0; i < combine_tree_candidate_cut_title.size();++i)
	//{
	//	combine_tree_candidate_cut_text.push_back(combine_tree_candidate_cut_title[i]);
	//}

	//for (text_num = 1; text_num < text_node; ++text_num)
	//{
		//DfsText(0, text_node - text_num);
	//}

	for (auto tree_root : combine_tree_candidate_cut_title)
	{
		//int text_num;
		text_node_list.clear();
		text_node_delete_list.clear();
		text_node_delete_temp.clear();
		parent_list.clear();
		//MergeTextNode(tree_root);
		GetParentNode(tree_root);
		int text_node = GetTextNode(tree_root);
		//combine_tree_candidate_cut_text.push_back(tree_root);
        if(text_num_needed > 0)
        {
            DfsText(0, text_node - text_num_needed);
        }
        else
        {
            for (int text_num = 1; text_num <= text_node; ++text_num)
            {
                DfsText(0, text_node - text_num);
            }
        }

		for (int i = 0; i < text_node_delete_list.size(); i++)//第i棵候选子树
		{
			CCombineTreeNode* root_candi = tree_root->DeepCopy();
			combine_tree_candidate_cut_text.push_back(root_candi);
			std::queue<CCombineTreeNode*> temp_queue;
			temp_queue.push(root_candi);
			while (!temp_queue.empty())
			{
				CCombineTreeNode* node = temp_queue.front();
				temp_queue.pop();
				if (text_node_delete_list[i].count(node->index) == 1)
				{
					CutTreeNode(node);
				}
				for (auto iter = node->children.begin(); iter != node->children.end(); iter++)
					temp_queue.push(*iter);
			}
		}
		//std::cout << "原树的文本节点数量：" << text_node << std::endl;
		//for (auto tree : relations_candidates) OutputStructure(tree);
		//std::vector<CCombineTreeNode*> combine_tree_candidate_cut_text;
	}
	for (auto candidate : combine_tree_candidate_cut_text)
	{
        MergeTextNode(candidate);
        MergePaddingNode(candidate);
        MergeExtraParent(candidate);
	}
	CutExtraTree(combine_tree_candidate_cut_text);
	std::cout << "删去文本后的结构数量：" << combine_tree_candidate_cut_text.size() << std::endl;
	std::vector<CCombineTreeNode*> combine_tree_candidate_cut_padding;

	for (auto tree_root : combine_tree_candidate_cut_text)
	{
		//int text_num;
		padding_node_list.clear();
		padding_node_delete_list.clear();
		padding_node_delete_temp.clear();
		parent_list.clear();
		
		//MergeTextNode(tree_root);
		GetParentNode(tree_root);
		int padding_node = GetPaddingNode(tree_root);
		//combine_tree_candidate_cut_padding.push_back(tree_root);
		for (int padding_num = 0; padding_num <= padding_node; ++padding_num)
		{
			DfsPadding(0, padding_node - padding_num);
		}
		for (int i = 0; i < padding_node_delete_list.size(); i++)//第i棵候选子树
		{
			CCombineTreeNode* root_candi = tree_root->DeepCopy();
			combine_tree_candidate_cut_padding.push_back(root_candi);
			std::queue<CCombineTreeNode*> temp_queue;
			temp_queue.push(root_candi);
			while (!temp_queue.empty())
			{
				CCombineTreeNode* node = temp_queue.front();
				temp_queue.pop();
				if (padding_node_delete_list[i].count(node->index) == 1)
				{
					CutTreeNode(node);
				}
				for (auto iter = node->children.begin(); iter != node->children.end(); iter++)
					temp_queue.push(*iter);
			}
		}
		//std::cout << "原树的文本节点数量：" << text_node << std::endl;
		//for (auto tree : relations_candidates) OutputStructure(tree);
		//std::vector<CCombineTreeNode*> combine_tree_candidate_cut_text;
	}
	for (auto candidate : combine_tree_candidate_cut_padding)
	{
		MergeTextNode(candidate);
        MergePaddingNode(candidate);
		MergeExtraParent(candidate);
	}
	CutExtraTree(combine_tree_candidate_cut_padding);
	std::cout << "删去padding后的结构数量：" << combine_tree_candidate_cut_padding.size() << std::endl;

    std::vector<CCombineTreeNode*> combine_tree_candidate;

    if(combine_tree_candidate_cut_padding.size() > 500)
    {
        int interval = combine_tree_candidate_cut_padding.size() / 500;
        int index = 0;
        for(int i = 0; i < 500; i ++)
        {
            combine_tree_candidate.push_back(combine_tree_candidate_cut_padding[index]);
            index += interval;
        }
        int left = combine_tree_candidate_cut_padding.size() % 500;
        index = combine_tree_candidate_cut_padding.size() - left;
        if(left < 200) interval = 1;
        else interval = 2;
        for(;index < combine_tree_candidate_cut_padding.size(); index += interval)
        {
            combine_tree_candidate.push_back(combine_tree_candidate_cut_padding[index]);
        }
    }
    else
        combine_tree_candidate = combine_tree_candidate_cut_padding;
	return combine_tree_candidate;
}



std::vector<std::vector<CCombineTreeNode*>> CombineTreeHandler::GetCombineTreeCandidateMultiPage(CCombineTreeNode* combine_tree,
	int img_num)
{
	std::vector<CCombineTreeNode*> relations_candidates = GetTreeCandidateByRelations(combine_tree);

	DeterminOrder(relations_candidates);
	std::cout << "由关系决定的结构数量：" << relations_candidates.size() << std::endl;
	for (auto candidate : relations_candidates)
	{
		MergeExtraParent(candidate);
		MergeRelationNode(candidate);
	}

	int title_num = 1;
	std::vector<CCombineTreeNode*> combine_tree_candidate_cut_title;

	for (auto tree_root : relations_candidates)
	{
		title_node_list.clear();
		parent_list.clear();
		title_node_delete_list.clear();
		title_node_delete_temp.clear();
		GetParentNode(tree_root);
		int title_node = GetTitleNode(tree_root);
		if (title_node == title_num)//原树就是唯一的候选
		{
			combine_tree_candidate_cut_title.push_back(tree_root);
		}
		else if (title_node < title_num)
		{
			std::cout << "too much title in content." << std::endl;
		}
		else DfsTitle(0, title_node - title_num);
		for (int i = 0; i < title_node_delete_list.size(); i++)//第i棵候选子树
		{
			CCombineTreeNode* root_candi = tree_root->DeepCopy();
			combine_tree_candidate_cut_title.push_back(root_candi);
			std::queue<CCombineTreeNode*> temp_queue;
			temp_queue.push(root_candi);
			while (!temp_queue.empty())
			{
				CCombineTreeNode* node = temp_queue.front();
				temp_queue.pop();
				if (title_node_delete_list[i].count(node->index) == 1)
				{
					CutTreeNode(node);
				}
				for (auto iter = node->children.begin(); iter != node->children.end(); iter++)
					temp_queue.push(*iter);
			}
		}
	}

	for (auto candidate : combine_tree_candidate_cut_title)
	{
		MergeTextNode(candidate);
        MergePaddingNode(candidate);
		MergeExtraParent(candidate);
	}
	CutExtraTree(combine_tree_candidate_cut_title);
	std::cout << "删去标题后的结构数量：" << combine_tree_candidate_cut_title.size() << std::endl;


	std::vector<CCombineTreeNode*> combine_tree_candidate_cut_text;
	for (auto tree_root : combine_tree_candidate_cut_title)
	{
		//int text_num;
		text_node_list.clear();
		text_node_delete_list.clear();
		text_node_delete_temp.clear();
		parent_list.clear();
		//MergeTextNode(tree_root);
		GetParentNode(tree_root);
		int text_node = GetTextNodeWithVoid(tree_root);
		//combine_tree_candidate_cut_text.push_back(tree_root);
		for (int text_num = 0; text_num <= text_node; ++text_num)
		{
			DfsText(0, text_node - text_num);
		}
		for (int i = 0; i < text_node_delete_list.size(); i++)//第i棵候选子树
		{
			CCombineTreeNode* root_candi = tree_root->DeepCopy();
			combine_tree_candidate_cut_text.push_back(root_candi);
			std::queue<CCombineTreeNode*> temp_queue;
			temp_queue.push(root_candi);
			while (!temp_queue.empty())
			{
				CCombineTreeNode* node = temp_queue.front();
				temp_queue.pop();
				if (text_node_delete_list[i].count(node->index) == 1)
				{
					CutTreeNode(node);
				}
				for (auto iter = node->children.begin(); iter != node->children.end(); iter++)
					temp_queue.push(*iter);
			}
		}
	}
	for (auto candidate : combine_tree_candidate_cut_text)
	{
		MergeTextNode(candidate);
        MergePaddingNode(candidate);
		MergeExtraParent(candidate);
		
	}
	CutExtraTree(combine_tree_candidate_cut_text);
	std::cout << "删去文本后的结构数量：" << combine_tree_candidate_cut_text.size() << std::endl;

	/*for (auto candidate : combine_tree_candidate_cut_text)
	{
		combine_trees_groups.push_back({candidate});
	}*/

	//std::vector<CCombineTreeNode*> combine_tree_candidate_cut_pic;
	////统计原tree的picture node个数

	std::vector<std::vector<std::vector<CCombineTreeNode*>>> combine_trees_groups;
	for (auto tree_root : combine_tree_candidate_cut_text)
	{
		//int text_num;
		pic_node_list.clear();
		pic_node_delete_list.clear();
		pic_node_delete_temp.clear();
		parent_list.clear();
		//MergeTextNode(tree_root);
		GetParentNode(tree_root);
		int pic_node = GetPicNode(tree_root);
		int max_img_num = std::min(pic_node, img_num - 1);
		combine_trees_groups.push_back(std::vector<std::vector<CCombineTreeNode*>>(max_img_num + 1));
		//combine_tree_candidate_cut_text.push_back(tree_root);
		for (int pic_num = 1; pic_num <= max_img_num; ++pic_num)
		{
			//std::cout << max_img_num - pic_num << ": ";
			DfsPic(0, pic_node - pic_num);
			//std::cout << std::endl;
		}
		for (int i = 0; i < pic_node_delete_list.size(); i++)//第i棵候选子树
		{
			int img_num_idx = max_img_num - pic_node_delete_list[i].size();
			CCombineTreeNode* root_candi = tree_root->DeepCopy();
			combine_trees_groups.back()[img_num_idx].push_back(root_candi);
			//combine_tree_candidate_cut_.push_back(root_candi);
			std::queue<CCombineTreeNode*> temp_queue;
			temp_queue.push(root_candi);
			//int cut_num = 0;
			while (!temp_queue.empty())
			{
				CCombineTreeNode* node = temp_queue.front();
				temp_queue.pop();
				if (pic_node_delete_list[i].count(node->index) == 1)
				{
					CutTreeNode(node);
					//cut_num++;
				}
				for (auto iter = node->children.begin(); iter != node->children.end(); iter++)
					temp_queue.push(*iter);
			}


			/*int pic_num = 0;
			std::queue<CCombineTreeNode*> queue;
			queue.push(root_candi);
			while (!queue.empty())
			{
				auto node = queue.front();
				queue.pop();

				if (node->node_present_type == NODE_PRESENT_TYPE::PICTURE) pic_num++;

				for (auto child : node->children)
				{
					queue.push(child);
				}
			}

			std::cout << img_num_idx << "_" <<pic_num << std::endl;*/
		}
		for (std::vector<CCombineTreeNode*>& img_candidate_group : combine_trees_groups.back())
		{
			for(auto candidate:img_candidate_group)
			{
				MergeTextNode(candidate);
                MergePaddingNode(candidate);
				MergeExtraParent(candidate);
			}
			CutExtraTree(img_candidate_group);
		}
		
	}


	/*for (auto combine_tree_group : combine_trees_groups)
	{
		std::cout << "group: " << std::endl;
		for (int i = 0; i < combine_tree_group.size(); ++i)
		{
			std::cout << "img num " << i << ":" << combine_tree_group[i].size() << std::endl;
			for(auto candidate : combine_tree_group[i])
			{
				int pic_num = 0;
				std::queue<CCombineTreeNode*> queue;
				queue.push(candidate);
				while (!queue.empty())
				{
					auto node = queue.front();
					queue.pop();

					if (node->node_present_type == NODE_PRESENT_TYPE::PICTURE) pic_num++;

					for (auto child : node->children)
					{
						queue.push(child);
					}
				}
				std::cout << pic_num << " ";
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}*/


	std::vector<std::vector<CCombineTreeNode*>> combine_tree_candidate_group_list;
	for (auto combine_tree_group : combine_trees_groups)
	{
		int max_img_size = combine_tree_group.size() - 1;
		std::vector<int> limit(combine_tree_group.size());
		for(int i = 0; i < combine_tree_group.size();++i)
		{
			limit[i] = combine_tree_group[i].size();
		}
		std::vector<int> solution(combine_tree_group.size());
		std::vector<std::vector<int>> solution_list;
		MultiPageFindSolutions(limit, solution, solution_list, 1, img_num);
		for(int i = 0 ; i < solution_list.size(); ++i)
		{
			std::vector<CCombineTreeNode*> combine_tree_candidate_group;
			for(int current_img_num = 1;current_img_num <= max_img_size; ++current_img_num)
			{
				std::vector<int> idx_list(combine_tree_group[current_img_num].size());
				for (int j = 0; j < idx_list.size(); ++j) idx_list[j] = j;
                std::random_device rd;
                std::mt19937 g(rd());
                std::shuffle(idx_list.begin(), idx_list.end(),g);
				//std::random_shuffle(idx_list.begin(), idx_list.end());
				for(int j = 0; j <solution_list[i][current_img_num];j++)
				{
					combine_tree_candidate_group.push_back(combine_tree_group[current_img_num][idx_list[j]]->DeepCopy());
				}
			}
            std::random_device rd;
            std::mt19937 g(rd());
            std::shuffle(combine_tree_candidate_group.begin(),combine_tree_candidate_group.end(),g);
			//std::random_shuffle(combine_tree_candidate_group.begin(),combine_tree_candidate_group.end());
			
			for(int j=1;j<combine_tree_candidate_group.size();++j)
			{
				CutTitleFromATree(combine_tree_candidate_group[j]);
			}
			combine_tree_candidate_group_list.push_back(combine_tree_candidate_group);
		}
	}

	/*for (auto combine_tree_candidate_group : combine_tree_candidate_group_list)
	{
		int pic_num = 0;
		for(auto tree : combine_tree_candidate_group)
		{
			std::queue<CCombineTreeNode*> queue;
			queue.push(tree);
			while (!queue.empty())
			{
				auto node = queue.front();
				queue.pop();

				if (node->node_present_type == NODE_PRESENT_TYPE::PICTURE) pic_num++;

				for (auto child : node->children)
				{
					queue.push(child);
				}
			}
		}
		std::cout << pic_num << std::endl;
	}*/


	/*for(auto group:combine_tree_candidate_group_list)
	{
		for(auto tree:group)
		{
			OutputStructure(tree);
		}
		std::cout << std::endl;
	}*/

	// delete all the fucking unused structures
	for(auto i:combine_trees_groups)
	{
		for(auto j:i)
		{
			for (auto k : j) delete k;
		}
	}
	return combine_tree_candidate_group_list;
}

std::vector<std::vector<CCombineTreeNode*>> CombineTreeHandler::GetCombineTreeCandidateMultiPage2(
	CCombineTreeNode* combine_tree, int img_num, int target_page)
{
	std::vector<CCombineTreeNode*> relations_candidates = GetTreeCandidateByRelations(combine_tree);

	DeterminOrder(relations_candidates);
	std::cout << "由关系决定的结构数量：" << relations_candidates.size() << std::endl;
	for (auto candidate : relations_candidates)
	{
		MergeExtraParent(candidate);
		MergeRelationNode(candidate);
	}

	int title_num = 1;
	std::vector<CCombineTreeNode*> combine_tree_candidate_cut_title;

	for (auto tree_root : relations_candidates)
	{
		title_node_list.clear();
		parent_list.clear();
		title_node_delete_list.clear();
		title_node_delete_temp.clear();
		GetParentNode(tree_root);
		int title_node = GetTitleNode(tree_root);
		if (title_node == title_num)//原树就是唯一的候选
		{
			combine_tree_candidate_cut_title.push_back(tree_root->DeepCopy());
		}
		else if (title_node < title_num)
		{
			std::cout << "too much title in content." << std::endl;
		}
		else DfsTitle(0, title_node - title_num);
		for (int i = 0; i < title_node_delete_list.size(); i++)//第i棵候选子树
		{
			CCombineTreeNode* root_candi = tree_root->DeepCopy();
			combine_tree_candidate_cut_title.push_back(root_candi);
			std::queue<CCombineTreeNode*> temp_queue;
			temp_queue.push(root_candi);
			while (!temp_queue.empty())
			{
				CCombineTreeNode* node = temp_queue.front();
				temp_queue.pop();
				if (title_node_delete_list[i].count(node->index) == 1)
				{
					CutTreeNode(node);
				}
				for (auto iter = node->children.begin(); iter != node->children.end(); iter++)
					temp_queue.push(*iter);
			}
		}
	}

	for (auto candidate : combine_tree_candidate_cut_title)
	{
		MergeTextNode(candidate);
        MergePaddingNode(candidate);
		MergeExtraParent(candidate);
	}
	CutExtraTree(combine_tree_candidate_cut_title);
	std::cout << "删去标题后的结构数量：" << combine_tree_candidate_cut_title.size() << std::endl;

    std::vector<CCombineTreeNode*> combine_tree_candidate_cut_padding;
    for (auto tree_root : combine_tree_candidate_cut_title)
    {
        //int text_num;
        padding_node_list.clear();
        padding_node_delete_list.clear();
        padding_node_delete_temp.clear();
        parent_list.clear();

        //MergeTextNode(tree_root);
        GetParentNode(tree_root);
        int padding_node = GetPaddingNode(tree_root);
        //combine_tree_candidate_cut_padding.push_back(tree_root);
        for (int padding_num = 0; padding_num <= padding_node; ++padding_num)
        {
            DfsPadding(0, padding_node - padding_num);
        }
        for (int i = 0; i < padding_node_delete_list.size(); i++)//第i棵候选子树
        {
            CCombineTreeNode* root_candi = tree_root->DeepCopy();
            combine_tree_candidate_cut_padding.push_back(root_candi);
            std::queue<CCombineTreeNode*> temp_queue;
            temp_queue.push(root_candi);
            while (!temp_queue.empty())
            {
                CCombineTreeNode* node = temp_queue.front();
                temp_queue.pop();
                if (padding_node_delete_list[i].count(node->index) == 1)
                {
                    CutTreeNode(node);
                }
                for (auto iter = node->children.begin(); iter != node->children.end(); iter++)
                    temp_queue.push(*iter);
            }
        }
        //std::cout << "原树的文本节点数量：" << text_node << std::endl;
        //for (auto tree : relations_candidates) OutputStructure(tree);
        //std::vector<CCombineTreeNode*> combine_tree_candidate_cut_text;
    }
    for (auto candidate : combine_tree_candidate_cut_padding)
    {
        MergeTextNode(candidate);
        MergePaddingNode(candidate);
        MergeExtraParent(candidate);
    }
    CutExtraTree(combine_tree_candidate_cut_padding);
    std::cout << "删去padding后的结构数量：" << combine_tree_candidate_cut_padding.size() << std::endl;


	int tree_img_num = 0;
	std::queue<CCombineTreeNode*> queue;
	queue.push(combine_tree);
	while(!queue.empty())
	{
		auto node = queue.front();
		queue.pop();
		if (node->node_present_type == NODE_PRESENT_TYPE::PICTURE) tree_img_num++;
		for(int i = 0; i < node->children.size();++i)
		{
			queue.push(node->children[i]);
		}
	}
	int max_img_num = std::min(tree_img_num, img_num - 1);
	//std::vector<CCombineTreeNode*> combine_tree_candidate_cut_pic;
	std::vector<std::vector<CCombineTreeNode*>>  combine_tree_candidate_cut_pic_group;
	for(int i = 0; i <= max_img_num; ++i)
	{
		combine_tree_candidate_cut_pic_group.push_back(std::vector<CCombineTreeNode*>());
	}
	
	for (auto tree_root : combine_tree_candidate_cut_padding)
	{
		for(int current_img_num = 1; current_img_num <= max_img_num; ++current_img_num)
		{
			pic_node_list.clear();
			parent_list.clear();
			pic_node_delete_list.clear();
			pic_node_delete_temp.clear();
			GetParentNode(tree_root);
			int pic_node = GetPicNode(tree_root);
			DfsPic(0, pic_node - current_img_num);
			//std::cout << current_img_num << ":" << std::endl;
			for (int i = 0; i < pic_node_delete_list.size(); i++)//第i棵候选子树
			{
				//std::cout << pic_node_delete_list[i].size() << std::endl;
				CCombineTreeNode* root_candi = tree_root->DeepCopy();
				combine_tree_candidate_cut_pic_group[current_img_num].push_back(root_candi);
				std::queue<CCombineTreeNode*> temp_queue;
				temp_queue.push(root_candi);
				while (!temp_queue.empty())
				{
					CCombineTreeNode* node = temp_queue.front();
					temp_queue.pop();
					if (pic_node_delete_list[i].count(node->index) == 1)
					{
						CutTreeNode(node);
					}
					for (auto iter = node->children.begin(); iter != node->children.end(); iter++)
						temp_queue.push(*iter);
				}
			}
		}
		
	}

	for (std::vector<CCombineTreeNode*>& candidate_group : combine_tree_candidate_cut_pic_group)
	{
		for(auto candidate:candidate_group)
		{
			MergeTextNode(candidate);
            MergePaddingNode(candidate);
			MergeExtraParent(candidate);
		}
		CutExtraTree(candidate_group);
	}
	std::cout << "删去图片后的结构数量："  << std::endl;
	for(int current_img_num = 0; current_img_num <= tree_img_num; ++current_img_num)
	{
		std::cout << current_img_num << "img: " << combine_tree_candidate_cut_pic_group[current_img_num].size() << std::endl;
	}

	std::map<int, std::vector<std::vector<CCombineTreeNode*>>> combine_tree_candidate_cut_text_group; //col_num -> img_num

	for (int current_img_num = 1; current_img_num <= max_img_num; ++current_img_num)
	{
		for (auto tree_root : combine_tree_candidate_cut_pic_group[current_img_num])
		{
			//int text_num;
			text_node_list.clear();
			text_node_delete_list.clear();
			text_node_delete_temp.clear();
			parent_list.clear();
			//MergeTextNode(tree_root);
			GetParentNode(tree_root);
			int text_node = GetTextNode(tree_root);
			//combine_tree_candidate_cut_text.push_back(tree_root);
			for (int text_num = 1; text_num <= text_node; ++text_num)
			{
				DfsText(0, text_node - text_num);
			}
			for (int i = 0; i < text_node_delete_list.size(); i++)//第i棵候选子树
			{
				CCombineTreeNode* root_candi = tree_root->DeepCopy();

				//combine_tree_candidate_cut_text.push_back(root_candi);
				std::queue<CCombineTreeNode*> temp_queue;
				temp_queue.push(root_candi);
				while (!temp_queue.empty())
				{
					CCombineTreeNode* node = temp_queue.front();
					temp_queue.pop();
					if (text_node_delete_list[i].count(node->index) == 1)
					{
						CutTreeNode(node);
					}
					for (auto iter = node->children.begin(); iter != node->children.end(); iter++)
						temp_queue.push(*iter);
				}
				int col_num = TreeColNum(root_candi);
				if(combine_tree_candidate_cut_text_group.count(col_num) == 0)
				{
					combine_tree_candidate_cut_text_group[col_num] = std::vector<std::vector<CCombineTreeNode*>>();
					for(int j = 0; j <= tree_img_num; ++j)
					{
						combine_tree_candidate_cut_text_group[col_num].push_back(std::vector<CCombineTreeNode*>());
					}
				}
				combine_tree_candidate_cut_text_group[col_num][current_img_num].push_back(root_candi);

				//MergeRelationNode(root_candi);
				MergeTextNode(root_candi);
                MergePaddingNode(root_candi);
				MergeExtraParent(root_candi);
			}
		}
	}
	std::cout << "删去文字后的结构数量：" << std::endl;
	for(std::pair<const int, std::vector<std::vector<CCombineTreeNode*>>>& col_group : combine_tree_candidate_cut_text_group)
	{
		for (int current_img_num = 0; current_img_num <= tree_img_num; ++current_img_num)
		{
			std::vector<CCombineTreeNode*>& tree_list = col_group.second[current_img_num];
			CutExtraTree(tree_list);
			std::cout << col_group.first << " col， " << current_img_num << " img: " << tree_list.size()<<std::endl;
		}
	}

    //now we have col_img_group

	std::vector<std::vector<CCombineTreeNode*>> combine_tree_candidate_group_list;
	for (auto combine_tree_group : combine_tree_candidate_cut_text_group)
	{
		int max_img_size = combine_tree_group.second.size() - 1;
		std::vector<int> limit(combine_tree_group.second.size());
		for (int i = 0; i < combine_tree_group.second.size(); ++i)
		{
			limit[i] = combine_tree_group.second[i].size();
		}
		std::vector<int> solution(combine_tree_group.second.size());
		std::vector<std::vector<int>> solution_list;
		MultiPageFindSolutions(limit, solution, solution_list, 1, img_num);

		std::vector<std::vector<int>> new_solution_list;
		for(int i = 0; i < solution_list.size(); ++i)
		{
			int page_count = 0;
			for(int j = 0; j < solution_list[i].size(); j++)
			{
				page_count += solution_list[i][j];
			}
			if (page_count >= target_page - 2 && page_count <= target_page + 2)
				new_solution_list.push_back(solution_list[i]);
		}
		solution_list = new_solution_list;

		for (int i = 0; i < solution_list.size(); ++i)
		{
			std::vector<CCombineTreeNode*> combine_tree_candidate_group;
			for (int current_img_num = 1; current_img_num <= max_img_size; ++current_img_num)
			{
				std::vector<int> idx_list(combine_tree_group.second[current_img_num].size());
				for (int j = 0; j < idx_list.size(); ++j) idx_list[j] = j;
                std::random_device rd;
                std::mt19937 g(rd());
                std::shuffle(idx_list.begin(), idx_list.end(),g);
                //std::random_shuffle(idx_list.begin(), idx_list.end());
				for (int j = 0; j < solution_list[i][current_img_num]; j++)
				{
					combine_tree_candidate_group.push_back(combine_tree_group.second[current_img_num][idx_list[j]]->DeepCopy());
				}
			}
            std::random_device rd;
            std::mt19937 g(rd());
            std::shuffle(combine_tree_candidate_group.begin(), combine_tree_candidate_group.end(),g);
			//std::random_shuffle(combine_tree_candidate_group.begin(), combine_tree_candidate_group.end());

			for (int j = 1; j < combine_tree_candidate_group.size(); ++j)
			{
				CutTitleFromATree(combine_tree_candidate_group[j]);
			}
			combine_tree_candidate_group_list.push_back(combine_tree_candidate_group);
		}
	}

	/*for (auto combine_tree_candidate_group : combine_tree_candidate_group_list)
	{
		int pic_num = 0;
		for(auto tree : combine_tree_candidate_group)
		{
			std::queue<CCombineTreeNode*> queue;
			queue.push(tree);
			while (!queue.empty())
			{
				auto node = queue.front();
				queue.pop();

				if (node->node_present_type == NODE_PRESENT_TYPE::PICTURE) pic_num++;

				for (auto child : node->children)
				{
					queue.push(child);
				}
			}
		}
		std::cout << pic_num << std::endl;
	}*/


	/*for(auto group:combine_tree_candidate_group_list)
	{
		for(auto tree:group)
		{
			OutputStructure(tree);
		}
		std::cout << std::endl;
	}*/

	// delete all the fucking unused structures
	for (auto i : combine_tree_candidate_cut_text_group)
	{
		for (auto j : i.second)
		{
			for (auto k : j) delete k;
		}
	}

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(combine_tree_candidate_group_list.begin(), combine_tree_candidate_group_list.end(),g);
	//std::random_shuffle(combine_tree_candidate_group_list.begin(), combine_tree_candidate_group_list.end());
    std::vector<std::vector<CCombineTreeNode*>> combine_tree_candidate;

    if(combine_tree_candidate_group_list.size() > 500)
    {
        int interval = combine_tree_candidate_group_list.size() / 500;
        int index = 0;
        for(int i = 0; i < 500; i ++)
        {
            combine_tree_candidate.push_back(combine_tree_candidate_group_list[index]);
            index += interval;
        }
        int left = combine_tree_candidate_group_list.size() % 500;
        index = combine_tree_candidate_group_list.size() - left;
        if(left < 200) interval = 1;
        else interval = 2;
        for(;index < combine_tree_candidate_group_list.size(); index += interval)
        {
            combine_tree_candidate.push_back(combine_tree_candidate_group_list[index]);
        }
    }
    else
        combine_tree_candidate = combine_tree_candidate_group_list;
    return combine_tree_candidate;
}

std::vector<std::vector<CCombineTreeNode*>> CombineTreeHandler::GetCombineTreeCandidateMultiPage3(
        CCombineTreeNode* combine_tree, int img_num, int target_page, std::vector<std::pair<int, double>> ref_idx_ratio_pair_list)
{
    std::vector<CCombineTreeNode*> relations_candidates = GetTreeCandidateByRelations(combine_tree);

    DeterminOrder(relations_candidates);
    std::cout << "由关系决定的结构数量：" << relations_candidates.size() << std::endl;
    for (auto candidate : relations_candidates)
    {
        MergeExtraParent(candidate);
        MergeRelationNode(candidate);
    }

    int title_num = 1;
    std::vector<CCombineTreeNode*> combine_tree_candidate_cut_title;

    for (auto tree_root : relations_candidates)
    {
        title_node_list.clear();
        parent_list.clear();
        title_node_delete_list.clear();
        title_node_delete_temp.clear();
        GetParentNode(tree_root);
        int title_node = GetTitleNode(tree_root);
        if (title_node == title_num)//原树就是唯一的候选
        {
            combine_tree_candidate_cut_title.push_back(tree_root->DeepCopy());
        }
        else if (title_node < title_num)
        {
            std::cout << "too much title in content." << std::endl;
        }
        else DfsTitle(0, title_node - title_num);
        for (int i = 0; i < title_node_delete_list.size(); i++)//第i棵候选子树
        {
            CCombineTreeNode* root_candi = tree_root->DeepCopy();
            combine_tree_candidate_cut_title.push_back(root_candi);
            std::queue<CCombineTreeNode*> temp_queue;
            temp_queue.push(root_candi);
            while (!temp_queue.empty())
            {
                CCombineTreeNode* node = temp_queue.front();
                temp_queue.pop();
                if (title_node_delete_list[i].count(node->index) == 1)
                {
                    CutTreeNode(node);
                }
                for (auto iter = node->children.begin(); iter != node->children.end(); iter++)
                    temp_queue.push(*iter);
            }
        }
    }

    for (auto candidate : combine_tree_candidate_cut_title)
    {
        MergeTextNode(candidate);
        MergePaddingNode(candidate);
        MergeExtraParent(candidate);
    }
    CutExtraTree(combine_tree_candidate_cut_title);
    std::cout << "删去标题后的结构数量：" << combine_tree_candidate_cut_title.size() << std::endl;

    std::vector<CCombineTreeNode*> combine_tree_candidate_cut_padding;
    for (auto tree_root : combine_tree_candidate_cut_title)
    {
        //int text_num;
        padding_node_list.clear();
        padding_node_delete_list.clear();
        padding_node_delete_temp.clear();
        parent_list.clear();

        //MergeTextNode(tree_root);
        GetParentNode(tree_root);
        int padding_node = GetPaddingNode(tree_root);
        //combine_tree_candidate_cut_padding.push_back(tree_root);
        for (int padding_num = 0; padding_num <= padding_node; ++padding_num)
        {
            DfsPadding(0, padding_node - padding_num);
        }
        for (int i = 0; i < padding_node_delete_list.size(); i++)//第i棵候选子树
        {
            CCombineTreeNode* root_candi = tree_root->DeepCopy();
            combine_tree_candidate_cut_padding.push_back(root_candi);
            std::queue<CCombineTreeNode*> temp_queue;
            temp_queue.push(root_candi);
            while (!temp_queue.empty())
            {
                CCombineTreeNode* node = temp_queue.front();
                temp_queue.pop();
                if (padding_node_delete_list[i].count(node->index) == 1)
                {
                    CutTreeNode(node);
                }
                for (auto iter = node->children.begin(); iter != node->children.end(); iter++)
                    temp_queue.push(*iter);
            }
        }
        //std::cout << "原树的文本节点数量：" << text_node << std::endl;
        //for (auto tree : relations_candidates) OutputStructure(tree);
        //std::vector<CCombineTreeNode*> combine_tree_candidate_cut_text;
    }
    for (auto candidate : combine_tree_candidate_cut_padding)
    {
        MergeTextNode(candidate);
        MergePaddingNode(candidate);
        MergeExtraParent(candidate);
    }
    CutExtraTree(combine_tree_candidate_cut_padding);
    std::cout << "删去padding后的结构数量：" << combine_tree_candidate_cut_padding.size() << std::endl;


    int tree_img_num = 0;
    std::queue<CCombineTreeNode*> queue;
    queue.push(combine_tree);
    while(!queue.empty())
    {
        auto node = queue.front();
        queue.pop();
        if (node->node_present_type == NODE_PRESENT_TYPE::PICTURE) tree_img_num++;
        for(int i = 0; i < node->children.size();++i)
        {
            queue.push(node->children[i]);
        }
    }
    int max_img_num = std::min(tree_img_num, img_num - 1);
    //std::vector<CCombineTreeNode*> combine_tree_candidate_cut_pic;
    std::vector<std::vector<CCombineTreeNode*>>  combine_tree_candidate_cut_pic_group;
    for(int i = 0; i <= max_img_num; ++i)
    {
        combine_tree_candidate_cut_pic_group.push_back(std::vector<CCombineTreeNode*>());
    }

    for (auto tree_root : combine_tree_candidate_cut_padding)
    {
        for(int current_img_num = 1; current_img_num <= max_img_num; ++current_img_num)
        {
            pic_node_list.clear();
            parent_list.clear();
            pic_node_delete_list.clear();
            pic_node_delete_temp.clear();
            GetParentNode(tree_root);
            int pic_node = GetPicNode(tree_root);
            DfsPic(0, pic_node - current_img_num);
            //std::cout << current_img_num << ":" << std::endl;
            for (int i = 0; i < pic_node_delete_list.size(); i++)//第i棵候选子树
            {
                //std::cout << pic_node_delete_list[i].size() << std::endl;
                CCombineTreeNode* root_candi = tree_root->DeepCopy();
                combine_tree_candidate_cut_pic_group[current_img_num].push_back(root_candi);
                std::queue<CCombineTreeNode*> temp_queue;
                temp_queue.push(root_candi);
                while (!temp_queue.empty())
                {
                    CCombineTreeNode* node = temp_queue.front();
                    temp_queue.pop();
                    if (pic_node_delete_list[i].count(node->index) == 1)
                    {
                        CutTreeNode(node);
                    }
                    for (auto iter = node->children.begin(); iter != node->children.end(); iter++)
                        temp_queue.push(*iter);
                }
            }
        }

    }

    for (std::vector<CCombineTreeNode*>& candidate_group : combine_tree_candidate_cut_pic_group)
    {
        for(auto candidate:candidate_group)
        {
            MergeTextNode(candidate);
            MergePaddingNode(candidate);
            MergeExtraParent(candidate);
        }
        CutExtraTree(candidate_group);
    }
    std::cout << "删去图片后的结构数量："  << std::endl;
    for(int current_img_num = 0; current_img_num <= tree_img_num; ++current_img_num)
    {
        std::cout << current_img_num << "img: " << combine_tree_candidate_cut_pic_group[current_img_num].size() << std::endl;
    }

    std::map<int, std::vector<std::vector<CCombineTreeNode*>>> combine_tree_candidate_cut_text_group; //col_num -> img_num

    for (int current_img_num = 1; current_img_num <= max_img_num; ++current_img_num)
    {
        for (auto tree_root : combine_tree_candidate_cut_pic_group[current_img_num])
        {
            //int text_num;
            text_node_list.clear();
            text_node_delete_list.clear();
            text_node_delete_temp.clear();
            parent_list.clear();
            //MergeTextNode(tree_root);
            GetParentNode(tree_root);
            int text_node = GetTextNode(tree_root);
            //combine_tree_candidate_cut_text.push_back(tree_root);
            for (int text_num = 1; text_num <= text_node; ++text_num)
            {
                DfsText(0, text_node - text_num);
            }
            for (int i = 0; i < text_node_delete_list.size(); i++)//第i棵候选子树
            {
                CCombineTreeNode* root_candi = tree_root->DeepCopy();

                //combine_tree_candidate_cut_text.push_back(root_candi);
                std::queue<CCombineTreeNode*> temp_queue;
                temp_queue.push(root_candi);
                while (!temp_queue.empty())
                {
                    CCombineTreeNode* node = temp_queue.front();
                    temp_queue.pop();
                    if (text_node_delete_list[i].count(node->index) == 1)
                    {
                        CutTreeNode(node);
                    }
                    for (auto iter = node->children.begin(); iter != node->children.end(); iter++)
                        temp_queue.push(*iter);
                }
                int col_num = TreeColNum(root_candi);
                if(combine_tree_candidate_cut_text_group.count(col_num) == 0)
                {
                    combine_tree_candidate_cut_text_group[col_num] = std::vector<std::vector<CCombineTreeNode*>>();
                    for(int j = 0; j <= tree_img_num; ++j)
                    {
                        combine_tree_candidate_cut_text_group[col_num].push_back(std::vector<CCombineTreeNode*>());
                    }
                }
                combine_tree_candidate_cut_text_group[col_num][current_img_num].push_back(root_candi);

                //MergeRelationNode(root_candi);
                MergeTextNode(root_candi);
                MergePaddingNode(root_candi);
                MergeExtraParent(root_candi);
            }
        }
    }
    std::cout << "删去文字后的结构数量：" << std::endl;
    for(std::pair<const int, std::vector<std::vector<CCombineTreeNode*>>>& col_group : combine_tree_candidate_cut_text_group)
    {
        for (int current_img_num = 0; current_img_num <= tree_img_num; ++current_img_num)
        {
            std::vector<CCombineTreeNode*>& tree_list = col_group.second[current_img_num];
            CutExtraTree(tree_list);
            std::cout << col_group.first << " col， " << current_img_num << " img: " << tree_list.size()<<std::endl;
        }
    }

    //now we have col_img_group

    std::vector<std::vector<CCombineTreeNode*>> combine_tree_candidate_group_list;
    //loop in cols
    for (auto combine_tree_group : combine_tree_candidate_cut_text_group)
    {
        int max_img_size = combine_tree_group.second.size() - 1;   // max img size in single page
        std::vector<int> limit(combine_tree_group.second.size()); // i img page limit
        for (int i = 0; i < combine_tree_group.second.size(); ++i)
        {
            limit[i] = combine_tree_group.second[i].size();
        }
        std::vector<int> solution(combine_tree_group.second.size());    // i element means that i image page appare x times

        std::vector<std::vector<int>> solution_list; // solution list
        MultiPageFindSolutions(limit, solution, solution_list, 1, img_num); // find all the solution

        std::vector<std::vector<int>> new_solution_list; //
        for(int i = 0; i < solution_list.size(); ++i)
        {
            int page_count = 0;
            for(int j = 0; j < solution_list[i].size(); j++)
            {
                page_count += solution_list[i][j];
            }
            if (page_count >= target_page - 2 && page_count <= target_page + 2)
                new_solution_list.push_back(solution_list[i]);
        }
        solution_list = new_solution_list;

        for (int i = 0; i < solution_list.size(); ++i)
        {
            // reference
            int page = 0;
            for(auto img_per_page:solution_list[i])
            {
                page += img_per_page;
            }
            //std::vector<int> img_to_page_list(img_num, -1); // i-th image should be in x-th page

            std::vector<int> require_list(page, 0); // i-th page should contain x images
            for(auto pair:ref_idx_ratio_pair_list)
            {
                int target = std::min(page - 1, static_cast<int>(page * pair.second));
                require_list[target] ++;
            }

            std::vector<int> actual_page_img_sum_list(page, 0);
            auto solution_copy = solution_list[i];

            for(int j = 0; j < actual_page_img_sum_list.size(); ++j)
            {
                if(require_list[j] > 0)
                {
                    for(int k = 0; k < solution_copy.size(); ++k)
                    {
                        if(solution_copy[k] > 0 && k >= require_list[j])
                        {
                            solution_copy[k]--;
                            actual_page_img_sum_list[j] = k;
                            break;
                        }
                    }
                }
            }

            std::vector<int> other_page_img_sum_list;
            for(int j = 0; j < solution_copy.size(); ++j)
            {
                for(int k = 0; k < solution_copy[j]; ++k)
                {
                    other_page_img_sum_list.push_back(j);
                }
            }
            std::random_device rd;
            std::mt19937 g(rd());
            std::shuffle(other_page_img_sum_list.begin(), other_page_img_sum_list.end(), g);

            for(int j = 0; j < actual_page_img_sum_list.size(); ++j)
            {
                if(actual_page_img_sum_list[j] == 0)
                {
                    actual_page_img_sum_list[j] = other_page_img_sum_list.back();
                    other_page_img_sum_list.pop_back();
                }
            }
            //reference



            std::vector<std::vector<CCombineTreeNode*>> img_structure_list(solution_list[i].size()); // for the chosen i-images structures

            std::vector<CCombineTreeNode*> combine_tree_candidate_group;
            for (int current_img_num = 1; current_img_num <= max_img_size; ++current_img_num)
            {
                std::vector<int> idx_list(combine_tree_group.second[current_img_num].size());
                for (int j = 0; j < idx_list.size(); ++j) idx_list[j] = j;
                std::random_device rd;
                std::mt19937 g(rd());
                std::shuffle(idx_list.begin(), idx_list.end(),g);
                //std::random_shuffle(idx_list.begin(), idx_list.end());
                for (int j = 0; j < solution_list[i][current_img_num]; j++)
                {
                    img_structure_list[current_img_num].push_back(combine_tree_group.second[current_img_num][idx_list[j]]->DeepCopy());
                    //combine_tree_candidate_group.push_back(combine_tree_group.second[current_img_num][idx_list[j]]->DeepCopy());
                }
            }
            // put structures into the fucking candidate
            for(int j = 0; j < actual_page_img_sum_list.size(); ++j)
            {
                auto structure = img_structure_list[actual_page_img_sum_list[j]].back();
                img_structure_list[actual_page_img_sum_list[j]].pop_back();
                combine_tree_candidate_group.push_back(structure);
            }

            std::cout<<"require: ";
            for(auto item:require_list)
            {
                std::cout<<item<<" ";
            }
            std::cout<<std::endl;

            std::cout<<"actual: ";
            for(auto item:actual_page_img_sum_list)
            {
                std::cout<<item<<" ";
            }
            std::cout<<std::endl;
//            std::random_device rd;
//            std::mt19937 g(rd());
//            std::shuffle(combine_tree_candidate_group.begin(), combine_tree_candidate_group.end(),g);
            //std::random_shuffle(combine_tree_candidate_group.begin(), combine_tree_candidate_group.end());

            // don't shuffle, define an order that satisfies the ref



            for (int j = 1; j < combine_tree_candidate_group.size(); ++j)
            {
                CutTitleFromATree(combine_tree_candidate_group[j]);
            }
            combine_tree_candidate_group_list.push_back(combine_tree_candidate_group);



        }
    }

    /*for (auto combine_tree_candidate_group : combine_tree_candidate_group_list)
    {
        int pic_num = 0;
        for(auto tree : combine_tree_candidate_group)
        {
            std::queue<CCombineTreeNode*> queue;
            queue.push(tree);
            while (!queue.empty())
            {
                auto node = queue.front();
                queue.pop();

                if (node->node_present_type == NODE_PRESENT_TYPE::PICTURE) pic_num++;

                for (auto child : node->children)
                {
                    queue.push(child);
                }
            }
        }
        std::cout << pic_num << std::endl;
    }*/


    /*for(auto group:combine_tree_candidate_group_list)
    {
        for(auto tree:group)
        {
            OutputStructure(tree);
        }
        std::cout << std::endl;
    }*/

    // delete all the fucking unused structures
    for (auto i : combine_tree_candidate_cut_text_group)
    {
        for (auto j : i.second)
        {
            for (auto k : j) delete k;
        }
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(combine_tree_candidate_group_list.begin(), combine_tree_candidate_group_list.end(),g);
    //std::random_shuffle(combine_tree_candidate_group_list.begin(), combine_tree_candidate_group_list.end());
    std::vector<std::vector<CCombineTreeNode*>> combine_tree_candidate;

    if(combine_tree_candidate_group_list.size() > 500)
    {
        int interval = combine_tree_candidate_group_list.size() / 500;
        int index = 0;
        for(int i = 0; i < 500; i ++)
        {
            combine_tree_candidate.push_back(combine_tree_candidate_group_list[index]);
            index += interval;
        }
        int left = combine_tree_candidate_group_list.size() % 500;
        index = combine_tree_candidate_group_list.size() - left;
        if(left < 200) interval = 1;
        else interval = 2;
        for(;index < combine_tree_candidate_group_list.size(); index += interval)
        {
            combine_tree_candidate.push_back(combine_tree_candidate_group_list[index]);
        }
    }
    else
        combine_tree_candidate = combine_tree_candidate_group_list;
    return combine_tree_candidate;
}

int CombineTreeHandler::GetPicNode(CCombineTreeNode* combine_tree_root)
{
	int cnt_picture = 0;
	for(auto node:parent_list)
	{
		std::vector<int> node_list;
		for(auto child:node->children)
		{
			if (child->node_present_type == NODE_PRESENT_TYPE::PICTURE)
			{
				node_list.push_back(child->index);
				cnt_picture++;
			}
		}
		pic_node_list[node->index] = node_list;
	}
	return cnt_picture;
}



void CombineTreeHandler::DfsPic(int parent_list_idx, int delete_num)
{
	if (delete_num == 0)
	{
		//if(pic_node_delete_temp.size() > 0)
			pic_node_delete_list.push_back(pic_node_delete_temp);
		return;
	}
	if (parent_list_idx >= parent_list.size()) return;
	int parent_idx = parent_list[parent_list_idx]->index;
	int parent_pic_num = pic_node_list[parent_idx].size();
	int del_num_bound = std::min(parent_pic_num, delete_num);
	for (int del_num = 0; del_num <= del_num_bound; del_num++)
	{
		//pic_node_delete_temp.insert(pic_node_list[parent_idx][i]);
		for(int i = 0;i < del_num;++i)
		{
			pic_node_delete_temp.insert(pic_node_list[parent_idx][i]);
		}
		DfsPic(parent_list_idx + 1, delete_num - del_num);
		for(int i = 0;i < del_num;++i)
		{
			pic_node_delete_temp.erase(pic_node_list[parent_idx][i]);
		}
	}
}


int CombineTreeHandler::GetTitleNode(CCombineTreeNode* combine_tree_root)
{
	int cnt_title = 0;
	for (auto node : parent_list)
	{
		std::vector<int> node_list;
		for (auto child : node->children)
		{
			if (child->node_present_type == NODE_PRESENT_TYPE::TITLE)
			{
				node_list.push_back(child->index);
				cnt_title ++;
			}
		}
		title_node_list[node->index] = node_list;
	}
	return cnt_title;
}



void CombineTreeHandler::DfsTitle(int parent_list_idx, int delete_num)
{
	if (delete_num == 0)
	{
		//if (title_node_delete_temp.size() > 0)
			title_node_delete_list.push_back(title_node_delete_temp);
		return;
	}
	if (parent_list_idx >= parent_list.size()) return;
	int parent_idx = parent_list[parent_list_idx]->index;
	int parent_title_num = title_node_list[parent_idx].size();
	int del_num_bound = std::min(parent_title_num, delete_num);
	for (int del_num = 0; del_num <= del_num_bound; del_num++)
	{
		//pic_node_delete_temp.insert(pic_node_list[parent_idx][i]);
		for (int i = 0; i < del_num; ++i)
		{
			title_node_delete_temp.insert(title_node_list[parent_idx][i]);
		}
		DfsTitle(parent_list_idx + 1, delete_num - del_num);
		for (int i = 0; i < del_num; ++i)
		{
			title_node_delete_temp.erase(title_node_list[parent_idx][i]);
		}
	}
}

void CombineTreeHandler::DfsText(int parent_list_idx, int delete_num)
{
	if (delete_num == 0)
	{
		//if (text_node_delete_temp.size() > 0)
			text_node_delete_list.push_back(text_node_delete_temp);
		return;
	}
	if (parent_list_idx >= parent_list.size()) return;
	int parent_idx = parent_list[parent_list_idx]->index;
	int parent_text_num = text_node_list[parent_idx].size();
	int del_num_bound = std::min(parent_text_num, delete_num);
	for (int del_num = 0; del_num <= del_num_bound; del_num++)
	{
		//pic_node_delete_temp.insert(pic_node_list[parent_idx][i]);
		for (int i = 0; i < del_num; ++i)
		{
			text_node_delete_temp.insert(text_node_list[parent_idx][i]);
		}
		DfsText(parent_list_idx + 1, delete_num - del_num);
		for (int i = 0; i < del_num; ++i)
		{
			text_node_delete_temp.erase(text_node_list[parent_idx][i]);
		}
	}	
}


void CombineTreeHandler::DfsPadding(int parent_list_idx, int delete_num)
{
	if (delete_num == 0)
	{
		//if (padding_node_delete_temp.size() > 0)
			padding_node_delete_list.push_back(padding_node_delete_temp);
		return;
	}
	if (parent_list_idx >= parent_list.size()) return;
	int parent_idx = parent_list[parent_list_idx]->index;
	int parent_padding_num = padding_node_list[parent_idx].size();
	int del_num_bound = std::min(parent_padding_num, delete_num);
	for (int del_num = 0; del_num <= del_num_bound; del_num++)
	{
		//pic_node_delete_temp.insert(pic_node_list[parent_idx][i]);
		for (int i = 0; i < del_num; ++i)
		{
			padding_node_delete_temp.insert(padding_node_list[parent_idx][i]);
		}
		DfsPadding(parent_list_idx + 1, delete_num - del_num);
		for (int i = 0; i < del_num; ++i)
		{
			padding_node_delete_temp.erase(padding_node_list[parent_idx][i]);
		}
	}
}

int CombineTreeHandler::GetTextNode(CCombineTreeNode* combine_tree_root)
{
	int cnt_text = 0;
	for (auto node : parent_list)
	{
		std::vector<int> node_list;
		for (auto child : node->children)
		{
			if (child->node_present_type == NODE_PRESENT_TYPE::TEXT)
			{
				node_list.push_back(child->index);
				cnt_text++;
			}
		}
		text_node_list[node->index] = node_list;
	}	
	return cnt_text;
}

int CombineTreeHandler::GetTextNodeWithVoid(CCombineTreeNode* combine_tree_root)
{
	int cnt_text = 0;
	for (auto node : parent_list)
	{
		std::vector<int> node_list;
		for (auto child : node->children)
		{
			if (child->node_present_type == NODE_PRESENT_TYPE::TEXT && child->node_corres_type != NODE_CORRES_TYPE::T_ABC)
			{
				node_list.push_back(child->index);
				cnt_text++;
			}
		}
		text_node_list[node->index] = node_list;
	}
	return cnt_text;
}

int CombineTreeHandler::GetPaddingNode(CCombineTreeNode* combine_tree_root)
{
	int cnt_padding = 0;
	for (auto node : parent_list)
	{
		std::vector<int> node_list;
		for (auto child : node->children)
		{
			if (child->node_present_type == NODE_PRESENT_TYPE::PADDING)
			{
				node_list.push_back(child->index);
				cnt_padding++;
			}
		}
		padding_node_list[node->index] = node_list;
	}
	return cnt_padding;
}


void CombineTreeHandler::OutputStructure(CCombineTreeNode* combine_tree)
{
	std::queue<CCombineTreeNode*> temp_queue;
	temp_queue.push(combine_tree);
	int this_layer = 1;
	int next_layer = 0;

	//std::cout << "combine tree structure:" << std::endl;
	while (!temp_queue.empty())
	{
		CCombineTreeNode* node = temp_queue.front();
		temp_queue.pop();

		std::cout << node->index << " " << node->node_present_type
			<< " " << (node->parent == nullptr ? (-node->index) : node->parent->index)
			<< "(" << node->tree0_node_id << ","
			<< node->tree1_node_id << ","
			<< node->tree2_node_id << ")"
			<< "  ";
		if (node->node_relation == NODE_RELATION::HORIZONTAL)
		{
			std::cout << "h ";
		}
		if (node->node_relation == NODE_RELATION::VERTICAL)
		{
			std::cout << "v ";
		}
		for (auto iter = node->children.begin(); iter != node->children.end(); iter++)
		{
			temp_queue.push(*iter);
			next_layer++;
		}
		this_layer--;
		if (this_layer == 0)
		{
			this_layer = next_layer;
			next_layer = 0;
			std::cout << std::endl;
		}
	}
	std::cout << std::endl;
}

void CombineTreeHandler::CutTreeNode(CCombineTreeNode* tree_node)
{
	CCombineTreeNode* p_parent = tree_node->parent;
	CCombineTreeNode* p_child = tree_node;
	while (p_child != nullptr && p_parent != nullptr)
	{
		//p_parent = p_child->parent;
		//if(p_parent == nullptr) p_child = 
		for (int k = 0; k < p_parent->children.size(); k++)
		{
			if (p_parent->children[k] == p_child)
			{
				p_parent->children.erase(p_parent->children.begin() + k);
				break;
			}
		}
		if (p_parent->children.size() == 0)
		{
			p_child = p_parent;
			p_parent = p_parent->parent;
		}
		else
		{
			//p_parent = nullptr;
			p_child = nullptr;
		}
	}
	//if(p_parent != nullptr)
	//{
	//	if(p_parent->children.size() == 1)
	//	{
	//		CCombineTreeNode* new_child = p_parent->children[0];
	//		CCombineTreeNode* new_parent = p_parent->parent;
	//		if (new_parent != nullptr)
	//		{
	//			for (int i = 0; i < new_parent->children.size(); ++i)
	//			{
	//				if (new_parent->children[i] == p_parent)
	//				{
	//					new_parent->children[i] = new_child;
	//					new_child->parent = new_parent;
	//					//new_parent->children.erase(new_parent->children.begin() + i);
	//					//new_parent->children.insert(new_parent->children.begin() + i, new_child);
	//					break;
	//				}
	//			}
	//		}
	//	}
	//}
}

//cut the parents which have only one child
void CombineTreeHandler::MergeExtraParent(CCombineTreeNode* tree)
{
	
	while(tree->children.size() == 1 && tree->children[0]->children.size() > 0)
	{
		auto temp = tree->children[0];
		tree->node_relation =temp->node_relation;
		tree->children = temp->children;
		for(int i=0;i<tree->children.size();++i)
		{
			tree->children[i]->parent = tree;
		}
		temp->children.clear();
		temp->parent = nullptr;
		CCombineTreeNode::DeepDestroy(temp);
	}


	std::queue<CCombineTreeNode*> temp_queue;
	temp_queue.push(tree);
	while (!temp_queue.empty())
	{
		auto node = temp_queue.front();
		temp_queue.pop();
		bool cut_child_exist = true;
		while (cut_child_exist)
		{
			int cut_child_index = -1;
			for (int i = 0; i < node->children.size(); ++i)
			{
				if (node->children[i]->children.size() == 1)
				{
					cut_child_exist = true;
					cut_child_index = i;
					break;
				}
			}
			if (cut_child_index == -1)
			{
				cut_child_exist = false;
			}
			else
			{
				int insert_index = cut_child_index;
				CCombineTreeNode* cut_child = node->children[cut_child_index];
				node->children.erase(node->children.begin() + cut_child_index);
				auto new_child = cut_child->children[0];
				node->children.emplace(node->children.begin() + insert_index, new_child);
				new_child->parent = node;
			}
		}
		for (auto child : node->children)
		{
			if (child->children.size() > 0) temp_queue.push(child);
		}
	}
}

void CombineTreeHandler::CutExtraTree(std::vector<CCombineTreeNode*>& tree_list)
{
	std::vector<CCombineTreeNode*> copy_list;
	std::vector<int> delete_id_list = {};
	for (int i = 0; i < tree_list.size(); ++i)
	{
		copy_list.push_back(tree_list[i]->DeepCopy());
		SortTreeForDuplicate(copy_list.back());
	}
	for(int i = 0 ; i < copy_list.size(); ++i)
	{
		for(int j = i + 1; j < copy_list.size(); ++j)
		{
			if(CombineNodeCompareDuplicateInt(copy_list[i],copy_list[j]) == 0)
			{
				delete_id_list.push_back(i);
				break;
			}
		}
	}
	for(int i= delete_id_list.size() - 1;i >= 0; --i)
	{
		int idx = delete_id_list[i];
		auto tree = tree_list[idx];
		CCombineTreeNode::DeepDestroy(tree);
		tree_list.erase(tree_list.begin() + idx);
	}
	for(int i=0;i<copy_list.size();++i)
	{
		CCombineTreeNode::DeepDestroy(copy_list[i]);
	}
}

void CombineTreeHandler::SortTreeForDuplicate(CCombineTreeNode* tree)
{
	for(int i=0;i<tree->children.size();++i)
	{
		if(tree->children[i]->node_present_type == NODE_PRESENT_TYPE::NONLABEL)
		{
			SortTreeForDuplicate(tree->children[i]);
		}
	}
	std::sort(tree->children.begin(), tree->children.end(), CombineNodeCompareDuplicateBool);
}

int DistanceToClosestSameParent(CCombineTreeNode *a, CCombineTreeNode *b)
{
    std::vector<CCombineTreeNode*> a_parent_list, b_parent_list;
    CCombineTreeNode *parent;
    parent = a->parent;
    while(parent != nullptr)
    {
        a_parent_list.push_back(parent);
        parent = parent->parent;
    }
    parent = b->parent;
    while(parent != nullptr)
    {
        b_parent_list.push_back(parent);
        parent = parent->parent;
    }
    for(int i=0;i < a_parent_list.size(); ++i)
    {
        for(int j=0; j < b_parent_list.size(); ++j)
        {
            if(a_parent_list[i] == b_parent_list[j])
                return i + j + 2;
        }
    }
    return 0;
}

bool CombineTreeHandler::DivideLayoutTreeInGroups(CCombineTreeNode *tree, std::vector<std::pair<int, int>> group_need)
{
    std::vector<CCombineTreeNode*> left_node_list;
    std::unordered_map<CCombineTreeNode*, int> node_to_index;
    std::stack<CCombineTreeNode*> stack;
    stack.push(tree);
    while(!stack.empty())
    {
        auto node = stack.top();
        stack.pop();
        if(node->children.size() == 0)
        {
            node_to_index[node] = left_node_list.size();
            left_node_list.push_back(node);
        }
        for (int i = node->children.size() - 1; i >= 0; i--)
        {
            stack.push(node->children[i]);
        }
    }
    std::vector<std::pair<int, int>> group_bound_list;
    //no consideration on complicated cases

    // find bounds for images
    int left_bound = -1, right_bound = -1;
    int pre_idx = 0;
    int group_num = group_need.size();
    int img_num = group_need[0].second;
    for(int i = 0; i < left_node_list.size() && pre_idx < group_num; ++i)
    {
        if(img_num == 0)
        {
            if(left_bound >= 0 && right_bound >= left_bound)
            {
                group_bound_list.push_back({left_bound, right_bound});
            }
            else
            {
                group_bound_list.push_back({-1, -1});
            }
            left_bound = -1;
            right_bound = -1;
            pre_idx++;
            img_num = group_need[pre_idx].second;
        }
        if(left_node_list[i]->node_present_type == NODE_PRESENT_TYPE::PICTURE)
        {
            img_num--;
            if(left_bound < 0) left_bound = i;
            right_bound = i;
        }
    }
    // greedy grow
    int pre_right_bound, next_left_bound;
    group_bound_list[0].first = 0;
    group_bound_list.back().second = left_node_list.size() - 1;
    for(pre_idx = 0; pre_idx < group_num - 1; ++pre_idx)
    {
        int next_idx = pre_idx + 1;
        pre_right_bound = group_bound_list[pre_idx].second;
        next_left_bound = group_bound_list[next_idx].first;
        while(pre_right_bound < next_left_bound - 1)
        {
            int i = pre_right_bound + 1;
            int j = next_left_bound - 1;

            int i_to_pre = DistanceToClosestSameParent(left_node_list[pre_idx], left_node_list[i]);
            int i_to_next = DistanceToClosestSameParent(left_node_list[i], left_node_list[next_idx]);

            if(i_to_pre <= i_to_next)
            {
                group_bound_list[pre_idx].second = i;
                pre_right_bound = i;
            }
            else
            {
                group_bound_list[next_idx].first = i;
                next_left_bound = i;
            }

            if(pre_right_bound >= next_left_bound - 1) break;

            int j_to_pre = DistanceToClosestSameParent(left_node_list[pre_idx], left_node_list[j]);
            int j_to_next = DistanceToClosestSameParent(left_node_list[j], left_node_list[next_idx]);

            if(j_to_next <= j_to_pre)
            {
                group_bound_list[next_idx].first = j;
                next_left_bound = j;
            }
            else
            {
                group_bound_list[pre_idx].second = j;
                pre_right_bound = j;
            }
        }

//        int need_text = group_need[pre_idx].first;
//        if(need_text > 0)
//        {
//            if(group_bound_list[pre_idx].first == -1 && group_bound_list[pre_idx].second == -1)
//            {
////                int leftmost = pre_idx == 0 ? 0:group_bound_list[pre_idx - 1].second + 1;
////                int rightmost = pre_idx == group_num - 1 ? left_node_list.size() - 1 : group_bound_list[pre_idx + 1].first;
//                int leftmost, rightmost;
//                if(pre_idx == 0)
//                {
//                    leftmost = 0;
//                }
//                else if(group_bound_list[pre_idx - 1].second == -1)
//                {
//                    leftmost =
//                }
//                for(int i = leftmost; i < rightmost; ++i)
//                {
//                    if(left_node_list[i]->node_present_type == NODE_PRESENT_TYPE::TEXT)
//                    {
//
//                    }
//                }
//
//            }
//        }
    }

    return true;
}

