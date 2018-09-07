#pragma once
#include <iostream>
#include <vector>
#include <cstdlib>
//#include <queue>
#include <cmath>
#include <random>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include "GraphOrig.h"
//#include "adaptive-betweenness-centrality.h"
#include "NBR.h"
#include "bc.h"
//#include "kernels.cuh"
//#include<boost/container/flat_set.hpp>
//using boost::unordered_set;
using namespace std;
//using namespace adaptive_betweenness_centrality;

NBR::NBR()
{

}
NBR::NBR(const GraphOrig &g, int M,int samp)
{

	grph.nodes = g.nodes;
	grph.original_mapping = g.original_mapping;
	grph.orig_file = g.orig_file;
	vector<double> btwss;
	vector<int> seeds;
	vector<vector<int>> new_nodes = g.nodes;

	// this returns the list in order of node removals
	cout << "Starting an abc" << endl;
	//int myK = g.nodes.size() / 2;
	int myK = 3 * g.nodes.size() / 4;
	//int myK = g.nodes.size()/2;
	// sometimes a single node cluster gets in there...so if it does don't bother with the abc
	if (g.nodes.size() > 2)
	{
		//adaptive_approximate_betweenness(new_nodes, g.nodes.size()*7, myK, seeds, btwss);// g.nodes.size() was M
		//adjust_betweenness(seeds);
		//adaptive_approximate_coverage(new_nodes, g.nodes.size()*2, myK, seeds, btwss);// g.nodes.size() was M
		//get_influence(myK, seeds, btwss);
		//get_iterative_btwss(M, myK, seeds, btwss);
		//seeds = get_gpu_bc_exact(g, 128, 20, op, source_vert);
		seeds = get_bader_approx(g, myK, M, samp);
		cout << "Returned from abc";
		grph = g;
		node_removal_order = seeds;

		// calculate VAT for list of node removals
		vector<double> vats;
		vector<double> integritys;
		double thevat;
		double theintegrity;
		for (size_t i = 0; i < seeds.size(); i++)
		{
			if (i % 1000 == 0) cout << "Calculating measure for: " << i << endl;
			calculate_measure(seeds, i, thevat, theintegrity);
			vats.push_back(thevat);
			integritys.push_back(theintegrity);
		}

		// go through vats and find the smallest
		int min_location_vat = 0;
		double min_vat = INT_MAX;
		int min_location_int = 0;
		double min_int = INT_MAX;
		for (size_t i = 0; i < vats.size(); i++)
		{
			if (vats[i] < min_vat)
			{
				min_vat = vats[i];
				min_location_vat = i;
			}
			if (integritys[i] < min_int)
			{
				min_int = integritys[i];
				min_location_int = i;
			}
		}

		//cout << "Smallest VATS found" << endl;
		num_removed_nodes_vat = min_location_vat + 1;
		num_removed_nodes_int = min_location_int + 1;
		vat = min_vat;
		integrity = min_int;


	}
}

void NBR::calculate_measure(vector<int> &removal_order, int num_removed, double &thevat, double &theintegrity)
{
	// create a removal list
	vector<bool> removal_list;
	for (size_t i = 0; i < grph.nodes.size(); i++)
	{
		removal_list.push_back(false);
	}
	for (size_t i = 0; i <= num_removed; i++)
	{
		removal_list[removal_order[i]] = true;
	}
	// create a new graph
	GraphOrig g(grph, removal_list);


	//vector<vector<int>> components = find_components(g);
	connectedComponents(g);
	if (comps.size() == 1 || comps.size() == 0)
	//if (components.size() == 1 || components.size() == 0)
	{
		thevat = INT_MAX;
		theintegrity = INT_MAX;
	}
		
		
	// find cmax
	int cmax = 0;
	for (size_t w = 0; w < comps.size(); w++)
	{
		//if (components[w].size() > cmax)
		if (comps[w] > cmax)
		{
			cmax = comps[w]; 
		}
	}
	
	//calculate measure
	int S = num_removed + 1;
	double myVAT = S / (grph.nodes.size() - S - cmax + 1.0);
	//calculate Integrity = (|S| + cMax)/V
	double myIntegrity = ((double)S + cmax) / grph.nodes.size();  // should s.length be g.numnodes?
	
	thevat = myVAT;
	theintegrity = myIntegrity;
}

