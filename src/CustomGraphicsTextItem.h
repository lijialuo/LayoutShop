#pragma once
#include <qfont.h>
#include <QGraphicsRectItem>

class CustomGraphicsTextItem :
    public QGraphicsRectItem
{
public:
    CustomGraphicsTextItem();
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) Q_DECL_OVERRIDE;
    std::vector<std::pair<double, double>> start_point_list_;
    std::vector<std::string> text_list_;
    QFont font;
};

