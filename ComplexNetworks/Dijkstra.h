#pragma once

#include <algorithm>
#include <vector>
#include <queue>
#include <map>
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <cstdarg>
#include <random>
#include <stack>
#include <time.h>  
#include "Graph2.h"
#include "MinHeapDijkstra.h"
using namespace std;


/*struct IndexedItem
{
	int HeapIndex;
	double NodeWeight;
	int NodeIndex;

	IndexedItem(int nodeIndex, double nodeWeight)
	{
		NodeIndex = nodeIndex;
		HeapIndex = -1;
		NodeWeight = nodeWeight;
	}
};
*/
class Dijkstra {
public:
	vector<vector<int> > fromList;
	vector<int> numberOfShortestPaths;
	stack<int> S;
	vector<double> shortestPathLength;

	Dijkstra(Graph2 &g, int v);


};