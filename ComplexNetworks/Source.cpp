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
#include "Immunization.h"

using namespace std;

// functions to call for each case
void runGSIZE(int numClusters, int numSamples, bool reassign, int samp_vertex, string graph_file);
void runKADABRA(int numClusters, int numSamples, bool reassign, int samp_vertex, string graph_file);
void runABC(int M, double closeness, string graph_file);
void runNET();


int main(int argc, char*argv[]) {
	for (int i = 1; i < argc; i++)
	{
		
		if ((string)argv[i] == "ABC") {
			cout << "running " << argv[i] << " " << argv[i+1] << " " << argv[i+2] << " on file " << argv[i+3] <<  endl;
			//case ABC  (Adaptive-bewteenness-centrality)
			// ABC Variables   M				Closeness				filepath
			runABC(stoi(argv[i + 1]), stod(argv[i + 2]), argv[i + 3]);
			i = i + 3;
		}
		else if ((string)argv[i] == "GSIZE") {
			cout << "running " << argv[i] << " " << argv[i + 1] << " " << argv[i + 2] << " " << argv[i + 3] << " " << argv[i + 4] << " on file " << argv[i + 5] << endl;
			//  case GSIZE
			//  Gsize variables(numClusters,  numSamples i.e. M,       Reassign (1 or 0 for bool), samp_vertex,    and   filepath for graph)
			runGSIZE(stoi(argv[i+1]), stoi(argv[i+2]), stoi(argv[i+3]), stoi(argv[i+4]), argv[i+5]);
			i = i + 5;
		}
		else if ((string)argv[i] == "KADABRA") {
			cout << "running " << argv[i] << " " << argv[i + 1] << " " << argv[i + 2] << " " << argv[i + 3] << " " << argv[i + 4] << " on file " << argv[i + 5] << endl;
			//case KADABRA
			// KADABRA variables (numClusters,  err,				     Reassign (1 or 0 for bool), groups,     and      filepath for graph)
			runKADABRA(stoi(argv[i + 1]), stoi(argv[i + 2]), stoi(argv[i + 3]), stoi(argv[i + 4]), argv[i + 5]);
			i = i + 5;
		}
		else if ((string)argv[i] == "Networkit") {
			//case Networkit
			runNET();
		}
		else {
			cout << "Algorithm name not recognized at argv " << i << endl;
			return 0;
		}
	}
	/*string filepath;
	ifstream graph_file;*/
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
	//string line;
	//cout << "Algorithms and Parameters:" << endl;
	//cout << "ABC      numSamples  closeness  filepath" << endl;
	//cout << "GSIZE    numClusters numSamples bool_Reassign(1 or 0) samp_vertex filepath" << endl;
	//cout << "KADABRA  numClusters err        bool_reassign         groups      filepath" << endl;
	//cout << "Networkit  -  -  adding later" << endl;
	//cout << "Enter 'Help' to see algorithms again." << endl;
	//cout << "Enter an algorthim and parameters, or exit ..." << endl;
	//getline(cin, line);
	//while (line != "exit") {
	//	//parse input for paramaters
	//	istringstream iss(line);

	//	vector<string> split_input;
	//	do {
	//		string sp;
	//		iss >> sp;
	//		split_input.push_back(sp);
	//	} while (iss);

	//	//if statements for deciding which algorithm (ex. KADABRA # # #)
	//	//case depends on first string in line after parsing
	//	// should maybe add validation of input, but at this point accept everything and assume the user wants what they enter
	//	if (split_input.at(0) == "ABC") {
	//		//case ABC  (Adaptive-bewteenness-centrality)
	//		// ABC Variables   M				Closeness				filepath
	//		runABC(stoi(split_input.at(1)), stod(split_input.at(2)), split_input.at(3));
	//	}
	//	else if (split_input.at(0) == "GSIZE") {
	//		//  case GSIZE
	//		//  Gsize variables(numClusters,  numSamples i.e. M,       Reassign (1 or 0 for bool), samp_vertex,    and   filepath for graph)
	//		runGSIZE(stoi(split_input.at(1)), stoi(split_input.at(2)), stoi(split_input.at(3)), stoi(split_input.at(4)), split_input.at(5));

	//	}
	//	else if (split_input.at(0) == "KADABRA") {
	//		//case KADABRA
	//		// KADABRA variables (numClusters,  err,				     Reassign (1 or 0 for bool), groups,     and      filepath for graph)
	//		runKADABRA(stoi(split_input.at(1)), stoi(split_input.at(2)), stoi(split_input.at(3)), stoi(split_input.at(4)), split_input.at(5));
	//	}
	//	else if (split_input.at(0) == "Networkit") {
	//		//case Networkit
	//		runNET();
	//	}
	//	else if (split_input.at(0) == "Help" || split_input.at(0) == "help") {
	//		cout << "Algorithms and Parameters:" << endl;
	//		cout << "ABC      numSamples  closeness  filepath" << endl;
	//		cout << "GSIZE    numClusters numSamples bool_Reassign(1 or 0) samp_vertex filepath" << endl;
	//		cout << "KADABRA  numClusters err        bool_reassign         groups      filepath" << endl;
	//		cout << "Networkit  -  -  adding later" << endl;
	//	}
	//	else {
	//		cout << "Algorithm name not recognized... Do something here" << endl;
	//	}
	//	cout << split_input.at(0) << " algorithm complete. Check folder for output files." << endl;
	//	cout << endl;
	//	cout << "Enter the next algorithm and parameters, or exit.." << endl;
	//	getline(cin, line);
	//}

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

void runKADABRA(int numClusters, int numSamples, bool reassign, int samp_vertex, string graph_file){
	//output file
	ofstream output;
	output.open("KADARBRA_" + graph_file + "_" + to_string(numClusters) + ".txt");
	GraphOrig h(graph_file);

	auto start_time = std::chrono::steady_clock::now();
	Clust clust1(h, numClusters, numSamples, reassign, samp_vertex);
	clust1.GetPartition();
	clust1.SaveINTPartition("KADABRA_" + graph_file + "_" + to_string(numSamples) + "_INT.cluster");
	auto end_time = std::chrono::steady_clock::now();
	unsigned long total_time = std::chrono::duration_cast<std::chrono::microseconds>(
		end_time - start_time).count() / 1000; // add 3 zeros to get seconds we are getting thousandths of seconds with 1000
	output << total_time << endl;
	output << "milliseconds" << endl;
	output.close();
}

void runABC(int M, double closeness, string graph_file){
	string output_file = "ABC_" + graph_file + "_" + to_string(M) + ".csv";
	string runtime_file = "ABC_" + graph_file + "_" + to_string(M) + ".txt";
	ofstream output;
	output.open(runtime_file);
	Graph h(graph_file);
	auto start_time = std::chrono::steady_clock::now();
	Immunization myImm(h, h.nodes.size() - 1, M, closeness);

	myImm.OutputReport(output_file); // output files for each Imm report
	auto end_time = std::chrono::steady_clock::now();
	unsigned long total_time = std::chrono::duration_cast<std::chrono::microseconds>(
		end_time - start_time).count() / 1000; // add 3 zeros to get seconds we are getting thousandths of seconds with 1000

	output << total_time << endl;
	output << "milliseconds" << endl;
	output.close();
}

void runNET() {

}