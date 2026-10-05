#include "LayoutEstimator.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QGraphicsView>
#include <chrono>
#include "ProjectPaths.h"

double LayoutEstimator::GetImgScore(QPixmap img)
{
    //auto tensor = torch::ones({1,3,70,99});
    img = img.scaled(70, 99);
    QImage image = img.toImage();
    float stack_array[1][3][70][99];
    for(size_t i = 0; i < 70; ++i)
    {
        for(size_t j = 0; j < 99; ++j)
        {
            QRgb rgb = image.pixel(i,j);
            stack_array[0][0][i][j] = qRed(rgb);
            stack_array[0][1][i][j] = qGreen(rgb);
            stack_array[0][2][i][j] = qBlue(rgb);
        }
    }
    torch::Tensor tensor = torch::from_blob(stack_array, { 1, 3, 70, 99}, dtype(torch::kFloat)).to(at::kCPU);
//    for(int i = 0; i < img.width();++i)
//    {
//        for(int j = 0; j < img.height(); ++j)
//        {
//            QRgb rgb = image.pixel(i,j);
//            tensor.index_put_({0, 0, i, j}, qRed(rgb));
//            tensor.index_put_({0, 1, i, j}, qGreen(rgb));
//            tensor.index_put_({0, 2, i, j}, qBlue(rgb));
//        }
//    }
    auto start = std::chrono::steady_clock::now();
    std::vector<torch::jit::IValue>  input_to_net;
    input_to_net.push_back(tensor);
    auto output = img_model->forward(input_to_net).toTensor();
    auto end = std::chrono::steady_clock::now();
    std::cout << "eval time: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << " ms" << std::endl;
    //std::cout<<"done"<<std::endl;
    auto output_accessor = output.accessor<float, 2>();
    double output_val = output_accessor[0][0];
    std::cout<<output_val<<std::endl;
    return output_val;
}

double LayoutEstimator::GetStructureScore(StructureGraph structure)
{
    auto start = std::chrono::steady_clock::now();
    Eigen::MatrixXd adj_undirected = GetUndirectedAdj(structure.adj);
    Eigen::MatrixXd adj_in = GetInAdj(structure.adj);
    Eigen::MatrixXd adj_out = GetOutAdj(structure.adj);

    auto model_output = dgcn_model.forward(structure.x, adj_undirected, adj_in, adj_out);
    auto end = std::chrono::steady_clock::now();
    std::cout << "structure eval time: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << " ms" << std::endl;
    std::cout<<model_output(0, 0) << std::endl;
    return model_output(0,0);
}

std::vector<std::vector<CCombineTreeNode*>> LayoutEstimator::EstimateSinglePageDefault(std::vector<std::vector<CCombineTreeNode*>> layout_list)
{
    threshold_ = 0.3;
    for(int i = 0; i < layout_list.size(); ++ i)
    {
        EstimateSingleLayout(layout_list[i]);
    }
    std::vector<std::vector<CCombineTreeNode*>> remain_list;
    std::vector<std::vector<CCombineTreeNode*>> single_filter_list;
    std::vector<std::vector<CCombineTreeNode*>> double_filter_list;

    for(auto layout:layout_list)
    {
        if(layout[0]->img_score < 0.30 || layout[0]->structure_score < 0.30)
        {
            if(layout[0]->img_score < 0.30 && layout[0]->structure_score < 0.30)
                double_filter_list.push_back(layout);
            else
                single_filter_list.push_back(layout);
        }
        else
        {
            remain_list.push_back(layout);
        }
    }

    std::sort(remain_list.begin(), remain_list.end(), LayoutCompareScore);
    std::sort(single_filter_list.begin(), single_filter_list.end(), LayoutCompareScore);
    std::sort(double_filter_list.begin(), double_filter_list.end(), LayoutCompareScore);

    int del_index = 0;
    if(remain_list.size() < 20)
    {
        int insert_num = 20 - remain_list.size();
        while(del_index < insert_num && del_index < single_filter_list.size())
        {
            remain_list.push_back(single_filter_list[del_index]);
            del_index ++;
        }
    }
    while(del_index < single_filter_list.size())
    {
        for(auto item:single_filter_list[del_index])
            CCombineTreeNode::DeepDestroy(item);
        del_index ++;
    }
    single_filter_list.clear();

    del_index = 0;
    if(remain_list.size() < 20)
    {
        int insert_num = 20 - remain_list.size();
        while(del_index < insert_num && del_index < double_filter_list.size())
        {
            remain_list.push_back(double_filter_list[del_index]);
            del_index ++;
        }
    }
    while(del_index < double_filter_list.size())
    {
        for(auto item:double_filter_list[del_index])
            CCombineTreeNode::DeepDestroy(item);
        del_index ++;
    }
    double_filter_list.clear();

    return remain_list;
    //std::sort(layout_list.begin(), layout_list.end(), LayoutCompareScore);
    //return  layout_list;

}

