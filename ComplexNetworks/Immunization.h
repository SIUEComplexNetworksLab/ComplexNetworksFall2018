#pragma once
#include "GraphOrig.h"
using namespace std;

class Immunization
{
public:
	GraphOrig graph;
	vector<int> node_removal_order;
	int num_removed_nodes;
	vector<int> comps;
	vector<int> largest_component;

	Immunization(); //  removed Immunization:: for each function -DB
	Immunization(const GraphOrig &g, int removed_nodes, int iterations, int samp);
	void OutputReport(string filename);
};