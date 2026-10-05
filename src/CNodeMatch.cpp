#include "CNodeMatch.h"


CNodeMatch::CNodeMatch()
{
	first_tree_root = nullptr;
	second_tree_root = nullptr;
	third_tree_root = nullptr;
	group_tree_root = nullptr;
	combine_tree_root = nullptr;

}

CNodeMatch::~CNodeMatch()
{
}

int CNodeMatch::HungaryCost(std::vector<CNode*>& nodeSet1, std::vector<CNode*>& nodeSet2, std::set<std::pair<int, int>>& crtCsp)
{
	std::vector<std::vector<int>> costMat;
	std::vector<std::vector<std::set<std::pair<int, int>>>> cspMat;
	std::vector<int> rowVec;
	std::vector<std::set<std::pair<int, int>>> rowCspVec;
	rowVec.resize(nodeSet1.size() + nodeSet2.size(), 0);
	rowCspVec.resize(nodeSet1.size() + nodeSet2.size(), std::set<std::pair<int, int>>());

	costMat.resize(nodeSet1.size() + nodeSet2.size(), rowVec);
	cspMat.resize(nodeSet1.size() + nodeSet2.size(), rowCspVec);

	for (auto& valueVec : costMat)
	{
		for (auto& thisValue : valueVec)
		{
			thisValue = 0;
		}
	}

	for (int i = 0; i < nodeSet1.size(); ++i)
	{
		CNode* ni = nodeSet1[i];
		for (int j = 0; j < nodeSet2.size(); ++j)
		{
			CNode* nj = nodeSet2[j];
			costMat[i][j] = ComputeCost(ni, nj);

			cspMat[i][j] = recordCorrespondence[std::pair<int, int>(ni->index, nj->index)];
		}
	}
	for (int i = nodeSet1.size(); i < nodeSet1.size() + nodeSet2.size(); ++i)
	{
		for (int j = 0; j < nodeSet2.size(); ++j)
		{
			CNode* nj = nodeSet2[j];
			costMat[i][j] = ComputeCost(NULL, nj);
			cspMat[i][j] = recordCorrespondence[std::pair<int, int>(-nj->index, nj->index)];
		}
	}
	for (int i = 0; i < nodeSet1.size(); ++i)
	{
		CNode* ni = nodeSet1[i];
		for (int j = nodeSet2.size(); j < nodeSet1.size() + nodeSet2.size(); ++j)
		{
			costMat[i][j] = ComputeCost(ni, NULL);
			cspMat[i][j] = recordCorrespondence[std::pair<int, int>(ni->index, -ni->index)];
		}
	}

	CHungarianAlgorithm hung_algo;
	std::vector<int> HungaryCsp;
	int thisCost = hung_algo.Solve(costMat, HungaryCsp);
	for (int i = 0; i < HungaryCsp.size(); ++i)
	{
		crtCsp.insert(cspMat[i][HungaryCsp[i]].begin(), cspMat[i][HungaryCsp[i]].end());// 把这个entry中的对应全部加到当前对应中
	}
	return thisCost;
}

int CNodeMatch::HungaryGrpThdCost(std::vector<CGroupTreeNode*>& nodeSet1, std::vector<CNode*>& nodeSet2, std::set<std::pair<int, int>>& crtCsp)
{
	std::vector<std::vector<int>> costMat;
	std::vector<std::vector<std::set<std::pair<int, int>>>> cspMat;
	std::vector<int> rowVec;
	std::vector<std::set<std::pair<int, int>>> rowCspVec;

	rowVec.resize(nodeSet1.size() + nodeSet2.size(), 0);
	rowCspVec.resize(nodeSet1.size() + nodeSet2.size(), std::set<std::pair<int, int>>());

	costMat.resize(nodeSet1.size() + nodeSet2.size(), rowVec);
	cspMat.resize(nodeSet1.size() + nodeSet2.size(), rowCspVec);

	for (auto& valueVec : costMat)
	{
		for (auto& thisValue : valueVec)
		{
			thisValue = 0;
		}
	}

	for (int i = 0; i < nodeSet1.size(); i++)
	{
		CGroupTreeNode* ni = nodeSet1[i];
		for (int j = 0; j < nodeSet2.size(); j++)
		{
			CNode* nj = nodeSet2[j];
			costMat[i][j] = CalGrpThdCost(ni, nj);
			cspMat[i][j] = recordGrpThdCrp[std::pair<int, int>(ni->index, nj->index)];
		}
	}

	for (int i = nodeSet1.size(); i < nodeSet1.size() + nodeSet2.size(); i++)
	{
		for (int j = 0; j < nodeSet2.size(); j++)
		{
			CNode* nj = nodeSet2[j];
			costMat[i][j] = CalGrpThdCost(nullptr, nj);
			cspMat[i][j] = recordGrpThdCrp[std::pair<int, int>(-nj->index, nj->index)];
		}
	}

	for (int i = 0; i < nodeSet1.size(); i++)
	{
		CGroupTreeNode* ni = nodeSet1[i];
		for (int j = nodeSet2.size(); j < nodeSet1.size() + nodeSet2.size(); j++)
		{
			costMat[i][j] = CalGrpThdCost(ni, nullptr);
			cspMat[i][j] = recordGrpThdCrp[std::pair<int, int>(ni->index, -ni->index)];
		}
	}

	CHungarianAlgorithm hung_algo;
	std::vector<int> HungaryCsp;
	int thisCost = hung_algo.Solve(costMat, HungaryCsp);
	for (int i = 0; i < HungaryCsp.size(); i++)
	{
		crtCsp.insert(cspMat[i][HungaryCsp[i]].begin(), cspMat[i][HungaryCsp[i]].end());
	}
	return thisCost;
}