LayoutEstimator::LayoutEstimator()
{
    const QString img_model_path = LayoutShopFindAsset("model/img_model.pt");
    try
    {
        if (QFileInfo::exists(img_model_path))
        {
            img_model = std::make_shared<torch::jit::script::Module>(
                    torch::jit::load(img_model_path.toStdString()));
        }
        else
        {
            std::cerr << "image model not found: " << img_model_path.toStdString() << std::endl;
        }
//                torch::jit::load(
//                        "/Users/lijialuo/project/scoring_network/traced_img_model.pt"));
        if (img_model)
        {
            img_model->eval();
        }
        torch::jit::getProfilingMode() = false;
        torch::jit::setGraphExecutorOptimize(false);
        torch::jit::getExecutorMode() = false;
        // Deserialize the ScriptModule from a file using torch::jit::load().
        //img_model = torch::jit::load("/Users/lijialuo/project/scoring_network/model.pt");
//        std::vector<torch::jit::IValue> inputs;
//        inputs.push_back(torch::zeros({1, 3, 70, 97}));
    }
    catch (const c10::Error& e)
    {
        std::cerr << "error loading the model\n";
    }
    LoadParams();

}


CustomGraphicsScene* LayoutEstimator::RenderScoreLayout(CCombineTreeNode *tree)
{
    std::queue<CCombineTreeNode*> node_queue;
    node_queue.push(tree);
    std::unordered_map<int, CustomGraphicsItem*> non_leaf_item_map;
    CustomGraphicsItem* root_item;
    while (!node_queue.empty())
    {
        auto node = node_queue.front();
        node_queue.pop();
        CustomGraphicsItem* node_item = new CustomGraphicsItem();
        //node_item->SetPType(render_type);
        node_item->SetNodeType(node->node_present_type);
        node_item->SetTreeNode(node);
        CustomGraphicsItem* parent_item = nullptr;
        if (node->parent != nullptr)
        {
            parent_item = non_leaf_item_map[node->parent->index];
            node_item->setParentItem(parent_item);
            node_item->setPos(node->x - node->parent->x, node->y - node->parent->y);
        }
        else
        {
            root_item = node_item;
            node_item->setPos(node->x, node->y);
        }

        node_item->setRect(0, 0, node->width, node->height);
        if (node->children.size() >0)
        {
            non_leaf_item_map[node->index] = node_item;
        }
        for (int  i = 0; i < node->children.size(); i++)
        {
            node_queue.push(node->children[i]);
        }
    }

    CustomGraphicsItem* paper_item = new CustomGraphicsItem();
    //paper_item->SetRenderType(render_type);
    paper_item->SetNodeType(NODE_PRESENT_TYPE::NONLABEL);
    paper_item->SetTreeNode(nullptr);
    paper_item->setPos(0, 0);
    paper_item->setRect(0, 0, W, H);
    root_item->setParentItem(paper_item);
    CustomGraphicsScene* scene = new CustomGraphicsScene();
    scene->SetPaintType(CustomGraphicsScene::PAINT_TYPE::SCORE);
    scene->addItem(paper_item);

    return scene;
}



