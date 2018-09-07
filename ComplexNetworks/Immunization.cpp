#pragma once

#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <random>
#include <fstream>
#include "GraphOrig.h"
#include "Immunization.h"
#include <algorithm> //added -DB
//#include "Katz.h"
//#include "betweennessCentrality.h"
//#include "adaptive-betweenness-centrality.h"
#include "NBR.h"

using namespace std;
//using namespace adaptive_betweenness_centrality;

Immunization::Immunization()
{
	
}

Immunization::Immunization(const GraphOrig &g, int num_removed_ndes, int M, int samp)
{
	// Initialize everything
	graph.nodes = g.nodes;
	graph.original_mapping = g.original_mapping;
	graph.orig_file = g.orig_file;
	//graph.weights = g.weights;
	
	num_removed_nodes = num_removed_ndes;
	int V = g.nodes.size();
	vector<double> btwss;
	vector<int> seeds;
	vector<vector<int>> new_nodes = g.nodes;
	// Use a method to get a list of nodes to be removed...
	//vector<int> seeds = Katz::adaptiveKatz2(graph, num_removed_ndes, iterations, closeness);
	//vector<int> seeds = betweennessCentrality::bcAdaptivek(graph, num_removed_ndes, 5); // the k is irrelevant - was used for adaptive k
	//adaptive_approximate_betweenness(new_nodes, M, V, seeds, btwss);
	seeds = NBR::get_bader_approx(g, g.nodes.size(), M, samp);
	node_removal_order = seeds;
	// make a removal list!
	vector <bool> removed_set;
	for (int i = 0; i < V; i++)
	{
		removed_set.push_back(false);
	}
	for (int i = 0; i < seeds.size(); i++)
	{
		removed_set[seeds[i]] = true;
		// create a new graph
		GraphOrig h(graph, removed_set);
		vector<int> cur_comps = h.connectedComponents();
		if (cur_comps.size() > 0) {
			comps.push_back(cur_comps.size());
			int max = *std::max_element(cur_comps.begin(), cur_comps.end()); // added std:: -DB
			largest_component.push_back(max);
		}
		else return;
	}
}

void Immunization::OutputReport(string filename)
{
	ofstream myfile;
	myfile.open(filename);
	myfile << "step,removed_node,num_components,largest_component" << endl;
	for (int i = 0; i < node_removal_order.size(); i++) {
		myfile << i << ", " <<node_removal_order[i] << ","<< comps[i] << "," << largest_component[i] << endl;
	}

	myfile.close();
}

