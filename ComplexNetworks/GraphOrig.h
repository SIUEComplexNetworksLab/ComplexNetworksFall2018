#pragma once

#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <random>
#include <fstream>
//#include "adaptive-betweenness-centrality.h"

using namespace std;

class GraphOrig
{
public:
	vector<vector<int> > nodes;
	vector<int> original_mapping;
	string orig_file;
	vector<int> removals;


	GraphOrig();
	GraphOrig(const GraphOrig &g);
	GraphOrig(const vector<vector<int> > &some_nodes);
	GraphOrig(string filename);
	GraphOrig(const GraphOrig &g, const vector<bool> &removed_set); // removed_set = true if removed


	void outputToGraph(string filename);
	void load_graph_from_Graph(const std::string& fn, std::vector<std::vector<int> >& G);
	vector<int> connectedComponents();
	void DFSUtil(int v, bool visited[], vector<int> & comps);
	// removed GraphOrig:: from functions -DB
};
