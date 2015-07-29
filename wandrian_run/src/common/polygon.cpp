/*
 * polygon.cpp
 *
 *  Created on: Jun 23, 2015
 *      Author: sontd
 */

#include <boost/next_prior.hpp>
#include "../../include/common/segment.hpp"
#include "../../include/common/polygon.hpp"

namespace common {

Polygon::Polygon(std::set<Point*> points) :
		points(points), graph() {
	std::set<Point*>::iterator point = this->points.begin();
	while (point != this->points.end()) {
		std::set<Point*>::iterator next =
				boost::next(point) != this->points.end() ?
						boost::next(point) : this->points.begin();
		if (**next == **point) {
			if (point != this->points.end())
				this->points.erase(next);
			else
				this->points.erase(point);
		} else
			point++;
	}
	build();
}

Polygon::~Polygon() {
	for (std::set<Point*>::iterator p = points.begin(); p != points.end(); p++) {
		delete *p;
	}
	for (std::map<Point*, std::set<Point*, PointComp> >::iterator n =
			graph.begin(); n != graph.end(); n++) {
		delete (*n).first;
		for (std::set<Point*>::iterator p = (*n).second.begin();
				p != (*n).second.end(); p++) {
			delete *p;
		}
	}
}

std::set<Point*> Polygon::upper_vertices() {
	return get_vertices(true);
}

std::set<Point*> Polygon::lower_vertices() {
	return get_vertices(false);
}

std::map<Point*, std::set<Point*, PointComp>, PointComp> Polygon::get_graph() {
	return graph;
}

void Polygon::build() {
	for (std::set<Point*>::iterator current = points.begin();
			current != points.end(); current++) {
		// Insert current point into graph if not yet
		if (graph.find(*current) == graph.end())
			graph.insert(
					std::pair<Point*, std::set<Point*, PointComp> >(*current,
							std::set<Point*, PointComp>()));
		// Find next point
		std::set<Point*>::iterator next;
		if (boost::next(current) != points.end())
			next = boost::next(current);
		else
			next = points.begin();
		// Insert next point into graph if not yet
		if (graph.find(*next) == graph.end())
			graph.insert(
					std::pair<Point*, std::set<Point*, PointComp> >(*next,
							std::set<Point*, PointComp>()));
		// Create edge
		if (current != next) {
			graph.find(*current)->second.insert(*next);
			graph.find(*next)->second.insert(*current);
		}
	}
	// Find all intersects
	std::map<Segment*, std::set<Point*, PointComp>, SegmentComp> segments;
	for (std::map<Point*, std::set<Point*, PointComp> >::iterator current =
			graph.begin(); current != graph.end(); current++) {
		for (std::set<Point*>::iterator current_adjacent = current->second.begin();
				current_adjacent != current->second.end(); current_adjacent++) {
			for (std::map<Point*, std::set<Point*, PointComp> >::iterator another =
					boost::next(current); another != graph.end(); another++) {
				for (std::set<Point*>::iterator another_adjacent =
						another->second.begin(); another_adjacent != another->second.end();
						another_adjacent++) {
					Segment *current_segment = new Segment(current->first,
							*current_adjacent);
					Segment *another_segment = new Segment(another->first,
							*another_adjacent);
					Point *intersect = *(current_segment) % *(another_segment);
					if (intersect != NULL) {
						if (segments.find(current_segment) == segments.end())
							segments.insert(
									std::pair<Segment*, std::set<Point*, PointComp> >(
											current_segment, std::set<Point*, PointComp>()));
						if (segments.find(another_segment) == segments.end())
							segments.insert(
									std::pair<Segment*, std::set<Point*, PointComp> >(
											another_segment, std::set<Point*, PointComp>()));

						if (*intersect != *(current->first)
								&& *intersect != **current_adjacent)
							segments.find(current_segment)->second.insert(intersect);
						if (*intersect != *(another->first)
								&& *intersect != **another_adjacent)
							segments.find(another_segment)->second.insert(intersect);
					}
				}
			}
		}
	}
	// Insert intersects and new edges into graph
	for (std::map<Segment*, std::set<Point*, PointComp> >::iterator segment =
			segments.begin(); segment != segments.end(); segment++) {
		for (std::set<Point*>::iterator intersect = segment->second.begin();
				intersect != segment->second.end(); intersect++) {
			graph.find(segment->first->p1)->second.insert(*intersect);
			graph.find(segment->first->p2)->second.insert(*intersect);
			if (graph.find(*intersect) == graph.end())
				graph.insert(
						std::pair<Point*, std::set<Point*, PointComp> >(*intersect,
								std::set<Point*, PointComp>()));
			graph.find(*intersect)->second.insert(segment->first->p1);
			graph.find(*intersect)->second.insert(segment->first->p2);
			for (std::set<Point*>::iterator another = segment->second.begin();
					another != segment->second.end(); another++) {
				if (**intersect != **another)
					graph.find(*intersect)->second.insert(*another);
			}
		}
	}
	// TODO Remove redundant edges
}

Point* Polygon::get_leftmost() {
	Point* leftmost = *(points.begin());
	for (std::set<Point*>::iterator current = boost::next(points.begin());
			current != points.end(); current++) {
		if (**current < *leftmost)
			leftmost = *current;
	}
	return leftmost;
}

Point* Polygon::get_rightmost() {
	Point* rightmost = *(points.begin());
	for (std::set<Point*>::iterator current = boost::next(points.begin());
			current != points.end(); current++) {
		if (**current > *rightmost)
			rightmost = *current;
	}
	return rightmost;
}

std::set<Point*> Polygon::get_vertices(bool getUpper) {
	std::set<Point*> vertices;
	Point *leftmost = get_leftmost();
	Point *rightmost = get_rightmost();
	vertices.insert(leftmost);

	// TODO Choose relevant epsilon value
	double EPS = 20 * std::numeric_limits<double>::epsilon();

	Point *current = leftmost;
	Point *previous = new Point(current->x - 1, current->y);
	while (*current != *rightmost) {
		double angle;
		double distance = std::numeric_limits<double>::infinity();
		if (getUpper)
			angle = 2 * M_PI;
		else
			angle = 0;
		Point *next;
		for (std::set<Point*>::iterator adjacent =
				graph.find(current)->second.begin();
				adjacent != graph.find(current)->second.end(); adjacent++) {
			double a = atan2(previous->y - current->y, previous->x - current->x)
					- atan2((*adjacent)->y - current->y, (*adjacent)->x - current->x);
			double d = sqrt(
					pow(current->x - (*adjacent)->x, 2)
							+ pow(current->y - (*adjacent)->y, 2));
			if (getUpper) {
				a = std::abs(a) <= EPS ? 2 * M_PI : a > 0 ? a : 2 * M_PI + a;
				if (a - angle < -EPS || (std::abs(a - angle) < EPS && d < distance)) {
					angle = a;
					distance = d;
					next = new Point(**adjacent);
				}
			} else {
				a = std::abs(a) <= EPS ? 0 : a > 0 ? a : 2 * M_PI + a;
				if (a - angle > EPS || (std::abs(a - angle) < EPS && d < distance)) {
					angle = a;
					distance = d;
					next = new Point(**adjacent);
				}
			}
		}
		previous = new Point(*current);
		current = new Point(*next);
		vertices.insert(current);
	}
	return vertices;
}

}
