#include "LayoutShop.h"
#include <gurobi_c++.h>
#include <iostream>
#include <qfiledialog.h>
#include <qsettings.h>
#include <fstream>
#include "CCombineTreeProcessor.h"
#include "rapidjson/document.h"
#include <qmessagebox.h>
#include <thread>
#include "LayoutPainter.h"
#include <QPaintDevice>
#include <sys/stat.h>
#include <QSvgGenerator>
#include "rapidjson/rapidjson.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "QTimer"
#include "LayoutGeneratorMultiPage.h"
#include "LayoutGeneratorNew.h"
#include "CustomGraphicsItem2.h"
#include "TemplateDialog.h"
#include "FlowLayout.h"
#include "CustomGraphicsView.h"
#include <QPdfWriter>
#include <QGraphicsProxyWidget>

LayoutShop::LayoutShop(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	setFocusPolicy(Qt::StrongFocus);
    CreateRefineWidget();
	UpdateOutputAndPreview();
	ui.output_view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
	ui.actionContentView->setChecked(true);
    //ui.preview_list_scroll->widget()->setLayout(new FlowLayout(PREVIEW_LIST_SCROLL_MARGIN, PREVIEW_LIST_SCROLL_H_SPACE, PREVIEW_LIST_SCROLL_V_SPACE));
    //ui.preview_list_scroll->widget()->resize(,preview_list_.size())
    LoadTemplates();
}

LayoutShop::~LayoutShop()
{}

void LayoutShop::SetDefaultScale(double paper_h)
{
    if(ui.output_view->scene()!= nullptr)
    {
        //double scene_height = ui.output_view->scene()->height();
        double output_view_height = ui.output_view->height();
        double ratio = output_view_height / paper_h;
        int scale = ratio * 100;
        ui.horizontalSlider->setValue(scale);
    }

	//ui.comboBox->setCurrentText("100%");
}

void LayoutShop::PreLayout()
{
	if (layout_index_ > 0)
	{
		layout_index_--;
		UpdateOutputAndPreview();
	}
}

void LayoutShop::NextLayout()
{
	if (layout_index_ >= 0 && layout_index_ < layout_num_ - 1)
	{
		layout_index_++;
		UpdateOutputAndPreview();
	}
}

void LayoutShop::sliderToComboBox(int num_val)
{
	std::string text_val;
	text_val = std::to_string(num_val);
	text_val.append("%");
	ui.comboBox->setCurrentText(QString(text_val.data()));
}

void LayoutShop::comboBoxSelect(int index)
{
	if(index >= 0)
	{
		QString current_text = ui.comboBox->currentText();
		current_text.remove(current_text.size() - 1, 1);
		int scale_value = atoi(current_text.toStdString().data());
		ui.horizontalSlider->setValue(scale_value);
	}
}

void LayoutShop::clickViewContentButton()
{

	if (ui.actionContentView->isChecked())
	{
		content_view_ = true;
	}
	else
	{
		content_view_ = false;
	}
    CustomGraphicsScene::PAINT_TYPE paint_type = content_view_ ?
            CustomGraphicsScene::PAINT_TYPE::CONTENT : CustomGraphicsScene::NO_CONTENT;

	for(auto scene : preview_list_)
	{
        dynamic_cast<CustomGraphicsScene *>(scene.actual_scene)->SetPaintType(paint_type);
		scene.actual_scene->update();
        dynamic_cast<CustomGraphicsScene *>(scene.preview_scene)->SetPaintType(paint_type);
		scene.preview_scene->update();
	}
}


void LayoutShop::LayoutIndexInputChange()
{
	int index = ui.layout_index_box->value() - 1;
	if(index >= 0 && index < layout_num_)
	{
		layout_index_ = index;
		UpdateOutputAndPreview();
	}
}

void LayoutShop::ReceivePreviewIndex(int index)
{
	if(index >= 0 && index < layout_num_)
	{
		layout_index_ = index;
		UpdateOutputAndPreview();
	}
}

void LayoutShop::ActiveEditNodeSize()
{
	if(ui.actionEdit_Node_Size->isChecked())
	{
		//ui.actionEdit_Node_Size->setChecked(true);
		//disable all other shit
		this->edit_type_ = CustomGraphicsScene::SIZE_ADJUST_NODE;
        ui.actionHorizontal_Align->setChecked(false);
        ui.actionAddText->setChecked(false);
        ui.actionAddPadding->setChecked(false);
        ui.actionMove->setChecked(false);
        ui.actionRemoveNode->setChecked(false);
        ui.actionVertical_Align->setChecked(false);
	}
	else
	{
		//ui.actionEdit_Node_Size->setChecked(false);
		this->edit_type_ = CustomGraphicsScene::NONE;
	} 
	SetSceneEditType();
}

void LayoutShop::ActiveMoveNode()
{
	if (ui.actionMove->isChecked())
	{
		//ui.actionEdit_Node_Size->setChecked(true);
		//disable all other shit
		this->edit_type_ = CustomGraphicsScene::MOVE_NODE;
        ui.actionEdit_Node_Size->setChecked(false);
        ui.actionAddText->setChecked(false);
        ui.actionAddPadding->setChecked(false);
        ui.actionHorizontal_Align->setChecked(false);
        ui.actionRemoveNode->setChecked(false);
        ui.actionVertical_Align->setChecked(false);
	}
	else
	{
		//ui.actionEdit_Node_Size->setChecked(false);
		this->edit_type_ = CustomGraphicsScene::NONE;
	}
	SetSceneEditType();
}

void LayoutShop::ActiveAddText()
{
	if (ui.actionAddText->isChecked())
	{
		//ui.actionEdit_Node_Size->setChecked(true);
		//disable all other shit
		this->edit_type_ = CustomGraphicsScene::ADD_TEXT;
        ui.actionEdit_Node_Size->setChecked(false);
        ui.actionHorizontal_Align->setChecked(false);
        ui.actionAddPadding->setChecked(false);
        ui.actionMove->setChecked(false);
        ui.actionRemoveNode->setChecked(false);
        ui.actionVertical_Align->setChecked(false);
	}
	else
	{
		//ui.actionEdit_Node_Size->setChecked(false);
		this->edit_type_ = CustomGraphicsScene::NONE;
	}
	SetSceneEditType();
}

void LayoutShop::ActiveAddPadding()
{
	if (ui.actionAddPadding->isChecked())
	{
		//disable all other shit
		this->edit_type_ = CustomGraphicsScene::ADD_PADDING;
        ui.actionEdit_Node_Size->setChecked(false);
        ui.actionAddText->setChecked(false);
        ui.actionHorizontal_Align->setChecked(false);
        ui.actionMove->setChecked(false);
        ui.actionRemoveNode->setChecked(false);
        ui.actionVertical_Align->setChecked(false);
	}
	else
	{
		this->edit_type_ = CustomGraphicsScene::NONE;
	}
	SetSceneEditType();
}

void LayoutShop::ActiveRemoveNode()
{
	if (ui.actionRemoveNode->isChecked())
	{
		//disable all other shit
		this->edit_type_ = CustomGraphicsScene::REMOVE_NODE;
        ui.actionEdit_Node_Size->setChecked(false);
        ui.actionAddText->setChecked(false);
        ui.actionAddPadding->setChecked(false);
        ui.actionMove->setChecked(false);
        ui.actionHorizontal_Align->setChecked(false);
        ui.actionVertical_Align->setChecked(false);
	}
	else
	{
		this->edit_type_ = CustomGraphicsScene::NONE;
	}
	SetSceneEditType();
}

void LayoutShop::ActiveHorizontalAlign()
{
    if (ui.actionHorizontal_Align->isChecked())
    {
        //disable all other shit
        this->edit_type_ = CustomGraphicsScene::HORIZONTAL_ALIGN;
        ui.actionEdit_Node_Size->setChecked(false);
        ui.actionAddText->setChecked(false);
        ui.actionAddPadding->setChecked(false);
        ui.actionMove->setChecked(false);
        ui.actionRemoveNode->setChecked(false);
        ui.actionVertical_Align->setChecked(false);
    }
    else
    {
        this->edit_type_ = CustomGraphicsScene::NONE;
    }
    SetSceneEditType();
}

void LayoutShop::ActiveVerticalAlign()
{
    if (ui.actionVertical_Align->isChecked())
    {
        //disable all other shit
        this->edit_type_ = CustomGraphicsScene::VERTICAL_ALIGN;
        ui.actionEdit_Node_Size->setChecked(false);
        ui.actionAddText->setChecked(false);
        ui.actionAddPadding->setChecked(false);
        ui.actionMove->setChecked(false);
        ui.actionRemoveNode->setChecked(false);
        ui.actionHorizontal_Align->setChecked(false);
    }
    else
    {
        this->edit_type_ = CustomGraphicsScene::NONE;
    }
    SetSceneEditType();
}

void LayoutShop::ReOptimize()
{
    double horizontalDPI = QPaintDevice::physicalDpiX();
    double verticalDPI = QPaintDevice::physicalDpiY();
    int paper_w = horizontalDPI / 2.54 * real_w_;
    int paper_h = verticalDPI / 2.54 * real_h_;
    //std::vector<std::vector<CCombineTreeNode*>> valid_layout_list;
    LayoutGeneratorMultiPage layout_generator;

    layout_generator.SetArticle(article_);
    layout_generator.SetTitleFontSizeRange(min_title_font_size_, max_title_font_size_);
    layout_generator.SetTextFontSizeRange(min_text_font_size_, max_text_font_size_);
    layout_generator.SetTitleFontType(title_font_type_);
    layout_generator.SetTextFontType(text_font_type_);
    layout_generator.SetW(paper_w);
    layout_generator.SetH(paper_h);

    GRBEnv env = GRBEnv(true);
    env.start();
    GRBModel model = GRBModel(env);
    if (multi_page_) {
        model.set("NonConvex", "2");
        model.set("TimeLimit", "2.0");
        model.set("OutputFlag", "0");
    } else {
        model.set("NonConvex", "2");
        model.set("TimeLimit", "1.0");
        model.set("OutputFlag", "0");
    }

    std::vector<CCombineTreeNode *> new_combine_tree;
    //auto old_combine_tree = combine_tree_list_[layout_index_];
    auto old_scene = preview_list_[layout_index_].actual_scene;
    auto old_preview = preview_list_[layout_index_].preview_scene;
    for (auto single_tree: combine_tree_list_[layout_index_]) {
        new_combine_tree.push_back(single_tree->DeepCopy());
    }

    CCombineTreeNode *fix_geometry_node = nullptr;

    for (auto root: new_combine_tree)
    {
        std::queue<CCombineTreeNode *> queue;
        queue.push(root);
        while (!queue.empty()) {
            auto node = queue.front();
            queue.pop();

            if (node->fix_geometry_level >= 0) {
                node->fix_geometry_level += 2;
            }

            for (auto child: node->children) {
                queue.push(child);
            }
        }
    }
    if (edit_type_ == CustomGraphicsScene::SIZE_ADJUST_NODE) {
        int node_idx;
        int page_idx;
        double fix_x, fix_y, fix_w, fix_h;
        for (auto item: old_scene->items()) {
            auto custom_item = dynamic_cast<CustomGraphicsItem *>(item);
            if (custom_item != nullptr && custom_item->IsFixGeometry()) {
                node_idx = custom_item->GetTreeNode()->index;
                page_idx = custom_item->GetPageIdx();
                fix_x = custom_item->scenePos().x();
                fix_y = custom_item->scenePos().y();
                fix_w = custom_item->rect().width();
                fix_h = custom_item->rect().height();
                break;
            }
            //find page idx, motherfucker
        }
        std::queue<CCombineTreeNode *> queue;
        queue.push(new_combine_tree[page_idx]);
        while (!queue.empty()) {
            auto node = queue.front();
            queue.pop();

            if (node->index == node_idx) {
                fix_geometry_node = node;
                fix_geometry_node->fix_geometry_level = 0;
                fix_geometry_node->fix_x = fix_x;
                fix_geometry_node->fix_y = fix_y;
                fix_geometry_node->fix_width = fix_w;
                fix_geometry_node->fix_height = fix_h;
                //std::cout << fix_x << " " << fix_y << " " << fix_w << " " << fix_h << std::endl;
                break;
            }

            for (int i = 0; i < node->children.size(); ++i) {
                queue.push(node->children[i]);
            }
        }
    } else if (edit_type_ == CustomGraphicsScene::MOVE_NODE) {
        auto source_item = old_scene->GetSourceItem();
        auto target_item = old_scene->GetTargetItem();
        CustomGraphicsScene::INSERT_DIRECTION insert_dir = old_scene->GetInsertDirection();
        int source_node_idx = source_item->GetTreeNode()->index;
        int target_node_idx = target_item->GetTreeNode()->index;
        int source_page_idx = source_item->GetPageIdx();
        int target_page_idx = target_item->GetPageIdx();

        CCombineTreeNode *source_node = nullptr;
        CCombineTreeNode *target_node = nullptr;
        std::queue<CCombineTreeNode *> queue;
        queue.push(new_combine_tree[source_page_idx]);
        while (!queue.empty()) {
            auto node = queue.front();
            queue.pop();

            if (node->index == source_node_idx) {
                source_node = node;
                break;
            }
            for (int i = 0; i < node->children.size(); ++i) {
                queue.push(node->children[i]);
            }
        }
        while (!queue.empty()) queue.pop();
        queue.push(new_combine_tree[target_page_idx]);
        while (!queue.empty()) {
            auto node = queue.front();
            queue.pop();

            if (node->index == target_node_idx) {
                target_node = node;
                break;
            }
            for (int i = 0; i < node->children.size(); ++i) {
                queue.push(node->children[i]);
            }
        }

        if (source_node != nullptr && target_node != nullptr) {
            auto add_node = source_node->DeepCopy();
            CombineTreeHandler::AddANodeToATree(add_node, target_node, insert_dir);
            CombineTreeHandler::CutTreeNode(source_node);
        }
        for (int i = 0; i < new_combine_tree.size(); ++i) {
            auto single_tree = new_combine_tree[i];
            while (single_tree->parent != nullptr) single_tree = single_tree->parent;
            new_combine_tree[i] = single_tree;
        }

    }
    else if (edit_type_ == CustomGraphicsScene::ADD_PADDING || edit_type_ == CustomGraphicsScene::ADD_TEXT) {

        auto target_item = old_scene->GetTargetItem();
        CustomGraphicsScene::INSERT_DIRECTION insert_dir = old_scene->GetInsertDirection();
        int target_node_idx = target_item->GetTreeNode()->index;
        int target_page_idx = target_item->GetPageIdx();
        CCombineTreeNode *target_node = nullptr;
        std::queue<CCombineTreeNode *> queue;

        queue.push(new_combine_tree[target_page_idx]);
        while (!queue.empty()) {
            auto node = queue.front();
            queue.pop();

            if (node->index == target_node_idx) {
                target_node = node;
                break;
            }
            for (int i = 0; i < node->children.size(); ++i) {
                queue.push(node->children[i]);
            }
        }

        if (target_node != nullptr) {
            auto add_node = new CCombineTreeNode();
            int new_node_index = CombineTreeHandler::GenerateNewIndex(target_node);
            add_node->parent = nullptr;
            add_node->index = new_node_index;
            add_node->node_type = LEAF;
            if (edit_type_ == CustomGraphicsScene::ADD_TEXT) add_node->node_present_type = TEXT;
            else if (edit_type_ == CustomGraphicsScene::ADD_PADDING) add_node->node_present_type = PADDING;
            CombineTreeHandler::AddANodeToATree(add_node, target_node, insert_dir);
        }
        for (int i = 0; i < new_combine_tree.size(); ++i) {
            auto single_tree = new_combine_tree[i];
            while (single_tree->parent != nullptr) single_tree = single_tree->parent;
            new_combine_tree[i] = single_tree;
        }

    } else if (edit_type_ == CustomGraphicsScene::REMOVE_NODE) {
        auto target_item = old_scene->GetTargetItem();
        int target_node_idx = target_item->GetTreeNode()->index;
        int target_page_idx = target_item->GetPageIdx();
        CCombineTreeNode *target_node = nullptr;
        std::queue<CCombineTreeNode *> queue;

        queue.push(new_combine_tree[target_page_idx]);
        while (!queue.empty()) {
            auto node = queue.front();
            queue.pop();

            if (node->index == target_node_idx) {
                target_node = node;
                break;
            }
            for (int i = 0; i < node->children.size(); ++i) {
                queue.push(node->children[i]);
            }
        }

        if (target_node != nullptr) {
            CombineTreeHandler::CutTreeNode(target_node);
        }
        for (int i = 0; i < new_combine_tree.size(); ++i) {
            auto single_tree = new_combine_tree[i];
            while (single_tree->parent != nullptr) single_tree = single_tree->parent;
            new_combine_tree[i] = single_tree;
        }
    }
    for (auto new_combine_tree_item: new_combine_tree) {
        CombineTreeHandler::MergeRelationNode(new_combine_tree_item);
        CombineTreeHandler::MergeTextNode(new_combine_tree_item);
        CombineTreeHandler::MergePaddingNode(new_combine_tree_item);
        CombineTreeHandler::MergeExtraParent(new_combine_tree_item);
    }


    layout_generator.SetCombineTree(new_combine_tree);

    bool success = layout_generator.GenerateLayout(model);
    if (success)
    {
        //if (fix_geometry_node != nullptr) fix_geometry_node->fix_geometry = false;
        combine_tree_stack_[layout_index_].push(combine_tree_list_[layout_index_]);
        combine_tree_list_[layout_index_] = new_combine_tree;
    }
    else
    {
        if(edit_type_ == CustomGraphicsScene::SIZE_ADJUST_NODE)
        {
            fix_geometry_node->fix_geometry_level++;
            layout_generator.SetCombineTree(new_combine_tree);
            success = layout_generator.GenerateLayout(model);
            if(success)
            {
                combine_tree_stack_[layout_index_].push(combine_tree_list_[layout_index_]);
                combine_tree_list_[layout_index_] = new_combine_tree;
            }
        }
    }


	LayoutPainter layout_painter;
    layout_painter.SetArticle(article_);
    layout_painter.SetTitleFontSizeRange(min_title_font_size_, max_title_font_size_);
    layout_painter.SetTextFontSizeRange(min_text_font_size_,max_text_font_size_);
    layout_painter.SetTitleFontType(title_font_type_);
    layout_painter.SetTextFontType(text_font_type_);
    layout_painter.SetW(paper_w);
    layout_painter.SetH(paper_h);
    layout_painter.CalTextAndTitleArea();
	layout_painter.SetLayoutTree(combine_tree_list_[layout_index_]);
	Preview scene = layout_painter.Draw();
    CustomGraphicsScene::PAINT_TYPE paint_type = content_view_ ?  CustomGraphicsScene::PAINT_TYPE::CONTENT : CustomGraphicsScene::PAINT_TYPE::NO_CONTENT;
    scene.actual_scene->SetPaintType(paint_type);
    scene.preview_scene->SetPaintType(paint_type);
	preview_list_[layout_index_] = scene;
    auto preview_list = ui.preview_list_scroll->widget()->findChildren<CustomGraphicsView*>();
    for(auto preview:preview_list)
    {
        if(preview->scene() == old_preview)   preview->setScene(scene.preview_scene);
    }
	UpdateOutputAndPreview();
    old_scene->disconnect();
	delete_scene_list_.push_back(old_preview);
	delete_scene_list_.push_back(old_scene);

    if(!success)
    {
        for(auto single_tree : new_combine_tree)
        {
            CCombineTreeNode::DeepDestroy(single_tree);
        }
        QMessageBox::information(NULL, "Info", "No solutions for the current edit.", QMessageBox::Ok);
    }
    else
        layout_estimator_.EstimateSingleLayout(new_combine_tree);

	if(edit_type_ == CustomGraphicsScene::ADD_PADDING )
	{
		ui.actionAddPadding->setChecked(false);
		ActiveAddPadding();
	}
	if(edit_type_ == CustomGraphicsScene::ADD_TEXT)
	{
		ui.actionAddText->setChecked(false);
		ActiveAddText();
	}

}

