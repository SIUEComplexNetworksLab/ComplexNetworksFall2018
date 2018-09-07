#pragma once
#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <random>
#include <fstream>
#include <functional>
#include <string>
#include<sstream>
#include <algorithm> // added for sort  -DB
#include "limits.h" // added for INT_MAX -DB
#include "GraphOrig.h"
//#include "adaptive-betweenness-centrality.h"
#include "Clust.h"
#include "NBR.h"
using namespace std;

Clust::Clust(GraphOrig &h, int numClusters, int myM, bool myReassign, int samp_vertex)
{
	graph = h;
	num_clusters = numClusters;
	M = myM;
	samp = samp_vertex;
	reassign = myReassign;
	metadata_vat = "";
	metadata_int = "";
	metadata_ten = "";
	vector<vector<int>> vat_clusters;
	vector<vector<int>> int_clusters;
	vector<vector<int>> ten_clusters;
}

void Clust::GetPartition()
{
	//ofstream results; results.open("C:\\Users\\John\\Dropbox\\Clust2\\big\\GPU3\\running.txt");
	//get the actual partition (if graph not necessarily connected)
	//vat_clusters = NBR::find_components(graph); 
	vat_clusters = connectedComponents2(graph);
	int_clusters = vat_clusters;
	ten_clusters = vat_clusters;
	metadata_vat += "Meta VATClust\n";
	metadata_int += "Meta IntClust\n";
	metadata_ten += "Meta TenClust\n";
	if (vat_clusters.size() >= num_clusters) 
	{
		metadata_vat += "Graph starts with enough clusters. No clustering done\n";
		metadata_int += "Graph starts with enough clusters. No clustering done\n";
		return;
	}
	//vector to hold the NBR items (each takes a long time to calculate and holds the key!) 
	vector<NBR> vatMap;
	vector<NBR> intMap;
	vector<NBR> tenMap;
	vector<int> vat_cut_set;
	vector<int> int_cut_set;
	vector<int> ten_cut_set;

	//create an nbr object for each component
	//the graph needs to have at least one component, so this should do something
	for (size_t i = 0; i < vat_clusters.size(); i++)
	{
		//create an exclusion list for this cluster
		vector<bool> exclude = create_exclusion_list(i, vat_clusters);
		//create a new graph from this cluster
		GraphOrig my_graph(graph, exclude);
		NBR myNBR(my_graph, M, samp);
		//cout << "Pushing back the vats." << endl;
		vatMap.push_back(myNBR);
		intMap.push_back(myNBR);
		tenMap.push_back(myNBR);
	}

	//at this point we have to work on vat and integrity one at a time
	//starting with vat
	/* 
	int current_clusters = vatMap.size();
	//cout << "current clusters: " << current_clusters << " num_clusters: " << num_clusters << endl;
	while (current_clusters < num_clusters)
	{
		// find the lowest vat cluster (each cluster lists a vat)
		int location; // location holds the location of the cluster we will be working on!
		double low_vat = INT_MAX;
		for (size_t i = 0; i < vatMap.size(); i++)
		{
			if (vatMap[i].vat < low_vat)
			{
				low_vat = vatMap[i].vat;
				location = i;
			}
		}

		// add "All calculated VATs:" to the metadata
		metadata_vat += "All calculated VATs:\n";
		//cout << "All calculated VATs:\n";
		for (size_t i = 0; i < vatMap.size(); i++)
		{
			metadata_vat += to_string(vatMap[i].vat) + " ";
			cout << to_string(vatMap[i].vat) + " ";
		}
		metadata_vat += "\n";
		cout << endl;
		// add "Vat: MinVat=" to the metadata
		metadata_vat += "Vat: MinVat=" + to_string(low_vat) + "\n";
		//cout << "Vat: MinVat=" + to_string(low_vat) + "\n";
		
			if (low_vat == INT_MAX) {
			
			cout << "low_vat not found.  This will never work." << endl;
			current_clusters = num_clusters;  // bail on this
			metadata_vat += "low_vat not found.  This will never work.\n";
			break;
		}
		if (low_vat == 1) { 
			cout << "low_vat = 1. This will never work!";
			current_clusters = num_clusters;  // bail on this
			metadata_vat += "low_vat = 1. This will never work!\n";
			break;
		}

		// add "Removed Count:1" to the metadata
		metadata_vat += "Removed Count:" + to_string(vatMap[location].num_removed_nodes_vat) + "\n";
		//cout << "Removed Count:" + to_string(vatMap[location].num_removed_nodes_vat) + "\n";
		// add list of removed nodes to the metadata
		for (size_t i = 0; i < vatMap[location].num_removed_nodes_vat-1; i++)
		{
			vat_cut_set.push_back(vatMap[location].grph.original_mapping[vatMap[location].node_removal_order[i]]);
			metadata_vat += to_string(vatMap[location].grph.original_mapping[vatMap[location].node_removal_order[i]]) + ",";
		}
		vat_cut_set.push_back(vatMap[location].grph.original_mapping[vatMap[location].node_removal_order[vatMap[location].num_removed_nodes_vat - 1]]);
		metadata_vat += to_string(vatMap[location].grph.original_mapping[vatMap[location].node_removal_order[vatMap[location].num_removed_nodes_vat - 1]]);
		metadata_vat += "\n";
		
		//SaveVATPartition("C:\\Users\\John\\Dropbox\\Clust2\\big\\gpu3\\network10VAT.cluster");
		
		// break up that cluster into subgraphs
		//make a new inclusion set
		vector<bool> exclude;
		for (size_t i = 0; i < vatMap[location].grph.nodes.size(); i++)
		{
			exclude.push_back(false);
		}
		for (size_t i = 0; i < vatMap[location].num_removed_nodes_vat; i++)
		{
			exclude[vatMap[location].node_removal_order[i]] = true;
		}
			//make a new graph
		GraphOrig myGraph(vatMap[location].grph, exclude);
			//find the clusters of the new graph
		//vector<vector<int>> new_clusters = NBR::find_components(myGraph);
		vector<vector<int>> new_clusters = connectedComponents2(myGraph);

		//cout << "made it to line 129" << endl;
		cout << metadata_vat << endl;

		// get the new count of clusters
		int new_count = new_clusters.size() + vat_clusters.size() - 1;
		if (new_count >= num_clusters)
		{
			// we have completed our clustering, 
			// create the clusters instance variable...the info is in myGraph
			// nicely, we do this without another call to abc, which is where all time cost is
			vector<vector<int>> new_cluster_list;
			for (size_t i = 0; i < vat_clusters.size(); i++)
			{
				if (i != location)
				{
					new_cluster_list.push_back(vat_clusters[i]);
				}
				else
				{
					// replace location with the data from new_clusters
					for (size_t j = 0; j < new_clusters.size(); j++)
					{
						vector<int> new_line;
						for (size_t m = 0; m < new_clusters[j].size(); m++)
						{
							new_line.push_back(myGraph.original_mapping[new_clusters[j][m]]);
						}
						new_cluster_list.push_back(new_line);
					}
				}
			}
			vat_clusters = new_cluster_list;
			current_clusters = new_count;
		}
		else
		{
			// if necessary, get the vats of the new subgraphs, create a list of clusters
			vector<vector<int>> orig_maps_main;
			for (size_t i = 0; i < new_clusters.size(); i++)
			{
				
				// copy the original mappings into an array:
				vector<int> orig_maps;
				for (size_t j = 0; j < new_clusters[i].size(); j++)
				{
					orig_maps.push_back(myGraph.original_mapping[new_clusters[i][j]]);
				}
				orig_maps_main.push_back(orig_maps);
			}
			vector<NBR> vatMap_addition;
			for (size_t i = 0; i < orig_maps_main.size(); i++)
			{
				//create an exclusion list for this clust
				vector<bool> exclude = create_exclusion_list(i, orig_maps_main);
				//create a new graph from this cluster
				GraphOrig my_graph(graph, exclude);
				NBR myNBR(my_graph, M);
				vatMap_addition.push_back(myNBR);
			}
			
			// make a new vatMap
			vector<NBR> new_vat_map;
			for (size_t i = 0; i < vatMap.size(); i++)
			{
				if (i != location)
				{
					new_vat_map.push_back(vatMap[i]);
				}
				else 
				{
					for (size_t j = 0; j < vatMap_addition.size(); j++)
					{
						new_vat_map.push_back(vatMap_addition[j]);
					}
				}
			}
			
			vatMap = new_vat_map;
			current_clusters = vatMap.size();

			// and we still need to make a new cluster list
			vector<vector<int>> new_cluster_list;
			for (size_t i = 0; i < vat_clusters.size(); i++)
			{
				if (i != location)
				{
					new_cluster_list.push_back(vat_clusters[i]);
				}
				else
				{
					// replace location with the data from new_clusters
					for (size_t j = 0; j < new_clusters.size(); j++)
					{
						vector<int> new_line;
						for (size_t m = 0; m < new_clusters[j].size(); m++)
						{
							new_line.push_back(myGraph.original_mapping[new_clusters[j][m]]);
						}
						new_cluster_list.push_back(new_line);
					}
				}
			}
			vat_clusters = new_cluster_list;

			//SaveVATPartition("C:\\Users\\John\\Dropbox\\Clust2\\big\\gpu3\\network10VAT.cluster");
		}

	}

	//if reassign, add the removed nodes back
	//in the previous program, nodes are assigned back immediately upon removal... which is better?
	if (reassign)
	{
		vector<int> memberships;
		for (size_t i = 0; i < graph.nodes.size(); i++)
		{
			memberships.push_back(-1);
		}
		for (size_t i = 0; i < vat_clusters.size(); i++)
		{
			for (size_t j = 0; j < vat_clusters[i].size(); j++)
			{
				memberships[vat_clusters[i][j]] = i;
			}
		}

		for (size_t i = 0; i < vat_cut_set.size(); i++)
		{
			vector<int> clust_memberships;
			for (size_t j = 0; j < vat_clusters.size(); j++)
			{
				clust_memberships.push_back(0);
			}
			vector<int> adjacencies = graph.nodes[vat_cut_set[i]];
			for (size_t q = 0; q < adjacencies.size(); q++)
			{
				if (memberships[adjacencies[q]] != -1)
				{
					clust_memberships[memberships[adjacencies[q]]]++;
				}
			}
			//find the largest
			int most_adjacencies = 0;
			int location_of_most_adjacencies = 0;
			for (size_t j = 0; j < clust_memberships.size(); j++)
			{
				if (clust_memberships[j] > most_adjacencies)
				{
					most_adjacencies = clust_memberships[j];
					location_of_most_adjacencies = j;
				}
			}
			// add the node to the cluster
			vat_clusters[location_of_most_adjacencies].push_back(vat_cut_set[i]);
		}
		
	}
	//SaveVATPartition("C:\\Users\\John\\Dropbox\\Clust2\\big\\gpu3\\network10VAT.cluster");
	// */
	//=================HERE STARTS INTEGRITY================================
	// ====================================================================
	//* 
	int current_clusters = intMap.size();
	//cout << "current clusters: " << current_clusters << " num_clusters: " << num_clusters << endl;
	while (current_clusters < num_clusters)
	{
		// find the lowest in cluster (each cluster lists a int)
		int location; // location holds the location of the cluster we will be working on!
		double low_int = INT_MAX;
		for (size_t i = 0; i < intMap.size(); i++)
		{
			if (intMap[i].integrity < low_int && intMap[i].grph.nodes.size() > 100)
			{
				low_int = intMap[i].integrity;
				location = i;
			}
		}

		// add "All calculated INTs:" to the metadata
		metadata_int += "All calculated INTs:\n";
		//cout << "All calculated INTs:\n";
		for (size_t i = 0; i < intMap.size(); i++)
		{
			metadata_int += to_string(intMap[i].integrity) + " ";
			cout << to_string(intMap[i].integrity) + " ";
		}
		metadata_int += "\n";
		cout << endl;
		// add "Int: MinInt=" to the metadata
		metadata_int += "Integrity: MinIntegrity=" + to_string(low_int) + "\n";
		//cout << "Integrity: MinInt=" + to_string(low_int) + "\n";

		if (low_int == INT_MAX) {

			cout << "low_int not found.  This will never work." << endl;
			current_clusters = num_clusters;  // bail on this
			metadata_int += "low_int not found.  This will never work.\n";
			break;
		}
		if (low_int == 1) {
			cout << "low_int = 1. This will never work!";
			current_clusters = num_clusters;  // bail on this
			metadata_int += "low_int = 1. This will never work!\n";
			break;
		}

		// add "Removed Count:1" to the metadata
		metadata_int += "Removed Count:" + to_string(intMap[location].num_removed_nodes_int) + "\n";
		//cout << "Removed Count:" + to_string(intMap[location].num_removed_nodes_int) + "\n";
		// add list of removed nodes to the metadata
		for (size_t i = 0; i < intMap[location].num_removed_nodes_int - 1; i++)
		{
			int_cut_set.push_back(intMap[location].grph.original_mapping[intMap[location].node_removal_order[i]]);
			metadata_int += to_string(intMap[location].grph.original_mapping[intMap[location].node_removal_order[i]]) + ",";
		}
		int_cut_set.push_back(intMap[location].grph.original_mapping[intMap[location].node_removal_order[intMap[location].num_removed_nodes_int - 1]]);
		metadata_int += to_string(intMap[location].grph.original_mapping[intMap[location].node_removal_order[intMap[location].num_removed_nodes_int - 1]]);
		metadata_int += "\n";

		//SaveINTPartition("C:\\Users\\John\\Dropbox\\Clust2\\big\\gpu3\\network10INT.cluster");

		// break up that cluster into subgraphs
		//make a new inclusion set
		vector<bool> exclude;
		for (size_t i = 0; i < intMap[location].grph.nodes.size(); i++)
		{
			exclude.push_back(false);
		}
		for (size_t i = 0; i < intMap[location].num_removed_nodes_int; i++)
		{
			exclude[intMap[location].node_removal_order[i]] = true;
		}
		//make a new graph
		GraphOrig myGraph(intMap[location].grph, exclude);
		//find the clusters of the new graph
		//vector<vector<int>> new_clusters = NBR::find_components(myGraph);
		vector<vector<int>> new_clusters = connectedComponents2(myGraph);

		//cout << "made it to line 129" << endl;
		cout << metadata_int << endl;

		// get the new count of clusters
		int new_count = new_clusters.size() + int_clusters.size() - 1;
		if (new_count >= num_clusters)
		{
			// we have completed our clustering, 
			// create the clusters instance variable...the info is in myGraph
			// nicely, we do this without another call to abc, which is where all time cost is
			vector<vector<int>> new_cluster_list;
			for (size_t i = 0; i < int_clusters.size(); i++)
			{
				if (i != location)
				{
					new_cluster_list.push_back(int_clusters[i]);
				}
				else
				{
					// replace location with the data from new_clusters
					for (size_t j = 0; j < new_clusters.size(); j++)
					{
						vector<int> new_line;
						for (size_t m = 0; m < new_clusters[j].size(); m++)
						{
							new_line.push_back(myGraph.original_mapping[new_clusters[j][m]]);
						}
						new_cluster_list.push_back(new_line);
					}
				}
			}
			int_clusters = new_cluster_list;
			current_clusters = new_count;
		}
		else
		{
			// if necessary, get the ints of the new subgraphs, create a list of clusters
			vector<vector<int>> orig_maps_main;
			for (size_t i = 0; i < new_clusters.size(); i++)
			{

				// copy the original mappings into an array:
				vector<int> orig_maps;
				for (size_t j = 0; j < new_clusters[i].size(); j++)
				{
					orig_maps.push_back(myGraph.original_mapping[new_clusters[i][j]]);
				}
				orig_maps_main.push_back(orig_maps);
			}
			vector<NBR> intMap_addition;
			for (size_t i = 0; i < orig_maps_main.size(); i++)
			{
				//create an exclusion list for this clust
				vector<bool> exclude = create_exclusion_list(i, orig_maps_main);
				//create a new graph from this cluster
				GraphOrig my_graph(graph, exclude);
				NBR myNBR(my_graph, M, samp);
				intMap_addition.push_back(myNBR);
			}

			// make a new intMap
			vector<NBR> new_int_map;
			for (size_t i = 0; i < intMap.size(); i++)
			{
				if (i != location)
				{
					new_int_map.push_back(intMap[i]);
				}
				else
				{
					for (size_t j = 0; j < intMap_addition.size(); j++)
					{
						new_int_map.push_back(intMap_addition[j]);
					}
				}
			}

			intMap = new_int_map;
			current_clusters = intMap.size();

			// and we still need to make a new cluster list
			vector<vector<int>> new_cluster_list;
			for (size_t i = 0; i < int_clusters.size(); i++)
			{
				if (i != location)
				{
					new_cluster_list.push_back(int_clusters[i]);
				}
				else
				{
					// replace location with the data from new_clusters
					for (size_t j = 0; j < new_clusters.size(); j++)
					{
						vector<int> new_line;
						for (size_t m = 0; m < new_clusters[j].size(); m++)
						{
							new_line.push_back(myGraph.original_mapping[new_clusters[j][m]]);
						}
						new_cluster_list.push_back(new_line);
					}
				}
			}
			int_clusters = new_cluster_list;

			//SaveINTPartition("C:\\Users\\John\\Dropbox\\Clust2\\big\\gpu3\\network10INT.cluster");

		}

	}
	
	//SaveINTPartition("C:\\Users\\John\\Dropbox\\Clust2\\big\\gpu3\\network10INT.cluster");
	
	//if reassign, add the removed nodes back
	//in the previous program, nodes are assigned back immediately upon removal... which is better?
	if (reassign)
	{
		vector<int> memberships;
		for (size_t i = 0; i < graph.nodes.size(); i++)
		{
			memberships.push_back(-1);
		}
		for (size_t i = 0; i < int_clusters.size(); i++)
		{
			for (size_t j = 0; j < int_clusters[i].size(); j++)
			{
				memberships[int_clusters[i][j]] = i;
			}
		}

		for (size_t i = 0; i < int_cut_set.size(); i++)
		{
			vector<int> clust_memberships;
			for (size_t j = 0; j < int_clusters.size(); j++)
			{
				clust_memberships.push_back(0);
			}
			vector<int> adjacencies = graph.nodes[int_cut_set[i]];
			for (size_t q = 0; q < adjacencies.size(); q++)
			{
				if (memberships[adjacencies[q]] != -1)
				{
					clust_memberships[memberships[adjacencies[q]]]++;
				}
			}
			//find the largest
			int most_adjacencies = 0;
			int location_of_most_adjacencies = 0;
			for (size_t j = 0; j < clust_memberships.size(); j++)
			{
				if (clust_memberships[j] > most_adjacencies)
				{
					most_adjacencies = clust_memberships[j];
					location_of_most_adjacencies = j;
				}
			}
			// add the node to the cluster
			int_clusters[location_of_most_adjacencies].push_back(int_cut_set[i]);
		}

	}
	// */

//THIS IS THE PREVIOUS INTEGRITY
/*

	current_clusters = intMap.size();
	while (current_clusters < num_clusters)
	{
		// find the lowest integrity cluster (each cluster lists a integrity)
		int location; // location holds the location of the cluster we will be working on!
		double low_int = INT_MAX;
		for (size_t i = 0; i < intMap.size(); i++)
		{
			if (intMap[i].integrity < low_int)
			{
				low_int = intMap[i].integrity;
				location = i;
			}
		}

		// add "All calculated Integritys:" to the metadata
		metadata_int += "All calculated Integritys:\n";
		for (size_t i = 0; i < intMap.size(); i++)
		{
			metadata_int += to_string(intMap[i].integrity) + " ";
		}
		metadata_int += "\n";
		// add "Integrity: MinIngegrity=" to the metadata
		metadata_int += "Integrity: MinIntegrity=" + to_string(low_int) + "\n";

		if (low_int == INT_MAX) {

			cout << "low_int not found.  This will never work." << endl;
			current_clusters = num_clusters;  // bail on this
			metadata_int += "low_int not found.  This will never work.\n";
			break;
		}
		if (low_int == 1) {
			cout << "low_int = 1. This will never work!";
			current_clusters = num_clusters;  // bail on this
			metadata_int += "low_int = 1. This will never work!\n";
			break;
		}

		// add "Removed Count:1" to the metadata
		metadata_int += "Removed Count:" + to_string(intMap[location].num_removed_nodes_int) + "\n";

		// add list of removed nodes to the metadata
		for (size_t i = 0; i < intMap[location].num_removed_nodes_int - 1; i++)
		{
			int_cut_set.push_back(intMap[location].grph.original_mapping[intMap[location].node_removal_order[i]]);
			metadata_int += to_string(intMap[location].grph.original_mapping[intMap[location].node_removal_order[i]]) + ",";
		}
		int_cut_set.push_back(intMap[location].grph.original_mapping[intMap[location].node_removal_order[intMap[location].num_removed_nodes_int - 1]]);
		metadata_int += to_string(intMap[location].grph.original_mapping[intMap[location].node_removal_order[intMap[location].num_removed_nodes_int - 1]]);
		metadata_int += "\n";

		// break up that cluster into subgraphs
		//make a new inclusion set
		vector<bool> exclude;
		for (size_t i = 0; i < intMap[location].grph.nodes.size(); i++)
		{
			exclude.push_back(false);
		}
		for (size_t i = 0; i < intMap[location].num_removed_nodes_int; i++)
		{
			exclude[intMap[location].node_removal_order[i]] = true;
		}
		//make a new graph
		GraphOrig myGraph(intMap[location].grph, exclude);
		//find the clusters of the new graph
		//vector<vector<int>> new_clusters = NBR::find_components(myGraph);
		vector<vector<int>> new_clusters = connectedComponents2(myGraph);

		cout << metadata_int;

		// get the new count of clusters
		int new_count = new_clusters.size() + int_clusters.size() - 1;
		if (new_count >= num_clusters)
		{
			// we have completed our clustering, 
			// create the clusters instance variable...the info is in myGraph
			// nicely, we do this without another call to abc, which is where all time cost is
			vector<vector<int>> new_cluster_list;
			for (size_t i = 0; i < int_clusters.size(); i++)
			{
				if (i != location)
				{
					new_cluster_list.push_back(int_clusters[i]);
				}
				else
				{
					// replace location with the data from new_clusters
					for (size_t j = 0; j < new_clusters.size(); j++)
					{
						vector<int> new_line;
						for (size_t m = 0; m < new_clusters[j].size(); m++)
						{
							new_line.push_back(myGraph.original_mapping[new_clusters[j][m]]);
						}
						new_cluster_list.push_back(new_line);
					}
				}
			}
			int_clusters = new_cluster_list;
			current_clusters = new_count;
		}
		else
		{
			// if necessary, get the integritys of the new subgraphs, create a list of clusters
			vector<vector<int>> orig_maps_main;
			for (size_t i = 0; i < new_clusters.size(); i++)
			{

				// copy the original mappings into an array:
				vector<int> orig_maps;
				for (size_t j = 0; j < new_clusters[i].size(); j++)
				{
					orig_maps.push_back(myGraph.original_mapping[new_clusters[i][j]]);
				}
				orig_maps_main.push_back(orig_maps);
			}
			vector<NBRbig> intMap_addition;
			for (size_t i = 0; i < orig_maps_main.size(); i++)
			{
				//create an exclusion list for this clust
				vector<bool> exclude = create_exclusion_list(i, orig_maps_main);
				//create a new graph from this cluster
				GraphOrig my_graph(graph, exclude);
				NBRbig myNBR(my_graph, M);
				intMap_addition.push_back(myNBR);
			}

			// make a new intMap
			vector<NBRbig> new_int_map;
			for (size_t i = 0; i < intMap.size(); i++)
			{
				if (i != location)
				{
					new_int_map.push_back(intMap[i]);
				}
				else
				{
					for (size_t j = 0; j < intMap_addition.size(); j++)
					{
						new_int_map.push_back(intMap_addition[j]);
					}
				}
			}

			intMap = new_int_map;
			current_clusters = intMap.size();

			// and we still need to make a new cluster list
			vector<vector<int>> new_cluster_list;
			for (size_t i = 0; i < int_clusters.size(); i++)
			{
				if (i != location)
				{
					new_cluster_list.push_back(int_clusters[i]);
				}
				else
				{
					// replace location with the data from new_clusters
					for (size_t j = 0; j < new_clusters.size(); j++)
					{
						vector<int> new_line;
						for (size_t m = 0; m < new_clusters[j].size(); m++)
						{
							new_line.push_back(myGraph.original_mapping[new_clusters[j][m]]);
						}
						new_cluster_list.push_back(new_line);
					}
				}
			}
			int_clusters = new_cluster_list;


		}

	}

	//if reassign, add the removed nodes back
	//in the previous program, nodes are assigned back immediately upon removal... which is better?
	if (reassign)
	{
		vector<int> memberships;
		for (size_t i = 0; i < graph.nodes.size(); i++)
		{
			memberships.push_back(-1);
		}
		for (size_t i = 0; i < int_clusters.size(); i++)
		{
			for (size_t j = 0; j < int_clusters[i].size(); j++)
			{
				memberships[int_clusters[i][j]] = i;
			}
		}

		for (size_t i = 0; i < int_cut_set.size(); i++)
		{
			vector<int> clust_memberships;
			for (size_t j = 0; j < int_clusters.size(); j++)
			{
				clust_memberships.push_back(0);
			}
			vector<int> adjacencies = graph.nodes[int_cut_set[i]];
			for (size_t q = 0; q < adjacencies.size(); q++)
			{
				if (memberships[adjacencies[q]] != -1)
				{
					clust_memberships[memberships[adjacencies[q]]]++;
				}
			}
			//find the largest
			int most_adjacencies = 0;
			int location_of_most_adjacencies = 0;
			for (size_t j = 0; j < clust_memberships.size(); j++)
			{
				if (clust_memberships[j] > most_adjacencies)
				{
					most_adjacencies = clust_memberships[j];
					location_of_most_adjacencies = j;
				}
			}
			// add the node to the cluster
			int_clusters[location_of_most_adjacencies].push_back(int_cut_set[i]);
		}

	}
	// */

	/*
	// HERE STARTS TENACITY
	current_clusters = tenMap.size();
	while (current_clusters < num_clusters)
	{
		// find the lowest tenacity cluster (each cluster lists a tenacity)
		int location; // location holds the location of the cluster we will be working on!
		double low_ten = INT_MAX;
		for (size_t i = 0; i < tenMap.size(); i++)
		{
			if (tenMap[i].tenacity < low_ten)
			{
				low_ten = tenMap[i].tenacity;
				location = i;
			}
		}

		// add "All calculated Tenacitys:" to the metadata
		metadata_ten += "All calculated Tenacitys:\n";
		for (size_t i = 0; i < tenMap.size(); i++)
		{
			metadata_ten += to_string(tenMap[i].tenacity) + " ";
		}
		metadata_ten += "\n";
		// add "Tenacity: MinTenacity=" to the metadata
		metadata_ten += "Tenacity: MinTenacity=" + to_string(low_ten) + "\n";

		
		if (low_ten == INT_MAX) {

			cout << "low_ten not found.  This will never work." << endl;
			current_clusters = num_clusters;  // bail on this
			metadata_ten += "low_ten not found.  This will never work.\n";
			break;
		}
		// these were used in vat and integrity, don't really work in tenacity
		//if (low_ten == 1) {
		//	cout << "low_ten = 1. This will never work!";
		//	current_clusters = num_clusters;  // bail on this
		//	metadata_ten += "low_ten = 1. This will never work!\n";
		//	break;
		//}

		// add "Removed Count:1" to the metadata
		metadata_ten += "Removed Count:" + to_string(tenMap[location].num_removed_nodes_ten) + "\n";

		// add list of removed nodes to the metadata
		for (size_t i = 0; i < tenMap[location].num_removed_nodes_ten - 1; i++)
		{
			ten_cut_set.push_back(tenMap[location].grph.original_mapping[tenMap[location].node_removal_order[i]]);
			metadata_ten += to_string(tenMap[location].grph.original_mapping[tenMap[location].node_removal_order[i]]) + ",";
		}
		ten_cut_set.push_back(tenMap[location].grph.original_mapping[tenMap[location].node_removal_order[tenMap[location].num_removed_nodes_ten - 1]]);
		metadata_ten += to_string(tenMap[location].grph.original_mapping[tenMap[location].node_removal_order[tenMap[location].num_removed_nodes_ten - 1]]);
		metadata_ten += "\n";

		// break up that cluster into subgraphs
		//make a new inclusion set
		vector<bool> exclude;
		for (size_t i = 0; i < tenMap[location].grph.nodes.size(); i++)
		{
			exclude.push_back(false);
		}
		for (size_t i = 0; i < tenMap[location].num_removed_nodes_ten; i++)
		{
			exclude[tenMap[location].node_removal_order[i]] = true;
		}
		//make a new graph
		GraphOrig myGraph(tenMap[location].grph, exclude);
		//find the clusters of the new graph
		//vector<vector<int>> new_clusters = NBR::find_components(myGraph);
		vector<vector<int>> new_clusters = connectedComponents2(myGraph);

		cout << metadata_ten;

		// get the new count of clusters
		int new_count = new_clusters.size() + ten_clusters.size() - 1;
		if (new_count >= num_clusters)
		{
			// we have completed our clustering, 
			// create the clusters instance variable...the info is in myGraph
			// nicely, we do this without another call to abc, which is where all time cost is
			vector<vector<int>> new_cluster_list;
			for (size_t i = 0; i < ten_clusters.size(); i++)
			{
				if (i != location)
				{
					new_cluster_list.push_back(ten_clusters[i]);
				}
				else
				{
					// replace location with the data from new_clusters
					for (size_t j = 0; j < new_clusters.size(); j++)
					{
						vector<int> new_line;
						for (size_t m = 0; m < new_clusters[j].size(); m++)
						{
							new_line.push_back(myGraph.original_mapping[new_clusters[j][m]]);
						}
						new_cluster_list.push_back(new_line);
					}
				}
			}
			ten_clusters = new_cluster_list;
			current_clusters = new_count;
		}
		else
		{
			// if necessary, get the tenacitys of the new subgraphs, create a list of clusters
			vector<vector<int>> orig_maps_main;
			for (size_t i = 0; i < new_clusters.size(); i++)
			{

				// copy the original mappings into an array:
				vector<int> orig_maps;
				for (size_t j = 0; j < new_clusters[i].size(); j++)
				{
					orig_maps.push_back(myGraph.original_mapping[new_clusters[i][j]]);
				}
				orig_maps_main.push_back(orig_maps);
			}
			vector<NBRbig> tenMap_addition;
			for (size_t i = 0; i < orig_maps_main.size(); i++)
			{
				//create an exclusion list for this clust
				vector<bool> exclude = create_exclusion_list(i, orig_maps_main);
				//create a new graph from this cluster
				GraphOrig my_graph(graph, exclude);
				NBRbig myNBR(my_graph, M);
				tenMap_addition.push_back(myNBR);
			}

			// make a new tenMap
			vector<NBRbig> new_ten_map;
			for (size_t i = 0; i < tenMap.size(); i++)
			{
				if (i != location)
				{
					new_ten_map.push_back(tenMap[i]);
				}
				else
				{
					for (size_t j = 0; j < tenMap_addition.size(); j++)
					{
						new_ten_map.push_back(tenMap_addition[j]);
					}
				}
			}

			tenMap = new_ten_map;
			current_clusters = tenMap.size();

			// and we still need to make a new cluster list
			vector<vector<int>> new_cluster_list;
			for (size_t i = 0; i < ten_clusters.size(); i++)
			{
				if (i != location)
				{
					new_cluster_list.push_back(ten_clusters[i]);
				}
				else
				{
					// replace location with the data from new_clusters
					for (size_t j = 0; j < new_clusters.size(); j++)
					{
						vector<int> new_line;
						for (size_t m = 0; m < new_clusters[j].size(); m++)
						{
							new_line.push_back(myGraph.original_mapping[new_clusters[j][m]]);
						}
						new_cluster_list.push_back(new_line);
					}
				}
			}
			ten_clusters = new_cluster_list;


		}

	}

	//if reassign, add the removed nodes back
	//in the previous program, nodes are assigned back immediately upon removal... which is better?
	if (reassign)
	{
		vector<int> memberships;
		for (size_t i = 0; i < graph.nodes.size(); i++)
		{
			memberships.push_back(-1);
		}
		for (size_t i = 0; i < ten_clusters.size(); i++)
		{
			for (size_t j = 0; j < ten_clusters[i].size(); j++)
			{
				memberships[ten_clusters[i][j]] = i;
			}
		}

		for (size_t i = 0; i < ten_cut_set.size(); i++)
		{
			vector<int> clust_memberships;
			for (size_t j = 0; j < ten_clusters.size(); j++)
			{
				clust_memberships.push_back(0);
			}
			vector<int> adjacencies = graph.nodes[ten_cut_set[i]];
			for (size_t q = 0; q < adjacencies.size(); q++)
			{
				if (memberships[adjacencies[q]] != -1)
				{
					clust_memberships[memberships[adjacencies[q]]]++;
				}
			}
			//find the largest
			int most_adjacencies = 0;
			int location_of_most_adjacencies = 0;
			for (size_t j = 0; j < clust_memberships.size(); j++)
			{
				if (clust_memberships[j] > most_adjacencies)
				{
					most_adjacencies = clust_memberships[j];
					location_of_most_adjacencies = j;
				}
			}
			// add the node to the cluster
			ten_clusters[location_of_most_adjacencies].push_back(ten_cut_set[i]);
		}

	}
	// */


}

