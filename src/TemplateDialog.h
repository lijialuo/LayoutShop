//
// Created by lijialuo on 2023/5/24.
//

#ifndef LAYOUTSHOP_TEMPLATEDIALOG_H
#define LAYOUTSHOP_TEMPLATEDIALOG_H

#include <QDialog>
#include <QtWidgets/QLabel>


namespace Ui {
    class TemplateDialog;
}

class TemplateDialog : public QDialog
{
    Q_OBJECT
    typedef struct TemplateLabel
    {
        QLabel* label;
        QString file_dir;

    } TemplateLabel;

    typedef struct Template
    {
        QLabel* label;
        std::string layout_file;
        std::string img_file;
    } Template;

public:
    explicit TemplateDialog(QWidget *parent = nullptr);
    ~TemplateDialog() override;
    int exec(int img_num);
    public slots:
    void SelectATemplate();
    void ClearTemplates();
    void ChangeTap();
    void RemoveATemplate(int index);
    void RemoveAll();

private:
    Ui::TemplateDialog *ui;
    std::vector<Template> template_list_;
    std::vector<QWidget*> template_widget_list_;
    //std::map<int, std::vector<Template>> template_map_;
    //Template *target_template_ = nullptr;
    int target_index_ = -1;

    void LoadTemplates();



protected:
    void mousePressEvent(QMouseEvent *event) override;
};



#endif //LAYOUTSHOP_TEMPLATEDIALOG_H