int CNodeMatch::ComputeCost(CNode* n1, CNode* n2)
{
	if (n1 == nullptr)
	{
		if (recordCost.find(std::pair<int, int>(-n2->index, n2->index)) != recordCost.end())
		{
			return recordCost[std::pair<int, int>(-n2->index, n2->index)];
		}

		recordCorrespondence[std::pair<int, int>(-n2->index, n2->index)].insert(std::pair<int, int>(-n2->index, n2->index));
		if (n2->node_type == NODE_TYPE::LEAF)
		{
			int thisCost = LeafToNULLCost(n2);
			recordCost[std::pair<int, int>(-n2->index, n2->index)] = thisCost;
			return thisCost;
		}

		int totalCost = 0;
		for (auto ptr : n2->children)
		{
			totalCost += ComputeCost(NULL, ptr);
			recordCorrespondence[std::pair<int, int>(-n2->index, n2->index)].insert(
				recordCorrespondence[std::pair<int, int>(-ptr->index, ptr->index)].begin(),
				recordCorrespondence[std::pair<int, int>(-ptr->index, ptr->index)].end());
		}
		recordCost[std::pair<int, int>(-n2->index, n2->index)] = totalCost;
		return totalCost;
	}
	else if (n2 == nullptr)
	{
		if (recordCost.find(std::pair<int, int>(n1->index, -n1->index)) != recordCost.end())
		{
			return recordCost[std::pair<int, int>(n1->index, -n1->index)];
		}

		recordCorrespondence[std::pair<int, int>(n1->index, -n1->index)].insert(std::pair<int, int>(n1->index, -n1->index));
		if (n1->node_type == NODE_TYPE::LEAF)
		{
			int thisCost = LeafToNULLCost(n1);
			recordCost[std::pair<int, int>(n1->index, -n1->index)] = thisCost;
			return thisCost;
		}

		int totalCost = 0;
		for (auto ptr : n1->children)
		{
			totalCost += ComputeCost(ptr, NULL);
			recordCorrespondence[std::pair<int, int>(n1->index, -n1->index)].insert(
				recordCorrespondence[std::pair<int, int>(ptr->index, -ptr->index)].begin(),
				recordCorrespondence[std::pair<int, int>(ptr->index, -ptr->index)].end());
		}
		recordCost[std::pair<int, int>(n1->index, -n1->index)] = totalCost;
		return totalCost;
	}

	if (recordCost.find(std::pair<int, int>(n1->index, n2->index)) != recordCost.end())
	{
		return recordCost[std::pair<int, int>(n1->index, n2->index)];
	}

	recordCorrespondence[std::pair<int, int>(n1->index, n2->index)].insert(std::pair<int, int>(n1->index, n2->index));

	if (n1->node_type == NODE_TYPE::LEAF && n2->node_type == NODE_TYPE::LEAF)
	{
		int thisCost = LeafToLeafCost(n1, n2);
		recordCost[std::pair<int, int>(n1->index, n2->index)] = thisCost;
		return thisCost;
	}
	else if (n1->node_type == NODE_TYPE::NONLEAF && n2->node_type == NODE_TYPE::LEAF)
	{
		std::vector<CNode*> nodeSet1, nodeSet2;
		nodeSet1 = n1->children;
		nodeSet2 = { n2 };
		std::set<std::pair<int, int>> newCsp;

		int thisCost = HungaryCost(nodeSet1, nodeSet2, newCsp);

		bool without_corres = true;
		for (auto each_pair : newCsp)
		{
			if (each_pair.first >= 0 && each_pair.second >= 0)
			{
				without_corres = false;
				break;
			}
		}
		if (without_corres == true)
		{
			thisCost = 100000;
		}

		recordCorrespondence[std::pair<int, int>(n1->index, n2->index)].insert(newCsp.begin(), newCsp.end());
		recordCost[std::pair<int, int>(n1->index, n2->index)] = thisCost;

		return thisCost;
	}
	else if (n1->node_type == NODE_TYPE::LEAF && n2->node_type == NODE_TYPE::NONLEAF)
	{
		std::vector<CNode*> nodeSet1, nodeSet2;

		nodeSet1 = { n1 };
		nodeSet2 = n2->children;
		std::set<std::pair<int, int>> newCsp;

		int thisCost = HungaryCost(nodeSet1, nodeSet2, newCsp);

		bool without_corres = true;
		for (auto each_pair : newCsp)
		{
			if (each_pair.first >= 0 && each_pair.second >= 0)
			{
				without_corres = false;
				break;
			}
		}
		if (without_corres == true)
		{
			thisCost = 100000;
		}

		recordCorrespondence[std::pair<int, int>(n1->index, n2->index)].insert(newCsp.begin(), newCsp.end());
		recordCost[std::pair<int, int>(n1->index, n2->index)] = thisCost;
		return thisCost;
	}
	else if (n1->node_type == NODE_TYPE::NONLEAF && n2->node_type == NODE_TYPE::NONLEAF)
	{
		std::vector<CNode*> nodeSet1, nodeSet2;

		// case 1: children to children correspondence
		nodeSet1 = n1->children;
		nodeSet2 = n2->children;
		std::set<std::pair<int, int>> newCspCase1;

		int case1Cost = HungaryCost(nodeSet1, nodeSet2, newCspCase1);

		// case 2: n1 to n2's children

		nodeSet1 = { n1 };
		nodeSet2 = n2->children;
		std::set<std::pair<int, int>> newCspCase2;


		int case2Cost = HungaryCost(nodeSet1, nodeSet2, newCspCase2);

		// case 3: n1's children to n2
		nodeSet1 = n1->children;
		nodeSet2 = { n2 };
		std::set<std::pair<int, int>> newCspCase3;


		int case3Cost = HungaryCost(nodeSet1, nodeSet2, newCspCase3);


		int minimalCost = std::min(case1Cost, std::min(case2Cost, case3Cost));

		if (case1Cost == minimalCost)
		{
			bool without_corres = true;
			for (auto each_pair : newCspCase1)
			{
				if (each_pair.first >= 0 && each_pair.second >= 0)
				{
					without_corres = false;
					break;
				}
			}
			if (without_corres == true)
			{
				minimalCost = 100000;
			}
			recordCorrespondence[std::pair<int, int>(n1->index, n2->index)].insert(newCspCase1.begin(), newCspCase1.end());
			//minimalCost = case1Cost;
		}
		else if (case2Cost == minimalCost)
		{
			recordCorrespondence[std::pair<int, int>(n1->index, n2->index)].insert(newCspCase2.begin(), newCspCase2.end());
			//minimalCost = case2Cost;
		}
		else
		{
			recordCorrespondence[std::pair<int, int>(n1->index, n2->index)].insert(newCspCase3.begin(), newCspCase3.end());
			//minimalCost = case3Cost;
		}

		recordCost[std::pair<int, int>(n1->index, n2->index)] = minimalCost;

		return minimalCost;
	}
	return 0;
}

void CNodeMatch::ComputeCsp(CNode* root1, CNode* root2)
{
	first_tree_root = root1;
	second_tree_root = root2;
	two_tree_cost = ComputeCost(root1, root2);
	std::cout << "前两个tree的对应cost = " << two_tree_cost << std::endl;
	return;
}

void CNodeMatch::CreateGroupTree1()
{
	std::queue<std::pair<int, int>> pairQueue;
	pairQueue.push(std::make_pair(first_tree_root->index, second_tree_root->index));

	group_tree_root = new CGroupTreeNode();
	group_tree_root->tree0_node = first_tree_root;
	group_tree_root->tree1_node = second_tree_root;
	group_tree_root->tree0_node_id = first_tree_root->index;
	group_tree_root->tree1_node_id = second_tree_root->index;
	group_tree_root->parent = nullptr;

	CGroupTreeNode* thisNode = group_tree_root;
	std::unordered_map<std::pair<int, int>, CGroupTreeNode*, pairhash> pairToNode;
	pairToNode[std::pair<int, int>(group_tree_root->tree0_node_id, group_tree_root->tree1_node_id)] = group_tree_root;

	std::set<std::pair<int, int>> validCsp = recordCorrespondence[std::pair<int, int>(group_tree_root->tree0_node_id, group_tree_root->tree1_node_id)];
	std::set<int>dangling_nodes_0;
	std::set<int>dangling_nodes_1;
	for (auto i : validCsp)
	{
		if (i.first < 0)
		{
			dangling_nodes_1.insert(i.second);
		}
		if (i.second < 0)
		{
			dangling_nodes_0.insert(i.first);
		}
	}

	while (!pairQueue.empty())
	{
		auto thisPair = pairQueue.front();
		pairQueue.pop();
		CGroupTreeNode* thisNode = pairToNode[thisPair];

		if (thisNode->tree0_node == nullptr)
		{
			std::vector<CNode*> nodeSet2 = thisNode->tree1_node->children;
			for (int i = 0; i < nodeSet2.size(); ++i)
			{
				CNode* n2 = nodeSet2[i];
				CGroupTreeNode* childNode = new CGroupTreeNode();
				childNode->parent = thisNode;
				thisNode->node_type = NODE_TYPE::NONLEAF;
				childNode->tree0_node_id = -n2->index;
				childNode->tree1_node_id = n2->index;
				childNode->tree0_node = NULL;
				childNode->tree1_node = n2;

				thisNode->children.push_back(childNode);

				pairQueue.push(std::pair<int, int>(-n2->index, n2->index));
				pairToNode[std::pair<int, int>(-n2->index, n2->index)] = childNode;
			}
		}
		else if (thisNode->tree1_node == nullptr)
		{
			std::vector<CNode*> nodeSet1 = thisNode->tree0_node->children;
			for (int i = 0; i < nodeSet1.size(); ++i)
			{
				CNode* n1 = nodeSet1[i];
				CGroupTreeNode* childNode = new CGroupTreeNode();
				childNode->parent = thisNode;
				thisNode->node_type = NODE_TYPE::NONLEAF;
				childNode->tree0_node_id = n1->index;
				childNode->tree1_node_id = -n1->index;
				childNode->tree0_node = n1;
				childNode->tree1_node = NULL;

				thisNode->children.push_back(childNode);

				pairQueue.push(std::pair<int, int>(n1->index, -n1->index));
				pairToNode[std::pair<int, int>(n1->index, -n1->index)] = childNode;
			}
		}
		else
		{
			std::vector<CNode*> nodeSet1, nodeSet2;
			nodeSet1 = thisNode->tree0_node->children;
			nodeSet1.push_back(thisNode->tree0_node);	//push thisNode->tree0_node就是为了下沉做准备

			nodeSet2 = thisNode->tree1_node->children;
			nodeSet2.push_back(thisNode->tree1_node);	//push thisNode->tree1_node就是为了下沉做准备

			std::vector<bool> nodeFlag1(nodeSet1.size(), false), nodeFlag2(nodeSet2.size(), false);

			for (int i = 0; i < nodeSet1.size(); ++i)
			{
				CNode* n1 = nodeSet1[i];
				for (int j = 0; j < nodeSet2.size(); ++j)
				{
					CNode* n2 = nodeSet2[j];
					std::pair<int, int> possiblePair(n1->index, n2->index);
					if (n1->index == thisNode->tree0_node_id && n2->index == thisNode->tree1_node_id)
					{
						continue;
					}

					if (validCsp.find(possiblePair) != validCsp.end())
					{
						CGroupTreeNode* childNode = new CGroupTreeNode();
						childNode->parent = thisNode;
						thisNode->node_type = NODE_TYPE::NONLEAF;
						childNode->tree0_node = n1;
						childNode->tree1_node = n2;
						childNode->tree0_node_id = n1->index;
						childNode->tree1_node_id = n2->index;

						thisNode->children.push_back(childNode);

						pairQueue.push(possiblePair);
						pairToNode[possiblePair] = childNode;
						nodeFlag1[i] = true;
						nodeFlag2[j] = true;
					}

				}
			}
			std::cout << "n1:flag" << std::endl;
			for (auto i : nodeFlag1)
			{
				std::cout << i << std::endl;
			}

			for (int i = 0; i < nodeSet1.size() - 1; ++i)
			{
				if (!nodeFlag1[i])
				{
					if (dangling_nodes_0.find(nodeSet1[i]->index) == dangling_nodes_0.end())
					{
						continue;
					}
					CNode* n1 = nodeSet1[i];
					CGroupTreeNode* childNode = new CGroupTreeNode();
					childNode->parent = thisNode;
					thisNode->node_type = NODE_TYPE::NONLEAF;
					childNode->tree0_node_id = n1->index;
					childNode->tree1_node_id = -n1->index;
					childNode->tree0_node = n1;
					childNode->tree1_node = NULL;

					thisNode->children.push_back(childNode);

					pairQueue.push(std::pair<int, int>(n1->index, -n1->index));
					pairToNode[std::pair<int, int>(n1->index, -n1->index)] = childNode;
				}
			}
			for (int i = 0; i < nodeSet2.size() - 1; ++i)
			{
				if (!nodeFlag2[i])
				{
					if (dangling_nodes_1.find(nodeSet2[i]->index) == dangling_nodes_1.end())
					{
						continue;
					}

					CNode* n2 = nodeSet2[i];
					CGroupTreeNode* childNode = new CGroupTreeNode();
					childNode->parent = thisNode;
					thisNode->node_type = NODE_TYPE::NONLEAF;
					childNode->tree0_node_id = -n2->index;
					childNode->tree1_node_id = n2->index;
					childNode->tree0_node = NULL;
					childNode->tree1_node = n2;

					thisNode->children.push_back(childNode);

					pairQueue.push(std::pair<int, int>(-n2->index, n2->index));
					pairToNode[std::pair<int, int>(-n2->index, n2->index)] = childNode;
				}
			}
		}
	}

	//YZJ:hang it only when it's done
	std::cout << "final matching" << std::endl;
	for (auto key : pairToNode)
	{
		//std::cout << key.first.first << " " << key.first.second << std::endl;
		std::cout << key.second->tree0_node_id << " " << key.second->tree1_node_id << std::endl;
	}

	return;
}

