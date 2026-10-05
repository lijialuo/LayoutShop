#include "LayoutPainter.h"
#include <iostream>
#include <qfontmetrics.h>
#include <QGraphicsRectItem>
#include <stack>
#include <unordered_map>


LayoutPainter::LayoutPainter()
{}

LayoutPainter::~LayoutPainter()
{}

LayoutShop::Preview LayoutPainter::Draw()
{
	LayoutShop::Preview preview;
	preview.actual_scene = nullptr;
	preview.preview_scene = nullptr;
	
	if (layout_tree_.size() == 0 || article_ == nullptr) return preview;
	
	preview.actual_scene = GenerateScene(false);
	preview.preview_scene = GenerateScene(true);

	return preview;
}

CustomGraphicsScene* LayoutPainter::GenerateScene(bool preview)
{
	std::vector<CustomGraphicsItem*> text_item_list;
	CustomGraphicsScene* scene = new CustomGraphicsScene();
	scene->setBackgroundBrush(QColor(188, 188, 188));
	int end_idx;
    int text_idx = 0;
	if (preview) end_idx = 1;
	else end_idx = layout_tree_.size();
	for (int i = 0; i < end_idx; ++i)
	{
		std::stack<CCombineTreeNode*> node_stack;
		node_stack.push(layout_tree_[i]);
		std::unordered_map<int, CustomGraphicsItem*> non_leaf_item_map;
		CustomGraphicsItem* root_item;
		while (!node_stack.empty())
		{
			auto node = node_stack.top();
			node_stack.pop();
			CustomGraphicsItem* node_item = new CustomGraphicsItem();
			QObject::connect(node_item, &CustomGraphicsItem::EditDone, scene, &	CustomGraphicsScene::NodeEditDone);
			node_item->SetNodeType(node->node_present_type);
			node_item->SetTreeNode(node);
			node_item->SetPageIdx(i);
			CustomGraphicsItem* parent_item = nullptr;
			if (node->parent != nullptr)
			{
				parent_item = non_leaf_item_map[node->parent->index];
				node_item->setParentItem(parent_item);
				node_item->setPos(node->x - node->parent->x, node->y - node->parent->y);
			}
			else
			{
				root_item = node_item;
				node_item->setPos(node->x, node->y);
			}

			node_item->setRect(0, 0, node->width, node->height);
			if (node->children.size() == 0)
			{
				//leaf_item_list.push_back(node_item);
				if (node->node_present_type == NODE_PRESENT_TYPE::TITLE)
				{
					FillTitle(scene, node_item);
				}
				else if (node->node_present_type == NODE_PRESENT_TYPE::TEXT)
				{
                    node_item->GetTreeNode()->text_idx = text_idx;
                    text_idx ++;
					text_item_list.push_back(node_item);
				}
				else if (node->node_present_type == NODE_PRESENT_TYPE::PICTURE)
				{
					FillImg(node_item, node->corres_img_idx);
				}
			}
			else
			{
				non_leaf_item_map[node->index] = node_item;
			}
			for (int j = node->children.size() - 1; j >= 0; j--)
			{
				node_stack.push(node->children[j]);
			}
		}
		CustomGraphicsItem* paper_item = new CustomGraphicsItem();
		paper_item->SetNodeType(NODE_PRESENT_TYPE::NONLABEL);
		paper_item->SetTreeNode(nullptr);
		paper_item->setPos(0, i * (H + 10));
		paper_item->setRect(0, 0, W, H);
		root_item->setParentItem(paper_item);
		scene->addItem(paper_item);
	}
    if(paragraph_) DrawTextForParas(scene, text_item_list);
	else DrawText(scene, text_item_list);

	/*for(auto pair_item:non_leaf_item_map)
	{
		scene_->addItem(pair_item.second);
		std::cout << pair_item.second->zValue();
	}*/
	/*std::cout << std::endl;*/
	if(!preview)
	{
		scene->SetHPadding(layout_tree_[0]->h_padding);
		scene->SetVPadding(layout_tree_[0]->v_padding);
		scene->GenerateHelperRects();
	}
	return scene;
}

void LayoutPainter::FillTitle(CustomGraphicsScene* scene, CustomGraphicsItem* item)
{

    QFont font;
    font.setFamily(title_font_type_.data());
    double title_area = item->rect().height() * item->rect().width();
    int font_size = title_size_list_.back();
    for (int i = title_size_list_.size() - 2 ; i >= 0; --i)
    {
        if (title_area >= title_area_max_list_[i])
        {
            font_size = title_size_list_[i + 1];

            break;
        }
    }
    if (title_area < title_area_max_list_[0]) font_size = title_size_list_[0];
    font.setPointSize(font_size);
    font.setBold(true);
    //text_font_.setFamily("Arial");

    /*row space*/
    scene->setTitleFont(font);
    scene->setTitleAlignH(CustomGraphicsScene::CENTER_ALIGN);
    scene->setTitleAlignV(CustomGraphicsScene::MIDDLE_ALIGN);
    if(item->rect().height() / item->rect().width() >= 2.0) scene->setTitleRotateValue(CustomGraphicsScene::ROTATE_90);
    else scene->setTitleRotateValue(CustomGraphicsScene::ROTATE_0);
	item->SetTitle(QString(article_->title_.data()));
}