bool LayoutEstimator::LayoutCompareScore(std::vector<CCombineTreeNode*> layout_1, std::vector<CCombineTreeNode*> layout_2)
{
    return layout_1[0]->total_score > layout_2[0]->total_score;
}

LayoutEstimator::StructureGraph LayoutEstimator::GetStructureGraph(CCombineTreeNode *tree)
{
    std::queue<CCombineTreeNode*> node_queue;
    std::vector<CCombineTreeNode*> node_list;
    node_queue.push(tree);
    while(!node_queue.empty())
    {
        auto node = node_queue.front();
        node_queue.pop();

        node->index = node_list.size();
        node_list.push_back(node);

        for(auto child:node->children)
        {
            node_queue.push(child);
        }
    }

    Eigen::MatrixXd x = Eigen::MatrixXd::Zero(node_list.size(), 10);
    Eigen::MatrixXd adj = Eigen::MatrixXd::Zero(node_list.size(), node_list.size());
    for(int i = 0 ; i < node_list.size(); ++i)
    {
        auto node = node_list[i];
        x.row(i) << node->x / W, node->y / H, node->width / W, node->height / H, 0, 0, 0, 0, 0, 0;
        if(node->node_present_type == NODE_PRESENT_TYPE::NONLABEL)
        {
            if(node->node_relation == NODE_RELATION::HORIZONTAL) x(i,8) = 1;
            else    x(i, 9) = 1;
        }
        else    x(i, 4 + node->node_present_type) = 1;

        for(auto child : node->children)
        {
            adj(child->index, i) = 1;
        }
    }

    StructureGraph res;
    res.x = x;
    res.adj = adj;
    return res;

}

Eigen::MatrixXd LayoutEstimator::GetUndirectedAdj(Eigen::MatrixXd adj)
{
    for(int i=0; i < adj.rows(); ++i)
    {
        for(int j = 0; j < adj.cols(); ++j)
        {
            if(adj(i,j) == 1)
            {
                adj(j,i) = 1;
            }
        }
    }
    return adj;
}

Eigen::MatrixXd LayoutEstimator::GetInAdj(Eigen::MatrixXd adj)
{
    std::vector<int> out_degree;
    for(int i = 0; i < adj.rows(); ++i)
    {
        int degree = 0;
        for(int j = 0; j < adj.cols(); ++j)
        {
            if(adj(i,j) != 0)
                degree ++;
        }
        if (degree == 0) degree += 1;
        out_degree.push_back(degree);
    }
    Eigen::MatrixXd adj_in = Eigen::MatrixXd::Zero(adj.rows(), adj.cols());
    for(int i = 0; i < adj.rows(); ++i)
    {
        adj_in += adj.row(i).transpose() * adj.row(i) / out_degree[i];
    }

    return adj_in;
}

Eigen::MatrixXd LayoutEstimator::GetOutAdj(Eigen::MatrixXd adj)
{
    std::vector<int> in_degree;
    for(int j = 0; j < adj.cols(); ++j)
    {
        int degree = 0;
        for(int i = 0; i < adj.rows(); ++i)
        {
            if(adj(i,j) != 0)
                degree ++;
        }
        if (degree == 0) degree += 1;
        in_degree.push_back(degree);
    }
    Eigen::MatrixXd adj_out = Eigen::MatrixXd::Zero(adj.rows(), adj.cols());
    for(int i = 0; i < adj.rows(); ++i)
    {
        adj_out += adj.col(i) * adj.col(i).transpose() / in_degree[i];
    }

    return adj_out;
}

