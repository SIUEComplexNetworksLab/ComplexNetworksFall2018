#pragma once
// Here we are going to create a VAT object that keeps track of its graph, 
// Its node removal order, the number of removed nodes, and its VAT.
// This can be reused and so will not have to be recaluclated!

#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <random>
#include <fstream>
//#include "adaptive-betweenness-centrality.h"
#include "GraphOrig.h"

using namespace std;

class NBR
{
public:
	GraphOrig grph;
	vector<int> node_removal_order;
	int num_removed_nodes_vat;
	int num_removed_nodes_int;
	int num_removed_nodes_ten;
	double vat;
	double integrity;
	double tenacity;
	vector<int> comps;
	//double hi_int;
	//double hi_vat;
	//int hi_clusters;
	int hi_loc;
	vector<int> memoize;

	NBR();
	NBR(const GraphOrig &g, int M, int samp);
	void calculate_measure(vector<int> &removal_order, int num_removed, double &thevat, double &theintegrity);
	//static vector<vector<int>> find_components(const Graph &g);
	void connectedComponents(const GraphOrig &g);
	void DFSUtil(int v, bool visited[], const GraphOrig &g);
	static std::vector<int> get_bader_approx(const GraphOrig &g, int myK, int constant_c, int samp);
	// removed NBR:: from functions -DB
};