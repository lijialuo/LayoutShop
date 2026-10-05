#include "OutputWidget.h"
#include <QGraphicsRectItem>

#include <iostream>
#include <qpainter.h>
#include <qwidget.h>
#include <stack>
#include<QMouseEvent>

OutputWidget::OutputWidget()
{
	layout_tree_ = nullptr;
	editleaf = nullptr;
	
}

OutputWidget::~OutputWidget()
{}

void OutputWidget::paintEvent(QPaintEvent* event)
{
	if(layout_tree_ == nullptr)
	{
		return;
	}

	if (isedit == true)//若编辑状态，设置鼠标自动跟踪
		setMouseTracking(true);
	else//否则取消自动跟踪
		setMouseTracking(false);
	QColor shape_color = QColor(45, 131, 209, 100);
	QPainter paint(this);
	DrawLayout(paint);
	//paint_state_ = false;
}


void OutputWidget::DrawFrame(QPainter& paint)
{
	paint.save();
	paint.setPen(Qt::white);
	paint.setBrush(QBrush(Qt::white));
	QRect this_rect = this->rect();
	paint.drawRect(this_rect);
	paint.restore();
}

void OutputWidget::DrawLayout(QPainter& painter)
{
	leaf_list_.clear();

	painter.save();

	if (layout_tree_)
	{
		std::queue<CCombineTreeNode*> temp_queue;

		temp_queue.push(layout_tree_);
		while (!temp_queue.empty())
		{
			CCombineTreeNode* node = temp_queue.front();
			temp_queue.pop();

			if (node->children.size() == 0)
			{
				DrawLeafNode(painter, node);
			}
			for (auto iter = node->children.begin(); iter != node->children.end(); iter++)
				temp_queue.push(*iter);
		}
		FillImg(painter);
		FillText(painter);

		auto rect = QRect(layout_tree_->x + move, layout_tree_->y + move, layout_tree_->width, layout_tree_->height);
		painter.setBrush(Qt::white);
		painter.drawRect(rect);

		QColor shape_color = QColor(45, 131, 209, 160);

		QPen thisPen;
		thisPen.setBrush(Qt::black);
		thisPen.setStyle(Qt::SolidLine);
		painter.setPen(thisPen);
		painter.setBrush(shape_color);
		//编辑时刻的绘制
		if (isedit == true)
		{
			if (editleaf !=nullptr)
			{
				QRect rect = QRect(editleaf->x + move, editleaf->y + move, editleaf->width, editleaf->height);
				QPen thisPen;
				thisPen.setWidth(10);
				thisPen.setColor(QColor(255, 0, 0));
				painter.setPen(thisPen);
				painter.drawPoints(rect);
				printf("w\n");

			}
		}
	}
	painter.restore();
}

void OutputWidget::DrawLeafNode(QPainter& paint, CCombineTreeNode* node)
{
	//for better visualization, giving a displacement
	QColor label_color[4] = {
		QColor(150, 150, 150, 30),  // padding
		QColor(45, 131, 109, 220),  // picture
		QColor(75, 151, 209, 100),  // text
		QColor(120, 120, 120, 100)  // title
	};

	QRect rect;
	QColor text_color;
	QPen thisPen;
	thisPen.setWidth(0);
	thisPen.setColor(QColor(255, 255, 255, 0));
	paint.setPen(thisPen);
	int font_size = 0;
	rect = QRect(node->x + move, node->y + move, node->width, node->height);
	leaf_list_.push_back(node);
	paint.setBrush(Qt::white);
	if (node->node_present_type == NODE_PRESENT_TYPE::PADDING)
	{
		paint.setBrush(label_color[0]);
		paint.drawRect(rect);
	}
	else if (node->node_present_type == NODE_PRESENT_TYPE::PICTURE)
	{
		//paint.setBrush(QColor(255, 255, 255));
		paint.drawRect(rect);
		
	}
	else if (node->node_present_type == NODE_PRESENT_TYPE::TEXT)
	{
		QRect text_rect;
		//int count = 0;
		//double text_width = node->width;
		//double text_x = rect.x() + 5;
		//paint.setBrush(QColor(255, 255, 255, 0));
		paint.drawRect(rect);
	}
	else if (node->node_present_type == NODE_PRESENT_TYPE::TITLE)
	{
		font_size = 18;
		//std::cout << "font_size::: "<<font_size << std::endl;
		QFont font = paint.font();
		font.setPixelSize(font_size);
		font.setBold(true);
		font.setFamily("Arial");
		paint.setFont(font);
		//paint.setBrush(QColor(255, 255, 255, 0));
		paint.drawRect(rect);
		text_color = QColor(30, 30, 30);
		paint.setPen(text_color);
		paint.drawText(rect, Qt::AlignCenter | Qt::AlignVCenter | Qt::TextWordWrap, article_.title_.data());
	}
	else
	{
		std::cout << "When drawing, do not have this label" << std::endl;
	}

	return;
}

