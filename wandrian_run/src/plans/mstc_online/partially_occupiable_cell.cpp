/*
 * partially_occupiable_cell.cpp
 *
 *  Created on: Dec 12, 2015
 *      Author: cslab
 */

#include "../../../include/plans/mstc_online/partially_occupiable_cell.hpp"

namespace wandrian {
namespace plans {
namespace mstc_online {

PartiallyOccupiableCell::PartiallyOccupiableCell(PointPtr center, double size,
    std::string robot_name) :
    Cell(center, size, robot_name), current_quadrant(IV) {
  for (int i = I; i <= IV; i++)
    quadrants[i] = NEW;
}

PartiallyOccupiableCell::~PartiallyOccupiableCell() {
}

PointPtr PartiallyOccupiableCell::get_current_position() {
  switch (current_quadrant) {
  case I:
    return PointPtr(new Point(center->x + size / 4, center->y + size / 4));
  case II:
    return PointPtr(new Point(center->x - size / 4, center->y + size / 4));
  case III:
    return PointPtr(new Point(center->x - size / 4, center->y - size / 4));
  case IV:
    return PointPtr(new Point(center->x + size / 4, center->y - size / 4));
  default:
    return PointPtr(new Point(center->x, center->y));
  }
}

Quadrant PartiallyOccupiableCell::get_current_quadrant() {
  return current_quadrant;
}

State* PartiallyOccupiableCell::get_quadrants() {
  return quadrants;
}

void PartiallyOccupiableCell::set_current_quadrant(Quadrant quadrant) {
  current_quadrant = quadrant;
  set_quadrants_state(quadrant, OLD);
}

void PartiallyOccupiableCell::set_quadrants_state(Quadrant quadrant,
    State state) {
  quadrants[quadrant] = state;
}

}
}
}
