#include "LayoutGeneratorNew.h"

#include <QGuiApplication>
#include <iostream>
#include <ctime>
#include <qfontmetrics.h>
#include <stack>


//void LayoutGeneratorNew::SetTreeCandidates(std::vector<CCombineTreeNode*> tree_candidates)
//{
//	tree_candidates_ = tree_candidates;
//}



void LayoutGeneratorNew::SetArticle(Article* article)
{
	article_ = article;
}

void LayoutGeneratorNew::SetCombineTree(CCombineTreeNode* tree)
{
	combine_tree_ = tree;
}

void LayoutGeneratorNew::TreePreProcess()
{
	std::queue<CCombineTreeNode*> combine_q;
	allnodes.clear();
	ptr_idx_map.clear();
	idx_ptr_map.clear();
	img_node_list.clear();

	combine_q.push(combine_tree_);
	while (!combine_q.empty())
	{
		idx_ptr_map[combine_q.front()->index] = combine_q.front();
		ptr_idx_map[combine_q.front()] = allnodes.size();
		allnodes.push_back(combine_q.front());
		combine_q.pop();
		////std::cout << allnodes.back()->index << std::endl;
		for (auto ptr : allnodes.back()->children)
		{
			combine_q.push(ptr);
		}
	}

	for (int i = 0; i < allnodes.size(); ++i)
	{
		if (allnodes[i]->node_present_type == NODE_PRESENT_TYPE::PICTURE)
		{
			img_node_list.push_back(allnodes[i]);
		}
	}
}

void LayoutGeneratorNew::InitVariables(GRBModel& model)
{
	x.clear();
	img_corres_x.clear();
	linear_constrs.clear();
	q_constrs.clear();
	/*align_x1.clear();
	align_x2.clear();*/
	for (int i = 0; i < allnodes.size(); ++i)
	{
		x.push_back(model.addVar(0, W, 0.0, GRB_CONTINUOUS, "x" + (i * 4)));
		x.push_back(model.addVar(0, H, 0.0, GRB_CONTINUOUS, "x" + (i * 4 + 1)));
		x.push_back(model.addVar(100, W, 0.0, GRB_CONTINUOUS, "x" + (i * 4 + 2)));
		x.push_back(model.addVar(100, H, 0.0, GRB_CONTINUOUS, "x" + (i * 4 + 3)));
	}
	//padding
	padding_index_h = allnodes.size() * 4;
	padding_index_v = allnodes.size() * 4 + 1;
	x.push_back(model.addVar(10.0, 30.0, 0.0, GRB_CONTINUOUS, "x" + padding_index_h)); //h padding
	x.push_back(model.addVar(10.0, 30.0, 0.0, GRB_CONTINUOUS, "x" + padding_index_v)); //v padding

	margin_index_left = allnodes.size() * 4 + 2;
	margin_index_right = margin_index_left + 1;
	margin_index_top = margin_index_right + 1;
	margin_index_bottom = margin_index_top + 1;
	//margin
	x.push_back(model.addVar(30.0, 80.0, 0.0, GRB_CONTINUOUS, "x" + margin_index_left)); //left margin
	x.push_back(model.addVar(30.0, 80.0, 0.0, GRB_CONTINUOUS, "x" + margin_index_right)); //right margin
	x.push_back(model.addVar(30.0, 80.0, 0.0, GRB_CONTINUOUS, "x" + margin_index_top)); //top margin
	x.push_back(model.addVar(30.0, 80.0, 0.0, GRB_CONTINUOUS, "x" + margin_index_bottom)); //bottom margin


	for (int i = 0; i < article_->img_list_.size(); ++i)
	{
		img_corres_x.push_back(std::vector<GRBVar>());
		for (int j = 0; j < img_node_list.size(); ++j)
		{
			img_corres_x[i].push_back(model.addVar(0, 1, 0, GRB_BINARY, "img_corres_" + std::to_string(i) + "_" + std::to_string(j)));
		}
	}

	/*for (int i = 0; i < allnodes.size(); ++i)
	{
		if (allnodes[i]->node_present_type == NODE_PRESENT_TYPE::NONLABEL)
		{
			align_x1[i] = model.addVar(0, 1, 0, GRB_BINARY, "align_1_" + std::to_string(i));
			align_x2[i] = model.addVar(0, 1, 0, GRB_BINARY, "align_2_" + std::to_string(i));
		}
	}*/
	
}


