#include "MaximalCliquesGTest.h"

#include "../MaximalCliques.h"
#include "../../graph/Graph.h"
#include "../../io/METISGraphReader.h"
#include "../../io/EdgeListReader.h"
#include "../../auxiliary/Log.h"
#include "../../auxiliary/Timer.h"

#ifndef NOGTEST

namespace NetworKit {

TEST_F(MaximalCliquesGTest, testMaximalCliques) {

	METISGraphReader reader;
	Graph G = reader.read("input/hep-th.graph");

	node seed = 2;
	auto sn = G.neighbors(seed);
	auto sneighbors = std::unordered_set<node>(sn.begin(), sn.end());
	sneighbors.insert(seed);
	auto subG = G.subgraphFromNodes(sneighbors);

	MaximalCliques clique(subG);

	DEBUG("Call MaximalCliques()");

	clique.run();
	const auto &result = clique.getCliques();

	EXPECT_GT(result.size(), 0u);

	// check results (are they cliques?)
	for (auto cliq : result) {
		auto cli = std::unordered_set<node>(cliq.begin(), cliq.end());
		auto cliqueGraph = G.subgraphFromNodes(cli);

		EXPECT_EQ(cliqueGraph.numberOfEdges(), (cliqueGraph.numberOfNodes() * (cliqueGraph.numberOfNodes() - 1) / 2));
		EXPECT_EQ(cli.count(seed), 1u);
	}
}

TEST_F(MaximalCliquesGTest, testMaximalCliquesOnWholeGraph) {

	METISGraphReader reader;
	Graph G = reader.read("input/hep-th.graph");

	MaximalCliques clique(G);

	DEBUG("Call MaximalCliques()");

	clique.run();
	const auto &result = clique.getCliques();

	EXPECT_GT(result.size(), 0u);

	// check results (are they cliques?)
	std::vector<bool> inClique(G.upperNodeIdBound());
	for (const auto& cliq : result) {
		for (node u : cliq) {
			inClique[u] = true;
		}

		const count expected_degree = cliq.size() - 1;

		for (node u : cliq) {
			count neighborsInClique = 0;
			G.forNeighborsOf(u, [&](node v) {
				neighborsInClique += inClique[v];
			});

			EXPECT_EQ(expected_degree, neighborsInClique);
		}

		for (node u : cliq) {
			inClique[u] = false;
		}
	}
}

TEST_F(MaximalCliquesGTest, testMaximalCliquesWithCallback) {

	METISGraphReader reader;
	Graph G = reader.read("input/hep-th.graph");

	std::vector<bool> inClique(G.upperNodeIdBound());

	count numCliques = 0;
	MaximalCliques clique(G, [&](const std::vector<node>& cliq) {
		++numCliques;

		for (node u : cliq) {
			inClique[u] = true;
		}

		const count expected_degree = cliq.size() - 1;

		for (node u : cliq) {
			count neighborsInClique = 0;
			G.forNeighborsOf(u, [&](node v) {
				neighborsInClique += inClique[v];
			});

			EXPECT_EQ(expected_degree, neighborsInClique);
		}

		for (node u : cliq) {
			inClique[u] = false;
		}
	});

	DEBUG("Call MaximalCliques()");

	clique.run();

	EXPECT_GT(numCliques, 1u);
}

TEST_F(MaximalCliquesGTest, benchMaximalCliques) {
	std::string graphPath;

	std::cout << "[INPUT] graph file path (edge list tab 0, like SNAP) > " << std::endl;
	std::getline(std::cin, graphPath);

	EdgeListReader r('\t',0,"#", false);
	Graph G = r.read(graphPath);
	G.removeSelfLoops();
	INFO(G.size());
	INFO("Starting MaximalCliques");
	Aux::Timer timer;
	count numCliques = 0;
	count maxSize = 0;
	MaximalCliques clique(G, [&](const std::vector<node>& clique) {
		++numCliques;
		if (clique.size() > maxSize) {
			maxSize = clique.size();
		}
	});
	timer.start();
	clique.run();
	timer.stop();
	INFO("Found ", numCliques, " cliques");
	INFO("Maximum clique size found: ", maxSize);
	INFO("Needed ", timer.elapsedMilliseconds(), "ms for clique detection");

	{
		INFO("Starting again to just find the maximum");
		Aux::Timer timer;
		MaximalCliques clique(G, true);
		timer.start();
		clique.run();
		timer.stop();
		const auto& cliques = clique.getCliques();
		EXPECT_EQ(1u, cliques.size());
		EXPECT_EQ(cliques.front().size(), maxSize);
		INFO("Just finding the maximum clique needed ", timer.elapsedMilliseconds(), "ms");
	}
}
}

#endif /* NOGTEST */