void LayoutShop::LoadArticle()
{
	////加载文章
	//std::string file_name_ = QFileDialog::getOpenFileName(this, tr("Load Article"), ".", tr("Articles(*.json)")).toStdString();//实际使用需取消该注释1
	//if (! file_name_.length() > 0)
	//{
	//	std::cout << "Choose a file!" << std::endl;
	//	return;
	//}

	//FILE* fp = fopen(file_name_.c_str(), "rb");
	//if (!fp) 
	//{
	//	std::cout << "open failed" << std::endl;
	//	return;
	//}
	//char* buf = new char[1024 * 16];
	//int n = fread(buf, 1, 1024 * 16, fp);
	//fclose(fp);
	//std::string file_content;
	//if (n >= 0) 
	//{
	//	file_content.append(buf, 0, n);
	//}
	//delete[]buf;
	//rapidjson::Document document;
	//document.Parse(file_content.c_str());
	//article_.title = document["title"].GetString();
	//article_.text_list.clear();
	//article_.img_list.clear();
	//const rapidjson::Value& text_value = document["text"];
	//for (size_t i = 0; i < text_value.Size(); i++)
	//	article_.text_list.push_back(text_value[i].GetString());

	//const rapidjson::Value& img_value = document["img"];
	//std::string prefix = file_name_.substr(0, file_name_.find_last_of('/') + 1);
	//for (size_t i = 0; i < img_value.Size(); i++)
	//{
	//	Img img;
	//	
	//	img.path = prefix + img_value[i]["path"].GetString();
	//	img.ratio = img_value[i]["ratio"].GetDouble();
	//	article_.img_list.push_back(img);
	//}
	//article_loaded_ = true;
	//std::cout << "artical loaded." << std::endl;
}

void LayoutShop::LoadAll()
{
	std::string file_name = QFileDialog::getOpenFileName(this, tr("Load All"), ".", tr("input(*.json)")).toStdString();
	if (!file_name.length() > 0)
	{
		std::cout << "Choose a file!" << std::endl;
		return;
	}

	FILE* fp = fopen(file_name.c_str(), "rb");
	if (!fp)
	{
		std::cout << "open failed" << std::endl;
		return;
	}
	char* buf = new char[1024 * 100];
	int n = fread(buf, 1, 1024 * 100, fp);
	fclose(fp);

	//ClearInput();

	std::string file_content;
	if (n >= 0)
	{
		file_content.append(buf, 0, n);
	}
	delete[]buf;
	rapidjson::Document document;
	document.Parse(file_content.c_str());
	//article_ = new Article();
	//template_list_.clear();

	//load title
	const rapidjson::Value& title_value = document["title"]["headline"];
	//std::string prefix = file_name_.substr(0, file_name_.find_last_of('/') + 1);
	std::string title = title_value.GetString();
	//article_->title_ = title;
    ui.title_input->setPlainText(title.data());
	//load text
	const rapidjson::Value& text_value = document["text"];
	//std::string prefix = file_name_.substr(0, file_name_.find_last_of('/') + 1);
    std::string whole_text;
    for (int i = 0; i < text_value.Size(); i++)
	{

		std::string str_val = text_value[i].GetString();
        whole_text.append(str_val);
        whole_text.append("\n");
	}
    ui.text_input->setPlainText(whole_text.data());
//	for (int i = 0; i < text_value.Size(); i++)
//	{
//		Text text;
//		std::string str_val = text_value[i].GetString();
//        std::istringstream in_stream(str_val);
//        //vector<string> vec;
//        std::string temp_str;
//        while(in_stream >> temp_str)
//        {
//            text.word_list.push_back(temp_str);
//        }
//        text.word_list[0] = std::string("    ").append(text.word_list[0]);
//        for(int i = 0; i < text.word_list.size() - 1; ++i)
//        {
//            text.word_list[i].append(" ");
//        }
//		article_->text_list_.push_back(text);
//	}

	//load img
	std::string prefix = file_name.substr(0, file_name.find_last_of('/') + 1);
	const rapidjson::Value& img_value = document["img"];
    for (int i = 0; i < img_value.Size(); i++)
    {
        QPixmap img;
        std::string path = prefix + img_value[i].GetString();
        bool read_img = img.load(QString::fromStdString(path));
        if (read_img == false) std::cout << "fail reading image" << std::endl;
        else ui.picture_box->AddPicture(img, QString(path.data()));
    }
//	for (int i = 0; i < img_value.Size(); i++)
//	{
//		QPixmap img;
//		std::string path = prefix + img_value[i].GetString();
//		bool read_img = img.load(QString::fromStdString(path));
//		if (read_img == false) std::cout << "fail reading image" << std::endl;
//		else article_->img_list_.push_back(img);
//	}

	//load layout
	const rapidjson::Value& layout_value = document["layout"];
    for (int i = 0; i < layout_value.Size(); i++)
    {
        QPixmap img;
        std::string path = prefix + layout_value[i]["path"].GetString();
        std::string img_path = prefix + layout_value[i]["img"].GetString();
        bool read_img = img.load(QString::fromStdString(img_path));
        if (read_img == false) std::cout << "fail reading image" << std::endl;
        else
        {
            ui.template_box->AddTemplate(img,path.data());
        }
    }

//     ref_list_.clear();
//     if(document.HasMember("ref"))
//     {
//         const rapidjson::Value& ref_value = document["ref"];
//         for (int i = 0; i < ref_value.Size(); i++)
//         {
//             int para, word_start, word_end;
//             int img_idx;
//             para = ref_value[i]["text"][0].GetInt();
//             word_start = ref_value[i]["text"][1].GetInt();
//             word_end = ref_value[i]["text"][2].GetInt();
//             img_idx = ref_value[i]["img"].GetInt();
//             ref_list_.push_back({{para, word_start, word_end}, img_idx});
//         }
//     }



//	for (int i = 0; i < layout_value.Size(); i++)
//	{
//		QPixmap img;
//		std::string path = prefix + layout_value[i].GetString();
//		template_list_.push_back((new CLayoutTree()));
//		template_list_[template_list_.size() - 1]->ReadFromFile(path);
//	}

	//article_loaded_ = true;
	std::cout << "Article loaded." << std::endl;
	std::cout << "Layout loaded." << std::endl;
}

void LayoutShop::OpenLayoutGroup()
{
	//file_name表示其绝对路径
	static const QString defaultPath("DEFAULT_LAYOUT_GROUP_PATH");
	QSettings thisSetting;
	QString path = thisSetting.value(defaultPath).toString();
	QString filter = "(*.grp);;(*.*)";
	QString file_name = QFileDialog::getOpenFileName(this, tr("Open layout group"), path, filter);//实际使用需取消注释2
	std::string fileName = file_name.toStdString();//实际使用需取消注释3
	//std::string fileName = "E:/projects/Content_aware_layout/Data/group/04grp_6_8/01grp_6.grp";//读入固定模板
	std::ifstream infile;
	infile.open(fileName.data());
	std::string each_line_string;

	if (fileName.length() > 0)
	{
		//可以加载多次
		for (int i = 0; i < template_list_.size(); i++)
		{
			if (template_list_[i] != nullptr)
			{
				delete template_list_[i];
                template_list_[i] = nullptr;
			}
		}

		template_list_.clear();

		//得到路径前缀
		int flag_pos = -1;
		for (int i = 0; i < fileName.size(); i++)
		{
			if (fileName[i] == '/')
			{
				flag_pos = i;
			}
		}
		std::string pre_string;
		if (flag_pos != -1)
		{
			pre_string.clear();
			for (int i = 0; i <= flag_pos; i++)
			{
				pre_string += fileName[i];
			}
		}
		//得到路径前缀

		while (getline(infile, each_line_string))
		{
#ifdef __APPLE__
            if(each_line_string.back() == '\r')
                each_line_string.erase(each_line_string.size() - 1);
#endif
			template_list_.push_back((new CLayoutTree()));
			std::string new_file_path = pre_string + each_line_string;
			template_list_[template_list_.size() - 1]->ReadFromFile(new_file_path);
		}
		std::string name = fileName;
		QDir crtDir;
		size_t found = name.find_last_of('/');
		name = name.substr(0, found);
		QString nameQ = QString::fromStdString(name);
		thisSetting.setValue(defaultPath, crtDir.absoluteFilePath(nameQ));
		std::cout << "group loaded." << std::endl;
	}
	return;
}

void LayoutShop::OpenNextLayout()
{

//    TestGraphScore();
//	QString file_name = QFileDialog::getOpenFileName(this, tr("Read txt"), ".", tr("Texts(*.txt *.lay)"));
//	std::string fileName = file_name.toStdString();
//
//	if (fileName.length() > 0)
//	{
//		if (template_list_.size() == 3)
//		{
//			for (int i = 0; i < 3; i++)
//			{
//				if (template_list_[i] != nullptr)
//				{
//					delete template_list_[i];
//                    template_list_[i] = nullptr;
//				}
//			}
//			template_list_.clear();
//		}
//		template_list_.push_back((new CLayoutTree()));
//		template_list_[template_list_.size() - 1]->ReadFromFile(fileName);
//	}
//	return;
}

void SingleThreadGenerateLayout(std::vector<std::vector<CCombineTreeNode*>> single_thread_tree_list,
	std::vector<std::vector<CCombineTreeNode*>>& valid_layout_list,
	Article* article,double paper_w,double paper_h)
{

	LayoutGeneratorMultiPage layout_generator;
	layout_generator.SetArticle(article);
	layout_generator.SetW(paper_w);
	layout_generator.SetH(paper_h);
	GRBEnv env = GRBEnv(true);
	env.start();
	GRBModel model = GRBModel(env);
	model.set("NonConvex", "2");
	model.set("TimeLimit", "0.1");
	model.set("OutputFlag", "0");
	for (int i = 0; i < single_thread_tree_list.size(); i++)
	{
		layout_generator.SetCombineTree( single_thread_tree_list[i] );
		if (layout_generator.GenerateLayout(model))
		{
			valid_layout_list.push_back(single_thread_tree_list[i]);
			//std::cout << "success" << std::endl;
		}
		else
		{
			for (auto tree : single_thread_tree_list[i])
			{
				delete tree;
				tree = nullptr;
			}
			single_thread_tree_list[i].clear();
		}
	}
}


