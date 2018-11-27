/*
 * RadeAux.cpp
 *
 *  Created on: 26.06.2015
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


#include "RadeAux.h"
#include "../auxiliary/Log.h"
#include "../Globals.h"

#include <cmath>
#include <limits>
#include <vector>

//#include <nlopt.h>
#include <nlopt.hpp>

namespace NetworKit {

class RunningLogSum {
    private:
        double value;
        // Given ln(a) and ln(b), compute ln(a+b)
        static inline double logsum(const double log_a, const double log_b) {
            double to_return;
            if (log_a <= std::numeric_limits<double>::lowest()) {
                to_return = log_b;
            } else if (log_b <= std::numeric_limits<double>::lowest()) {
                to_return = log_a;
            } else {
                to_return = std::fmax(log_a, log_b) + log1p(exp(std::fmin(log_a, log_b) - std::fmax(log_a, log_b)));
#ifdef DEBUG
                assert(to_return > std::fmax(log_a, log_b) || to_return < nexttoward(std::fmax(log_a,log_b), std::numeric_limits<double>::max()));
#endif
            }
            return to_return;
        }

    public:
        RunningLogSum(): value(std::numeric_limits<double>::lowest()) {}
        RunningLogSum(const double init): value(init) {}

        RunningLogSum& operator+=(const RunningLogSum& rhs) {
            this->value = logsum(this->value, rhs.value);
            return *this;
        }

        friend RunningLogSum operator+(RunningLogSum lhs, RunningLogSum& rhs) {
            lhs += rhs;
            return lhs;
        }

        RunningLogSum& operator+=(const double rhs) {
            this->value = logsum(this->value, rhs);
            return *this;
        }

        double getValue() { return value; }
};


// Compute the objective function (to be optimized) and the gradient at x[0]
double objectiveFunction(const std::vector<double> &x, std::vector<double> &grad, void* f_data) {
    std::vector<double> *sum_exponents_p = (std::vector<double> *) f_data;
    const double s_square = x[0] * x[0];
    #pragma omp declare reduction (runningLogSumReduction: RunningLogSum: omp_out += omp_in)
    RunningLogSum runningLogSum((*sum_exponents_p)[0]);
    RunningLogSum runningLogSumForGrad2ndTerm(s_square * (*sum_exponents_p)[0] + log((*sum_exponents_p)[0]));
    #pragma omp parallel for reduction(runningLogSumReduction:runningLogSum,runningLogSumForGrad2ndTerm)
    for (index i = 1; i < (*sum_exponents_p).size(); ++i) {
        double s_expo = s_square * (*sum_exponents_p)[i];
        runningLogSum += s_expo;
        if (grad.size() > 0) {
            runningLogSumForGrad2ndTerm += s_expo + log((*sum_exponents_p)[i]);
        }
    }
    double logSum = runningLogSum.getValue();
    if (grad.size() > 0) {
        grad[0] = 2 * exp(runningLogSumForGrad2ndTerm.getValue() - logSum) - logSum / (x[0] * x[0]);
//        TRACE("x[0]=", x[0], ", logSum=", logSum, ", value=", logSum / x[0],
//                ", grad[0]=", grad[0], ", logSumForGrad2ndTerm=",
//                runningLogSumForGrad2ndTerm.getValue());
    } else {
//        TRACE("x[0]=", x[0], ", logSum=", logSum, ", value=", logSum / x[0]);
    }
    return logSum / x[0];
}


inline void getEmpiricalRademacherAverageBound(std::vector<double> &objectiveFunctionSumExponents, std::pair<double,double> &minimizationResults) {
    // Setup and solve the optimization problem
    // We are using a *local* optimization algorithm because the function is
    // convex.
    // TODO MR We should try and compare for performance other algorithms, e.g.,
    // LD_LBFGS, and even a derivative-free algorithm like
    // LN_COBYLA, which we use in progrsamplfi. More info at
    // http://ab-initio.mit.edu/wiki/index.php/NLopt_Algorithms
    // BTW, for some reasons, LD_TNEWTON_PRECOND does not seem to work (we get
    // an NLopt failure), and LD_MMA terminates immediately, claiming that the
    // optimal value is at the initialization point.
    nlopt::opt optProb(nlopt::LD_SLSQP, 1u);
    optProb.set_min_objective(objectiveFunction, &objectiveFunctionSumExponents);
    std::vector<double> lb({std::numeric_limits<double>::min()});
    optProb.set_lower_bounds(lb);
    optProb.set_xtol_abs(1e-4);
    optProb.set_ftol_abs(1e-6);
    // Set initialization point
    std::vector<double> x(1, (double) minimizationResults.second);
    nlopt::result optResult = optProb.optimize(x, minimizationResults.first);
    if (optResult >= 0) {
        minimizationResults.second = x[0];
//        DEBUG("Optimization done, optimum value=", minimizationResults.first,
//                " for s=", minimizationResults.second);
    } else  {
        // TODO MR FIXME There must something better to do here.
//        FATAL("Optimization failed! return code: " , optResult);
        std::exit(1);
    }
}


double getSupDeviationBoundVanilla(const double delta, const count sampleSize, const double penaltyFactor, std::vector<double> &objectiveFunctionSumExponents, std::pair<double,double> &minimizationResults, const bool doMinimization = true) {
//    DEBUG("Computing Vanilla SupDeviationBound");
    if (doMinimization) {
        getEmpiricalRademacherAverageBound(objectiveFunctionSumExponents, minimizationResults);
    }
//    DEBUG("Sup Deviation Bound, first term: ", 2 * minimizationResults.first, ", second term: ", 3 * penaltyFactor * sqrt(log(2 / delta) / (2 * sampleSize)));
    return 2 * minimizationResults.first + 3 * penaltyFactor * sqrt(log(2 / delta) / (2 * sampleSize));
}


double getSupDeviationBoundRefined(const double delta, const count sampleSize, const double penaltyFactor, std::vector<double> &objectiveFunctionSumExponents, std::pair<double,double> &minimizationResults, const bool doMinimization = false) {
//    DEBUG("Computing Refined SupDeviationBound");
    if (doMinimization) {
        getEmpiricalRademacherAverageBound(objectiveFunctionSumExponents, minimizationResults);
    }
    const double logTwoOverDelta = log(2.0 / delta);
    const double twoTimesSampleSize = 2 * sampleSize;
    const double alpha = minimizationResults.first > 0 ? logTwoOverDelta / (logTwoOverDelta + sqrt(logTwoOverDelta * (twoTimesSampleSize * minimizationResults.first + logTwoOverDelta))) : 0.5;
//    TRACE("radeBound: ", minimizationResults.first , ", logTwoOverDelta: ", logTwoOverDelta, ", twoTimesSampleSize: ", twoTimesSampleSize, ", alpha: ", alpha);
//    DEBUG("Sup Deviation Bound, first term: ", (twoTimesSampleSize * alpha * minimizationResults.first + logTwoOverDelta) /
//        (twoTimesSampleSize * alpha * (1 - alpha)), ", second term: ", sqrt(logTwoOverDelta / twoTimesSampleSize));
    return (twoTimesSampleSize * alpha * minimizationResults.first + logTwoOverDelta) /
        (twoTimesSampleSize * alpha * (1 - alpha)) + sqrt(logTwoOverDelta / twoTimesSampleSize);
}


double getSupDeviationBound(const double delta, const count sampleSize, const double penaltyFactor, std::vector<double> &objectiveFunctionSumExponents, std::pair<double,double> &minimizationResults) {
    double vanilla = getSupDeviationBoundVanilla(delta, sampleSize,
            penaltyFactor, objectiveFunctionSumExponents, minimizationResults);
    double refined = getSupDeviationBoundRefined(delta, sampleSize,
            penaltyFactor, objectiveFunctionSumExponents, minimizationResults);
    double smaller = 0;
    if (vanilla < refined) {
        smaller = vanilla;
//        DEBUG("Vanilla is better");
    } else if (vanilla > refined) {
        smaller = refined;
//        DEBUG("Refined is better");
    } else {
//        DEBUG("Vanilla and Refined are equal");
        smaller = refined;
    }
    return smaller;
}

} /* namespace NetworKit */