void OutputWidget::FillText(QPainter& paint)
{

	std::stack<CCombineTreeNode*> node_stack;
	size_t text_para_index = 0;
	size_t text_body_index = 0;
	int row_space = 1;
	QFont font;
	//int font_size;
	//const int row_space = 1;
	//font.setPixelSize(font_size);
	double text_area = 0.0;

	node_stack.push(layout_tree_);
	while (!node_stack.empty())
	{
		auto node = node_stack.top();
		node_stack.pop();
		if (node->node_present_type == NODE_PRESENT_TYPE::TEXT)
		{
			text_area += node->width * node->height;
		}
		for (int i = node->children.size() - 1; i >= 0; i--)
		{
			node_stack.push(node->children[i]);
		}
	}
	while (!node_stack.empty())
	{
		node_stack.pop();
	}

	for(int i=font_size_list.size() - 1; i >= 0; --i)
	{
		if(text_area >= text_area_min_list[i] )
		{
			font.setPointSize(font_size_list[i]);
			break;
		}
	}

	if (text_area < text_area_min_list[0]) font.setPointSize(font_size_list[0]);

	//std::cout << "font size : " << font.pointSize() << std::endl;
	font.setFamily("Courier New");
	//font.setPointSize(font_size);

	QFontMetrics font_metrics(font);
	paint.setFont(font);
	paint.setPen(Qt::black);
	//dfs
	node_stack.push(layout_tree_);
	while (!node_stack.empty() && text_para_index < article_.text_list_.size())
	{
		auto node = node_stack.top();
		node_stack.pop();
		if (node->node_present_type == NODE_PRESENT_TYPE::TEXT)
		{
			int words_per_row;
			words_per_row = node->width / font_metrics.averageCharWidth();
			double x = node->x + move + static_cast<double>(static_cast<int>(node->width) % font_metrics.averageCharWidth()) / 2;
			int row = static_cast<int>(node->height) / (font_metrics.ascent() + font_metrics.descent() + row_space);
			double y = node->y + move + static_cast<double>(static_cast<int>(node->height) % (font_metrics.ascent() + font_metrics.descent() + row_space)) / 2;
			for (int row_idx = 0; row_idx < row; row_idx++, y += font_metrics.ascent() + font_metrics.descent() + row_space)
			{
				if (article_.text_list_[text_para_index][text_body_index] == ' ')
				{
					if (text_body_index < article_.text_list_[text_para_index].size() - 1)
					{
						text_body_index++;
					}
					else
					{
						text_para_index++;
						text_body_index = 0;
						if (text_para_index >= article_.text_list.size()) break;
					}
				}
				if (text_body_index + words_per_row < article_.text_list[text_para_index].size())
				{
					paint.drawText(x, y + font_metrics.ascent(), article_.text_list[text_para_index].substr(text_body_index, words_per_row).data());
					text_body_index += words_per_row;
				}
				else
				{
					paint.drawText(x, y + font_metrics.ascent(), article_.text_list[text_para_index].substr(text_body_index, article_.text_list[text_para_index].size() - text_body_index).data());
					text_para_index++;
					text_body_index = 0;
					if (row_idx < row - 1)
					{
						row_idx++;
						y += font_metrics.ascent() + font_metrics.descent() + row_space;
					}
				}
				if (text_para_index >= article_.text_list.size()) break;
			}
		}
		for (int i = node->children.size() - 1; i >= 0; i--)
		{
			node_stack.push(node->children[i]);
		}
	}
	while (!node_stack.empty())
	{
		node_stack.pop();
	}
}

