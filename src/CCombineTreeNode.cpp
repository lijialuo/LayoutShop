#include "CCombineTreeNode.h"

void CCombineTreeNode::DeleteNode(CCombineTreeNode* node)
{
	if (node == nullptr)
	{
		return;
	}

	if (node->parent != nullptr)
	{
		node->parent->children.erase(std::find(node->parent->children.begin(), node->parent->children.end(), node));
		/*if (node->parent->children.size() == 0)
		{
			node->parent->node_type = NODE_TYPE::LEAF;
		}*/
		node->parent = nullptr;
	}

	auto pointer = node;

	while (true)
	{
		//有点像深度优先
		while (!pointer->children.empty())
		{
			pointer = pointer->children.back();
		}
		auto pointer_parent = pointer->parent;

		delete pointer;
		pointer = nullptr;
		pointer = pointer_parent;

		if (pointer == nullptr)
		{
			break;
		}

		auto back_iterator = pointer->children.end() - 1;
		pointer->children.erase(back_iterator);
	}

	node = nullptr;
}

void CCombineTreeNode::DeepDestroy(CCombineTreeNode* aNode)
{
	std::queue<CCombineTreeNode*> nodeQueue;
	nodeQueue.push(aNode);

	while (!nodeQueue.empty())
	{
		CCombineTreeNode* thisNode = nodeQueue.front();
		nodeQueue.pop();
		for (int i = 0; i < thisNode->children.size(); ++i)
		{
			nodeQueue.push(thisNode->children[i]);
		}
		delete thisNode;
		thisNode = nullptr;
	}

	return;
}

CCombineTreeNode* CCombineTreeNode::DeepCopy()
{
	CCombineTreeNode* newRoot = new CCombineTreeNode();
	*newRoot = *this;
	std::queue<CCombineTreeNode*> nodeQueue;
	nodeQueue.push(newRoot);

	while (!nodeQueue.empty())
	{
		CCombineTreeNode* thisNode = nodeQueue.front();
		nodeQueue.pop();
		std::vector<CCombineTreeNode*> newChildren;

		for (int i = 0; i < thisNode->children.size(); i++)
		{
			CCombineTreeNode* newChild = new CCombineTreeNode();
			*newChild = *(thisNode->children[i]);

			newChild->parent = thisNode;
			newChildren.push_back(newChild);
			nodeQueue.push(newChild);
		}
		thisNode->children = newChildren;
		newChildren.clear();
	}
	return newRoot;
}