/*
vector<vector<int>> NBR::find_components(const Graph &g)
{
	vector<vector<int>> components;
	vector<int> component;
	components.push_back(component);
	vector<bool> visited;
	for (size_t i = 0; i < g.nodes.size(); i++)
	{
		visited.push_back(false);
	}
	int source = 0;
	//visited[source] = true;
	bool nodes_remain = true;
	int cluster_num = 0;
	while (nodes_remain)
	{
		std:queue<int> q;
		q.push(source);

		while (!q.empty())
		{
			int u = q.front();
			q.pop();
			if (!visited[u])
			{
				visited[u] = true;
				components[cluster_num].push_back(u);
			}
			for (size_t i = 0; i < g.nodes[u].size(); i++)
			{
				if (!visited[g.nodes[u][i]])
				{
					q.push(g.nodes[u][i]);
				}
			}
		}
		// see where we are:
		for (size_t i = 0; i < visited.size(); i++)
		{
			nodes_remain = false;
			if (!visited[i])
			{
				source = i;
				cluster_num++;
				components.push_back(component);
				nodes_remain = true;
				break;
			}			
		}
	}
	return components;
}
*/
//taken from geeks for geeks
void NBR::connectedComponents(const GraphOrig &g)
{
	// clear the instance variable
	comps.clear();
	// Mark all the vertices as not visited
	int V = g.nodes.size();
	bool *visited = new bool[V];
	for (int v = 0; v < V; v++)
		visited[v] = false;

	for (int v = 0; v<V; v++)
	{
		if (visited[v] == false)
		{
			comps.push_back(0);
			// print all reachable vertices
			// from v
			DFSUtil(v, visited, g);

			//cout << "\n";
		}
	}

	delete [] visited;
}

void NBR::DFSUtil(int v, bool visited[], const GraphOrig &g)
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
	for (std::vector<int>::const_iterator i = g.nodes[v].begin(); i != g.nodes[v].end(); ++i) {
		if (!visited[*i])  // was *i
			DFSUtil(*i, visited, g);
	}
}

