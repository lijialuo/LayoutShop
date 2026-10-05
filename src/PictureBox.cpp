//
// Created by lijialuo on 2023/5/23.
//

#include "PictureBox.h"
#include "QMouseEvent"
#include "QLayout"
#include <iostream>


PictureBox::PictureBox()
{
    menu_ = new QMenu();
    QAction* act = menu_->addAction("delete");
    QObject::connect(act, &QAction::triggered, this, [=]()
    {
        if(target_index >= 0 && target_index < label_list.size())
        {
            RemovePicture(target_index);
        }
    });
    //QObject::connect(this, SIGNAL(EmitDel(PictureLabel*)), root_widget, SLOT(DeletePicture(PictureLabel*)));
}


PictureBox::~PictureBox()
{
    if(menu_ != nullptr)
    {
        for(auto act : menu_->actions())
        {
            delete act;
        }
        delete menu_;
    }
}

void PictureBox::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    if(event->button() == Qt::RightButton)
    {
        auto children = this->findChildren<QLabel*>();
        for(int i = 0; i < children.size() ;++i)
        {
            std::cout<< children.size()<<std::endl;
            auto label =  children[i];
            auto pos = event->pos();

            if(label->geometry().contains(pos))
            {
                target_index = i;
                menu_->exec(QCursor::pos());
                target_index = -1;
                break;
            }
        }
    }
}

void PictureBox::AddPicture(QPixmap picture , QString file_name)
{
    QLabel *label = new QLabel();
    PictureBox::PictureLabel picture_label;
    picture = picture.scaledToHeight(125,Qt::SmoothTransformation);
//    picture = picture.scaled(QSize(125,125),
//                           Qt::KeepAspectRatio,
//                           Qt::SmoothTransformation);
    label->setPixmap(picture);
    label->setAlignment(Qt::AlignCenter);
    picture_label.label = label;
    picture_label.file_dir = QString(file_name.data());
    label_list.push_back(picture_label);
    this->layout()->addWidget(label);
}

void PictureBox::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);


}

void PictureBox::RemovePicture(int target)
{
    auto label = label_list[target].label;
    //delete label;
    if(label != nullptr)
    {
        label->deleteLater();
        //delete label;
    }

    label_list.erase(label_list.begin() + target);
}

void PictureBox::RemoveAll()
{
    for(int i = label_list.size() - 1; i >= 0; --i)
    {
        RemovePicture(i);
    }
}



