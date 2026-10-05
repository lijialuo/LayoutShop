#include "CCombineTreeProcessor.h"
#include "CDelauntor.h"
#include <QPainterPath>

CCombineTreeProcessor::CCombineTreeProcessor()
{
	alpha = 0.0;
	beta = 0.0;
	gamma = 0.0;
	combine_tree_root = nullptr;
}

CCombineTreeProcessor::~CCombineTreeProcessor()
{
}

void CCombineTreeProcessor::ResetFunction()
{
	alpha = 0.0;
	beta = 0.0;
	gamma = 0.0;
}

void CCombineTreeProcessor::ConstructEachPairCost(std::vector<CLayoutTree*> m_allLayout)
{
	if (m_allLayout.size() < 3)
	{
		std::cout << "less than 3" << std::endl;
		return;
	}

	cost_matrix.resize(m_allLayout.size(), m_allLayout.size());
	cost_matrix.setZero();

	for (int i = 0; i < m_allLayout.size(); i++)
	{
		CNode* first_tree_root = m_allLayout[i]->GetRoot();
		for (int j = i; j < m_allLayout.size(); j++)
		{
			if (i == j)
			{
				cost_matrix(i, j) = 0;
			}
			else
			{
				CNode* second_tree_root = m_allLayout[j]->GetRoot();
				CNodeMatch node_match;
				cost_matrix(i, j) = node_match.ComputeCost(first_tree_root, second_tree_root);
				cost_matrix(j, i) = cost_matrix(i, j);
			}
		}
	}

	//std::cout << "cost_matrix================================" << std::endl;
	//cout << cost_matrix << endl;
	//std::cout << "cost_matrix=========================" << std::endl;
	return;
}

void CCombineTreeProcessor::MdsFuction()
{
	if (cost_matrix.rows() == 0 || cost_matrix.cols() == 0)
	{
		std::cout << "cost_matrix is empty" << std::endl;
		return;
	}

	// Compute MDS (embedding into a 2-dimensional space)
	result_matrix = MDS::computeMDS(cost_matrix, 2);

	/*std::cout << "Show the mds result" << std::endl;
	cout << result_matrix << endl;*/
	return;
}

void CCombineTreeProcessor::DelaunayFuction()
{
	if (result_matrix.rows() == 0 || result_matrix.cols() == 0)
	{
		std::cout << "result_matrix is empty" << std::endl;
		return;
	}
	/* x0, y0, x1, y1, ... */
	std::vector<double> coords;
	coords.clear();

	//std::cout << "查看result_matrix中每一个二维坐标*********" << std::endl;
	for (int j = 0; j < result_matrix.cols(); j++)
	{
		for (int i = 0; i < result_matrix.rows(); i++)
		{
			coords.push_back(result_matrix(i, j));
			//std::cout << result_matrix(i, j) << "     ";
		}
	}
	//std::cout << std::endl << "查看result_matrix中每一个二维坐标*********" << std::endl;

	//triangulation happens here
	delaunator::Delaunator d(coords);

	for (std::size_t i = 0; i < d.triangles.size(); i += 3)
	{
		printf(
			"Triangle points: [[%f, %f], [%f, %f], [%f, %f]]\n",
			d.coords[2 * d.triangles[i]],        //tx0
			d.coords[2 * d.triangles[i] + 1],    //ty0
			d.coords[2 * d.triangles[i + 1]],    //tx1
			d.coords[2 * d.triangles[i + 1] + 1],//ty1
			d.coords[2 * d.triangles[i + 2]],    //tx2
			d.coords[2 * d.triangles[i + 2] + 1] //ty2
		);
	}

	for (std::size_t i = 0; i < d.triangles.size(); i += 3)
	{
		navigation_triangle.sig_triangle_points.resize(3);

		//先平移60个单位在扩大3倍
		navigation_triangle.sig_triangle_points[0].point = QPoint(std::abs((d.coords[2 * d.triangles[i]] + 60) * 3 + 0.5), std::abs((d.coords[2 * d.triangles[i] + 1] + 60) * 3 + 0.5));
		for (int j = 0; j < result_matrix.cols(); j++)
		{
			if (d.coords[2 * d.triangles[i]] == result_matrix(0, j) && d.coords[2 * d.triangles[i] + 1] == result_matrix(1, j))
			{
				navigation_triangle.sig_triangle_points[0].corres_layout = j + 1;		//对应第几个layout
				break;
			}
		}

		navigation_triangle.sig_triangle_points[1].point = QPoint(std::abs((d.coords[2 * d.triangles[i + 1]] + 60) * 3 + 0.5), std::abs((d.coords[2 * d.triangles[i + 1] + 1] + 60) * 3 + 0.5));
		for (int j = 0; j < result_matrix.cols(); j++)
		{
			if (d.coords[2 * d.triangles[i + 1]] == result_matrix(0, j) && d.coords[2 * d.triangles[i + 1] + 1] == result_matrix(1, j))
			{
				navigation_triangle.sig_triangle_points[1].corres_layout = j + 1;		//对应第几个layout
				break;
			}
		}

		navigation_triangle.sig_triangle_points[2].point = QPoint(std::abs((d.coords[2 * d.triangles[i + 2]] + 60) * 3 + 0.5), std::abs((d.coords[2 * d.triangles[i + 2] + 1] + 60) * 3 + 0.5));
		for (int j = 0; j < result_matrix.cols(); j++)
		{
			if (d.coords[2 * d.triangles[i + 2]] == result_matrix(0, j) && d.coords[2 * d.triangles[i + 2] + 1] == result_matrix(1, j))
			{
				navigation_triangle.sig_triangle_points[2].corres_layout = j + 1;		//对应第几个layout
				break;
			}
		}
	}

	m_Edge edge_one, edge_two, edge_three;
	edge_one.point1 = navigation_triangle.sig_triangle_points[0];
	edge_one.point2 = navigation_triangle.sig_triangle_points[1];

	edge_two.point1 = navigation_triangle.sig_triangle_points[1];
	edge_two.point2 = navigation_triangle.sig_triangle_points[2];

	edge_three.point1 = navigation_triangle.sig_triangle_points[0];
	edge_three.point2 = navigation_triangle.sig_triangle_points[2];

	navigation_triangle.sig_triangle_edges.clear();
	navigation_triangle.sig_triangle_edges.push_back(edge_one);
	navigation_triangle.sig_triangle_edges.push_back(edge_two);
	navigation_triangle.sig_triangle_edges.push_back(edge_three);
}