//void LayoutPainter::FillText(CustomGraphicsItem* item, int& text_para_index, int& text_body_index)
//{
//
//    int row_space = 0;
//    QFontMetrics font_metrics(text_font_);
//    item->SetTextFont(text_font_);
//
////    int words_per_row;
//    double width = item->rect().width();
//    double height = item->rect().height();
//
////    words_per_row = width / font_metrics.averageCharWidth();
//    double x;
////    double x =  static_cast<double>(static_cast<int>(width) % font_metrics.averageCharWidth()) / 2;  //how?
//    int row = static_cast<int>(height) / (font_metrics.height() + row_space);
//    double y = static_cast<double>(static_cast<int>(height) % (font_metrics.height() + row_space)) / 2;
//    std::vector<std::pair<double, double>> start_point_list;
//    std::vector<QString> text_list;
//    int sub_word_index = 0;
//    for (int row_idx = 0; row_idx < row && text_para_index < article_->text_list_.size(); row_idx++, y += font_metrics.height() + row_space)
//    {
//        x = 0.0;
//        int word_count = 0;
//        QString row_text = "";
//        bool break_line = false;
//        while(!break_line)
//        {
//            QString word = QString(article_->text_list_[text_para_index].word_list[text_body_index].data());
//            word = word.last(word.size() - sub_word_index);
//            double add_width = font_metrics.horizontalAdvance(word);
//            if(x + add_width <= width)
//            {
//                text_body_index ++;
//                x += add_width;
//                row_text.append(word);
//                word_count++;
//                sub_word_index = 0;
//            }
//            else
//            {
//                if(word.at(word.size() - 1) == ' ' && x + font_metrics.horizontalAdvance(word.first(word.size() - 1)) <= width)
//                {
//                    text_body_index ++;
//                    row_text.append(word.first(word.size() - 1));
//                    sub_word_index = 0;
//                }
//                else if(word_count <= 2)
//                {
//                    for(int i = 1; i <= word.size(); ++i)
//                    {
//                        if(x + font_metrics.horizontalAdvance(word.first(i)) <= width)
//                        {
//                            row_text.append(word.at(i - 1));
//                            sub_word_index ++;
//                        }
//                    }
//                }
//                break_line = true;
//            }
//            if( text_body_index == article_->text_list_[text_para_index].word_list.size())
//            {
//                text_para_index++;
//                text_body_index = 0;
//                sub_word_index = 0;
//                break_line = true;
//            }
//        }
//        start_point_list.emplace_back(0.0,y + font_metrics.ascent());
//        text_list.push_back(row_text);
//    }
//    item->SetStartPointList(start_point_list);
//    item->SetTextList(text_list);
//}

//void LayoutPainter::FillText(CustomGraphicsItem* item, int& text_para_index, int& text_body_index)
//{
//
//	int row_space = 0;
//	QFontMetrics font_metrics(text_font_);
//	item->SetTextFont(text_font_);
//
//	int words_per_row;
//	double width = item->rect().width();
//	double height = item->rect().height();
//
//	words_per_row = width / font_metrics.averageCharWidth();
//	double x =  static_cast<double>(static_cast<int>(width) % font_metrics.averageCharWidth()) / 2;
//	int row = static_cast<int>(height) / (font_metrics.ascent() + font_metrics.descent() + row_space);
//	double y = static_cast<double>(static_cast<int>(height) % (font_metrics.ascent() + font_metrics.descent() + row_space)) / 2;
//	std::vector<std::pair<double, double>> start_point_list;
//	std::vector<std::string> text_list;
//
//
//
//	for (int row_idx = 0; row_idx < row && text_para_index < text_list_.size(); row_idx++, y += font_metrics.ascent() + font_metrics.descent() + row_space)
//	{
//		if (text_list_[text_para_index][text_body_index] == ' ')
//		{
//			if (text_body_index < text_list_[text_para_index].size() - 1)
//			{
//				text_body_index++;
//			}
//			else
//			{
//				text_para_index++;
//				text_body_index = 0;
//				if (text_para_index >= text_list_.size()) break;
//			}
//		}
//		if (text_body_index + words_per_row < text_list_[text_para_index].size())
//		{
//			start_point_list.push_back(std::pair<double, double>(x, y + font_metrics.ascent()));
//			text_list.push_back(text_list_[text_para_index].substr(text_body_index, words_per_row).data());
//			//	paint.drawText(x, y + font_metrics.ascent(), article_.text_list[text_para_index].substr(text_body_index, words_per_row).data());
//			text_body_index += words_per_row;
//		}
//		else
//		{
//			start_point_list.push_back(std::pair<double, double>(x, y + font_metrics.ascent()));
//			text_list.push_back(text_list_[text_para_index].substr(text_body_index, text_list_[text_para_index].size() - text_body_index).data());
//			//paint.drawText(x, y + font_metrics.ascent(), article_.text_list[text_para_index].substr(text_body_index, article_.text_list[text_para_index].size() - text_body_index).data());
//			text_para_index++;
//			text_body_index = 0;
//			if (row_idx < row - 1)
//			{
//				//row_idx++;
////				y += font_metrics.ascent() + font_metrics.descent() + row_space;
//			}
//		}
//	}
//	item->SetStartPointList(start_point_list);
//	item->SetTextList(text_list);
//}

