#pragma once

#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <random>
#include <fstream>
#include "Graph2.h"
#include "graph/GraphOrig.h"
#include "Immunization.h"
#include <algorithm> //added -DB
//#include "Katz.h"
//#include "betweennessCentrality.h"
#include "adaptive-betweenness-centrality.h"
#include "NBR.h"
//#include "centrality/ApproxBetweenness.h"
//#include "centrality/EstimateBetweenness.h"
#include "graph/Graph.h"
#include "KADABRA/Probabilistic.h"
#include "Globals.h"

using namespace std;
//using namespace adaptive_betweenness_centrality;

Immunization::Immunization()
{
	
}
// Added from Networkit
// Constructor to do the Approx
Immunization::Immunization(const GraphOrig &g, int num_removed_ndes, double epsilon, double delta)
{
	// Initialize everything
	graph.nodes = g.nodes;
	graph.original_mapping = g.original_mapping;
	graph.orig_file = g.orig_file;
	//graph.weights = g.weights;

	num_removed_nodes = num_removed_ndes;
	int V = g.nodes.size();



	vector<int> seeds = get_bc_approx(g, num_removed_nodes, epsilon, delta);

	node_removal_order = seeds;
	// make a removal list!
	vector <bool> removed_set;
	for (size_t i = 0; i < V; i++)
	{
		removed_set.push_back(false);
	}
	for (size_t i = 0; i < seeds.size(); i++)
	{
		removed_set[seeds[i]] = true;
		// create a new graph
		GraphOrig h(graph, removed_set);
		vector<int> cur_comps = h.connectedComponents();
		comps.push_back(cur_comps.size());
		int max = *max_element(cur_comps.begin(), cur_comps.end());
		largest_component.push_back(max);
	}
}
// Added from Networkit
// Constructor to do the Estimated
Immunization::Immunization(const GraphOrig &g, int num_removed_ndes, int samples, bool normalized, bool in_parallel)
{
	// Initialize everything
	graph.nodes = g.nodes;
	graph.original_mapping = g.original_mapping;
	graph.orig_file = g.orig_file;
	//graph.weights = g.weights;

	num_removed_nodes = num_removed_ndes;
	int V = g.nodes.size();



	vector<int> seeds = get_bc_estim(g, num_removed_nodes, samples, normalized, in_parallel);

	node_removal_order = seeds;
	// make a removal list!
	vector <bool> removed_set;
	for (size_t i = 0; i < V; i++)
	{
		removed_set.push_back(false);
	}
	for (size_t i = 0; i < seeds.size(); i++)
	{
		removed_set[seeds[i]] = true;
		// create a new graph
		GraphOrig h(graph, removed_set);
		vector<int> cur_comps = h.connectedComponents();
		comps.push_back(cur_comps.size());
		int max = *max_element(cur_comps.begin(), cur_comps.end());
		largest_component.push_back(max);
	}
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

Immunization::Immunization(const Graph2 &g, int num_removed_ndes, int M, double closeness)
{
	// Initialize everything
	gr.nodes = g.nodes;
	gr.original_mapping = g.original_mapping;
	gr.orig_file = g.orig_file;
	gr.weights = g.weights;

	num_removed_nodes = num_removed_ndes;
	int V = g.nodes.size();
	vector<double> btwss;
	vector<int> seeds;
	vector<vector<int>> new_nodes = g.nodes;
	// Use a method to get a list of nodes to be removed...
	//vector<int> seeds = Katz::adaptiveKatz2(graph, num_removed_ndes, iterations, closeness);
	//vector<int> seeds = betweennessCentrality::bcAdaptivek(graph, num_removed_ndes, 5); // the k is irrelevant - was used for adaptive k
	adaptive_betweenness_centrality::adaptive_approximate_betweenness(new_nodes, M, V, seeds, btwss);
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
		Graph2 h(gr, removed_set);
		vector<int> cur_comps = h.connectedComponents();
		if (cur_comps.size() > 0) {
			comps.push_back(cur_comps.size());
			int max = *max_element(cur_comps.begin(), cur_comps.end());
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
/*// added from Networkit.cpp -DB
vector<int> Immunization::get_bc_approx(const GraphOrig & g, int num_removed_nodes, double epsilon, double delta)
{

	//
	// convert scores to seeds


	//source_vertices.clear();
	//cout << "made it into get_gpu_bc!" << endl;
	NetworKit::Graph h(g); // -DB )
	GraphOrig ourGraph(g); 
	//graph h = parse_our_graph(g);
	std::vector<int> finished;
	for (size_t i = 0; i < g.nodes.size(); i++)
	{
		//std::vector<float> bc_g;
		// get betweenness centrality for all nodes
		NetworKit::ApproxBetweenness centrality(h, epsilon, delta);
		centrality.run();
		std::vector<double> bc_g = centrality.scores();
		//bc_g = bc_gpu(h, max_threads_per_block, number_of_SMs, op, source_vertices);
		// find the node with largest betweenness centrality
		float max_btws = 0;
		int max_loc = 0;
		for (size_t j = 0; j < bc_g.size(); j++)
		{
			if (bc_g[j] > max_btws)
			{
				max_btws = bc_g[j];
				max_loc = j;
			}
		}
		if (max_btws == 0) // we're done then!
		{
			break;
		}
		finished.push_back(max_loc);
		// now need to remove the max_loc vertex from the graph...
		// remove node from ourGraph
		// now we have to remove node maxCl_Location from the graph
		// to prepare to find the next max
		vector<int> adjs = ourGraph.nodes[max_loc];
		for (size_t j = 0; j < adjs.size(); j++)
		{
			int location_to_remove_from = adjs[j];
			// go to h.nodes[location_to_remove] and remove maxCl_Location
			auto it = find(ourGraph.nodes[location_to_remove_from].begin(), ourGraph.nodes[location_to_remove_from].end(), max_loc);
			if (it != ourGraph.nodes[location_to_remove_from].end())
			{
				ourGraph.nodes[location_to_remove_from].erase(it);
			}

		}
		ourGraph.nodes[max_loc].clear();
		cout << "Finished node " << i << endl;


		// reconvert
		//delete[] h.R;
		//delete[] h.C;
		//delete[] h.F;
		//h = parse_our_graph(ourGraph);
		NetworKit::Graph h(ourGraph); // -DB 
	}

	cout << "finished one line" << endl;
	return finished;
}
*/ 
vector<int> Immunization::get_bc_approx(const GraphOrig & g, int num_removed_nodes, int k, double delta, double err)
{

	//
	// convert scores to seeds


	//source_vertices.clear();
	//cout << "made it into get_gpu_bc!" << endl;

	GraphOrig ourGraph(g);
	//Graph h(g);
	//graph h = parse_our_graph(g);
	std::vector<int> finished;
	//for (size_t i = 0; i < g.nodes.size(); i++)
	for (size_t i = 0; i < num_removed_nodes; i++)
	{
		//std::vector<float> bc_g;
		// get betweenness centrality for all nodes
		//Probabilistic G(graph_file, directed, verb);
		Probabilistic G(ourGraph, true, 0);
		//Probabilistic(const GraphOrig & filename, const bool directed, const double verb)
		//ApproxBetweenness centrality(h, epsilon, delta);
		//centrality.run();
		//G.run((uint32_t)k, delta, err);
		G.run((uint32_t)k, delta, err);
		int max_loc = G.top_k->get(0);
		float max_btws = G.top_k->get_value(0);
		/*
		std::vector<double> bc_g;// = centrality.scores();
		//bc_g = bc_gpu(h, max_threads_per_block, number_of_SMs, op, source_vertices);
		// find the node with largest betweenness centrality
		float max_btws = 0;
		int max_loc = 0;
		for (size_t j = 0; j < bc_g.size(); j++)
		{
			if (bc_g[j] > max_btws)
			{
				max_btws = bc_g[j];
				max_loc = j;
			}
		}
		*/
		if (max_btws == 0) // we're done then!
		{
			break;
		}
		finished.push_back(max_loc);
		// now need to remove the max_loc vertex from the graph...
		// remove node from ourGraph
		// now we have to remove node maxCl_Location from the graph
		// to prepare to find the next max
		vector<int> adjs = ourGraph.nodes[max_loc];
		for (size_t j = 0; j < adjs.size(); j++)
		{
			int location_to_remove_from = adjs[j];
			// go to h.nodes[location_to_remove] and remove maxCl_Location
			auto it = std::find(ourGraph.nodes[location_to_remove_from].begin(), ourGraph.nodes[location_to_remove_from].end(), max_loc); //added std:: -DB
			if (it != ourGraph.nodes[location_to_remove_from].end())
			{
				ourGraph.nodes[location_to_remove_from].erase(it);
			}

		}
		ourGraph.nodes[max_loc].clear();
		cout << "Finished node " << i << endl;


		// reconvert
		//delete[] h.R;
		//delete[] h.C;
		//delete[] h.F;
		//h = parse_our_graph(ourGraph);

		//if (i<num_removed_nodes-1)
		//h = Graph(ourGraph);
	}

	cout << "finished one line" << endl;
	return finished;
}
/*
vector<int> Immunization::get_bc_estim(const GraphOrig & g, int num_removed_nodes, int samples, bool normalized, bool in_parallel)
{

	cout << "Doing estimated" << endl;
	//
	// convert scores to seeds


	//source_vertices.clear();
	//cout << "made it into get_gpu_bc!" << endl;

	GraphOrig ourGraph(g);
	NetworKit::Graph h(g); 
	//graph h = parse_our_graph(g);
	std::vector<int> finished;
	for (size_t i = 0; i < num_removed_nodes; i++)
	{
		//std::vector<float> bc_g;
		// get betweenness centrality for all nodes
		//NetworKit::ApproxBetweenness centrality(h, epsilon, delta);
		NetworKit::EstimateBetweenness ebc(h, samples, normalized, in_parallel);
		ebc.run();
		std::vector<double> bc_g = ebc.scores();
		//bc_g = bc_gpu(h, max_threads_per_block, number_of_SMs, op, source_vertices);
		// find the node with largest betweenness centrality
		float max_btws = 0;
		int max_loc = 0;
		for (size_t j = 0; j < bc_g.size(); j++)
		{
			if (bc_g[j] > max_btws)
			{
				max_btws = bc_g[j];
				max_loc = j;
			}
		}
		if (max_btws == 0) // we're done then!
		{
			break;
		}
		finished.push_back(max_loc);
		// now need to remove the max_loc vertex from the graph...
		// remove node from ourGraph
		// now we have to remove node maxCl_Location from the graph
		// to prepare to find the next max
		vector<int> adjs = ourGraph.nodes[max_loc];
		for (size_t j = 0; j < adjs.size(); j++)
		{
			int location_to_remove_from = adjs[j];
			// go to h.nodes[location_to_remove] and remove maxCl_Location
			auto it = find(ourGraph.nodes[location_to_remove_from].begin(), ourGraph.nodes[location_to_remove_from].end(), max_loc);
			if (it != ourGraph.nodes[location_to_remove_from].end())
			{
				ourGraph.nodes[location_to_remove_from].erase(it);
			}

		}
		ourGraph.nodes[max_loc].clear();
		cout << "Finished node " << i << endl;


		// reconvert
		//delete[] h.R;
		//delete[] h.C;
		//delete[] h.F;
		//h = parse_our_graph(ourGraph);
		NetworKit::Graph h(ourGraph); 
	}

	cout << "finished one line" << endl;
	return finished;
}
*/