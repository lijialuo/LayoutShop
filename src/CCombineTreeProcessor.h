#pragma once

#include <QWidget>
#include <QPainter>
#include <qevent.h>
#include <vector>
#include "CLayoutTree.h"
#include "CMds.h"
#include "CNodeMatch.h"
#include <Eigen/Core>
#include <Eigen/Dense>
#include <iostream>

#include "CCombineTreeNode.h"

//多边形中顶点不仅包含位置还包含对应的layout
struct m_Point
{
	QPoint point;
	int corres_layout = 0;
};

struct m_Edge
{
	m_Point point1;
	m_Point point2;
};

struct m_Triangle
{
	//单个三角形的三个点
	std::vector<m_Point> sig_triangle_points;
	std::vector<m_Edge> sig_triangle_edges;
};

class CCombineTreeProcessor 
{
public:
	CCombineTreeProcessor();
	~CCombineTreeProcessor();

	void AllFunction();		//包含计算的所有函数

public:
	

	double alpha;
	double beta;
	double gamma;
	std::vector<CLayoutTree*> m_allLayout;
	m_Triangle navigation_triangle;		//用于explore的三角形
	CCombineTreeNode* combine_tree_root;



private:
	void ResetFunction();	  //将一些容器及变量重置一下
	void ConstructEachPairCost(std::vector<CLayoutTree*> m_allLayout);		//构建cost matrix
	void MdsFuction();		//降维函数
	void DelaunayFuction();		//三角刨分函数
	
	int FindThirdTree();

	void FindMinCostEdge(m_Triangle navigation_triangle, Eigen::MatrixXd cost_matrix);
	void CalAllCorrespond();	//计算所有的对应


private:
	Eigen::MatrixXd cost_matrix;	    //计算两两tree之间的cost,用于降维
	Eigen::MatrixXd result_matrix;		//降维后的结果矩阵

	m_Edge min_cost_edge;	//记录三角形里有最小cost的一条边。

};

