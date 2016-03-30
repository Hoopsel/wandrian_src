/*
 * partially_occupiable_identifiable_cell.hpp
 *
 *  Created on: Mar 11, 2016
 *      Author: cslab
 */

#ifndef WANDRIAN_INCLUDE_ENVIRONMENT_MSTC_ONLINE_PARTIALLY_OCCUPIABLE_IDENTIFIABLE_CELL_HPP_
#define WANDRIAN_INCLUDE_ENVIRONMENT_MSTC_ONLINE_PARTIALLY_OCCUPIABLE_IDENTIFIABLE_CELL_HPP_

#include "../../environment/mstc_online/identifiable_cell.hpp"
#include "../../environment/partially_occupiable.hpp"

namespace wandrian {
namespace environment {
namespace mstc_online {

class PartiallyOccupiableIdentifiableCell: public IdentifiableCell,
    public PartiallyOccupiable {

public:
  PartiallyOccupiableIdentifiableCell(PointPtr, double, std::string);
  ~PartiallyOccupiableIdentifiableCell();

  PointPtr _center();
  double _size();
};

typedef boost::shared_ptr<PartiallyOccupiableIdentifiableCell> PartiallyOccupiableIdentifiableCellPtr;
typedef boost::shared_ptr<PartiallyOccupiableIdentifiableCell const> PartiallyOccupiableIdentifiableCellConstPtr;

}
}
}

#endif /* WANDRIAN_INCLUDE_ENVIRONMENT_MSTC_ONLINE_PARTIALLY_OCCUPIABLE_IDENTIFIABLE_CELL_HPP_ */