void LayoutGeneratorNew::SetObject(GRBModel& model)
{
	
	object.clear();
	//text balance
	double W1 = 100, W2 = 100;
	for (int i = 0; i < allnodes.size(); ++i)
	{
		if (allnodes[i]->node_present_type == NODE_PRESENT_TYPE::TEXT)
		{
			int width_idx1 = i * 4 + 2;
			int height_idx1 = i * 4 + 3;
			for (int j = i + 1; j < allnodes.size(); ++j)
			{
				if (allnodes[j]->node_present_type == NODE_PRESENT_TYPE::TEXT)
				{
					int width_idx2 = j * 4 + 2;
					int height_idx2 = j * 4 + 3;
					object += W1 * (x[width_idx1] * x[width_idx1] - 2 * x[width_idx1] * x[width_idx2] + x[width_idx2] * x[width_idx2]);
					object += W2 * (x[height_idx1] * x[height_idx1] - 2 * x[height_idx1] * x[height_idx2] + x[height_idx2] * x[height_idx2]);
				}
			}
		}
	}
	double W3 = 50;
	for (int i = 0; i < allnodes.size(); ++i)
	{
		if (allnodes[i]->node_present_type == NODE_PRESENT_TYPE::PICTURE)
		{
			int width_idx1 = i * 4 + 2;
			int height_idx1 = i * 4 + 3;
			for (int j = i + 1; j < allnodes.size(); ++j)
			{
				if (allnodes[j]->node_present_type == NODE_PRESENT_TYPE::PICTURE)
				{
					int width_idx2 = j * 4 + 2;
					int height_idx2 = j * 4 + 3;
					object += W3 * (x[width_idx1] + x[height_idx1]
						- x[width_idx2] - x[height_idx2]) *
						(x[width_idx1] + x[height_idx1]
							- x[width_idx2] - x[height_idx2]);
				}
			}
		}
	}
	////img balance
	///
	double W4 = 30;
	for (int i = 0; i < allnodes.size(); ++i)
	{
		if (allnodes[i]->node_present_type != NODE_PRESENT_TYPE::NONLABEL
			&& allnodes[i]->node_present_type != NODE_PRESENT_TYPE::PADDING)
		{
			int width_idx1 = i * 4 + 2;
			int height_idx1 = i * 4 + 3;
			for (int j = i + 1; j < allnodes.size(); ++j)
			{
				if (allnodes[j]->node_present_type != NODE_PRESENT_TYPE::NONLABEL
					&& allnodes[j]->node_present_type != NODE_PRESENT_TYPE::PADDING)
				{
					int width_idx2 = j * 4 + 2;
					int height_idx2 = j * 4 + 3;
					object += W4 * (x[width_idx1] + x[height_idx1]
						- x[width_idx2] - x[height_idx2]) *
						(x[width_idx1] + x[height_idx1]
							- x[width_idx2] - x[height_idx2]);
				}
			}
		}
	}
	//double W4 = 2000;
	//for(auto item:align_x1)
	//{
	//	object += W4* (1-item.second);
	//}
	//for (auto item : align_x2)
	//{
	//	object += W4 * (1 - item.second);
	//}

	model.setObjective(object, GRB_MINIMIZE);
}