void LayoutShop::GenerateLayout()
{
//	if (article_ == nullptr)
//	{
//		std::cout << "Choose an article" << std::endl;
//		return;
//	}
//	if (template_list_.size() < 3)
//	{
//		std::cout << "Please choose 3 layouts" << std::endl;
//		return;
//	}

	ClearOutput();
	combine_tree_root_ = combine_tree_handler_.GenerateCombineTree(template_list_);
	combine_tree_list_.clear();

    if(!CheckCombineTreeSatisfied(combine_tree_root_))
        return;
    int text_num_needed;
    if(paragraph_) text_num_needed = article_->text_list_.size();
    else text_num_needed = -1;
	auto candidate_list  = combine_tree_handler_.GetCombineTreeCandidate(combine_tree_root_, article_->img_list_.size(), text_num_needed);
	for (auto candidate : candidate_list)
	{
		combine_tree_list_.push_back({ candidate });
	}
	std::cout << "candidate num: " << combine_tree_list_.size() << std::endl;
	//std::vector<CCombineTreeNode*> valid_solutions;
	//LayoutGenerator layout_generator;
	//layout_generator.SetArticle(article_);

    if(ui.img_order_check->isChecked())
    {
        for(auto combine_tree : combine_tree_list_)
        {
            PresetImgCorres(combine_tree);
        }
    }

	double horizontalDPI = QPaintDevice::physicalDpiX();
	double verticalDPI = QPaintDevice::physicalDpiY();
	int paper_w = horizontalDPI / 2.54 * real_w_;
	int paper_h = verticalDPI / 2.54 * real_h_;
	if(multi_thread_)
	{
		int thread_count;
		if (combine_tree_list_.size() <= 50) thread_count = 1;
		else if (combine_tree_list_.size() <= 100) thread_count = 2;
		else if (combine_tree_list_.size() <= 150) thread_count = 3;
		else thread_count = 4;
		std::vector<std::thread> thread_list(thread_count);
		std::vector<std::vector<std::vector<CCombineTreeNode*>>> valid_layout_list(thread_count, std::vector<std::vector<CCombineTreeNode*> >());
		for (int thread_i = 0; thread_i < thread_count; ++thread_i)
		{
			int start_i, end_i;
			start_i = thread_i * (combine_tree_list_.size() / thread_count);
			if (thread_i == thread_count - 1) end_i = combine_tree_list_.size();
			else end_i = (thread_i + 1) * (combine_tree_list_.size() / thread_count);
			std::vector<std::vector<CCombineTreeNode*>> sub_list;
			for (int i = start_i; i < end_i; ++i) sub_list.push_back(combine_tree_list_[i]);
			thread_list[thread_i] = std::thread(SingleThreadGenerateLayout, sub_list, std::ref(valid_layout_list[thread_i]), article_,paper_w,paper_h);
		}
		for (int thread_i = 0; thread_i < thread_count; ++thread_i) thread_list[thread_i].join();
		combine_tree_list_.clear();
		for (int thread_i = 0; thread_i < thread_count; ++thread_i)
		{
			for (int i = 0; i < valid_layout_list[thread_i].size(); ++i)
			{
				combine_tree_list_.push_back(valid_layout_list[thread_i][i]);
			}
			valid_layout_list[thread_i].clear();
		}
	}
	else
	{
        //int min_title_size,max_title_size,min_text_size,max_text_size;
//        std::string title_font,text_font;
//        bool img_order,multi_page;
		std::vector<std::vector<CCombineTreeNode*>> valid_layout_list;
		LayoutGeneratorMultiPage layout_generator;

        layout_generator.SetArticle(article_);
        layout_generator.SetTitleFontSizeRange(min_title_font_size_, max_title_font_size_);
        layout_generator.SetTextFontSizeRange(min_text_font_size_,max_text_font_size_);
        layout_generator.SetTitleFontType(title_font_type_);
        layout_generator.SetTextFontType(text_font_type_);
        layout_generator.SetW(paper_w);
        layout_generator.SetH(paper_h);
		// layout_generator.SetW(1803);
		// layout_generator.SetH(2382);
		GRBEnv env = GRBEnv(true);
		env.start();
		GRBModel model = GRBModel(env);
		model.set("NonConvex", "2");
		model.set("TimeLimit", "0.1");
		model.set("OutputFlag", "0");
		for (int i = 0; i < combine_tree_list_.size(); i++)
		{
			layout_generator.SetCombineTree(combine_tree_list_[i] );
			if (layout_generator.GenerateLayout(model))
			{
				valid_layout_list.push_back(combine_tree_list_[i]);
			}
			else
			{
				for (auto tree : combine_tree_list_[i])
				{
					delete tree;
					tree = nullptr;
				}
				combine_tree_list_[i].clear();
			}
		}
		combine_tree_list_.clear();
		combine_tree_list_ = valid_layout_list;
	}

    layout_estimator_.SetH(paper_h);
    layout_estimator_.SetW(paper_w);
    //combine_tree_list_ = layout_estimator_.EstimateSinglePage(combine_tree_list_, cross_estimate_mode_);
    combine_tree_list_ = layout_estimator_.EstimateSinglePage(combine_tree_list_, cross_estimate_mode_);

	//paint layout
	LayoutPainter layout_painter;
    layout_painter.SetArticle(article_);
    layout_painter.SetTitleFontSizeRange(min_title_font_size_, max_title_font_size_);
    layout_painter.SetTextFontSizeRange(min_text_font_size_,max_text_font_size_);
    layout_painter.SetTitleFontType(title_font_type_);
    layout_painter.SetTextFontType(text_font_type_);
    layout_painter.SetW(paper_w);
    layout_painter.SetH(paper_h);
    layout_painter.CalTextAndTitleArea();
    std::vector<std::vector<CCombineTreeNode*>> list;
//    for( int i = 0; i< 4; i ++)
//    {
//        list.push_back(combine_tree_list_[i]);
//    }
//    combine_tree_list_ = list;

//

	for(auto combine_tree : combine_tree_list_)
	{
		layout_painter.SetLayoutTree(combine_tree);
		Preview scene = layout_painter.Draw();
        CustomGraphicsScene::PAINT_TYPE paint_type = content_view_ ?  CustomGraphicsScene::PAINT_TYPE::CONTENT : CustomGraphicsScene::PAINT_TYPE::NO_CONTENT;
        scene.actual_scene->SetPaintType(paint_type);
        scene.preview_scene->SetPaintType(paint_type);
		preview_list_.push_back(scene);
	}


    layout_num_ = combine_tree_list_.size();
	combine_tree_stack_ = std::vector<std::stack<std::vector<CCombineTreeNode*>>>(layout_num_);
	if(layout_num_ > 0)
	{
		layout_index_ = 0;
		//preview_page_ = 0;
		//ui.output_view->setScene(preview_list_[layout_index_]);
		//page_num_ = layout_num_ / 4 + 1;
	} 
	else
	{
		layout_index_ = -1;
		//preview_page_ = -1;
		//ui.output_view->setScene(nullptr);
		//page_num_ = 0;
	}
    //create preview widget
    CreatePreviewWidgets();
	UpdateOutputAndPreview();
    SetDefaultScale(paper_h);
    ShowRefineWidget();
}

void LayoutShop::GenerateLayoutMultiPage()
{
	ClearOutput();

	int letters_per_page = 2000;
	int letter_count = 0;
	for(auto text:article_->text_list_)
	{
		for (auto word : text.word_list)
			letter_count += word.size();
	}
	int target_page = letter_count / letters_per_page + 1;
	std::cout << "target page:" << target_page << std::endl;

	combine_tree_root_ = combine_tree_handler_.GenerateCombineTree(template_list_);
	combine_tree_list_.clear();
	combine_tree_list_ = combine_tree_handler_.GetCombineTreeCandidateMultiPage2(combine_tree_root_, article_->img_list_.size(), target_page);

    std::cout << "candidate num: " << combine_tree_list_.size() << std::endl;
	/*for(int i=0;i<combine_tree_list_.size();++i)
	{
		for (int j = i + 1; j < combine_tree_list_.size(); ++j) combine_tree_handler_.CountDistanceBetweenTwoTrees(combine_tree_list_[i], combine_tree_list_[j]);
	}*/
	//combine_tree_handler_.CountDistanceBetweenTwoTrees(combine_tree_list_[0], combine_tree_list_[1]);
    if(ui.img_order_check->isChecked())
    {
        for(auto combine_tree : combine_tree_list_)
        {
            PresetImgCorres(combine_tree);
        }
    }

	double horizontalDPI = QPaintDevice::physicalDpiX();
	double verticalDPI = QPaintDevice::physicalDpiY();
	int paper_w = horizontalDPI / 2.54 * real_w_;
	int paper_h = verticalDPI / 2.54 * real_h_;
	if (multi_thread_)
	{
		int thread_count;
		if (combine_tree_list_.size() <= 50) thread_count = 1;
		else if (combine_tree_list_.size() <= 100) thread_count = 2;
		else if (combine_tree_list_.size() <= 150) thread_count = 3;
		else thread_count = 4;
		std::vector<std::thread> thread_list(thread_count);
		std::vector<std::vector<std::vector<CCombineTreeNode*>>> valid_layout_list(thread_count, std::vector<std::vector<CCombineTreeNode*> >());
		for (int thread_i = 0; thread_i < thread_count; ++thread_i)
		{
			int start_i, end_i;
			start_i = thread_i * (combine_tree_list_.size() / thread_count);
			if (thread_i == thread_count - 1) end_i = combine_tree_list_.size();
			else end_i = (thread_i + 1) * (combine_tree_list_.size() / thread_count);
			std::vector<std::vector<CCombineTreeNode*>> sub_list;
			for (int i = start_i; i < end_i; ++i) sub_list.push_back(combine_tree_list_[i]);
			thread_list[thread_i] = std::thread(SingleThreadGenerateLayout, sub_list, std::ref(valid_layout_list[thread_i]), article_, paper_w, paper_h);
		}
		for (int thread_i = 0; thread_i < thread_count; ++thread_i) thread_list[thread_i].join();
		combine_tree_list_.clear();
		for (int thread_i = 0; thread_i < thread_count; ++thread_i)
		{
			for (int i = 0; i < valid_layout_list[thread_i].size(); ++i)
			{
				combine_tree_list_.push_back(valid_layout_list[thread_i][i]);
			}
			valid_layout_list[thread_i].clear();
		}
	}
	else
	{
        int min_title_size,max_title_size,min_text_size,max_text_size;
        std::string title_font,text_font;
        bool img_order,multi_page;
        std::vector<std::vector<CCombineTreeNode*>> valid_layout_list;
        LayoutGeneratorMultiPage layout_generator;
        min_title_size = ui.min_title_font_size_box->value();
        max_title_size = ui.max_title_font_size_box->value();
        min_text_size = ui.min_text_font_size_box->value();
        max_text_size = ui.max_text_font_size_box->value();
        title_font = ui.title_font_combo->currentText().toStdString();
        text_font = ui.text_font_combo->currentText().toStdString();
        img_order = ui.img_order_check->isChecked();

        layout_generator.SetArticle(article_);
        layout_generator.SetTitleFontSizeRange(min_title_font_size_, max_title_font_size_);
        layout_generator.SetTextFontSizeRange(min_text_font_size_,max_text_font_size_);
        layout_generator.SetTitleFontType(title_font_type_);
        layout_generator.SetTextFontType(text_font_type_);
        layout_generator.SetW(paper_w);
        layout_generator.SetH(paper_h);

		GRBEnv env = GRBEnv(true);
		env.start();
		GRBModel model = GRBModel(env);
		model.set("NonConvex", "2");
		model.set("TimeLimit", "1.0");
		model.set("OutputFlag", "0");
		for (int i = 0; i < combine_tree_list_.size(); i++)
		{
			layout_generator.SetCombineTree(combine_tree_list_[i]);
			if (layout_generator.GenerateLayout(model))
			{
				//temp method
				valid_layout_list.push_back(combine_tree_list_[i]);
			}
			else
			{
				for (auto tree : combine_tree_list_[i])
				{
					delete tree;
					tree = nullptr;
				}
				combine_tree_list_[i].clear();
			}
		}
		combine_tree_list_.clear();
		combine_tree_list_ = valid_layout_list;
	}
    layout_estimator_.SetH(paper_h);
    layout_estimator_.SetW(paper_w);
    combine_tree_list_ = layout_estimator_.EstimateSinglePage(combine_tree_list_, cross_estimate_mode_);
	//paint layout
	LayoutPainter layout_painter;

    layout_painter.SetArticle(article_);
    layout_painter.SetTitleFontSizeRange(min_title_font_size_, max_title_font_size_);
    layout_painter.SetTextFontSizeRange(min_text_font_size_,max_text_font_size_);
    layout_painter.SetTitleFontType(title_font_type_);
    layout_painter.SetTextFontType(text_font_type_);
    layout_painter.SetW(paper_w);
    layout_painter.SetH(paper_h);
    layout_painter.CalTextAndTitleArea();

	for (auto combine_tree : combine_tree_list_)
	{
		layout_painter.SetLayoutTree(combine_tree);
		Preview scene = layout_painter.Draw();
        CustomGraphicsScene::PAINT_TYPE paint_type = content_view_ ?  CustomGraphicsScene::PAINT_TYPE::CONTENT : CustomGraphicsScene::PAINT_TYPE::NO_CONTENT;
        scene.actual_scene->SetPaintType(paint_type);
        scene.preview_scene->SetPaintType(paint_type);
		preview_list_.push_back(scene);
	}
	layout_num_ = combine_tree_list_.size();
	combine_tree_stack_ = std::vector<std::stack<std::vector<CCombineTreeNode*>>>(layout_num_);
	if (layout_num_ > 0)
	{
		layout_index_ = 0;
		//preview_page_ = 0;
		//ui.output_view->setScene(preview_list_[layout_index_]);
		//page_num_ = layout_num_ / 4 + 1;
	}
	else
	{
		layout_index_ = -1;
		//preview_page_ = -1;
		//ui.output_view->setScene(nullptr);
		//page_num_ = 0;
	}

    CreatePreviewWidgets();

	UpdateOutputAndPreview();
    SetDefaultScale(paper_h);
    ShowRefineWidget();
}