void CNodeMatch::CreateGroupTree()
{
	std::unordered_map<int, CNode*> first_idx_ptr_map;
	std::unordered_map<int, CNode*> second_idx_ptr_map;

	if (first_tree_root)
	{
		std::queue<CNode*> temp_queue;
		temp_queue.push(first_tree_root);
		while (!temp_queue.empty())
		{
			CNode* front_node = temp_queue.front();
			temp_queue.pop();

			first_idx_ptr_map[front_node->index] = front_node;

			for (int i = 0; i < front_node->children.size(); i++)
			{
				temp_queue.push(front_node->children[i]);
			}
		}
	}

	if (second_tree_root)
	{
		std::queue<CNode*> temp_queue;
		temp_queue.push(second_tree_root);
		while (!temp_queue.empty())
		{
			CNode* front_node = temp_queue.front();
			temp_queue.pop();
			second_idx_ptr_map[front_node->index] = front_node;
			for (int i = 0; i < front_node->children.size(); i++)
			{
				temp_queue.push(front_node->children[i]);
			}
		}
	}

	group_tree_root = new CGroupTreeNode();
	group_tree_root->tree0_node_id = first_tree_root->index;
	group_tree_root->tree1_node_id = second_tree_root->index;
	group_tree_root->tree0_node = first_tree_root;
	group_tree_root->tree1_node = second_tree_root;
	group_tree_root->parent = nullptr;

	std::set<std::pair<int, int>> validCsp = recordCorrespondence[std::pair<int, int>(group_tree_root->tree0_node_id, group_tree_root->tree1_node_id)];

	std::cout << "查看validCsp**************" << std::endl;
	for (auto i : validCsp)
	{
		std::cout << i.first << "    " << i.second << std::endl;
	}
	std::cout << "查看validCsp**************" << std::endl;

	std::queue<CNode*> temp_queue;
	temp_queue.push(first_tree_root);
	while (!temp_queue.empty())
	{
		CNode* front_node = temp_queue.front();
		temp_queue.pop();

		std::set<std::pair<int, int>> pair_first_with_front_node_index;
		pair_first_with_front_node_index.clear();
		for (auto i : validCsp)
		{
			if (i.first == front_node->index)
			{
				pair_first_with_front_node_index.insert(i);
			}
		}

		std::set<std::pair<int, int>> pair_first_with_front_node_index1 = pair_first_with_front_node_index;
		while (pair_first_with_front_node_index1.size() != 0)
		{
			pair_first_with_front_node_index = pair_first_with_front_node_index1;
			//pair_first_with_front_node_index里有两种情况, node->node 或者 node->null
			for (auto j : pair_first_with_front_node_index)
			{
				if (j.first > 0 && j.second > 0)
				{
					//node->node

					if (PairExistedInGroupTree(j, group_tree_root) == true)
					{
						//这个pair已经存在于CombineTree中
						pair_first_with_front_node_index1.erase(pair_first_with_front_node_index1.find(j));
						continue;
					}

					if (PairDoNotExistedInGroupTree(j, group_tree_root))
					{
						//这个pair不存在于CombineTree中,且pair first和pair second均不存在于CombineTree中
						//serach parent node
						int first_node_parent_index = first_idx_ptr_map[j.first]->parent->index;
						int second_node_parent_index = second_idx_ptr_map[j.second]->parent->index;
						CGroupTreeNode* temp_parent_node = ReturnParentGroupNode(first_node_parent_index, second_node_parent_index);
						if (temp_parent_node != nullptr)
						{
							CGroupTreeNode* new_node = new CGroupTreeNode();
							new_node->tree0_node_id = j.first;
							new_node->tree1_node_id = j.second;
							new_node->tree0_node = first_idx_ptr_map[new_node->tree0_node_id];
							new_node->tree1_node = second_idx_ptr_map[new_node->tree1_node_id];
							new_node->parent = temp_parent_node;
							temp_parent_node->node_type = NODE_TYPE::NONLEAF;
							temp_parent_node->children.push_back(new_node);
							pair_first_with_front_node_index1.erase(pair_first_with_front_node_index1.find(j));
						}
					}
					else if (OnlyPairFirstExistedInGroupTree(j, group_tree_root))
					{
						//这个pair不存在于CombineTree中,pair first存在于CombineTree中
						//左下沉,那么父节点是pair second的父节点所在的节点
						int second_node_parent_index = second_idx_ptr_map[j.second]->parent->index;
						CGroupTreeNode* temp_parent_node = ReturnRightParentGroupNode(second_node_parent_index);
						if (temp_parent_node != nullptr)
						{
							CGroupTreeNode* new_node = new CGroupTreeNode();
							new_node->tree0_node_id = j.first;
							new_node->tree1_node_id = j.second;
							new_node->tree0_node = first_idx_ptr_map[new_node->tree0_node_id];
							new_node->tree1_node = second_idx_ptr_map[new_node->tree1_node_id];
							new_node->parent = temp_parent_node;
							temp_parent_node->node_type = NODE_TYPE::NONLEAF;
							temp_parent_node->children.push_back(new_node);
							pair_first_with_front_node_index1.erase(pair_first_with_front_node_index1.find(j));
						}
					}
					else if (OnlyPairSecondExistedInGroupTree(j, group_tree_root))
					{
						//这个pair不存在于CombineTree中,pair second存在于CombineTree中
						//右下沉,那么父节点是pair first父节点
						int first_node_parent_index = first_idx_ptr_map[j.first]->parent->index;
						CGroupTreeNode* temp_parent_node = ReturnLeftParentGroupNode(first_node_parent_index);
						if (temp_parent_node != nullptr)
						{
							CGroupTreeNode* new_node = new CGroupTreeNode();
							new_node->tree0_node_id = j.first;
							new_node->tree1_node_id = j.second;
							new_node->tree0_node = first_idx_ptr_map[new_node->tree0_node_id];
							new_node->tree1_node = second_idx_ptr_map[new_node->tree1_node_id];
							new_node->parent = temp_parent_node;
							temp_parent_node->node_type = NODE_TYPE::NONLEAF;
							temp_parent_node->children.push_back(new_node);
							pair_first_with_front_node_index1.erase(pair_first_with_front_node_index1.find(j));
						}
					}
				}
				else
				{
					//node->null,只出现在group tree中的节点
					if (PairExistedInGroupTree(j, group_tree_root) == true)
					{
						//这个pair已经存在于CombineTree中
						pair_first_with_front_node_index1.erase(pair_first_with_front_node_index1.find(j));
						continue;
					}
					int first_node_parent_index = first_idx_ptr_map[j.first]->parent->index;
					CGroupTreeNode* temp_parent_node = ReturnLeftParentGroupNode(first_node_parent_index);
					if (temp_parent_node != nullptr)
					{
						CGroupTreeNode* new_node = new CGroupTreeNode();
						new_node->tree0_node_id = j.first;
						new_node->tree1_node_id = -new_node->tree0_node_id;

						new_node->tree0_node = first_idx_ptr_map[new_node->tree0_node_id];
						new_node->tree1_node = nullptr;

						new_node->parent = temp_parent_node;
						temp_parent_node->node_type = NODE_TYPE::NONLEAF;
						temp_parent_node->children.push_back(new_node);
						pair_first_with_front_node_index1.erase(pair_first_with_front_node_index1.find(j));
					}
				}
			}

		}

		for (int i = 0; i < front_node->children.size(); i++)
		{
			temp_queue.push(front_node->children[i]);
		}
	}

	//至此，已处理完group tree中的节点,剩下处理third tree对应为空的情况
	std::set<std::pair<int, int>> remaining_pair;
	remaining_pair.clear();
	for (auto each_pair : validCsp)
	{
		if (each_pair.first < 0 && each_pair.second > 0)
		{
			remaining_pair.insert(each_pair);
		}
	}

	std::set<std::pair<int, int>> remaining_pair_ = remaining_pair;
	while (remaining_pair_.size() != 0)
	{
		remaining_pair = remaining_pair_;
		for (auto each_pair : remaining_pair)
		{
			if (PairExistedInGroupTree(each_pair, group_tree_root) == true)
			{
				//这个pair已经存在于CombineTree中
				remaining_pair_.erase(remaining_pair_.find(each_pair));
				continue;
			}
			int second_node_parent_index = second_idx_ptr_map[each_pair.second]->parent->index;
			CGroupTreeNode* temp_parent_node = ReturnRightParentGroupNode(second_node_parent_index);

			if (temp_parent_node != nullptr)
			{
				CGroupTreeNode* new_node = new CGroupTreeNode();
				new_node->tree0_node_id = -each_pair.second;
				new_node->tree1_node_id = each_pair.second;

				new_node->tree0_node = nullptr;
				new_node->tree1_node = second_idx_ptr_map[new_node->tree1_node_id];

				new_node->parent = temp_parent_node;
				temp_parent_node->node_type = NODE_TYPE::NONLEAF;
				temp_parent_node->children.push_back(new_node);
				//std::cout << "call here here" << std::endl;
				remaining_pair_.erase(remaining_pair_.find(each_pair));
				//std::cout << "call here here here" << std::endl;
			}
		}
	}
	return;
}

