//
// Created by lijialuo on 2023/5/24.
//

#ifndef CONTENTAWARELAYOUT_TEMPLATEBOX_H
#define CONTENTAWARELAYOUT_TEMPLATEBOX_H
#include <QtWidgets/QLabel>
#include <QMenu>

class TemplateBox : public QWidget
{
    Q_OBJECT

public:
    typedef struct TemplateLabel
    {
        QLabel* label;
        QString file_dir;

    } TemplateLabel;

    TemplateBox();
    ~TemplateBox();
    std::vector<TemplateLabel> label_list;
    int target_index = -1;
    QMenu* menu_;

    void AddTemplate(QPixmap picture, QString file_name);

    signals:
            void EmitDel(int index);

protected:
    // 鼠标按下, 该函数被Qt框架调用, 需要重写该函数
    void mousePressEvent(QMouseEvent *event) override;
};


#endif //CONTENTAWARELAYOUT_TEMPLATEBOX_H
