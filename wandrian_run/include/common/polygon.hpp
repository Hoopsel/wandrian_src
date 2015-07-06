/*
 * polygon.hpp
 *
 *  Created on: Jun 23, 2015
 *      Author: sontd
 */

#ifndef WANDRIAN_RUN_INCLUDE_COMMON_POLYGON_HPP_
#define WANDRIAN_RUN_INCLUDE_COMMON_POLYGON_HPP_

#include <map>
#include <set>
#include "point.hpp"

namespace common {
namespace shapes {

class Polygon {

public:
	Polygon(std::set<Point*, PointComp> points);
	~Polygon();
	void add(Point* point);
	std::set<Point*> upper_vertices(); // list of points
	std::set<Point*> lower_vertices(); // list of points
	std::map<Point*, std::set<Point*, PointComp>, PointComp > get_graph();

private:
	std::set<Point*, PointComp> points;
	std::map<Point*, std::set<Point*, PointComp>, PointComp > graph;
	void build();
};
}
}

#endif /* WANDRIAN_RUN_INCLUDE_COMMON_POLYGON_HPP_ */
