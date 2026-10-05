#include "CustomGraphicsView.h"

#include <qpainter.h>

CustomGraphicsView::CustomGraphicsView()
{

}

CustomGraphicsView::CustomGraphicsView(QWidget* widget)
{
}

void CustomGraphicsView::SetSelected(bool selected)
{
	selected_ = selected;
	if (selected) this->setStyleSheet(QString("CustomGraphicsView#") + this->objectName() +
		QString("{border-style: solid;border-width: 5px;border-color: #569cc6;}"));
	else this->setStyleSheet("");
}

void CustomGraphicsView::enterEvent(QEvent* event)
{
	this->setStyleSheet(QString("CustomGraphicsView#") + this->objectName() +
		QString("{border-style: solid;border-width: 5px;border-color: #D3D3D3;}"));
}

void CustomGraphicsView::leaveEvent(QEvent* event)
{
	if (selected_)
		this->setStyleSheet(QString("CustomGraphicsView#") + this->objectName() +
			QString("{border-style: solid;border-width: 5px;border-color: #569cc6;}"));
	else
		this->setStyleSheet("");
}

void CustomGraphicsView::mousePressEvent(QMouseEvent* event)
{
	emit ClickPreview(preview_index_);
}

void CustomGraphicsView::resizeEvent(QResizeEvent *event)
{
//    QGraphicsView::resizeEvent(event);
//    double target_width = this->width();
//    double new_scale_w = target_width;
//   // double new_scale_w_ratio = new_scale_w / preview_list_[i].preview_scene->width();
//    double new_scale_h = (this->scene()->height() / this->scene()->width()) * target_width ;
//    //double new_scale_h_ratio = new_scale_h / preview_list_[i].preview_scene->height();
//    //view->scale(new_scale_w_ratio, new_scale_h_ratio);
//    //view->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
//    //view->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
//    //ui.preview_list_scroll->widget()->layout()->addWidget(view);
//    //this->resize(new_scale_w, new_scale_h);

}

void CustomGraphicsView::ScaleLayout(double scale_val)
{
    double temp = scale_val;
    scale_val /= current_scale_val_;
    this->scale(scale_val, scale_val);
    current_scale_val_ = temp;
}
