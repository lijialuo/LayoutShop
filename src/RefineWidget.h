//
// Created by lijialuo on 2023/7/18.
//
#include <QWidget>
#include "ui_RefineWidget.h"

#ifndef CONTENTAWARELAYOUT_REFINEWIDGET_H
#define CONTENTAWARELAYOUT_REFINEWIDGET_H

namespace Ui {
    class Refine;
}

class RefineWidget: public QWidget
{
Q_OBJECT

public:
    Ui::Refine *ui;
    RefineWidget()
    {
        ui = new Ui::Refine;
        ui->setupUi(this);
    }

    signals:
        void SetActionUnchecked();
        void SetActionChecked();
protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;
};


#endif //CONTENTAWARELAYOUT_REFINEWIDGET_H
