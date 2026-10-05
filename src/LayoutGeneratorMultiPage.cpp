#include "LayoutGeneratorMultiPage.h"
#include <QGuiApplication>
#include <iostream>
#include <qfontmetrics.h>
#include <stack>


//void LayoutGeneratorMultiPage::SetTreeCandidates(std::vector<CCombineTreeNode*> tree_candidates)
//{
//	tree_candidates_ = tree_candidates;
//}



void LayoutGeneratorMultiPage::SetArticle(Article* article)
{
	article_ = article;
}

void LayoutGeneratorMultiPage::SetCombineTree(std::vector<CCombineTreeNode*> tree)
{
	combine_tree_ = tree;
}

void LayoutGeneratorMultiPage::TreePreProcess()
{
	
	allnodes.clear();
	ptr_idx_map.clear();
	img_node_list.clear();
    text_node_list.clear();
	page_seperate_idx.clear();
    img_node_page_list.clear();

	for(int i = 0; i < combine_tree_.size();++i)
	{
		page_seperate_idx.push_back(allnodes.size());
		std::queue<CCombineTreeNode*> combine_q;
		combine_q.push(combine_tree_[i]);
		while (!combine_q.empty())
		{
			ptr_idx_map[combine_q.front()] = allnodes.size();
			allnodes.push_back(combine_q.front());
			combine_q.pop();
			////std::cout << allnodes.back()->index << std::endl;
			for (auto ptr : allnodes.back()->children)
			{
				combine_q.push(ptr);
			}
		}
	}

//	for (int i = 0; i < allnodes.size(); ++i)
//	{
//		if (allnodes[i]->node_present_type == NODE_PRESENT_TYPE::PICTURE)
//		{
//			img_node_list.push_back(allnodes[i]);
//		}
//	}

    for (int page_idx = 0; page_idx < combine_tree_.size(); ++page_idx)
    {
        std::stack<CCombineTreeNode*> stack;
        stack.push(combine_tree_[page_idx]);
        while (!stack.empty())
        {
            auto node = stack.top();
            stack.pop();
            if (node->node_present_type == NODE_PRESENT_TYPE::PICTURE)
            {
               img_node_list.push_back(node);
               img_node_page_list.push_back(page_idx);
            }
            else if(node->node_present_type == NODE_PRESENT_TYPE::TEXT)
            {
                text_node_list.push_back(node);
            }
            for (int i = node->children.size() - 1; i >= 0; i--)
            {
                stack.push(node->children[i]);
            }
        }
    }
}

