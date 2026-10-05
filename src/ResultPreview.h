#pragma once
#include <QGraphicsScene>
#include <qwidget.h>
#include "CCombineTreeNode.h"

class ResultPreview :
    public QWidget
{
public:
    ResultPreview(QWidget* widget);
    ResultPreview();
    ~ResultPreview(); 

    void AddPreview(QGraphicsScene* scene);
    QGraphicsScene* GetPreview(int index);
    void ClearAllPreview();
private:
    std::vector<QGraphicsScene*> preview_list_;

};

