/*
 * RadeAux.h
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
#ifndef _RADE_H
#define _RADE_H

#include "../Globals.h"
#include <vector>

namespace NetworKit {
double getSupDeviationBound(const double, const count, const double, std::vector<double> &, std::pair<double,double> &);

}
#endif /* _RADE_H_ */