void LayoutPainter::FillImg(CustomGraphicsItem* item, int img_idx)
{
	QPixmap pixmap;
	if (img_idx < 0 || img_idx >= article_->img_list_.size()) img_idx = 0; //delete this
	pixmap = article_->img_list_[img_idx];
	item->SetPixmap(pixmap);
}


void LayoutPainter::SetTitleFontSizeRange(int min_val, int max_val)
{
    title_size_list_.clear();
    for(int val = min_val; val <= max_val;++val)
    {
        title_size_list_.push_back(val);
    }
}

void LayoutPainter::SetTextFontSizeRange(int min_val, int max_val)
{
    text_size_list_.clear();
    for(int val = min_val; val <= max_val; ++val)
    {
        text_size_list_.push_back(val);
    }
}

void LayoutPainter::CalTextAndTitleArea()
{
//    QFont font;
//    //const int font_size = 4;//12;
//    const int row_space_min = 3;//3;
//    const int row_space_max = 10;
//    //text_font_.setPixelSize(font_size);
//    font.setFamily(text_font_type_.data());
//
//    //std::vector<std::string> text_list_t;
////	for(int i=0;i< article_->text_list_.size();++i)
////	{
////		for(int j= 0;j< article_->text_list_[i].body_text.size();++j)
////		{
////			text_list_.push_back(article_->text_list_[i].body_text[j]);
////		}
////	}
////
////	for (size_t i = 0; i < text_list_.size(); ++i)
////	{
////		letter_count += text_list_[i].size();
////	}
////
//    text_area_max_list_.clear();
//    text_area_min_list_.clear();
//
//
//    for (int i = 0; i < text_size_list_.size(); ++i)
//    {
//        double min_area = 0.0, max_area = 0.0;
//        font.setPointSize(text_size_list_[i]);
//        QFontMetrics font_metrics(font);
//        for (size_t i = 0; i < article_->text_list_.size(); ++i)
//        {
//            for (size_t j = 0; j < article_->text_list_[i].word_list.size(); ++j)
//            {
//                min_area += font_metrics.horizontalAdvance(QString(article_->text_list_[i].word_list[j].data())) * (row_space_min + font_metrics.height());
//                max_area += font_metrics.horizontalAdvance(QString(article_->text_list_[i].word_list[j].data())) * (row_space_max + font_metrics.height());
//            }
//        }
//        text_area_max_list_.push_back(max_area);
//        text_area_min_list_.push_back(min_area);
//    }
//
//



//text

    int letter_count = 0;
    QFont font;
    //const int font_size = 4;//12;
    const int row_space_min = 3;//3;
    const int row_space_max = 10;
    //font.setPixelSize(font_size);
    font.setFamily(text_font_type_.data());
    para_text_area_max_list_.clear();
    para_text_area_min_list_.clear();
    all_text_area_min_list_.clear();
    all_text_area_max_list_.clear();

    for(int i = 0; i < text_size_list_.size(); i++)
    {
        int text_font_size = text_size_list_[i];
        font.setPointSize(text_font_size);
        QFontMetrics  font_metrics = QFontMetrics(font);
        double all_min_area = 0, all_max_area = 0;
        para_text_area_min_list_.push_back(std::vector<double>());
        para_text_area_max_list_.push_back(std::vector<double>());
        for (size_t j = 0; j < article_->text_list_.size(); ++j)
        {
            double para_min_area = 0, para_max_area = 0;
            for (size_t k = 0; k < article_->text_list_[j].word_list.size(); ++k)
            {
                para_min_area += font_metrics.horizontalAdvance(QString(article_->text_list_[j].word_list[k].data())) * (row_space_min + font_metrics.height());
                para_max_area += font_metrics.horizontalAdvance(QString(article_->text_list_[j].word_list[k].data())) * (row_space_max + font_metrics.height());
            }
            para_text_area_min_list_[i].push_back(para_min_area);
            para_text_area_max_list_[i].push_back(para_max_area);
            all_min_area += para_min_area;
            all_max_area += para_max_area;
        }
        all_text_area_min_list_.push_back(all_min_area);
        all_text_area_max_list_.push_back(all_max_area);
    }


    //title
    title_area_max_list_.clear();
    title_area_min_list_.clear();
    font.setFamily(title_font_type_.data());
    int least_title_len = 80;
    double low_bound_factor = 1.5, high_bound_factor = 4.5;
    for(int i = 0; i < title_size_list_.size(); ++i)
    {
        double min_area = 0.0, max_area = 0.0;
        font.setPointSize(title_size_list_[i]);
        QFontMetrics font_metrics(font);
        min_area = font_metrics.horizontalAdvance(QString(article_->title_.data())) * font_metrics.height() * low_bound_factor ;
        max_area = std::max(font_metrics.horizontalAdvance(QString(article_->title_.data())) , font_metrics.averageCharWidth() * least_title_len) * font_metrics.height() * high_bound_factor;
        title_area_max_list_.push_back(max_area);
        title_area_min_list_.push_back(min_area);
    }
}

