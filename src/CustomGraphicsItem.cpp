#include "CustomGraphicsItem.h"

#include <iostream>
#include <qcursor.h>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QSvgRenderer>
#include <qpainter.h>
#include <ui_LayoutShop.h>
#include <QGraphicsProxyWidget>

#include "CustomGraphicsScene.h"


CustomGraphicsItem::CustomGraphicsItem()
{
	//this->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
	this->setAcceptHoverEvents(true);
}

void CustomGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	if(dynamic_cast<CustomGraphicsScene *>(this->scene())->GetPaintType() == CustomGraphicsScene::PAINT_TYPE::CONTENT)
	{
		double width = this->rect().width();
		double height = this->rect().height();
		QPen pen;
		pen.setWidth(0);
		pen.setColor(QColor(255, 255, 255, 0));
		QBrush brush;
		brush.setColor(Qt::white);
		brush.setStyle(Qt::SolidPattern);
		painter->setBrush(brush);
		painter->setPen(pen);
		painter->drawRect(0, 0, width, height);
        auto text_font = dynamic_cast<CustomGraphicsScene*>(this->scene())->getTextFont();
		if (node_type_ == NODE_PRESENT_TYPE::TEXT)
		{
			painter->setFont(text_font);
			painter->setPen(Qt::black);
			for (int i = 0; i < start_point_list_.size(); ++i)
			{
				auto start_point = start_point_list_[i];
				painter->drawText(start_point.first, start_point.second, text_list_[i]);
			}
           // painter->setBrush()
		}
		else if (node_type_ == NODE_PRESENT_TYPE::PICTURE)
		{

			double img_ratio =  static_cast<double>(pixmap_.width()) / pixmap_.height();
			double rect_ratio = this->rect().width() / this->rect().height();

			QRectF source_rect;
			QRectF draw_rect = this->rect();

            if (img_ratio >= rect_ratio)
            {
                double draw_width, draw_height;
                draw_height = pixmap_.height();
                draw_width = draw_height * rect_ratio;

                source_rect.setY(0);
                source_rect.setX((pixmap_.width() - draw_width) / 2);
                source_rect.setHeight(draw_height);
                source_rect.setWidth(draw_width);

            }
            else
            {
                double draw_width, draw_height;
                draw_width = pixmap_.width();
                draw_height = draw_width / rect_ratio;
                source_rect.setX(0);
                source_rect.setY((pixmap_.height() - draw_height) / 2);
                source_rect.setHeight(draw_height);
                source_rect.setWidth(draw_width);
            }
            //qreal pixel_ratio = painter->device()->devicePixelRatioF();
            //pixmap_ = pixmap_.scaled(QSize(pixmap_.width() * pixel_ratio, pixmap_.height() * pixel_ratio)
            //, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            //painter->drawPixmap(draw_rect, pixmap_, source_rect);
            QImage image = pixmap_.toImage();
            QPixmap show_pixmap;
            show_pixmap = QPixmap::fromImage(image.copy(source_rect.x(),source_rect.y(), source_rect.width() ,source_rect.height()));

            //pixmap_ = pixmap_.copy(source_rect.x(),source_rect.y(), source_rect.width() ,source_rect.height());
            show_pixmap  = show_pixmap.scaled(draw_rect.width() ,draw_rect.height(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
            painter->drawPixmap(draw_rect.x(),draw_rect.y(), draw_rect.width() ,draw_rect.height(),show_pixmap);
        }
		else if (node_type_ == NODE_PRESENT_TYPE::TITLE)
		{
			QPen pen;
            Qt::AlignmentFlag h_align_flag, v_align_flag;
			auto title_font = dynamic_cast<CustomGraphicsScene*>(this->scene())->getTitleFont();
            auto h_align = dynamic_cast<CustomGraphicsScene*>(this->scene())->getTitleAlignH();
            auto v_align = dynamic_cast<CustomGraphicsScene*>(this->scene())->getTitleAlignV();
            auto title_rotate = dynamic_cast<CustomGraphicsScene*>(this->scene())->getTitleRotateValue();
            pen.setColor(Qt::black);
            painter->setPen(pen);
			painter->setFont(title_font);

            if(title_rotate == CustomGraphicsScene::ROTATE_0)
            {
                if(h_align == CustomGraphicsScene::LEFT_ALIGN)
                {
                    h_align_flag = Qt::AlignLeft;
                }
                else if(h_align == CustomGraphicsScene::CENTER_ALIGN)
                {
                    h_align_flag = Qt::AlignHCenter;
                }
                else if(h_align == CustomGraphicsScene::RIGHT_ALIGN)
                {
                    h_align_flag = Qt::AlignRight;
                }

                if(v_align == CustomGraphicsScene::TOP_ALIGN)
                {
                    v_align_flag = Qt::AlignTop;
                }
                else if(v_align == CustomGraphicsScene::MIDDLE_ALIGN)
                {
                    v_align_flag = Qt::AlignVCenter;
                }
                else if(v_align == CustomGraphicsScene::BOTTOM_ALIGN)
                {
                    v_align_flag = Qt::AlignBottom;
                }
                painter->drawText(QRect(0, 0, width, height), h_align_flag | v_align_flag | Qt::TextWordWrap,title_);
            }
            else if(title_rotate == CustomGraphicsScene::ROTATE_90)
            {
                if(h_align == CustomGraphicsScene::LEFT_ALIGN)
                {
                    v_align_flag = Qt::AlignBottom;
                }
                else if(h_align == CustomGraphicsScene::CENTER_ALIGN)
                {
                    v_align_flag = Qt::AlignVCenter;
                }
                else if(h_align == CustomGraphicsScene::RIGHT_ALIGN)
                {
                    v_align_flag = Qt::AlignTop;
                }

                if(v_align == CustomGraphicsScene::TOP_ALIGN)
                {
                    h_align_flag = Qt::AlignLeft;
                }
                else if(v_align == CustomGraphicsScene::MIDDLE_ALIGN)
                {
                    h_align_flag = Qt::AlignHCenter;
                }
                else if(v_align == CustomGraphicsScene::BOTTOM_ALIGN)
                {
                    h_align_flag = Qt::AlignRight;
                }
                painter->translate(width,0);
                painter->rotate(90);
                painter->drawText(QRect(0, 0, height, width), h_align_flag | v_align_flag | Qt::TextWordWrap,title_);
                painter->rotate(270);
                painter->translate(-width,0);
            }
            else if(title_rotate == CustomGraphicsScene::ROTATE_180)
            {
                if(h_align == CustomGraphicsScene::LEFT_ALIGN)
                {
                    h_align_flag = Qt::AlignRight;
                }
                else if(h_align == CustomGraphicsScene::CENTER_ALIGN)
                {
                    h_align_flag = Qt::AlignHCenter;
                }
                else if(h_align == CustomGraphicsScene::RIGHT_ALIGN)
                {
                    h_align_flag = Qt::AlignLeft;
                }

                if(v_align == CustomGraphicsScene::TOP_ALIGN)
                {
                    v_align_flag = Qt::AlignBottom;
                }
                else if(v_align == CustomGraphicsScene::MIDDLE_ALIGN)
                {
                    v_align_flag = Qt::AlignVCenter;
                }
                else if(v_align == CustomGraphicsScene::BOTTOM_ALIGN)
                {
                    v_align_flag = Qt::AlignTop;
                }
                painter->translate(width, height);
                painter->rotate(180);
                painter->drawText(QRect(0, 0, width, height), h_align_flag | v_align_flag | Qt::TextWordWrap,title_);
                painter->rotate(180);
                painter->translate(-width,-height);
            }
            else if(title_rotate == CustomGraphicsScene::ROTATE_270)
            {
                if(h_align == CustomGraphicsScene::LEFT_ALIGN)
                {
                    v_align_flag = Qt::AlignTop;
                }
                else if(h_align == CustomGraphicsScene::CENTER_ALIGN)
                {
                    v_align_flag = Qt::AlignVCenter;
                }
                else if(h_align == CustomGraphicsScene::RIGHT_ALIGN)
                {
                    v_align_flag = Qt::AlignBottom;
                }

                if(v_align == CustomGraphicsScene::TOP_ALIGN)
                {
                    h_align_flag = Qt::AlignRight;
                }
                else if(v_align == CustomGraphicsScene::MIDDLE_ALIGN)
                {
                    h_align_flag = Qt::AlignHCenter;
                }
                else if(v_align == CustomGraphicsScene::BOTTOM_ALIGN)
                {
                    h_align_flag = Qt::AlignLeft;
                }
                painter->translate(0,height);
                painter->rotate(-90);
                painter->drawText(QRect(0, 0, height, width), h_align_flag | v_align_flag | Qt::TextWordWrap,title_);
                painter->rotate(90);
                painter->translate(0,-height);
            }


//            if(height/width >= 2.0)
//            {
//                painter->translate(width,0);
//                painter->rotate(90);
//                painter->drawText(QRect(0, 0, height, width), Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextWordWrap,title_);
//                //painter->resetTransform();
//                painter->rotate(270);
//                painter->translate(-width,0);
//            }
//			//title_item->setDefaultTextColor(QColor(30, 30, 30));
//			else
		}
	}
	else if(dynamic_cast<CustomGraphicsScene *>(this->scene())->GetPaintType() == CustomGraphicsScene::PAINT_TYPE::NO_CONTENT)
	{
		double width = this->rect().width();
		double height = this->rect().height();
		QPen pen;
		pen.setWidth(0);
		pen.setColor(QColor(255, 255, 255, 0));
		QBrush brush;
		brush.setColor(LABEL_COLOR_[node_type_]);
		brush.setStyle(Qt::SolidPattern);
		painter->setBrush(brush);
		painter->setPen(pen);
		painter->drawRect(0,0,width,height);

		QString icon_address = LAYOUT_ICON_[node_type_];
		if (icon_address != nullptr)
		{
			QSvgRenderer icon;
			icon.load(icon_address);
			if (width < 90 || height < 90)
			{
				icon.render(painter, QRectF((width - 40) / 2, (height - 40) / 2, 40, 40));
			}
			else
			{
				icon.render(painter, QRectF((width - 80) / 2, (height - 80) / 2, 80, 80));
			}
		}
	}
    else if(dynamic_cast<CustomGraphicsScene *>(this->scene())->GetPaintType() == CustomGraphicsScene::PAINT_TYPE::SCORE)
    {
        double width = this->rect().width();
        double height = this->rect().height();
        QPen pen;
        pen.setWidth(0);
        pen.setColor(QColor(255, 255, 255, 0));
        QBrush brush;
        brush.setColor(LABEL_SCORE_COLOR_[node_type_]);
        brush.setStyle(Qt::SolidPattern);
        painter->setBrush(brush);
        painter->setPen(pen);
        painter->drawRect(0,0,width,height);
    }
    if (size_handle_) DrawSizeHandleRects(painter);
//    if(dynamic_cast<CustomGraphicsScene*> (this->scene())->GetEditType() == CustomGraphicsScene::HORIZONTAL_ALIGN
//       &&	node_type_ != NODE_PRESENT_TYPE::NONLABEL)
//    {
//        DrawHorizontalRects(painter);
//        auto align_pair_list = this->tree_node_->align_pair_list;
//        for(int i = 0; i < align_pair_list.size();++i)
//        {
//            auto align_pair = align_pair_list[i];
//            if(align_pair->align_type == CCombineTreeNode::ALIGN_TYPE::H)
//            {
//                QLine line;
//                QPoint q1, q2;
//                if(align_pair->source_point_idx % 2 == 0)   q1.setX(align_pair->source_node->x);
//                else q1.setX(align_pair->source_node->x + align_pair->source_node->width);
//                if(align_pair->source_point_idx <= 1) q1.setY(align_pair->source_node->y);
//                else if(align_pair->source_point_idx <= 3) q1.setY(align_pair->source_node->y + align_pair->source_node->height / 2);
//                else if(align_pair->source_point_idx <= 5) q1.setY(align_pair->source_node->y + align_pair->source_node->height);
//
//                if(align_pair->target_point_idx % 2 == 0)   q2.setX(align_pair->target_node->x);
//                else q2.setX(align_pair->target_node->x + align_pair->target_node->width);
//                if(align_pair->target_point_idx <= 1) q2.setY(align_pair->target_node->y);
//                else if(align_pair->target_point_idx <= 3) q2.setY(align_pair->target_node->y + align_pair->target_node->height / 2);
//                else if(align_pair->target_point_idx <= 5) q2.setY(align_pair->target_node->y + align_pair->target_node->height);
//
//                q1.setX(q1.x() - this->scenePos().x());
//                q1.setY(q1.y() - this->scenePos().y());
//                q2.setX(q2.x() - this->scenePos().x());
//                q2.setY(q2.y() - this->scenePos().y());
//                line.setP1(q1);
//                line.setP2(q2);
//                QPen pen;
//                pen.setColor(Qt::black);
//                pen.setStyle(Qt::DashLine);
//                painter->setPen(pen);
//                painter->drawLine(line);
//            }
//            else if(align_pair->align_type == CCombineTreeNode::ALIGN_TYPE::V)
//            {
//
//            }
//        }
//    }
//
//    if(dynamic_cast<CustomGraphicsScene*> (this->scene())->GetEditType() == CustomGraphicsScene::VERTICAL_ALIGN
//       &&	node_type_ != NODE_PRESENT_TYPE::NONLABEL)   DrawVerticalRects(painter);
    if(picture_show_lock_)
    {
        //QPainter painter(this);
        painter->setBrush(Qt::white);
        painter->setPen(LABEL_COLOR_[2]);
        painter->drawEllipse(picture_lock_icon_rect_);
        QSvgRenderer icon;
        if(this->tree_node_->picture_lock) icon.load(LOCK_ICON_);
        else icon.load(UNLOCK_ICON_);
        int actual_icon_size = picture_lock_icon_rect_.width() - 4;
        icon.render(painter, QRectF( - actual_icon_size / 2, - actual_icon_size / 2, actual_icon_size, actual_icon_size));
    }

}

void CustomGraphicsItem::UpdateSizeHandleRects()
{
	double x, y;
	double half_len = 5;
	double full_len = 2 * half_len;

    if(this->node_type_ != NODE_PRESENT_TYPE::NONLABEL)
    {
        size_handle_rects_.clear();

        x = 0;
        y = 0;
        size_handle_rects_.push_back(QRect(x - half_len, y - half_len, full_len, full_len));

        x = this->rect().width() / 2 ;
        y = 0;
        size_handle_rects_.push_back(QRect(x - half_len, y - half_len, full_len, full_len));

        x = this->rect().width();
        y = 0;
        size_handle_rects_.push_back(QRect(x - half_len, y - half_len, full_len, full_len));

        x = 0;
        y = this->rect().height()  / 2;
        size_handle_rects_.push_back(QRect(x - half_len, y - half_len, full_len, full_len));

        x = this->rect().width() ;
        y = this->rect().height() / 2 ;
        size_handle_rects_.push_back(QRect(x - half_len, y - half_len, full_len, full_len));

        x = 0;
        y = this->rect().height();
        size_handle_rects_.push_back(QRect(x - half_len, y - half_len, full_len, full_len));

        x = this->rect().width() / 2;
        y = this->rect().height();
        size_handle_rects_.push_back(QRect(x - half_len, y - half_len, full_len, full_len));

        x = this->rect().width() ;
        y = this->rect().height();
        size_handle_rects_.push_back(QRect(x - half_len, y - half_len, full_len, full_len));
    }

}

void CustomGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
	if(dynamic_cast<CustomGraphicsScene*> (this->scene())->GetEditType() == CustomGraphicsScene::SIZE_ADJUST_NODE
		&&	node_type_ != NODE_PRESENT_TYPE::NONLABEL)
	{
        size_handle_ = true;
        this->update();
	}
    if(dynamic_cast<CustomGraphicsScene*> (this->scene())->GetEditType() == CustomGraphicsScene::EDIT_TYPE::NONE
       &&	node_type_ == NODE_PRESENT_TYPE::PICTURE)
    {
        picture_show_lock_ = true;
        this->update();
    }
}

void CustomGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
	if (dynamic_cast<CustomGraphicsScene*> (this->scene())->GetEditType() == CustomGraphicsScene::SIZE_ADJUST_NODE
		&& node_type_ != NODE_PRESENT_TYPE::NONLABEL)
	{
		size_handle_ = false;
		this->update();
	}
    if(dynamic_cast<CustomGraphicsScene*> (this->scene())->GetEditType() == CustomGraphicsScene::EDIT_TYPE::NONE
       &&	node_type_ == NODE_PRESENT_TYPE::PICTURE)
    {
        picture_show_lock_ = false;
        this->update();
    }
}

void CustomGraphicsItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
	if (dynamic_cast<CustomGraphicsScene*> (this->scene())->GetEditType() == CustomGraphicsScene::SIZE_ADJUST_NODE
		&& node_type_ != NODE_PRESENT_TYPE::NONLABEL)
	{
		auto point = event->pos();

		bool contain = false;
		for (int i = 0; i < size_handle_rects_.size(); ++i)
		{
			contain = size_handle_rects_[i].contains(point.x(), point.y());
			if (contain)
			{
				this->setCursor(CURSOR_SHAPE_[i]);
				//this->cursor().   setShape(CURSOR_SHAPE_[i]);
				break;
			}
		}
		if (!contain)  this->setCursor(Qt::ArrowCursor);
	}
}

void CustomGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	if (dynamic_cast<CustomGraphicsScene*> (this->scene())->GetEditType() == CustomGraphicsScene::SIZE_ADJUST_NODE
		&& node_type_ != NODE_PRESENT_TYPE::NONLABEL)
	{
		auto point = event->pos();

		bool contain = false;
		for (int i = 0; i < size_handle_rects_.size(); ++i)
		{
			contain = size_handle_rects_[i].contains(point.x(), point.y());
			if (contain)
			{
				size_adjust_direction_ = SIZE_ADJUST_DIRECTION(i);
				//this->cursor().   setShape(CURSOR_SHAPE_[i]);
				auto current_item = this;
				while(current_item != nullptr)
				{
					current_item->setZValue(999);
					current_item = dynamic_cast<CustomGraphicsItem*>(current_item->parentItem());
				}
				//this->setZValue(999);
				this->setOpacity(0.5);
				auto scene = this->scene();
                dynamic_cast<CustomGraphicsScene *>(scene)->SetPaintType(CustomGraphicsScene::NO_CONTENT);
				break;
			}
		}
	}

//    if (dynamic_cast<CustomGraphicsScene*> (this->scene())->GetEditType() == CustomGraphicsScene::HORIZONTAL_ALIGN
//        && node_type_ != NODE_PRESENT_TYPE::NONLABEL)
//    {
//        auto point = event->pos();
//
//        bool contain = false;
//        for (int i = 0; i < horizontal_align_rects_.size(); ++i)
//        {
//            contain = horizontal_align_rects_[i].contains(point.x(), point.y());
//            if (contain)
//            {
//                if(i==0 || i==1)    horizontal_align_edge_ = HORIZONTAL_ALIGN_EDGE::TOP;
//                else if(i==2 || i==3)    horizontal_align_edge_ = HORIZONTAL_ALIGN_EDGE::MIDDLE;
//                else if(i==4 || i==5)    horizontal_align_edge_ = HORIZONTAL_ALIGN_EDGE::BOTTOM;
//                //this->cursor().   setShape(CURSOR_SHAPE_[i]);
//                break;
//            }
//        }
//    }
//
//    if (dynamic_cast<CustomGraphicsScene*> (this->scene())->GetEditType() == CustomGraphicsScene::VERTICAL_ALIGN
//        && node_type_ != NODE_PRESENT_TYPE::NONLABEL)
//    {
//        auto point = event->pos();
//
//        bool contain = false;
//        for (int i = 0; i < vertical_align_rects_.size(); ++i)
//        {
//            contain = vertical_align_rects_[i].contains(point.x(), point.y());
//            if (contain)
//            {
//                if(i==0 || i==3)    vertical_align_edge_ = VERTICAL_ALIGN_EDGE::LEFT;
//                else if(i==1 || i==4)    vertical_align_edge_ = VERTICAL_ALIGN_EDGE::CENTER;
//                else if(i==2 || i==5)    vertical_align_edge_ = VERTICAL_ALIGN_EDGE::RIGHT;
//                //this->cursor().   setShape(CURSOR_SHAPE_[i]);
//                break;
//            }
//        }
//    }

    if (dynamic_cast<CustomGraphicsScene*> (this->scene())->GetEditType() == CustomGraphicsScene::NONE
        && node_type_ == NODE_PRESENT_TYPE::PICTURE)
    {
        if(picture_lock_icon_rect_.contains(event->pos().x(),event->pos().y()))
            this->tree_node_->picture_lock = !this->tree_node_->picture_lock;
        this->update();
    }

}

void CustomGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	if (dynamic_cast<CustomGraphicsScene*> (this->scene())->GetEditType() == CustomGraphicsScene::SIZE_ADJUST_NODE
		&& node_type_ != NODE_PRESENT_TYPE::NONLABEL && size_adjust_direction_ != SIZE_ADJUST_DIRECTION::NONE_DIRECTION)
	{
		size_adjust_direction_ = SIZE_ADJUST_DIRECTION::NONE_DIRECTION;
		emit EditDone();
	}
}

void CustomGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	if (dynamic_cast<CustomGraphicsScene*> (this->scene())->GetEditType() == CustomGraphicsScene::SIZE_ADJUST_NODE
		&& node_type_ != NODE_PRESENT_TYPE::NONLABEL)
	{
		auto last_point = event->lastPos();
		auto point = event->pos();
		double x, y, w, h;
		double x_bias, y_bias;
		double x_map_to_scene, y_map_to_scene;
		x_bias = point.x() - last_point.x();
		y_bias = point.y() - last_point.y();
		x_map_to_scene = this->scenePos().x();
		y_map_to_scene = this->scenePos().y();
		//std::cout << point.x() << " " << point.y() << " ";
		if (size_adjust_direction_ == SIZE_ADJUST_DIRECTION::LEFT_TOP)
		{
			x = this->pos().x() + x_bias;
			y = this->pos().y() + y_bias;
			w = this->rect().width() - x_bias;
			h = this->rect().height() - y_bias;
			x_map_to_scene += x_bias;
			y_map_to_scene += y_bias;
		}
		else if (size_adjust_direction_ == SIZE_ADJUST_DIRECTION::MIDDLE_TOP)
		{
			x = this->pos().x();
			y = this->pos().y() + y_bias;
			w = this->rect().width();
			h = this->rect().height() - y_bias;
			y_map_to_scene += y_bias;
		}
		else if (size_adjust_direction_ == SIZE_ADJUST_DIRECTION::RIGHT_TOP)
		{
			x = this->pos().x();
			y = this->pos().y() + y_bias;
			w = this->rect().width() + x_bias;
			h = this->rect().height() - y_bias;
			y_map_to_scene += y_bias;
		}
		else if (size_adjust_direction_ == SIZE_ADJUST_DIRECTION::LEFT_CENTER)
		{
			x = this->pos().x() + x_bias;
			y = this->pos().y();
			w = this->rect().width() - x_bias;
			h = this->rect().height();
			x_map_to_scene += x_bias;
		}
		else if (size_adjust_direction_ == SIZE_ADJUST_DIRECTION::RIGHT_CENTER)
		{
			x = this->pos().x();
			y = this->pos().y();
			w = this->rect().width() + x_bias;
			h = this->rect().height();
		}
		else if (size_adjust_direction_ == SIZE_ADJUST_DIRECTION::LEFT_BOTTOM)
		{
			x = this->pos().x() + x_bias;
			y = this->pos().y();
			w = this->rect().width() - x_bias;
			h = this->rect().height() + y_bias;
			x_map_to_scene += x_bias;
		}
		else if (size_adjust_direction_ == SIZE_ADJUST_DIRECTION::MIDDLE_BOTTOM)
		{
			x = this->pos().x();
			y = this->pos().y();
			w = this->rect().width();
			h = this->rect().height() + y_bias;
		}
		else if (size_adjust_direction_ == SIZE_ADJUST_DIRECTION::RIGHT_BOTTOM)
		{
			x = this->pos().x();
			y = this->pos().y();
			w = this->rect().width() + x_bias;
			h = this->rect().height() + y_bias;
		}
		if (w < 100 || h < 100) return;
		if (x_map_to_scene < 0 || y_map_to_scene < 0 ||
			x_map_to_scene + w >= this->scene()->width() || y_map_to_scene + h >= this->scene()->height()) return;
		/*std::cout << x_map_to_scene << " " << y_map_to_scene << std::endl;
		std::cout << this->scene()->width() << " " << this->scene()->height() << std::endl;*/
		//
		/*auto  map_to_scene_shape = this->mapToScene(x,y,w,h);
		std::cout << map_to_scene_shape.boundingRect().x() << " " << map_to_scene_shape.boundingRect().y()
		<<" "<< map_to_scene_shape.boundingRect().width()<<" "<< map_to_scene_shape.boundingRect().height() << std::endl;*/
		this->setPos(x, y);
		this->setRect(0, 0, w, h);
		//dynamic_cast<CustomGraphicsScene*>(this->scene());
		//std::cout << this->pos().x() << " " << this->pos().y() << " " << this->rect().width() << " " << this->rect().height() << std::endl;
		this->UpdateSizeHandleRects();
		this->fix_geometry_ = true;
		
	}
}