void CCombineTreeProcessor::AllFunction()
{
	ResetFunction();
	ConstructEachPairCost(m_allLayout);
	MdsFuction();
	DelaunayFuction();
	FindMinCostEdge(navigation_triangle, cost_matrix);
	CalAllCorrespond();
	return;
}



int CCombineTreeProcessor::FindThirdTree()
{
	for (int i = 0; i < navigation_triangle.sig_triangle_points.size(); i++)
	{
		m_Point each_point = navigation_triangle.sig_triangle_points[i];
		if (each_point.corres_layout != min_cost_edge.point1.corres_layout && each_point.corres_layout != min_cost_edge.point2.corres_layout)
		{
			return each_point.corres_layout;
		}
	}
	return 0;
}

void CCombineTreeProcessor::FindMinCostEdge(m_Triangle navigation_triangle, Eigen::MatrixXd cost_matrix)
{
	int min_cost = 1000000;

	for (int i = 0; i < navigation_triangle.sig_triangle_edges.size(); i++)
	{
		m_Edge each_edge = navigation_triangle.sig_triangle_edges[i];

		if (cost_matrix(each_edge.point1.corres_layout - 1, each_edge.point2.corres_layout - 1) < min_cost)
		{
			min_cost = cost_matrix(each_edge.point1.corres_layout - 1, each_edge.point2.corres_layout - 1);
			min_cost_edge = each_edge;
		}
	}

	/*std::cout << "输出min_cost_edge_pos信息*******" << std::endl;
	std::cout << "min_cost_edge_pos对应的第一个点：" << min_cost_edge.point1.corres_layout << std::endl;
	std::cout << "min_cost_edge_pos对应的第二个点：" << min_cost_edge.point2.corres_layout << std::endl;
	std::cout << "输出min_cost_edge_pos信息*******" << std::endl;*/

	return;
}

void CCombineTreeProcessor::CalAllCorrespond()
{
	CNodeMatch m_nodeMathch;
	//first tree 与 second tree 形成 group tree
	std::cout << "first tree是: " << min_cost_edge.point1.corres_layout << std::endl;
	CNode* first_tree_root = m_allLayout[min_cost_edge.point1.corres_layout - 1]->GetRoot();
	CNode* second_tree_root = m_allLayout[min_cost_edge.point2.corres_layout - 1]->GetRoot();
	std::cout << "second tree是: " << min_cost_edge.point2.corres_layout << std::endl;

	m_nodeMathch.ComputeCsp(first_tree_root, second_tree_root);
	m_nodeMathch.CreateGroupTree();
	m_nodeMathch.AssignGroupTreeIndex();

	int corres_third_layout = FindThirdTree();
	std::cout << "third tree = " << corres_third_layout << std::endl;

	m_nodeMathch.SetThirdTree(m_allLayout[corres_third_layout - 1]->GetRoot());
	m_nodeMathch.ComputeGrpThdCsp();
	m_nodeMathch.CreateCombineTree();
	m_nodeMathch.AssignCombineTreeIndex();
	m_nodeMathch.GetCrpInFirstTwoTree();

	combine_tree_root = m_nodeMathch.GetCombineTreeRoot();
	return;
}