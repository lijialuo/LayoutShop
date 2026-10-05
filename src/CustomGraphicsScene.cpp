#include "CustomGraphicsScene.h"

#include <iostream>

void CustomGraphicsScene::GenerateHelperRects()
{
	for(auto item:this->items())
	{
		auto custom_item = dynamic_cast<CustomGraphicsItem*>(item);
		if(custom_item != nullptr && custom_item->GetTreeNode() != nullptr)
		{
			QRect left_rect, right_rect, up_rect, bottom_rect;
			left_rect = QRect(
				custom_item->scenePos().x() - h_padding_,
				custom_item->scenePos().y(),
				this->h_padding_,
				custom_item->rect().height());
			right_rect = QRect(
				custom_item->scenePos().x() + custom_item->rect().width() ,
				custom_item->scenePos().y(),
				this->h_padding_,
				custom_item->rect().height());
			up_rect = QRect(
				custom_item->scenePos().x(),
				custom_item->scenePos().y() - v_padding_,
				custom_item->rect().width(),
				v_padding_);
			bottom_rect = QRect(
				custom_item->scenePos().x() ,
				custom_item->scenePos().y() + custom_item->rect().height(),
				custom_item->rect().width(),
				v_padding_);

			item_helper_map_[custom_item] = { left_rect,right_rect,up_rect,bottom_rect };

		}
	}

}

void CustomGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	QGraphicsScene::mouseMoveEvent(event);
	if(this->edit_type_ == EDIT_TYPE::MOVE_NODE && dragging_)
	{
		//display the moving item
		double move_item_width = this->move_item_->rect().width();
		double move_item_height = this->move_item_->rect().height();
		auto pos = event->scenePos();
		this->move_item_->setPos(pos.x() - move_item_width / 2, pos.y() - move_item_height / 2);

		//check if any rectangle is covered
		std::vector<QRect> covered_rect_list;
		QRect move_rect = QRect(this->move_item_->scenePos().x(), this->move_item_->scenePos().y(),
			move_item_width, move_item_height);
		for(auto pair : item_helper_map_)
		{
			for(auto rect:pair.second)
			{
				if(rect.intersects(move_rect))
				{
					covered_rect_list.push_back(rect);
				}
			}
		}
		if(!covered_rect_list.empty())
		{
			double min_dist = 99999999999999.0;
			int min_idx = -1;
			for(int i = 0; i < covered_rect_list.size();++i)
			{
				double rect_center_x = covered_rect_list[i].center().x();
				double rect_center_y = covered_rect_list[i].center().y();
				double cursor_x = event->scenePos().x();
				double cursor_y = event->scenePos().y();
				double dist = (cursor_x - rect_center_x) * (cursor_x - rect_center_x) + (cursor_y - rect_center_y) * (cursor_y - rect_center_y);
				if(dist < min_dist)
				{
					min_dist = dist;
					min_idx = i;
				}
			}
			this->target_rect_ = covered_rect_list[min_idx];
			this->target_rect_item_->setRect(this->target_rect_);
			this->target_rect_item_->setVisible(true);
			for(auto pair : item_helper_map_)
			{
				bool found = false;
				for(auto rect:pair.second)
				{
					if (rect == this->target_rect_)
					{
						this->target_item_ = pair.first;
						found = true;
						break;
					}
				}
				if (found) break;
			}
		}
		else
		{
			this->target_rect_item_->setVisible(false);
			this->target_item_ = nullptr;
		}
	}
	else if (this->edit_type_ == EDIT_TYPE::ADD_PADDING || this->edit_type_ == EDIT_TYPE::ADD_TEXT)
	{
		if(event->scenePos().x() >= 0 && event->scenePos().y() >= 0 && event->scenePos().x() < this->width() && event->scenePos().y() < this->height())
		{
			//display the moving item
			if (move_item_ == nullptr)
			{
                origin_paint_type_ = paint_type_;
			    paint_type_ = PAINT_TYPE::NO_CONTENT;
				move_item_ = new CustomGraphicsItem();
				if (this->edit_type_ == EDIT_TYPE::ADD_PADDING)
				{
					move_item_->SetNodeType(PADDING);
				}
				else if (this->edit_type_ == EDIT_TYPE::ADD_TEXT)
				{
					move_item_->SetNodeType(TEXT);
				}
				move_item_->setRect(0, 0, 200, 200);
				move_item_->setZValue(999);
				move_item_->setOpacity(0.5);
				this->addItem(move_item_);
				this->target_rect_item_ = new QGraphicsRectItem();
                QPen pen;
                pen.setWidth(0);
                pen.setColor(QColor(255, 255, 255, 0));
                this->target_rect_item_->setPen(pen);
                //target_rect_item_->setBrush(brush);
                this->target_rect_item_->setBrush(Qt::yellow);
                this->target_rect_item_->setOpacity(0.5);
				this->target_rect_item_->setVisible(false);

				this->addItem(this->target_rect_item_);
				this->target_item_ = nullptr;
			}
			double move_item_width = this->move_item_->rect().width();
			double move_item_height = this->move_item_->rect().height();
			auto pos = event->scenePos();
			this->move_item_->setPos(pos.x() - move_item_width / 2, pos.y() - move_item_height / 2);

			//check if any rectangle is covered
			std::vector<QRect> covered_rect_list;
			QRect move_rect = QRect(this->move_item_->scenePos().x(), this->move_item_->scenePos().y(),
				move_item_width, move_item_height);
			for (auto pair : item_helper_map_)
			{
				for (auto rect : pair.second)
				{
					if (rect.intersects(move_rect))
					{
						covered_rect_list.push_back(rect);
					}
				}
			}
			if (!covered_rect_list.empty())
			{
				double min_dist = 99999999999999.0;
				int min_idx = -1;
				for (int i = 0; i < covered_rect_list.size(); ++i)
				{
					double rect_center_x = covered_rect_list[i].center().x();
					double rect_center_y = covered_rect_list[i].center().y();
					double cursor_x = event->scenePos().x();
					double cursor_y = event->scenePos().y();
					double dist = (cursor_x - rect_center_x) * (cursor_x - rect_center_x) + (cursor_y - rect_center_y) * (cursor_y - rect_center_y);
					if (dist < min_dist)
					{
						min_dist = dist;
						min_idx = i;
					}
				}
				this->target_rect_ = covered_rect_list[min_idx];
				this->target_rect_item_->setRect(this->target_rect_);
				this->target_rect_item_->setVisible(true);
				for (auto pair : item_helper_map_)
				{
					bool found = false;
					for (auto rect : pair.second)
					{
						if (rect == this->target_rect_)
						{
							this->target_item_ = pair.first;
							found = true;
							break;
						}
					}
					if (found) break;
				}
			}
			else
			{
				this->target_rect_item_->setVisible(false);
				this->target_item_ = nullptr;
			}
		}
		else
		{
			if (move_item_ != nullptr)
			{
				this->removeItem(move_item_);
				move_item_ = nullptr;
				delete move_item_;
			}
			if (target_rect_item_ != nullptr)
			{
				this->removeItem(target_rect_item_);
				target_rect_item_ = nullptr;
				delete target_rect_item_;
			}
			if (target_item_ != nullptr) target_item_ = nullptr;
			this->paint_type_ = origin_paint_type_;
		}
	}
    else if (this->edit_type_ == EDIT_TYPE::REMOVE_NODE)
    {
        bool contain = false;
        for (auto item : this->items())
        {
            if (item == delete_rect_item_) continue;
            auto custom_item = dynamic_cast<CustomGraphicsItem*>(item);
            if (custom_item != nullptr &&(custom_item->GetNodeType() == NODE_PRESENT_TYPE::TEXT ||
                custom_item->GetNodeType() == NODE_PRESENT_TYPE::PADDING))
            {
                auto scene_pos = custom_item->scenePos();
                QRectF rect = QRect(scene_pos.x(), scene_pos.y(), custom_item->rect().width(), custom_item->rect().height());
                if (rect.contains(event->scenePos()))
                {
                    if(delete_rect_item_ == nullptr)
                    {
                        delete_rect_item_ = new QGraphicsRectItem();
                        QPen pen;
                        pen.setColor(QColor(255, 255, 255, 0));
                        pen.setWidth(0);
                        delete_rect_item_->setPen(pen);
                        delete_rect_item_->setBrush(QColor("#ffcccb"));
                        delete_rect_item_->setRect(rect);
                        delete_rect_item_->setOpacity(0.5);
                        this->addItem(delete_rect_item_);
                    }
                    contain = true;
                    break;
                }
            }
        }
        if(!contain && delete_rect_item_ != nullptr)
        {
            this->removeItem(delete_rect_item_);
            delete delete_rect_item_;
            delete_rect_item_ = nullptr;
        }
    }

    if(current_line_ != nullptr)
    {
        current_line_->setLine(QLineF(current_line_->line().p1(), event->scenePos()));
        update();
    }

}

void CustomGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	QGraphicsScene::mousePressEvent(event);
	if(this->edit_type_ == EDIT_TYPE::MOVE_NODE && this->source_item_ == nullptr)
	{
        origin_paint_type_ = paint_type_;
        paint_type_ = PAINT_TYPE::NO_CONTENT;
		for (auto item : this->items())
		{
			auto custom_item = dynamic_cast<CustomGraphicsItem*>(item);
			if (custom_item != nullptr && custom_item->GetNodeType() != NODE_PRESENT_TYPE::NONLABEL)
			{
				auto scene_pos = custom_item->scenePos();
				QRectF rect = QRect(scene_pos.x(), scene_pos.y(), custom_item->rect().width(), custom_item->rect().height());
				if (rect.contains(event->scenePos()))
				{
					source_item_ = custom_item;
					move_item_ = new CustomGraphicsItem();
					double move_rect_width = custom_item->rect().width() > 100 ? custom_item->rect().width() : 100;
					double move_rect_height = custom_item->rect().height() > 100 ? custom_item->rect().height() : 100;
					move_item_->setPos(event->scenePos().x() - move_rect_width / 2, event->scenePos().y() - move_rect_height / 2);
					move_item_->setRect(0, 0, move_rect_width, move_rect_height);
					move_item_->SetNodeType(custom_item->GetNodeType());
					move_item_->setOpacity(0.5);
					target_rect_item_ = new QGraphicsRectItem();
					source_item_->setVisible(false);
					target_rect_item_->setVisible(false);
                    QPen pen;
                    pen.setWidth(0);
                    pen.setColor(QColor(255, 255, 255, 0));
                    target_rect_item_->setPen(pen);
                    //target_rect_item_->setBrush(brush);
					target_rect_item_->setBrush(Qt::yellow);
                    target_rect_item_->setOpacity(0.5);
					this->addItem(target_rect_item_);
					this->addItem(move_item_);
					dragging_ = true;
					break;
				}
			}
		}
	}

	else if(this->edit_type_ == EDIT_TYPE::ADD_TEXT || this->edit_type_ == EDIT_TYPE::ADD_PADDING)
	{
		if(this->target_item_!=nullptr)
		{
			emit ReOptimizeReady();
		}
		if (move_item_ != nullptr)
		{
			this->removeItem(move_item_);
			move_item_ = nullptr;
			delete move_item_;
		}
		if (target_rect_item_ != nullptr)
		{
			this->removeItem(target_rect_item_);
			target_rect_item_ = nullptr;
			delete target_rect_item_;
		}
		if (target_item_ != nullptr) target_item_ = nullptr;
        this->paint_type_ = origin_paint_type_;
		
	}
	else if (this->edit_type_ == EDIT_TYPE::REMOVE_NODE )
	{
		for (auto item : this->items())
		{
            if(item == delete_rect_item_) continue;
			auto custom_item = dynamic_cast<CustomGraphicsItem*>(item);
			if (custom_item != nullptr && (custom_item->GetNodeType() == NODE_PRESENT_TYPE::TEXT ||
				custom_item->GetNodeType() == NODE_PRESENT_TYPE::PADDING))
			{
				auto scene_pos = custom_item->scenePos();
				QRectF rect = QRect(scene_pos.x(), scene_pos.y(), custom_item->rect().width(), custom_item->rect().height());
				if (rect.contains(event->scenePos()))
				{
					target_item_ = custom_item;
					emit ReOptimizeReady();
					break;
				}
			}
		}
	}
