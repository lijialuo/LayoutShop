#include "LayoutGenerator.h"


#include <QGuiApplication>
#include <iostream>
#include <ctime>
#include <qfontmetrics.h>
#include <stack>


void LayoutGenerator::SetArticle(Article* article)
{
	article_ = article;
}

void LayoutGenerator::SetCombineTree(CCombineTreeNode* tree)
{
	combine_tree_ = tree;
}

void LayoutGenerator::TreePreProcess()
{
	std::queue<CCombineTreeNode*> combine_q;
	allnodes.clear();
	ptr_idx_map.clear();
	idx_ptr_map.clear();
	img_node_list.clear();

	combine_q.push(combine_tree_);
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
	}

	for (int i = 0; i < allnodes.size(); ++i)
	{
		if (allnodes[i]->node_present_type == NODE_PRESENT_TYPE::PICTURE)
		{
			img_node_list.push_back(allnodes[i]);
		}
	}
}

void LayoutGenerator::InitVariables(GRBModel& model)
{
	x.clear();
	img_corres_x.clear();
	linear_constrs.clear();
	q_constrs.clear();
	for (int i = 0; i < allnodes.size(); ++i)
	{
		x.push_back(model.addVar(1, grid_num_h, 0.0, GRB_INTEGER, "x" + (i * 4)));
		x.push_back(model.addVar(1, grid_num_v, 0.0, GRB_INTEGER, "x" + (i * 4 + 1)));
		x.push_back(model.addVar(1, grid_num_h, 0.0, GRB_INTEGER, "x" + (i * 4 + 2)));
		x.push_back(model.addVar(1, grid_num_v, 0.0, GRB_INTEGER, "x" + (i * 4 + 3)));
	}
	//padding
	padding_index_h = allnodes.size() * 4; //横边距变量的下标
	padding_index_v = allnodes.size() * 4 + 1; //竖边距变量的下标
	x.push_back(model.addVar(5.0, 30.0, 0.0, GRB_CONTINUOUS, "x" + padding_index_h)); //h padding
	x.push_back(model.addVar(5.0, 30.0, 0.0, GRB_CONTINUOUS, "x" + padding_index_v)); //v padding
	
	margin_index_left = allnodes.size() * 4 + 2;
	margin_index_right = margin_index_left + 1;
	margin_index_top = margin_index_right + 1;
	margin_index_bottom = margin_index_top + 1;
	//margin
	x.push_back(model.addVar(30.0, 50.0, 0.0, GRB_CONTINUOUS, "x" + margin_index_left)); //left margin
	x.push_back(model.addVar(30.0, 50.0, 0.0, GRB_CONTINUOUS, "x" + margin_index_right)); //right margin
	x.push_back(model.addVar(30.0, 50.0, 0.0, GRB_CONTINUOUS, "x" + margin_index_top)); //top margin
	x.push_back(model.addVar(30.0, 50.0, 0.0, GRB_CONTINUOUS, "x" + margin_index_bottom)); //bottom margin
	

	for (int i = 0; i < article_->img_list_.size(); ++i)
	{
		img_corres_x.push_back(std::vector<GRBVar>());
		for (int j = 0; j < img_node_list.size(); ++j)
		{
			img_corres_x[i].push_back(model.addVar(0, 1, 0, GRB_BINARY, "img_corres_" + std::to_string(i) + "_" + std::to_string(j)));
		}
	}
}


void LayoutGenerator::SetObject(GRBModel& model)
{
	object.clear();
	for (int i = 0; i < allnodes.size(); ++i)
	{
		if (allnodes[i]->node_present_type == NODE_PRESENT_TYPE::TEXT)
		{
			int width_idx1 = i * 4 + 2;
			for (int j = i + 1; j < allnodes.size(); ++j)
			{
				if (allnodes[j]->node_present_type == NODE_PRESENT_TYPE::TEXT)
				{
					int width_idx2 = j * 4 + 2;
					object += (x[width_idx1] * x[width_idx1] - 2 * x[width_idx1] * x[width_idx2] + x[width_idx2] * x[width_idx2]) * grid_width * grid_width;
				}
			}
		}
	}
	model.setObjective(object, GRB_MINIMIZE);
}