vector<bool> Clust::create_exclusion_list(int cluster_number, vector<vector<int>> clusters)
{
	vector<bool> removal_list;

	for (size_t i = 0; i < graph.nodes.size(); i++)
	{
		removal_list.push_back(true);
	}
	for (size_t i = 0; i < clusters[cluster_number].size(); i++)
	{
		removal_list[clusters[cluster_number][i]] = false;
	}
	return removal_list;
}

void Clust::SaveVATPartition(string filename)
{
	ofstream myfile;
	myfile.open(filename);
	
	myfile << "Graph " << graph.orig_file << endl;
	myfile << "Clusters " << vat_clusters.size() << endl;

	// we should sort the clusters for convenience:
	for (size_t i = 0; i < vat_clusters.size(); i++)
	{
		sort(vat_clusters[i].begin(), vat_clusters[i].end());
	}
	sort(vat_clusters.begin(), vat_clusters.end(), compare_arrays);

	// now print the clusters
	for (size_t i = 0; i < vat_clusters.size(); i++)
	{
		myfile << vat_clusters[i].size() << endl;
		for (size_t j = 0; j < vat_clusters[i].size(); j++)
		{
			myfile << vat_clusters[i][j] << " ";
		}
		myfile << endl;
	}

	myfile << metadata_vat;
	myfile.close();
}

