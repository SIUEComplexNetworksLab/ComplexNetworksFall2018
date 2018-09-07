/* Source File to run multiple algorithms
	CS 495 - John Matta
	Student - Dalton Brooks
	Version 1 - 9/7/2018
*/
#include <string>
#include <iostream>
#include <fstream>
#include "Graph.h"
#include "GraphOrig.h"
#include "Clust.h"
#include <vector>
#include <sstream>
#include <chrono>
#include <ctime>

using namespace std;

// functions to call for each case
void runGSIZE(int numClusters, int numSamples, bool reassign, int samp_vertex, string graph_file);
void runKADABRA(int numClusters, int numSamples, bool reassign, int samp_vertex, string graph_file);
void runABC();
void runNET();


int main() {
	string filepath;
	ifstream graph_file;
	////ask user for file path
	//cout << "Enter a file path of a graph ..." << endl;
	//cin >> filepath;
	//
	//// ASSURE FILE EXISTS
	//graph_file.open(filepath);
	//while (!graph_file.is_open()) {
	//	cout << "File not found. Please enter a different file path ..." << endl;
	//	cin >> filepath;
	//	graph_file.open(filepath);
	//}
	//graph_file.close();


	//get input from user (or script?) on which algorithm to run
	string line;
	cout << "Algorithms and Parameters:" << endl;
	cout << "ABC numSamples closeness filepath" << endl;
	cout << "GSIZE numClusters numSamples bool_Reassign(1 or 0) samp_vertex filepath" << endl;
	cout << "KADABRA numClusters err bool_reassign filepath groups filepath" << endl;
	cout << "Networkit -- adding later" << endl;
	cout << "Enter an algorthim and parameters ..." << endl;
	getline(cin, line);

	//parse input for paramaters
	istringstream iss(line);
	
	vector<string> split_input;
	do {
		string sp;
		iss >> sp;
		split_input.push_back(sp);
	} while (iss);

	//if statements for deciding which algorithm (ex. KADABRA # # #)
	//case depends on first string in line after parsing
	if (split_input.at(0) == "ABC") {
	
	}
	else if (split_input.at(0) == "GSIZE") {
		//  case GSIZE
		//  Gsize have variables (numClusters, numSamples i.e. M, Reassign (1 or 0 for bool), samp_vertex, and filepath for graph)
		runGSIZE(stoi(split_input.at(1)), stoi(split_input.at(2)), stoi(split_input.at(3)), stoi(split_input.at(4)), split_input.at(5));
	

	}
	else if (split_input.at(0) == "KADABRA") {
	
	}
	else if (split_input.at(0) == "Networkit") {

	}
	else {
		cout << "Algorithm name not recognized... Do something here" << endl;
	}
	


	//case GSIZE

	//case KADABRA

	//case ABC  (Adaptive-bewteenness-centrality)

	//case Networkit



	return 0;
}


void runGSIZE(int numClusters, int numSamples, bool reassign, int samp_vertex, string graph_file) {
	//output file
	ofstream output;
	output.open("GSIZE_" + graph_file + "_" + to_string(numClusters) + ".txt");
	GraphOrig h(graph_file);
	
	auto start_time = std::chrono::steady_clock::now();
	Clust clust1(h, numClusters, numSamples, reassign, samp_vertex);
	clust1.GetPartition();
	clust1.SaveINTPartition("GSIZE_" + graph_file + "_" + to_string(numSamples) + "_INT.cluster");
	auto end_time = std::chrono::steady_clock::now();
	unsigned long total_time = std::chrono::duration_cast<std::chrono::microseconds>(
		end_time - start_time).count() / 1000; // add 3 zeros to get seconds we are getting thousandths of seconds with 1000
	output << total_time << endl;
	output << "milliseconds" << endl;
	output.close();
}

void runKADABRA(){

}

void runABC(){

}

void runNET() {

}