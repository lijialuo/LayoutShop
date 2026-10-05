//
// Created by lijialuo on 2023/5/12.
//

#include "CustomGraphicsItem2.h"
#include "CustomGraphicsItem2.h"

#include <iostream>
#include <qcursor.h>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QSvgRenderer>
#include <qpainter.h>



CustomGraphicsItem2::CustomGraphicsItem2()
{
    //this->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
    this->setAcceptHoverEvents(true);
}

void CustomGraphicsItem2::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    double width = this->rect().width();
    double height = this->rect().height();
    QPen pen;
    pen.setWidth(0);
    pen.setColor(QColor(255, 255, 255, 0));
    QBrush brush;
    brush.setColor(LABEL_COLOR_ALT[node_type_]);
    brush.setStyle(Qt::SolidPattern);
    painter->setBrush(brush);
    painter->setPen(pen);
    painter->drawRect(0,0,width,height);
}

void CustomGraphicsItem2::UpdateSizeHandleRects()
{
    double x, y;
    double half_len = 5;
    double full_len = 2 * half_len;

    size_handle_rects.clear();

    x = 0;
    y = 0;
    size_handle_rects.push_back(QRect(x - half_len, y - half_len, full_len, full_len));

    x = this->rect().width() / 2 ;
    y = 0;
    size_handle_rects.push_back(QRect(x - half_len, y - half_len, full_len, full_len));

    x = this->rect().width();
    y = 0;
    size_handle_rects.push_back(QRect(x - half_len, y - half_len, full_len, full_len));

    x = 0;
    y = this->rect().height()  / 2;
    size_handle_rects.push_back(QRect(x - half_len, y - half_len, full_len, full_len));

    x = this->rect().width() ;
    y = this->rect().height() / 2 ;
    size_handle_rects.push_back(QRect(x - half_len, y - half_len, full_len, full_len));

    x = 0;
    y = this->rect().height();
    size_handle_rects.push_back(QRect(x - half_len, y - half_len, full_len, full_len));

    x = this->rect().width() / 2;
    y = this->rect().height();
    size_handle_rects.push_back(QRect(x - half_len, y - half_len, full_len, full_len));

    x = this->rect().width() ;
    y = this->rect().height();
    size_handle_rects.push_back(QRect(x - half_len, y - half_len, full_len, full_len));
}