//void LayoutGenerator::Generate2()
//{
//	GRBEnv env = GRBEnv(true);
//	env.start();
//	GRBModel model = GRBModel(env);
//	model.set("NonConvex", "2");
//	model.set("TimeLimit", "0.1");
//	model.set("OutputFlag", "0");
//	for (size_t index = 0; index < tree_candidates_.size(); ++index)
//	{
//		auto tree = tree_candidates_[index];
//
//		double root_x = 10;
//		double root_y = 10;
//		//double root_width = tree->tree0_node->width * alpha_ + tree->tree1_node->width * beta_ + tree->tree2_node->width * gamma_;
//		//double root_height = tree->tree0_node->height * alpha_ + tree->tree1_node->height * beta_ + tree->tree2_node->height * gamma_;
//		double root_width = 630;
//		double root_height = 891;
//		//supposed that BFS put the nodes in.
//		std::vector<CCombineTreeNode*> allnodes;
//		std::queue<CCombineTreeNode*> combine_q;
//		std::unordered_map<CCombineTreeNode*, int> ptr_idx_map;
//		std::unordered_map<int, CCombineTreeNode*> idx_ptr_map;
//		std::vector<CCombineTreeNode*> img_node_list;
//
//		combine_q.push(tree);
//		while (!combine_q.empty())
//		{
//			//注意：我的combine tree index从1开始
//			idx_ptr_map[combine_q.front()->index] = combine_q.front();
//			ptr_idx_map[combine_q.front()] = allnodes.size();	//根据ptr映射出在allnodes中的order;
//
//			allnodes.push_back(combine_q.front());
//			combine_q.pop();
//			////std::cout << allnodes.back()->index << std::endl;
//			for (auto ptr : allnodes.back()->children)
//			{
//				combine_q.push(ptr);
//			}
//		}
//
//		for(int i = 0; i < allnodes.size();++i)
//		{
//			if(allnodes[i]->node_present_type == NODE_PRESENT_TYPE::PICTURE)
//			{
//				img_node_list.push_back(allnodes[i]);
//			}
//		}
//
//		for(int template_idx = 0; template_idx < grid_template_list.size();++template_idx)
//		{
//			int grid_num_h = grid_template_list[template_idx].first;
//			int grid_num_v = grid_template_list[template_idx].second;
//			double grid_width = W / grid_num_h;
//			double grid_height = H / grid_num_v;
//
//			std::vector<GRBVar> x;
//			std::vector<std::vector<GRBVar>> img_corres_x;
//			std::vector<GRBConstr> linear_constrs;
//			std::vector<GRBQConstr> q_constrs;
//			for (int i = 0; i < allnodes.size() ; ++i)
//			{
//				x.push_back(model.addVar(1, grid_num_h, 0.0, GRB_INTEGER, "x" + (i * 4)));
//				x.push_back(model.addVar(1, grid_num_v, 0.0, GRB_INTEGER, "x" + (i * 4 + 1)));
//				x.push_back(model.addVar(1, grid_num_h, 0.0, GRB_INTEGER, "x" + (i * 4 + 2)));
//				x.push_back(model.addVar(1, grid_num_v, 0.0, GRB_INTEGER, "x" + (i * 4 + 3)));
//			} 
//			x.push_back(model.addVar(5.0, 20.0, 0.0, GRB_CONTINUOUS, "x" + allnodes.size() * 4));
//			x.push_back(model.addVar(5.0, 20.0, 0.0, GRB_CONTINUOUS, "x" + (allnodes.size() * 4 + 1)));
//
//			for(int i = 0; i < article_.img_list.size();++i)
//			{
//				img_corres_x.push_back(std::vector<GRBVar>());
//				for(int j = 0; j < img_node_list.size();++j)
//				{
//					img_corres_x[i].push_back(model.addVar(0, 1, 0, GRB_BINARY, "img_corres_" + std::to_string(i) + "_" + std::to_string(j)));
//				}
//			}
//
//			GRBQuadExpr object;
//
//			double title_ratio = 1.0 / 4;
//			double weight_title = 10;
//			for (int i = 0; i < allnodes.size(); ++i)
//			{
//				if (allnodes[i]->node_present_type == NODE_PRESENT_TYPE::TITLE)
//				{
//					int width_idx = i * 4 + 2;
//					int height_idx = i * 4 + 3;
//					double width_pre = title_ratio * root_width;
//					double height_pre = title_ratio * root_height;
//					object += weight_title * (x[width_idx] * x[width_idx] * grid_width * grid_width - width_pre * 2 * x[width_idx] * grid_width + width_pre * width_pre);
//					object += weight_title * (x[height_idx] * x[height_idx] * grid_height * grid_height - height_pre * 2 * x[height_idx] * grid_height + height_pre * height_pre);
//				}
//				else if (allnodes[i]->node_present_type == NODE_PRESENT_TYPE::TEXT)
//				{
//					int width_idx1 = i * 4 + 2;
//					for (int j = i + 1; j < allnodes.size(); ++j)
//					{
//						if (allnodes[j]->node_present_type == NODE_PRESENT_TYPE::TEXT)
//						{
//							int width_idx2 = j * 4 + 2;
//							object += (x[width_idx1] * x[width_idx1] - 2 * x[width_idx1] * x[width_idx2] + x[width_idx2] * x[width_idx2]) * grid_width * grid_width;
//						}
//					}
//				}
//			}
//
//			model.setObjective(object, GRB_MINIMIZE);
//			//model.add(IloMinimize(env, optimize_exp));
//			//optimize_exp.end();
//
//
//			//添加直接插值的硬约束
//			int root_order = ptr_idx_map[idx_ptr_map[tree->index]];
//			int x_location_root = root_order * 4 + 1 - 1;
//			int y_location_root = root_order * 4 + 2 - 1;
//			int width_location_root = root_order * 4 + 3 - 1;
//			int height_location_root = root_order * 4 + 4 - 1;
//
//			linear_constrs.push_back(model.addConstr(x[x_location_root] == 1));
//			linear_constrs.push_back(model.addConstr(x[y_location_root] == 1));
//			linear_constrs.push_back(model.addConstr(x[width_location_root] == grid_num_h));
//			linear_constrs.push_back(model.addConstr(x[height_location_root] == grid_num_v));
//
//
//			//vertical_align
//			for (int i = 0; i < allnodes.size(); i++)
//			{
//				if (allnodes[i]->node_relation == NODE_RELATION::VERTICAL)
//				{
//					for (int j = 0; j < allnodes[i]->children.size() - 1; j++)
//					{
//						int order_first = ptr_idx_map[idx_ptr_map[allnodes[i]->children[j]->index]];
//						int order_second = ptr_idx_map[idx_ptr_map[allnodes[i]->children[j + 1]->index]];
//						int x_location_first = order_first * 4 + 1 - 1;
//						int x_location_second = order_second * 4 + 1 - 1;
//						int width_location_first = order_first * 4 + 3 - 1;
//						int width_location_second = order_second * 4 + 3 - 1;
//
//						linear_constrs.push_back(model.addConstr(x[x_location_first] - x[x_location_second] == 0));
//						linear_constrs.push_back(model.addConstr(x[x_location_first] + x[width_location_first] - x[x_location_second] - x[width_location_second] == 0));
//					}
//				}
//			}
//
//			////left_align
//			//for (int i = 0; i < allnodes.size(); i++)
//			//{
//			//	if (allnodes[i]->left_align_node.size() > 0)
//			//	{
//			//		for (int j = 0; j < allnodes[i]->left_align_node.size() - 1; j++)
//			//		{
//			//			int order_first = ptr_idx_map[idx_ptr_map[allnodes[i]->left_align_node[j]]];
//			//			int order_second = ptr_idx_map[idx_ptr_map[allnodes[i]->left_align_node[j + 1]]];
//			//			int x_location_first = order_first * 4 + 1 - 1;
//			//			int x_location_second = order_second * 4 + 1 - 1;
//
//			//			linear_constrs.push_back(model.addConstr(x[x_location_first] - x[x_location_second] == 0));
//			//			//con.add(x[x_location_first] - x[x_location_second] == 0);
//			//		}
//			//	}
//			//}
//
//			////right_align
//			//for (int i = 0; i < allnodes.size(); i++)
//			//{
//			//	if (allnodes[i]->right_align_node.size() > 0)
//			//	{
//			//		for (int j = 0; j < allnodes[i]->right_align_node.size() - 1; j++)
//			//		{
//			//			int order_first = ptr_idx_map[idx_ptr_map[allnodes[i]->right_align_node[j]]];
//			//			int order_second = ptr_idx_map[idx_ptr_map[allnodes[i]->right_align_node[j + 1]]];
//
//			//			int x_location_first = order_first * 4 + 1 - 1;
//			//			int x_location_second = order_second * 4 + 1 - 1;
//			//			int width_location_first = order_first * 4 + 3 - 1;
//			//			int width_location_second = order_second * 4 + 3 - 1;
//			//			linear_constrs.push_back(model.addConstr(x[x_location_first] + x[width_location_first] - x[x_location_second] - x[width_location_second] == 0));
//			//		}
//			//	}
//			//}
//
//			//horizontal_align
//			for (int i = 0; i < allnodes.size(); i++)
//			{
//				if (allnodes[i]->node_relation == NODE_RELATION::HORIZONTAL)
//				{
//					for (int j = 0; j < allnodes[i]->children.size() - 1; j++)
//					{
//						int order_first = ptr_idx_map[idx_ptr_map[allnodes[i]->children[j]->index]];
//						int order_second = ptr_idx_map[idx_ptr_map[allnodes[i]->children[j + 1]->index]];
//						int y_location_first = order_first * 4 + 2 - 1;
//						int y_location_second = order_second * 4 + 2 - 1;
//						int h_location_first = order_first * 4 + 4 - 1;
//						int h_location_second = order_second * 4 + 4 - 1;
//						linear_constrs.push_back(model.addConstr(x[y_location_first] - x[y_location_second] == 0));
//						//con.add(x[y_location_first] - x[y_location_second] == 0);
//						linear_constrs.push_back(model.addConstr(x[y_location_first] + x[h_location_first] - x[y_location_second] - x[h_location_second] == 0));
//					}
//				}
//			}
//
//			////top align
//			//for (int i = 0; i < allnodes.size(); i++)
//			//{
//			//	if (allnodes[i]->top_align_node.size() > 0)
//			//	{
//			//		for (int j = 0; j < allnodes[i]->top_align_node.size() - 1; j++)
//			//		{
//			//			int order_first = ptr_idx_map[idx_ptr_map[allnodes[i]->top_align_node[j]]];
//			//			int order_second = ptr_idx_map[idx_ptr_map[allnodes[i]->top_align_node[j + 1]]];
//			//			int y_location_first = order_first * 4 + 2 - 1;
//			//			int y_location_second = order_second * 4 + 2 - 1;
//			//			linear_constrs.push_back(model.addConstr(x[y_location_first] - x[y_location_second] == 0));
//			//			//con.add(x[y_location_first] - x[y_location_second] == 0);
//
//			//		}
//			//	}
//			//}
//
//			////bottom align
//			//for (int i = 0; i < allnodes.size(); i++)
//			//{
//			//	if (allnodes[i]->bottom_align_node.size() > 0)
//			//	{
//			//		for (int j = 0; j < allnodes[i]->bottom_align_node.size() - 1; j++)
//			//		{
//			//			int order_first = ptr_idx_map[idx_ptr_map[allnodes[i]->bottom_align_node[j]]];
//			//			int order_second = ptr_idx_map[idx_ptr_map[allnodes[i]->bottom_align_node[j + 1]]];
//
//			//			int y_location_first = order_first * 4 + 2 - 1;
//			//			int y_location_second = order_second * 4 + 2 - 1;
//			//			int h_location_first = order_first * 4 + 4 - 1;
//			//			int h_location_second = order_second * 4 + 4 - 1;
//			//			linear_constrs.push_back(model.addConstr(x[y_location_first] + x[h_location_first] - x[y_location_second] - x[h_location_second] == 0));
//			//		}
//			//	}
//			//}
//
//
//			int padding_index_h = allnodes.size() * 4; //横边距变量的下标
//			int padding_index_v = allnodes.size() * 4 + 1; //竖边距变量的下标
//
//			for (int i = 0; i < allnodes.size(); ++i)
//			{
//				int left_bound_idx = 0;
//				int right_bound_idx = 0;
//				int top_bound_idx = 0;
//				int bottom_bound_idx = 0;
//
//				if (!allnodes[i]->children.empty())
//				{
//					int children_size = allnodes[i]->children.size();
//					left_bound_idx = ptr_idx_map[allnodes[i]->children[0]];
//					top_bound_idx = ptr_idx_map[allnodes[i]->children[0]];
//					right_bound_idx = ptr_idx_map[allnodes[i]->children[children_size - 1]];
//					bottom_bound_idx = ptr_idx_map[allnodes[i]->children[children_size - 1]];
//
//					linear_constrs.push_back(model.addConstr(x[4 * left_bound_idx] - x[4 * i] == 0));
//					linear_constrs.push_back(model.addConstr(x[4 * right_bound_idx] + x[4 * right_bound_idx + 2]  - x[4 * i] - x[4 * i + 2] == 0));
//					linear_constrs.push_back(model.addConstr(x[4 * top_bound_idx + 1] - x[4 * i + 1] == 0));
//					linear_constrs.push_back(model.addConstr(x[4 * bottom_bound_idx + 1] + x[4 * bottom_bound_idx + 3] - x[4 * i + 1] - x[4 * i + 3] == 0));
//
//				}
//			}
//
//			//add interval constraints
//			
//			for (int i = 0; i < allnodes.size(); ++i)
//			{
//				//int final_interval = min_interval;
//
//				if (allnodes[i]->node_relation == NODE_RELATION::HORIZONTAL)
//				{
//					std::vector<CCombineTreeNode*> child_nodes = allnodes[i]->children;
//					//std::sort(child_nodes.begin(), child_nodes.end(), CombineNodeCompareX);
//					for (int j = 1; j < child_nodes.size(); ++j)
//					{
//						int thisIdx = ptr_idx_map[child_nodes[j]];
//						int leftNodeIdx = ptr_idx_map[child_nodes[j - 1]];
//
//						linear_constrs.push_back(model.addConstr(x[4 * thisIdx] - x[4 * leftNodeIdx] - x[4 * leftNodeIdx + 2]  == 0));
//						//con.add(x[4 * thisIdx] - x[4 * leftNodeIdx] - x[4 * leftNodeIdx + 2] - x[interval_index] == 0);
//					}
//				}
//				else if (allnodes[i]->node_relation == NODE_RELATION::VERTICAL)
//				{
//					std::vector<CCombineTreeNode*> child_nodes = allnodes[i]->children;
//					//std::sort(child_nodes.begin(), child_nodes.end(), CombineNodeCompareY);
//					for (int j = 1; j < child_nodes.size(); ++j)
//					{
//						int thisIdx = ptr_idx_map[child_nodes[j]];
//						int upNodeIdx = ptr_idx_map[child_nodes[j - 1]];
//
//						linear_constrs.push_back(model.addConstr(x[4 * thisIdx + 1] - x[4 * upNodeIdx + 1] - x[4 * upNodeIdx + 3]  == 0));
//						//con.add(x[4 * thisIdx + 1] - x[4 * upNodeIdx + 1] - x[4 * upNodeIdx + 3] - x[interval_index] == 0);
//					}
//				}
//			}
//
//
//			//img corres constr
//			for(int i = 0; i < img_corres_x.size();++i)
//			{
//				GRBLinExpr constr_expr;
//				for(int j = 0; j < img_corres_x[i].size(); ++j)
//				{
//					constr_expr += img_corres_x[i][j];
//				}
//				linear_constrs.push_back(model.addConstr(constr_expr == 1));
//			}
//
//			for (int j = 0; j < img_corres_x[0].size(); ++j)
//			{
//				GRBLinExpr constr_expr;
//				for (int i = 0; i < img_corres_x.size(); ++i)
//				{
//					constr_expr += img_corres_x[i][j];
//				}
//				linear_constrs.push_back(model.addConstr(constr_expr == 1));
//			}
//
//			//img ratio constraints
//			int img_idx = 0;
//			std::stack<CCombineTreeNode*> node_stack;
//			while (!node_stack.empty())	node_stack.pop();
//			double slack_factor_less = 0.8; //图片宽高比松弛因子
//			double slack_factor_more = 1.2;
//			double inifine_num = 10000000.0;
//			for(int i=0;i<img_node_list.size();++i)
//			{
//				for(int j = 0; j < article_.img_list.size();++j)
//				{
//					double ratio = article_.img_list[j].ratio;
//					linear_constrs.push_back(model.addConstr(grid_width* x[4 * ptr_idx_map[img_node_list[i]] + 2] - x[padding_index_h] - (ratio * slack_factor_more) * (grid_height * x[4 * ptr_idx_map[img_node_list[i]] + 3] - x[padding_index_v]) + inifine_num * (img_corres_x[i][j] - 1) <= 0));
//					linear_constrs.push_back(model.addConstr( - grid_width* x[4 * ptr_idx_map[img_node_list[i]] + 2] - x[padding_index_h] + (ratio * slack_factor_less) * (grid_height * x[4 * ptr_idx_map[img_node_list[i]] + 3] - x[padding_index_v]) + inifine_num * (img_corres_x[i][j] - 1) <= 0));
//				}
//			}
//
//
//			//double text_area_max = 0.0, text_area_min = 0.0;
//			int letter_count = 0;
//			QFont font;
//			//const int font_size = 4;//12;
//			const int row_space = 1;//3;
//			//font.setPixelSize(font_size);
//			font.setFamily("Courier New");
//
//			for (size_t i = 0; i < article_.text_list.size(); ++i)
//			{
//				letter_count += article_.text_list[i].size();
//			}
//			//text_area_max = (letter_count + 300 * article_.text_list.size()) * font_metrics.averageCharWidth() * (font_metrics.ascent() + font_metrics.descent() + row_space);
//			//text_area_min = (letter_count + 30 * article_.text_list.size()) * font_metrics.averageCharWidth() * (font_metrics.ascent() + font_metrics.descent() + row_space);
//			text_area_max_list.clear();
//			text_area_min_list.clear();
//			for (int i = 0; i < font_size_list.size(); ++i)
//			{
//				font.setPointSize(font_size_list[i]);
//				QFontMetrics font_metrics(font);
//				text_area_max_list.push_back((letter_count + 300 * article_.text_list.size()) * font_metrics.averageCharWidth() * (font_metrics.ascent() + font_metrics.descent() + row_space));
//				text_area_min_list.push_back((letter_count + 30 * article_.text_list.size()) * font_metrics.averageCharWidth() * (font_metrics.ascent() + font_metrics.descent() + row_space));
//			}
//
//
//			//IloExpr text_up_bound_con_exp(env);
//			//IloExpr text_low_bound_con_exp(env);
//
//			GRBQuadExpr text_up_bound_con_exp;
//			GRBQuadExpr text_low_bound_con_exp;
//
//			for (size_t i = 0; i < allnodes.size(); ++i)
//			{
//				auto node = allnodes[i];
//				if (node->node_present_type == NODE_PRESENT_TYPE::TEXT)
//				{
//					int idx = ptr_idx_map[node];
//					int w = 4 * idx + 2;
//					int h = 4 * idx + 3;
//					text_up_bound_con_exp += grid_width * grid_height * x[w] * x[h];
//					text_low_bound_con_exp += -grid_width * grid_height * x[w] * x[h];
//
//					//text_up_bound_con_exp += x[w] * x[h];
//					//text_low_bound_con_exp += -x[w] * x[h];
//					//文字面积小于约束
//					//文字面积大于约束		
//				}
//			}
//			text_up_bound_con_exp += -text_area_max_list.back();
//			text_low_bound_con_exp += text_area_min_list[0];
//
//			//con.add(text_up_bound_con_exp <= 0);
//			//con.add(text_low_bound_con_exp <= 0);
//
//
//			//text_up_bound_con_exp.end();
//			//text_low_bound_con_exp.end();
//
//			q_constrs.push_back(model.addQConstr(text_up_bound_con_exp <= 0));
//			q_constrs.push_back(model.addQConstr(text_low_bound_con_exp <= 0));
//
//			try
//			{
//				clock_t start = clock();
//				model.optimize();
//				//std::cout << model.get(GRB_IntAttr_Status) << std::endl;
//				if (model.get(GRB_IntAttr_Status) == GRB_OPTIMAL || model.get(GRB_IntAttr_Status) == GRB_SUBOPTIMAL)
//				{
//					double interval_h = x[padding_index_h].get(GRB_DoubleAttr_X);
//					double interval_v = x[padding_index_v].get(GRB_DoubleAttr_X);
//
//					for (int i = 0; i < allnodes.size(); ++i)
//					{
//						int start_grid_x = x[i * 4].get(GRB_DoubleAttr_X);
//						int start_grid_y = x[i * 4 + 1].get(GRB_DoubleAttr_X);
//						int cross_grid_num_h = x[i * 4 + 2].get(GRB_DoubleAttr_X);
//						int cross_grid_num_v = x[i * 4 + 3].get(GRB_DoubleAttr_X);
//
//						if (start_grid_x == 1) allnodes[i]->x = 0.0;
//						else allnodes[i]->x = (start_grid_x - 1) * grid_width + 0.5 * interval_h;
//						if (start_grid_y == 1) allnodes[i]->y = 0.0;
//						else allnodes[i]->y = (start_grid_y - 1) * grid_height + 0.5 * interval_v;
//
//						allnodes[i]->width = cross_grid_num_h * grid_width - interval_h;
//						allnodes[i]->height = cross_grid_num_v * grid_height - interval_v;
//
//						if (start_grid_x == 1) allnodes[i]->width += 0.5 * interval_h;
//						if (start_grid_x + cross_grid_num_h - 1 == grid_num_h) allnodes[i]->width += 0.5 * interval_h;
//
//						if (start_grid_y == 1) allnodes[i]->height += 0.5 * interval_v;
//						if (start_grid_y + cross_grid_num_v - 1 == grid_num_v) allnodes[i]->height += 0.5 * interval_v;
//
//						for (int i = 0; i < img_corres_x.size(); ++i)
//						{
//							for (int j = 0; j < img_corres_x[i].size(); ++j)
//							{
//								int corres = img_corres_x[i][j].get(GRB_DoubleAttr_X);
//								if (corres == 1)
//								{
//									allnodes[ptr_idx_map[img_node_list[i]]]->corres_img_idx = j;
//									break;
//								}
//							}
//						}
//					}
//					valid_solutions_.push_back(tree_candidates_[index]);
//					clock_t end = clock();
//					double used_time = static_cast<double>(end - start) / static_cast<double>(CLOCKS_PER_SEC); //计算时间
//					std::cout << "solved time: " << used_time << std::endl;
//					template_idx = grid_template_list.size();
//					tree->grid_template = { grid_num_h,grid_num_v };
//				}
//				//else std::cout << "oops!" << std::endl;
//			}
//			catch (GRBException e)
//			{
//				std::cout << "Error code = " << e.getErrorCode() << std::endl;
//				std::cout << e.getMessage() << std::endl;
//			}
//			catch (...)
//			{
//				std::cout << "Exception during optimization" << std::endl;
//			}
//			for (int i = 0; i < x.size(); ++i)
//			{
//				model.remove(x[i]);
//			}
//			for (int i = 0; i < linear_constrs.size(); ++i)
//			{
//				model.remove(linear_constrs[i]);
//			}
//			for (int i = 0; i < q_constrs.size(); ++i)
//			{
//				model.remove(q_constrs[i]);
//			}
//			for(int i= 0; i < img_corres_x.size();++i)
//			{
//				for(int j=0; j < img_corres_x[i].size(); ++j)
//				{
//					model.remove(img_corres_x[i][j]);
//				}
//			}
//		}
//	}
//}
void LayoutGenerator::RootConstrains(GRBModel& model)
{
	int root_order = ptr_idx_map[idx_ptr_map[combine_tree_->index]];
	int x_location_root = root_order * 4 + 1 - 1;
	int y_location_root = root_order * 4 + 2 - 1;
	int width_location_root = root_order * 4 + 3 - 1;
	int height_location_root = root_order * 4 + 4 - 1;

	linear_constrs.push_back(model.addConstr(x[x_location_root] == 1));
	linear_constrs.push_back(model.addConstr(x[y_location_root] == 1));
	linear_constrs.push_back(model.addConstr(x[width_location_root] == grid_num_h));
	linear_constrs.push_back(model.addConstr(x[height_location_root] == grid_num_v));
}

