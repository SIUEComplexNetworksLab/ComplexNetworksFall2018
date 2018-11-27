/*
 * ApproxBetweennessRadeRade.cpp
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
 *
 */

#include "RadeAux.h"
#include "ApproxBetweennessRade.h"
#include "../auxiliary/Random.h"
#include "../graph/Sampling.h"
#include "../distance/Dijkstra.h"
#include "../distance/BFS.h"
#include "../distance/SSSP.h"
#include "../auxiliary/Log.h"
#include "../auxiliary/SignalHandling.h"

#include <array>
#include <chrono>
#include <cmath>
#include <limits>
#include <unordered_map>
#include <vector>

#include <omp.h>

namespace NetworKit {

class RunningObjFuncSumExponents {
    private:
        std::map<size_t,double> objFuncSumExponentsMap;
    public:
        RunningObjFuncSumExponents() {}
        RunningObjFuncSumExponents(const std::map<size_t, double> &
                objFuncSumExponentsMap):
            objFuncSumExponentsMap(objFuncSumExponentsMap) {}

        RunningObjFuncSumExponents& operator+=(const RunningObjFuncSumExponents& rhs) {
            objFuncSumExponentsMap.insert(rhs.objFuncSumExponentsMap.begin(),
                    rhs.objFuncSumExponentsMap.end());
            return *this;
        }

        friend RunningObjFuncSumExponents operator+(RunningObjFuncSumExponents
                lhs, const RunningObjFuncSumExponents& rhs) {
            lhs += rhs;
            return lhs;
        }

        RunningObjFuncSumExponents& operator+=(const std::pair<size_t, double> &
                rhs) {
            objFuncSumExponentsMap.insert(rhs);
            return *this;
        }