void CustomGraphicsItem::DrawSizeHandleRects(QPainter* painter)
{
	QPen pen;
	pen.setWidth(0);
	pen.setColor(QColor(255, 255, 255, 0));
	QBrush brush;
	brush.setColor("#29b6f2");			
	brush.setStyle(Qt::SolidPattern);
	painter->setBrush(brush);
	painter->setPen(pen);
	for (auto rect : size_handle_rects_)
    {
        painter->drawRect(rect);
        //std::cout<<rect.x()<< " " << rect.y()<< " "<< rect.width() << " "<< rect.height()<<std::endl;
    }
}

CustomGraphicsItem::~CustomGraphicsItem()
{

}

void CustomGraphicsItem::UpdateHorizontalAlignRects()
{
    double x, y;
    double half_len = 5;
    double full_len = 2 * half_len;

    if(this->node_type_ != NODE_PRESENT_TYPE::NONLABEL)
    {
        horizontal_align_rects_.clear();

        x = 0;
        y = 0;
        horizontal_align_rects_.push_back(QRect(x - half_len, y - half_len, full_len, full_len));

        x = this->rect().width();
        y = 0;
        horizontal_align_rects_.push_back(QRect(x - half_len, y - half_len, full_len, full_len));

        x = 0;
        y = this->rect().height()  / 2;
        horizontal_align_rects_.push_back(QRect(x - half_len, y - half_len, full_len, full_len));

        x = this->rect().width() ;
        y = this->rect().height() / 2 ;
        horizontal_align_rects_.push_back(QRect(x - half_len, y - half_len, full_len, full_len));

        x = 0;
        y = this->rect().height();
        horizontal_align_rects_.push_back(QRect(x - half_len, y - half_len, full_len, full_len));

        x = this->rect().width() ;
        y = this->rect().height();
        horizontal_align_rects_.push_back(QRect(x - half_len, y - half_len, full_len, full_len));
    }

}