//    else if (this->edit_type_ == EDIT_TYPE::HORIZONTAL_ALIGN)
//    {
//        auto pos = event->scenePos();
//        if(current_line_ == nullptr)
//        {
//            bool contain = false;
//            for(auto item : this->items())
//            {
//                CustomGraphicsItem* custom_item = dynamic_cast<CustomGraphicsItem*>(item);
//                if(custom_item == nullptr || custom_item->GetNodeType() == NONLABEL) continue;
//                for (int i = 0; i < custom_item->horizontal_align_rects_.size(); ++i)
//                {
//                    QRect rect;
//                    rect.setRect(custom_item->scenePos().x() + custom_item->horizontal_align_rects_[i].x(),
//                                 custom_item->scenePos().y() + custom_item->horizontal_align_rects_[i].y(),
//                                 custom_item->horizontal_align_rects_[i].width(),
//                                 custom_item->horizontal_align_rects_[i].height()) ;
//
//                    contain = rect.contains(pos.x(), pos.y());
//                    if (contain)
//                    {
//                        current_align_pair_ = new CCombineTreeNode::AlignPair;
//                        current_align_pair_->source_node = custom_item->GetTreeNode();
//                        current_align_pair_->source_point_idx = i;
//                        current_align_pair_->align_type = CCombineTreeNode::H;
//                        current_line_ = new QGraphicsLineItem();
//                        current_line_->setLine(pos.x(),pos.y(),pos.x(),pos.y());
//                        this->addItem(current_line_);
//                        break;
//                    }
//                }
//                if(contain) break;
//            }
//        }
//        else
//        {
//            bool contain = false;
//            for(auto item : this->items())
//            {
//                CustomGraphicsItem* custom_item = dynamic_cast<CustomGraphicsItem*>(item);
//                if(custom_item == nullptr || custom_item->GetNodeType() == NONLABEL) continue;
//                for (int i = 0; i < custom_item->horizontal_align_rects_.size(); ++i)
//                {
//                    QRect rect;
//                    rect.setRect(custom_item->scenePos().x() + custom_item->horizontal_align_rects_[i].x(),
//                                 custom_item->scenePos().y() + custom_item->horizontal_align_rects_[i].y(),
//                                 custom_item->horizontal_align_rects_[i].width(),
//                                 custom_item->horizontal_align_rects_[i].height());
//                    contain = rect.contains(pos.x(), pos.y());
//                    if (contain)
//                    {
//                        current_align_pair_->target_node = custom_item->GetTreeNode();
//                        current_align_pair_->target_point_idx = i;
//                        align_pair_list_.push_back(current_align_pair_);
//                        current_align_pair_->source_node->align_pair_list.push_back(current_align_pair_);
//                        //current_line_->source_node->align_line_list.push_back(current_line_);
//                        break;
//                    }
//                }
//                if(contain) break;
//            }
//            if(!contain)
//            {
//                delete this->current_align_pair_;
//            }
//            current_align_pair_ = nullptr;
//            this->removeItem(current_line_);
//
//            delete current_line_;
//            current_line_ = nullptr;
//        }
//    }
}

void CustomGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	QGraphicsScene::mouseReleaseEvent(event);
	if(this->edit_type_ == EDIT_TYPE::MOVE_NODE)
	{
		if (dragging_) dragging_ = false;

		if(this->target_item_ != nullptr &&
			this->source_item_ !=nullptr && 
			this->source_item_ != this->target_item_) 
			emit ReOptimizeReady();

		if (source_item_ != nullptr)
		{
			source_item_->setVisible(true);
			source_item_ = nullptr;
		}

		if (move_item_ != nullptr)
		{
			this->removeItem(move_item_);
			move_item_ = nullptr;
			delete move_item_;
		}
		if (target_rect_item_ != nullptr)
		{
			this->removeItem(target_rect_item_);
			target_rect_item_ = nullptr;
			delete target_rect_item_;
		}
		if (target_item_ != nullptr) target_item_ = nullptr;
        this->paint_type_ = origin_paint_type_;
	}
	
	
}

void CustomGraphicsScene::NodeEditDone()
{
	emit ReOptimizeReady();
}

const QFont &CustomGraphicsScene::getTitleFont() const
{
    return title_font_;
}

void CustomGraphicsScene::setTitleFont(const QFont &titleFont)
{
    title_font_ = titleFont;
}

const QFont &CustomGraphicsScene::getTextFont() const
{
    return text_font_;
}

void CustomGraphicsScene::setTextFont(const QFont &textFont)
{
    text_font_ = textFont;
}

CustomGraphicsScene::TITLE_ROTATE CustomGraphicsScene::getTitleRotateValue() const
{
    return title_rotate_value_;
}

void CustomGraphicsScene::setTitleRotateValue(CustomGraphicsScene::TITLE_ROTATE titleRotateValue)
{
    title_rotate_value_ = titleRotateValue;
}

CustomGraphicsScene::TITLE_ALIGN_H CustomGraphicsScene::getTitleAlignH() const
{
    return title_align_h;
}

void CustomGraphicsScene::setTitleAlignH(CustomGraphicsScene::TITLE_ALIGN_H titleAlignH)
{
    title_align_h = titleAlignH;
}

CustomGraphicsScene::TITLE_ALIGN_V CustomGraphicsScene::getTitleAlignV() const
{
    return title_align_v;
}

void CustomGraphicsScene::setTitleAlignV(CustomGraphicsScene::TITLE_ALIGN_V titleAlignV)
{
    title_align_v = titleAlignV;
}

