#pragma once
#include "GraphOrig.h"

class Clust 
{
	public:
		GraphOrig graph;
		int num_clusters;
		int M;
		int samp;
		bool reassign;
		string metadata_vat;
		string metadata_int;
		string metadata_ten;
		vector<vector<int>> vat_clusters;
		vector<vector<int>> int_clusters;
		vector<vector<int>> ten_clusters;

		Clust(GraphOrig &h, int numClusters, int M, bool reassign, int samp_vertex);
		void GetPartition();
		vector<bool> create_exclusion_list(int cluster_number, vector<vector<int> > clusters);
		void SaveVATPartition(string filename);
		void SaveINTPartition(string filename);
		void SaveTENPartition(string filename);
		static bool compare_arrays(vector<int> & i, vector<int> & j);
		vector<vector<int>> connectedComponents2(const GraphOrig &g);
		void DFSUtil2(int v, bool visited[], const GraphOrig &g, vector<vector<int>> & comps);
		// removed Clust:: from functions -DB
};