void LayoutPainter::SetTitleFontType(std::string font_type)
{
    title_font_type_ = font_type;
}

void LayoutPainter::SetTextFontType(std::string font_type)
{
    text_font_type_ = font_type;
}



void LayoutPainter::DrawText(CustomGraphicsScene* scene, std::vector<CustomGraphicsItem*> text_item_list)
{
    QFont font;
    font.setFamily(text_font_type_.data());
    double text_area = 0.0;
    for (auto item : text_item_list)
    {
        int width = item->rect().width();
        int height = item->rect().height();
        text_area += width * height;
    }
    for (int i = text_size_list_.size() - 1 ; i >= 0; --i)
    {
        if (text_area >= all_text_area_min_list_[i])
        {
            font.setPointSize(text_size_list_[i]);
            break;
        }
    }
    if (text_area < all_text_area_min_list_[0]) font.setPointSize(text_size_list_[0]);
    //text_font_.setFamily("Arial");

    /*row space*/
    double content_width = 0.0;
    QFontMetrics font_metrics(font);
    for (size_t i = 0; i < article_->text_list_.size(); ++i)
    {
        for (size_t j = 0; j < article_->text_list_[i].word_list.size(); ++j)
        {
            content_width += font_metrics.horizontalAdvance(QString(article_->text_list_[i].word_list[j].data()));
        }
    }
    int max_row_space = text_area / content_width  - font_metrics.height();
//	for (auto item : text_item_list)
//	{
//		FillText(item, text_para_idx, text_body_idx);
//	}

    std::vector<int> row_space_list(text_item_list.size(), max_row_space); // row space for every item
    bool text_over = false; // over flag
    bool decrease_terminate = false; // stop decreasing row space?
    int increase_index = row_space_list.size() - 1; // which item's row space should be increased now?
    std::vector<std::vector<QString>> text_list_backup; // newest successful result - text
    std::vector<std::vector<std::pair<double,double>>> position_list_backup; //  newest successful result - position
    scene->setTextFont(font);
    while(!text_over)
    {
        int text_para_index = 0, text_body_index = 0;
        int item_index;
        double x,y;
        for(item_index = 0; item_index < text_item_list.size(); ++item_index)
        {
            int row_space = row_space_list[item_index];
            auto item = text_item_list[item_index];
//    int words_per_row;
            double width = item->rect().width();
            double height = item->rect().height();

//    words_per_row = width / font_metrics.averageCharWidth();
            //double x;
//    double x =  static_cast<double>(static_cast<int>(width) % font_metrics.averageCharWidth()) / 2;  //how?
            int row = static_cast<int>(height) / (font_metrics.height() + row_space);
            //y = static_cast<double>(static_cast<int>(height) % (font_metrics.height() + row_space)) / 2;
            y = 0.0;
            std::vector<std::pair<double, double>> start_point_list;
            std::vector<QString> text_list;
            int sub_word_index = 0;
            for (int row_idx = 0; row_idx < row && text_para_index < article_->text_list_.size(); row_idx++, y += font_metrics.height() + row_space)
            {
                x = 0.0;
                int word_count = 0;
                QString row_text = "";
                bool break_line = false;
                while(!break_line)
                {
                    QString word = QString(article_->text_list_[text_para_index].word_list[text_body_index].data());
                    word = word.last(word.size() - sub_word_index);
                    double add_width = font_metrics.horizontalAdvance(word);
                    if(x + add_width <= width)
                    {
                        text_body_index ++;
                        x += add_width;
                        row_text.append(word);
                        word_count++;
                        sub_word_index = 0;
                    }
                    else
                    {
                        if(word.at(word.size() - 1) == ' ' && x + font_metrics.horizontalAdvance(word.first(word.size() - 1)) <= width)
                        {
                            text_body_index ++;
                            row_text.append(word.first(word.size() - 1));
                            sub_word_index = 0;
                        }
                        else if(word_count <= 2)
                        {
                            for(int i = 1; i <= word.size(); ++i)
                            {
                                if(x + font_metrics.horizontalAdvance(word.first(i)) <= width)
                                {
                                    row_text.append(word.at(i - 1));
                                    sub_word_index ++;
                                }
                            }
                        }
                        break_line = true;
                    }
                    if( text_body_index == article_->text_list_[text_para_index].word_list.size())
                    {
                        text_para_index++;
                        text_body_index = 0;
                        sub_word_index = 0;
                        break_line = true;
                    }
                }
                start_point_list.emplace_back(0.0,y + font_metrics.ascent());
                text_list.push_back(row_text);
            }
            item->SetStartPointList(start_point_list);
            item->SetTextList(text_list);
        }


        /*fill text blocks*/
        if(text_para_index < article_->text_list_.size() || text_body_index < article_->text_list_[text_para_index].word_list.size())
        {
            //fail filling all the words
            if(decrease_terminate)
            {
                //can not  decrease row space anymore, so we have to quit, the newest result is the best result
                for(int i=0; i < text_item_list.size(); ++i)
                {
                    text_item_list[i]->SetStartPointList(position_list_backup[i]);
                    text_item_list[i]->SetTextList(text_list_backup[i]);
                }
                text_over = true;
            }
            else
            {
                // try decreasing row space in all items and start the process again
                int row_space = row_space_list[0];
                if(row_space == 0) text_over = true;
                else
                {
                    for(auto item : text_item_list)
                    {
                        item->SetStartPointList({});
                        item->SetTextList({});
                    }
                    row_space_list = std::vector<int>(row_space_list.size(), row_space - 1);
                }
            }
        }

        else
        {
            //layout space left
            decrease_terminate = true; //no more decreasing from now on
            double left_area =  item_index == text_item_list.size()  ?  text_item_list.back()->rect().width() * (text_item_list.back()->rect().height() - y) : -1;

            if(item_index == text_item_list.size() && left_area <= 1.0/3.0 * text_item_list.back()->rect().width() * text_item_list.back()->rect().height())
            {
                //the remaining space is not that large, so we quit
                text_over = true;
            }
            else if(increase_index < 0)
            {
                //no more increasing, because every item's row space has reach the lower bound
                for(int i=0; i < text_item_list.size(); ++i)
                {
                    text_item_list[i]->SetStartPointList(position_list_backup[i]);
                    text_item_list[i]->SetTextList(text_list_backup[i]);
                }
                text_over = true;
            }
            else
            {
                //backup
                text_list_backup.clear();
                position_list_backup.clear();
                for(int i=0; i < text_item_list.size(); ++i)
                {
                    text_list_backup.push_back(text_item_list[i]->GetTextList());
                    position_list_backup.push_back(text_item_list[i]->GetStartPointList());
                    text_item_list[i]->SetStartPointList({});
                    text_item_list[i]->SetTextList({});
                }
                //increase
                row_space_list[increase_index] ++; //increase an item's row space
                increase_index --; //next time we should increase another item's row space
            }
        }
    }
    //FillText(text_item_list, std::max(row_space - 1,0));
}

