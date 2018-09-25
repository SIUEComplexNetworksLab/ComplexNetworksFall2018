/*
 * DynAPSP.h
 *
 *  Created on: 12.08.2015
 *      Author: Arie Slobbe, Elisabetta Bergamini
 */

#ifndef DYNAPSP_H_
#define DYNAPSP_H_

#include "APSP.h"
#include "../dynamics/GraphEvent.h"
#include "../base/DynAlgorithm.h"

namespace NetworKit {

/**
 * @ingroup graph
 * Dynamic APSP.
 */
class DynAPSP : public APSP, public DynAlgorithm {

public:
	/**
	 * Creates the object for @a G.
	 *
	 * @param G The graph.
	 */
	DynAPSP(Graph& G);

	/** initialize distances and Pred by repeatedly running the Dijkstra2 algorithm */
	void run();

  /**
  * Updates the pairwise distances after an edge insertions on the graph.
  * Notice: it works only with edge insertions.
  *
  * @param e The edge insertions.
  */
  void update(GraphEvent e) override;

  /**
  * Updates the pairwise distances after a batch of edge insertions on the graph.
  * Notice: it works only with edge insertions.
  *
  * @param batch The batch of edge insertions.
  */
  void updateBatch(const std::vector<GraphEvent>& batch) override;


	/** Returns number of visited pairs */
	count visPairs();

	/**
	* Returns a vector containing a shortest path from node u to node v, and an empty path if u and v are not connected.
	*
	*/
	std::vector<node> getPath(node u, node v);

private:

	const edgeweight infDist = std::numeric_limits<edgeweight>::max();
	const edgeweight epsilon = 0.0000000001; //make sure that no legitimate edge weight is below that.
	count visitedPairs = 0;
};

} /* namespace NetworKit */

#endif /* DynAPSP_H_ */
