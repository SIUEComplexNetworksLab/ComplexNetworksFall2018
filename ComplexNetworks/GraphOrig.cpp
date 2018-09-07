#pragma once
#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <random>
#include <fstream>
#include <sstream>
#include <algorithm> // added for sort -DB
#include "GraphOrig.h"
//#include "adaptive-betweenness-centrality.h"
using namespace std;
//using namespace adaptive_betweenness_centrality;

GraphOrig::GraphOrig()
{

}

GraphOrig::GraphOrig(const GraphOrig &g)
{
	nodes = g.nodes;
	orig_file = g.orig_file;
	original_mapping = g.original_mapping;
}

GraphOrig::GraphOrig(const std::vector<std::vector<int> > &some_nodes)
{
	nodes = some_nodes;
	for (size_t i = 0; i < nodes.size(); i++)
	{
		original_mapping.push_back(i);

	}
}

GraphOrig::GraphOrig(string filename)
{
	std::vector<std::vector<int> > G;
	load_graph_from_Graph(filename, G);
	nodes = G;
	orig_file = filename;
	for (size_t i = 0; i < nodes.size(); i++)
	{
		original_mapping.push_back(i);

	}
}

GraphOrig::GraphOrig(const GraphOrig &g, const vector<bool> &removed_set)
{
	orig_file = g.orig_file;
	original_mapping = g.original_mapping;
	int original_size = g.nodes.size();
	int new_size = 0;
	//for (size_t i = 0; i < original_size; i++)
	//{
	//	original_mapping.push_back(-1);
	//}

	int removed_size = 0;
	for (size_t i = 0; i < removed_set.size(); i++)
	{
		if (removed_set[i])
		{
			removed_size++;
		}
	}

	int num_nodes = g.nodes.size() - removed_size;

	int nodeID = 0;
	vector<int> oldIDToNewID(g.nodes.size());
	vector<int> oldLabel(num_nodes);
	//Now we need to itterate over each node in lwg
	for (int v = 0; v < g.nodes.size(); v++)
	{
		if (!removed_set[v])
		{
			oldIDToNewID[v] = nodeID;
			oldLabel[nodeID] = v;
			nodeID++;
		}
	}

	vector<vector<int>> edgesList(num_nodes);

	//now we should add our edges
	nodeID = 0;
	for (int v = 0; v < g.nodes.size(); v++)
	{
		if (!removed_set[v]) //if this is not a removed node we should add the edges
		{
			vector<int> edges = g.nodes[v];
			//Go through all of the edges and only add those not removed
			for (int u = 0; u < g.nodes[v].size(); u++)
			{
				int edgeTo = edges[u];
				if (!removed_set[edgeTo]) //this edge is still valid so we should add it
				{
					edgesList[nodeID].push_back(oldIDToNewID[edgeTo]);
				}
			}
			nodeID++;
		}
	}
	nodes = edgesList;
	//original_mapping = oldLabel;
	vector<int> new_orig_mapping;
	for (size_t i = 0; i < original_mapping.size(); i++)
	{
		if (!removed_set[i])
		{
			new_orig_mapping.push_back(original_mapping[i]);
		}
	}
	original_mapping = new_orig_mapping;
}

void GraphOrig::outputToGraph(string filename)
{
	ofstream myfile;
	myfile.open(filename);
	for (size_t i = 0; i < nodes.size(); i++)
	{
		myfile << i;
		for (size_t j = 0; j < nodes[i].size(); j++)
		{
			myfile << " " << nodes[i][j];
		}
		myfile << endl;
	}
}


void GraphOrig::load_graph_from_Graph(const std::string& fn, std::vector<std::vector<int> >& G) {
	bool weighted = false;
	std::ifstream ifs(fn.c_str());
	for (std::string line; getline(ifs, line); ) {
		if (line.size() > 0 && line[0] == 'w') { weighted = true; continue; }
		if (!weighted)
		{
			std::istringstream iss(line);
			int u = 0; int v = 0;
			iss >> u;
			if (u >= (int)G.size())  G.resize(u + 1);
			while (iss >> v)
			{
				G[u].push_back(v);
			}
		}
		if (weighted)
		{
			std::istringstream iss(line);
			int u = 0; int v = 0; double w = 0;
			iss >> u;
			if (u >= (int)G.size())  G.resize(u + 1);
			while (iss >> v)
			{
				iss >> w;
				G[u].push_back(v);
			}
		}
	}
	for (int u = 0; u < G.size(); u++) {
		//rep(u, G.size()) {
		sort(G[u].begin(), G[u].end());
		G[u].erase(unique(G[u].begin(), G[u].end()), G[u].end());
	}
}

vector<int> GraphOrig::connectedComponents()
{
	// clear the instance variable
	vector<int> comps; //comps.clear();
					   // Mark all the vertices as not visited
	int V = nodes.size();
	bool *visited = new bool[V];
	for (int v = 0; v < V; v++)
		visited[v] = false;
	for (int i = 0; i < removals.size(); i++) {
		visited[removals[i]] = true;
	}
	for (int v = 0; v<V; v++)
	{
		if (visited[v] == false)
		{
			comps.push_back(0);
			// print all reachable vertices
			// from v
			DFSUtil(v, visited, comps);

			//cout << "\n";
		}
	}

	delete[] visited;
	return comps;
}

void GraphOrig::DFSUtil(int v, bool visited[], vector<int> & comps)
{
	// Mark the current node as visited and print it
	visited[v] = true;
	//cout << v << " ";
	comps[comps.size() - 1]++;
	// Recur for all the vertices
	// adjacent to this vertex
	//vector<int>::iterator i;
	//for (std::vector<int>::size_type i = 0; i != g.nodes[v].size(); i++) {
	//	for (i = g.nodes[v].begin(); i != g.nodes[v].end(); ++i) {
	for (std::vector<int>::const_iterator i = nodes[v].begin(); i != nodes[v].end(); ++i) {
		if (!visited[*i])  // was *i
			DFSUtil(*i, visited, comps);
	}
}