void OutputWidget::FillImg(QPainter& paint)
{


	std::stack<CCombineTreeNode*> node_stack;
	//size_t img_index = 0;
	//dfs
	node_stack.push(layout_tree_);
	while (!node_stack.empty())
	{
		auto node = node_stack.top();
		node_stack.pop();
		if (node->node_present_type == NODE_PRESENT_TYPE::PICTURE)
		{
			QRect rect(node->x + move, node->y + move, node->width, node->height);
			QPixmap pixmap;
			int img_index = node->corres_img_idx;
			pixmap.load(QString::fromStdString(article_.img_list[img_index].path));
			double rect_ratio = static_cast<double>(rect.width()) / rect.height();
			double img_ratio = article_.img_list[img_index].ratio;
			QRect draw_rect;
			if (img_ratio > rect_ratio)
			{
				double draw_width, draw_height;
				draw_width = rect.width();
				draw_height = rect.width() * 1.0 / img_ratio;
				draw_rect.setX(rect.x());
				draw_rect.setY(rect.y() + static_cast<double>(rect.height() - draw_height) / 2);
				draw_rect.setWidth(draw_width);
				draw_rect.setHeight(draw_height);
			}
			else
			{
				double draw_width, draw_height;
				draw_height = rect.height();
				draw_width = rect.height() * img_ratio;
				draw_rect.setY(rect.y());
				draw_rect.setX(rect.x() + static_cast<double>(rect.width() - draw_width) / 2);
				draw_rect.setHeight(draw_height);
				draw_rect.setWidth(draw_width);
			}
			qreal pixel_ratio = paint.device()->devicePixelRatioF(); // 获取不同显示器的分辨率比例，防止不同分辨率下图片失真
			pixmap = pixmap.scaled(QSize(draw_rect.width() * pixel_ratio, draw_rect.height() * pixel_ratio)
				, Qt::KeepAspectRatio, Qt::SmoothTransformation);//缩放图片到当前分辨率下的显示大小
			paint.drawPixmap(draw_rect, pixmap);
		}
		for (int i = node->children.size() - 1; i >= 0; i--)
		{
			node_stack.push(node->children[i]);
		}
	}
	while (!node_stack.empty())
	{
		node_stack.pop();
	}
}

void OutputWidget::DrawGrid(QPainter& paint)
{
	paint.save();
	QPen pen;
	pen.setWidth(1);

	QColor c(0, 0, 0, 70);

	pen.setColor(c);

	int grid_width = 100;

	paint.setPen(pen);

	paint.setRenderHint(QPainter::Antialiasing, false);
	QRect this_rect = this->rect();

	int total_width = this_rect.width();
	int total_height = this_rect.height();

	int start_point_ = 0;

	while (start_point_ <= total_height)
	{
		QPoint p[2];
		p[0].setX(0);
		p[1].setX(total_width);
		p[0].setY(start_point_);
		p[1].setY(start_point_);

		paint.drawLine(p[0], p[1]);
		start_point_ += grid_width;
	}

	start_point_ = 0;

	while (start_point_ <= total_width)
	{
		QPoint p[2];
		p[0].setX(start_point_);
		p[1].setX(start_point_);

		p[0].setY(0);
		p[1].setY(total_height);

		paint.drawLine(p[0], p[1]);
		start_point_ += grid_width;
	}

	paint.restore();
}
void OutputWidget::SetLayoutTree(CCombineTreeNode* tree)
{
	layout_tree_ = tree;
}
void OutputWidget::SetArticle(Article article)
{
	article_ = article;
	int letter_count = 0;
	QFont font;
	//const int font_size = 4;//12;
	const int row_space = 1;//3;
	//font.setPixelSize(font_size);
	font.setFamily("Courier New");

	for (size_t i = 0; i < article_.text_list.size(); ++i)
	{
		letter_count += article_.text_list[i].size();
	}
	//text_area_max = (letter_count + 300 * article_.text_list.size()) * font_metrics.averageCharWidth() * (font_metrics.ascent() + font_metrics.descent() + row_space);
	//text_area_min = (letter_count + 30 * article_.text_list.size()) * font_metrics.averageCharWidth() * (font_metrics.ascent() + font_metrics.descent() + row_space);
	text_area_max_list.clear();
	text_area_min_list.clear();
	for (int i = 0; i < font_size_list.size(); ++i)
	{
		font.setPointSize(font_size_list[i]);
		QFontMetrics font_metrics(font);
		text_area_max_list.push_back((letter_count + 300 * article_.text_list.size()) * font_metrics.averageCharWidth() * (font_metrics.ascent() + font_metrics.descent() + row_space));
		text_area_min_list.push_back((letter_count + 30 * article_.text_list.size()) * font_metrics.averageCharWidth() * (font_metrics.ascent() + font_metrics.descent() + row_space));
	}
}

void OutputWidget::SetPaintState(bool state)
{
	paint_state_ = state;
}

