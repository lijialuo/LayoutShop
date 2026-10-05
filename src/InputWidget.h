#pragma once
#include <qwidget.h>
#include "Article.h"
#include "CCombineTreeNode.h"
class InputWidget :
    public QWidget
{
	//for using 'signal' and 'slot'
	Q_OBJECT

public:
	InputWidget();
	~InputWidget();

	void ConfirmInput();



};

