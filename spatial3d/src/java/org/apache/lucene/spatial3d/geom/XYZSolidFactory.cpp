using namespace std;

#include "XYZSolidFactory.h"

namespace org::apache::lucene::spatial3d::geom
{

XYZSolidFactory::XYZSolidFactory() {}

shared_ptr<XYZSolid> XYZSolidFactory::makeXYZSolid(
    shared_ptr<PlanetModel> planetModel, double const minX, double const maxX,
    double const minY, double const maxY, double const minZ, double const maxZ)
{
  if (abs(maxX - minX) < Vector::MINIMUM_RESOLUTION) {
    if (abs(maxY - minY) < Vector::MINIMUM_RESOLUTION) {
      if (abs(maxZ - minZ) < Vector::MINIMUM_RESOLUTION) {
        return make_shared<dXdYdZSolid>(planetModel, (minX + maxX) * 0.5,
                                        (minY + maxY) * 0.5, minZ);
      } else {
        return make_shared<dXdYZSolid>(planetModel, (minX + maxX) * 0.5,
                                       (minY + maxY) * 0.5, minZ, maxZ);
      }
    } else {
      if (abs(maxZ - minZ) < Vector::MINIMUM_RESOLUTION) {
        return make_shared<dXYdZSolid>(planetModel, (minX + maxX) * 0.5, minY,
                                       maxY, (minZ + maxZ) * 0.5);
      } else {
        return make_shared<dXYZSolid>(planetModel, (minX + maxX) * 0.5, minY,
                                      maxY, minZ, maxZ);
      }
    }
  }
  if (abs(maxY - minY) < Vector::MINIMUM_RESOLUTION) {
    if (abs(maxZ - minZ) < Vector::MINIMUM_RESOLUTION) {
      return make_shared<XdYdZSolid>(planetModel, minX, maxX,
                                     (minY + maxY) * 0.5, (minZ + maxZ) * 0.5);
    } else {
      return make_shared<XdYZSolid>(planetModel, minX, maxX,
                                    (minY + maxY) * 0.5, minZ, maxZ);
    }
  }
  if (abs(maxZ - minZ) < Vector::MINIMUM_RESOLUTION) {
    return make_shared<XYdZSolid>(planetModel, minX, maxX, minY, maxY,
                                  (minZ + maxZ) * 0.5);
  }
  return make_shared<StandardXYZSolid>(planetModel, minX, maxX, minY, maxY,
                                       minZ, maxZ);
}

shared_ptr<XYZSolid>
XYZSolidFactory::makeXYZSolid(shared_ptr<PlanetModel> planetModel,
                              shared_ptr<XYZBounds> bounds)
{
  return makeXYZSolid(planetModel, bounds->getMinimumX(), bounds->getMaximumX(),
                      bounds->getMinimumY(), bounds->getMaximumY(),
                      bounds->getMinimumZ(), bounds->getMaximumZ());
}
} // namespace org::apache::lucene::spatial3d::geom