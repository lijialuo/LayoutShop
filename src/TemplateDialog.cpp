//
// Created by lijialuo on 2023/5/24.
//
#include "LayoutShop.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/stat.h>
#include "QLayout"
#include "TemplateDialog.h"
#include "ProjectPaths.h"
#include "ui_TemplateDialog.h"
#include "FlowLayout.h"
#include "QMouseEvent"


TemplateDialog::TemplateDialog(QWidget *parent) : QDialog(parent), ui(new Ui::TemplateDialog)
{
    ui->setupUi(this);
    this->setFixedSize(this->size());
    for(int i = 0; i < ui->tabWidget->count(); ++i)
    {
        QScrollArea *scroll = new QScrollArea(ui->tabWidget->widget(i));
        scroll->setFixedSize(ui->tabWidget->size().width()- 10, ui->tabWidget->size().height() - 10);
        QWidget *temp_widget = new QWidget(scroll);
        temp_widget->setLayout(new FlowLayout(10,10,10));
        temp_widget->setFixedWidth(scroll->width());
        //std::cout<< height<<std::endl;
        //scroll->setLayout(new QGridLayout());
        scroll->setWidget(temp_widget);
        //LoadTemplates();
        template_widget_list_.push_back(temp_widget);
    }
    QObject::connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(ChangeTap()));
    QObject::connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(SelectATemplate()));
    QObject::connect(this, SIGNAL(finished(int)), this, SLOT(ClearTemplates()));

    QObject::connect(ui->template_box, SIGNAL(EmitDel(int)),this,SLOT(RemoveATemplate(int)));
    QObject::connect( dynamic_cast<LayoutShop*>(this->parent())->ui.template_box, SIGNAL(EmitDel(int)),this,SLOT(RemoveATemplate(int)));

}

TemplateDialog::~TemplateDialog()
{
    delete ui;
}

void TemplateDialog::LoadTemplates()
{
    int img_num = ui->tabWidget->currentIndex();
    std::string dir = LayoutShopFindAsset(QString("templates/%1").arg(img_num)).toStdString();
    std::string list_file_dir = dir+ "/data_list.txt";
    std::ifstream infile;
    std::string buffer, item;
    std::istringstream iss;
    infile.open(list_file_dir.data());
    template_list_.clear();
    target_index_ = -1;
    ui->pushButton->setEnabled(false);
    std::string layout_file,img_file;
    while(getline(infile,buffer))
    {
        Template a_template;
        iss.clear();
        iss.str(buffer);
        iss >> layout_file;
        iss >> img_file;

        layout_file = std::string().append(dir).append("/").append(layout_file);
        img_file = std::string().append(dir).append("/").append(img_file);

        if (std::filesystem::exists(layout_file) && std::filesystem::exists(img_file))
        {
            a_template.layout_file = layout_file;
            a_template.img_file = img_file;
            QPixmap picture((a_template.img_file).data());
            //picture = picture.scaledToHeight(125,Qt::SmoothTransformation);
            a_template.label = new QLabel();
            //    a_template.label->setObjectName((std::to_string(img_num) + "_" + a_template.layout_file).data());
            a_template.label->setPixmap(picture);

            template_list_.push_back(a_template);
        }
    }
    infile.close();
        //ui->tabWidget->widget(img_num)->setLayout(new QGridLayout());

        auto widget = template_widget_list_[img_num];
        int width = widget->width();
        int single_width = template_list_[0].label->pixmap().width() + 10;
        int single_height = template_list_[0].label->pixmap().height() + 10;
        int size = template_list_.size();

        int more = (size % (width / single_width ) ) ? 1 : 0 ;
        int height = (size / (width / single_width) + more) * single_height + 10;
        widget->setFixedHeight(height);

        //ui->tabWidget->widget(img_num)->layout()->addWidget(new QScrollArea());
        //ui->tabWidget->widget(img_num)->layout()->addWidget(scroll);
       // scroll->layout()->addWidget(temp_widget);

        //ui->tabWidget->widget(img_num)->setLayout(new FlowLayout());
        for(const auto& temp : template_list_)
        {
            //std::cout<< temp.label->pixmap().size().width() <<std::endl;
//            ui->tabWidget->widget(img_num)->layout()->addWidget(temp.label);
            widget->layout()->addWidget(temp.label);
        }

}

int TemplateDialog::exec(int img_num)
{
    int old_img_num = ui->tabWidget->currentIndex();
    if(img_num == -1) img_num = old_img_num;
    if(old_img_num == img_num) LoadTemplates();
    else  if(img_num >= 0 && img_num < ui->tabWidget->count()) ui->tabWidget->setCurrentIndex(img_num);
    return QDialog::exec();
}

void TemplateDialog::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);

    if(event->button() == Qt::LeftButton)
    {
        int active_index = -1;
        for(int i = 0; i < template_list_.size(); ++i)
        {
            auto a_template = template_list_[i];
            auto pos = dynamic_cast<QWidget*>(a_template.label->parent())->mapFromGlobal(mapToGlobal(event->pos()));
            if(a_template.label->geometry().contains(pos))
            {
                active_index = i;
                break;
            }
        }

        if(active_index >= 0 && active_index < template_list_.size())
        {
            for(int i = 0; i < template_list_.size(); ++i)
            {
               template_list_[i].label->setStyleSheet("");
            }
            template_list_[active_index].label->setStyleSheet(QString("QLabel{border-style: solid;border-width: 3px;border-color: #569cc6;}"));
            target_index_ = active_index;
            ui->pushButton->setEnabled(true);
        }
    }

}

void TemplateDialog::SelectATemplate()
{
    auto children = ui->template_box->findChildren<QLabel*>();
    int temp_size;
    temp_size = children.size();
    if( temp_size < 3 && target_index_ >= 0 && target_index_ < template_list_.size())
    {
        TemplateLabel template_label;
        QPixmap pixmap( template_list_[target_index_].label->pixmap());
        ui->template_box->AddTemplate(pixmap, QString(template_list_[target_index_].layout_file.data()));
        dynamic_cast<LayoutShop*>(this->parent())->ui.template_box->AddTemplate(pixmap, QString(template_list_[target_index_].layout_file.data()));

//        label_list.push_back(picture_label);
//        this->layout()->addWidget(label);

    }
}

void TemplateDialog::ClearTemplates()
{
    for(int i = 0; i < template_list_.size(); ++i)
    {
        auto a_template = template_list_[i];
        a_template.label->deleteLater();
    }
    template_list_.clear();
    target_index_ = -1 ;
    ui->pushButton->setEnabled(false);
}

void TemplateDialog::ChangeTap()
{
    ClearTemplates();
    LoadTemplates();
}

void TemplateDialog::RemoveATemplate(int index)
{
    auto& label_list = ui->template_box->label_list;
    if(index >= 0 && index < label_list.size())
    {
        auto label = label_list[index].label;
        //delete label;
        if(label != nullptr)
        {
            label->deleteLater();
            //delete label;
        }
        label_list.erase(label_list.begin() + index);
    }


    auto& label_list_2 = dynamic_cast<LayoutShop*>(this->parent())->ui.template_box->label_list;
    if(index >= 0 && index < label_list_2.size())
    {
        auto label = label_list_2[index].label;
        //delete label;
        if(label != nullptr)
        {
            label->deleteLater();
            //delete label;
        }
        label_list_2.erase(label_list_2.begin() + index);
    }
}

void TemplateDialog::RemoveAll()
{
    for(int i = ui->template_box->label_list.size() - 1; i >= 0; --i)
    {
        RemoveATemplate(i);
    }
}