void LayoutEstimator::LoadParams()
{
    Eigen::MatrixXd lin1, lin2, lin3;
    Eigen::MatrixXd lin_out1, lin_out2;
    Eigen::MatrixXd bias1, bias2, bias3;
    Eigen::MatrixXd bias_out1, bias_out2;

    const QString params_path = LayoutShopFindAsset("model/graph_model.txt");
    std::ifstream input_file(params_path.toStdString());
    if(!input_file.is_open())
    {
        std::cout<<"Empty input: "<<params_path.toStdString()<<std::endl;
        return;
    }

    int rows,cols;
    input_file>>rows;
    input_file>>cols;
    bias1 = Eigen::MatrixXd::Zero(rows,cols);
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            input_file >> bias1(i,j);
        }
    }

    input_file>>rows;
    input_file>>cols;
    bias2 = Eigen::MatrixXd::Zero(rows,cols);
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            input_file >> bias2(i,j);
        }
    }

    input_file>>rows;
    input_file>>cols;
    bias3 = Eigen::MatrixXd::Zero(rows,cols);
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            input_file >> bias3(i,j);
        }
    }

    input_file>>rows;
    input_file>>cols;
    lin1 = Eigen::MatrixXd::Zero(rows,cols);
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
           input_file >> lin1(i,j);
        }
    }


    input_file>>rows;
    input_file>>cols;

    lin2 = Eigen::MatrixXd::Zero(rows,cols);
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            input_file >> lin2(i,j);
        }
    }

    input_file>>rows;
    input_file>>cols;
    lin3 = Eigen::MatrixXd::Zero(rows,cols);
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            input_file >> lin3(i,j);
        }
    }


    input_file>>rows;
    input_file>>cols;
    lin_out1 = Eigen::MatrixXd::Zero(rows,cols);
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            input_file >> lin_out1(i,j);
        }
    }

    input_file>>rows;
    input_file>>cols;
    bias_out1 = Eigen::MatrixXd::Zero(rows,cols);
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            input_file >> bias_out1(i,j);
        }
    }

    input_file>>rows;
    input_file>>cols;
    lin_out2 = Eigen::MatrixXd::Zero(rows,cols);
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            input_file >> lin_out2(i,j);
        }
    }

    input_file>>rows;
    input_file>>cols;

    bias_out2 = Eigen::MatrixXd::Zero(rows,cols);
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            input_file >> bias_out2(i,j);
        }
    }

    input_file.close();

    dgcn_model.lin1 = lin1;
    dgcn_model.lin2 = lin2;
    dgcn_model.lin3 = lin3;
    dgcn_model.bias1 = bias1;
    dgcn_model.bias2 = bias2;
    dgcn_model.bias3 = bias3;
    dgcn_model.lin_out1 = lin_out1;
    dgcn_model.lin_out2 = lin_out2;
    dgcn_model.bias_out1 = bias_out1;
    dgcn_model.bias_out2 = bias_out2;
}

void LayoutEstimator::EstimateTest(CNode *layout)
{
    /*score score*/
    StructureGraph  structure_graph;
    std::queue<CNode*> node_queue;
    std::vector<CNode*> node_list;
    node_queue.push(layout);
    while(!node_queue.empty())
    {
        auto node = node_queue.front();
        node_queue.pop();

        node->index = node_list.size();
        node_list.push_back(node);

        for(auto child:node->children)
        {
            node_queue.push(child);
        }
    }

    Eigen::MatrixXd x = Eigen::MatrixXd::Zero(node_list.size(), 10);
    Eigen::MatrixXd adj = Eigen::MatrixXd::Zero(node_list.size(), node_list.size());
    for(int i = 0 ; i < node_list.size(); ++i)
    {
        auto node = node_list[i];
        x.row(i) << node->x / W, node->y / H, node->width / W, node->height / H, 0, 0, 0, 0, 0, 0;
        if(node->node_present_type == NODE_PRESENT_TYPE::NONLABEL)
        {
            if(node->node_relation == NODE_RELATION::HORIZONTAL) x(i,8) = 1;
            else    x(i, 9) = 1;
        }
        else    x(i, 4 + node->node_present_type) = 1;

        for(auto child : node->children)
        {
            adj(child->index, i) = 1;
        }
    }

    structure_graph.x = x;
    structure_graph.adj = adj;
    std::cout<<x<<std::endl;
    std::cout<<adj<<std::endl;

    double structure_score  = GetStructureScore(structure_graph);
    std::cout<< structure_score <<std::endl;

}