void LayoutGeneratorNew::RootConstrains(GRBModel& model)
{
	int root_order = ptr_idx_map[idx_ptr_map[combine_tree_->index]];
	int x_location_root = root_order * 4 + 1 - 1;
	int y_location_root = root_order * 4 + 2 - 1;
	int width_location_root = root_order * 4 + 3 - 1;
	int height_location_root = root_order * 4 + 4 - 1;

	linear_constrs.push_back(model.addConstr(x[x_location_root] == x[margin_index_left]));
	linear_constrs.push_back(model.addConstr(x[y_location_root] == x[margin_index_top]));
	linear_constrs.push_back(model.addConstr(x[width_location_root] == W - x[margin_index_right] - x[margin_index_left]));
	linear_constrs.push_back(model.addConstr(x[height_location_root] == H - x[margin_index_bottom] - x[margin_index_top]));
}

void LayoutGeneratorNew::AlignmentConstrains(GRBModel& model)
{
	//vertical_align
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
				/*q_constrs.push_back(model.addQConstr((x[x_location_first] - x[x_location_second]) * align_x1[i] == 0));
				q_constrs.push_back(model.addQConstr((x[x_location_first] + x[width_location_first] - x[x_location_second] - x[width_location_second]) * align_x2[i] == 0));
				linear_constrs.push_back(model.addConstr( align_x1[i] + align_x2[i] >= 1));*/
				linear_constrs.push_back(model.addConstr(x[x_location_first] - x[x_location_second] == 0));
				linear_constrs.push_back(model.addConstr(x[x_location_first] + x[width_location_first] - x[x_location_second] - x[width_location_second] == 0));
			}
		}
	}

	//horizontal_align
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
				/*q_constrs.push_back(model.addQConstr((x[y_location_first] - x[y_location_second]) * align_x1[i] == 0));
				q_constrs.push_back(model.addQConstr((x[y_location_first] + x[h_location_first] - x[y_location_second] - x[h_location_second]) * align_x2[i] == 0));
				linear_constrs.push_back(model.addConstr(align_x1[i] + align_x2[i] >= 1));*/
				linear_constrs.push_back(model.addConstr(x[y_location_first] - x[y_location_second] == 0));
				linear_constrs.push_back(model.addConstr(x[y_location_first] + x[h_location_first] - x[y_location_second] - x[h_location_second] == 0));
			}
		}
	}
}

void LayoutGeneratorNew::BoundingBoxConstrains(GRBModel& model)
{


	for (int i = 0; i < allnodes.size(); ++i)
	{
		int left_bound_idx = 0;
		int right_bound_idx = 0;
		int top_bound_idx = 0;
		int bottom_bound_idx = 0;

		if (!allnodes[i]->children.empty())
		{
			int children_size = allnodes[i]->children.size();
			left_bound_idx = ptr_idx_map[allnodes[i]->children[0]];
			top_bound_idx = ptr_idx_map[allnodes[i]->children[0]];
			right_bound_idx = ptr_idx_map[allnodes[i]->children[children_size - 1]];
			bottom_bound_idx = ptr_idx_map[allnodes[i]->children[children_size - 1]];

			linear_constrs.push_back(model.addConstr(x[4 * left_bound_idx] - x[4 * i] == 0));
			linear_constrs.push_back(model.addConstr(x[4 * right_bound_idx] + x[4 * right_bound_idx + 2] - x[4 * i] - x[4 * i + 2] == 0));
			linear_constrs.push_back(model.addConstr(x[4 * top_bound_idx + 1] - x[4 * i + 1] == 0));
			linear_constrs.push_back(model.addConstr(x[4 * bottom_bound_idx + 1] + x[4 * bottom_bound_idx + 3] - x[4 * i + 1] - x[4 * i + 3] == 0));

		}
	}
}