void Clust::SaveINTPartition(string filename)
{
	ofstream myfile;
	myfile.open(filename);

	myfile << "Graph " << graph.orig_file << endl;
	myfile << "Clusters " << int_clusters.size() << endl;

	// we should sort the clusters for convenience:
	for (size_t i = 0; i < int_clusters.size(); i++)
	{
		sort(int_clusters[i].begin(), int_clusters[i].end());
	}
	sort(int_clusters.begin(), int_clusters.end(), compare_arrays);

	// now print the clusters
	for (size_t i = 0; i < int_clusters.size(); i++)
	{
		myfile << int_clusters[i].size() << endl;
		for (size_t j = 0; j < int_clusters[i].size(); j++)
		{
			myfile << int_clusters[i][j] << " ";
		}
		myfile << endl;
	}

	myfile << metadata_int;
	myfile.close();
}


void Clust::SaveTENPartition(string filename)
{
	ofstream myfile;
	myfile.open(filename);

	myfile << "Graph " << graph.orig_file << endl;
	myfile << "Clusters " << ten_clusters.size() << endl;

	// we should sort the clusters for convenience:
	for (size_t i = 0; i < ten_clusters.size(); i++)
	{
		sort(ten_clusters[i].begin(), ten_clusters[i].end());
	}
	sort(ten_clusters.begin(), ten_clusters.end(), compare_arrays);

	// now print the clusters
	for (size_t i = 0; i < ten_clusters.size(); i++)
	{
		myfile << ten_clusters[i].size() << endl;
		for (size_t j = 0; j < ten_clusters[i].size(); j++)
		{
			myfile << ten_clusters[i][j] << " ";
		}
		myfile << endl;
	}

	myfile << metadata_ten;
	myfile.close();
}