//void LayoutShop::PageUp()
//{
//	if(preview_page_ > 0)
//	{
//		preview_page_--;
//		UpdateOutputAndPreview(false);
//	}
//
//}
//void LayoutShop::PageDown()
//{
//	if(preview_page_ >= 0 && preview_page_ < page_num_ - 1)
//	{
//		preview_page_++;
//		UpdateOutputAndPreview(false);
//	}
//
//}

void LayoutShop::Undo()
{
	if(layout_index_ >= 0 && layout_index_ < layout_num_)
	{
		if(!combine_tree_stack_[layout_index_].empty())
		{
			auto old_scene = preview_list_[layout_index_].actual_scene;
			auto old_preview = preview_list_[layout_index_].preview_scene;
			auto old_combine_tree = combine_tree_list_[layout_index_];
			auto new_combine_tree = combine_tree_stack_[layout_index_].top();
			combine_tree_stack_[layout_index_].pop();
			combine_tree_list_[layout_index_] = new_combine_tree;
			LayoutPainter layout_painter;
			double horizontalDPI = QPaintDevice::physicalDpiX();
			double verticalDPI = QPaintDevice::physicalDpiY();
			int paper_w = horizontalDPI / 2.54 * real_w_;
			int paper_h = verticalDPI / 2.54 * real_h_;
            layout_painter.SetArticle(article_);
            layout_painter.SetTitleFontSizeRange(min_title_font_size_, max_title_font_size_);
            layout_painter.SetTextFontSizeRange(min_text_font_size_,max_text_font_size_);
            layout_painter.SetTitleFontType(title_font_type_);
            layout_painter.SetTextFontType(text_font_type_);
            layout_painter.SetW(paper_w);
            layout_painter.SetH(paper_h);
            layout_painter.CalTextAndTitleArea();
            layout_painter.SetLayoutTree(combine_tree_list_[layout_index_]);
			Preview scene = layout_painter.Draw();
            CustomGraphicsScene::PAINT_TYPE paint_type = content_view_ ?  CustomGraphicsScene::PAINT_TYPE::CONTENT : CustomGraphicsScene::PAINT_TYPE::NO_CONTENT;
            scene.actual_scene->SetPaintType(paint_type);
            scene.preview_scene->SetPaintType(paint_type);
			preview_list_[layout_index_] = scene;
            auto preview_list = ui.preview_list_scroll->widget()->findChildren<CustomGraphicsView*>();
            for(auto preview:preview_list)
            {
                if(preview->scene() == old_preview)   preview->setScene(scene.preview_scene);
            }
			UpdateOutputAndPreview();
			DestroyScene(old_preview);
			DestroyScene(old_scene);
			for(auto single_tree:old_combine_tree)
			{
				if (single_tree != nullptr) CCombineTreeNode::DeepDestroy(single_tree);
			}
		}
	}
}

void LayoutShop::SetSceneEditType()
{
	auto scene = ui.output_view->scene();
	if(scene != nullptr)
	{
		dynamic_cast<CustomGraphicsScene*>(scene)->SetEditType(edit_type_);
		if (edit_type_ == CustomGraphicsScene::REMOVE_NODE)
		{
			ui.output_view->viewport()->setCursor(QCursor(QPixmap(":/LayoutShop/resources/ui-icon/remove-cursor.png")));
			//ui.output_view->viewport()->setCursor(QCursor(QPixmap("C:\\Users\\lijia\\Downloads\\layout-icon\\add_text.png")));
		}
		else
		{
			ui.output_view->viewport()->setCursor(Qt::ArrowCursor);
		}
        scene->update();
	}

}

void LayoutShop::ClearInput()
{
	ui.title_input->clear();
    ui.title_font_combo->setCurrentIndex(0);
    ui.min_title_font_size_box->setValue(20);
    ui.max_title_font_size_box->setValue(30);
    ui.text_input->clear();
    ui.text_font_combo->setCurrentIndex(0);
    ui.min_text_font_size_box->setValue(12);
    ui.max_text_font_size_box->setValue(20);
    ui.img_order_check->setChecked(false);
    ui.multi_page_check->setChecked(false);
    ui.picture_box->RemoveAll();
    template_dialog_->RemoveAll();

}

void LayoutShop::ClearOutput()
{
	ui.output_view->setScene(nullptr);
	for (auto preview : preview_list_)
	{
		if (preview.actual_scene != nullptr) DestroyScene(preview.actual_scene);
		if (preview.preview_scene != nullptr) DestroyScene(preview.preview_scene);
	}
	preview_list_.clear();
    auto preview_widget_list = ui.preview_list_scroll->widget()->findChildren<CustomGraphicsView*>();
    for(auto & preview : preview_widget_list)
    {
        delete preview;
    }
	if(combine_tree_root_ != nullptr) CCombineTreeNode::DeepDestroy(combine_tree_root_);
	combine_tree_root_ = nullptr;
	for(auto combine_tree:combine_tree_list_)
	{
		for(auto single_tree:combine_tree)
		{
			if(single_tree != nullptr) CCombineTreeNode::DeepDestroy(single_tree);
		}
        combine_tree.clear();
	}
	combine_tree_list_.clear();

	for(auto stack : combine_tree_stack_)
	{
		while(!stack.empty())
		{
			auto combine_tree = stack.top();
			stack.pop();
			for (auto single_tree : combine_tree)
			{
				if (single_tree != nullptr) CCombineTreeNode::DeepDestroy(single_tree);
			}
		}
		
	}
	combine_tree_stack_.clear();

	for(auto scene : delete_scene_list_)
	{
		if(scene != nullptr) DestroyScene(scene);
	}
	delete_scene_list_.clear();

	layout_index_ = -1; 
	//preview_page_ = -1;
	layout_num_ = 0; 
	//page_num_ = 0;
	UpdateOutputAndPreview();
}

void LayoutShop::DestroyScene(CustomGraphicsScene* scene)
{
	for (auto item : scene->items())
	{
		delete item;
	}
	delete scene;
}

void LayoutShop::UpdateOutputAndPreview()
{
	//update output view
	if (layout_index_ >= 0 && layout_index_ < layout_num_)
	{
		/*auto old_scene = ui.output_view->scene();
		if(old_scene != nullptr)
		{
			QObject::disconnect(old_scene, 0,this, 0);
		}*/
		ui.output_view->setScene(preview_list_[layout_index_].actual_scene);
		SetSceneEditType();
		QObject::connect(preview_list_[layout_index_].actual_scene,  &CustomGraphicsScene::ReOptimizeReady,
			this, &LayoutShop::ReOptimize);
		/*QObject::connect(preview_list_[layout_index_].actual_scene, SIGNAL(ReOptimizeReady()),
			this,SLOT(ReOptimize()));*/
	}
//
	//update layout pre and next buttons
	if (layout_index_ > 0 && layout_index_ < layout_num_) ui.pre_layout_button->setEnabled(true);
	else ui.pre_layout_button->setEnabled(false);
	if (layout_index_ >= 0 && layout_index_ < layout_num_ - 1) ui.next_layout_button->setEnabled(true);
	else ui.next_layout_button->setEnabled(false);
//
	//update layout index input
	if (layout_index_ >= 0 && layout_index_ < layout_num_)
	{
		ui.layout_index_box->setValue(layout_index_ + 1);
		ui.layout_index_box->setSuffix(QString(" / ") + std::to_string(layout_num_).data());
		ui.layout_index_box->setMinimum(1);
		ui.layout_index_box->setMaximum(layout_num_);
		ui.layout_index_box->setEnabled(true);
	}
	else
	{
		ui.layout_index_box->clear();
		ui.layout_index_box->setEnabled(false);
	}

    auto preview_widget_list = ui.preview_list_scroll->widget()->findChildren<CustomGraphicsView*>();
    for(int i = 0; i < preview_widget_list.size(); ++i)
    {
        auto view = preview_widget_list[i];
        if (i == layout_index_) view->SetSelected(true);
        else view->SetSelected(false);
    }

    SetRefineWidgetValue();


}

void LayoutShop::SaveOutput()
{
    const QString selected_dir = QFileDialog::getExistingDirectory(this, tr("Save layouts"), ".");
    if (selected_dir.isEmpty()) return;
    std::string target_dir = selected_dir.toStdString() + "/";
    std::vector<std::string> path_list;

    std::ofstream img_list(target_dir + "img_data_list.txt");
    std::ofstream layout_list(target_dir + "layout_data_list.txt");
    int count = 0;
    for (int i = 0; i < combine_tree_list_.size();++i)
    {
        auto tree = combine_tree_list_[i];
        auto layout = tree[0];
        count++;
        std::string main_dir = target_dir + std::to_string(count);
        mkdir(main_dir.c_str(), 0777);

        QGraphicsView render_view;
        render_view.setScene(preview_list_[i].actual_scene);
        //render1_view.resize(render1_view.sceneRect().toRect().width(),render1_view.sceneRect().toRect().height());
//        render0_view.scale(210.0 / render0_view.sceneRect().toRect().width(), 297.0 / render0_view.sceneRect().toRect().height());
//        render0_view.resize(210.0, 297);
        double horizontalDPI = QPaintDevice::physicalDpiX();
        double verticalDPI = QPaintDevice::physicalDpiY();
        int paper_w = horizontalDPI / 2.54 * real_w_;
        int paper_h = verticalDPI / 2.54 * real_h_;
        W = paper_w;
        H = paper_h;
       // render_view.scale(  paper_w / render_view.sceneRect().toRect().width(), paper_h / render_view.sceneRect().toRect().height());
        render_view.resize(paper_w, paper_h);
        QPixmap pixmap = render_view.grab();
        std::string render_path = main_dir + "/example.png";
        pixmap.save(render_path.c_str(),"png");

        //RefineLayout(layout);
        SaveFiles(layout,main_dir);
        img_list << std::to_string(count) + "/render.png"
                 << std::endl;

        layout_list << std::to_string(count) + "/layout.json"
                    << std::endl;
        //  if(file.path().extension().string())
    }
    img_list.close();
    layout_list.close();
}


rapidjson::Value LayoutShop::CreateJsonDFS(CCombineTreeNode *layout_node, rapidjson::MemoryPoolAllocator<> &allocator)
{
    rapidjson::Value json_value (rapidjson::Type::kObjectType);

    json_value.AddMember("index", layout_node->index,allocator);
    json_value.AddMember("x", layout_node->x,allocator);
    json_value.AddMember("y", layout_node->y,allocator);
    json_value.AddMember("width", layout_node->width,allocator);
    json_value.AddMember("height", layout_node->height,allocator);
    json_value.AddMember("type", layout_node->node_present_type,allocator);
    json_value.AddMember("relation", layout_node->node_relation,allocator);

    rapidjson::Value json_children (rapidjson::Type::kArrayType);
    for(int i = 0;i < layout_node->children.size();++i)
    {
        rapidjson::Value json_child = CreateJsonDFS(layout_node->children[i],allocator);
        json_children.PushBack(json_child,allocator);
    }
    json_value.AddMember("children", json_children,allocator);

    return json_value;
}

//rapidjson::Value CreateJsonDFS(CNode *layout_node, rapidjson::MemoryPoolAllocator<> &allocator)
//{
//    rapidjson::Value json_value (rapidjson::Type::kObjectType);
//
//    json_value.AddMember("index", layout_node->index,allocator);
//    json_value.AddMember("x", layout_node->x,allocator);
//    json_value.AddMember("y", layout_node->y,allocator);
//    json_value.AddMember("width", layout_node->width,allocator);
//    json_value.AddMember("height", layout_node->height,allocator);
//    json_value.AddMember("type", layout_node->node_present_type,allocator);
//    json_value.AddMember("relation", layout_node->node_relation,allocator);
//
//    rapidjson::Value json_children (rapidjson::Type::kArrayType);
//    for(int i = 0;i < layout_node->children.size();++i)
//    {
//        rapidjson::Value json_child = CreateJsonDFS(layout_node->children[i],allocator);
//        json_children.PushBack(json_child,allocator);
//    }
//    json_value.AddMember("children", json_children,allocator);
//
//    return json_value;
//}

void LayoutShop::RefineLayout(CCombineTreeNode *layout)
{
    std::queue<CCombineTreeNode*> node_queue;
    node_queue.push(layout);
    while(!node_queue.empty())
    {
        auto node = node_queue.front();
        node_queue.pop();
        if(node->node_present_type == NODE_PRESENT_TYPE::NONLABEL)
        {
            if (node->bottom_align_node.size()>0 && node->top_align_node.size()>0)  node->node_relation = NODE_RELATION::HORIZONTAL;
            else if(node->left_align_node.size()>0 && node->right_align_node.size()>0) node->node_relation = NODE_RELATION::VERTICAL;
        }
        for (int i = 0; i < node->children.size(); i++)
        {
            node_queue.push(node->children[i]);
        }
    }
//rm move
    double move_x,move_y;
    move_x = layout->x;
    move_y = layout->y;
    node_queue.push(layout);
    while(!node_queue.empty())
    {
        auto node = node_queue.front();
        node_queue.pop();
        node->x -= move_x;
        node->y -= move_y;
        for (int i = 0; i < node->children.size(); i++)
        {
            node_queue.push(node->children[i]);
        }
    }
//fit in the page
    double page_w = 630, page_h = 891;
    double margin_min_x = 30, margin_min_y = 30;
    if(layout->width > page_w - 2 * margin_min_x || layout->height > page_h - 2 * margin_min_y)
    {
        double ratio = layout->width / layout->height;
        double page_ratio = page_w / page_h;
        double resize_ratio;
        if(ratio >= page_ratio)
        {
            resize_ratio = (page_w - 2 * margin_min_x) / layout->width;
        }
        else
        {
            resize_ratio = (page_h - 2 * margin_min_y) / layout->height;
        }
        node_queue.push(layout);
        while(!node_queue.empty())
        {
            auto node = node_queue.front();
            node_queue.pop();
            node->width *= resize_ratio;
            node->height *= resize_ratio;
            node->x *= resize_ratio;
            node->y *= resize_ratio;
            for (int i = 0; i < node->children.size(); i++)
            {
                node_queue.push(node->children[i]);
            }
        }
    }

    move_x = (page_w - layout->width) / 2;
    move_y = (page_h - layout->height) / 2;
    node_queue.push(layout);
    while(!node_queue.empty())
    {
        auto node = node_queue.front();
        node_queue.pop();
        node->x += move_x;
        node->y += move_y;
        for (int i = 0; i < node->children.size(); i++)
        {
            node_queue.push(node->children[i]);
        }
    }
}

