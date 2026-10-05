// Author: Anirudh Dagar 10/03/20

/*//////////////////////////////////////////////////////////////////////////
// Definition of the DGCN class, which uses the graph Convolutional        //
// operator DGCNConv from layers.h to implement                            //
// Semi-supervised Classification with Graph Convolutional Networks"      //
// <https://arxiv.org/abs/1609.02907> paper  used for Message Passing     //
// Neural Networks.                                                       //
//////////////////////////////////////////////////////////////////////////*/

#pragma once

#ifndef GCN_MODEL
#define GCN_MODEL

#include <iostream>
#include <Eigen/Core>
#include "DGCNConv.h"
#include <vector>

class DGCN
{
public:
    // Default Constructor
    DGCNConv conv;
    Eigen::MatrixXd lin1, lin2, lin3;
    Eigen::MatrixXd lin_out1, lin_out2;
    Eigen::MatrixXd bias1, bias2, bias3;
    Eigen::MatrixXd bias_out1, bias_out2;

    DGCN(){}
    DGCN (Eigen::MatrixXd lin1,Eigen::MatrixXd lin2,Eigen::MatrixXd lin3,
          Eigen::MatrixXd lin_out1, Eigen::MatrixXd lin_out2,
          Eigen::MatrixXd bias1,Eigen::MatrixXd  bias2,Eigen::MatrixXd  bias3,
          Eigen::MatrixXd bias_out1, Eigen::MatrixXd bias_out2)
    {
        conv = DGCNConv();
        this->lin1 = lin1;
        this->lin2 = lin2;
        this->lin3 = lin3;
        this->lin_out1 = lin_out1;
        this->lin_out2 = lin_out2;
        this->bias1 = bias1;
        this->bias2 = bias2;
        this->bias3 = bias3;
        this->bias_out1 = bias_out1;
        this->bias_out2 = bias_out2;
    }

        
    Eigen::MatrixXd forward(Eigen::MatrixXd x, Eigen::MatrixXd adj, Eigen::MatrixXd adj_in, Eigen::MatrixXd adj_out)
    {
        Eigen::MatrixXd x1, x2, x3;
        x = x * lin1;
        x1 = conv.forward(x, adj);
        x2 = conv.forward(x, adj_in);
        x3 = conv.forward(x, adj_out);

        for(int i=0;i<x1.rows();++i)
        {
            x1.row(i) = x1.row(i) + bias1;
            x2.row(i) = x2.row(i) + bias1;
            x3.row(i) = x3.row(i) + bias1;
        }

        x = Eigen::MatrixXd(x.rows(), x1.cols() + x2.cols() + x3.cols());
        x << x1, x2, x3;
        x = relu(x);

        x = x * lin2;
        x1 = conv.forward(x, adj);
        x2 = conv.forward(x, adj_in);
        x3 = conv.forward(x, adj_out);

        for(int i=0;i<x1.rows();++i)
        {
            x1.row(i) = x1.row(i) + bias2;
            x2.row(i) = x2.row(i) + bias2;
            x3.row(i) = x3.row(i) + bias2;
        }

        x = Eigen::MatrixXd(x.rows(), x1.cols() + x2.cols() + x3.cols());
        x << x1, x2, x3;
        x = relu(x);

        x = x * lin3;
        x1 = conv.forward(x, adj);
        x2 = conv.forward(x, adj_in);
        x3 = conv.forward(x, adj_out);

        for(int i=0;i<x1.rows();++i)
        {
            x1.row(i) = x1.row(i) + bias3;
            x2.row(i) = x2.row(i) + bias3;
            x3.row(i) = x3.row(i) + bias3;
        }

        x = Eigen::MatrixXd(x.rows(), x1.cols() + x2.cols() + x3.cols());
        x << x1, x2, x3;
        x = relu(x);

        x = x.row(0);

        x = x * lin_out1;
        for(int i=0;i<x.rows();++i)
        {
            x.row(i) = x.row(i) + bias_out1;
        }

        x = relu(x);
        x = x * lin_out2;
        for(int i=0;i<x.rows();++i)
        {
            x.row(i) = x.row(i) + bias_out2;
        }
        x = sigmoid(x);
        return x;
    }

    Eigen::MatrixXd relu(Eigen::MatrixXd &out)
    {
        return out.array().cwiseMax(0.0);
    }


    Eigen::MatrixXd sigmoid(Eigen::MatrixXd &out)
    {
        Eigen::MatrixXd res = (1.0 + (-out).array().exp()).inverse().matrix();
        return res;
    }

};

#endif //GCN_MODEL