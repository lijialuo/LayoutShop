#pragma once
#include "LayoutEstimator.h"
#include <stack>
#include <QtWidgets/QMainWindow>
#include <rapidjson/document.h>

#include "Article.h"
#include "CCombineTreeNode.h"
#include "CLayoutTree.h"
#include "CombineTreeHandler.h"
#include "CustomGraphicsScene.h"
#include "LayoutGenerator.h"
#include "OutputWidget.h"
#include "ui_LayoutShop.h"
#include "CustomGraphicsScene.h"
#include "TemplateDialog.h"
#include "RefineWidget.h"

class LayoutShop : public QMainWindow
{
    Q_OBJECT

public:
    typedef struct
    {
        CustomGraphicsScene* actual_scene;
        CustomGraphicsScene* preview_scene;
    } Preview;
    LayoutShop(QWidget *parent = nullptr);
    ~LayoutShop();

    void SetDefaultScale(double paper_h);
   // bool eventFilter(QObject* watched, QEvent* event);
public:
    Ui::LayoutShopClass ui;



public slots:
    void LoadArticle();
    void LoadAll();
    void OpenLayoutGroup();
    void OpenNextLayout();
   // void GenerateLayout();
    void GenerateLayout();
	void GenerateLayoutMultiPage();
   // void SingleThreadGenerateLayout(std::vector<CCombineTreeNode*> single_thread_tree_list);
    void PreLayout();
    void NextLayout();
    void sliderToComboBox(int num_val);
    void comboBoxSelect(int index);
    void clickViewContentButton();
    void LayoutIndexInputChange();
    void ReceivePreviewIndex(int index);
    void ActiveEditNodeSize();
    void ActiveMoveNode();
    void ActiveAddText();
    void ActiveAddPadding();
    void ActiveRemoveNode();
    void ActiveHorizontalAlign();
    void ActiveVerticalAlign();
   // void ClickConfirm();
   // void ClickEdit();
   // void ClickClear();
    void ReOptimize();
    void SaveOutput();

    void Undo();
   // void Confirmlayouts();
    //void Clearlayouts();

   // void Editresult();
    //void Confirmresult();

    void AddPicture();
    void SelectTemplate();
    void GenerateButtonPush();
    void OutputImage();
    void OutputJSON();
    void OutputSVG();
    void OutputPDF();
    void OutputAll();
    void ClearInput();
    void resizeEvent(QResizeEvent * event ) override;

    void ChangeRefineWidgetValue();
    void RotateButtonPush();
    void Align_h_left_trigger();
    void Align_h_center_trigger();
    void Align_h_right_trigger();
    void Align_v_top_trigger();
    void Align_v_middle_trigger();
    void Align_v_bottom_trigger();
    void RefineTrigger();
    void RefineRegenerate();
    void SaveGRIDSLayout();
    void OutputImgScoreRender();
//    void TestGraphScore();
  
private:
    Article* article_ = nullptr;
    std::vector<CLayoutTree*> template_list_;
    CCombineTreeNode* combine_tree_root_ = nullptr; // combine tree
    CombineTreeHandler combine_tree_handler_; // template handler
    LayoutEstimator layout_estimator_;
    std::vector<std::vector<CCombineTreeNode*>> combine_tree_list_; //candidate templates
	std::vector<std::stack<std::vector<CCombineTreeNode*>>> combine_tree_stack_;  // edit history

    std::vector<Preview> preview_list_;
    CustomGraphicsScene::EDIT_TYPE edit_type_ = CustomGraphicsScene::EDIT_TYPE::NONE;
    bool multi_thread_ = false;
    int layout_index_ = -1; //current layout index
//    int preview_page_ = -1;
    int layout_num_ = 0; //total layout num
//    int page_num_ = 0;
    bool content_view_ = true;
    bool paragraph_ = false;
    bool cross_estimate_mode_ = false;
    // double real_w_ = 91.4,real_h_ = 121.9;
    double real_w_ = 21,real_h_ = 29.7;
    // double real_w_ = 40, real_h_ = 30;
    // double real_w_ = 24, real_h_ = 38;
    // double real_w_ = 27, real_h_ = 44;
    // double real_w_ = 42, real_h_ = 60;
    std::vector<CustomGraphicsScene*> delete_scene_list_;
	//std::vector<CCombineTreeNode*> show_valid_solutions_;
    TemplateDialog *template_dialog_ ;
    RefineWidget *refine_widget_;
    bool refine_active_ = true;

//    double scale_w1 = 1, scale_w2 = 1, scale_w3 = 1, scale_w4 = 1;
//    double scale_h1 = 1, scale_h2 = 1, scale_h3 = 1, scale_h4 = 1;

    bool multi_page_;
    int max_title_font_size_, min_title_font_size_, max_text_font_size_, min_text_font_size_;
    std::string title_font_type_, text_font_type_;

    double W,H;
    const int PREVIEW_LIST_SCROLL_MARGIN = 20, PREVIEW_LIST_SCROLL_H_SPACE = 10, PREVIEW_LIST_SCROLL_V_SPACE = 10;

    std::vector<std::pair<std::vector<int>, int>> ref_list_;

    void SetSceneEditType();
    void ClearOutput();

    void DestroyScene(CustomGraphicsScene* scene);
    void UpdateOutputAndPreview();
    void SaveFiles(CCombineTreeNode *tree,std::string dir);
    void RefineLayout(CCombineTreeNode* layout);
    void LoadTemplates();
    rapidjson::Value CreateJsonDFS(CCombineTreeNode *layout_node, rapidjson::MemoryPoolAllocator<> &allocator);
    void ResizePreview();
    void CreatePreviewWidgets();
    bool CheckCombineTreeSatisfied(CCombineTreeNode* combine_tree);
    void PresetImgCorres(std::vector<CCombineTreeNode*> layout_tree);
    void CreateRefineWidget();
    void ShowRefineWidget();
    void HideRefineWidget();
    void SetRefineWidgetValue();

};
