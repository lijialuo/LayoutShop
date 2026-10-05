//
// Created by lijialuo on 2023/7/18.
//

#include "RefineWidget.h"

void RefineWidget::closeEvent(QCloseEvent *event)
{
    emit SetActionUnchecked();
}

void RefineWidget::showEvent(QShowEvent *event)
{
    emit SetActionChecked();
}