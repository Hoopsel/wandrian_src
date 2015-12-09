/*
 * full_spiral_stc.cpp
 *
 *  Created on: Dec 3, 2015
 *      Author: cslab
 */

#include "../../../include/plans/spiral_stc/full_spiral_stc.hpp"

#define NORMAL true
#define ABNORMAL false

namespace wandrian {
namespace plans {
namespace spiral_stc {

FullSpiralStc::FullSpiralStc() {
}

FullSpiralStc::~FullSpiralStc() {
}

void FullSpiralStc::cover() {
  old_cells.insert(starting_cell);
  starting_cell->set_current_quadrant(IV);
  scan(starting_cell);
}

bool FullSpiralStc::check(CellPtr cell_to_check) {
  std::set<CellPtr, CellComp>::iterator cell = old_cells.find(cell_to_check);
  return (cell != old_cells.end()) ? OLD : NEW;
}

void FullSpiralStc::scan(CellPtr current) {
  std::cout << "\033[1;34mcurrent-\033[0m\033[1;32mBEGIN:\033[0m "
      << current->get_center()->x << "," << current->get_center()->y << "\n";
  VectorPtr orientation = (current->get_parent()->get_center()
      - current->get_center()) / 2 / robot_size;
  VectorPtr initial_orientation = orientation++;
  // While current cell has a new obstacle-free neighboring cell
  bool is_starting_cell = current == starting_cell;
  do {
    // Scan for new neighbor of current cell in counterclockwise order
    CellPtr neighbor = CellPtr(
        new Cell(current->get_center() + orientation * 2 * robot_size,
            2 * robot_size));
    std::cout << "  \033[1;33mneighbor:\033[0m " << neighbor->get_center()->x
        << "," << neighbor->get_center()->y;
    if (check(neighbor) == OLD) { // Old cell
      std::cout << " \033[1;45m(OLD)\033[0m\n";
      continue;
    } else {
      // Go with absolute orientation
      bool succeed = go_across(current, neighbor);
      if (!succeed) { // Obstacle
        std::cout << " \033[1;46m(OBSTACLE)\033[0m\n";
      } else { // New free neighbor
        std::cout << "\n";
        // Construct a spanning-tree edge
        neighbor->set_parent(current);
        old_cells.insert(neighbor);
        scan(neighbor);
      }
    }
  } while (orientation++ % initial_orientation
      != (is_starting_cell ? BEHIND : AT_LEFT_SIDE));
  // Back to sub-cell of parent (need to check sub-cell of parent is occupied or not)
  if (!is_starting_cell) {
    go_across(current, current->get_parent());
  }
  std::cout << "\033[1;34mcurrent-\033[0m\033[1;31mEND:\033[0m "
      << current->get_center()->x << "," << current->get_center()->y << "\n";
}

bool FullSpiralStc::go_across(CellPtr current, CellPtr next) {
  VectorPtr orientation = (next->get_center() - current->get_center())
      / (2 * robot_size);
  Quadrant quadrant = current->get_current_quadrant();
  Quadrant q4;
  Quadrant q1;
  Quadrant q2;
  Quadrant q3;
  Quadrant q;
  switch (~orientation) {
  case AT_RIGHT_SIDE:
    q = IV;
    break;
  case IN_FRONT:
    q = I;
    break;
  case AT_LEFT_SIDE:
    q = II;
    break;
  case BEHIND:
    q = III;
    break;
  }
  q4 = q;
  q1 = ++q;
  q2 = ++q;
  q3 = ++q;
  if (quadrant == q4 || quadrant == q1) {
    if (!see_obstacle(orientation, robot_size / 2)) {
      return visit(next, quadrant == q4 ? q3 : q2, STRICTLY);
    } else {
      if (!see_obstacle(quadrant == q4 ? ++orientation : --orientation,
          robot_size / 2)) {
        visit(current, quadrant == q4 ? q1 : q4, STRICTLY);
        if (!see_obstacle(quadrant == q4 ? --orientation : ++orientation,
            robot_size / 2))
          return visit(next, quadrant == q4 ? q2 : q3, STRICTLY);
        else
          return false;
      } else
        return false;
    }
  } else if (quadrant == q3 || quadrant == q2) {
    if (!see_obstacle(orientation, robot_size / 2)) {
      visit(current, quadrant == q3 ? q4 : q1, STRICTLY);
      return go_across(current, next);
    } else {
      if (!see_obstacle(quadrant == q3 ? ++orientation : --orientation,
          robot_size / 2)) {
        visit(current, quadrant == q3 ? q2 : q3, STRICTLY);
        if (!see_obstacle(quadrant == q3 ? --orientation : ++orientation,
            robot_size / 2)) {
          visit(current, quadrant == q3 ? q1 : q4, STRICTLY);
          return go_across(current, next);
        } else
          return false;
      } else
        return false;
    }
  } else
    return false;
}

bool FullSpiralStc::visit(CellPtr cell, Quadrant quadrant, bool flexibly) {
  cell->set_current_quadrant(quadrant);
  return go_to(cell->get_current_position(), flexibly);
}

}
}
}