void CNodeMatch::CreateCombineTree()
{
	std::unordered_map<int, CGroupTreeNode*> group_idx_ptr_map;
	std::unordered_map<int, CNode*> third_idx_ptr_map;

	if (group_tree_root)
	{
		std::queue<CGroupTreeNode*> temp_queue;
		temp_queue.push(group_tree_root);
		while (!temp_queue.empty())
		{
			CGroupTreeNode* front_node = temp_queue.front();
			temp_queue.pop();

			group_idx_ptr_map[front_node->index] = front_node;

			for (int i = 0; i < front_node->children.size(); i++)
			{
				temp_queue.push(front_node->children[i]);
			}
		}
	}

	if (third_tree_root)
	{
		std::queue<CNode*> temp_queue;
		temp_queue.push(third_tree_root);
		while (!temp_queue.empty())
		{
			CNode* front_node = temp_queue.front();
			temp_queue.pop();
			third_idx_ptr_map[front_node->index] = front_node;
			for (int i = 0; i < front_node->children.size(); i++)
			{
				temp_queue.push(front_node->children[i]);
			}
		}
	}

	//设置根节点
	combine_tree_root = new CCombineTreeNode();
	combine_tree_root->group_tree_node = group_tree_root;
	combine_tree_root->tree2_node = third_tree_root;
	combine_tree_root->group_tree_node_id = group_tree_root->index;
	combine_tree_root->tree2_node_id = third_tree_root->index;
	combine_tree_root->parent = nullptr;

	std::set<std::pair<int, int>> validCsp = recordGrpThdCrp[std::pair<int, int>(combine_tree_root->group_tree_node_id, combine_tree_root->tree2_node_id)];

	std::cout << "查看validCsp**************" << std::endl;
	for (auto i : validCsp)
	{
		std::cout << i.first << "    " << i.second << std::endl;
	}
	std::cout << "查看validCsp**************" << std::endl;

	std::queue<CGroupTreeNode*> temp_queue;
	temp_queue.push(group_tree_root);
	while (!temp_queue.empty())
	{
		CGroupTreeNode* front_node = temp_queue.front();
		temp_queue.pop();

		std::set<std::pair<int, int>> pair_first_with_front_node_index;
		pair_first_with_front_node_index.clear();
		for (auto i : validCsp)
		{
			if (i.first == front_node->index)
			{
				pair_first_with_front_node_index.insert(i);
			}
		}

		std::set<std::pair<int, int>> pair_first_with_front_node_index1 = pair_first_with_front_node_index;
		while (pair_first_with_front_node_index1.size() != 0)
		{
			pair_first_with_front_node_index = pair_first_with_front_node_index1;
			//pair_first_with_front_node_index里有两种情况, node->node 或者 node->null
			for (auto j : pair_first_with_front_node_index)
			{
				if (j.first > 0 && j.second > 0)
				{
					//node->node

					if (PairExistedInCombineTree(j, combine_tree_root) == true)
					{
						//这个pair已经存在于CombineTree中
						pair_first_with_front_node_index1.erase(pair_first_with_front_node_index1.find(j));
						continue;
					}

					if (PairDoNotExistedInCombineTree(j, combine_tree_root))
					{
						//这个pair不存在于CombineTree中,且pair first和pair second均不存在于CombineTree中
						//serach parent node
						int group_node_parent_index = group_idx_ptr_map[j.first]->parent->index;
						int third_node_parent_index = third_idx_ptr_map[j.second]->parent->index;
						CCombineTreeNode* temp_parent_node = ReturnParentNode(group_node_parent_index, third_node_parent_index);
						if (temp_parent_node != nullptr)
						{
							CCombineTreeNode* new_node = new CCombineTreeNode();
							new_node->group_tree_node_id = j.first;
							new_node->tree2_node_id = j.second;
							new_node->group_tree_node = group_idx_ptr_map[new_node->group_tree_node_id];
							new_node->tree2_node = third_idx_ptr_map[new_node->tree2_node_id];
							new_node->parent = temp_parent_node;
							temp_parent_node->children.push_back(new_node);
							temp_parent_node->node_type = NODE_TYPE::NONLEAF;
							pair_first_with_front_node_index1.erase(pair_first_with_front_node_index1.find(j));
						}
					}
					else if (OnlyPairFirstExistedInCombineTree(j, combine_tree_root))
					{
						//这个pair不存在于CombineTree中,pair first存在于CombineTree中
						//左下沉,那么父节点是pair second的父节点所在的节点
						int third_node_parent_index = third_idx_ptr_map[j.second]->parent->index;
						CCombineTreeNode* temp_parent_node = ReturnRightParentNode(third_node_parent_index);
						if (temp_parent_node != nullptr)
						{
							CCombineTreeNode* new_node = new CCombineTreeNode();
							new_node->group_tree_node_id = j.first;
							new_node->tree2_node_id = j.second;
							new_node->group_tree_node = group_idx_ptr_map[new_node->group_tree_node_id];
							new_node->tree2_node = third_idx_ptr_map[new_node->tree2_node_id];
							new_node->parent = temp_parent_node;
							temp_parent_node->children.push_back(new_node);
							temp_parent_node->node_type = NODE_TYPE::NONLEAF;
							pair_first_with_front_node_index1.erase(pair_first_with_front_node_index1.find(j));
						}
					}
					else if (OnlyPairSecondExistedInCombineTree(j, combine_tree_root))
					{
						//这个pair不存在于CombineTree中,pair second存在于CombineTree中
						//右下沉,那么父节点是pair first父节点
						int group_node_parent_index = group_idx_ptr_map[j.first]->parent->index;
						CCombineTreeNode* temp_parent_node = ReturnLeftParentNode(group_node_parent_index);
						if (temp_parent_node != nullptr)
						{
							CCombineTreeNode* new_node = new CCombineTreeNode();
							new_node->group_tree_node_id = j.first;
							new_node->tree2_node_id = j.second;
							new_node->group_tree_node = group_idx_ptr_map[new_node->group_tree_node_id];
							new_node->tree2_node = third_idx_ptr_map[new_node->tree2_node_id];
							new_node->parent = temp_parent_node;
							temp_parent_node->children.push_back(new_node);
							temp_parent_node->node_type = NODE_TYPE::NONLEAF;
							pair_first_with_front_node_index1.erase(pair_first_with_front_node_index1.find(j));
						}
					}
				}
				else
				{
					//node->null,只出现在group tree中的节点
					if (PairExistedInCombineTree(j, combine_tree_root) == true)
					{
						//这个pair已经存在于CombineTree中
						pair_first_with_front_node_index1.erase(pair_first_with_front_node_index1.find(j));
						continue;
					}
					int group_node_parent_index = group_idx_ptr_map[j.first]->parent->index;
					CCombineTreeNode* temp_parent_node = ReturnLeftParentNode(group_node_parent_index);
					if (temp_parent_node != nullptr)
					{
						CCombineTreeNode* new_node = new CCombineTreeNode();
						new_node->group_tree_node_id = j.first;
						new_node->tree2_node_id = -new_node->group_tree_node_id;

						new_node->group_tree_node = group_idx_ptr_map[new_node->group_tree_node_id];
						new_node->tree2_node = nullptr;

						new_node->parent = temp_parent_node;
						temp_parent_node->children.push_back(new_node);
						temp_parent_node->node_type = NODE_TYPE::NONLEAF;
						pair_first_with_front_node_index1.erase(pair_first_with_front_node_index1.find(j));
					}
				}
			}

		}

		for (int i = 0; i < front_node->children.size(); i++)
		{
			temp_queue.push(front_node->children[i]);
		}
	}

	//至此，已处理完group tree中的节点,剩下处理third tree对应为空的情况
	std::set<std::pair<int, int>> remaining_pair;
	remaining_pair.clear();
	for (auto each_pair : validCsp)
	{
		if (each_pair.first < 0 && each_pair.second > 0)
		{
			remaining_pair.insert(each_pair);
		}
	}

	std::set<std::pair<int, int>> remaining_pair_ = remaining_pair;
	while (remaining_pair_.size() != 0)
	{
		remaining_pair = remaining_pair_;
		for (auto each_pair : remaining_pair)
		{
			if (PairExistedInCombineTree(each_pair, combine_tree_root) == true)
			{
				//这个pair已经存在于CombineTree中
				remaining_pair_.erase(remaining_pair_.find(each_pair));
				continue;
			}
			int third_node_parent_index = third_idx_ptr_map[each_pair.second]->parent->index;
			CCombineTreeNode* temp_parent_node = ReturnRightParentNode(third_node_parent_index);
			if (temp_parent_node != nullptr)
			{
				CCombineTreeNode* new_node = new CCombineTreeNode();
				new_node->group_tree_node_id = -each_pair.second;
				new_node->tree2_node_id = each_pair.second;

				new_node->group_tree_node = nullptr;
				new_node->tree2_node = third_idx_ptr_map[new_node->tree2_node_id];

				new_node->parent = temp_parent_node;
				temp_parent_node->children.push_back(new_node);
				temp_parent_node->node_type = NODE_TYPE::NONLEAF;
				//std::cout << "call here here" << std::endl;
				remaining_pair_.erase(remaining_pair_.find(each_pair));
				//std::cout << "call here here here" << std::endl;
			}
		}
	}
	return;
}

