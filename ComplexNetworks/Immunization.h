#pragma once
#include "graph/GraphOrig.h"
#include "Graph2.h"
#include "graph/Graph.h"
#include <vector>
using namespace std;

class Immunization
{
public:
	GraphOrig graph;
	Graph2 gr;
	NetworKit::Graph h;
	vector<int> node_removal_order;
	int num_removed_nodes;
	vector<int> comps;
	vector<int> largest_component;

	Immunization(); //  removed Immunization:: for each function -DB
	Immunization(const GraphOrig &g, int removed_nodes, int iterations, int samp);
	Immunization(const Graph2 &g, int num_removed_ndes, int M, double closeness);
	Immunization(const GraphOrig &g, int num_removed_ndes, double epsilon, double delta);
	Immunization(const GraphOrig &g, int num_removed_ndes, int samples, bool normalized, bool in_parallel);
	void OutputReport(string filename);
	vector<int> get_bc_approx(const GraphOrig &g, int num_removed_nodes, double epsilon, double delta); // NetworKit
	vector<int> get_bc_approx(const GraphOrig & g, int num_removed_nodes, int k, double delta, double err); // KADABRA
	vector<int> get_bc_estim(const GraphOrig &g, int num_removed_nodes, int samples, bool normalized, bool in_parallel);
};