std::vector<std::vector<CCombineTreeNode *>>
LayoutEstimator::EstimateSinglePageCross(std::vector<std::vector<CCombineTreeNode *>> layout_list)
{
    for(int i = 0; i < layout_list.size(); ++ i)
    {
        EstimateSingleLayout(layout_list[i]);
    }

    std::vector<std::vector<CCombineTreeNode*>> positive_list;
    std::vector<std::vector<CCombineTreeNode*>> negative_list;
    std::vector<std::vector<CCombineTreeNode*>> cross_list;
    int low_level_negative_num = 0, high_level_negative_num = 0;
    for(auto layout : layout_list)
    {
        if(layout[0]->img_score < 0.30 || layout[0]->structure_score < 0.30)  low_level_negative_num ++;
        else if(layout[0]->img_score < 0.50 || layout[0]->structure_score < 0.50) high_level_negative_num ++;
    }
    if(static_cast<double>(low_level_negative_num) / layout_list.size() < 1.0 / 3.0)
    {
        threshold_ = 0.5;
    }
    else threshold_ = 0.3;
    for(auto layout:layout_list)
    {
        if(layout[0]->img_score < threshold_ || layout[0]->structure_score < threshold_)
        {
            negative_list.push_back(layout);
        }
        else
        {
            positive_list.push_back(layout);
        }
    }

    std::sort(positive_list.begin(), positive_list.end(), LayoutCompareScore);
    std::sort(negative_list.begin(), negative_list.end(), LayoutCompareScore);
    srand((int)time(0));
    int i,j;
    for( i = 0, j = 0; i < positive_list.size() && j < negative_list.size(); ++i, ++j)
    {
        double rand_num = rand() / double(RAND_MAX);
        if(rand_num < 0.5)
        {
            cross_list.push_back(positive_list[i]);
            cross_list.push_back(negative_list[j]);
        }
        else
        {
            cross_list.push_back(negative_list[j]);
            cross_list.push_back(positive_list[i]);
        }
    }
    for(; i < positive_list.size(); ++i)
    {
        cross_list.push_back(positive_list[i]);
    }
    for(; j < negative_list.size(); ++j)
    {
        cross_list.push_back(negative_list[j]);
    }
    return cross_list;
}

