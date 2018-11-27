#pragma once
#include "Dijkstra.h"
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
#include "IndexedItem.h"
#include "Graph2.h"
#include "MinHeapDijkstra.h"
#include <climits>  // added for INT_MAX value	 -DB

using namespace std;

/*
struct IndexedItem
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
	Dijkstra::Dijkstra(Graph2 &g, int v) {

		int numNodes = g.nodes.size();
		//shortestPathLength = new double[numNodes];
		for (int i = 0; i < numNodes; i++) {
			shortestPathLength.push_back(INT_MAX);
		}
		//S = new Stack();
		for (int i = 0; i < numNodes; i++) {
			numberOfShortestPaths.push_back(0);
		}
		numberOfShortestPaths[v] = 1;
		//List of nodes (we will use this to 
		//fromList = new ArrayList[numNodes];

		for (int i = 0; i < numNodes; i++) {
			vector<int> row;
			fromList.push_back(row);
		}

		//now we need to setup our heap
		//set each node to infinite distance
		vector<IndexedItem> items;// = new IndexedItem[numNodes];
		for (int i = 0; i < numNodes; i++) {
			items.push_back(IndexedItem(i, INT_MAX));
		}
		items[v].NodeWeight = 0;
		MinHeapDijkstra minHeap(numNodes, items[v]);

		//dikstra main
		while (!minHeap.isEmpty()) {
			IndexedItem h = minHeap.extractMin();

			int uIndex = h.NodeIndex;
			S.push(uIndex);

			//check all edges
			//var u = g.Nodes[uIndex];
			vector<int> u = g.nodes[uIndex];
			int uEdgeCount = g.nodes[uIndex].size();
			vector<double> uWeights = g.weights[uIndex];
			for (int i = 0; i < uEdgeCount; i++) {
				double newWeight = h.NodeWeight + uWeights[i];
				int toIndex = u[i];
				IndexedItem &to = items[toIndex];
				double toNodeWeight = to.NodeWeight;
				if (newWeight < toNodeWeight) {
					to.NodeWeight = newWeight;
					//items[toIndex].NodeWeight = newWeight;
					shortestPathLength[toIndex] = newWeight;
					fromList[toIndex].clear();
					fromList[toIndex].push_back(uIndex);
					numberOfShortestPaths[toIndex] = numberOfShortestPaths[uIndex];
					if (to.HeapIndex == -1) //first encounter
					{
						minHeap.addItem(to);
					}
					else {
						minHeap.decreaseKey(to.HeapIndex);
					}
				}
				else if (newWeight == toNodeWeight) {
					fromList[toIndex].push_back(uIndex);//Add the node
					numberOfShortestPaths[toIndex] += numberOfShortestPaths[uIndex];
				}
			}
		}
	}