void CNodeMatch::CreateCombineTree1()
{
	std::queue<std::pair<int, int>> pairQueue;
	pairQueue.push(std::make_pair(group_tree_root->index, third_tree_root->index));

	//设置根节点
	combine_tree_root = new CCombineTreeNode();
	combine_tree_root->group_tree_node = group_tree_root;
	combine_tree_root->tree2_node = third_tree_root;
	combine_tree_root->group_tree_node_id = group_tree_root->index;
	combine_tree_root->tree2_node_id = third_tree_root->index;
	combine_tree_root->parent = nullptr;

	//CCombineTreeNode* thisNode = combine_tree_root;
	std::unordered_map<std::pair<int, int>, CCombineTreeNode*, pairhash> pairToNode;
	pairToNode[std::pair<int, int>(combine_tree_root->group_tree_node_id, combine_tree_root->tree2_node_id)] = combine_tree_root;

	std::set<std::pair<int, int>> validCsp = recordGrpThdCrp[std::pair<int, int>(combine_tree_root->group_tree_node_id, combine_tree_root->tree2_node_id)];
	std::set<int> dangling_nodes_0;
	std::set<int> dangling_nodes_1;

	std::cout << "查看validCsp**************" << std::endl;
	for (auto i : validCsp)
	{
		std::cout << i.first << "    " << i.second << std::endl;
	}
	std::cout << "查看validCsp**************" << std::endl;

	for (auto i : validCsp)
	{
		if (i.first < 0)
		{
			dangling_nodes_1.insert(i.second);
		}
		if (i.second < 0)
		{
			dangling_nodes_0.insert(i.first);
		}
	}

	while (!pairQueue.empty())
	{
		auto thisPair = pairQueue.front();
		pairQueue.pop();
		CCombineTreeNode* thisNode = pairToNode[thisPair];

		if (thisNode->group_tree_node == nullptr)
		{
			std::vector<CNode*> nodeSet2 = thisNode->tree2_node->children;
			for (int i = 0; i < nodeSet2.size(); ++i)
			{
				CNode* n2 = nodeSet2[i];
				CCombineTreeNode* childNode = new CCombineTreeNode();
				childNode->parent = thisNode;
				thisNode->node_type = NODE_TYPE::NONLEAF;
				childNode->group_tree_node_id = -n2->index;
				childNode->tree2_node_id = n2->index;
				childNode->group_tree_node = nullptr;
				childNode->tree2_node = n2;

				thisNode->children.push_back(childNode);

				pairQueue.push(std::pair<int, int>(-n2->index, n2->index));
				pairToNode[std::pair<int, int>(-n2->index, n2->index)] = childNode;
			}
		}
		else if (thisNode->tree2_node == nullptr)
		{
			std::vector<CGroupTreeNode*> nodeSet1 = thisNode->group_tree_node->children;
			for (int i = 0; i < nodeSet1.size(); ++i)
			{
				CGroupTreeNode* n1 = nodeSet1[i];
				CCombineTreeNode* childNode = new CCombineTreeNode();

				childNode->parent = thisNode;
				thisNode->node_type = NODE_TYPE::NONLEAF;

				childNode->group_tree_node_id = n1->index;
				childNode->tree2_node_id = -n1->index;
				childNode->group_tree_node = n1;
				childNode->tree2_node = nullptr;

				thisNode->children.push_back(childNode);

				pairQueue.push(std::pair<int, int>(n1->index, -n1->index));
				pairToNode[std::pair<int, int>(n1->index, -n1->index)] = childNode;
			}
		}
		else
		{
			std::vector<CGroupTreeNode*> nodeSet1;
			std::vector<CNode*> nodeSet2;

			nodeSet1 = thisNode->group_tree_node->children;
			nodeSet1.push_back(thisNode->group_tree_node);

			nodeSet2 = thisNode->tree2_node->children;
			nodeSet2.push_back(thisNode->tree2_node);

			std::vector<bool> nodeFlag1(nodeSet1.size(), false), nodeFlag2(nodeSet2.size(), false);

			for (int i = 0; i < nodeSet1.size(); ++i)
			{
				CGroupTreeNode* n1 = nodeSet1[i];
				for (int j = 0; j < nodeSet2.size(); ++j)
				{
					CNode* n2 = nodeSet2[j];
					std::pair<int, int> possiblePair(n1->index, n2->index);

					if (n1->index == thisNode->group_tree_node_id && n2->index == thisNode->tree2_node_id)
					{
						continue;
					}

					if (validCsp.find(possiblePair) != validCsp.end())
					{
						CCombineTreeNode* childNode = new CCombineTreeNode();
						childNode->parent = thisNode;
						thisNode->node_type = NODE_TYPE::NONLEAF;

						childNode->group_tree_node = n1;
						childNode->tree2_node = n2;
						childNode->group_tree_node_id = n1->index;
						childNode->tree2_node_id = n2->index;

						thisNode->children.push_back(childNode);

						pairQueue.push(possiblePair);
						pairToNode[possiblePair] = childNode;
						nodeFlag1[i] = true;
						nodeFlag2[j] = true;
					}
				}
			}
			std::cout << "n1:flag" << std::endl;
			for (auto i : nodeFlag1)
			{
				std::cout << i << std::endl;
			}
			//YZJ:Here are some questions.
			//YZJ:possible not pair,but not sure here.
			for (int i = 0; i < nodeSet1.size() - 1; ++i)
			{
				if (!nodeFlag1[i])
				{
					if (dangling_nodes_0.find(nodeSet1[i]->index) == dangling_nodes_0.end())
					{
						continue;
					}

					CGroupTreeNode* n1 = nodeSet1[i];
					CCombineTreeNode* childNode = new CCombineTreeNode();
					childNode->parent = thisNode;
					thisNode->node_type = NODE_TYPE::NONLEAF;

					childNode->group_tree_node_id = n1->index;
					childNode->tree2_node_id = -n1->index;
					childNode->group_tree_node = n1;
					childNode->tree2_node = nullptr;

					thisNode->children.push_back(childNode);

					pairQueue.push(std::pair<int, int>(n1->index, -n1->index));
					pairToNode[std::pair<int, int>(n1->index, -n1->index)] = childNode;
				}
			}
			for (int i = 0; i < nodeSet2.size() - 1; ++i)
			{
				if (!nodeFlag2[i])
				{
					if (dangling_nodes_1.find(nodeSet2[i]->index) == dangling_nodes_1.end())
					{
						continue;
					}

					CNode* n2 = nodeSet2[i];
					CCombineTreeNode* childNode = new CCombineTreeNode();
					childNode->parent = thisNode;
					thisNode->node_type = NODE_TYPE::NONLEAF;

					childNode->group_tree_node_id = -n2->index;
					childNode->tree2_node_id = n2->index;
					childNode->group_tree_node = nullptr;
					childNode->tree2_node = n2;

					thisNode->children.push_back(childNode);

					pairQueue.push(std::pair<int, int>(-n2->index, n2->index));
					pairToNode[std::pair<int, int>(-n2->index, n2->index)] = childNode;
				}
			}
		}
	}

	//YZJ:hang it only when it's done
	std::cout << "final matching" << std::endl;
	for (auto key : pairToNode)
	{
		//std::cout << key.first.first << " " << key.first.second << std::endl;
		std::cout << key.second->group_tree_node_id << " " << key.second->tree2_node_id << std::endl;
	}
}