void CustomGraphicsItem::UpdateVerticalAlignRects()
{
    double x, y;
    double half_len = 5;
    double full_len = 2 * half_len;

    if(this->node_type_ != NODE_PRESENT_TYPE::NONLABEL)
    {
        vertical_align_rects_.clear();

        x = 0;
        y = 0;
        vertical_align_rects_.push_back(QRect(x - half_len, y - half_len, full_len, full_len));

        x = this->rect().width() / 2 ;
        y = 0;
        vertical_align_rects_.push_back(QRect(x - half_len, y - half_len, full_len, full_len));

        x = this->rect().width();
        y = 0;
        vertical_align_rects_.push_back(QRect(x - half_len, y - half_len, full_len, full_len));

        x = 0;
        y = this->rect().height();
        vertical_align_rects_.push_back(QRect(x - half_len, y - half_len, full_len, full_len));

        x = this->rect().width() / 2;
        y = this->rect().height();
        vertical_align_rects_.push_back(QRect(x - half_len, y - half_len, full_len, full_len));

        x = this->rect().width() ;
        y = this->rect().height();
        vertical_align_rects_.push_back(QRect(x - half_len, y - half_len, full_len, full_len));
    }

}

void CustomGraphicsItem::DrawHorizontalRects(QPainter *painter)
{
    QPen pen;
    pen.setWidth(0);
    pen.setColor(QColor(255, 255, 255, 0));
    QBrush brush;
    brush.setColor("#29b6f2");
    brush.setStyle(Qt::SolidPattern);
    painter->setBrush(brush);
    painter->setPen(pen);
    for (auto rect : horizontal_align_rects_)
    {
        painter->drawRect(rect);
        //std::cout<<rect.x()<< " " << rect.y()<< " "<< rect.width() << " "<< rect.height()<<std::endl;
    }
}

void CustomGraphicsItem::DrawVerticalRects(QPainter *painter)
{
    QPen pen;
    pen.setWidth(0);
    pen.setColor(QColor(255, 255, 255, 0));
    QBrush brush;
    brush.setColor("#29b6f2");
    brush.setStyle(Qt::SolidPattern);
    painter->setBrush(brush);
    painter->setPen(pen);
    for (auto rect : vertical_align_rects_)
    {
        painter->drawRect(rect);
        //std::cout<<rect.x()<< " " << rect.y()<< " "<< rect.width() << " "<< rect.height()<<std::endl;
    }
}