void LayoutGenerator::AlignmentConstrains(GRBModel& model)
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
				linear_constrs.push_back(model.addConstr(x[y_location_first] - x[y_location_second] == 0));
				//con.add(x[y_location_first] - x[y_location_second] == 0);
				linear_constrs.push_back(model.addConstr(x[y_location_first] + x[h_location_first] - x[y_location_second] - x[h_location_second] == 0));
			}
		}
	}
}

void LayoutGenerator::BoundingBoxConstrains(GRBModel& model)
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

void LayoutGenerator::PaddingConstrains(GRBModel& model)
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

				linear_constrs.push_back(model.addConstr(x[4 * thisIdx] - x[4 * leftNodeIdx] - x[4 * leftNodeIdx + 2] == 0));
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

				linear_constrs.push_back(model.addConstr(x[4 * thisIdx + 1] - x[4 * upNodeIdx + 1] - x[4 * upNodeIdx + 3] == 0));
				//con.add(x[4 * thisIdx + 1] - x[4 * upNodeIdx + 1] - x[4 * upNodeIdx + 3] - x[interval_index] == 0);
			}
		}
	}
}

void LayoutGenerator::MarginConstrains(GRBModel& model)
{
	linear_constrs.push_back(model.addConstr(x[margin_index_left] - x[margin_index_right]  == 0));
	linear_constrs.push_back(model.addConstr(x[margin_index_top] - x[margin_index_bottom]  == 0));
}