void CNodeMatch::GetCrpInFirstTwoTree()
{
	if (combine_tree_root)
	{
		std::queue<CCombineTreeNode*> temp_queue;
		temp_queue.push(combine_tree_root);
		while (!temp_queue.empty())
		{
			CCombineTreeNode* node = temp_queue.front();
			temp_queue.pop();
			//迁移对应
			if (node->group_tree_node != nullptr)
			{
				node->tree0_node = node->group_tree_node->tree0_node;
				node->tree1_node = node->group_tree_node->tree1_node;
				node->tree0_node_id = node->group_tree_node->tree0_node_id;
				node->tree1_node_id = node->group_tree_node->tree1_node_id;
			}
			for (auto iter = node->children.begin(); iter != node->children.end(); iter++)
				temp_queue.push(*iter);
		}
	}
	return;
}

void CNodeMatch::AssignGroupTreeIndex()
{
	int start_index = 1;

	if (group_tree_root)
	{
		std::queue<CGroupTreeNode*> temp_queue;
		temp_queue.push(group_tree_root);
		while (!temp_queue.empty())
		{
			CGroupTreeNode* node = temp_queue.front();
			temp_queue.pop();
			node->index = start_index;
			start_index++;
			for (auto iter = node->children.begin(); iter != node->children.end(); iter++)
				temp_queue.push(*iter);
		}
	}

	std::cout << "check group tree" << std::endl;
	if (group_tree_root)
	{
		std::queue<CGroupTreeNode*> temp_queue;
		temp_queue.push(group_tree_root);
		while (!temp_queue.empty())
		{
			CGroupTreeNode* node = temp_queue.front();
			temp_queue.pop();

			if (node != group_tree_root)
			{
				std::cout << "index = " << node->index << "   first tree corres = " << node->tree0_node_id << "   second tree corres = " << node->tree1_node_id << "  parent index = " << node->parent->index << std::endl;
			}
			else
			{
				std::cout << "index = " << node->index << "   first tree corres = " << node->tree0_node_id << "   second tree corres = " << node->tree1_node_id << std::endl;
			}

			for (auto iter = node->children.begin(); iter != node->children.end(); iter++)
				temp_queue.push(*iter);
		}
	}
	std::cout << "check group tree" << std::endl;
	return;
}

void CNodeMatch::AssignCombineTreeIndex()
{
	int start_index = 1;
	if (combine_tree_root)
	{
		std::queue<CCombineTreeNode*> temp_queue;
		temp_queue.push(combine_tree_root);
		while (!temp_queue.empty())
		{
			CCombineTreeNode* node = temp_queue.front();
			temp_queue.pop();

			node->index = start_index;
			start_index++;

			for (auto iter = node->children.begin(); iter != node->children.end(); iter++)
				temp_queue.push(*iter);
		}
	}

	return;
}

void CNodeMatch::SetThirdTree(CNode* tree_root)
{
	third_tree_root = tree_root;
	return;
}

void CNodeMatch::ComputeGrpThdCsp()
{
	int cost = CalGrpThdCost(group_tree_root, third_tree_root);
	std::cout << "grp To thd cost = " << cost << std::endl;
	return;
}

int CNodeMatch::CalGrpThdCost(CGroupTreeNode* n1, CNode* n2)
{
	if (n1 == nullptr)
	{
		if (recordGrpThdCost.find(std::pair<int, int>(-n2->index, n2->index)) != recordGrpThdCost.end())
		{
			return recordGrpThdCost[std::pair<int, int>(-n2->index, n2->index)];
		}

		recordGrpThdCrp[std::pair<int, int>(-n2->index, n2->index)].insert(std::pair<int, int>(-n2->index, n2->index));
		if (n2->node_type == NODE_TYPE::LEAF)
		{
			int this_cost = LeafToNULLCost(n2);
			this_cost = this_cost * 2;		//注意要乘以二
			recordGrpThdCost[std::pair<int, int>(-n2->index, n2->index)] = this_cost;
			return this_cost;
		}

		//若n2为非叶子节点
		int	totalCost = 0;
		for (auto ptr : n2->children)
		{
			totalCost += CalGrpThdCost(nullptr, ptr);
			recordGrpThdCrp[std::pair<int, int>(-n2->index, n2->index)].insert(
				recordGrpThdCrp[std::pair<int, int>(-ptr->index, ptr->index)].begin(),
				recordGrpThdCrp[std::pair<int, int>(-ptr->index, ptr->index)].end());
		}
		recordGrpThdCost[std::pair<int, int>(-n2->index, n2->index)] = totalCost;
		return totalCost;
	}
	else if (n2 == nullptr)
	{
		//vanish cost
		if (recordGrpThdCost.find(std::pair<int, int>(n1->index, -n1->index)) != recordGrpThdCost.end())
		{
			return recordGrpThdCost[std::pair<int, int>(n1->index, -n1->index)];
		}

		recordGrpThdCrp[std::pair<int, int>(n1->index, -n1->index)].insert(std::pair<int, int>(n1->index, -n1->index));

		if (n1->node_type == NODE_TYPE::LEAF)
		{
			int first_cost = 0, second_cost = 0, total_cost = 0;
			if (n1->tree0_node != nullptr && n1->tree1_node != nullptr)
			{
				first_cost = std::abs(n1->tree0_node->width - 0) + std::abs(n1->tree0_node->height - 0);
				second_cost = std::abs(n1->tree1_node->width - 0) + std::abs(n1->tree1_node->height - 0);
				total_cost = first_cost + second_cost;
			}
			else if (n1->tree0_node == nullptr && n1->tree1_node != nullptr)
			{
				second_cost = std::abs(n1->tree1_node->width - 0) + std::abs(n1->tree1_node->height - 0);
				total_cost = first_cost + second_cost;
			}
			else
			{
				first_cost = std::abs(n1->tree0_node->width - 0) + std::abs(n1->tree0_node->height - 0);
				total_cost = first_cost + second_cost;
			}
			recordGrpThdCost[std::pair<int, int>(n1->index, -n1->index)] = total_cost;
			return total_cost;
		}

		int all_cost = 0;
		for (auto ptr : n1->children)
		{
			all_cost += CalGrpThdCost(ptr, nullptr);
			recordGrpThdCrp[std::pair<int, int>(n1->index, -n1->index)].insert(
				recordGrpThdCrp[std::pair<int, int>(ptr->index, -ptr->index)].begin(),
				recordGrpThdCrp[std::pair<int, int>(ptr->index, -ptr->index)].end());
		}
		recordGrpThdCost[std::pair<int, int>(n1->index, -n1->index)] = all_cost;
		return all_cost;
	}

	if (recordGrpThdCost.find(std::pair<int, int>(n1->index, n2->index)) != recordGrpThdCost.end())
	{
		return recordGrpThdCost[std::pair<int, int>(n1->index, n2->index)];
	}

	recordGrpThdCrp[std::pair<int, int>(n1->index, n2->index)].insert(std::pair<int, int>(n1->index, n2->index));

	if (n1->node_type == NODE_TYPE::LEAF && n2->node_type == NODE_TYPE::LEAF)
	{
		int first_cost = 0, second_cost = 0, total_cost = 0;
		if (n1->tree0_node != nullptr && n1->tree1_node != nullptr)
		{
			first_cost = LeafToLeafCost(n1->tree0_node, n2);
			second_cost = LeafToLeafCost(n1->tree1_node, n2);
			total_cost = first_cost + second_cost;
		}
		else if (n1->tree0_node == nullptr && n1->tree1_node != nullptr)
		{
			first_cost = std::abs(0 - n2->width) + std::abs(0 - n2->height);
			second_cost = LeafToLeafCost(n1->tree1_node, n2);
			total_cost = first_cost + second_cost;
		}
		else
		{
			first_cost = LeafToLeafCost(n1->tree0_node, n2);
			second_cost = std::abs(0 - n2->width) + std::abs(0 - n2->height);
			total_cost = first_cost + second_cost;
		}

		recordGrpThdCost[std::pair<int, int>(n1->index, n2->index)] = total_cost;
		return total_cost;
	}
	else if (n1->node_type == NODE_TYPE::NONLEAF && n2->node_type == NODE_TYPE::NONLEAF)
	{
		std::vector<CNode*> nodeSet2;
		std::vector<CGroupTreeNode*> nodeSet1;

		// case 1: children to children correspondence
		nodeSet1 = n1->children;
		nodeSet2 = n2->children;
		std::set<std::pair<int, int>> newCspCase1;

		int case1Cost = HungaryGrpThdCost(nodeSet1, nodeSet2, newCspCase1);

		// case 2: n1 to n2's children
		nodeSet1 = { n1 };
		nodeSet2 = n2->children;
		std::set<std::pair<int, int>> newCspCase2;
		int case2Cost = HungaryGrpThdCost(nodeSet1, nodeSet2, newCspCase2);


		// case 3: n1's children to n2
		nodeSet1 = n1->children;
		nodeSet2 = { n2 };
		std::set<std::pair<int, int>> newCspCase3;
		int case3Cost = HungaryGrpThdCost(nodeSet1, nodeSet2, newCspCase3);

		//std::cout << "case1Cost = " << case1Cost << "   case2Cost  = " << case2Cost << "  case3Cost = " <<case3Cost<< std::endl;

		int minimalCost = std::min(case1Cost, std::min(case2Cost, case3Cost));

		if (case1Cost == minimalCost)
		{
			bool without_corres = true;
			//std::cout << "newCspCase1 size = " << newCspCase1.size() << std::endl;
			for (auto each_pair : newCspCase1)
			{
				//std::cout << "each_pair.first = " << each_pair.first << "  each_pair.second = " << each_pair.second << std::endl;
				if (each_pair.first >= 0 && each_pair.second >= 0)
				{
					without_corres = false;
					break;
				}
			}
			if (without_corres == true)
			{
				//std::cout << "call nonleaf nonleaf here" << std::endl;
				//std::cout << n1->index << "     " << n2->index << std::endl;
				minimalCost = 100000;
			}
			recordGrpThdCrp[std::pair<int, int>(n1->index, n2->index)].insert(newCspCase1.begin(), newCspCase1.end());
			//minimalCost = case1Cost;
		}
		else if (case2Cost == minimalCost)
		{
			recordGrpThdCrp[std::pair<int, int>(n1->index, n2->index)].insert(newCspCase2.begin(), newCspCase2.end());
			//minimalCost = case2Cost;
		}
		else
		{
			recordGrpThdCrp[std::pair<int, int>(n1->index, n2->index)].insert(newCspCase3.begin(), newCspCase3.end());
			//minimalCost = case3Cost;
		}

		recordGrpThdCost[std::pair<int, int>(n1->index, n2->index)] = minimalCost;

		return minimalCost;
	}
	else if (n1->node_type == NODE_TYPE::NONLEAF && n2->node_type == NODE_TYPE::LEAF)
	{
		std::vector<CGroupTreeNode*> nodeSet1;
		std::vector<CNode*> nodeSet2;

		nodeSet1 = n1->children;
		nodeSet2 = { n2 };
		std::set<std::pair<int, int>> newCsp;

		int thisCost = HungaryGrpThdCost(nodeSet1, nodeSet2, newCsp);

		bool without_corres = true;
		for (auto each_pair : newCsp)
		{
			if (each_pair.first >= 0 && each_pair.second >= 0)
			{
				without_corres = false;
				break;
			}
		}
		if (without_corres == true)
		{
			thisCost = 100000;
		}

		recordGrpThdCrp[std::pair<int, int>(n1->index, n2->index)].insert(newCsp.begin(), newCsp.end());
		recordGrpThdCost[std::pair<int, int>(n1->index, n2->index)] = thisCost;
		return thisCost;
	}
	else if (n1->node_type == NODE_TYPE::LEAF && n2->node_type == NODE_TYPE::NONLEAF)
	{
		std::vector<CNode*> nodeSet2;
		std::vector<CGroupTreeNode*> nodeSet1;

		nodeSet1 = { n1 };
		nodeSet2 = n2->children;
		std::set<std::pair<int, int>> newCsp;

		int thisCost = HungaryGrpThdCost(nodeSet1, nodeSet2, newCsp);

		bool without_corres = true;
		for (auto each_pair : newCsp)
		{
			if (each_pair.first >= 0 && each_pair.second >= 0)
			{
				without_corres = false;
				break;
			}
		}
		if (without_corres == true)
		{
			thisCost = 100000;
		}

		recordGrpThdCrp[std::pair<int, int>(n1->index, n2->index)].insert(newCsp.begin(), newCsp.end());
		recordGrpThdCost[std::pair<int, int>(n1->index, n2->index)] = thisCost;

		return thisCost;
	}

	return 0;
}