void LayoutShop::GenerateButtonPush()
{
//
//    SaveGRIDSLayout();
//    return;

    if(article_ != nullptr) delete article_;
    article_ = nullptr;
    for (auto layout : template_list_)
    {
        if(layout != nullptr)	layout->DestroyAllData();
    }
    template_list_.clear();

    /* read article */
    article_ = new Article();
    std::string full_title = ui.title_input->document()->toPlainText().toStdString();
    std::string full_text = ui.text_input->document()->toPlainText().toStdString();
    std::vector<std::string> text_list;
    std::stringstream ss(full_text);
    std::string para;
    while (std::getline(ss, para, '\n'))
    {
        text_list.push_back(para);
    }
    std::istringstream title_stream(full_title);
    //vector<string> vec;
    std::string temp_str;

    std::vector<std::string> title_word_list;
    while(title_stream >> temp_str)
    {
        title_word_list.push_back(temp_str);
    }
    if(title_word_list.size() > 0)
    {
        article_->title_.append(title_word_list[0]);
        for(int i = 1; i < title_word_list.size(); ++i)
        {
            article_->title_.append(" ");
            article_->title_.append(title_word_list[i]);
        }
    }

    for (int i = 0; i < text_list.size(); i++)
    {
        Text text;
        std::istringstream text_stream(text_list[i]);
        //vector<string> vec;
        while(text_stream >> temp_str)
        {
            text.word_list.push_back(temp_str);
        }
        if(text.word_list.size() > 0)
        {
            text.word_list[0] = std::string("    ").append(text.word_list[0]);
            for(int i = 0; i < text.word_list.size() - 1; ++i)
            {
                text.word_list[i].append(" ");
            }

            article_->text_list_.push_back(text);
        }
    }

    for(auto picture_item : ui.picture_box->label_list)
    {
        QPixmap picture(picture_item.file_dir);
        article_->img_list_.push_back(picture);
    }

    if(article_->text_list_.size() == 0 && article_->title_.size() == 0 && article_->img_list_.size() == 0)
    {
        QMessageBox::information(NULL, "Info", "Please input content.", QMessageBox::Ok);
        return;
    }
    if(ui.template_box->label_list.size() < 3)
    {
        QMessageBox::information(NULL, "Info", "Please select 3 templates.", QMessageBox::Ok);
        return;
    }

//     for(const auto& ref:ref_list_)
//     {
//         article_->ref_list_.push_back(ref);
//     }

//     int total = 0;
//     for(int i = 0; i < article_->text_list_.size(); ++i)
//     {
//         total += article_->text_list_[i].word_list.size();
//     }
//     for(auto ref:article_->ref_list_)
//     {
//         int para = ref.first[0];
//         int word = ref.first[1] + (ref.first[2] - ref.first[1]) / 2;
//         int img_idx = ref.second;
//         int target = 0;
//         for(int i = 0; i < para; ++i)
//         {
//             if(i < para)
//             {
//                 target +=  article_->text_list_[i].word_list.size();
//             }
//         }
//         target += word;
//         //int target_page = std::min(static_cast<int>(combine_tree_.size() - 1), static_cast<int>(combine_tree_.size() * (static_cast<double>(target) / total)));
//         //std::cout<< target << " " << total << " " << static_cast<double>(target) / total << " " <<  target_page << std::endl;
// //        for(int i = 0; i < img_node_page_list.size(); ++i)
// //        {
// //            if(img_node_page_list[i] == target_page)
// //            {
// //                // set soft constraint
// //                object += (1 - img_corres_x[i][img_idx]) * 100000 ;
// //            }
// //        }
//         article_->ref_ratio_list_.push_back({img_idx, static_cast<double>(target) / total});
//         std::cout<<"ratio: "<< img_idx << " " << static_cast<double>(target) / total <<std::endl;
//     }
    /* read templates */
    for(auto template_item:ui.template_box->label_list)
    {
        template_list_.push_back((new CLayoutTree()));
        template_list_[template_list_.size() - 1]->ReadFromFile(template_item.file_dir.toStdString());
    }

    /* other parameters */

    min_title_font_size_ = ui.min_title_font_size_box->value();
    max_title_font_size_ = ui.max_title_font_size_box->value();
    if(min_title_font_size_ > max_title_font_size_)
    {
        QMessageBox::information(NULL, "Info", "Invalid title font size.", QMessageBox::Ok);
        return;
    }

    min_text_font_size_ = ui.min_text_font_size_box->value();
    max_text_font_size_ = ui.max_text_font_size_box->value();
    if(min_text_font_size_ > max_text_font_size_)
    {
        QMessageBox::information(NULL, "Info", "Invalid text font size.", QMessageBox::Ok);
        return;
    }
    title_font_type_ = ui.title_font_combo->currentText().toStdString();
    text_font_type_ = ui.text_font_combo->currentText().toStdString();
    multi_page_ = ui.multi_page_check->isChecked();
    if(!multi_page_) GenerateLayout();
    else GenerateLayoutMultiPage();

    ui.generate_button->setEnabled(false);
    QTimer::singleShot(1000, this, [=]() {
        ui.generate_button->setEnabled(true);
    });
}


void LayoutShop::AddPicture()
{
    std::string file_name = QFileDialog::getOpenFileName(this, tr("add a picture"), ".", tr("image(*.jpg *.jpeg *.png)")).toStdString();
    if (!file_name.length() > 0)
    {
        std::cout << "Choose a file!" << std::endl;
        return;
    }
    QPixmap pixmap(file_name.data());
    ui.picture_box->AddPicture(pixmap, QString(file_name.data()));

}

void LayoutShop::SelectTemplate()
{
    int img_num = -1;
    if(!ui.multi_page_check->isChecked()) img_num = ui.picture_box->label_list.size();
    int ref = template_dialog_->exec(img_num);
}

void LayoutShop::LoadTemplates()
{
    template_dialog_  = new TemplateDialog(this);
    Qt::WindowFlags flags = template_dialog_->windowFlags();
    template_dialog_->setWindowFlags(flags | Qt::MSWindowsFixedSizeDialogHint);
}

//void LayoutShop::TestGraphScore()
//{
//    CLayoutTree* layout = new CLayoutTree();
//    std::string fileName = "/Users/lijialuo/Documents/153.lay";
//    layout->ReadFromFile(fileName);
//
//    std::queue<CNode*> node_queue;
//    node_queue.push(layout->GetRoot());
//    while(!node_queue.empty())
//    {
//        auto node = node_queue.front();
//        node_queue.pop();
//        if(node->node_present_type == NODE_PRESENT_TYPE::NONLABEL)
//        {
//            std::cout<<node->bottom_align_node.size()<<" "<<node->left_align_node.size()<<std::endl;
//            if (node->bottom_align_node.size()>0 && node->top_align_node.size()>0)  node->node_relation = NODE_RELATION::HORIZONTAL;
//            else if(node->left_align_node.size()>0 && node->right_align_node.size()>0) node->node_relation = NODE_RELATION::VERTICAL;
//        }
//        for (int i = 0; i < node->children.size(); i++)
//        {
//            node_queue.push(node->children[i]);
//        }
//    }
//
//    rapidjson::Document document;
//    document.SetObject();
//    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
//    rapidjson::Value json_root =  CreateJsonDFS(layout->GetRoot(),allocator);
//    document.AddMember("layout"    ,json_root,allocator);
//
//    std::string json_path =  "/Users/lijialuo/Documents/layout.json";
//
//    std::fstream fs;
//    fs.open(json_path, std::ios::in);
//    if (!fs)
//    {
//        std::ofstream fout(json_path);
//        if (fout) fout.close();
//    }
//    else fs.close();
//
//    rapidjson::StringBuffer oBuffer;
//    rapidjson::Writer<rapidjson::StringBuffer> oWriter(oBuffer);
//    document.Accept(oWriter);
//
//    FILE* pFile = fopen(json_path.c_str(), "w");
//    if (pFile == nullptr)
//    {
//        printf("Error: Open File '%s' Failed.\n", json_path.c_str());
//    }
//    fputs(oBuffer.GetString(), pFile);
//    fclose(pFile);
//
//
//    double horizontalDPI = QPaintDevice::physicalDpiX();
//    double verticalDPI = QPaintDevice::physicalDpiY();
//    int paper_w = 630;
//    int paper_h = 891;
//
//    layout_estimator_.SetH(paper_h);
//    layout_estimator_.SetW(paper_w);
//    layout_estimator_.EstimateTest(layout->GetRoot());
//}


void LayoutShop::SaveFiles(CCombineTreeNode*tree,std::string dir)
{
    //save json file
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
    rapidjson::Value json_root =  CreateJsonDFS(tree,allocator);
    document.AddMember("layout"    ,json_root,allocator);

    std::string json_path = dir + "/layout.json";

    std::fstream fs;
    fs.open(json_path, std::ios::in);
    if (!fs)
    {
        std::ofstream fout(json_path);
        if (fout) fout.close();
    }
    else fs.close();

    rapidjson::StringBuffer oBuffer;
    rapidjson::Writer<rapidjson::StringBuffer> oWriter(oBuffer);
    document.Accept(oWriter);

    FILE* pFile = fopen(json_path.c_str(), "w");
    if (pFile == nullptr)
    {
        printf("Error: Open File '%s' Failed.\n", json_path.c_str());
    }
    fputs(oBuffer.GetString(), pFile);
    fclose(pFile);
    //save demo img
    std::queue<CCombineTreeNode*> node_queue;
    node_queue.push(tree);
    std::unordered_map<int, CustomGraphicsItem2*> non_leaf_item_map;
    CustomGraphicsItem2* root_item;
    while (!node_queue.empty())
    {
        auto node = node_queue.front();
        node_queue.pop();
        CustomGraphicsItem2* node_item = new CustomGraphicsItem2();
        node_item->SetNodeType(node->node_present_type);
        node_item->SetTreeNode(node);
        CustomGraphicsItem2* parent_item = nullptr;
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
        if (node->children.size() >0)
        {
            non_leaf_item_map[node->index] = node_item;
        }
        for (int  i = 0; i < node->children.size(); i++)
        {
            node_queue.push(node->children[i]);
        }
    }

    int paper_w = W;//original_layout->GetRoot()->width;
    int paper_h = H;//original_layout->GetRoot()->height;
    CustomGraphicsItem2* paper_item = new CustomGraphicsItem2();
    paper_item->SetNodeType(NODE_PRESENT_TYPE::NONLABEL);
    paper_item->SetTreeNode(nullptr);
    paper_item->setPos(0, 0);
    paper_item->setRect(0, 0, paper_w, paper_h);
    root_item->setParentItem(paper_item);
    QGraphicsScene* scene = new QGraphicsScene();
    scene->addItem(paper_item);
    QGraphicsView render_view;

    render_view.setScene(scene);
    //render1_view.resize(render1_view.sceneRect().toRect().width(),render1_view.sceneRect().toRect().height());
    render_view.scale(real_w_ * 10 / render_view.sceneRect().toRect().width(), real_h_ * 10 / render_view.sceneRect().toRect().height());
    render_view.resize(real_w_ * 10, real_h_ * 10);
    QPixmap pixmap = render_view.grab();
    std::string render_path = dir + "/render.png";
    pixmap.save(render_path.c_str(),"png");

//       //save render img
//       QGraphicsView example_view;
//       scene =  PaintLayout(tree,false);
//       example_view.setScene(scene);
//       //render1_view.resize(render1_view.sceneRect().toRect().width(),render1_view.sceneRect().toRect().height());
//       example_view.resize(example_view.sceneRect().toRect().width(), example_view.sceneRect().toRect().height());
//       pixmap = example_view.grab();
//       std::string example_path = dir + "/example.png";
//       pixmap.save(example_path.c_str(),"png");
}

void LayoutShop::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    ResizePreview();
}

void LayoutShop::ResizePreview()
{
    auto preview_list = ui.preview_list_scroll->widget()->findChildren<CustomGraphicsView*>();
    if(preview_list.size() == 0) return;
    FlowLayout* new_layout = new FlowLayout(PREVIEW_LIST_SCROLL_MARGIN, PREVIEW_LIST_SCROLL_H_SPACE, PREVIEW_LIST_SCROLL_V_SPACE);
    FlowLayout* old_layout = dynamic_cast<FlowLayout*>(ui.preview_list_scroll->widget()->layout());
    for(auto preview:preview_list)
    {
        new_layout->addWidget(preview);
        old_layout->removeWidget(preview);
    }
    delete old_layout;
    ui.preview_list_scroll->widget()->setLayout(new_layout);
    int widget_width,widget_height;
    double new_scale_w, new_scale_ratio, new_scale_h;
    new_scale_w = ui.preview_list_scroll->width() - 2 * PREVIEW_LIST_SCROLL_MARGIN - 10;
    //new_scale_w = ui.preview_list_scroll->widget()->width() - 2 * PREVIEW_LIST_SCROLL_MARGIN;
    new_scale_ratio = new_scale_w / preview_list[0]->scene()->width();
    new_scale_h = preview_list[0]->scene()->height() * new_scale_ratio;

    //view->scale(new_scale_w_ratio, new_scale_h_ratio);
    widget_width = ui.preview_list_scroll->width();
    widget_height = preview_list.size() * new_scale_h + (preview_list.size() - 1) * PREVIEW_LIST_SCROLL_V_SPACE + 2 * PREVIEW_LIST_SCROLL_MARGIN + 10;
    ui.preview_list_scroll->widget()->resize(widget_width,widget_height);
    for(auto & view : preview_list)
    {
        view->resize(new_scale_w, new_scale_h);
        view->ScaleLayout(new_scale_ratio);
        //view->viewport()->resize(new_scale_w, new_scale_h);
    }
    //todo: fix the position

}

void LayoutShop::CreatePreviewWidgets()
{
    FlowLayout* flowLayout = new FlowLayout(PREVIEW_LIST_SCROLL_MARGIN, PREVIEW_LIST_SCROLL_H_SPACE, PREVIEW_LIST_SCROLL_V_SPACE);
    ui.preview_list_scroll->widget()->setLayout(flowLayout);
    for(int i = 0; i < preview_list_.size(); ++i)
    {
        CustomGraphicsView* view = new CustomGraphicsView();
        view->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        view->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        view->setScene(preview_list_[i].preview_scene);
        ui.preview_list_scroll->widget()->layout()->addWidget(view);
        view->setObjectName(QString("preview_") + QString(std::to_string(i).data()));
        view->SetPreviewIndex(i);
        view->setResizeAnchor(QGraphicsView::AnchorViewCenter);
        QObject::connect(view, SIGNAL(ClickPreview(int)),
                         this, SLOT(ReceivePreviewIndex(int)));
    }
    ResizePreview();
}

void LayoutShop::OutputImage()
{
    if(layout_index_ < 0) return;
    int good_layout = 1;
    if(combine_tree_list_[layout_index_][0]->img_score < layout_estimator_.GetThreshold()  || combine_tree_list_[layout_index_][0]->structure_score < layout_estimator_.GetThreshold())
    {
        good_layout = 0;
    }
    QString file_name = QFileDialog::getSaveFileName(this,
                                                    tr("Save PNG"),
                                                     std::to_string(layout_index_).append(
                                                                     "_").append(std::to_string(combine_tree_list_[layout_index_][0]->img_score)).append("_")
                                                             .append(std::to_string(combine_tree_list_[layout_index_][0]->structure_score)).append("_")
                                                             .append(std::to_string(combine_tree_list_[layout_index_][0]->total_score)).append("_")
                                                             .append(std::to_string(good_layout)).append(".png").data(),
                                                    tr("Image Files (*.png)"));
    if (!file_name.isEmpty())
    {
        QImage image(ui.output_view->scene()->width(),ui.output_view->scene()->height(),QImage::Format_RGB32);
        QPainter painter(&image);
        ui.output_view->scene()->render(&painter);   //关键函数
        if(!image.save(file_name))
        {
            QMessageBox::critical(NULL, "Error", "Failed saving the file.", QMessageBox::Ok);
        }
    }


}

