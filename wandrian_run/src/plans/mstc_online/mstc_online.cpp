/*
 * spiral_stc.cpp
 *
 *  Created on: Sep 15, 2015
 *      Author: manhnh
 */

#include <algorithm>
#include "../../../include/plans/mstc_online/mstc_online.hpp"

namespace wandrian {
namespace plans {
namespace mstc_online {

MstcOnline::MstcOnline() :
    tool_size(0) {
}

MstcOnline::~MstcOnline() {
}

void MstcOnline::initialize(PointPtr starting_point, double tool_size,
    CommunicatorPtr communicator) {
  communicator->set_tool_size(tool_size);
  this->tool_size = tool_size;
  this->communicator = communicator;
  // Initialize starting_cell
  starting_cell = CellPtr(
      new Cell(
          PointPtr(
              new Point(starting_point->x - tool_size / 2,
                  starting_point->y + tool_size / 2)), 2 * tool_size,
          communicator->get_robot_name()));
  starting_cell->set_parent(
      CellPtr(
          new Cell(
              PointPtr(
                  new Point(starting_cell->get_center()->x,
                      starting_cell->get_center()->y - 2 * tool_size)),
              2 * tool_size, communicator->get_robot_name())));
  path.insert(path.end(), starting_point);
}

void MstcOnline::cover() {
  communicator->read_message_then_update_old_cells();
  communicator->insert_old_cell(starting_cell);
  std::string message = communicator->create_old_cells_message();
  std::string status = communicator->create_status_message(starting_cell);
  communicator->write_old_cells_message(message);
  communicator->write_status_message(status);
  scan(starting_cell);
}

void MstcOnline::set_behavior_see_obstacle(
    boost::function<bool(VectorPtr, double)> behavior_see_obstacle) {
  this->behavior_see_obstacle = behavior_see_obstacle;
}

bool MstcOnline::go_to(PointPtr position, bool flexibly) {
  std::cout << "    pos: " << position->x << "," << position->y;
  path.insert(path.end(), position);
  if (behavior_go_to)
    return behavior_go_to(position, flexibly);
  return true;
}

bool MstcOnline::see_obstacle(VectorPtr orientation, double distance) {
  bool get_obstacle;
  if (behavior_see_obstacle)
    get_obstacle = behavior_see_obstacle(orientation, distance);
  else
    get_obstacle = false;
  if (get_obstacle)
    std::cout << " \033[1;46m(OBSTACLE)\033[0m\n";
  return get_obstacle;
}

State MstcOnline::state_of(CellPtr cell) {
  State state = (communicator->find_old_cell(cell)) ? OLD : NEW;
  if (state == OLD)
    std::cout << " \033[1;45m(OLD)\033[0m\n";
  return state;
}

void MstcOnline::scan(CellPtr current) {
  std::string status;
  status = communicator->create_status_message(current);
  communicator->write_status_message(status);
  communicator->read_message_then_update_old_cells();
  std::cout << "\033[1;34mcurrent-\033[0m\033[1;32mBEGIN:\033[0m "
      << current->get_center()->x << "," << current->get_center()->y << "\n";
  VectorPtr orientation = (current->get_parent()->get_center()
      - current->get_center()) / 2 / tool_size;
  VectorPtr initial_orientation = orientation++;
  // While current cell has a new obstacle-free neighboring cell
  bool is_starting_cell = current == starting_cell;
  do {
    // Scan for new neighbor of current cell in counterclockwise order
    CellPtr neighbor = CellPtr(
        new Cell(current->get_center() + orientation * 2 * tool_size,
            2 * tool_size, communicator->get_robot_name()));
    std::cout << "  \033[1;33mneighbor:\033[0m " << neighbor->get_center()->x
        << "," << neighbor->get_center()->y;
    communicator->read_message_then_update_old_cells();
    if (state_of(neighbor) == OLD) { // Old cell
      // Go to next sub-cell
      if (communicator->ask_other_robot_still_alive(
          communicator->find_robot_name(neighbor))) {
        // Still alive
        go_with(++orientation, tool_size);
        continue;
      } else {
        std::cout << "\n";
        neighbor->set_parent(current);
        communicator->read_message_then_update_old_cells();
        communicator->insert_old_cell(neighbor);
        std::string message = communicator->create_old_cells_message();
        communicator->write_old_cells_message(message);
        go_with(orientation++, tool_size);
        scan(neighbor);
        continue;
      }
    }
    if (see_obstacle(orientation, tool_size / 2)) { // Obstacle
      // Go to next sub-cell
      go_with(++orientation, tool_size);
    } else { // New free neighbor
      std::cout << "\n";
      // Construct a spanning-tree edge
      neighbor->set_parent(current);
      communicator->read_message_then_update_old_cells();
      communicator->insert_old_cell(neighbor);
      std::string message = communicator->create_old_cells_message();
      communicator->write_old_cells_message(message);
      go_with(orientation++, tool_size);
      scan(neighbor);
    }
  } while (orientation % initial_orientation
      != (is_starting_cell ? AT_RIGHT_SIDE : IN_BACK));
  // Back to sub-cell of parent
  if (!is_starting_cell) {
    go_with(orientation, tool_size);
  }
  std::cout << "\033[1;34mcurrent-\033[0m\033[1;31mEND:\033[0m "
      << current->get_center()->x << "," << current->get_center()->y << "\n";
}

bool MstcOnline::go_with(VectorPtr orientation, double distance) {
  PointPtr last_position = *(--path.end());
  PointPtr new_position = last_position + orientation * distance;
  bool succeed = go_to(new_position, STRICTLY);
  std::cout << "\n";
  return succeed;
}

}
}
}