void LayoutGeneratorNew::PaddingConstrains(GRBModel& model)
{
	for (int i = 0; i < allnodes.size(); ++i)
	{
		//int final_interval = min_interval;

		if (allnodes[i]->node_relation == NODE_RELATION::HORIZONTAL)
		{
			std::vector<CCombineTreeNode*> child_nodes = allnodes[i]->children;
			//std::sort(child_nodes.begin(), child_nodes.end(), CombineNodeCompareX);
			for (int j = 1; j < child_nodes.size(); ++j)
			{
				int thisIdx = ptr_idx_map[child_nodes[j]];
				int leftNodeIdx = ptr_idx_map[child_nodes[j - 1]];

				linear_constrs.push_back(model.addConstr(x[4 * thisIdx] - x[4 * leftNodeIdx] - x[4 * leftNodeIdx + 2] - x[padding_index_h] == 0));
				//con.add(x[4 * thisIdx] - x[4 * leftNodeIdx] - x[4 * leftNodeIdx + 2] - x[interval_index] == 0);
			}
		}
		else if (allnodes[i]->node_relation == NODE_RELATION::VERTICAL)
		{
			std::vector<CCombineTreeNode*> child_nodes = allnodes[i]->children;
			//std::sort(child_nodes.begin(), child_nodes.end(), CombineNodeCompareY);
			for (int j = 1; j < child_nodes.size(); ++j)
			{
				int thisIdx = ptr_idx_map[child_nodes[j]];
				int upNodeIdx = ptr_idx_map[child_nodes[j - 1]];

				linear_constrs.push_back(model.addConstr(x[4 * thisIdx + 1] - x[4 * upNodeIdx + 1] - x[4 * upNodeIdx + 3] - x[padding_index_v] == 0));
				//con.add(x[4 * thisIdx + 1] - x[4 * upNodeIdx + 1] - x[4 * upNodeIdx + 3] - x[interval_index] == 0);
			}
		}
	}
}

void LayoutGeneratorNew::MarginConstrains(GRBModel& model)
{
	//linear_constrs.push_back(model.addConstr(x[margin_index_left] - x[margin_index_right] == 0));
	//linear_constrs.push_back(model.addConstr(x[margin_index_top] - x[margin_index_bottom] == 0));
}

void LayoutGeneratorNew::ImgCorresConstrains(GRBModel& model)
{
	//img corres constr
	for (int i = 0; i < img_corres_x.size(); ++i)
	{
		GRBLinExpr constr_expr;
		for (int j = 0; j < img_corres_x[i].size(); ++j)
		{
			constr_expr += img_corres_x[i][j];
		}
		linear_constrs.push_back(model.addConstr(constr_expr == 1));
	}

	for (int j = 0; j < img_corres_x[0].size(); ++j)
	{
		GRBLinExpr constr_expr;
		for (int i = 0; i < img_corres_x.size(); ++i)
		{
			constr_expr += img_corres_x[i][j];
		}
		linear_constrs.push_back(model.addConstr(constr_expr == 1));
	}
}

void LayoutGeneratorNew::ImgRatioConstrains(GRBModel& model)
{
	//img ratio constraints
	int img_idx = 0;
	std::stack<CCombineTreeNode*> node_stack;
	while (!node_stack.empty())	node_stack.pop();
	double slack_factor_less = 0.7;
	double slack_factor_more = 1.3;
	double inifine_num = 10000000.0;
	for (int i = 0; i < img_node_list.size(); ++i)
	{
		for (int j = 0; j < article_->img_list_.size(); ++j)
		{
			double ratio = static_cast<double>(article_->img_list_[j].width()) / article_->img_list_[j].height();
			int w_idx = 4 * ptr_idx_map[img_node_list[i]] + 2;
			int h_idx = 4 * ptr_idx_map[img_node_list[i]] + 3;
			linear_constrs.push_back(model.addConstr(x[w_idx] - ratio * slack_factor_more* x[h_idx] + inifine_num * (img_corres_x[i][j] - 1) <= 0));
			linear_constrs.push_back(model.addConstr( - x[w_idx] + ratio * slack_factor_less * x[h_idx] + inifine_num * (img_corres_x[i][j] - 1) <= 0));
		}
	}
}