CCombineTreeNode* CNodeMatch::GetCombineTreeRoot()
{
	if (combine_tree_root != nullptr)
	{
		return combine_tree_root;
	}
	else
	{
		std::cout << "combine_tree_root empty" << std::endl;
		return nullptr;
	}
}

int CNodeMatch::LeafToNULLCost(CNode* node)
{
	return (abs(0 - node->width) + abs(0 - node->height));
}

int CNodeMatch::LeafToLeafCost(CNode* n1, CNode* n2)
{
	if (n1->node_present_type != n2->node_present_type)
	{
		return 100000;	//当两个叶子节点的语义信息不同时，返回一个较大的cost。
	}

	int leafToleaf_cost = 0;
	leafToleaf_cost = abs(n1->x - n2->x) + abs(n1->y - n2->y);    //位置
	leafToleaf_cost += (abs(n1->width - n2->width) + abs(n1->height - n2->height));    //大小
	return leafToleaf_cost;
}

int CNodeMatch::GroupLeafToLeafCost(CGroupTreeNode* n1, CNode* n2)
{
	int groupleaf_leaf_cost = 0;

	if (n1->tree0_node != nullptr && n1->tree1_node != nullptr)
	{
		int first_cost = LeafToLeafCost(n1->tree0_node, n2);
		int second_cost = LeafToLeafCost(n1->tree1_node, n2);
		groupleaf_leaf_cost = first_cost + second_cost;
		return groupleaf_leaf_cost;
	}
	else if (n1->tree0_node == nullptr && n1->tree1_node != nullptr)
	{
		//first_cost为node_two生成的cost
		int first_cost = 0;
		first_cost = std::abs(0 - n2->width) + std::abs(0 - n2->height);
		int second_cost = LeafToLeafCost(n1->tree1_node, n2);
		groupleaf_leaf_cost = first_cost + second_cost;
		return groupleaf_leaf_cost;
	}
	else if (n1->tree0_node != nullptr && n1->tree1_node == nullptr)
	{
		int first_cost = LeafToLeafCost(n1->tree0_node, n2);
		int second_cost = 0;
		second_cost = std::abs(0 - n2->width) + std::abs(0 - n2->height);
		groupleaf_leaf_cost = first_cost + second_cost;
		return groupleaf_leaf_cost;
	}

	return groupleaf_leaf_cost;
}

bool CNodeMatch::PairExistedInCombineTree(std::pair<int, int>& thisPair, CCombineTreeNode* combine_tree_root_)
{
	if (combine_tree_root_)
	{
		std::queue<CCombineTreeNode*> temp_queue;
		temp_queue.push(combine_tree_root_);
		while (!temp_queue.empty())
		{
			CCombineTreeNode* front_node = temp_queue.front();
			temp_queue.pop();
			if (front_node->group_tree_node_id == thisPair.first && front_node->tree2_node_id == thisPair.second)
			{
				return true;
			}
			for (int i = 0; i < front_node->children.size(); i++)
			{
				temp_queue.push(front_node->children[i]);
			}
		}
	}
	return false;
}

bool CNodeMatch::OnlyPairFirstExistedInCombineTree(std::pair<int, int>& thisPair, CCombineTreeNode* combine_tree_root_)
{
	bool first_flag = true;
	bool second_flag = false;
	bool third_flag = false;
	if (combine_tree_root_)
	{
		std::queue<CCombineTreeNode*> temp_queue;
		temp_queue.push(combine_tree_root_);
		while (!temp_queue.empty())
		{
			CCombineTreeNode* front_node = temp_queue.front();
			temp_queue.pop();
			if (front_node->group_tree_node_id == thisPair.first && front_node->tree2_node_id == thisPair.second)
			{
				first_flag = false;
			}
			if (front_node->group_tree_node_id == thisPair.first)
			{
				second_flag = true;
			}
			if (front_node->tree2_node_id == thisPair.second)
			{
				third_flag = true;
			}
			for (int i = 0; i < front_node->children.size(); i++)
			{
				temp_queue.push(front_node->children[i]);
			}
		}
	}
	if (first_flag == true && second_flag == true && third_flag == false)
	{
		return true;
	}
	return false;
}

bool CNodeMatch::PairDoNotExistedInCombineTree(std::pair<int, int>& thisPair, CCombineTreeNode* combine_tree_root_)
{
	bool first_flag = true;
	bool second_flag = true;
	bool third_flag = true;

	if (combine_tree_root_)
	{
		std::queue<CCombineTreeNode*> temp_queue;
		temp_queue.push(combine_tree_root_);
		while (!temp_queue.empty())
		{
			CCombineTreeNode* front_node = temp_queue.front();
			temp_queue.pop();

			if (front_node->group_tree_node_id == thisPair.first && front_node->tree2_node_id == thisPair.second)
			{
				first_flag = false;
			}

			if (front_node->group_tree_node_id == thisPair.first)
			{
				second_flag = false;
			}

			if (front_node->tree2_node_id == thisPair.second)
			{
				third_flag = false;
			}

			for (int i = 0; i < front_node->children.size(); i++)
			{
				temp_queue.push(front_node->children[i]);
			}
		}
	}

	if (first_flag && second_flag && third_flag)
	{
		return true;
	}

	return false;
}

