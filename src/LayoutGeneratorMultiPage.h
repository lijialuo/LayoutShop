#pragma once
#include "CCombineTreeNode.h"
#include <gurobi_c++.h>
#include <unordered_map>
#include <QFont>

#include "Article.h"

class LayoutGeneratorMultiPage
{
private:
	double W, H;
	//std::vector<CCombineTreeNode*> tree_candidates_;
	std::vector<CCombineTreeNode*> combine_tree_;
	Article* article_;
    int min_title_font_size_,max_title_font_size_,min_text_font_size_,max_text_font_size_;
    std::vector<std::vector<int>> para_max_text_area_list, para_min_text_area_list;   // font_size ,para
    std::vector<int> all_max_text_area_list, all_min_text_area_list;
    std::string title_font_type_,text_font_type_;
	std::vector<GRBVar> x;
	std::vector<std::vector<GRBVar>> img_corres_x;
    std::vector<GRBVar> paragraph_x;
	std::vector<GRBVar> padding_x;
	std::vector<GRBVar> margin_x;
    bool paragraph_ = false;


    std::vector<double> padding_low_coff = {0.022, 0.016};
    std::vector<double> padding_high_coff = {0.033, 0.024};
    std::vector<double> margin_low_coff = {0.044, 0.044, 0.0317, 0.0317};
//    std::vector<double> margin_low_coff = {0, 0, 0, 0};
//    std::vector<double> margin_high_coff = {0, 0, 0, 0.09};
    std::vector<double> margin_high_coff = {0.11, 0.11, 0.079, 0.079};
    //double element_low_coff_h = 0.11, element_high_coff_h = 1.0;
    std::vector<double> element_low_coff_h = {0.13, 0.17, 0.17, 0.13, 0.0};
    std::vector<double> element_high_coff_h = {1.0, 1.0, 1.0, 1.0 , 1.0};
    std::vector<double> element_low_coff_v = {0.08, 0.12, 0.12, 0.08, 0.0};
    std::vector<double> element_high_coff_v = {1.0, 1.0, 1.0, 1.0, 1.0};

	//std::unordered_map<int,GRBVar> align_x1;
	//std::unordered_map<int, GRBVar> align_x2;
	std::vector<GRBConstr> linear_constrs;
	std::vector<GRBQConstr> q_constrs;
	GRBQuadExpr object;
	std::vector<CCombineTreeNode*> allnodes;
	std::vector<int> page_seperate_idx;
	std::unordered_map<CCombineTreeNode*, int> ptr_idx_map;
	std::vector<CCombineTreeNode*> img_node_list;
    std::vector<CCombineTreeNode*> text_node_list;
    std::vector<int> img_node_page_list;


	void TreePreProcess();
	void InitVariables(GRBModel& model);
	void SetObject(GRBModel& model);
    void RefSoftConst(GRBModel& model);
    void RefConstraints(GRBModel& model);
	void RootConstrains(GRBModel& model);
	void AlignmentConstrains(GRBModel& model);
	void BoundingBoxConstrains(GRBModel& model);
	void PaddingConstrains(GRBModel& model);
	void MarginConstrains(GRBModel& model);
	void ImgCorresConstrains(GRBModel& model);
	void ImgRatioConstrains(GRBModel& model);
	void TextAreaConstrains(GRBModel& model);
	void TitleAreaConstrains(GRBModel& model);
    void PaddingElementConstrains(GRBModel& model);
	void GetResult(GRBModel& model);
	void FixGeometryConstraints(GRBModel& model);

public:

	//std::vector<CCombineTreeNode*> valid_solutions_;
	LayoutGeneratorMultiPage()
	{
		// W = 630.0;
		// H = 891.0;
		// W = 960.0;
		// H = 720.0;
	}

	//void SetTreeCandidates(std::vector<CCombineTreeNode*> tree_candidates);
	void SetArticle(Article* article);
	void SetCombineTree(std::vector<CCombineTreeNode*> tree);
	bool GenerateLayout(GRBModel model);

	void SetW(double w)
	{
		W = w;
	}
	void SetH(double h)
	{
		H = h;
	}
    void SetTitleFontSizeRange(int min_val, int max_val);
    void SetTextFontSizeRange(int min_val, int max_val);
    void SetTitleFontType(std::string font_type);
    void SetTextFontType(std::string font_type);
};

