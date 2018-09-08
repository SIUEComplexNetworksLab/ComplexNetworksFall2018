#pragma once
#include "GraphOrig.h"
#include "Graph.h"
using namespace std;

class Immunization
{
public:
	GraphOrig graph;
	Graph gr;
	vector<int> node_removal_order;
	int num_removed_nodes;
	vector<int> comps;
	vector<int> largest_component;

	Immunization(); //  removed Immunization:: for each function -DB
	Immunization(const GraphOrig &g, int removed_nodes, int iterations, int samp);
	Immunization(const Graph &g, int num_removed_ndes, int M, double closeness);
	void OutputReport(string filename);
};