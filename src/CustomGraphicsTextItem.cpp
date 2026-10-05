#include "CustomGraphicsTextItem.h"
#include <QPainter>
CustomGraphicsTextItem::CustomGraphicsTextItem()
{

}


void CustomGraphicsTextItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	painter->setFont(font);
	painter->setPen(Qt::black);
	for(int i = 0; i < start_point_list_.size(); ++ i)
	{
		auto start_point = start_point_list_[i];
		painter->drawText(start_point.first, start_point.second, text_list_[i].data());
	}
}