void LayoutGenerator::ImgCorresConstrains(GRBModel& model)
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

void LayoutGenerator::ImgRatioConstrains(GRBModel& model)
{
	//img ratio constraints
	int img_idx = 0;
	std::stack<CCombineTreeNode*> node_stack;
	while (!node_stack.empty())	node_stack.pop();
	double slack_factor_less = 0.8; //图片宽高比松弛因子
	double slack_factor_more = 1.2;
	double inifine_num = 10000000.0;
	for (int i = 0; i < img_node_list.size(); ++i)
	{
		for (int j = 0; j < article_->img_list_.size(); ++j)
		{
			double ratio = article_->img_list_[j].width() / article_->img_list_[j].height();
			int x_idx = 4 * ptr_idx_map[img_node_list[i]] + 0;
			int y_idx = 4 * ptr_idx_map[img_node_list[i]] + 1;
			int w_idx = 4 * ptr_idx_map[img_node_list[i]] + 2;
			int h_idx = 4 * ptr_idx_map[img_node_list[i]] + 3;
			linear_constrs.push_back(model.addConstr(grid_width *( x[4 * ptr_idx_map[img_node_list[i]] + 2] - (x[margin_index_left] + x[margin_index_right]) / grid_num_h)- x[padding_index_h]  - (ratio * slack_factor_more) * (grid_height * x[4 * ptr_idx_map[img_node_list[i]] + 3] - x[padding_index_v] - (x[margin_index_top] + x[margin_index_bottom]) / grid_num_v) + inifine_num * (img_corres_x[i][j] - 1) <= 0));
			linear_constrs.push_back(model.addConstr(- grid_width * x[4 * ptr_idx_map[img_node_list[i]] + 2] - x[padding_index_h] - (x[margin_index_left] + x[margin_index_right]) / grid_num_h + (ratio * slack_factor_less) * (grid_height * x[4 * ptr_idx_map[img_node_list[i]] + 3] - x[padding_index_v] - (x[margin_index_top] + x[margin_index_bottom]) / grid_num_v) + inifine_num * (img_corres_x[i][j] - 1) <= 0));
		}
	}
}

