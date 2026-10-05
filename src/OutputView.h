#pragma once
#include <qgraphicsview.h>

#include "Article.h"
#include "CCombineTreeNode.h"

class OutputView :
    public QGraphicsView
{
	Q_OBJECT

public:
	OutputView(QWidget* widget = nullptr);
	~OutputView();

	bool content_view_ = true;
	bool grid_view_ = false;

public slots:
	void scaleLayout(int slider_val);

private:
	double current_scale_val_ = 1.0;

};