std::vector<int> NBR::get_bader_approx(const GraphOrig &g, int myK, int c_constant, int samp)
{
	//int highBC[] = { 8779,9637,9627,9054,9799,7874,9824,9751,9981,9954,9587,8287,9861,9472,8920,9834,4268,9066,9780,7472,7947,9972,7586,9145,9338,5565,5542,8474,8487,7866,9048,9939,9937,9268,9738,9161,7258,6028,9705,9711,8452,8108,8396,9915,6352,8930,8616,9944,9017,9292,8957,9270,9652,8864,9830,6419,9491,6923,7120,9846,9633,8735,9876,1199,7249,9409,9091,7532,9596,9673,9566,8660,5078,8916,8285,7304,6096,7096,4677,4569,7788,5558,1567,3972,5246,8378,4493,3209,8713,8270,7449,6514,9889,7619,9114,9214,9631,6975,9430,9381,1227,9347,9266,8459,9477,9791,8686,8794,7295,4640,9320,8763,9242,8833,8374,8496,9586,5860,9921,9131,6532,7287,8367,9420,9120,8438,9706,4836,8860,9684,6733,8889,6429,7705,9366,9588,9273,8296,9842,8873,9584,6962,1647,8471,7749,8795,3490,9957,3812,5767,3113,9869,9494,9460,5027,2592,9624,8807,9254,8530,9968,4885,3862,7728,6062,8890,3521,9742,5910,6327,9104,6222,3064,5681,7381,7451,6671,8362,9153,6594,6631,9924,1573,6674,2731,8762,7438,6799,6757,5107,4932,9469,6722,5579,4844,421,6969,6548,9734,8523,8669,8832,7670,8240,6475,7414,886,3652,7413,8346,6951,2849,9783,9444,7537,3871,8249,9434,9390,6882,8162,7511,6623,8705,8847,6233,9059,9239,9078,5097,5786,2841,6751,9613,8899,9727,9466,8500,7484,7539,4443,9951,8721,5882,7324,2990,9129,1524,7087,8314,5741,2927,762,8219,3649,5475,1255,1355,5759,1692,5380,8195,1824,4768,8008,9081,4011,7802,4598,1611,7857,3827,4245,9395,9761,8764,9940,9700,9953,9606,7209,9853,6768,7666,7783,3877,7727,2425,9301,8946,9639,9240,9868,9910,7301,8853,5226,1091,7351,8880,9535,8382,8502,9610,5076,7473,8571,6065,9344,9262,5261,2607,8224,7793,9794,8288,9467,7833,3906,7658,9383,7154,8671,7807,8758,9600,6496,9539,9509,6934,8953,8567,6289,9568,9160,8924,7799,8217,8330,9417,5325,8048,415,5599,9864,9030,6445,8509,5401,9088,9920,9062,3450,8989,9844,5229,7815,7218,9052,2485,9140,9741,5635,7081,9250,5259,9179,6630,3765,6342,2394,7325,4920,5279,8519,7635,4712,1217,6610,2775,8430,9311,8380,9746,9974,6981,6933,5255,7563,4403,9414,9201,9914,9330,7735,1839,9816,5864,9413,299,6931,2823,9560,9302,5836,8911,6919,7315,7854,9654,8978,5720,7463,5646,5257,6967,9762,8480,9443,8433,7516,624,6176,5031,6172,9668,4482,7387,6074,8727,6186,8022,7134,7068,6067,7015,6537,8942,6831,9554,6876,5018,8636,5999,9839,8948,2336,8043,8511,8045,8155,8684,4289,9644,790,6063,1221,6763,7638,1786,4984,6251,4430,7283,9170,3313,5708,6478,6358,6243,682,482,4491,794,9856,6213,9212,9805,2717,7467,5400,8284,8238,7975,9682,3091,9790,8823,5523,9985,8815,7123,9519,9010,8267,8146,9640,8723,9327,8228,8782,9173,9245,3351,8923,9154,2020,2016,8147,9106,9987,9012,8893,9646,6178,6295,2687,749,1781,5785,7873,3387,9387,8554,6078,9932,5560,8664,4183,9190,5119,9990,8521,7135,8534,8917,6900,4762,5540,1822,2291,1218,1894,532,3385,1174,7567,8906,6328,7952,3610,7263,9782,9476,9300,7814,9525,9125,9720,9767,8951,8608,6026,7894,7177,9786,6459,1297,9034,9905,9267,5110,9989,8801,7896,8577,8377,9108,9699,9599,7594,9996,8546,7716,8454,9772,8484,9146,9305,8983,9199,6229,9335,5017,4798,2652,8522,9031,9150,9831,9685,7839,9296,9750,4851,9897,3323,3139,7992,7481,9003,9695,6995,8105,4849,7342,491,7250,3752,9182,5918,9601,8985,8552,9492,8537,1270,3811,268,5060,9149,9709,9457,9019,6351,9666,8303,8835,8875,8278,3014,8334,8218,7167,2477,1643,7153,8324,6620,4078,8340,8180,8943,5132,5203,6116,8313,7736,1693,3882,7551,7028,6005,3235,9576,9258,9045,3561,7411,9912,6201,8914,3215,7030,592,1481,8811,1703,6439,1887,9871,9185,7610,9630,8813,9575,9380,9683,8892,9907,8478,8992,5975,5925,4907,4964,8971,8700,9116,4758,9901,9838,2882,9714,3001,9354,8894,8897,9792,5933,7377,6117,9174,8096,6912,6944,8862,9235,9811,4143,9319,8766,8977,4724,6338,8263,6691,3457,4590,5210,2355,6580,4024,8696,400,7031,5404,7396,8154,1488,9960,4271,9692,9368,6622,4678,9450,9765,9885,7113,9481,9926,3588,8009,6293,8034,9726,8755,9956,9769,8729,9899,9891,6156,8202,8299,7988,9343,6845,6333,5402,2696,5815,7405,1162,8159,6718,9001,8749,9397,4264,6125,9406,4338,2445,4102,2093,2251,855,567,4695,6817,3435,9663,7524,2031,1278,2982,3491,4316,1430,5787,3427,2088,9865,6978,9880,7206,3983,7923,6824,9626,7220,8130,7363,9308,5595,7762,3057,4559,8658,8621,3170,5129,5773,1650,1687,9495,7650,8589,7306,9800,5434,5453,7884,8981,7116,8139,7733,5699,5330,2209,4892,9655,6856,9775,7720,1012,4189,1902,4405,5514,5352,2242,6783,8716,9069,6960,9638,9781,8908,9556,7949,8781,9893,8859,7934,1917,9814,7921,9963,4428,8852,9543,8565,7214,377,8738,6349,9823,7617,8015,8557,3766,8444,8837,9952,7663,7364,8680,8455,7639,9812,8915,9277,9008,4939,7847,9702,8786,9073,9122,6416,9438,9970,5717,2850,6425,623,4837,9095,6143,4828,7442,5924,3611,9322,593,1597,8397,9304,9986,3475,6471,9594,7244,9448,7620,7090,1376,4285,2436,3374,5916,5757,5100,2609,608,7522,5111,2124,28,6288,3697,7732,7137,6457,7431,7070,9737,8171,8301,8692,9130,8724,9291,8532,8109,9462,7302,7676,8639,7435,8629,6578,9536,8462,6639,6961,5151,5771,7559,7518,9424,8411,6551,9604,2433,2188,4532,5253,5510,8094,8731,7970,8826,6093,2027,5513,9992,8796,8013,7293,7475,9793,9719,8607,8127,7608,9962,7630,4733,7861,7938,9650,3734,9063,7462,9007,8725,6511,7724,9079,8861,9177,7380,8803,9662,8269,5808,8780,9887,4394,9617,6942,3206,8322,4847,9165,1444,3295,4801,7609,6311,4922,8482,684,3037,9949,6413,3665,3127,7330,6869,4027,7453,681,8262,5561,9164,2761,4805,8364,6600,3555,5104,3784,3350,9616,9080,4097,8578,8093,4175,7653,7673,8168,9969,7621,6369,3613,7870,4341,3737,8872,6540,4975,913,9181,6534,2172,9845,8089,4596,6200,6436,5544,8843,9144,8555,9192,9607,5469,9365,7991,8672,7329,9248,5439,7268,8819,4280,8642,7489,5798,8973,3258,2249,271,9223,2843,1930,5876,1381,4298,1514,5628,996,7350,2890,1552,8348,8416,7786,8383,9593,7904,6381,9269,7558,8047,9238,7721,5443,8103,6437,5372,7796,7553,1340,6132,2290,9092,1995,2776,710,4069,9755,9611,6877,7631,991,578,9215,3125,9369,9551,9283,7575,9126,8351,9997,7965,6696,7950,9648,9625,5058,9553,8133,6379,8602,9881,7285,9454,9391,2019,7644,9378,7603,6105,3774,7290,9426,4479,813,2932,837,5075,7390,4313,5664,1350,3185,8513,8514,2856,367,3200,4963,8947,7960,590,6491,5801,6174,8634,9084,8868,6401,8994,8307,7147,6660,7222,9941,8617,1367,524,9461,8467,9707,9222,5598,9643,8997,7652,6142,1104,8373,7616,8586,8810,8709,8673,8236,9851,7500,3023,9619,8704,3485,7964,9511,8068,8129,9449,5831,9294,5053,3919,5522,700,3318,7956,8477,9171,9363,8921,8618,9674,5592,7813,5666,9521,8584,9002,5921,6886,9671,9884,7428,6438,7686,5863,3695,5013,9877,3576,6402,2177,3895,1525,6111,6562,7726,7198,8884,6456,1960,2727,9384,8494,7687,9440,9696,8122,7714,9489,7805,9168,5066,9463,9297,8874,2176,7194,9933,6130,7826,9416,8186,9770,9458,7851,9888,9961,4754,6673,8197,5288,3120,1243,7984,5285,4820,6163,9431,6444,3920,8290,5328,6138,9117,3286,5870,7561,7544,9233,4549,259,7753,8858,8693,3000,9237,8117,4046,7765,5884,2898,6331,5077,9204,9636,7050,6234,4012,2094,6046,5003,6892,8222,8194,8200,9998,2611,9038,4328,6060,7439,1981,6185,9392,9453,8941,8736,9892,8394,9163,9909,9929,9632,8125,9620,7767,7568,9936,1073,7545,9725,8583,8788,7526,9371,8655,9346,3921,7264,4929,4039,7144,7005,7300,7852,7933,6196,9128,7572,6618,8603,8516,7066,6157,336,7738,6715,597,8237,1536,7747,2767,6698,8468,3531,9101,9362,8007,9965,6388,4803,8132,9404,7684,5589,7223,7026,6461,9860,9349,2125,9931,6917,9983,7760,6599,6231,8896,9635,7241,6304,9272,9801,4998,6508,4251,3661,6035,3364,1284,2971,4470,9710,7172,4637,5329,9806,7433,8934,9184,3708,5791,8347,9902,9402,5244,7318,7816,9257,1473,6391,9697,4766,5015,6516,5209,9670,4409,6593,8668,6264,7176,7741,4682,2694,6716,6337,2452,6683,8701,6862,8371,9585,9718,5109,6193,6818,4389,8220,6270,9388,566,2571,7202,8151,9051,8747,4212,8279,9103,6155,4007,8681,1740,3619,8579,848,9603,2883,2134,8031,4780,4858,7994,6030,8761,7593,8398,9504,4001,5559,8423,5563,8926,8261,5308,5539,8161,8517,8230,8280,8784,7145,3591,9358,6829,8665,8541,9621,5998,5286,6964,2518,6503,9768,9874,4050,8600,9597,8140,9137,9779,4999,7916,2816,8768,1617,9752,7397,9119,6094,9261,5387,287,8624,6957,9351,7224,1928,8405,9918,8012,5483,9499,9841,8167,9557,5766,8821,8208,9474,8829,9822,3327,1814,9341,6175,8442,4371,1966,6866,6628,3367,506,6291,5650,8259,7892,1833,9784,8976,7156,1086,9573,9978,8980,8791,773,8232,9657,9332,9948,1225,2993,2480,3698,9089,8424,9567,9203,5651,7623,5290,5283,5923,1398,8891,4487,9704,1869,8722,8027 };
	vector<int> finished;
	GraphOrig ourGraph(g);
	char c;
	
	//for (int z = 0; z < 1652; z++)
	for (size_t z = 0; z < myK; z++)
	{
		sgraph_t comp;
		comp.nodes_vec = ourGraph.nodes;
		vector<double> RAND1_between_centrality_result;
		
		//int adaptive_sample_size = GSIZE(comp, GSIZE_between_centrality_result, samp, c_constant);
		//int adaptive_sample_size = GSIZE(comp, GSIZE_between_centrality_result, highBC[z], c_constant);
		RAND1(comp, RAND1_between_centrality_result, c_constant);
		//cout << "sample Size " << c_constant << endl;
		
		for (int i = 0; i<ourGraph.nodes.size(); i++) {
			if (c_constant!=0)
			RAND1_between_centrality_result[i] = (comp.size() / c_constant)*RAND1_between_centrality_result[i];
		}
		
		
		double max_btws = 0;
		double max_degree = 0;
		int max_degree_loc = 0;
		int max_loc = 0;
		for (size_t j = 0; j < ourGraph.nodes.size(); j++)
		{
			if (RAND1_between_centrality_result[j] > max_btws)
			{
				max_btws = RAND1_between_centrality_result[j];
				max_loc = j;
			}
			if (ourGraph.nodes[j].size() > max_degree)
			{
				max_degree = ourGraph.nodes[j].size();
				max_degree_loc = j;
			}

		}
		samp = max_degree_loc;
		if (max_btws <= 1) // we're done then!
		{
			break;
		}
		//if (std::find(finished.begin(), finished.end(),max_loc ) == finished.end())
		finished.push_back(max_loc);

		//------------------------------------------------------------------------------------------------------------------
		// This part removes the node from the graph and converts again
		// now need to remove the max_loc vertex from the graph...
		// remove node from ourGraph
		// now we have to remove node maxCl_Location from the graph
		// to prepare to find the next max
		vector<int> adjs = ourGraph.nodes[max_loc];
		for (size_t j = 0; j < adjs.size(); j++)
		{
			int location_to_remove_from = adjs[j];
			// go to h.nodes[location_to_remove] and remove maxCl_Location
			auto it = find(ourGraph.nodes[location_to_remove_from].begin(), ourGraph.nodes[location_to_remove_from].end(), max_loc);
			if (it != ourGraph.nodes[location_to_remove_from].end())
			{
				ourGraph.nodes[location_to_remove_from].erase(it);
			}

		}
		ourGraph.nodes[max_loc].clear();
		cout << "Finished node " << z << " removed " << max_loc << endl;

		

	}

	return finished;

}

