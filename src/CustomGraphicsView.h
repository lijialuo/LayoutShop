#pragma once
#include <iostream>
#include <QGraphicsView>

class CustomGraphicsView :
    public QGraphicsView
{
    Q_OBJECT
public:
    CustomGraphicsView();
    CustomGraphicsView(QWidget* widget);
    void SetSelected(bool selected);
    void SetPreviewIndex(int index) { preview_index_ = index; }
    void ScaleLayout(double scale_val);
signals:
	void ClickPreview(int index);

protected:
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent * event ) override;
private:
    bool selected_ = false;
    int preview_index_;
    double current_scale_val_ = 1.0;
};

