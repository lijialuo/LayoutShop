//
// Created by lijialuo on 2023/5/23.
//

#ifndef CONTENTAWARELAYOUT_PICTUREBOX_H
#define CONTENTAWARELAYOUT_PICTUREBOX_H


#include <QtWidgets/QLabel>
#include <QMenu>
class PictureBox: public QWidget
{
Q_OBJECT

public:
    typedef struct PictureLabel
    {
        QLabel* label;
        QString file_dir;

    } PictureLabel;

    PictureBox();
    ~PictureBox();
    std::vector<PictureLabel> label_list;
    int target_index = -1;
    QMenu* menu_;

    void AddPicture(QPixmap picture, QString file_name);
    void RemovePicture(int target);
    void RemoveAll();

signals:
    void EmitDel(PictureLabel*);
protected:
    // 鼠标按下, 该函数被Qt框架调用, 需要重写该函数
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
};


#endif //CONTENTAWARELAYOUT_PICTUREBOX_H