void LayoutGeneratorMultiPage::InitVariables(GRBModel& model)
{
	x.clear();
	img_corres_x.clear();
	linear_constrs.clear();
	q_constrs.clear();
	padding_x.clear();
	margin_x.clear();
    paragraph_x.clear();
	/*align_x1.clear();
	align_x2.clear();*/
	for(int i = 0;i<allnodes.size();++i)
	{
        double low_bound_width, low_bound_height;

        if(allnodes[i]->fix_geometry_level >= 0)
        {
            if(allnodes[i]->fix_width <  element_low_coff_h[allnodes[i]->node_present_type] * W)
                low_bound_width = allnodes[i]->fix_width - 10;
            else
                low_bound_width = element_low_coff_h[allnodes[i]->node_present_type] * W;
            if(allnodes[i]->fix_height < element_low_coff_v[allnodes[i]->node_present_type] * H)
                low_bound_height = allnodes[i]->fix_height - 10;
            else
                low_bound_height = element_low_coff_v[allnodes[i]->node_present_type] * H;
        }
        else
        {
            low_bound_width = element_low_coff_h[allnodes[i]->node_present_type] * W;
            low_bound_height = element_low_coff_v[allnodes[i]->node_present_type] * H;
        }
        x.push_back(model.addVar(0, W, 0.0, GRB_CONTINUOUS, "x " + std::to_string(i )));
        x.push_back(model.addVar(0, H, 0.0, GRB_CONTINUOUS,  "y " + std::to_string(i )));
        x.push_back(model.addVar(low_bound_width, element_high_coff_h[allnodes[i]->node_present_type] * W, 0.0, GRB_CONTINUOUS, "w " + std::to_string(i )));
        x.push_back(model.addVar(low_bound_height, element_high_coff_v[allnodes[i]->node_present_type] * H, 0.0, GRB_CONTINUOUS, "h " + std::to_string(i )));
	}
	//padding
	padding_x.push_back(model.addVar(padding_low_coff[0] * W, padding_high_coff[0] * W, 0.0, GRB_CONTINUOUS, "padding h" )); //h padding
	padding_x.push_back(model.addVar(padding_low_coff[1] * H, padding_high_coff[1] * H, 0.0, GRB_CONTINUOUS, "padding v" )); //v padding

	//margin
	margin_x.push_back(model.addVar(margin_low_coff[0] * W, margin_high_coff[0] * W, 0.0, GRB_CONTINUOUS, "margin left" )); //left margin
	margin_x.push_back(model.addVar(margin_low_coff[1] * W, margin_high_coff[1] * W, 0.0, GRB_CONTINUOUS, "margin right" )); //right margin
	margin_x.push_back(model.addVar(margin_low_coff[2] * H, margin_high_coff[2] * H, 0.0, GRB_CONTINUOUS, "margin top" )); //top margin
	margin_x.push_back(model.addVar(margin_low_coff[3] * H, margin_high_coff[3] * H, 0.0, GRB_CONTINUOUS, "margin bottom" )); //bottom margin

//    std::cout<< padding_low_coff[0] * W << " " << padding_high_coff[0] * W <<std::endl;
//    std::cout<< padding_low_coff[1] * H << " " << padding_high_coff[1] * H <<std::endl;
//    std::cout<< margin_low_coff[0] * W << " " << margin_high_coff[0] * W <<std::endl;
//    std::cout<< margin_low_coff[1] * W << " " << margin_high_coff[1] * W <<std::endl;
//    std::cout<< margin_low_coff[2] * H << " " << margin_high_coff[2] * H <<std::endl;
//    std::cout<< margin_low_coff[3] * H << " " << margin_high_coff[3] * H <<std::endl;
//
//    std::cout<< element_low_coff_h * W <<" " << element_high_coff_h * W << std::endl;
//    std::cout<< element_low_coff_v * H <<" " << element_high_coff_v * H << std::endl;

	for (int i = 0; i < article_->img_list_.size(); ++i)
	{
		img_corres_x.push_back(std::vector<GRBVar>());
		for (int j = 0; j < img_node_list.size(); ++j)
		{
			img_corres_x[i].push_back(model.addVar(0, 1, 0, GRB_BINARY, "img_corres_" + std::to_string(i) + "_" + std::to_string(j)));
		}
	}

    if(paragraph_)
    {
        for(int i = min_text_font_size_; i <= max_text_font_size_; ++i)
        {
            paragraph_x.push_back(model.addVar(0, 1, 0, GRB_BINARY, "paragraph_font_" + std::to_string(i)));
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

void LayoutGeneratorMultiPage::RefSoftConst(GRBModel& model)
{
    for(auto ref:article_->ref_list_)
    {
        int para = ref.first[0];
        int word = ref.first[1] + (ref.first[2] - ref.first[1]) / 2;
        int img_idx = ref.second;
        int total = 0, target = 0;
        for(int i = 0; i < article_->text_list_.size(); ++i)
        {
            total += article_->text_list_[i].word_list.size();
            if(i < para)
            {
                target +=  article_->text_list_[i].word_list.size();
            }
        }
        target += word;
        int target_page = std::min(static_cast<int>(combine_tree_.size() - 1), static_cast<int>(combine_tree_.size() * (static_cast<double>(target) / total)));
        std::cout<< target << " " << total << " " << static_cast<double>(target) / total << " " <<  target_page << std::endl;
        for(int i = 0; i < img_node_page_list.size(); ++i)
        {
            if(img_node_page_list[i] == target_page)
            {
                // set soft constraint
                object += (1 - img_corres_x[i][img_idx]) * 100000 ;
            }
        }

    }


}


void LayoutGeneratorMultiPage::SetObject(GRBModel& model)
{
	object.clear();
	for(int page_idx = 0; page_idx < combine_tree_.size(); ++page_idx)
	{
		int page_start = page_seperate_idx[page_idx];
		int page_end = (page_idx < combine_tree_.size() - 1) ? page_seperate_idx[page_idx + 1] : allnodes.size();
		//text balance
		
		double W1 = 100, W2 = 100;
		for (int i = page_start; i < page_end; ++i)
		{
			if (allnodes[i]->node_present_type == NODE_PRESENT_TYPE::TEXT)
			{
				int width_idx1 = i * 4 + 2;
				int height_idx1 = i * 4 + 3;
				for (int j = i + 1; j < page_end; ++j)
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
        ////img balance
        ///
		double W3 = 50;
		for (int i = page_start; i < page_end; ++i)
		{
			if (allnodes[i]->node_present_type == NODE_PRESENT_TYPE::PICTURE)
			{
				int width_idx1 = i * 4 + 2;
				int height_idx1 = i * 4 + 3;
				for (int j = i + 1; j < page_end; ++j)
				{
					if (allnodes[j]->node_present_type == NODE_PRESENT_TYPE::PICTURE)
					{
						int width_idx2 = j * 4 + 2;
						int height_idx2 = j * 4 + 3;
						object += W3 * (x[width_idx1] + x[height_idx1]
							- x[width_idx2] - x[height_idx2]) *
							(x[width_idx1] + x[height_idx1]
								- x[width_idx2] - x[height_idx2]);
//                        object += W3 * (x[width_idx1] - x[width_idx2]) * (x[width_idx1] - x[width_idx2] );
//                        object += W3 * (x[height_idx1] - x[height_idx2]) * (x[height_idx1] - x[height_idx2] );
					}
				}
			}
		}

		double W4 = 30;
		for (int i = page_start; i < page_end; ++i)
		{
			if (allnodes[i]->node_present_type != NODE_PRESENT_TYPE::NONLABEL
				&& allnodes[i]->node_present_type != NODE_PRESENT_TYPE::PADDING)
			{
				int width_idx1 = i * 4 + 2;
				int height_idx1 = i * 4 + 3;
				for (int j = i + 1; j < page_end; ++j)
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

        double W5 = 50;
        QFont prefer_font;
        prefer_font.setFamily(QString(text_font_type_.data()));
        prefer_font.setPointSize((min_text_font_size_ + max_text_font_size_) / 2);
        QFontMetrics font_metrics(prefer_font);
        double ave_len = 5, word_num = 8;
        double prefer_width = (ave_len + 1) * word_num * font_metrics.averageCharWidth();
        for (int i = page_start; i < page_end; ++i)
        {
            if (allnodes[i]->node_present_type == NODE_PRESENT_TYPE::TEXT)
            {
                int width_idx = i * 4 + 2;
                object += W5 * (x[width_idx] * x[width_idx] - 2 * x[width_idx] * prefer_width + prefer_width * prefer_width);
            }
        }

        double W6 = 10000;
        for (int i = page_start; i < page_end; ++i)
        {
            if (allnodes[i]->fix_geometry_level >= 1)
            {
                int x_idx = i * 4;
                int y_idx = i * 4 + 1;
                int width_idx = i * 4 + 2;
                int height_idx = i * 4 + 3;

                object += W6 / allnodes[i]->fix_geometry_level * (x[x_idx] * x[x_idx] - 2 * x[x_idx] * allnodes[i]->fix_x + allnodes[i]->fix_x * allnodes[i]->fix_x);
                object += W6 / allnodes[i]->fix_geometry_level * (x[y_idx] * x[y_idx] - 2 * x[y_idx] * allnodes[i]->fix_y + allnodes[i]->fix_y * allnodes[i]->fix_y);
                object += W6 / allnodes[i]->fix_geometry_level * (x[width_idx] * x[width_idx] - 2 * x[width_idx] * allnodes[i]->fix_width + allnodes[i]->fix_width * allnodes[i]->fix_width);
                object += W6 / allnodes[i]->fix_geometry_level * (x[height_idx] * x[height_idx] - 2 * x[height_idx] * allnodes[i]->fix_height + allnodes[i]->fix_height * allnodes[i]->fix_height);

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

    //img ratio const
    double W7 = 2000;
    for(auto node:img_node_list)
    {
        if(node->picture_lock)
        {
            int w_idx = ptr_idx_map[node] * 4 + 2;
            int h_idx = ptr_idx_map[node] * 4 + 3;
            int img_idx = node->corres_img_idx;
            double ratio = article_->img_list_[img_idx].width() / article_->img_list_[img_idx].height();
            object += W7 * (w_idx - ratio * h_idx) * (w_idx - ratio * h_idx);
        }
    }

	model.setObjective(object, GRB_MINIMIZE);
}


void LayoutGeneratorMultiPage::RootConstrains(GRBModel& model)
{
	for(int i = 0; i < combine_tree_.size(); ++i)
	{
		int root_order = ptr_idx_map[combine_tree_[i]];
		int x_location_root = root_order * 4 ;
		int y_location_root = root_order * 4 + 1;
		int width_location_root = root_order * 4 + 2;
		int height_location_root = root_order * 4 + 3;

		linear_constrs.push_back(model.addConstr(x[x_location_root] ==  margin_x[0],"root x const"));
		linear_constrs.push_back(model.addConstr(x[y_location_root] == margin_x[2]));
		linear_constrs.push_back(model.addConstr(x[width_location_root] == W - margin_x[0] - margin_x[1]));
		linear_constrs.push_back(model.addConstr(x[height_location_root] == H - margin_x[2] - margin_x[3]));
	}
	
}

void LayoutGeneratorMultiPage::AlignmentConstrains(GRBModel& model)
{
	//vertical_align
	for (int i = 0; i < allnodes.size(); i++)
	{
		if (allnodes[i]->node_relation == NODE_RELATION::VERTICAL)
		{
			for (int j = 0; j < allnodes[i]->children.size() - 1; j++)
			{
				int order_first = ptr_idx_map[allnodes[i]->children[j]];
				int order_second = ptr_idx_map[allnodes[i]->children[j + 1]];
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
				int order_first = ptr_idx_map[allnodes[i]->children[j]];
				int order_second = ptr_idx_map[allnodes[i]->children[j + 1]];
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

void LayoutGeneratorMultiPage::BoundingBoxConstrains(GRBModel& model)
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

void LayoutGeneratorMultiPage::PaddingConstrains(GRBModel& model)
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

				linear_constrs.push_back(model.addConstr(x[4 * thisIdx] - x[4 * leftNodeIdx] - x[4 * leftNodeIdx + 2] -  padding_x[0] == 0));
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

				linear_constrs.push_back(model.addConstr(x[4 * thisIdx + 1] - x[4 * upNodeIdx + 1] - x[4 * upNodeIdx + 3] - padding_x[1] == 0));
				//con.add(x[4 * thisIdx + 1] - x[4 * upNodeIdx + 1] - x[4 * upNodeIdx + 3] - x[interval_index] == 0);
			}
		}
	}
}

void LayoutGeneratorMultiPage::MarginConstrains(GRBModel& model)
{
	//linear_constrs.push_back(model.addConstr(x[margin_index_left] - x[margin_index_right] == 0));
	//linear_constrs.push_back(model.addConstr(x[margin_index_top] - x[margin_index_bottom] == 0));
}

void LayoutGeneratorMultiPage::ImgCorresConstrains(GRBModel& model)
{

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

    for(int i = 0; i < img_node_list.size(); ++i)
    {
        auto node = img_node_list[i];
        if(node->picture_lock)
        {
            linear_constrs.push_back(model.addConstr(img_corres_x[i][node->corres_img_idx] == 1));
        }
    }
	//img corres constr
//	if(img_order_)
//	{
//		for (int i = 0; i < img_corres_x.size(); ++i)
//		{
//			for (int j = 0; j < img_corres_x[i].size(); ++j)
//			{
//				if(i==j) linear_constrs.push_back(model.addConstr(img_corres_x[i][j] == 1));
//				else linear_constrs.push_back(model.addConstr(img_corres_x[i][j] == 0));
//			}
//		}
//	}
//	else
//	{
//    for (int i = 0; i < img_corres_x.size(); ++i)
//    {
//        GRBLinExpr constr_expr;
//        for (int j = 0; j < img_corres_x[i].size(); ++j)
//        {
//            constr_expr += img_corres_x[i][j];
//        }
//        linear_constrs.push_back(model.addConstr(constr_expr == 1));
//    }
//
//    for (int j = 0; j < img_corres_x[0].size(); ++j)
//    {
//        GRBLinExpr constr_expr;
//        for (int i = 0; i < img_corres_x.size(); ++i)
//        {
//            constr_expr += img_corres_x[i][j];
//        }
//        linear_constrs.push_back(model.addConstr(constr_expr == 1));
//    }
//	}

}

void LayoutGeneratorMultiPage::ImgRatioConstrains(GRBModel& model)
{
	//img ratio constraints

    double slack_factor_less;
    double slack_factor_more;
    double inifine_num = 10000000.0;
    for (int i = 0; i < img_node_list.size(); ++i)
    {
        if(img_node_list[i]->picture_lock)
        {
            slack_factor_less = 0.5;
            slack_factor_more = 1.5;
        }
        else
        {
            slack_factor_less = 0.7;
            slack_factor_more = 1.3;
        }

        for (int j = 0; j < article_->img_list_.size(); ++j)
        {
            double ratio = static_cast<double>(article_->img_list_[j].width()) / article_->img_list_[j].height();
            int w_idx = 4 * ptr_idx_map[img_node_list[i]] + 2;
            int h_idx = 4 * ptr_idx_map[img_node_list[i]] + 3;
            linear_constrs.push_back(model.addConstr(x[w_idx] - ratio * slack_factor_more * x[h_idx] + inifine_num * (img_corres_x[i][j] - 1) <= 0));
            linear_constrs.push_back(model.addConstr(-x[w_idx] + ratio * slack_factor_less * x[h_idx] + inifine_num * (img_corres_x[i][j] - 1) <= 0));
        }
    }
}

void LayoutGeneratorMultiPage::TextAreaConstrains(GRBModel& model)
{
	int letter_count = 0;
	QFont font;
	//const int font_size = 4;//12;
	const int row_space_min = 3;//3;
    const int row_space_max = 10;
	//font.setPixelSize(font_size);
    font.setFamily(text_font_type_.data());
	//font.setFamily("Courier New");
	//int body_text_num = 0;

//    font.setPointSize(min_text_font_size_);
//
//    QFontMetrics font_metrics(font);
//    for (size_t i = 0; i < article_->text_list_.size(); ++i)
//    {
//        for (size_t j = 0; j < article_->text_list_[i].word_list.size(); ++j)
//        {
//            min_area += font_metrics.horizontalAdvance(QString(article_->text_list_[i].word_list[j].data())) * (row_space_min + font_metrics.height());
//        }
//    }
//
//    font.setPointSize(max_text_font_size_);
//    font_metrics = QFontMetrics(font);
//    for (size_t i = 0; i < article_->text_list_.size(); ++i)
//    {
//        for (size_t j = 0; j < article_->text_list_[i].word_list.size(); ++j)
//        {
//            max_area += font_metrics.horizontalAdvance(QString(article_->text_list_[i].word_list[j].data())) * (row_space_max + font_metrics.height());
//        }
//    }

    para_min_text_area_list.clear();
    para_max_text_area_list.clear();
    all_min_text_area_list.clear();
    all_max_text_area_list.clear();
    for(int text_font_size = min_text_font_size_; text_font_size <= max_text_font_size_; text_font_size++)
    {
        font.setPointSize(text_font_size);
        QFontMetrics  font_metrics = QFontMetrics(font);
        double all_min_area = 0, all_max_area = 0;
        para_min_text_area_list.push_back(std::vector<int>());
        para_max_text_area_list.push_back(std::vector<int>());

        for (size_t i = 0; i < article_->text_list_.size(); ++i)
        {
            double para_min_area = 0, para_max_area = 0;
            for (size_t j = 0; j < article_->text_list_[i].word_list.size(); ++j)
            {
                para_min_area += font_metrics.horizontalAdvance(QString(article_->text_list_[i].word_list[j].data())) * (row_space_min + font_metrics.height());
                para_max_area += font_metrics.horizontalAdvance(QString(article_->text_list_[i].word_list[j].data())) * (row_space_max + font_metrics.height());
            }
            para_min_text_area_list[text_font_size - min_text_font_size_].push_back(para_min_area);
            para_max_text_area_list[text_font_size - min_text_font_size_].push_back(para_max_area);
            all_min_area += para_min_area;
            all_max_area += para_max_area;
        }
        all_min_text_area_list.push_back(all_min_area);
        all_max_text_area_list.push_back(all_max_area);
    }


//	for (size_t i = 0; i < article_->text_list_.size(); ++i)
//	{
//		for (size_t j = 0; j < article_->text_list_[i].body_text.size(); ++j)
//		{
//			letter_count += article_->text_list_[i].body_text[j].size();
//			//body_text_num++;
//		}
//	}


//	text_area_max_list.clear();
//	text_area_min_list.clear();
//	for (int i = 0; i < text_font_size_list.size(); ++i)
//	{
//		font.setPointSize(text_font_size_list[i]);
//		QFontMetrics font_metrics(font);
//		text_area_max_list.push_back((letter_count) * font_metrics.averageCharWidth() * (font_metrics.ascent() + font_metrics.descent() + row_space_max));
//		text_area_min_list.push_back((letter_count) * font_metrics.averageCharWidth() * (font_metrics.ascent() + font_metrics.descent() + row_space_min));
//	}

    if(!paragraph_)
    {
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
                text_up_bound_con_exp += x[w] * x[h];
                text_low_bound_con_exp += -x[w] * x[h];

                //text_up_bound_con_exp += x[w] * x[h];
                //text_low_bound_con_exp += -x[w] * x[h];
            }
        }
//	text_up_bound_con_exp += -text_area_max_list.back();
//	text_low_bound_con_exp += text_area_min_list[0];

        text_up_bound_con_exp += -all_max_text_area_list[all_max_text_area_list.size() - 1];
        text_low_bound_con_exp += all_min_text_area_list[0];

        //con.add(text_up_bound_con_exp <= 0);
        //con.add(text_low_bound_con_exp <= 0);


        //text_up_bound_con_exp.end();
        //text_low_bound_con_exp.end();

        q_constrs.push_back(model.addQConstr(text_up_bound_con_exp <= 0));
        q_constrs.push_back(model.addQConstr(text_low_bound_con_exp <= 0));
    }
    else
    {
        int infinite_num = 100000000.0;
        for (int i = 0; i < article_->text_list_.size(); ++i)
        {
            GRBQuadExpr constr_expr;
            for (int j = 0; j < paragraph_x.size(); ++j)
            {
                GRBQuadExpr expr;
                int w_idx = 4 * ptr_idx_map[text_node_list[i]] + 2;
                int h_idx = 4 * ptr_idx_map[text_node_list[i]] + 3;
                expr = x[w_idx] * x[h_idx];
                if(j == paragraph_x.size() - 1)
                    q_constrs.push_back(model.addQConstr(expr <= infinite_num * (1 - paragraph_x[j]) + para_max_text_area_list[j][i])); // latter term * paragraph_x[j]
                else
                    q_constrs.push_back(model.addQConstr(expr <= infinite_num * (1 - paragraph_x[j]) + para_min_text_area_list[j+1][i])); // latter term * paragraph_x[j]
                q_constrs.push_back(model.addQConstr(expr >= paragraph_x[j] *  para_min_text_area_list[j][i]));
            }
        }

        GRBLinExpr constr_expr;
        for (int i = 0; i < paragraph_x.size(); ++i)
        {
            constr_expr = constr_expr + paragraph_x[i];
        }
        linear_constrs.push_back(model.addConstr(constr_expr == 1));
    }

}

void LayoutGeneratorMultiPage::TitleAreaConstrains(GRBModel& model)
{

//	QFont font;
//	//const int font_size = 4;//12;
//	const int row_space = 2;//3;
//	//font.setPixelSize(font_size);
//	font.setFamily("Times New Roman");
//	int letter_count = article_->title_.headline.size();
//	//int body_text_num = 0;
//	std::vector<int> title_font_size_list = { 16,18,20,22,24,26,28,30 };
//	std::vector<double> title_area_min_list = {};
//	std::vector<double> title_area_max_list = {};
//	title_area_max_list.clear();
//	title_area_min_list.clear();
//	for (int i = 0; i < title_font_size_list.size(); ++i)
//	{
//		font.setPointSize(title_font_size_list[i]);
//		QFontMetrics font_metrics(font);
//		double area = letter_count * font_metrics.averageCharWidth() * (font_metrics.height() + row_space);
//		title_area_max_list.push_back(area * 5);
//		title_area_min_list.push_back(area * 2);
//		//std::cout << title_area_min_list[i] << " " << title_area_max_list[i] << std::endl;
//	}
//
//
//	GRBQuadExpr title_up_bound_con_exp;
//	GRBQuadExpr title_low_bound_con_exp;
//
//	for (size_t i = 0; i < allnodes.size(); ++i)
//	{
//		auto node = allnodes[i];
//		if (node->node_present_type == NODE_PRESENT_TYPE::TITLE)
//		{
//			int idx = ptr_idx_map[node];
//			int w = 4 * idx + 2;
//			int h = 4 * idx + 3;
//			title_up_bound_con_exp += x[w] * x[h];
//			title_low_bound_con_exp += -x[w] * x[h];
//
//			//title_up_bound_con_exp += x[w] * x[h];
//			//title_low_bound_con_exp += -x[w] * x[h];
//
//
//		}
//	}
//	title_up_bound_con_exp += -title_area_max_list.back();
//	title_low_bound_con_exp += title_area_min_list[0];
//
//	//con.add(title_up_bound_con_exp <= 0);
//	//con.add(title_low_bound_con_exp <= 0);
//
//
//	//title_up_bound_con_exp.end();
//	//title_low_bound_con_exp.end();
//
//	q_constrs.push_back(model.addQConstr(title_up_bound_con_exp <= 0));
//	q_constrs.push_back(model.addQConstr(title_low_bound_con_exp <= 0));
    GRBQuadExpr title_up_bound_con_exp;
    GRBQuadExpr title_low_bound_con_exp;
    CCombineTreeNode* title_node;
    for (size_t i = 0; i < allnodes.size(); ++i)
    {
        title_node = allnodes[i];
        if (title_node->node_present_type == NODE_PRESENT_TYPE::TITLE)
        {
            int idx = ptr_idx_map[title_node];
            int w = 4 * idx + 2;
            int h = 4 * idx + 3;
            title_up_bound_con_exp += x[w] * x[h];
            title_low_bound_con_exp += -x[w] * x[h];


            //////  this is for the fucking overlap title！！！
//            double ratio = 1.87;
//            double high = ratio * 1.3;
//            double low = ratio * 0.7;
//            linear_constrs.push_back(model.addConstr(x[w] - x[h] * high <= 0 ));
//            linear_constrs.push_back(model.addConstr(- x[w] + x[h] * low <= 0 ));
        }
    }

    QFont font;
    //const int font_size = 4;//12;
    //font.setPixelSize(font_size);
    font.setFamily(title_font_type_.data());
    //font.setFamily("Courier New");
    //int body_text_num = 0;
    double min_area = 0.0, max_area = 0.0;
    int least_title_len = 80;
    double low_bound_factor = 1.5, high_bound_factor = 4.5;
	// double low_bound_factor = 1.3, high_bound_factor = 10.0;
    font.setPointSize(min_title_font_size_);
    QFontMetrics font_metrics(font);
    min_area = font_metrics.horizontalAdvance(QString(article_->title_.data())) * font_metrics.height() * low_bound_factor ;


    font.setPointSize(max_title_font_size_);
    font_metrics = QFontMetrics(font);
    max_area = std::max(font_metrics.horizontalAdvance(QString(article_->title_.data())) , font_metrics.averageCharWidth() * least_title_len) * font_metrics.height() * high_bound_factor;

//    double max_area_special = 0.0;
//    if(title_node->parent->parent == nullptr)
//    {
//        if(title_node->parent->node_relation == HORIZONTAL)
//        {
//
//            max_area_special = W * (element_low_coff_v[TITLE] * H + 20);
//        }
//        else if(title_node->parent->node_relation == VERTICAL)
//        {
//            max_area_special = H * (element_low_coff_h[TITLE] * W + 20);
//        }
//    }
//
//    max_area = std::max(max_area, max_area_special);
//    if(max_area < 0.5 * H * W) max_area = 0.5 * H * W;
    title_up_bound_con_exp += -max_area;
    title_low_bound_con_exp += min_area;
    q_constrs.push_back(model.addQConstr(title_up_bound_con_exp <= 0));
    q_constrs.push_back(model.addQConstr(title_low_bound_con_exp <= 0));

}


void LayoutGeneratorMultiPage::PaddingElementConstrains(GRBModel &model)
{
    for(int i = 0; i < allnodes.size(); ++i)
    {
        if (allnodes[i]->node_present_type == NODE_PRESENT_TYPE::PADDING)
        {
            auto parent = allnodes[i]->parent;
            int sibling_size = parent->children.size();
            int parent_idx = ptr_idx_map[parent];
            if(parent->node_relation == NODE_RELATION::HORIZONTAL)
            {
                linear_constrs.push_back(model.addConstr(x[4 * i + 2]  <=  1.0 / sibling_size * x[4 * parent_idx + 2] ));
            }
            else if(parent->node_relation == NODE_RELATION::VERTICAL)
            {
                linear_constrs.push_back(model.addConstr(x[4 * i + 3]  <=  1.0 / sibling_size * x[4 * parent_idx + 3] ));
            }
        }
    }

}



void LayoutGeneratorMultiPage::FixGeometryConstraints(GRBModel& model)
{
	for (int i = 0; i < allnodes.size(); ++i)
	{
		if (allnodes[i]->fix_geometry_level == 0)
		{
			int x_idx = 4 * i;
			int y_idx = 4 * i + 1;
			int w_idx = 4 * i + 2;
			int h_idx = 4 * i + 3;
			linear_constrs.push_back(model.addConstr(allnodes[i]->fix_x - 10 <= x[x_idx]));
			linear_constrs.push_back(model.addConstr(x[x_idx] <= allnodes[i]->fix_x + 10));
			linear_constrs.push_back(model.addConstr(allnodes[i]->fix_y - 10 <= x[y_idx]));
			linear_constrs.push_back(model.addConstr(x[y_idx] <= allnodes[i]->fix_y + 10));
			linear_constrs.push_back(model.addConstr(x[w_idx] == allnodes[i]->fix_width));
			linear_constrs.push_back(model.addConstr(x[h_idx] == allnodes[i]->fix_height));
			//std::cout << "fix const" << std::endl;
		}
	}

}



void LayoutGeneratorMultiPage::GetResult(GRBModel& model)
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
		allnodes[i]->h_padding = padding_x[0].get(GRB_DoubleAttr_X);
		allnodes[i]->v_padding = padding_x[1].get(GRB_DoubleAttr_X);
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

bool LayoutGeneratorMultiPage::GenerateLayout(GRBModel model)
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
    //RefSoftConst(model);
	RootConstrains(model);
	AlignmentConstrains(model);
	BoundingBoxConstrains(model);
	PaddingConstrains(model);
	MarginConstrains(model);
	TitleAreaConstrains(model);
    PaddingElementConstrains(model);
    ImgCorresConstrains(model);
	ImgRatioConstrains(model);
    TextAreaConstrains(model);
	FixGeometryConstraints(model);
    // RefConstraints(model);

	try
	{
		//::cout << "start" << std::endl;
        auto start = std::chrono::steady_clock::now();
		model.optimize();
		
		/*GRBQConstr* c = 0;
		c = model.getQConstrs();

		for (int i = 0; i < model.get(GRB_IntAttr_NumQConstrs); ++i)
		{
			std::cout << c[i].get(GRB_StringAttr_QCName) << std::endl;
			std::cout << c[i].get(GRB_IntAttr_IISQConstr) << std::endl;
		}*/
			//std::cout << model.get(GRB_IntAttr_Status) << std::endl;
		if (model.get(GRB_IntAttr_Status) == GRB_OPTIMAL || model.get(GRB_IntAttr_Status) == GRB_SUBOPTIMAL)
		{
			GetResult(model);
			success = true;
			//valid_solutions_.push_back(combine_tree_);
            auto end = std::chrono::steady_clock::now();
            std::cout << "solved time: "
                      << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                      << " ms" << std::endl;
		}
		//else std::cout << "oops!" << std::endl;
	}
	catch (GRBException e)
	{
		std::cout << "Error code = " << e.getErrorCode() << std::endl;
		std::cout << e.getMessage() << std::endl;
		model.computeIIS();
		// model.write("C:\\Users\\lijia\\Desktop\\model.ilp");
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
	for(int i = 0; i < padding_x.size();++i)
	{
		model.remove(padding_x[i]);
	}
	for (int i = 0; i < margin_x.size(); ++i)
	{
		model.remove(margin_x[i]);
	}
	
	return success;
}

void LayoutGeneratorMultiPage::SetTitleFontSizeRange(int min_val, int max_val)
{
    min_title_font_size_ = min_val;
    max_title_font_size_ = max_val;
}

void LayoutGeneratorMultiPage::SetTextFontSizeRange(int min_val, int max_val)
{
    min_text_font_size_ = min_val;
    max_text_font_size_ = max_val;
}

void LayoutGeneratorMultiPage::SetTitleFontType(std::string font_type)
{
    title_font_type_ = font_type;
}

void LayoutGeneratorMultiPage::SetTextFontType(std::string font_type)
{
    text_font_type_ = font_type;
}

void LayoutGeneratorMultiPage::RefConstraints(GRBModel &model)
{
    int page_sum = combine_tree_.size();
    if(page_sum <= 1) return;
    std::vector<std::vector<int>> page_img_idx_list(page_sum , std::vector<int>());// each page should contain which images
    //std::vector<int> require_list(page, 0);
    std::vector<int> actual_page_img_node_list(page_sum, 0); // each page has what number of images

    for(auto ref_ratio:article_->ref_ratio_list_)
    {
        int target = std::min(page_sum - 1, static_cast<int>(page_sum * ref_ratio.second));
        page_img_idx_list[target].push_back(ref_ratio.first);
    }

    for(int i = 0; i < img_node_page_list.size(); ++i)
    {
        actual_page_img_node_list[img_node_page_list[i]] ++;
    }

    std::cout<< "page_img_idx_list:";
    for(auto item:page_img_idx_list) std::cout << item.size() << " ";
    std::cout << std::endl;

    std::cout<< "actual_page_img_node_list:";
    for(auto item: actual_page_img_node_list) std::cout << item << " ";
    std::cout << std::endl;

    for(int i = 0; i < page_img_idx_list.size(); ++i)
    {
        bool enough; // each page has enough images?
        if(page_img_idx_list[i].size() <= actual_page_img_node_list[i])
        {
            enough = true;
        }
        else
        {
            enough = false;
        }

        for(int j = 0; j < img_node_page_list.size(); ++j)
        {
//            if(enough && img_node_page_list[j] != i)
//            {
//                for(auto img_idx:page_img_idx_list[i])
//                {
//                    model.addConstr(img_corres_x[j][img_idx] == 0);
//                }
//            }
//            else if((!enough) && std::abs(img_node_page_list[j] - i) > 1)
//            {
//                std::cout<< img_node_page_list[j] << " " << i << std::endl;
//                for(auto img_idx:page_img_idx_list[i])
//                {
//                    model.addConstr(img_corres_x[j][img_idx] == 0);
//                }
//            }
            if(std::abs(img_node_page_list[j] - i) > 1)
            {
                //std::cout<< img_node_page_list[j] << " " << i << std::endl;
                for(auto img_idx:page_img_idx_list[i])
                {
                    model.addConstr(img_corres_x[j][img_idx] == 0);
                }
            }
        }



    }


}