bool Clust::compare_arrays(vector<int> & i, vector<int> & j)
{
	return (i[0] < j[0]);
}

//taken from geeks for geeks
vector<vector<int>> Clust::connectedComponents2(const GraphOrig &g)
{
	// clear the instance variable
	vector<vector<int>>comps;
	// Mark all the vertices as not visited
	int V = g.nodes.size();
	bool *visited = new bool[V];
	for (int v = 0; v < V; v++)
		visited[v] = false;

	for (int v = 0; v<V; v++)
	{
		if (visited[v] == false)
		{
			vector<int> current_comps;
			comps.push_back(current_comps);
			// print all reachable vertices
			// from v
			DFSUtil2(v, visited, g, comps);

			//cout << "\n";
		}
	}

	delete[] visited;
	return comps;
}

void Clust::DFSUtil2(int v, bool visited[], const GraphOrig &g, vector<vector<int>> & comps)
{
	// Mark the current node as visited and print it
	visited[v] = true;
	//cout << v << " ";
	comps[comps.size() - 1].push_back(v);
	// Recur for all the vertices
	// adjacent to this vertex
	//vector<int>::iterator i;
	//for (std::vector<int>::size_type i = 0; i != g.nodes[v].size(); i++) {
	//	for (i = g.nodes[v].begin(); i != g.nodes[v].end(); ++i) {
	for (std::vector<int>::const_iterator i = g.nodes[v].begin(); i != g.nodes[v].end(); ++i) {
		if (!visited[*i])  // was *i
			DFSUtil2(*i, visited, g, comps);
	}
}
