#include "CLayoutTree.h"
#include <fstream>
#include <sstream>
#include <iostream>

CLayoutTree::CLayoutTree()
{
	this->root = nullptr;
}

CLayoutTree::CLayoutTree(CNode* root)
{
	this->root = root;
}


CLayoutTree::~CLayoutTree()
{
	DestroyAllData();
}

void CLayoutTree::InsertNode(CNode* node, CNode* parent)
{
	if (node)
	{
		if (!parent)
		{
			if (root)
			{
				parent = root;
			}
			else
			{
				root = node;
				return;
			}
		}
		node->parent = parent;
		parent->children.push_back(node);
		parent->node_type = NODE_TYPE::NONLEAF;		//插入节点时将父节点的类型设为非叶子节点
	}
	else
	{
		return;
	}
}

void CLayoutTree::DeleteNode(CNode* node)
{
	if (node == nullptr)
	{
		return;
	}
	if (node->parent != nullptr)
	{
		node->parent->children.erase(std::find(node->parent->children.begin(), node->parent->children.end(), node));
		if (node->parent->children.size() == 0)
		{
			node->parent->node_type = NODE_TYPE::LEAF;
		}
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

	//要有,node置为nullptr
	node = nullptr;
}

void CLayoutTree::DestroyAllData()
{
	DeleteNode(root);
	ptr_idx_map.clear();
	idx_ptr_map.clear();

	return;
}

bool GetLineFromInfile(std::ifstream &infile,std::string &line)
{
    if(getline(infile,line))
    {
#ifdef __APPLE__
        if(line.back() == '\r')
            line.erase(line.size() - 1);
#endif
        //std::cout<<line<<" ";
        return true;
    }
    else return false;
}

void CLayoutTree::ReadFromFile(std::string fileName)
{
	DestroyAllData();

	std::ifstream infile;	//输入文件流
	infile.open(fileName.data());

	int nodes_number = 0;
	int equal_size_num = 0;
	int equal_space_num = 0;
	int left_align_num = 0;
	int right_align_num = 0;
	int top_align_num = 0;
	int bottom_align_num = 0;

	std::string each_line_string;	//每一行的字符串
	std::string split_string;
	std::vector<int> each_line_data;

	while (GetLineFromInfile(infile,each_line_string))
	{
		if (each_line_string == "node attribute")
		{
            GetLineFromInfile(infile,each_line_string);
			nodes_number = StringToInteger(each_line_string);
			for (int i = 0; i < nodes_number; i++)
			{
                GetLineFromInfile(infile,each_line_string);
				std::istringstream record(each_line_string);	//按空格分隔字符串

				while (record >> split_string)
				{
					each_line_data.push_back(StringToInteger(split_string));
				}

				if (each_line_data.back() == 0)
				{
					//根节点,先全设为LEAF,插入的时候会修改NODE_TYPE
					root = new CNode(each_line_data[0], NODE_TYPE::LEAF, each_line_data[3], each_line_data[4], each_line_data[5],
						each_line_data[6]);

					ptr_idx_map[root] = root->index;
					idx_ptr_map[root->index] = root;

					root->node_present_type = SetNodePresentType(each_line_data[7]);
				}
				else
				{
					CNode* non_root = new CNode(each_line_data[0], NODE_TYPE::LEAF, each_line_data[3], each_line_data[4], each_line_data[5],
						each_line_data[6]);

					ptr_idx_map[non_root] = non_root->index;
					idx_ptr_map[non_root->index] = non_root;

					non_root->node_present_type = SetNodePresentType(each_line_data[7]);

					this->InsertNode(non_root, idx_ptr_map[each_line_data[8]]);
				}

				each_line_data.clear();
			}
		}
		else if (each_line_string == "equal size")
		{
            GetLineFromInfile(infile,each_line_string);
			equal_size_num = StringToInteger(each_line_string);

			for (int i = 0; i < equal_size_num; i++)
			{
                GetLineFromInfile(infile,each_line_string);
				std::istringstream record(each_line_string);	//按空格分隔字符串

				while (record >> split_string)
				{
					each_line_data.push_back(StringToInteger(split_string));
				}

				std::pair<int, int> temp_pair(each_line_data[0], each_line_data[1]);
				idx_ptr_map[each_line_data[2]]->equal_size_node.insert(temp_pair);
				each_line_data.clear();
			}
		}
		else if (each_line_string == "equal space")
		{
            GetLineFromInfile(infile,each_line_string);
			equal_space_num = StringToInteger(each_line_string);

			for (int i = 0; i < equal_space_num; i++)
			{
                GetLineFromInfile(infile,each_line_string);
				std::istringstream record(each_line_string);	//按空格分隔字符串

				while (record >> split_string)
				{
					each_line_data.push_back(StringToInteger(split_string));
				}

				int vec_size = each_line_data.size();

				//each_line_data的最后一位是父节点的index,倒数第二位是euqal space模式（竖 或 横）
				//1表示竖着equal space    0表示横着equal space
				for (int j = 0; j < vec_size - 1; j++)
				{
					idx_ptr_map[each_line_data[vec_size - 1]]->equal_space_node.push_back(each_line_data[j]);
				}

				each_line_data.clear();
			}
		}
		else if (each_line_string == "left align")
		{
            GetLineFromInfile(infile,each_line_string);
			left_align_num = StringToInteger(each_line_string);

			for (int i = 0; i < left_align_num; i++)
			{
                GetLineFromInfile(infile,each_line_string);
				std::istringstream record(each_line_string);	//按空格分隔字符串

				while (record >> split_string)
				{
					each_line_data.push_back(StringToInteger(split_string));
				}

				int vec_size = each_line_data.size();

				for (int j = 0; j < vec_size - 1; j++)
				{
					idx_ptr_map[each_line_data[vec_size - 1]]->left_align_node.push_back(each_line_data[j]);
				}

				each_line_data.clear();
			}
		}
		else if (each_line_string == "right align")
		{
            GetLineFromInfile(infile,each_line_string);
			right_align_num = StringToInteger(each_line_string);

			for (int i = 0; i < right_align_num; i++)
			{
                GetLineFromInfile(infile,each_line_string);
				std::istringstream record(each_line_string);	//按空格分隔字符串

				while (record >> split_string)
				{
					each_line_data.push_back(StringToInteger(split_string));
				}

				int vec_size = each_line_data.size();

				for (int j = 0; j < vec_size - 1; j++)
				{
					idx_ptr_map[each_line_data[vec_size - 1]]->right_align_node.push_back(each_line_data[j]);
				}

				each_line_data.clear();
			}
		}
		else if (each_line_string == "top align")
		{
            GetLineFromInfile(infile,each_line_string);
			top_align_num = StringToInteger(each_line_string);

			for (int i = 0; i < top_align_num; i++)
			{
                GetLineFromInfile(infile,each_line_string);
				std::istringstream record(each_line_string);	//按空格分隔字符串

				while (record >> split_string)
				{
					each_line_data.push_back(StringToInteger(split_string));
				}

				int vec_size = each_line_data.size();

				for (int j = 0; j < vec_size - 1; j++)
				{
					idx_ptr_map[each_line_data[vec_size - 1]]->top_align_node.push_back(each_line_data[j]);
				}

				each_line_data.clear();
			}
		}
		else if (each_line_string == "bottom align")
		{
            GetLineFromInfile(infile,each_line_string);
			bottom_align_num = StringToInteger(each_line_string);

			for (int i = 0; i < bottom_align_num; i++)
			{
                GetLineFromInfile(infile,each_line_string);
				std::istringstream record(each_line_string);	//按空格分隔字符串

				while (record >> split_string)
				{
					each_line_data.push_back(StringToInteger(split_string));
				}

				int vec_size = each_line_data.size();

				for (int j = 0; j < vec_size - 1; j++)
				{
					idx_ptr_map[each_line_data[vec_size - 1]]->bottom_align_node.push_back(each_line_data[j]);
				}

				each_line_data.clear();
			}
		}
		else
		{
			std::cout << "tree中待添加关系" << std::endl;
		}
	}
	infile.close();
	return;
}

void CLayoutTree::BreadthFirstSearch()
{
	if (root)
	{
		std::queue<CNode*> temp_queue;
		temp_queue.push(root);
		while (!temp_queue.empty())
		{
			CNode* node = temp_queue.front();
			temp_queue.pop();

			std::cout << "节点的index = " << node->index;

			for (auto iter = node->children.begin(); iter != node->children.end(); iter++)
				temp_queue.push(*iter);
		}
	}
	else
	{
		return;
	}
}

void CLayoutTree::DrawLayout(QPainter& painter, double resizeCoef)
{
	//缩放系数
	if (resizeCoef <= 0)
		resizeCoef = 1;

	if (root)
	{
		std::queue<CNode*> temp_queue;
		temp_queue.push(root);
		while (!temp_queue.empty())
		{
			CNode* node = temp_queue.front();
			temp_queue.pop();
			if (node->node_type == LEAF)//叶子结点就plot出来
			{
				DrawLeafNode(painter, node, resizeCoef);
			}
			//std::cout << "node index = " << node->index << "         " << "node correspond = " << node->correspond_index << "          node width = " << node->width << "\n";
			for (auto iter = node->children.begin(); iter != node->children.end(); iter++)
				temp_queue.push(*iter);
		}
	}
	else
	{
		std::cout << "fail" << std::endl;
	}
	return;
}

int CLayoutTree::StringToInteger(std::string str)
{
	int number = 0;
	std::stringstream ss;
	ss << str;
	ss >> number;
	return number;
}

NODE_PRESENT_TYPE CLayoutTree::SetNodePresentType(int flag)
{
	if (flag == 0)
	{
		return NODE_PRESENT_TYPE::NONLABEL;
	}
	else if (flag == 1)
	{
		return NODE_PRESENT_TYPE::PICTURE;
	}
	else if (flag == 2)
	{
		return NODE_PRESENT_TYPE::TEXT;
	}
	else if (flag == 3)
	{
		return NODE_PRESENT_TYPE::PADDING;
	}
	else if (flag == 4)
	{
		return NODE_PRESENT_TYPE::TITLE;
	}
	else
	{
		std::cout << "SetNodePresentType call here" << std::endl;
	}
	return NODE_PRESENT_TYPE();
}

void CLayoutTree::DrawLeafNode(QPainter& paint, CNode* node_, double& resizeCoef)
{
	CNode* node = new CNode();
	node->x = int(node_->x * resizeCoef);
	node->y = int(node_->y * resizeCoef);
	node->width = int(node_->width * resizeCoef);
	node->height = int(node_->height * resizeCoef);
	node->node_present_type = node_->node_present_type;

	int move = 0;  // 为了使得绘制出的Rect不要贴紧左上角，加了一个偏置值

	QColor label_color[4] = {
		// 0:none  1:image  2:text 3:title
		QColor(150, 150, 150, 30),  // none(其实是padding)
		QColor(45, 131, 109, 220),  // image
		QColor(75, 151, 209, 100),  // text
		// QColor(150, 131, 109, 120)  // title
		QColor(120, 120, 120, 100)
	};

	QRect rect;
	QColor text_color;
	QPen thisPen;
	thisPen.setWidth(0);
	thisPen.setColor(QColor(255, 255, 255, 0));
	paint.setPen(thisPen);

	//QSvgRenderer image_icon;
	//image_icon.load(QString("./Resources/ContentImage/image.svg"));

	int font_size = 0;
	if (node->height >= 20.0 && node->height < 60.0)
	{
		font_size = int(node->height * 0.8);
	}
	else if (node->height >= 60.0)
	{
		font_size = 45;
	}
	else
	{
		font_size = int(node->height);
	}

	QFont font = paint.font();
	font.setPixelSize(font_size);
	font.setFamily("Arial");
	font.setBold(true);
	paint.setFont(font);
	rect = QRect(node->x + move, node->y + move, node->width, node->height);

	if (node->node_present_type == NODE_PRESENT_TYPE::PADDING)
	{
		paint.setBrush(label_color[0]);
		paint.drawRoundedRect(rect, 5, 5);
	}
	else if (node->node_present_type == NODE_PRESENT_TYPE::PICTURE)
	{
		paint.setBrush(label_color[1]);
		paint.drawRoundedRect(rect, 5, 5);
	}
	else if (node->node_present_type == NODE_PRESENT_TYPE::TITLE)
	{
		paint.setBrush(label_color[3]);
		paint.drawRoundedRect(rect, 5, 5);
		text_color = QColor(30, 30, 30);
		paint.setPen(text_color);
		paint.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, " Title");

		/*if (1.0 * node->width / node->height > 3.0) {
			paint.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, " Headline");
		}
		else if (1.0 * node->width / node->height <= 0.5)
		{
			paint.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, " T");
		}
		else
		{
			paint.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, " Title");
		}*/
	}
	else if (node->node_present_type == NODE_PRESENT_TYPE::TEXT)
	{
		QRect text_rect;
		int count = 0;
		int text_width = node->width;
		int text_x = rect.x() + 5;

		paint.setBrush(label_color[2]);
		paint.drawRoundedRect(rect, 5, 5);

		for (int i = 0; i <= node->height - 18; i += 14) {
			count++;
			if (count % 8 == 0) {
				text_x = rect.x() + 5;
				text_width = int(node->width / 2);
			}
			else if (count % 8 == 1) {
				text_x = rect.x() + 15;
				text_width = node->width - 10;
			}
			else {
				text_x = rect.x() + 5;
				text_width = node->width;
			}
			text_rect = QRect(text_x, rect.y() + i + 6, text_width - 10, 6);
			paint.setBrush(QColor(60, 60, 60, 220));
			paint.drawRoundedRect(text_rect, 2, 2);
			if (count % 8 == 0) {
				i += 15;
			}
		}
	}
	else
	{
		std::cout << "node index = " << node->index << "     暂时还没有的label" << std::endl;
	}

	delete node;
	node = nullptr;
}
