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
namespace shapes {

Polygon::Polygon(std::set<Point*> points) :
		points(points) {
	build();
}

Polygon::~Polygon() {
	for (std::set<Point*>::iterator p = points.begin(); p != points.end();
			p++) {
		delete *p;
	}
	for (std::map<Point*, std::set<Point*> >::iterator n = graph.begin();
			n != graph.end(); n++) {
		delete (*n).first;
		for (std::set<Point*>::iterator p = (*n).second.begin();
				p != (*n).second.end(); p++) {
			delete *p;
		}
	}
}

void Polygon::add(Point* point) {
	points.insert(point);
	build();
}

std::set<Point*> Polygon::upper_vertices() {
	std::set<Point*> vertices;

	return vertices;
}

std::set<Point*> Polygon::lower_vertices() {
	std::set<Point*> vertices;

	return vertices;
}

void Polygon::build() {
	for (std::set<Point*>::iterator current = points.begin();
			current != points.end(); current++) {
		// Insert current point into graph if not yet
		if (graph.find(*current) == graph.end())
			graph.insert(
					std::pair<Point*, std::set<Point*> >(*current,
							std::set<Point*>()));
		// Find next point
		std::set<Point*>::iterator next;
		if (current != points.end())
			next = boost::next(current);
		else
			next = points.begin();
		// Insert next point into graph if not yet
		if (graph.find(*next) == graph.end())
			graph.insert(
					std::pair<Point*, std::set<Point*> >(*next,
							std::set<Point*>()));
		graph.find(*current)->second.insert(*next);
		graph.find(*next)->second.insert(*current);
	}

	// Find all intersects
	for (std::map<Point*, std::set<Point*> >::iterator current = graph.begin();
			current != graph.end(); current++) {
		for (std::set<Point*>::iterator current_adjacent =
				current->second.begin();
				current_adjacent != current->second.end(); current_adjacent++) {
			for (std::map<Point*, std::set<Point*> >::iterator another =
					boost::next(current); another != graph.end(); another++) {
				for (std::set<Point*>::iterator another_adjacent =
						another->second.begin();
						another_adjacent != another->second.end();
						another_adjacent++) {
					Point *intersect = *(new Segment(current->first,
							*current_adjacent))
							% *(new Segment(another->first, *another_adjacent));
					if (intersect != NULL) {
						if (graph.find(intersect) == graph.end())
							graph.insert(
									std::pair<Point*, std::set<Point*> >(
											intersect, std::set<Point*>()));
					}
				}
			}
		}
	}
}

}
}