void LayoutShop::OutputJSON()
{
    if(layout_index_ < 0) return;
    int good_layout = 1;
    if(combine_tree_list_[layout_index_][0]->img_score < layout_estimator_.GetThreshold()  || combine_tree_list_[layout_index_][0]->structure_score < layout_estimator_.GetThreshold())
    {
        good_layout = 0;
    }
    QString file_name = QFileDialog::getSaveFileName(this,
                                                     tr("Save JSON"),
                                                     std::to_string(layout_index_).append(
                                                             "_").append(std::to_string(combine_tree_list_[layout_index_][0]->img_score)).append("_")
                                                             .append(std::to_string(combine_tree_list_[layout_index_][0]->structure_score)).append("_")
                                                             .append(std::to_string(combine_tree_list_[layout_index_][0]->total_score)).append("_")
                                                             .append(std::to_string(good_layout)).append(".json").data(),
                                                     tr("JSON Files (*.json)"));
    if (!file_name.isEmpty())
    {
        rapidjson::Document document;
        document.SetObject();
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
        rapidjson::Value json_layout_list(rapidjson::Type::kArrayType);
        for(auto layout:combine_tree_list_[layout_index_])
        {
            rapidjson::Value json_root =  CreateJsonDFS(layout,allocator);
            json_layout_list.PushBack(json_root,allocator);
        }
        document.AddMember("layouts", json_layout_list,allocator);
        //生成字符串
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);

        //写到文件
        FILE* myFile = fopen(file_name.toStdString().c_str(), "w");  //windows平台要使用wb
        if (myFile)
        {
            fputs(buffer.GetString(), myFile);
            fclose(myFile);
        }
        else
        {
            QMessageBox::critical(NULL, "Error", "Failed saving the file.", QMessageBox::Ok);
        }
    }
}

void LayoutShop::OutputSVG()
{
    if(layout_index_ < 0) return;
    int good_layout = 1;
    if(combine_tree_list_[layout_index_][0]->img_score < layout_estimator_.GetThreshold()  || combine_tree_list_[layout_index_][0]->structure_score < layout_estimator_.GetThreshold())
    {
        good_layout = 0;
    }
    QString file_name = QFileDialog::getSaveFileName(this,
                                                     tr("Save SVG"),
                                                     std::to_string(layout_index_).append("_")
                                                     .append(std::to_string(combine_tree_list_[layout_index_][0]->img_score)).append("_")
                                                             .append(std::to_string(combine_tree_list_[layout_index_][0]->structure_score)).append("_")
                                                             .append(std::to_string(combine_tree_list_[layout_index_][0]->total_score)).append("_")
                                                             .append(std::to_string(good_layout)).append(".svg").data(),
                                                     tr("SVG Files (*.svg)"));
    if (!file_name.isEmpty())
    {
        QSvgGenerator generator;
        generator.setFileName(file_name);
        generator.setSize(QSize(ui.output_view->scene()->width(), ui.output_view->scene()->height()));
        generator.setViewBox(QRect(0, 0, ui.output_view->scene()->width(), ui.output_view->scene()->height()));
        generator.setTitle("Layout SVG");
//        auto scene = dynamic_cast<CustomGraphicsScene*>(ui.output_view->scene());
//        auto paint_type =  scene->GetPaintType();
//        scene->SetPaintType(CustomGraphicsScene::PAINT_TYPE::NO_CONTENT);
        QPainter painter;
        painter.begin(&generator);
        ui.output_view->scene()->render(&painter);
        //this->window()->render(&painter);
        painter.end();

//        scene->SetPaintType(paint_type);
    }
}

void LayoutShop::OutputPDF()
{
//    if(layout_index_ < 0) return;
//    int good_layout = 1;
//    if(combine_tree_list_[layout_index_][0]->img_score < layout_estimator_.GetThreshold()  || combine_tree_list_[layout_index_][0]->structure_score < layout_estimator_.GetThreshold())
//    {
//        good_layout = 0;
//    }
//    QString file_name = QFileDialog::getSaveFileName(this,
//                                                     tr("Save PDF"),
//                                                     std::to_string(layout_index_).append("_")
//                                                            .append(std::to_string(combine_tree_list_[layout_index_][0]->img_score)).append("_")
//                                                             .append(std::to_string(combine_tree_list_[layout_index_][0]->structure_score)).append("_")
//                                                             .append(std::to_string(combine_tree_list_[layout_index_][0]->total_score)).append("_")
//                                                             .append(std::to_string(good_layout)).append(".pdf").data(),
//                                                     tr("PDF Files (*.pdf)"));
//    if (!file_name.isEmpty())
//    {
////        QPrinter pdf_printer;
////        pdf_printer.setOutputFormat(QPrinter::PdfFormat);
////        pdf_printer.setPageSize(QPageSize::A4);
////        pdf_printer.setOutputFileName(file_name);
////        pdf_printer.setResolution(QPaintDevice::physicalDpiX());
////        QPainter pdf_writer;
////        pdf_writer.setRenderHint(QPainter::Antialiasing);
////        pdf_writer.begin(&pdf_printer);
//
//
//        QFile pdf_file(file_name);
//        pdf_file.open(QIODevice::WriteOnly);
//        QPdfWriter* pdf_writer = new QPdfWriter(&pdf_file);
//        pdf_writer->setPageSize(QPageSize::A4);
//       // double dpi = QPaintDevice::physicalDpiX();
//        //pdf_writer->setResolution(dpi);
//        pdf_writer->setResolution(QPaintDevice::physicalDpiY());
//        //pdf_writer->setPageMargins();
//        QPainter* pdf_painter = new QPainter(pdf_writer);
//        auto scene = ui.output_view->scene();
//
//
//        for(auto item : scene->items())
//        {
//            auto custom_item = dynamic_cast<CustomGraphicsItem*>(item);
//            auto scene_pos =  custom_item->scenePos();
//            if(custom_item->GetNodeType() == NODE_PRESENT_TYPE::TITLE)
//            {
//                QPen pen;
//                pen.setColor(Qt::black);
//                pdf_painter->setPen(pen);
//                QFont font ;
//                font.setFamily(custom_item->GetTitleFont().family());
//                font.setPointSizeF(custom_item->GetTitleFont().pointSizeF() );
//                pdf_painter->setFont(font);
//                //pdf_painter->setFont(custom_item->GetTitleFont());
//                if(custom_item->rect().height()/custom_item->rect().width() >= 2.0)
//                {
//                    pdf_painter->translate(custom_item->rect().width(), 0);
//                    pdf_painter->rotate(90);
//                    pdf_painter->drawText(QRect(scene_pos.y(), scene_pos.x(), custom_item->rect().height(), custom_item->rect().width()), Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextWordWrap, custom_item->GetTitle());
//                    pdf_painter->resetTransform();
//                }
//                    //title_item->setDefaultTextColor(QColor(30, 30, 30));
//                else pdf_painter->drawText(QRect(scene_pos.x(), scene_pos.y(), custom_item->rect().width(), custom_item->rect().height()), Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextWordWrap, custom_item->GetTitle());
//            }
//            else if(custom_item->GetNodeType() == NODE_PRESENT_TYPE::TEXT)
//            {
//                QFont font ;
//                font.setFamily(custom_item->GetTextFont().family());
//                font.setPointSizeF(custom_item->GetTextFont().pointSizeF());
//                pdf_painter->setPen(Qt::black);
//                pdf_painter->setFont(font);
//                for (int i = 0; i < custom_item->GetStartPointList().size(); ++i)
//                {
//                    auto start_point = custom_item->GetStartPointList()[i];
//                    pdf_painter->drawText(scene_pos.x() + start_point.first, scene_pos.y() + start_point.second, custom_item->GetTextList()[i]);
//                }
//            }
//            else if(custom_item->GetNodeType() == NODE_PRESENT_TYPE::PICTURE)
//            {
//                double img_ratio =  static_cast<double>(custom_item->GetPixmap().width()) / custom_item->GetPixmap().height();
//                double rect_ratio = custom_item->rect().width() / custom_item->rect().height();
//
//                QRectF source_rect;
//                QRectF draw_rect = custom_item->rect();
//
//                if (img_ratio >= rect_ratio)
//                {
//                    double draw_width, draw_height;
//                    draw_height = custom_item->GetPixmap().height();
//                    draw_width = draw_height * rect_ratio;
//                    source_rect.setY(0);
//                    source_rect.setX((custom_item->GetPixmap().width() - draw_width) / 2);
//                    source_rect.setHeight(draw_height);
//                    source_rect.setWidth(draw_width);
//                }
//                else
//                {
//                    double draw_width, draw_height;
//                    draw_width = custom_item->GetPixmap().width();
//                    draw_height = draw_width / rect_ratio;
//                    source_rect.setX(0);
//                    source_rect.setY((custom_item->GetPixmap().height() - draw_height) / 2);
//                    source_rect.setHeight(draw_height);
//                    source_rect.setWidth(draw_width);
//                }
//
//                //qreal pixel_ratio = pdf_writer->device()->devicePixelRatioF();
//                //pixmap_ = pixmap_.scaled(QSize(pixmap_.width() * pixel_ratio, pixmap_.height() * pixel_ratio)
//                //, Qt::KeepAspectRatio, Qt::SmoothTransformation);
//                //pdf_writer->drawPixmap(draw_rect, pixmap_, source_rect);
//                QImage image = custom_item->GetPixmap().toImage();
//                QPixmap show_pixmap;
//                show_pixmap = QPixmap::fromImage(image.copy(source_rect.x(),source_rect.y(), source_rect.width() ,source_rect.height()));
//
//                //pixmap_ = pixmap_.copy(source_rect.x(),source_rect.y(), source_rect.width() ,source_rect.height());
//                show_pixmap  = show_pixmap.scaled(draw_rect.width() ,draw_rect.height(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
//                pdf_painter->drawPixmap(scene_pos.x() + draw_rect.x(), scene_pos.y() + draw_rect.y(), draw_rect.width() , draw_rect.height(), show_pixmap);
//            }
//        }
//        delete pdf_painter;
//        delete pdf_writer;
//        pdf_file.close();
//        //pdf_writer.end();
//    }
    return;

}

void LayoutShop::OutputAll()
{
    if(layout_index_ < 0) return;
    int good_layout = 1;
    if(combine_tree_list_[layout_index_][0]->img_score < layout_estimator_.GetThreshold()  || combine_tree_list_[layout_index_][0]->structure_score < layout_estimator_.GetThreshold())
    {
        good_layout = 0;
    }
    QString file_name = QFileDialog::getSaveFileName(this,
                                                     tr("Save All"),
                                                     std::to_string(layout_index_).append(
                                                                     "_").append(std::to_string(combine_tree_list_[layout_index_][0]->img_score)).append("_")
                                                             .append(std::to_string(combine_tree_list_[layout_index_][0]->structure_score)).append("_")
                                                             .append(std::to_string(combine_tree_list_[layout_index_][0]->total_score)).append("_")
                                                             .append(std::to_string(good_layout)).data(),
                                                     tr("All Files (*.*)"));
    if (file_name.isEmpty()) return;
    mkdir(file_name.toStdString().data(), 0777);
    if (!file_name.isEmpty())
    {
        rapidjson::Document document;
        document.SetObject();
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
        rapidjson::Value json_layout_list(rapidjson::Type::kArrayType);
        for(auto layout:combine_tree_list_[layout_index_])
        {
            rapidjson::Value json_root =  CreateJsonDFS(layout,allocator);
            json_layout_list.PushBack(json_root,allocator);
        }
        document.AddMember("layouts", json_layout_list,allocator);
        //生成字符串
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);

        //写到文件
        FILE* myFile = fopen((file_name + QString("/json.json")).toStdString().c_str(), "w");
        if (myFile)
        {
            fputs(buffer.GetString(), myFile);
            fclose(myFile);
        }
        else
        {
            QMessageBox::critical(NULL, "Error", "Failed saving the file.", QMessageBox::Ok);
        }
        QImage image(ui.output_view->scene()->width(),ui.output_view->scene()->height(),QImage::Format_RGB32);
        QPainter painter(&image);
        ui.output_view->scene()->render(&painter);   //关键函数
        if(!image.save((file_name + QString("/img.png"))))
        {
            QMessageBox::critical(NULL, "Error", "Failed saving the file.", QMessageBox::Ok);
        }


        QSvgGenerator generator;
        generator.setFileName((file_name + QString("/svg.svg")));
        generator.setSize(QSize(ui.output_view->scene()->width(), ui.output_view->scene()->height()));
        generator.setViewBox(QRect(0, 0, ui.output_view->scene()->width(), ui.output_view->scene()->height()));
        generator.setTitle("Layout SVG");
//        auto scene = dynamic_cast<CustomGraphicsScene*>(ui.output_view->scene());
//        auto paint_type =  scene->GetPaintType();
//        scene->SetPaintType(CustomGraphicsScene::PAINT_TYPE::NO_CONTENT);
        QPainter painter_svg;
        painter_svg.begin(&generator);
        ui.output_view->scene()->render(&painter_svg);
        //this->window()->render(&painter);
        painter_svg.end();






//
//        double horizontalDPI = QPaintDevice::physicalDpiX();
//        double verticalDPI = QPaintDevice::physicalDpiY();
//        int paper_w = horizontalDPI / 2.54 * real_w_;
//        int paper_h = verticalDPI / 2.54 * real_h_;
//        for( int i = 0; i < combine_tree_list_[layout_index_].size(); ++i)
//        {
//            generator.setFileName((file_name + QString("/") + QString(std::to_string(i).c_str()) + QString(".svg")));
//            generator.setSize(QSize(paper_w, paper_h));
//            generator.setViewBox(QRect(0, i * paper_h, paper_w, paper_h));
//            generator.setTitle("Layout SVG");
//            QPainter painter_svg;
//            painter_svg.begin(&generator);
//            ui.output_view->scene()->render(&painter_svg);
//            //this->window()->render(&painter);
//            painter_svg.end();
//        }


//        auto scene = dynamic_cast<CustomGraphicsScene*>(ui.output_view->scene());
//        auto paint_type =  scene->GetPaintType();
//        scene->SetPaintType(CustomGraphicsScene::PAINT_TYPE::NO_CONTENT);




    }
}

