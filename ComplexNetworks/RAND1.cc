#include <iostream>
#include <stdio.h>
#include <fstream>
#include <list>
#include <algorithm>
#include <cmath>
#include "graph_t.h"
#include "bc.h"
#include "utility.h"
#include <ctime>
#include "GraphOrig.h"
#include <chrono>
#include "Clust.h"
#include "Immunization.h"

using namespace std;

void run_RAND1(
	GraphOrig g,
	//string  graph_path,
        char  output_path[],
        int     sample_size
        )
{
	sgraph_t comp;
	//prepare_sgraph(graph_path,comp) ;
	comp.nodes_vec = g.nodes;

	vector<double> RAND1_between_centrality_result ;

	printf("===================================================================================") ;
	printf("\nComputation is Running for RAND1 and sample size = %d\n",sample_size) ;

	//timer tm ;
	//tm.start();
	RAND1(comp, RAND1_between_centrality_result,sample_size);
	//tm.stop();

	//std::string graph_name = extract_graph_name(graph_path) ;
	printf("\n@STAT	Alg#	RAND1	GRAPH#	%s	TIME#	%f	SAMPLES#	%d\n", "myGraph", 6, sample_size) ;

	int graph_size = comp.size() ;
	for(int i = 0 ; i<RAND1_between_centrality_result.size() ; i++){
		RAND1_between_centrality_result[i] = (graph_size/sample_size)*RAND1_between_centrality_result[i];
	}

	FILE *RAND1Output ;
	RAND1Output = fopen(output_path,"w");
	for(int i = 0 ; i<RAND1_between_centrality_result.size() ; i++){
		fprintf(RAND1Output,"%d:	     %f\n",i,RAND1_between_centrality_result[i]);
	}
	printf("\nResults computed and printed in (%s)\n",output_path);

}

/*
int main(  )
{
	int i;
	int size;
	char str_message[100];

	//if(argc != 4){
	//printf("\nWrong Arguments..\n");
	//printf("Program will terminate\n");
	//exit(1) ;
	//}
	/*
	i = 1;
	int sampleSize = 200; 		//ZIYAD_COMMENT: Take the sample size
	string graph_path =  "C:\\Users\\John\\Dropbox\\Clust2\\results\\brandes\\karate.mtx" ; 			//ZIYAD_COMMENT: Take the graph path
	char* result_path = "C:\\Users\\John\\Dropbox\\Clust2\\results\\brandes\\karate.txt";			//ZIYAD_COMMENT: Take the output path
	//GraphOrig h("C:\\Users\\John\\Dropbox\\Clust2\\big\\karate.graph");
	GraphOrig h("C:\\Users\\John\\Dropbox\\Clust2\\DATA\\10000\\network" + to_string(i) + ".graph");
	run_RAND1(h, result_path, sampleSize);
	*/

	/*
	// THis does the clustering problem
	int chosen_vertex = 0; // atoi(argv[1]);
	double c_constant = 5; // atof(argv[2]);
	int C[] = {10,25,50, 100, 200};
	int numClustersArr[] = { 0,36,38,40,37,42,39 };  // 100,000
	//int numClustersArr[] = { 0,38,39,40,39,40,38 }; //10,000
	bool reassign = true;
	ofstream myfile;
	for (size_t j = 0; j < 1; j++)
	{
		for (int i = 0; i < 7; i++)
		{

			myfile.open("C:\\Users\\John\\Dropbox\\Clust2\\results\\Brandes\\100000\\TIME" + to_string(i) + "." + to_string(C[j]) + ".txt");
			GraphOrig h("C:\\Users\\John\\Dropbox\\Clust2\\DATA\\100000\\network" + to_string(i) + ".graph");
			//GraphOrig h("C:\\Users\\John\\Dropbox\\Clust2\\big\\karate.graph");

			//char * outfile = "C:\\Users\\John\\Dropbox\\Clust2\\results\\Bader\\10000\\karate.txt";
			//run_GSIZE(h, outfile, 0, 5);

			
			int numClusters = numClustersArr[i];
			auto start_time = std::chrono::steady_clock::now();
			Clust clust1(h, numClusters, C[j], reassign);
			clust1.GetPartition();
			clust1.SaveINTPartition("C:\\Users\\John\\Dropbox\\Clust2\\results\\Brandes\\100000\\network" + to_string(i) + "." + to_string(C[j]) + "INT.cluster");
			auto end_time = std::chrono::steady_clock::now();
			unsigned long total_time = std::chrono::duration_cast<std::chrono::microseconds>(
				end_time - start_time).count() / 1000; // add 3 zeros to get seconds we are getting thousandths of seconds with 1000
			myfile << total_time << endl;
			myfile << "milliseconds" << endl;
			myfile.close();
			
		}
	}

	


	// this does the immunization problem

	int M[] = { 250}; // for smaller 50,100,250
										 //int M[] = { 20000, 160000, 640000 };
	ofstream myfile;
	for (int i = 0; i < 2; i++)
	{
		myfile.open("C:\\Users\\John\\Dropbox\\Clust2\\results\\Brandes\\TIMEEnron." + to_string(M[i]) + ".txt");
		GraphOrig h("C:\\Users\\John\\Dropbox\\Clust2\\DATA\\email-Enron.graph");
		//Graph h("C:\\Users\\John\\Dropbox\\Clust2\\SF1\\network1.graph");
		//myfile.open("C:\\Users\\John\\Dropbox\\Clust2\\SF1\\times.txt"); 
		auto start_time = std::chrono::steady_clock::now();
		//Immunization myImm(const Graph &g, int removed_nodes, int M, double closeness);
		Immunization myImm(h, h.nodes.size() - 1, M[i], .001);
		myImm.OutputReport("C:\\Users\\John\\Dropbox\\Clust2\\results\\Brandes\\email-Enron" + to_string(M[i]) + ".csv");
		auto end_time = std::chrono::steady_clock::now();
		unsigned long total_time = std::chrono::duration_cast<std::chrono::microseconds>(
			end_time - start_time).count() / 1000; // add 3 zeros to get seconds we are getting thousandths of seconds with 1000
		myfile << total_time << endl;
		myfile << "milliseconds" << endl;
		myfile.close();

	}


	//system("pause");  no such command for Linux shell -DB

	return 0;
}
*/