void LayoutPainter::DrawText(CustomGraphicsScene *scene, std::vector<CustomGraphicsItem *> text_item_list, QFont font)
{
    double text_area = 0.0;
    for (auto item : text_item_list)
    {
        int width = item->rect().width();
        int height = item->rect().height();
        text_area += width * height;
    }

    /*row space*/
    double content_width = 0.0;
    QFontMetrics font_metrics(font);
    for (size_t i = 0; i < article_->text_list_.size(); ++i)
    {
        for (size_t j = 0; j < article_->text_list_[i].word_list.size(); ++j)
        {
            content_width += font_metrics.horizontalAdvance(QString(article_->text_list_[i].word_list[j].data()));
        }
    }
    int max_row_space = text_area / content_width  - font_metrics.height();
    std::vector<int> row_space_list(text_item_list.size(), max_row_space);
    bool text_over = false;
    bool decrease_terminate = false;
    int increase_index = row_space_list.size() - 1;
    std::vector<std::vector<QString>> text_list_backup;
    std::vector<std::vector<std::pair<double,double>>> position_list_backup;
    scene->setTextFont(font);
    while(!text_over)
    {
        int text_para_index = 0, text_body_index = 0;
        int item_index;
        double x,y;
        for(item_index = 0; item_index < text_item_list.size(); ++item_index)
        {
            int row_space = row_space_list[item_index];
            auto item = text_item_list[item_index];
//    int words_per_row;
            double width = item->rect().width();
            double height = item->rect().height();

//    words_per_row = width / font_metrics.averageCharWidth();
            //double x;
//    double x =  static_cast<double>(static_cast<int>(width) % font_metrics.averageCharWidth()) / 2;  //how?
            int row = static_cast<int>(height) / (font_metrics.height() + row_space);
            //y = static_cast<double>(static_cast<int>(height) % (font_metrics.height() + row_space)) / 2;
            y = 0.0;
            std::vector<std::pair<double, double>> start_point_list;
            std::vector<QString> text_list;
            int sub_word_index = 0;
            for (int row_idx = 0; row_idx < row && text_para_index < article_->text_list_.size(); row_idx++, y += font_metrics.height() + row_space)
            {
                x = 0.0;
                int word_count = 0;
                QString row_text = "";
                bool break_line = false;
                while(!break_line)
                {
                    QString word = QString(article_->text_list_[text_para_index].word_list[text_body_index].data());
                    word = word.last(word.size() - sub_word_index);
                    double add_width = font_metrics.horizontalAdvance(word);
                    if(x + add_width <= width)
                    {
                        text_body_index ++;
                        x += add_width;
                        row_text.append(word);
                        word_count++;
                        sub_word_index = 0;
                    }
                    else
                    {
                        if(word.at(word.size() - 1) == ' ' && x + font_metrics.horizontalAdvance(word.first(word.size() - 1)) <= width)
                        {
                            text_body_index ++;
                            row_text.append(word.first(word.size() - 1));
                            sub_word_index = 0;
                        }
                        else if(word_count <= 2)
                        {
                            for(int i = 1; i <= word.size(); ++i)
                            {
                                if(x + font_metrics.horizontalAdvance(word.first(i)) <= width)
                                {
                                    row_text.append(word.at(i - 1));
                                    sub_word_index ++;
                                }
                            }
                        }
                        break_line = true;
                    }
                    if( text_body_index == article_->text_list_[text_para_index].word_list.size())
                    {
                        text_para_index++;
                        text_body_index = 0;
                        sub_word_index = 0;
                        break_line = true;
                    }
                }
                start_point_list.emplace_back(0.0,y + font_metrics.ascent());
                text_list.push_back(row_text);
            }
            item->SetStartPointList(start_point_list);
            item->SetTextList(text_list);
        }


        /*fill text blocks*/
        if(text_para_index < article_->text_list_.size() || text_body_index < article_->text_list_[text_para_index].word_list.size())
        {
            if(decrease_terminate)
            {
                for(int i=0; i < text_item_list.size(); ++i)
                {
                    text_item_list[i]->SetStartPointList(position_list_backup[i]);
                    text_item_list[i]->SetTextList(text_list_backup[i]);
                }
                text_over = true;
            }
            else
            {
                int row_space = row_space_list[0];
                if(row_space == 0) text_over = true;
                else
                {
                    for(auto item : text_item_list)
                    {
                        item->SetStartPointList({});
                        item->SetTextList({});
                    }
                    row_space_list = std::vector<int>(row_space_list.size(), row_space - 1);
                }
            }
        }
            //layout space left
        else
        {
            decrease_terminate = true;
            double left_area =  item_index == text_item_list.size()  ?  text_item_list.back()->rect().width() * (text_item_list.back()->rect().height() - y) : -1;


            if(item_index == text_item_list.size() && left_area <= 1.0/3.0 * text_item_list.back()->rect().width() * text_item_list.back()->rect().height())
            {
                text_over = true;
            }
            else if(increase_index < 0)
            {
                //no more increase
                for(int i=0; i < text_item_list.size(); ++i)
                {
                    text_item_list[i]->SetStartPointList(position_list_backup[i]);
                    text_item_list[i]->SetTextList(text_list_backup[i]);
                }
                text_over = true;
            }
            else
            {
                //backup
                text_list_backup.clear();
                position_list_backup.clear();
                for(int i=0; i < text_item_list.size(); ++i)
                {
                    text_list_backup.push_back(text_item_list[i]->GetTextList());
                    position_list_backup.push_back(text_item_list[i]->GetStartPointList());
                    text_item_list[i]->SetStartPointList({});
                    text_item_list[i]->SetTextList({});
                }
                //increase
                row_space_list[increase_index] ++;
                increase_index --;
            }
        }
    }
}

