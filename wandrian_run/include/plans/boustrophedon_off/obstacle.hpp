

#ifndef WANDRIAN_RUN_INCLUDE_PLANS_BOUSTROPHEDON_OBSTACLE_HPP_
#define WANDRIAN_RUN_INCLUDE_PLANS_BOUSTROPHEDON_OBSTACLE_HPP_

#include "../../common/polygon.hpp"

using namespace wandrian::common;

namespace wandrian {
namespace plans {
namespace boustrophedon_off {

#define OLD_Obstacle false
#define NEW_Obstacle true

class Obstacle: public Polygon {

public:

  Obstacle(PointPtr, double, double);
  PointPtr get_center();
  double get_sizex();
  double get_sizey();
private:
  PointPtr center;
  double sizex;
  double sizey;
};

typedef boost::shared_ptr<Obstacle const> ObstacleConstPtr;
typedef boost::shared_ptr<Obstacle> ObstaclePtr;

inline bool operator<(const Obstacle &c1, const Obstacle &c2) {
  // TODO Choose relevant epsilon value
  double EPS = 20 * std::numeric_limits<double>::epsilon();
  ObstaclePtr Obstacle1 = ObstaclePtr(new Obstacle(c1));
  ObstaclePtr Obstacle2 = ObstaclePtr(new Obstacle(c2));
  return
      std::abs(Obstacle1->get_center()->x - Obstacle2->get_center()->x) > EPS ?
          (Obstacle1->get_center()->x < Obstacle2->get_center()->x) :
          (Obstacle1->get_center()->y < Obstacle2->get_center()->y);
}

struct ObstacleComp {
  bool operator()(ObstacleConstPtr c1, ObstacleConstPtr c2) const {
    return *c1 < *c2;
  }
};

}
}
}

#endif /* WANDRIAN_RUN_INCLUDE_PLANS_BOUSTROPHEDON_Obstacle_HPP_ */
