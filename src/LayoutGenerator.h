#pragma once
#include "CCombineTreeNode.h"
#include <gurobi_c++.h>
#include <unordered_map>

#include "Article.h"


class LayoutGenerator
{
private:
	double W,H;
	//std::vector<CCombineTreeNode*> tree_candidates_;
	CCombineTreeNode* combine_tree_;
	Article* article_;
	//GRBModel model;// = GRBModel(GRBEnv(true));
	const std::vector<int> font_size_list = {3,4,5,6,7,8};
	std::vector<double> text_area_min_list = {};
	std::vector<double> text_area_max_list = {};
	std::vector<std::pair<int, int>> grid_template_list = { {4,6},{4,7},{4,8},{4,9},{4,10},
	{5,6},{5,7},{5,8},{5,9},{5,10},
	{6,6},{6,7},{6,8},{6,9},{6,10}};
	//std::vector<std::pair<int, int>> grid_template_list = { {12,20}};
	std::vector<std::vector<double>> grid_ratio_list;
	std::vector<GRBVar> x;
	std::vector<std::vector<GRBVar>> img_corres_x;
	std::vector<GRBConstr> linear_constrs;
	std::vector<GRBQConstr> q_constrs;
	GRBQuadExpr object;
	std::vector<CCombineTreeNode*> allnodes;
	std::unordered_map<CCombineTreeNode*, int> ptr_idx_map;
	std::unordered_map<int, CCombineTreeNode*> idx_ptr_map;
	std::vector<CCombineTreeNode*> img_node_list;
	int grid_num_h;
	int grid_num_v;
	double grid_width;
	double grid_height;
	int padding_index_h ; //横边距变量的下标
	int padding_index_v ; //竖边距变量的下标
	int margin_index_left;
	int margin_index_right;
	int margin_index_top;
	int margin_index_bottom;

	void TreePreProcess();
	void InitVariables(GRBModel& model);
	void SetObject(GRBModel& model);
	void RootConstrains(GRBModel& model);
	void AlignmentConstrains(GRBModel& model);
	void BoundingBoxConstrains(GRBModel& model);
	void PaddingConstrains(GRBModel& model);
	void MarginConstrains(GRBModel& model);
	void ImgCorresConstrains(GRBModel& model);
	void ImgRatioConstrains(GRBModel& model);
	void TextAreaConstrains(GRBModel& model);
	void FixGeometryConstraints(GRBModel& model);
	void GetResult(GRBModel& model);

public:
	
	//std::vector<CCombineTreeNode*> valid_solutions_;
	LayoutGenerator()
	{
		W = 630.0;
		H = 891.0;
		for (size_t i = 0; i < grid_template_list.size(); ++i)
		{
			int w, h, w_max, h_max;
			w_max = grid_template_list[i].first;
			h_max = grid_template_list[i].second;
			grid_ratio_list.push_back(std::vector<double>());
			for (w = 1; w <= w_max; ++w)
			{
				for (h = 1; h <= h_max; ++h)
				{
					double ratio;
					ratio = W * w / w_max / H * h / h_max;
					grid_ratio_list[i].push_back(ratio);
				}
			}
		}
		//GRBEnv env = model.getEnv();
		//GRBEnv env = GRBEnv(true);
		//env.start();
		//model = GRBModel(env);
		//model.set("NonConvex", "2");
		//model.set("TimeLimit", "0.1");
		//model.set("OutputFlag", "0");
	}

	//void SetTreeCandidates(std::vector<CCombineTreeNode*> tree_candidates);
	void SetArticle(Article* article);
	void SetCombineTree(CCombineTreeNode* tree);
	bool GenerateLayout(GRBModel model);
	
	void SetW(double w)
	{
		W = w;
	}
	void SetH(double h)
	{
		H = h;
	}
};



