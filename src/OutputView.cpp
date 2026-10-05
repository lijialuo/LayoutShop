#include "OutputView.h"

#include <iostream>
#include <QMouseEvent>
#include <qtextdocument.h>

#include "CustomGraphicsItem.h"


OutputView::OutputView(QWidget* widget)
{}

OutputView::~OutputView()
{}


void OutputView::scaleLayout(int slider_val)
{
	double scale_val;
	scale_val = static_cast<double>(slider_val) / 100;
	double temp = scale_val;
	scale_val /= current_scale_val_;
	this->scale(scale_val, scale_val);
	current_scale_val_ = temp;
}


