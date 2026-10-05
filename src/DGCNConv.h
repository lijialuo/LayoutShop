// Author: Anirudh Dagar 10/03/20

/*//////////////////////////////////////////////////////////////////////////
// Definition of the DGCNConv class and relu method                        //
// for the graph convolutional operator from the paper:                   //
// Semi-supervised Classification with Graph Convolutional Networks"      //
// <https://arxiv.org/abs/1609.02907> paper  used for Message Passing     //
// Neural Networks.                                                       //
//////////////////////////////////////////////////////////////////////////*/

#pragma once

#ifndef LAYERS_MPNN
#define LAYERS_MPNN

#include <iostream>
#include <Eigen/Core>
#include <Eigen/LU>


class DGCNConv
{
private:

public:
    // Constructor
    DGCNConv()
    {}

    Eigen::MatrixXd forward(Eigen::MatrixXd xw, Eigen::MatrixXd adj)
    {
        int num_nodes = adj.rows();
        // Add self-loops to Adjacecny Matrix
        adj = adj + Eigen::MatrixXd::Identity(num_nodes, num_nodes);

        //Degree Diagonal Matrix D
        Eigen::MatrixXd D = Eigen::MatrixXd::Zero(num_nodes, num_nodes);
        for(int i=0; i<num_nodes; i++)
        {
            for(int j=0; j<num_nodes; j++)
            {
                if(i==j)
                {
                    D(i,j) = adj.rowwise().sum()(i);
                }
            }
        }

        // Symmetric Normalization of Adjacency Matrix
        D = D.inverse();
        D = D.cwiseSqrt();
        adj = (D * adj) * D;
        //Eigen::MatrixXd xw  = x * this->weight;
        Eigen::MatrixXd axw = adj * xw;
        return axw;
    }

};



#endif //LAYERS_MPNN