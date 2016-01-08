/*
 * spiral_stc.cpp
 *
 *  Created on: Sep 15, 2015
 *      Author: manhnh
 */

#include "../../../include/plans/spiral_stc/spiral_stc.hpp"
#include "../../../include/global.hpp"

namespace wandrian {
namespace plans {
namespace spiral_stc {

SpiralStc::SpiralStc() :
    robot_size(0) {
}

SpiralStc::~SpiralStc() {
}

void SpiralStc::initialize(PointPtr starting_point, double robot_size) {
  Global::get_instance()->robot_size = robot_size;
  this->robot_size = robot_size;
  // Initialize starting_cell
  starting_cell = CellPtr(
      new Cell(
          PointPtr(
              new Point(starting_point->x - robot_size / 2,
                  starting_point->y + robot_size / 2)), 2 * robot_size));
  starting_cell->set_parent(
      CellPtr(
          new Cell(
              PointPtr(
                  new Point(starting_cell->get_center()->x,
                      starting_cell->get_center()->y - 2 * robot_size)),
              2 * robot_size)));
  path.insert(path.end(), starting_point);
}

void SpiralStc::cover() {
  Global::get_instance()->old_cells.insert(starting_cell);

  Global::get_instance()->read_message();
  std::string message = Global::get_instance()->create_message_from_old_cells();
  Global::get_instance()->write_message(message);

  scan(starting_cell);
}

void SpiralStc::set_behavior_see_obstacle(
    boost::function<bool(VectorPtr, double)> behavior_see_obstacle) {
  this->behavior_see_obstacle = behavior_see_obstacle;
}

bool SpiralStc::go_to(PointPtr position, bool flexibly) {
  std::cout << "    pos: " << position->x << "," << position->y;
  path.insert(path.end(), position);
  if (behavior_go_to)
    return behavior_go_to(position, flexibly);
  return true;
}

bool SpiralStc::see_obstacle(VectorPtr orientation, double distance) {
  bool get_obstacle;
  if (behavior_see_obstacle)
    get_obstacle = behavior_see_obstacle(orientation, distance);
  else
    get_obstacle = false;
  if (get_obstacle)
    std::cout << " \033[1;46m(OBSTACLE)\033[0m\n";
  return get_obstacle;
}

State SpiralStc::state_of(CellPtr cell) {
  State state =
      (Global::get_instance()->old_cells.find(cell)
          != Global::get_instance()->old_cells.end()) ? OLD : NEW;
  if (state == OLD)
    std::cout << " \033[1;45m(OLD)\033[0m\n";
  return state;
}

void SpiralStc::scan(CellPtr current) {
  Global::get_instance()->read_message();

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

    Global::get_instance()->read_message();

    if (state_of(neighbor) == OLD) { // Old cell
      // Go to next sub-cell
      go_with(++orientation, robot_size);
      continue;
    }
    if (see_obstacle(orientation, robot_size / 2)) { // Obstacle
      // Go to next sub-cell
      go_with(++orientation, robot_size);
    } else { // New free neighbor
      std::cout << "\n";
      // Construct a spanning-tree edge
      neighbor->set_parent(current);
      go_with(orientation++, robot_size);

      Global::get_instance()->read_message();
      Global::get_instance()->old_cells.insert(neighbor);

      std::string message = Global::get_instance()->create_message_from_old_cells();
      Global::get_instance()->write_message(message);

      scan(neighbor);
    }
  } while (orientation % initial_orientation
      != (is_starting_cell ? AT_RIGHT_SIDE : BEHIND));
  // Back to sub-cell of parent
  if (!is_starting_cell) {
    go_with(orientation, robot_size);
  }
  std::cout << "\033[1;34mcurrent-\033[0m\033[1;31mEND:\033[0m "
      << current->get_center()->x << "," << current->get_center()->y << "\n";
}

bool SpiralStc::go_with(VectorPtr orientation, double distance) {
  PointPtr last_position = *(--path.end());
  PointPtr new_position = last_position + orientation * distance;
  bool succeed = go_to(new_position, STRICTLY);
  std::cout << "\n";
  return succeed;
}

}
}
}
