#pragma once

#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <random>
#include <fstream>
#include "Graph.h"
#include "adaptive-betweenness-centrality.h"
#include "Dijkstra.h"
#include <climits>  // added for INT_MAX value	 -DB
#include <list> // added for list class use      -DB
using namespace std;
//using namespace adaptive_betweenness_centrality;

Graph::Graph()
{

}

Graph::Graph(const Graph &g)
{
	nodes = g.nodes;
	weights = g.weights;
	orig_file = g.orig_file;
	original_mapping = g.original_mapping;
}

Graph::Graph(const std::vector<std::vector<int> > &some_nodes)
{
	nodes = some_nodes;
	for (size_t i = 0; i < nodes.size(); i++)
	{
		original_mapping.push_back(i);

	}
}

Graph::Graph(string filename)
{
	std::vector<std::vector<int> > G;
	std::vector<std::vector<double> > W;
	adaptive_betweenness_centrality::load_graph_from_Graph(filename, G);
	nodes = G;
	orig_file = filename;
	for (size_t i = 0; i < nodes.size(); i++)
	{
		original_mapping.push_back(i);

	}
	for (size_t i = 0; i < nodes.size(); i++)
	{
		vector<double> nv;
		W.push_back(nv);
		for (size_t j = 0; j < nodes[i].size(); j++)
		{
			W[i].push_back(1);

		}
	}
	weights = W;

}

Graph::Graph(const Graph &g, const vector<bool> &removed_set)
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
	vector<vector<double> > weightsList(num_nodes);

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
					weightsList[nodeID].push_back(1);

				}
			}
			nodeID++;
		}
	}
	nodes = edgesList;
	weights = weightsList;
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

void Graph::outputToGraph(string filename)
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

 void Graph::removeNodes(vector<bool> &removed_set) {
	// removed_set = true if removed
	for (int i = 0; i < removed_set.size(); i++) {
		if (removed_set[i]) {
			removals.push_back(i);
			// remove vertex i
			for (int j = 0; j < nodes.size(); j++) {
				vector<int> currentList(nodes[j]);
				vector<double> currentWeights(weights[j]);
				nodes[j].clear();
				weights[j].clear();
				for (int k = 0; k < currentList.size(); k++) {
					if (currentList[k] != i) {
						nodes[j].push_back(currentList[k]);
						weights[j].push_back(currentWeights[k]);
					}
				}

			}
			nodes[i].clear();

		}
	}
}

 vector<int> Graph::connectedComponents()
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

 void Graph::DFSUtil(int v, bool visited[], vector<int> & comps)
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

 double Graph::getDiameter()
	 
	 {
		 double longestDistance = 0;
		 for (int i = 0; i < nodes.size(); i++) {
			 Dijkstra asp(*this, i);
			 double max = 0;
			 for (int j = 0; j < asp.shortestPathLength.size(); j++) {
				 if (asp.shortestPathLength[j] != INT_MAX && asp.shortestPathLength[j] > max) max = asp.shortestPathLength[j];
			 }
			 if (max > longestDistance) longestDistance = max;
		 }
		 return longestDistance;
		
	 return 0.0;
 }

 int Graph::BFS(int s)
 {
	 // Mark all the vertices as not visited
	 int V = nodes.size();
	 bool *visited = new bool[V];
	 
	 vector<int> dist(V);
	 for (size_t i = 0; i < V; i++) 
		 dist[i] = INT_MAX;
	 
	 for (int i = 0; i < V; i++)
		 visited[i] = false;

	 // Create a queue for BFS
	 list<int> queue;

	 // Mark the current node as visited and enqueue it
	 visited[s] = true;
	 dist[s] = 0;
	 queue.push_back(s);

	 // 'i' will be used to get all adjacent
	 // vertices of a vertex
	 vector<int>::iterator i;

	 while (!queue.empty())
	 {
		 // Dequeue a vertex from queue and print it
		 s = queue.front();
		 //cout << s << " ";
		 queue.pop_front();

		 // Get all adjacent vertices of the dequeued
		 // vertex s. If a adjacent has not been visited, 
		 // then mark it visited and enqueue it
		 for (i = nodes[s].begin(); i != nodes[s].end(); ++i)
		 {
			 if (!visited[*i])
			 {
				 visited[*i] = true;
				 dist[*i] = dist[s] + 1;
				 queue.push_back(*i);
			 }
		 }
	 }
	 //double max = *max_element(std::begin(dist), std::end(dist));
	 int max = 0;
	 //int max_loc = 0;
	 for (size_t i = 0; i < V; i++)
	 {
		 if (dist[i] > max && dist[i]!= INT_MAX)
		 {
			 max = dist[i];
			 //max_loc = i;
		 }
	 }
	 return max;
 }
 int Graph::BFSDiameter()
 {
	 int V = nodes.size();
	 int max_distance=0;
	 for (size_t i = 0; i < V; i++)
	 {
		 if (i%1000==0)cout << "Inner Iter " << i << endl;
		 int cur_dist = BFS(i);
		 if (cur_dist > max_distance && cur_dist != INT_MAX) max_distance = cur_dist;
	 }
	 return max_distance;
 }