void LayoutGenerator::TextAreaConstrains(GRBModel& model)
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
		for(size_t j = 0; j < article_->text_list_[i].body_text.size();++j)
		{
			letter_count += article_->text_list_[i].body_text[j].size();
			body_text_num++;
		}
		
	}
	//text_area_max = (letter_count + 300 * article_.text_list.size()) * font_metrics.averageCharWidth() * (font_metrics.ascent() + font_metrics.descent() + row_space);
	//text_area_min = (letter_count + 30 * article_.text_list.size()) * font_metrics.averageCharWidth() * (font_metrics.ascent() + font_metrics.descent() + row_space);
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
			text_up_bound_con_exp += grid_width * grid_height * x[w] * x[h];
			text_low_bound_con_exp += -grid_width * grid_height * x[w] * x[h];

			//text_up_bound_con_exp += x[w] * x[h];
			//text_low_bound_con_exp += -x[w] * x[h];
			//文字面积小于约束
			//文字面积大于约束		
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

void LayoutGenerator::FixGeometryConstraints(GRBModel& model)
{
	for (int i = 0; i < allnodes.size(); ++i)
	{
		if(allnodes[i]->fix_geometry)
		{
			int x_idx = 4 * i;
			int y_idx = 4 * i + 1;
			int w_idx = 4 * i + 2;
			int h_idx = 4 * i + 3;
			linear_constrs.push_back(model.addConstr(allnodes[i]->x - 10 <= x[x_idx]));
			linear_constrs.push_back(model.addConstr(x[x_idx] <= allnodes[i]->x + 10));
			linear_constrs.push_back(model.addConstr(allnodes[i]->y - 10 <= x[y_idx]));
			linear_constrs.push_back(model.addConstr(x[y_idx] <= allnodes[i]->y + 10));
			linear_constrs.push_back(model.addConstr(x[w_idx] == allnodes[i]->width));
			linear_constrs.push_back(model.addConstr(x[h_idx] == allnodes[i]->height));
			std::cout << "fix const" << std::endl;
		}
	}

}