std::vector<std::vector<CCombineTreeNode *>>
LayoutEstimator::EstimateSinglePage(std::vector<std::vector<CCombineTreeNode *>> layout_list, bool cross)
{
    for(int i = 0; i < layout_list.size(); ++ i)
    {
        EstimateSingleLayout(layout_list[i]);
    }
    if(cross)
    {
        std::vector<std::vector<CCombineTreeNode*>> positive_list;
        std::vector<std::vector<CCombineTreeNode*>> negative_list;
        std::vector<std::vector<CCombineTreeNode*>> cross_list;

        int low_level_negative_num = 0, high_level_negative_num = 0;
        for(auto layout : layout_list)
        {
            if(layout[0]->img_score < 0.30 || layout[0]->structure_score < 0.30)  low_level_negative_num ++;
            else if(layout[0]->img_score < 0.50 || layout[0]->structure_score < 0.50) high_level_negative_num ++;
        }
        if(static_cast<double>(low_level_negative_num) / layout_list.size() < 1.0 / 3.0)
        {
            threshold_ = 0.5;
        }
        else threshold_ = 0.3;

        for(auto layout:layout_list)
        {
            if(layout[0]->img_score < threshold_ || layout[0]->structure_score < threshold_)
            {
                negative_list.push_back(layout);
            }
            else
            {
                positive_list.push_back(layout);
            }
        }

        std::sort(positive_list.begin(), positive_list.end(), LayoutCompareScore);
        std::sort(negative_list.begin(), negative_list.end(), LayoutCompareScore);
        srand((int)time(0));
        int i,j;
        for( i = 0, j = 0; i < positive_list.size() && j < negative_list.size(); ++i, ++j)
        {
            double rand_num = rand() / double(RAND_MAX);
            if(rand_num < 0.5)
            {
                cross_list.push_back(positive_list[i]);
                cross_list.push_back(negative_list[j]);
            }
            else
            {
                cross_list.push_back(negative_list[j]);
                cross_list.push_back(positive_list[i]);
            }
        }
        for(; i < positive_list.size(); ++i)
        {
            cross_list.push_back(positive_list[i]);
        }
        for(; j < negative_list.size(); ++j)
        {
            cross_list.push_back(negative_list[j]);
        }
        return cross_list;
    }
    else
    {
        threshold_ = 0.3;
        std::vector<std::vector<CCombineTreeNode*>> remain_list;
        std::vector<std::vector<CCombineTreeNode*>> single_filter_list;
        std::vector<std::vector<CCombineTreeNode*>> double_filter_list;

        for(auto layout:layout_list)
        {
            if(layout[0]->img_score < 0.30 || layout[0]->structure_score < 0.30)
            {
                if(layout[0]->img_score < 0.30 && layout[0]->structure_score < 0.30)
                    double_filter_list.push_back(layout);
                else
                    single_filter_list.push_back(layout);
            }
            else
            {
                remain_list.push_back(layout);
            }
        }

        std::sort(remain_list.begin(), remain_list.end(), LayoutCompareScore);
        std::sort(single_filter_list.begin(), single_filter_list.end(), LayoutCompareScore);
        std::sort(double_filter_list.begin(), double_filter_list.end(), LayoutCompareScore);

        int del_index = 0;
        if(remain_list.size() < 20)
        {
            int insert_num = 20 - remain_list.size();
            while(del_index < insert_num && del_index < single_filter_list.size())
            {
                remain_list.push_back(single_filter_list[del_index]);
                del_index ++;
            }
        }
        while(del_index < single_filter_list.size())
        {
            for(auto item:single_filter_list[del_index])
                CCombineTreeNode::DeepDestroy(item);
            del_index ++;
        }
        single_filter_list.clear();

        del_index = 0;
        if(remain_list.size() < 20)
        {
            int insert_num = 20 - remain_list.size();
            while(del_index < insert_num && del_index < double_filter_list.size())
            {
                remain_list.push_back(double_filter_list[del_index]);
                del_index ++;
            }
        }
        while(del_index < double_filter_list.size())
        {
            for(auto item:double_filter_list[del_index])
                CCombineTreeNode::DeepDestroy(item);
            del_index ++;
        }
        double_filter_list.clear();

        return remain_list;
    }
}

void LayoutEstimator::EstimateSingleLayout(std::vector<CCombineTreeNode *> layout)
{
    auto scene =  RenderScoreLayout(layout[0]);
    QGraphicsView score_render_view;
    score_render_view.setScene(scene);
    score_render_view.scale(70.0 / score_render_view.sceneRect().toRect().width(), 99.0 / score_render_view.sceneRect().toRect().height());
    score_render_view.resize(70, 99);
    QPixmap pixmap =  score_render_view.grab();
    //std::cout<<"done"<<std::endl;
    layout[0]->img_score = GetImgScore(pixmap);
    delete scene;
    StructureGraph  structure_graph = GetStructureGraph(layout[0]);
    layout[0]->structure_score  = GetStructureScore(structure_graph);
    layout[0]->total_score = layout[0]->img_score + layout[0]->structure_score;
}