void LayoutPainter::DrawTextForParas(CustomGraphicsScene *scene, std::vector<CustomGraphicsItem *> text_item_list)
{

    QFont font;
    font.setFamily(text_font_type_.data());
    int width = text_item_list[0]->rect().width();
    int height = text_item_list[0]->rect().height();
    double text_area = width * height;
    for(int i =  text_size_list_.size() - 1; i >= 0; --i)
    {
        if(text_area >= para_text_area_min_list_[i][0])
        {
            font.setPointSize(text_size_list_[i]);
            break;
        }
    }
    if(text_area < para_text_area_min_list_[0][0]) font.setPointSize(text_size_list_[0]);
    scene->setTextFont(font);

    for (int i = 0; i < text_item_list.size(); ++i)
    {
        auto item = text_item_list[i];
        text_area = item->rect().width() * item->rect().height();
/*row space*/

        double content_width = 0.0;
        QFontMetrics font_metrics(font);
        for (size_t j = 0; j < article_->text_list_[i].word_list.size(); ++j)
        {
            content_width += font_metrics.horizontalAdvance(QString(article_->text_list_[i].word_list[j].data()));
        }
        int row_space = text_area / content_width  - font_metrics.height();
        bool text_over = false; // over flag
        bool decrease_terminate = false; // stop decreasing row space?
        std::vector<QString> text_list_backup; // newest successful result - text
        std::vector<std::pair<double,double>> position_list_backup; //  newest successful result - position
        while(!text_over)
        {
            int text_body_index = 0;
            double x,y;
            width = item->rect().width();
            height = item->rect().height();
            int row = static_cast<int>(height) / (font_metrics.height() + row_space);
            y = 0.0;
            std::vector<std::pair<double, double>> start_point_list;
            std::vector<QString> text_list;
            int sub_word_index = 0;
            for (int row_idx = 0; row_idx < row; row_idx++, y += font_metrics.height() + row_space)
            {
                x = 0.0;
                int word_count = 0;
                QString row_text = "";
                bool break_line = false;
                while(!break_line)
                {
                    QString word = QString(article_->text_list_[i].word_list[text_body_index].data());
                    word = word.last(word.size() - sub_word_index);
                    double add_width = font_metrics.horizontalAdvance(word);
                    if(x + add_width <= width)
                    {
                        text_body_index ++;
                        x += add_width;
                        row_text.append(word);
                        word_count++;
                        sub_word_index = 0;
                    }
                    else
                    {
                        if(word.at(word.size() - 1) == ' ' && x + font_metrics.horizontalAdvance(word.first(word.size() - 1)) <= width)
                        {
                            text_body_index ++;
                            row_text.append(word.first(word.size() - 1));
                            sub_word_index = 0;
                        }
                        else if(word_count <= 2)
                        {
                            for(int i = 1; i <= word.size(); ++i)
                            {
                                if(x + font_metrics.horizontalAdvance(word.first(i)) <= width)
                                {
                                    row_text.append(word.at(i - 1));
                                    sub_word_index ++;
                                }
                            }
                        }
                        break_line = true;
                    }
                    if( text_body_index == article_->text_list_[i].word_list.size())
                    {
                        row_idx = row;
                        break_line = true;
                    }
                }
                start_point_list.emplace_back(0.0,y + font_metrics.ascent());
                text_list.push_back(row_text);
            }
            item->SetStartPointList(start_point_list);
            item->SetTextList(text_list);


            /*fill text blocks*/
            if(text_body_index < article_->text_list_[i].word_list.size())
            {
                //fail filling all the words
                if(decrease_terminate)
                {
                    //can not  decrease row space anymore, so we have to quit, the newest result is the best result
                    item->SetStartPointList(position_list_backup);
                    item->SetTextList(text_list_backup);
                    text_over = true;
                }
                else
                {
                    // try decreasing row space in all items and start the process again
                    if(row_space == 0) text_over = true;
                    else
                    {
                        item->SetStartPointList({});
                        item->SetTextList({});
                        row_space--;
                    }
                }
            }
            else
            {
                //layout space left
                decrease_terminate = true; //no more decreasing from now on
                double left_area =    item->rect().width() * (item->rect().height() - y);

                if(left_area <= 1.0/3.0 * item->rect().width() * item->rect().height())
                {
                    //the remaining space is not that large, so we quit
                    text_over = true;
                }
                else if(row_space > 10)  ///this!!!!!
                {
                    //no more increasing, because every item's row space has reach the lower bound
                    item->SetStartPointList(position_list_backup);
                    item->SetTextList(text_list_backup);
                    text_over = true;
                }
                else
                {
                    //backup
                    text_list_backup.clear();
                    position_list_backup.clear();
                    text_list_backup = item->GetTextList();
                    position_list_backup = item->GetStartPointList();
                    item->SetStartPointList({});
                    item->SetTextList({});
                    //increase
                    row_space ++;
                }
            }
        }
    }


}

