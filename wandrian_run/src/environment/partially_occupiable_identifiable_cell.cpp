/*
 * partially_occupiable_identifiable_cell.cpp
 *
 *  Created on: Mar 11, 2016
 *      Author: cslab
 */

#include "../../include/environment/partially_occupiable_identifiable_cell.hpp"

namespace wandrian {
namespace environment {

PartiallyOccupiableIdentifiableCell::PartiallyOccupiableIdentifiableCell(
    PointPtr center, double size, std::string robot_name) :
    IdentifiableCell(center, size, robot_name) {
}

PartiallyOccupiableIdentifiableCell::~PartiallyOccupiableIdentifiableCell() {
}

PointPtr PartiallyOccupiableIdentifiableCell::get_current_position() {
  return current_position(center, size);
}

}
}
