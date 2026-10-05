//
// Created by lijialuo on 2023/5/24.
//

#include "TemplateBox.h"
#include "QMouseEvent"
#include "QLayout"
#include <iostream>


TemplateBox::TemplateBox()
{
    menu_ = new QMenu();
    QAction* act = menu_->addAction("delete");
    QObject::connect(act, &QAction::triggered, this, [=]()
    {
        emit EmitDel(target_index);
    });
    //QObject::connect(this, SIGNAL(EmitDel(PictureLabel*)), root_widget, SLOT(DeletePicture(PictureLabel*)));
}


TemplateBox::~TemplateBox()
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

void TemplateBox::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    if(event->button() == Qt::RightButton)
    {
        auto children = this->findChildren<QLabel*>();
        for(int i = 0; i < children.size() ;++i)
        {
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

void TemplateBox::AddTemplate(QPixmap picture , QString file_name)
{
    QLabel *label = new QLabel();
    TemplateBox::TemplateLabel template_label;
    picture = picture.scaled(QSize(120,170),
                             Qt::KeepAspectRatio,
                             Qt::SmoothTransformation);
    label->setPixmap(picture);
    label->setAlignment(Qt::AlignCenter);
    template_label.label = label;
    template_label.file_dir = QString(file_name.data());
    label_list.push_back(template_label);
    this->layout()->addWidget(label);
}



