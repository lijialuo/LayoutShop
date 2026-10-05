#ifdef slots
#pragma push_macro("slots")
#undef slots
#define LAYOUTSHOP_RESTORE_QT_SLOTS
#endif
#include <torch/script.h>
#ifdef LAYOUTSHOP_RESTORE_QT_SLOTS
#pragma pop_macro("slots")
#undef LAYOUTSHOP_RESTORE_QT_SLOTS
#endif
#include <QPixmap>
#include "CCombineTreeNode.h"
#include "CustomGraphicsScene.h"
#include "DGCN.h"

class LayoutEstimator
{
    typedef struct StructureGraph
    {
        Eigen::MatrixXd x;
        Eigen::MatrixXd adj;
    } StructureGraph;
public:
    LayoutEstimator();
    double GetImgScore(QPixmap img);
    double GetStructureScore(StructureGraph structure);
    std::vector<std::vector<CCombineTreeNode*>> EstimateSinglePageDefault(std::vector<std::vector<CCombineTreeNode*>> layout_list);
    std::vector<std::vector<CCombineTreeNode*>> EstimateSinglePageCross(std::vector<std::vector<CCombineTreeNode*>> layout_list);
    std::vector<std::vector<CCombineTreeNode*>> EstimateSinglePage(std::vector<std::vector<CCombineTreeNode*>> layout_list, bool cross);

    CustomGraphicsScene* RenderScoreLayout(CCombineTreeNode *tree);
    void LoadParams();
    StructureGraph GetStructureGraph(CCombineTreeNode *tree);
    Eigen::MatrixXd GetUndirectedAdj(Eigen::MatrixXd adj);
    Eigen::MatrixXd GetInAdj(Eigen::MatrixXd adj);
    Eigen::MatrixXd GetOutAdj(Eigen::MatrixXd adj);

    static bool LayoutCompareScore(std::vector<CCombineTreeNode*> layout_1, std::vector<CCombineTreeNode*> layout_2);
    void SetW(double W){this->W = W;}
    void SetH(double H){this->H = H;}
    double GetThreshold(){return threshold_;}
    void EstimateTest(CNode* layout);
    void EstimateSingleLayout(std::vector<CCombineTreeNode*> layout);

private:
    std::shared_ptr<torch::jit::script::Module> img_model;
    DGCN dgcn_model;
    double W,H;
    double threshold_ = 0.3;

};
