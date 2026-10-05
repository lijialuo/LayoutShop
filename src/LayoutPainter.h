#pragma once
#include "LayoutShop.h"
#include <qgraphicsscene.h>
#include "Article.h"
#include "CCombineTreeNode.h"
#include "CustomGraphicsItem.h"


class LayoutPainter
{
	
public:
	LayoutPainter();
	~LayoutPainter();



    void SetArticle(Article* article){ article_ = article; }
	void SetW(double w) { W = w; }
	void SetH(double h) { H = h; }
    void SetLayoutTree(std::vector<CCombineTreeNode*> tree_list){layout_tree_ = tree_list;}
    void SetTitleFontSizeRange(int min_val, int max_val);
    void SetTextFontSizeRange(int min_val, int max_val);
    void SetTitleFontType(std::string font_type);
    void SetTextFontType(std::string font_type);
	LayoutShop::Preview Draw();
	CustomGraphicsScene* GenerateScene(bool preview);
    void DrawText(CustomGraphicsScene* scene, std::vector<CustomGraphicsItem*> text_item_list, QFont font);
    void DrawTextForParas(CustomGraphicsScene* scene, std::vector<CustomGraphicsItem*> text_item_list, QFont font);
	void CalTextAndTitleArea();


private:
	//QGraphicsScene* scene_;
	std::vector<CCombineTreeNode*> layout_tree_;
	Article* article_;
//	QFont text_font_;
//    QFont title_font_;
	std::vector<int> text_size_list_ = {};
    std::vector<int> title_size_list_ = {};
	std::vector<double> all_text_area_min_list_ = {};
	std::vector<double> all_text_area_max_list_ = {};
    std::vector<std::vector<double>> para_text_area_min_list_ = {};
    std::vector<std::vector<double>> para_text_area_max_list_ = {};
    std::vector<double> title_area_min_list_ = {};
    std::vector<double> title_area_max_list_ = {};
//    int min_title_font_size_,max_title_font_size_,min_text_font_size_,max_text_font_size_;
    std::string title_font_type_,text_font_type_;
	double W, H;
	bool paragraph_ = false;

	void DrawText(CustomGraphicsScene* scene, std::vector<CustomGraphicsItem*> leaf_item_list);
    void DrawTextForParas(CustomGraphicsScene* scene, std::vector<CustomGraphicsItem*> leaf_item_list);
	//void FillText(std::vector<CustomGraphicsItem*> item_list, int row_space);
	void FillImg(CustomGraphicsItem* item, int img_idx);
	void FillTitle(CustomGraphicsScene* scene, CustomGraphicsItem* item);

	const std::vector<QColor>  LABEL_COLOR_ = {
		QColor(220, 220, 170),  // title_
		QColor(78, 201, 176),  // text
		QColor(86, 156, 198),  // picture
		QColor(154, 154, 154),  // padding
		nullptr
	};

	const std::vector<QString>  LAYOUT_ICON_ =
	{
		":/LayoutShop/resources/layout-icon/header-white.svg",  // title_
		":/LayoutShop/resources/layout-icon/text-white.svg",  // text
		":/LayoutShop/resources/layout-icon/image.svg",  // picture
		nullptr,  // padding
		nullptr //non-label
	};
};