        std::vector<double> getVector() {
            std::vector<double> v;
            for (auto entry : objFuncSumExponentsMap) {
                v.push_back(entry.second);
            }
            return v;
        }
};


ApproxBetweennessRade::ApproxBetweennessRade(const Graph& G, const double
        epsilon, const double delta, const double sampleScheduleMultiplier, const bool useRefinedEstimator) :
    Centrality(G, true), useRefinedEstimator(useRefinedEstimator),
    epsilon(epsilon), delta(delta),
    sampleScheduleMultiplier(sampleScheduleMultiplier),
    penaltyFactor((useRefinedEstimator) ? 2.0 : 1.0), sampling_time(0ul),
    stopping_condition_time(0ul),
    total_time(0ul), iterations(0ul) {
}


void ApproxBetweennessRade::run(void) {
    auto total_start_time = std::chrono::steady_clock::now();
    Aux::SignalHandler handler;
    scoreData.clear();
    scoreData.resize(G.upperNodeIdBound());

    std::pair<double,double> minimizationResults(0, 100); // minimum and point of minimum for the objective function used in stopping condition.

    // The commented out initial sample size is for the "vanilla" bound to the
    // supremum of the deviation
    //sampleSize = 9.0 * penaltyFactor * log(2 / delta) / (epsilon * epsilon);
    // The following initial sample size is suitable for the refined bound to
    // the supremum of the deviation from Oneto et al., "An improved analysis of
    // the Rademacher data-dependent bound using its self bounding property",
    // Neural Networks 44, 107-111, 2013.
    // TODO MR Think about the penaltyFactor for the refined estimator
    sampleSize = ceil(log(2.0 / delta) * (sqrt(1 + 16 * epsilon)  + 1 + 8 * epsilon) / (4 * epsilon * epsilon));
    count toSample = sampleSize;
//    DEBUG("initial sample size is ", toSample, " samples");
    const count maxThreads = omp_get_max_threads();
//    DEBUG("max threads: ", maxThreads);
    // XXX MR TODO MR The following is kind of ugly as is preallocates the
    // memory for the maps considering a worst case scenario. This is done to
    // avoid issues (that happened) when a map needs to be reallocated and there
    // are multiple threads accessing the vector, as it invalidates pointers to
    // the map. I believe the real solution would be to reorganize the
    // computation so that we can use a reduction.
    std::vector<std::unordered_map<count, double>>
        threadSumContribsSquared(maxThreads, std::unordered_map<count,
                double>({{0,0.0}}, G.upperNodeIdBound()));
    std::vector<std::unordered_map<count, double>> threadSumContribs(maxThreads,
            std::unordered_map<count, double>({{0,0.0}}, G.upperNodeIdBound()));
    std::vector<std::unordered_map<count, count>>
        threadSumContribsCounts(maxThreads, std::unordered_map<count,
                count>({{0,G.upperNodeIdBound()}}, G.upperNodeIdBound()));
    std::vector<std::vector<count>> threadNodeMap(maxThreads,
            std::vector<count>(G.upperNodeIdBound(), 0));
    std::vector<count> threadKeyCounter(maxThreads, 0);
//    DEBUG("score per thread size: ", threadNodeMap.size());
    double supDeviationBound = 1;
    do {
        auto sampling_start_time = std::chrono::steady_clock::now();
        ++iterations;
//        DEBUG("Iteration: ", iterations, ", going to sample ", toSample, " pairs of nodes");
        handler.assureRunning();
        #pragma omp parallel for //num_threads(1)
        for (count i = 1; i <= toSample; ++i) {
            if (!handler.isRunning()) continue;
            const count thread = omp_get_thread_num();
//            TRACE("sample ", i, " of ", toSample);
            const node source = Sampling::randomNode(G);
            node target = Sampling::randomNode(G);
            while (target == source) {
                target = Sampling::randomNode(G);
            }
            bool forward = false; // direction for the refined estimator
            if (useRefinedEstimator) {
                forward = Aux::Random::integer(1u);
            }

            // runs faster for unweighted graphs
            std::unique_ptr<SSSP> sssp;
            if (G.isWeighted()) {
                sssp.reset(new Dijkstra(G, source, true, false, target));
            } else {
                sssp.reset(new BFS(G, source, true, false, target));
            }
//            TRACE("running shortest path algorithm from node ", source, " to node ", target);
            if (!handler.isRunning()) continue;
            sssp->run();
            if (!handler.isRunning()) continue;
            if (sssp->numberOfPaths(target) > 0 && sssp->distance(target) > 1) {
                // at least one path from source to target exists, and the SP distance
                // from source to target is greater than 1.
//                TRACE("updating estimate for nodes on SPs from ", source, " to ", target);
                // Number of paths from node to target
                // XXX MR: why bigfloat?
                std::unordered_map<node, bigfloat> numberOfSPsToTarget;
                // Priority queue to go through nodes in reverse order of distance from source.
                auto less_distance = [&](const node a, const node b) -> bool { return sssp->distance(a) < sssp->distance(b); };
                std::priority_queue<node, std::vector<node>, decltype(less_distance)> nodeQueue(less_distance);
                // Adding the predecessors of target to the maps to start the backward traversal.
                for (const node pred : sssp->getPredecessors(target)) {
                    // There is obviously one and only one SP from a predecessor of target to target.
                    numberOfSPsToTarget.insert(std::make_pair(pred, 1));
                    nodeQueue.push(pred);
                }
                // This is a thread-local map that tells us which old keys are mapped to which new (thread-local) keys in the threadSumContribs*dictionaries.
                std::unordered_map<double, count> new_map_keys;
                // Start the backtracking, visiting nodes in reverse order of distance from source.
                node curr = nodeQueue.top();
                nodeQueue.pop();
                do {
//                    TRACE("THREAD: ", thread, ", i: ", i, ", curr: ", curr);
                    const bigfloat numberOfPathsThroughCurr = sssp->numberOfPaths(curr) * numberOfSPsToTarget[curr];
                    // Contribution of (source,target) to the BC estimation of curr.
                    bigfloat bfContrib = numberOfPathsThroughCurr / sssp->numberOfPaths(target);
                    if (useRefinedEstimator) {
                        const bigfloat multiplier = 2 * ( (forward) ? sssp->distance(curr) / sssp->distance(target) : 1 - sssp->distance(curr) / sssp->distance(target));
                        bfContrib *= multiplier;
                    }
                    double contrib;
                    bfContrib.ToDouble(contrib);
                    const count currKey = threadNodeMap[thread][curr];
//                    TRACE("\t currKey: ", currKey);
                    // Compute the new key for curr following the update. It
                    // contains info about the current key, and the contrib.
                    // The additional factor 2 is needed to avoid collisions when using the refined estimator.
                    double key_to_new_map_key = contrib + 2.0 * currKey;
                    if (new_map_keys.count(key_to_new_map_key) == 0) {
                        threadKeyCounter[thread] += 1;
                        count new_key = threadKeyCounter[thread];
                        new_map_keys.insert(std::make_pair(key_to_new_map_key, new_key));
                        threadSumContribsSquared[thread].insert(std::make_pair(new_key,
                                    threadSumContribsSquared[thread][currKey] + (contrib * contrib)));
                        threadSumContribs[thread].insert(std::make_pair(new_key,
                                    threadSumContribs[thread][currKey] + contrib));
                        threadSumContribsCounts[thread].insert(std::make_pair(new_key, 1));
                    } else {
                        threadSumContribsCounts[thread][new_map_keys[key_to_new_map_key]] += 1;
                    }
                    threadNodeMap[thread][curr] = new_map_keys[key_to_new_map_key];
                    if (threadSumContribsCounts[thread][currKey] == 1) {
                        threadSumContribsCounts[thread].erase(currKey);
                        threadSumContribs[thread].erase(currKey);
                        threadSumContribsSquared[thread].erase(currKey);
                    } else {
                        assert(threadSumContribsCounts[thread][currKey] > 1);
                        threadSumContribsCounts[thread][currKey] -= 1;
                    }
                    // Add the predecessors of curr to the relevant maps for backtracking
                    for (const node pred: sssp->getPredecessors(curr)) {
                        if (numberOfSPsToTarget.count(pred) == 0) {
                            numberOfSPsToTarget.insert(std::make_pair(pred, numberOfSPsToTarget[curr]));
                            nodeQueue.push(pred);
                        } else {
                            numberOfSPsToTarget[pred] += numberOfSPsToTarget[curr];
                        }
                    }
                    curr = nodeQueue.top();
                    nodeQueue.pop();
                } while (curr != source);
            } // End of if block checking if we reached the target
        } // End of for loop to do the sampling
        sampling_time +=
            std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()
                    - sampling_start_time).count();
        handler.assureRunning();
        auto stopping_condition_start_time = std::chrono::steady_clock::now();

//        DEBUG("merging thread-local dictionaries");
        const double exponentDenominator = 2 * (sampleSize * sampleSize);
        #pragma omp declare reduction (runningObjFuncSumExponentsReduction: RunningObjFuncSumExponents: omp_out += omp_in)
        RunningObjFuncSumExponents runningObjFuncSumExponents;
        #pragma omp parallel for reduction(runningObjFuncSumExponentsReduction:runningObjFuncSumExponents)
        for (node curr = 0; curr < G.upperNodeIdBound(); ++curr) {
            // For each node, collect its thread-local keys and compute an
            // hash of them. Nodes with identical hashes must have the same
            // global key
            std::size_t hash = 0;
            double objFuncSumExponent = 0.0;
            scoreData[curr] = 0;
            for (count thread_i = 0; thread_i < maxThreads; ++thread_i) {
                count currKey = threadNodeMap[thread_i][curr];
                scoreData[curr] += threadSumContribs[thread_i][currKey];
                objFuncSumExponent += threadSumContribsSquared[thread_i][currKey];
                // This way of computing the hash of a vector is
                // equivalent (?) to what is done in the boost hash_range
                // function.
                // See http://www.boost.org/doc/libs/1_58_0/doc/html/hash/reference.html#header.boost.functional.hash_hpp
                // It was taken from http://stackoverflow.com/questions/20511347/a-good-hash-function-for-a-vector
                hash ^= currKey + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            }
            runningObjFuncSumExponents += std::make_pair(hash, objFuncSumExponent / exponentDenominator);
        }

        handler.assureRunning();
        std::vector<double> objFuncSumExponents = runningObjFuncSumExponents.getVector();
//        DEBUG("checking stopping condition");
        supDeviationBound = getSupDeviationBound(delta, sampleSize, penaltyFactor, objFuncSumExponents, minimizationResults);
//        DEBUG("bound to the maximum deviation: ", supDeviationBound);
        if (supDeviationBound <= epsilon) {
            G.parallelForNodes([&](node curr) {
                scoreData[curr] = scoreData[curr] / sampleSize;
            });
        } else {
            // compute new sample size and iterate
            toSample = sampleSize;
            if (sampleScheduleMultiplier == 0.0) {
                // The commented out method to compute the next sample size is
                // for the "vanilla" bound to the supremum of the deviation
                //sampleSize *= (supDeviationBound * supDeviationBound) / (epsilon * epsilon);
                // The following method to compute the next sample size is
                // suitable for the refined bound to the supremum of the
                // deviations from Oneto et al., "An improved analysis of
                // the Rademacher data-dependent bound using its self bounding
                // property", Neural Networks 44, 107-111, 2013.
                // TODO MR Think about the penaltyFactor for the refined estimator
                assert(minimizationResults.first < epsilon);
                const double mylog = log(2 / delta);
                const double epsilon_square = epsilon * epsilon;
                const double radebound_square = minimizationResults.first * minimizationResults.first;
                const double twentyseven_radebound_square = 27 * radebound_square;
                const double next_multiplier = mylog / (6 * (epsilon_square - 2 * epsilon * minimizationResults.first + radebound_square));
                const double next_1st_term = 2 + 8 * epsilon;
                const double next_2nd_term_multiplier = sqrt(48 * minimizationResults.first + 1 + 8 * epsilon + 16 * epsilon_square);
                const double atan2_1st_arg = 12 * sqrt(3) * fabs(-1 + 2 * (minimizationResults.first + epsilon)) * sqrt(- (twentyseven_radebound_square - epsilon_square * (1 + 16 * epsilon) - minimizationResults.first * (1 + 18 * epsilon)));
                const double atan2_2nd_arg = - ( -1 - 12 * epsilon + 8 * ( twentyseven_radebound_square + (21 - 8 * epsilon) * epsilon_square + 18 * minimizationResults.first * (1 + epsilon)));
                const double atan2_res = atan2(atan2_1st_arg, atan2_2nd_arg);
                const double theta = atan2_res / 3;
                const double cos_theta = cos(theta);
                const double sin_theta = sin(theta);
                const std::array<count, 3> roots = {
                    (count) ceil(next_multiplier * (next_1st_term - 2 * next_2nd_term_multiplier * cos_theta)),
                    (count) ceil(next_multiplier * (next_1st_term + next_2nd_term_multiplier * (cos_theta + sqrt(3) * sin_theta))),
                    (count) ceil(next_multiplier * (next_1st_term + next_2nd_term_multiplier * (cos_theta - sqrt(3) * sin_theta)))};
                sampleSize = *(std::max_element(roots.begin(), roots.end()));
                for (auto root : roots) {
                    if (root <= toSample || root == sampleSize) {
//                        TRACE("NEXT, root: ", root, " skipping as non informative");
                        continue;
                    }
                    const double alpha = mylog / (mylog + sqrt(mylog * (2 * root * minimizationResults.first + mylog)));
                    const double first_term = sqrt(mylog / (2 * root));
                    const double second_term = minimizationResults.first / (1 - alpha);
                    const double third_term = mylog / (2 * root * alpha * (1 - alpha));
                    const double next_deviation = first_term + second_term + third_term;
 //                   TRACE("NEXT, root: ", root, ", alpha: ",
 //                           alpha, ", first_term: ", first_term,
 //                           ", second_term: ", second_term, ", third_term: ",
 //                           third_term, ", deviation: ", next_deviation);
                    if (next_deviation <= epsilon && root < sampleSize) {
//                        TRACE("Smaller next sample size: ", root, " (old: ", sampleSize, ")");
                        sampleSize = root;
                    }
                }
//                TRACE("next sample size: ", sampleSize);
                assert(sampleSize > toSample);
            } else {
                sampleSize = ceil(sampleSize * sampleScheduleMultiplier);
 //               TRACE("next sample size: ", sampleSize);
            }
            toSample = sampleSize - toSample;
        }
        stopping_condition_time +=
            std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()
                    - stopping_condition_start_time).count();
    } while (supDeviationBound > epsilon);

    total_time =
        std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()
                - total_start_time).count();
//    TRACE("sampling_time=", sampling_time,
//            ", stopping_condition_time=", stopping_condition_time,
//            ", total_time=", total_time, ", iterations=", iterations);
    hasRun = true;
}


count ApproxBetweennessRade::numberOfSamples(void) {
//    DEBUG("Number of samples used ", sampleSize);
    return sampleSize;
}


void ApproxBetweennessRade::getStats(unsigned long *stats) {
    stats[0] = sampling_time;
    stats[1] = stopping_condition_time;
    stats[2] = total_time;
    stats[3] = iterations;
}


} /* namespace NetworKit */