bool OutputWidget::CombineNodeCompareX(CCombineTreeNode* node_one, CCombineTreeNode* node_two)
{
	return node_one->x < node_two->x;
}

bool OutputWidget::CombineNodeCompareY(CCombineTreeNode* a, CCombineTreeNode* b)
{
	return a->y < b->y;
}

void OutputWidget::mousePressEvent(QMouseEvent* event)	//鼠标击发响应函数
{
	//绘制编辑时刻
	update();
	if (isedit == true)
	{
		//确定选中矩形
		for (int i = 0; i < leaf_list_.size(); i++)
		{
			QRect rect = QRect(leaf_list_[i]->x + move, leaf_list_[i]->y + move, leaf_list_[i]->width, leaf_list_[i]->height);
			if (rect.contains(event->pos()))
			{
				editleaf = leaf_list_[i];
				printf("%d\n", i);			
			}
		}		
		QRect rect = QRect(editleaf->x + move, editleaf->y + move, editleaf->width, editleaf->height);
		lefttop = rect.topLeft();
		leftbottom = rect.bottomLeft();
		righttop = rect.topRight();
		rightbottom = rect.bottomRight();

		if (abs(lefttop.x() - event->pos().x()) <= 3 && abs(lefttop.y() - event->pos().y()) <= 3)
			editmode = Mov_lefttop;
		if (abs(leftbottom.x() - event->pos().x()) <= 3 && abs(leftbottom.y() - event->pos().y()) <= 3)
			editmode = Mov_leftbottom;
		if (abs(righttop.x() - event->pos().x()) <= 3 && abs(righttop.y() - event->pos().y()) <= 3)
			editmode = Mov_righttop;
		if (abs(rightbottom.x() - event->pos().x()) <= 3 && abs(rightbottom.y() - event->pos().y()) <= 3)
			editmode = Mov_rightbottom;
	}
}
void OutputWidget::mouseMoveEvent(QMouseEvent* event)	//鼠标移动响应函数
{
	update();
	if (isedit == true && editleaf != nullptr)
	{
		if (abs(lefttop.x() - event->pos().x()) <= 3 && abs(lefttop.y() - event->pos().y()) <= 3)
			setCursor(Qt::SizeFDiagCursor);
		else if (abs(leftbottom.x() - event->pos().x()) <= 3 && abs(leftbottom.y() - event->pos().y()) <= 3)
			setCursor(Qt::SizeBDiagCursor);
		else if (abs(righttop.x() - event->pos().x()) <= 3 && abs(righttop.y() - event->pos().y()) <= 3)
			setCursor(Qt::SizeBDiagCursor);
		else if (abs(rightbottom.x() - event->pos().x()) <= 3 && abs(rightbottom.y() - event->pos().y()) <= 3)
			setCursor(Qt::SizeFDiagCursor);
		else
			setCursor(Qt::ArrowCursor);

		//QRect rect = QRect(editleaf->x + move, editleaf->y + move, editleaf->width, editleaf->height);
		if (editmode == Mov_lefttop)
		{
			// 选中左上角点
			// edit.y+move=event.pos.y
			// edit.x+move=event.pos.x
			editleaf->x = event->pos().x()- move;
			editleaf->y = event->pos().y() - move;
			editleaf->width = righttop.x() - editleaf->x - move;
			editleaf->height = leftbottom.y() - editleaf->y - move;

		}
		else if (editmode == Mov_leftbottom)
		{
			// 选中左下角点
			//edit.x+move=event.pos.x
			//edit.y+move+height=event.pos.y
			// 
			//width=left.x-edit.x
			editleaf->x = event->pos().x() - move;
			editleaf->width = rightbottom.x() - editleaf->x - move;
			editleaf->height = event->pos().y()-editleaf->y - move;
		}
		else if (editmode == Mov_righttop)
		{
			editleaf->y = event->pos().y() - move;
			editleaf->width = event->pos().x() - lefttop.x();
			editleaf->height = rightbottom.y() - event->pos().y();
		}
		else if (editmode == Mov_rightbottom)
		{
			// 选中右下角点
			// edit.y+move+edit.hight=event.pos.y
			// edit.x+move+edit.width=event.pos.x
			editleaf->height = event->pos().y() - move - editleaf->y;
			editleaf->width = event->pos().x() - move - editleaf->x;
		}
	}


}
void OutputWidget::mouseReleaseEvent(QMouseEvent* event)	//鼠标释放响应函数
{
	update();
	editmode = Default;

}
