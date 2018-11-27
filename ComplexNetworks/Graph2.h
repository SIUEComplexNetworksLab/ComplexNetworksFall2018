#pragma once

#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <random>
#include <fstream>
#include "graph/GraphOrig.h"
//#include "adaptive-betweenness-centrality.h"

using namespace std;

class Graph2
{
public:
	vector<vector<int> > nodes;
	vector<vector<double> > weights;
	vector<int> original_mapping;
	string orig_file;
	vector<int> removals;

	Graph2();
	Graph2(const Graph2 &g);
	Graph2(const vector<vector<int> > &some_nodes);
	Graph2(string filename);
	Graph2(const Graph2 &g, const vector<bool> &removed_set); // removed_set = true if removed
	/*
	void Graph::outputToGraph(string filename);
	void Graph::removeNodes(vector<bool> & removedSet);
	vector<int> Graph::connectedComponents();
	void Graph::DFSUtil(int v, bool visited[], vector<int> & comps);
	double Graph::getDiameter();
	int Graph::BFS(int s);
	int Graph::BFSDiameter();
	  **Removing :: class identifiers in switch to UNIX		-DB 
	*/
	void outputToGraph(string filename);
	void removeNodes(vector<bool> & removedSet);
	vector<int> connectedComponents();
	void DFSUtil(int v, bool visited[], vector<int> & comps);
	double getDiameter();
	int BFS(int s);
	int BFSDiameter();
};