void LayoutGeneratorNew::TextAreaConstrains(GRBModel& model)
{
	int letter_count = 0;
	QFont font;
	//const int font_size = 4;//12;
	const int row_space = 1;//3;
	//font.setPixelSize(font_size);
	font.setFamily("Courier New");
	int body_text_num = 0;
	for (size_t i = 0; i < article_->text_list_.size(); ++i)
	{
		for (size_t j = 0; j < article_->text_list_[i].body_text.size(); ++j)
		{
			letter_count += article_->text_list_[i].body_text[j].size();
			body_text_num ++;
		}
	}

	text_area_max_list.clear();
	text_area_min_list.clear();
	for (int i = 0; i < font_size_list.size(); ++i)
	{
		font.setPointSize(font_size_list[i]);
		QFontMetrics font_metrics(font);
		text_area_max_list.push_back((letter_count + 300 * body_text_num) * font_metrics.averageCharWidth() * (font_metrics.ascent() + font_metrics.descent() + row_space));
		text_area_min_list.push_back((letter_count + 30 * body_text_num) * font_metrics.averageCharWidth() * (font_metrics.ascent() + font_metrics.descent() + row_space));
	}

	GRBQuadExpr text_up_bound_con_exp;
	GRBQuadExpr text_low_bound_con_exp;

	for (size_t i = 0; i < allnodes.size(); ++i)
	{
		auto node = allnodes[i];
		if (node->node_present_type == NODE_PRESENT_TYPE::TEXT)
		{
			int idx = ptr_idx_map[node];
			int w = 4 * idx + 2;
			int h = 4 * idx + 3;
			text_up_bound_con_exp +=  x[w] * x[h];
			text_low_bound_con_exp += - x[w] * x[h];

			//text_up_bound_con_exp += x[w] * x[h];
			//text_low_bound_con_exp += -x[w] * x[h];
		}
	}
	text_up_bound_con_exp += -text_area_max_list.back();
	text_low_bound_con_exp += text_area_min_list[0];

	//con.add(text_up_bound_con_exp <= 0);
	//con.add(text_low_bound_con_exp <= 0);


	//text_up_bound_con_exp.end();
	//text_low_bound_con_exp.end();

	q_constrs.push_back(model.addQConstr(text_up_bound_con_exp <= 0));
	q_constrs.push_back(model.addQConstr(text_low_bound_con_exp <= 0));
}

void LayoutGeneratorNew::TitleAreaConstrains(GRBModel& model)
{
	
	QFont font;
	//const int font_size = 4;//12;
	const int row_space = 2;//3;
	//font.setPixelSize(font_size);
	font.setFamily("Courier New");
	int letter_count = article_->title_.headline.size();
	//int body_text_num = 0;
	std::vector<int> title_font_size_list = { 20,22,24,26,28,30 };
	std::vector<double> title_area_min_list = {};
	std::vector<double> title_area_max_list = {};
	title_area_max_list.clear();
	title_area_min_list.clear();
	for (int i = 0; i < title_font_size_list.size(); ++i)
	{
		font.setPointSize(title_font_size_list[i]);
		QFontMetrics font_metrics(font);
		double area = letter_count * font_metrics.averageCharWidth() * (font_metrics.ascent() + font_metrics.descent() + row_space);
		title_area_max_list.push_back(area * 4);
		title_area_min_list.push_back(area * 2);
	}

	GRBQuadExpr title_up_bound_con_exp;
	GRBQuadExpr title_low_bound_con_exp;

	for (size_t i = 0; i < allnodes.size(); ++i)
	{
		auto node = allnodes[i];
		if (node->node_present_type == NODE_PRESENT_TYPE::TITLE)
		{
			int idx = ptr_idx_map[node];
			int w = 4 * idx + 2;
			int h = 4 * idx + 3;
			title_up_bound_con_exp += x[w] * x[h];
			title_low_bound_con_exp += -x[w] * x[h];

			//text_up_bound_con_exp += x[w] * x[h];
			//text_low_bound_con_exp += -x[w] * x[h];
		}
	}
	title_up_bound_con_exp += -title_area_max_list.back();
	title_low_bound_con_exp += title_area_min_list[0];

	//con.add(text_up_bound_con_exp <= 0);
	//con.add(text_low_bound_con_exp <= 0);


	//text_up_bound_con_exp.end();
	//text_low_bound_con_exp.end();

	q_constrs.push_back(model.addQConstr(title_up_bound_con_exp <= 0));
	q_constrs.push_back(model.addQConstr(title_low_bound_con_exp <= 0));
}