bool LayoutShop::CheckCombineTreeSatisfied(CCombineTreeNode *combine_tree)
{
    int tree_title_num = 0, tree_text_num = 0, tree_img_num = 0;
    int content_title_num = 0, content_text_num = 0, content_img_num = 0;

    std::queue<CCombineTreeNode*> temp_queue;
    temp_queue.push(combine_tree);
    while(!temp_queue.empty())
    {
        auto node = temp_queue.front();
        temp_queue.pop();

        if(node->node_present_type == NODE_PRESENT_TYPE::TITLE) tree_title_num ++;
        else if(node->node_present_type == NODE_PRESENT_TYPE::TEXT) tree_text_num ++;
        else if(node->node_present_type == NODE_PRESENT_TYPE::PICTURE) tree_img_num ++;

        for(auto child : node->children)
        {
            temp_queue.push(child);
        }
    }
    if(ui.title_input->document()->toPlainText().toStdString().size() > 0)
    {
        content_title_num = 1;
    }
    if(ui.text_input->document()->toPlainText().toStdString().size() > 0)
    {
        if(paragraph_) content_text_num = article_->text_list_.size();
        else content_text_num = 1;
    }
    content_img_num = ui.picture_box->label_list.size();

    if(content_title_num > tree_title_num)
    {
        QMessageBox::information(NULL, "Info", "Need more title nodes in templates.", QMessageBox::Ok);
        return false;
    }
    else if(content_text_num > tree_text_num)
    {
        QMessageBox::information(NULL, "Info", "Need more text nodes in templates.", QMessageBox::Ok);
        return false;
    }
    else if(content_img_num > tree_img_num)
    {
        QMessageBox::information(NULL, "Info", "Need more image nodes in templates.", QMessageBox::Ok);
        return false;
    }
    return true;
}

void LayoutShop::PresetImgCorres(std::vector<CCombineTreeNode *> layout_tree)
{
    int img_idx = 0;
    for (int page_idx = 0; page_idx < layout_tree.size(); ++page_idx)
    {
        std::stack<CCombineTreeNode*> stack;
        stack.push(layout_tree[page_idx]);
        while (!stack.empty())
        {
            auto node = stack.top();
            stack.pop();
            if (node->node_present_type == NODE_PRESENT_TYPE::PICTURE)
            {
                node->corres_img_idx = img_idx;
                node->picture_lock = true;
                img_idx++;
            }
            for (int i = node->children.size() - 1; i >= 0; i--)
            {
                stack.push(node->children[i]);
            }
        }
    }
}

void LayoutShop::ShowRefineWidget()
{
    if(refine_widget_ == nullptr)
    {
        CreateRefineWidget();
    }
    auto output_view_pos = ui.output_view->pos();
    QObject* widget = ui.output_view;
    while(widget->parent() != nullptr)
    {
        output_view_pos = dynamic_cast<QWidget*>(widget)->mapToParent(output_view_pos);
        widget = widget->parent();
    }
    output_view_pos = mapToGlobal(output_view_pos);

    refine_widget_->setGeometry( output_view_pos.x() + ui.output_view->width() - refine_widget_->width() - 10,
                                 output_view_pos.y() + ui.output_view->height() - refine_widget_->height() - 10,
                                refine_widget_->width(), refine_widget_->height());
    refine_widget_->show();
}

void LayoutShop::HideRefineWidget()
{
    if(refine_widget_ != nullptr)
    {
        refine_widget_->hide();
    }

}

void LayoutShop::SetRefineWidgetValue()
{
    if(layout_index_ < 0)
    {
        refine_widget_->setEnabled(false);
    }
    else
    {
        refine_active_ = false;
        auto layout_scene = dynamic_cast<CustomGraphicsScene*>(ui.output_view->scene());

        int index = refine_widget_->ui->title_font_combo->findText(layout_scene->getTitleFont().family());
        refine_widget_->ui->title_font_combo->setCurrentIndex(index);
        index = refine_widget_->ui->text_font_combo->findText(layout_scene->getTextFont().family());
        refine_widget_->ui->text_font_combo->setCurrentIndex(index);
        int size = layout_scene->getTextFont().pointSize();
        refine_widget_->ui->text_size_spinbox->setValue(size);
        size = layout_scene->getTitleFont().pointSize();
        refine_widget_->ui->title_size_spinbox->setValue(size);
        auto rotate_value = layout_scene->getTitleRotateValue();
        int rotate_int_value = 0;
        if(rotate_value == CustomGraphicsScene::ROTATE_0)   rotate_int_value = 0;
        else if(rotate_value == CustomGraphicsScene::ROTATE_90) rotate_int_value = 90;
        else if(rotate_value == CustomGraphicsScene::ROTATE_180) rotate_int_value = 180;
        else if(rotate_value == CustomGraphicsScene::ROTATE_270) rotate_int_value = 270;
        refine_widget_->ui->rotate_value_line->setText(std::to_string(rotate_int_value).data());
        auto h_align = layout_scene->getTitleAlignH();
        auto v_align = layout_scene->getTitleAlignV();
        if(h_align == CustomGraphicsScene::LEFT_ALIGN)   refine_widget_->ui->align_h_left->setChecked(true);
        else refine_widget_->ui->align_h_left->setChecked(false);
        if(h_align == CustomGraphicsScene::CENTER_ALIGN)   refine_widget_->ui->align_h_center->setChecked(true);
        else refine_widget_->ui->align_h_center->setChecked(false);
        if(h_align == CustomGraphicsScene::RIGHT_ALIGN)   refine_widget_->ui->align_h_right->setChecked(true);
        else refine_widget_->ui->align_h_right->setChecked(false);
        if(v_align == CustomGraphicsScene::TOP_ALIGN)   refine_widget_->ui->align_v_top->setChecked(true);
        else refine_widget_->ui->align_v_top->setChecked(false);
        if(v_align == CustomGraphicsScene::MIDDLE_ALIGN)   refine_widget_->ui->align_v_middle->setChecked(true);
        else refine_widget_->ui->align_v_middle->setChecked(false);
        if(v_align == CustomGraphicsScene::BOTTOM_ALIGN)   refine_widget_->ui->align_v_bottom->setChecked(true);
        else refine_widget_->ui->align_v_bottom->setChecked(false);

        refine_widget_->setEnabled(true);
        refine_active_ = true;
    }
}

bool CompareTextItem(CustomGraphicsItem* item_1, CustomGraphicsItem* item_2)
{
    return item_1->GetTreeNode()->text_idx < item_2->GetTreeNode()->text_idx;
}

void LayoutShop::ChangeRefineWidgetValue()
{
    if(layout_index_ >= 0 && refine_active_) {
        auto preview = preview_list_[layout_index_];
        auto actual_scene = preview.actual_scene;
        auto preview_scene = preview.preview_scene;

        QFont title_font;
        title_font.setPointSize(refine_widget_->ui->title_size_spinbox->value());
        title_font.setFamily(refine_widget_->ui->title_font_combo->currentText());
        title_font.setBold(true);
        actual_scene->setTitleFont(title_font);
        preview_scene->setTitleFont(title_font);

        QFont text_font;
        text_font.setPointSize(refine_widget_->ui->text_size_spinbox->value());
        text_font.setFamily(refine_widget_->ui->text_font_combo->currentText());
        actual_scene->setTextFont(text_font);
        preview_scene->setTextFont(text_font);

        CustomGraphicsScene::TITLE_ROTATE rotate_value;
        int rotate_int_value = std::stoi(refine_widget_->ui->rotate_value_line->text().toStdString());
        if (rotate_int_value == 0)  rotate_value = CustomGraphicsScene::ROTATE_0;
        else  if (rotate_int_value == 90)  rotate_value = CustomGraphicsScene::ROTATE_90;
        else  if (rotate_int_value == 180)  rotate_value = CustomGraphicsScene::ROTATE_180;
        else  if (rotate_int_value == 270)  rotate_value = CustomGraphicsScene::ROTATE_270;
        actual_scene->setTitleRotateValue(rotate_value);
        preview_scene->setTitleRotateValue(rotate_value);

        CustomGraphicsScene::TITLE_ALIGN_H align_h;
        if(refine_widget_->ui->align_h_left->isChecked()) align_h = CustomGraphicsScene::LEFT_ALIGN;
        else  if(refine_widget_->ui->align_h_center->isChecked()) align_h = CustomGraphicsScene::CENTER_ALIGN;
        else  if(refine_widget_->ui->align_h_right->isChecked()) align_h = CustomGraphicsScene::RIGHT_ALIGN;
        actual_scene->setTitleAlignH(align_h);
        preview_scene->setTitleAlignH(align_h);

        CustomGraphicsScene::TITLE_ALIGN_V align_v;
        if(refine_widget_->ui->align_v_top->isChecked()) align_v = CustomGraphicsScene::TOP_ALIGN;
        else  if(refine_widget_->ui->align_v_middle->isChecked()) align_v = CustomGraphicsScene::MIDDLE_ALIGN;
        else  if(refine_widget_->ui->align_v_bottom->isChecked()) align_v = CustomGraphicsScene::BOTTOM_ALIGN;
        actual_scene->setTitleAlignV(align_v);
        preview_scene->setTitleAlignV(align_v);

        LayoutPainter layout_painter;
        layout_painter.SetArticle(article_);

        std::vector<CustomGraphicsItem*> text_item_list_actual;
        std::vector<CustomGraphicsItem*> text_item_list_preview;

        for(auto item:actual_scene->items())
        {
            auto item_custom = dynamic_cast<CustomGraphicsItem*>(item);
            if(item_custom != nullptr && item_custom->GetTreeNode() != nullptr && item_custom->GetTreeNode()->text_idx >= 0)
            {
                text_item_list_actual.push_back(item_custom);
            }
        }

        for(auto item:preview_scene->items())
        {
            auto item_custom = dynamic_cast<CustomGraphicsItem*>(item);
            if(item_custom != nullptr && item_custom->GetTreeNode() != nullptr && item_custom->GetTreeNode()->text_idx >= 0)
            {
                text_item_list_preview.push_back(item_custom);
            }
        }

        std::sort(text_item_list_actual.begin(), text_item_list_actual.end(), CompareTextItem);
        std::sort(text_item_list_preview.begin(), text_item_list_preview.end(), CompareTextItem);


        layout_painter.DrawText(actual_scene, text_item_list_actual, text_font);
        layout_painter.DrawText(preview_scene, text_item_list_preview, text_font);

        actual_scene->update();
        preview_scene->update();
    }
}

void LayoutShop::CreateRefineWidget()
{
    refine_widget_ = new RefineWidget();
    refine_widget_->setWindowFlags(refine_widget_->windowFlags() | Qt::WindowStaysOnTopHint);
    refine_widget_->setFixedSize(refine_widget_->width(), refine_widget_->height());
    refine_widget_->ui->align_h_left->setCheckable(true);
    refine_widget_->ui->align_h_center->setCheckable(true);
    refine_widget_->ui->align_h_right->setCheckable(true);
    refine_widget_->ui->align_v_top->setCheckable(true);
    refine_widget_->ui->align_v_middle->setCheckable(true);
    refine_widget_->ui->align_v_bottom->setCheckable(true);

    QObject::connect(refine_widget_->ui->title_font_combo, SIGNAL(currentIndexChanged(int)),
                     this, SLOT(ChangeRefineWidgetValue()));
    QObject::connect(refine_widget_->ui->text_font_combo, SIGNAL(currentIndexChanged(int)),
                     this, SLOT(ChangeRefineWidgetValue()));
    QObject::connect(refine_widget_->ui->title_size_spinbox, SIGNAL(valueChanged(int)),
                     this, SLOT(ChangeRefineWidgetValue()));
    QObject::connect(refine_widget_->ui->text_size_spinbox, SIGNAL(valueChanged(int)),
                     this, SLOT(ChangeRefineWidgetValue()));
    QObject::connect(refine_widget_->ui->rotate_button, SIGNAL(clicked()),
                     this, SLOT(RotateButtonPush()));

    QObject::connect(refine_widget_->ui->align_h_left, SIGNAL(clicked()),
                     this, SLOT(Align_h_left_trigger()));
    QObject::connect(refine_widget_->ui->align_h_center, SIGNAL(clicked()),
                     this, SLOT(Align_h_center_trigger()));
    QObject::connect(refine_widget_->ui->align_h_right, SIGNAL(clicked()),
                     this, SLOT(Align_h_right_trigger()));

    QObject::connect(refine_widget_->ui->align_v_top, SIGNAL(clicked()),
                     this, SLOT(Align_v_top_trigger()));
    QObject::connect(refine_widget_->ui->align_v_middle, SIGNAL(clicked()),
                     this, SLOT(Align_v_middle_trigger()));
    QObject::connect(refine_widget_->ui->align_v_bottom, SIGNAL(clicked()),
                     this, SLOT(Align_v_bottom_trigger()));
    QObject::connect(refine_widget_, &RefineWidget::SetActionChecked, this, [=]()
    {
        ui.actionRefine->setChecked(true);
    });
    QObject::connect(refine_widget_, &RefineWidget::SetActionUnchecked, this, [=]()
    {
        ui.actionRefine->setChecked(false);
    });
    QObject::connect(refine_widget_->ui->regenerate_button, SIGNAL(clicked()), this, SLOT(RefineRegenerate()));


}

void LayoutShop::RotateButtonPush()
{
    int rotate_int_value_old = std::stoi(refine_widget_->ui->rotate_value_line->text().toStdString());
    int rotate_int_value_new;
    if(rotate_int_value_old == 0)    refine_widget_->ui->rotate_value_line->setText("90");
    else if(rotate_int_value_old == 90)     refine_widget_->ui->rotate_value_line->setText("180");
    else if(rotate_int_value_old == 180)    refine_widget_->ui->rotate_value_line->setText("270");
    else if(rotate_int_value_old == 270)    refine_widget_->ui->rotate_value_line->setText("0");

    ChangeRefineWidgetValue();
}

void LayoutShop::Align_h_left_trigger()
{
    refine_widget_->ui->align_h_left->setChecked(true);
    refine_widget_->ui->align_h_center->setChecked(false);
    refine_widget_->ui->align_h_right->setChecked(false);

    ChangeRefineWidgetValue();
}

void LayoutShop::Align_h_center_trigger()
{
    refine_widget_->ui->align_h_left->setChecked(false);
    refine_widget_->ui->align_h_center->setChecked(true);
    refine_widget_->ui->align_h_right->setChecked(false);

    ChangeRefineWidgetValue();
}

void LayoutShop::Align_h_right_trigger()
{
    refine_widget_->ui->align_h_left->setChecked(false);
    refine_widget_->ui->align_h_center->setChecked(false);
    refine_widget_->ui->align_h_right->setChecked(true);

    ChangeRefineWidgetValue();
}

void LayoutShop::Align_v_top_trigger()
{
    refine_widget_->ui->align_v_top->setChecked(true);
    refine_widget_->ui->align_v_middle->setChecked(false);
    refine_widget_->ui->align_v_bottom->setChecked(false);

    ChangeRefineWidgetValue();
}

