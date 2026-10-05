#pragma once
#include <QGraphicsView>
#include <QWidget>
#include "Article.h"
#include "CCombineTreeNode.h"

class OutputWidget : public QWidget
{
	//for using 'signal' and 'slot'
	Q_OBJECT

public:
	OutputWidget();
	~OutputWidget();

	void paintEvent(QPaintEvent* event);
	void mousePressEvent(QMouseEvent* event);	//鼠标击发响应函数
	void mouseMoveEvent(QMouseEvent* event);	//鼠标移动响应函数
	void mouseReleaseEvent(QMouseEvent* event);	//鼠标释放响应函数

public:
	
	void SetLayoutTree(CCombineTreeNode* tree);
	void SetArticle(Article article);
	void SetPaintState(bool state);

	bool isedit = false;

private:
	CCombineTreeNode* layout_tree_;
	Article article_;
	double move = 50;
	bool paint_state_ = false;

	const std::vector<int> font_size_list = { 3,4,5,6,7,8 };
	std::vector<double> text_area_min_list = {};
	std::vector<double> text_area_max_list = {};

	void DrawGrid(QPainter& paint);
	void DrawFrame(QPainter& paint);
	void DrawLayout(QPainter& painter);
	void DrawLeafNode(QPainter& paint, CCombineTreeNode* node);
	void FillText(QPainter& paint);
	void FillImg(QPainter& paint);
	static bool CombineNodeCompareX(CCombineTreeNode* node_one, CCombineTreeNode* node_two);
	static bool CombineNodeCompareY(CCombineTreeNode* a, CCombineTreeNode* b);

	std::vector<CCombineTreeNode*> leaf_list_;
	CCombineTreeNode* editleaf;//选中的编辑节点
	QPoint lefttop, leftbottom, righttop, rightbottom;//矩形的四个角点
	enum EditMode{
		Default,
		Mov_lefttop,//缩放左上角点
		Mov_leftbottom,//缩放左下角点
		Mov_righttop,//缩放右上角点
		Mov_rightbottom,//缩放右下角点
		Mov_center      //移动元素矩形
	}editmode;

	
};