bool CNodeMatch::OnlyPairSecondExistedInCombineTree(std::pair<int, int>& thisPair, CCombineTreeNode* combine_tree_root_)
{
	bool first_flag = true;
	bool second_flag = false;
	bool third_flag = false;
	if (combine_tree_root_)
	{
		std::queue<CCombineTreeNode*> temp_queue;
		temp_queue.push(combine_tree_root_);
		while (!temp_queue.empty())
		{
			CCombineTreeNode* front_node = temp_queue.front();
			temp_queue.pop();
			if (front_node->group_tree_node_id == thisPair.first && front_node->tree2_node_id == thisPair.second)
			{
				first_flag = false;
			}
			if (front_node->group_tree_node_id == thisPair.first)
			{
				second_flag = true;
			}
			if (front_node->tree2_node_id == thisPair.second)
			{
				third_flag = true;
			}
			for (int i = 0; i < front_node->children.size(); i++)
			{
				temp_queue.push(front_node->children[i]);
			}
		}
	}
	if (first_flag == true && second_flag == false && third_flag == true)
	{
		return true;
	}
	return false;
}

bool CNodeMatch::PairExistedInGroupTree(std::pair<int, int>& thisPair, CGroupTreeNode* group_tree_root_)
{
	if (group_tree_root_)
	{
		std::queue<CGroupTreeNode*> temp_queue;
		temp_queue.push(group_tree_root_);
		while (!temp_queue.empty())
		{
			CGroupTreeNode* front_node = temp_queue.front();
			temp_queue.pop();
			if (front_node->tree0_node_id == thisPair.first && front_node->tree1_node_id == thisPair.second)
			{
				return true;
			}
			for (int i = 0; i < front_node->children.size(); i++)
			{
				temp_queue.push(front_node->children[i]);
			}
		}
	}
	return false;
}

bool CNodeMatch::OnlyPairFirstExistedInGroupTree(std::pair<int, int>& thisPair, CGroupTreeNode* group_tree_root_)
{
	bool first_flag = true;
	bool second_flag = false;
	bool third_flag = false;
	if (group_tree_root_)
	{
		std::queue<CGroupTreeNode*> temp_queue;
		temp_queue.push(group_tree_root_);
		while (!temp_queue.empty())
		{
			CGroupTreeNode* front_node = temp_queue.front();
			temp_queue.pop();
			if (front_node->tree0_node_id == thisPair.first && front_node->tree1_node_id == thisPair.second)
			{
				first_flag = false;
			}
			if (front_node->tree0_node_id == thisPair.first)
			{
				second_flag = true;
			}
			if (front_node->tree1_node_id == thisPair.second)
			{
				third_flag = true;
			}
			for (int i = 0; i < front_node->children.size(); i++)
			{
				temp_queue.push(front_node->children[i]);
			}
		}
	}
	if (first_flag == true && second_flag == true && third_flag == false)
	{
		return true;
	}
	return false;
}

bool CNodeMatch::PairDoNotExistedInGroupTree(std::pair<int, int>& thisPair, CGroupTreeNode* group_tree_root_)
{
	bool first_flag = true;
	bool second_flag = true;
	bool third_flag = true;

	if (group_tree_root_)
	{
		std::queue<CGroupTreeNode*> temp_queue;
		temp_queue.push(group_tree_root_);
		while (!temp_queue.empty())
		{
			CGroupTreeNode* front_node = temp_queue.front();
			temp_queue.pop();

			if (front_node->tree0_node_id == thisPair.first && front_node->tree1_node_id == thisPair.second)
			{
				first_flag = false;
			}

			if (front_node->tree0_node_id == thisPair.first)
			{
				second_flag = false;
			}

			if (front_node->tree1_node_id == thisPair.second)
			{
				third_flag = false;
			}

			for (int i = 0; i < front_node->children.size(); i++)
			{
				temp_queue.push(front_node->children[i]);
			}
		}
	}

	if (first_flag && second_flag && third_flag)
	{
		return true;
	}
	return false;
}

bool CNodeMatch::OnlyPairSecondExistedInGroupTree(std::pair<int, int>& thisPair, CGroupTreeNode* group_tree_root_)
{
	bool first_flag = true;
	bool second_flag = false;
	bool third_flag = false;
	if (group_tree_root_)
	{
		std::queue<CGroupTreeNode*> temp_queue;
		temp_queue.push(group_tree_root_);
		while (!temp_queue.empty())
		{
			CGroupTreeNode* front_node = temp_queue.front();
			temp_queue.pop();
			if (front_node->tree0_node_id == thisPair.first && front_node->tree1_node_id == thisPair.second)
			{
				first_flag = false;
			}
			if (front_node->tree0_node_id == thisPair.first)
			{
				second_flag = true;
			}
			if (front_node->tree1_node_id == thisPair.second)
			{
				third_flag = true;
			}
			for (int i = 0; i < front_node->children.size(); i++)
			{
				temp_queue.push(front_node->children[i]);
			}
		}
	}
	if (first_flag == true && second_flag == false && third_flag == true)
	{
		return true;
	}
	return false;
}

CCombineTreeNode* CNodeMatch::ReturnParentNode(int index1, int index2)
{
	if (combine_tree_root)
	{
		std::queue<CCombineTreeNode*> temp_queue;
		temp_queue.push(combine_tree_root);
		while (!temp_queue.empty())
		{
			CCombineTreeNode* front_node = temp_queue.front();
			temp_queue.pop();
			if (front_node->group_tree_node_id == index1 && front_node->tree2_node_id == index2)
			{
				return front_node;
			}
			for (int i = 0; i < front_node->children.size(); i++)
			{
				temp_queue.push(front_node->children[i]);
			}
		}
	}
	return nullptr;
}

CCombineTreeNode* CNodeMatch::ReturnRightParentNode(int index2)
{
	CCombineTreeNode* return_node = nullptr;

	if (combine_tree_root)
	{
		std::queue<CCombineTreeNode*> temp_queue;
		temp_queue.push(combine_tree_root);
		while (!temp_queue.empty())
		{
			CCombineTreeNode* front_node = temp_queue.front();
			temp_queue.pop();
			if (front_node->tree2_node_id == index2)
			{
				return_node = front_node;
			}
			for (int i = 0; i < front_node->children.size(); i++)
			{
				temp_queue.push(front_node->children[i]);
			}
		}
	}
	return return_node;
}

CCombineTreeNode* CNodeMatch::ReturnLeftParentNode(int index1)
{
	CCombineTreeNode* return_node = nullptr;
	if (combine_tree_root)
	{
		std::queue<CCombineTreeNode*> temp_queue;
		temp_queue.push(combine_tree_root);
		while (!temp_queue.empty())
		{
			CCombineTreeNode* front_node = temp_queue.front();
			temp_queue.pop();
			if (front_node->group_tree_node_id == index1)
			{
				return_node = front_node;
			}
			for (int i = 0; i < front_node->children.size(); i++)
			{
				temp_queue.push(front_node->children[i]);
			}
		}
	}
	return return_node;
}

CGroupTreeNode* CNodeMatch::ReturnParentGroupNode(int index1, int index2)
{
	if (group_tree_root)
	{
		std::queue<CGroupTreeNode*> temp_queue;
		temp_queue.push(group_tree_root);
		while (!temp_queue.empty())
		{
			CGroupTreeNode* front_node = temp_queue.front();
			temp_queue.pop();
			if (front_node->tree0_node_id == index1 && front_node->tree1_node_id == index2)
			{
				return front_node;
			}
			for (int i = 0; i < front_node->children.size(); i++)
			{
				temp_queue.push(front_node->children[i]);
			}
		}
	}
	return nullptr;
}

CGroupTreeNode* CNodeMatch::ReturnRightParentGroupNode(int index2)
{
	CGroupTreeNode* return_node = nullptr;

	if (group_tree_root)
	{
		std::queue<CGroupTreeNode*> temp_queue;
		temp_queue.push(group_tree_root);
		while (!temp_queue.empty())
		{
			CGroupTreeNode* front_node = temp_queue.front();
			temp_queue.pop();
			if (front_node->tree1_node_id == index2)
			{
				return_node = front_node;
			}
			for (int i = 0; i < front_node->children.size(); i++)
			{
				temp_queue.push(front_node->children[i]);
			}
		}
	}
	return return_node;
}

CGroupTreeNode* CNodeMatch::ReturnLeftParentGroupNode(int index1)
{
	CGroupTreeNode* return_node = nullptr;
	if (group_tree_root)
	{
		std::queue<CGroupTreeNode*> temp_queue;
		temp_queue.push(group_tree_root);
		while (!temp_queue.empty())
		{
			CGroupTreeNode* front_node = temp_queue.front();
			temp_queue.pop();
			if (front_node->tree0_node_id == index1)
			{
				return_node = front_node;
			}
			for (int i = 0; i < front_node->children.size(); i++)
			{
				temp_queue.push(front_node->children[i]);
			}
		}
	}
	return return_node;
}