void LayoutGeneratorNew::GetResult(GRBModel& model)
{

	for (int i = 0; i < allnodes.size(); ++i)
	{
		double result_x = x[i * 4].get(GRB_DoubleAttr_X);
		double result_y = x[i * 4 + 1].get(GRB_DoubleAttr_X);
		double result_w = x[i * 4 + 2].get(GRB_DoubleAttr_X);
		double result_h = x[i * 4 + 3].get(GRB_DoubleAttr_X);
		allnodes[i]->x = result_x;
		allnodes[i]->y = result_y;
		allnodes[i]->width = result_w;
		allnodes[i]->height = result_h;

		for (int i = 0; i < img_corres_x.size(); ++i)
		{
			for (int j = 0; j < img_corres_x[i].size(); ++j)
			{
				int corres = img_corres_x[i][j].get(GRB_DoubleAttr_X);
				if (corres == 1)
				{
					allnodes[ptr_idx_map[img_node_list[i]]]->corres_img_idx = j;
					break;
				}
			}
		}
	}

}

bool LayoutGeneratorNew::GenerateLayout(GRBModel model)
{
	/*GRBEnv env = GRBEnv(true);
	env.start();
	GRBModel model = GRBModel(env);
	model.set("NonConvex", "2");
	model.set("TimeLimit", "0.1");
	model.set("OutputFlag", "0");*/


	TreePreProcess();
	//double root_x = 0;
	//double root_y = 0;
	//double root_width = tree->tree0_node->width * alpha_ + tree->tree1_node->width * beta_ + tree->tree2_node->width * gamma_;
	//double root_height = tree->tree0_node->height * alpha_ + tree->tree1_node->height * beta_ + tree->tree2_node->height * gamma_;
	//double root_width = 630;
	//double root_height = 891;
	//supposed that BFS put the nodes in.
	bool success = false;
	InitVariables(model);
	SetObject(model);
	RootConstrains(model);
	AlignmentConstrains(model);
	BoundingBoxConstrains(model);
	PaddingConstrains(model);
	MarginConstrains(model);
	TitleAreaConstrains(model);
	ImgCorresConstrains(model);
	ImgRatioConstrains(model);
	TextAreaConstrains(model);
	
	try
	{
		//::cout << "start" << std::endl;
		clock_t start = clock();
		model.optimize();
		//std::cout << model.get(GRB_IntAttr_Status) << std::endl;
		if (model.get(GRB_IntAttr_Status) == GRB_OPTIMAL || model.get(GRB_IntAttr_Status) == GRB_SUBOPTIMAL)
		{
			GetResult(model);
			success = true;
			//valid_solutions_.push_back(combine_tree_);
			clock_t end = clock();
			double used_time = static_cast<double>(end - start) / static_cast<double>(CLOCKS_PER_SEC);
			std::cout << "solved time: " << used_time << std::endl;
		}
		//else std::cout << "oops!" << std::endl;
	}
	catch (GRBException e)
	{
		std::cout << "Error code = " << e.getErrorCode() << std::endl;
		std::cout << e.getMessage() << std::endl;
	}
	catch (...)
	{
		std::cout << "Exception during optimization" << std::endl;
	}
	for (int i = 0; i < x.size(); ++i)
	{
		model.remove(x[i]);
	}
	for (int i = 0; i < linear_constrs.size(); ++i)
	{
		model.remove(linear_constrs[i]);
	}
	for (int i = 0; i < q_constrs.size(); ++i)
	{
		model.remove(q_constrs[i]);
	}
	for (int i = 0; i < img_corres_x.size(); ++i)
	{
		for (int j = 0; j < img_corres_x[i].size(); ++j)
		{
			model.remove(img_corres_x[i][j]);
		}
	}
	return success;
}
