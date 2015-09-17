/*
 * spiral_stc.cpp
 *
 *  Created on: Sep 15, 2015
 *      Author: sontd
 */

#include "../../../include/plans/spiral_stc/spiral_stc.hpp"

#include <stdio.h>
#include <iostream>

namespace wandrian {
namespace plans {
namespace spiral_stc {

SpiralStc::SpiralStc(Environment *environment, Point *starting_point,
		const double sub_cell_size) :
		environment(environment), is_bumper_pressing(false), sub_cell_size(
				sub_cell_size), step_size(sub_cell_size / 2) {
	// Initialize starting_cell
	starting_cell = new Cell(
			new Point(starting_point->x - sub_cell_size / 2,
					starting_point->y + sub_cell_size / 2), 2 * sub_cell_size);
	starting_cell->set_parent(
			new Cell(
					new Point(starting_cell->get_center()->x,
							starting_cell->get_center()->y - 2 * sub_cell_size),
					2 * sub_cell_size));
	path.insert(path.end(), starting_point);
}

void SpiralStc::cover() {
	spiral_stc(starting_cell);
}

std::list<Point*> SpiralStc::get_path() {
	return path;
}

void SpiralStc::go(Vector2d *direction, int step) {
//	for (std::set<Point*>::iterator p = path.begin(); p != path.end(); p++) {
//		std::cout << (*p)->x << "," << (*p)->y << "; ";
//	}
//	std::cout << "\n";

	Point *last_position = *(--path.end());
	Point *new_position = new Point(
			*last_position + *direction * step * step_size);
	path.insert(path.end(), new_position);
	std::cout << "  p: " << new_position->x << "," << new_position->y << "; ("
			<< last_position->x << "," << last_position->y << "; " << direction->x
			<< "," << direction->y << "; " << step << ")\n";
//	getchar();

// Bumper event here
// TODO: correctly check (now temporarily)
	Cell *space = ((Cell*) environment->space);
	if (new_position->x >= space->get_center()->x + space->get_size() / 2
			|| new_position->x <= space->get_center()->x - space->get_size() / 2
			|| new_position->y >= space->get_center()->y + space->get_size() / 2
			|| new_position->y <= space->get_center()->y - space->get_size() / 2) {
		is_bumper_pressing = true;
		return;
	}
	for (std::set<Polygon*>::iterator o = environment->obstacles.begin();
			o != environment->obstacles.end(); o++) {
		Cell *obstacle = ((Cell*) *o);
		if (new_position->x <= obstacle->get_center()->x + obstacle->get_size() / 2
				&& new_position->x
						>= obstacle->get_center()->x - obstacle->get_size() / 2
				&& new_position->y
						<= obstacle->get_center()->y + obstacle->get_size() / 2
				&& new_position->y
						>= obstacle->get_center()->y - obstacle->get_size() / 2) {
			is_bumper_pressing = true;
			return;
		}
	}
}

void SpiralStc::spiral_stc(Cell *current) {
	std::cout << "current-BEGIN: " << current->get_center()->x << ","
			<< current->get_center()->y << "\n";
//	if (current->get_center()->x == -15)
//		getchar();
	// TODO: correctly compute starting direction
	Vector2d *direction = new Vector2d(
			(*(current->get_parent()->get_center()) - *(current->get_center())) / 2
					/ sub_cell_size);
	int neighbors_count = 0;
	// While current cell has a new obstacle-free neighboring cell
	while (neighbors_count < 3) {
		direction = direction->rotate_counterclockwise();
		// Scan for the first new neighbor of current cell in counterclockwise order
		Cell *neighbor = new Cell(
				new Point(*(current->get_center()) + *direction * 2 * sub_cell_size),
				2 * sub_cell_size);
		std::cout << "  neighbor: " << neighbor->get_center()->x << ","
				<< neighbor->get_center()->y;
		neighbors_count++;
		if (check(neighbor) == OLD_CELL) {
			std::cout << " (OLD)\n";
			// Go to next sub-cell
			go(direction->rotate_counterclockwise(), sub_cell_size / step_size);
			continue;
		} else {
			std::cout << "\n";
		}
		go(direction, sub_cell_size / 2 / step_size);
		if (is_bumper_pressing) {
			std::cout << "    (BUMP)\n";
			// Go back
			path.erase(--path.end());
			is_bumper_pressing = false;
			// Go to next sub-cell
			go(direction->rotate_counterclockwise(), sub_cell_size / step_size);
		} else {
			neighbor->set_parent(current);
			// Construct a spanning-tree edge
			current->neighbors.insert(neighbor);
			go(direction, sub_cell_size / 2 / step_size);
			spiral_stc(neighbor);
		}
	}
	// Back to sub-cell of parent
	if (*(current->get_center()) != *(starting_cell->get_center())) {
		direction = direction->rotate_counterclockwise();
		go(direction, sub_cell_size / step_size);
	}
	std::cout << "current-END: " << current->get_center()->x << ","
			<< current->get_center()->y << "\n";
}

bool SpiralStc::check(Cell *cell_to_check) {
	std::set<Cell*> list;
	list.insert(starting_cell);
	while (list.size() > 0) {
		Cell *cell = *(list.begin());
		if (*(cell->get_center()) == *(cell_to_check->get_center()))
			return OLD_CELL;
		list.erase(list.begin());
		for (std::set<Cell*>::iterator c = cell->neighbors.begin();
				c != cell->neighbors.end(); c++)
			list.insert(*c);
	}
	return NEW_CELL;
}

}
}
}
