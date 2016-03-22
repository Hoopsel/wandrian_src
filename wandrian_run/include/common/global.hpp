/*
 * global.hpp
 *
 *  Created on: Mar 22, 2016
 *      Author: cslab
 */

#ifndef WANDRIAN_RUN_INCLUDE_COMMON_GLOBAL_HPP_
#define WANDRIAN_RUN_INCLUDE_COMMON_GLOBAL_HPP_

#include <limits>

// TODO: Choose relevant epsilon value
const double EPSILON = 20 * std::numeric_limits<double>::epsilon();
const double SMALL_EPSILON = std::numeric_limits<double>::epsilon();

#endif /* WANDRIAN_RUN_INCLUDE_COMMON_GLOBAL_HPP_ */