void LayoutPainter::DrawTextForParas(CustomGraphicsScene *scene, std::vector<CustomGraphicsItem *> text_item_list,
                                     QFont font)
{

    scene->setTextFont(font);

    for (int i = 0; i < text_item_list.size(); ++i)
    {
        auto item = text_item_list[i];
        double width, height, text_area;
        text_area = item->rect().width() * item->rect().height();
/*row space*/
        double content_width = 0.0;
        QFontMetrics font_metrics(font);
        for (size_t j = 0; j < article_->text_list_[i].word_list.size(); ++j)
        {
            content_width += font_metrics.horizontalAdvance(QString(article_->text_list_[i].word_list[j].data()));
        }
        int row_space = text_area / content_width  - font_metrics.height();
        bool text_over = false; // over flag
        bool decrease_terminate = false; // stop decreasing row space?
        std::vector<QString> text_list_backup; // newest successful result - text
        std::vector<std::pair<double,double>> position_list_backup; //  newest successful result - position
        while(!text_over)
        {
            int text_body_index = 0;
            double x,y;
            width = item->rect().width();
            height = item->rect().height();
            int row = static_cast<int>(height) / (font_metrics.height() + row_space);
            y = 0.0;
            std::vector<std::pair<double, double>> start_point_list;
            std::vector<QString> text_list;
            int sub_word_index = 0;
            for (int row_idx = 0; row_idx < row; row_idx++, y += font_metrics.height() + row_space)
            {
                x = 0.0;
                int word_count = 0;
                QString row_text = "";
                bool break_line = false;
                while(!break_line)
                {
                    QString word = QString(article_->text_list_[i].word_list[text_body_index].data());
                    word = word.last(word.size() - sub_word_index);
                    double add_width = font_metrics.horizontalAdvance(word);
                    if(x + add_width <= width)
                    {
                        text_body_index ++;
                        x += add_width;
                        row_text.append(word);
                        word_count++;
                        sub_word_index = 0;
                    }
                    else
                    {
                        if(word.at(word.size() - 1) == ' ' && x + font_metrics.horizontalAdvance(word.first(word.size() - 1)) <= width)
                        {
                            text_body_index ++;
                            row_text.append(word.first(word.size() - 1));
                            sub_word_index = 0;
                        }
                        else if(word_count <= 2)
                        {
                            for(int i = 1; i <= word.size(); ++i)
                            {
                                if(x + font_metrics.horizontalAdvance(word.first(i)) <= width)
                                {
                                    row_text.append(word.at(i - 1));
                                    sub_word_index ++;
                                }
                            }
                        }
                        break_line = true;
                    }
                    if( text_body_index == article_->text_list_[i].word_list.size())
                    {
                        row_idx = row;
                        break_line = true;
                    }
                }
                start_point_list.emplace_back(0.0,y + font_metrics.ascent());
                text_list.push_back(row_text);
            }
            item->SetStartPointList(start_point_list);
            item->SetTextList(text_list);


            /*fill text blocks*/
            if(text_body_index < article_->text_list_[i].word_list.size())
            {
                //fail filling all the words
                if(decrease_terminate)
                {
                    //can not  decrease row space anymore, so we have to quit, the newest result is the best result
                    item->SetStartPointList(position_list_backup);
                    item->SetTextList(text_list_backup);
                    text_over = true;
                }
                else
                {
                    // try decreasing row space in all items and start the process again
                    if(row_space == 0) text_over = true;
                    else
                    {
                        item->SetStartPointList({});
                        item->SetTextList({});
                        row_space--;
                    }
                }
            }
            else
            {
                //layout space left
                decrease_terminate = true; //no more decreasing from now on
                double left_area =    item->rect().width() * (item->rect().height() - y);

                if(left_area <= 1.0/3.0 * item->rect().width() * item->rect().height())
                {
                    //the remaining space is not that large, so we quit
                    text_over = true;
                }
                else if(row_space > 10)  ///this!!!!!
                {
                    //no more increasing, because every item's row space has reach the lower bound
                    item->SetStartPointList(position_list_backup);
                    item->SetTextList(text_list_backup);
                    text_over = true;
                }
                else
                {
                    //backup
                    text_list_backup.clear();
                    position_list_backup.clear();
                    text_list_backup = item->GetTextList();
                    position_list_backup = item->GetStartPointList();
                    item->SetStartPointList({});
                    item->SetTextList({});
                    //increase
                    row_space ++;
                }
            }
        }
    }
}



