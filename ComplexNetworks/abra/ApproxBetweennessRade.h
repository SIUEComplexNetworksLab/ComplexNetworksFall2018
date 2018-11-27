/*
 * ApproxBetweennessRade.h
 *
 *  Created on: 23.06.2015
 *      Author: Matteo Riondato
 *
 *  Copyright 2016 Matteo Riondato <riondato@acm.org>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef APPROXBETWEENNESSRADE_H_
#define APPROXBETWEENNESSRADE_H_

#include "../centrality/Centrality.h"

namespace NetworKit {


/**
 * @ingroup centrality
 * Approximation of betweenness centrality according to algorithm described in
 * Matteo Riondato and Eli Upfal: Approximating Betweenness Centrality
 * in Static and Dynamic Graphs with Rademacher Averages
 */
class ApproxBetweennessRade: public NetworKit::Centrality {

public:

    /**
     * The algorithm approximates the betweenness of all vertices so that the scores are
     * within an additive error @a epsilon with probability at least (1- @a delta).
     * The values are normalized by default.
     *
     * @param   G                       the graph
     * @param   epsilon                 maximum additive error
     * @param   delta                   probability that the values are not within the error guarantee
     * @param   useRefinedEstimator     controls whether to use the refined
     * "linear scaling" estimator from Geisberger et al., "Better Approximation
     * of Betweenness Centrality", ALENEX'08.
     */
    ApproxBetweennessRade(const Graph& G, const double epsilon=0.01,
            const double delta=0.1, const double sampleScheduleMultiplier=0.01,
            const bool useRefinedEstimator=false);

    void run() override;

    /**
     * @return number of samples taken in last run
     */
    count numberOfSamples();

    /**
     * Fill the passed array of size at least 4 with stats about the execution.
     * The first element will contain the runtime for sampling, including
     * performing the SSSP computations and the backtracking.
     * The second element will contain the runtime for checking the stopping
     * condition.
     * The third element will contain the total time, all included.
     * The fourth element will contain the number of iterations.
     *
     * @param stats an array of size at least 4 that will be filled with the values.
     */
    void getStats(unsigned long *stats);

private:

    const bool useRefinedEstimator;
    const double epsilon;
    const double delta;
    const double sampleScheduleMultiplier;
    const double penaltyFactor; // 2 if useRefinedEstimator==true, 1 otherwise
    count sampleSize; // number of samples taken in last run
    unsigned long sampling_time;
    unsigned long stopping_condition_time;
    unsigned long total_time;
    unsigned long iterations;
};

} /* namespace NetworKit */

#endif /* APPROXBETWEENNESSRADE_H_ */