void LayoutGenerator::GetResult(GRBModel& model)
{
	double padding_h = x[padding_index_h].get(GRB_DoubleAttr_X);
	double padding_v = x[padding_index_v].get(GRB_DoubleAttr_X);
	double margin_left = x[margin_index_left].get(GRB_DoubleAttr_X);
	double margin_right = x[margin_index_right].get(GRB_DoubleAttr_X);
	double margin_top = x[margin_index_top].get(GRB_DoubleAttr_X);
	double margin_bottom = x[margin_index_bottom].get(GRB_DoubleAttr_X);


	for (int i = 0; i < allnodes.size(); ++i)
	{
		int start_grid_x = x[i * 4].get(GRB_DoubleAttr_X);
		int start_grid_y = x[i * 4 + 1].get(GRB_DoubleAttr_X);
		int cross_grid_num_h = x[i * 4 + 2].get(GRB_DoubleAttr_X);
		int cross_grid_num_v = x[i * 4 + 3].get(GRB_DoubleAttr_X);


		if (start_grid_x == 1) allnodes[i]->x = 0.0 + margin_left;
		else allnodes[i]->x = (start_grid_x - 1) * grid_width + 0.5 * padding_h + margin_left;
		if (start_grid_y == 1) allnodes[i]->y = 0.0 + margin_top;
		else allnodes[i]->y = (start_grid_y - 1) * grid_height + 0.5 * padding_v;

		allnodes[i]->width = cross_grid_num_h * grid_width - padding_h;
		allnodes[i]->height = cross_grid_num_v * grid_height - padding_v;

		if (start_grid_x == 1) allnodes[i]->width += 0.5 * padding_h;
		if (start_grid_x + cross_grid_num_h - 1 == grid_num_h) allnodes[i]->width += 0.5 * padding_h;

		if (start_grid_y == 1) allnodes[i]->height += 0.5 * padding_v;
		if (start_grid_y + cross_grid_num_v - 1 == grid_num_v) allnodes[i]->height += 0.5 * padding_v;

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

bool LayoutGenerator::GenerateLayout(GRBModel model)
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
	for (int template_idx = 0; template_idx < grid_template_list.size(); ++template_idx)
	{
		grid_num_h = grid_template_list[template_idx].first;
		grid_num_v = grid_template_list[template_idx].second;
		grid_width = W / grid_num_h;
		grid_height = H / grid_num_v;

		InitVariables(model);
		SetObject(model);
		RootConstrains( model);
		AlignmentConstrains(model);
		BoundingBoxConstrains(model);
		PaddingConstrains(model);
		ImgCorresConstrains(model);
		ImgRatioConstrains(model);
		TextAreaConstrains(model);
		FixGeometryConstraints(model);
		try
		{
			clock_t start = clock();
			model.optimize();
			//std::cout << model.get(GRB_IntAttr_Status) << std::endl;
			if (model.get(GRB_IntAttr_Status) == GRB_OPTIMAL || model.get(GRB_IntAttr_Status) == GRB_SUBOPTIMAL)
			{
				GetResult(model);
				success = true;
				//valid_solutions_.push_back(combine_tree_);
				clock_t end = clock();
				double used_time = static_cast<double>(end - start) / static_cast<double>(CLOCKS_PER_SEC); //计算时间
				std::cout << "solved time: " << used_time << std::endl;
				template_idx = grid_template_list.size();
				combine_tree_->grid_template = { grid_num_h,grid_num_v };
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
	}
	return success;
}
