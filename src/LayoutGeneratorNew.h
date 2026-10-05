#pragma once
#include "CCombineTreeNode.h"
#include <gurobi_c++.h>
#include <unordered_map>

#include "Article.h"


class LayoutGeneratorNew
{
private:
	double W, H;
	//std::vector<CCombineTreeNode*> tree_candidates_;
	CCombineTreeNode* combine_tree_;
	Article* article_;
	//GRBModel model;// = GRBModel(GRBEnv(true));
	const std::vector<int> font_size_list = { 6,7,8,9,10,11,12,13,14 };
	std::vector<double> text_area_min_list = {};
	std::vector<double> text_area_max_list = {};
	//std::vector<std::pair<int, int>> grid_template_list = { {12,20}};
	std::vector<GRBVar> x;
	std::vector<std::vector<GRBVar>> img_corres_x;
	//std::unordered_map<int,GRBVar> align_x1;
	//std::unordered_map<int, GRBVar> align_x2;
	std::vector<GRBConstr> linear_constrs;
	std::vector<GRBQConstr> q_constrs;
	GRBQuadExpr object;
	std::vector<CCombineTreeNode*> allnodes;
	std::unordered_map<CCombineTreeNode*, int> ptr_idx_map;
	std::unordered_map<int, CCombineTreeNode*> idx_ptr_map;
	std::vector<CCombineTreeNode*> img_node_list;
	
	int padding_index_h; //横边距变量的下标
	int padding_index_v; //竖边距变量的下标
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
	void TitleAreaConstrains(GRBModel& model);
	void GetResult(GRBModel& model);

public:

	//std::vector<CCombineTreeNode*> valid_solutions_;
	LayoutGeneratorNew()
	{
		W = 630.0;
		H = 891.0;
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