void LayoutShop::Align_v_middle_trigger()
{
    refine_widget_->ui->align_v_top->setChecked(false);
    refine_widget_->ui->align_v_middle->setChecked(true);
    refine_widget_->ui->align_v_bottom->setChecked(false);

    ChangeRefineWidgetValue();
}

void LayoutShop::Align_v_bottom_trigger()
{
    refine_widget_->ui->align_v_top->setChecked(false);
    refine_widget_->ui->align_v_middle->setChecked(false);
    refine_widget_->ui->align_v_bottom->setChecked(true);

    ChangeRefineWidgetValue();
}

void LayoutShop::RefineTrigger()
{
    if(ui.actionRefine->isChecked()) ShowRefineWidget();
    else HideRefineWidget();
}

void LayoutShop::RefineRegenerate()
{
    double horizontalDPI = QPaintDevice::physicalDpiX();
    double verticalDPI = QPaintDevice::physicalDpiY();
    int paper_w = horizontalDPI / 2.54 * real_w_;
    int paper_h = verticalDPI / 2.54 * real_h_;
    //std::vector<std::vector<CCombineTreeNode*>> valid_layout_list;
    QFont title_font;
    title_font.setPointSize(refine_widget_->ui->title_size_spinbox->value());
    title_font.setFamily(refine_widget_->ui->title_font_combo->currentText());
    title_font.setBold(true);

    QFont text_font;
    text_font.setPointSize(refine_widget_->ui->text_size_spinbox->value());
    text_font.setFamily(refine_widget_->ui->text_font_combo->currentText());

    LayoutGeneratorMultiPage layout_generator;

    layout_generator.SetArticle(article_);
    layout_generator.SetTitleFontSizeRange(title_font.pointSize(), title_font.pointSize());
    layout_generator.SetTextFontSizeRange(text_font.pointSize(), text_font.pointSize());
    layout_generator.SetTitleFontType(title_font.family().toStdString());
    layout_generator.SetTextFontType(text_font.family().toStdString());
    layout_generator.SetW(paper_w);
    layout_generator.SetH(paper_h);

    GRBEnv env = GRBEnv(true);
    env.start();
    GRBModel model = GRBModel(env);
    if (multi_page_) {
        model.set("NonConvex", "2");
        model.set("TimeLimit", "2.0");
        model.set("OutputFlag", "0");
    } else {
        model.set("NonConvex", "2");
        model.set("TimeLimit", "1.0");
        model.set("OutputFlag", "0");
    }

    std::vector<CCombineTreeNode *> new_combine_tree;
    //auto old_combine_tree = combine_tree_list_[layout_index_];
    auto old_scene = preview_list_[layout_index_].actual_scene;
    auto old_preview = preview_list_[layout_index_].preview_scene;
    for (auto single_tree: combine_tree_list_[layout_index_]) {
        new_combine_tree.push_back(single_tree->DeepCopy());
    }

    layout_generator.SetCombineTree(new_combine_tree);

    bool success = layout_generator.GenerateLayout(model);
    if (success)
    {
        //if (fix_geometry_node != nullptr) fix_geometry_node->fix_geometry = false;
        combine_tree_stack_[layout_index_].push(combine_tree_list_[layout_index_]);
        combine_tree_list_[layout_index_] = new_combine_tree;

        LayoutPainter layout_painter;
        layout_painter.SetArticle(article_);
        layout_painter.SetTitleFontSizeRange(title_font.pointSize(), title_font.pointSize());
        layout_painter.SetTextFontSizeRange(text_font.pointSize(), text_font.pointSize());
        layout_painter.SetTitleFontType(title_font.family().toStdString());
        layout_painter.SetTextFontType(text_font.family().toStdString());
        layout_painter.SetW(paper_w);
        layout_painter.SetH(paper_h);
        layout_painter.CalTextAndTitleArea();
        layout_painter.SetLayoutTree(combine_tree_list_[layout_index_]);
        Preview scene = layout_painter.Draw();
        CustomGraphicsScene::PAINT_TYPE paint_type = content_view_ ?  CustomGraphicsScene::PAINT_TYPE::CONTENT : CustomGraphicsScene::PAINT_TYPE::NO_CONTENT;
        scene.actual_scene->SetPaintType(paint_type);
        scene.preview_scene->SetPaintType(paint_type);
        preview_list_[layout_index_] = scene;
        auto preview_list = ui.preview_list_scroll->widget()->findChildren<CustomGraphicsView*>();
        for(auto preview:preview_list)
        {
            if(preview->scene() == old_preview)   preview->setScene(scene.preview_scene);
        }
        UpdateOutputAndPreview();
        old_scene->disconnect();
        delete_scene_list_.push_back(old_preview);
        delete_scene_list_.push_back(old_scene);
        layout_estimator_.EstimateSingleLayout(new_combine_tree);
    }
    else
    {
        for(auto single_tree : new_combine_tree)
        {
            CCombineTreeNode::DeepDestroy(single_tree);
        }
        QMessageBox::information(NULL, "Info", "No solutions for the current edit.", QMessageBox::Ok);
    }
}

void LayoutShop::SaveGRIDSLayout()
{
    std::string file_name = QFileDialog::getOpenFileName(this, tr("Choose a JSON file"), ".", tr("input(*.json)")).toStdString();
    if (!file_name.length() > 0)
    {
        std::cout << "Choose a file!" << std::endl;
        return;
    }

    FILE* fp = fopen(file_name.c_str(), "rb");
    if (!fp)
    {
        std::cout << "open failed" << std::endl;
        return;
    }
    char* buf = new char[1024 * 16];
    int n = fread(buf, 1, 1024 * 16, fp);
    fclose(fp);

    //ClearInput();

    std::string file_content;
    if (n >= 0)
    {
        file_content.append(buf, 0, n);
    }
    delete[]buf;
    rapidjson::Document document;
    document.Parse(file_content.c_str());

    const rapidjson::Value& element_list = document["layouts"][0]["elements"];
    auto root = new CCombineTreeNode();
    root->x = 0;
    root->y = 0;
    root->width = document["layouts"][0]["canvasWidth"].GetDouble();
    root->height = document["layouts"][0]["canvasHeight"].GetDouble();
    root->node_present_type = NODE_PRESENT_TYPE::NONLABEL;
    root->node_relation = NODE_RELATION::HORIZONTAL;
    int img_idx = 0;
    for (int i = 0; i < element_list.Size(); i++)
    {
        auto node = new CCombineTreeNode();
        node->x = element_list[i]["x"].GetDouble();
        node->y = element_list[i]["y"].GetDouble();
        node->width = element_list[i]["width"].GetDouble();
        node->height = element_list[i]["height"].GetDouble();
        node->parent = root;
        if(std::string (element_list[i]["type"].GetString()) == "title")
            node->node_present_type = NODE_PRESENT_TYPE::TITLE;
        else if(std::string (element_list[i]["type"].GetString()) == "text")
            node->node_present_type = NODE_PRESENT_TYPE::TEXT;
        else if(std::string (element_list[i]["type"].GetString()) == "image")
            node->node_present_type = NODE_PRESENT_TYPE::PICTURE;
        if(std::string (element_list[i]["type"].GetString()) == "image")
            node->corres_img_idx = img_idx ++;
        root->children.push_back(node);
    }
//    for(auto child:root->children)
//    {
//        std::cout<<child->node_present_type<<" "<<child->x<<" "<<child->y<<" "<<child->width<<" "<<child->height<<std::endl;
//    }
    for(int i = 0; i < root->children.size() - 1; ++ i)
    {
        for(int j = 0; j < root->children.size() - i - 1; ++j)
        {
//            if(root->children[j]->node_present_type == NODE_PRESENT_TYPE::TEXT
//                && root->children[j+1]->node_present_type != NODE_PRESENT_TYPE::TEXT)
//            {
//                auto temp = root->children[j+1];
//                root->children[j+1] = root->children[j];
//                root->children[j] = temp;
//            }
//            else if(root->children[j]->node_present_type == NODE_PRESENT_TYPE::TEXT
//               && root->children[j+1]->node_present_type == NODE_PRESENT_TYPE::TEXT)
//            {
                if(root->children[j]->y > root->children[j + 1]->y)
                {
                    auto temp = root->children[j+1];
                    root->children[j+1] = root->children[j];
                    root->children[j] = temp;
                }
                else if(root->children[j]->y == root->children[j + 1]->y
                        && root->children[j]->x > root->children[j + 1]->x)
                {
                    auto temp = root->children[j+1];
                    root->children[j+1] = root->children[j];
                    root->children[j] = temp;
                }
//            }
        }
    }

    int text_idx = 0;
    for(auto node : root->children)
    {
        if(node->node_present_type == NODE_PRESENT_TYPE::TEXT)
            node->text_idx = text_idx ++;
    }
    if(article_ != nullptr) delete article_;
    article_ = new Article();
    std::string full_title = ui.title_input->document()->toPlainText().toStdString();
    std::string full_text = ui.text_input->document()->toPlainText().toStdString();
    std::vector<std::string> text_list;
    std::stringstream ss(full_text);
    std::string para;
    while (std::getline(ss, para, '\n'))
    {
        text_list.push_back(para);
    }
    std::istringstream title_stream(full_title);
    //vector<string> vec;
    std::string temp_str;

    std::vector<std::string> title_word_list;
    while(title_stream >> temp_str)
    {
        title_word_list.push_back(temp_str);
    }
    if(title_word_list.size() > 0)
    {
        article_->title_.append(title_word_list[0]);
        for(int i = 1; i < title_word_list.size(); ++i)
        {
            article_->title_.append(" ");
            article_->title_.append(title_word_list[i]);
        }
    }

    for (int i = 0; i < text_list.size(); i++)
    {
        Text text;
        std::istringstream text_stream(text_list[i]);
        //vector<string> vec;
        while(text_stream >> temp_str)
        {
            text.word_list.push_back(temp_str);
        }
        if(text.word_list.size() > 0)
        {
            text.word_list[0] = std::string("    ").append(text.word_list[0]);
            for(int i = 0; i < text.word_list.size() - 1; ++i)
            {
                text.word_list[i].append(" ");
            }

            article_->text_list_.push_back(text);
        }
    }

    for(auto picture_item : ui.picture_box->label_list)
    {
        QPixmap picture(picture_item.file_dir);
        article_->img_list_.push_back(picture);
    }


    /* other parameters */

    min_title_font_size_ = ui.min_title_font_size_box->value();
    max_title_font_size_ = ui.max_title_font_size_box->value();

    min_text_font_size_ = ui.min_text_font_size_box->value();
    max_text_font_size_ = ui.max_text_font_size_box->value();

    title_font_type_ = ui.title_font_combo->currentText().toStdString();
    text_font_type_ = ui.text_font_combo->currentText().toStdString();
    multi_page_ = ui.multi_page_check->isChecked();


    LayoutPainter layout_painter;

    layout_painter.SetArticle(article_);
    layout_painter.SetTitleFontSizeRange(min_title_font_size_, max_title_font_size_);
    layout_painter.SetTextFontSizeRange(min_text_font_size_,max_text_font_size_);
    layout_painter.SetTitleFontType(title_font_type_);
    layout_painter.SetTextFontType(text_font_type_);
    layout_painter.SetW(root->width);
    layout_painter.SetH(root->height);
    layout_painter.CalTextAndTitleArea();

    layout_painter.SetLayoutTree({root});
    Preview scene = layout_painter.Draw();
    CustomGraphicsScene::PAINT_TYPE paint_type = content_view_ ?  CustomGraphicsScene::PAINT_TYPE::CONTENT : CustomGraphicsScene::PAINT_TYPE::NO_CONTENT;
    scene.actual_scene->SetPaintType(paint_type);
    scene.preview_scene->SetPaintType(paint_type);
    preview_list_.push_back(scene);
    this->ui.output_view->setScene(scene.actual_scene);

    QString file_name_save = QFileDialog::getSaveFileName(this,
                                             tr("Save All"),
                                             "grids_layout",
                                                     tr("All Files (*.*)"));
    mkdir(file_name_save.toStdString().data(), 0777);
    if (!file_name_save.isEmpty())
    {

        QImage image(ui.output_view->scene()->width(),ui.output_view->scene()->height(),QImage::Format_RGB32);
        QPainter painter(&image);
        ui.output_view->scene()->render(&painter);
        if(!image.save((file_name_save + QString("/img.png"))))
        {
            QMessageBox::critical(NULL, "Error", "Failed saving the file.", QMessageBox::Ok);
        }
        QSvgGenerator generator;
        generator.setFileName((file_name_save + QString("/svg.svg")));
        generator.setSize(QSize(ui.output_view->scene()->width(), ui.output_view->scene()->height()));
        generator.setViewBox(QRect(0, 0, ui.output_view->scene()->width(), ui.output_view->scene()->height()));
        generator.setTitle("Layout SVG");
//        auto scene = dynamic_cast<CustomGraphicsScene*>(ui.output_view->scene());
//        auto paint_type =  scene->GetPaintType();
//        scene->SetPaintType(CustomGraphicsScene::PAINT_TYPE::NO_CONTENT);
        QPainter painter_svg;
        painter_svg.begin(&generator);
        ui.output_view->scene()->render(&painter_svg);
        //this->window()->render(&painter);
        painter_svg.end();
    }

}

void LayoutShop::OutputImgScoreRender()
{
    const QString selected_dir = QFileDialog::getExistingDirectory(this, tr("Save score images"), ".");
    if (selected_dir.isEmpty()) return;
    LayoutEstimator layout_estimator;
    double horizontalDPI = QPaintDevice::physicalDpiX();
    double verticalDPI = QPaintDevice::physicalDpiY();
    int paper_w = horizontalDPI / 2.54 * real_w_;
    int paper_h = verticalDPI / 2.54 * real_h_;
    layout_estimator.SetW(paper_w);
    layout_estimator.SetH(paper_h);
    int index = 0;
    for(auto layout:combine_tree_list_)
    {
        auto scene =  layout_estimator.RenderScoreLayout(layout[0]);
        QGraphicsView score_render_view;
        score_render_view.setBackgroundBrush(QBrush(QColor(0,0,0)));
        score_render_view.setScene(scene);
        score_render_view.scale(296.0 * 210.0 / 297.0 /  score_render_view.sceneRect().toRect().width(), 296.0 /  score_render_view.sceneRect().toRect().height());
        score_render_view.resize(216, 296);
//    score_render_view.scale(70.0 /  score_render_view.sceneRect().toRect().width(), 99.0 /  score_render_view.sceneRect().toRect().height());
//    score_render_view.resize(70, 99);
        QPixmap pixmap =  score_render_view.grab();
        //std::cout<<"done"<<std::endl;
        QString file_name = selected_dir + "/";
        file_name.append(c10::to_string(index).c_str());
        file_name.append(".png");
        pixmap.save(file_name, "PNG");
        index++;
    }

}
