using namespace std;

#include "GeoBBoxFactory.h"

namespace org::apache::lucene::spatial3d::geom
{

GeoBBoxFactory::GeoBBoxFactory() {}

shared_ptr<GeoBBox>
GeoBBoxFactory::makeGeoBBox(shared_ptr<PlanetModel> planetModel, double topLat,
                            double bottomLat, double leftLon, double rightLon)
{
  // System.err.println("Making rectangle for topLat="+topLat*180.0/Math.PI+",
  // bottomLat="+bottomLat*180.0/Math.PI+", leftLon="+leftLon*180.0/Math.PI+",
  // rightlon="+rightLon*180.0/Math.PI);
  if (topLat > M_PI * 0.5) {
    topLat = M_PI * 0.5;
  }
  if (bottomLat < -M_PI * 0.5) {
    bottomLat = -M_PI * 0.5;
  }
  if (leftLon < -M_PI) {
    leftLon = -M_PI;
  }
  if (rightLon > M_PI) {
    rightLon = M_PI;
  }
  if ((abs(leftLon + M_PI) < Vector::MINIMUM_ANGULAR_RESOLUTION &&
       abs(rightLon - M_PI) < Vector::MINIMUM_ANGULAR_RESOLUTION) ||
      (abs(rightLon + M_PI) < Vector::MINIMUM_ANGULAR_RESOLUTION &&
       abs(leftLon - M_PI) < Vector::MINIMUM_ANGULAR_RESOLUTION)) {
    if (abs(topLat - M_PI * 0.5) < Vector::MINIMUM_ANGULAR_RESOLUTION &&
        abs(bottomLat + M_PI * 0.5) < Vector::MINIMUM_ANGULAR_RESOLUTION) {
      return make_shared<GeoWorld>(planetModel);
    }
    if (abs(topLat - bottomLat) < Vector::MINIMUM_ANGULAR_RESOLUTION) {
      if (abs(topLat - M_PI * 0.5) < Vector::MINIMUM_ANGULAR_RESOLUTION ||
          abs(topLat + M_PI * 0.5) < Vector::MINIMUM_ANGULAR_RESOLUTION) {
        return make_shared<GeoDegeneratePoint>(planetModel, topLat, 0.0);
      }
      return make_shared<GeoDegenerateLatitudeZone>(planetModel, topLat);
    }
    if (abs(topLat - M_PI * 0.5) < Vector::MINIMUM_ANGULAR_RESOLUTION) {
      return make_shared<GeoNorthLatitudeZone>(planetModel, bottomLat);
    } else if (abs(bottomLat + M_PI * 0.5) <
               Vector::MINIMUM_ANGULAR_RESOLUTION) {
      return make_shared<GeoSouthLatitudeZone>(planetModel, topLat);
    }
    return make_shared<GeoLatitudeZone>(planetModel, topLat, bottomLat);
  }
  // System.err.println(" not latitude zone");
  double extent = rightLon - leftLon;
  if (extent < 0.0) {
    extent += M_PI * 2.0;
  }
  if (topLat == M_PI * 0.5 && bottomLat == -M_PI * 0.5) {
    if (abs(leftLon - rightLon) < Vector::MINIMUM_ANGULAR_RESOLUTION) {
      return make_shared<GeoDegenerateLongitudeSlice>(planetModel, leftLon);
    }

    if (extent >= M_PI) {
      return make_shared<GeoWideLongitudeSlice>(planetModel, leftLon, rightLon);
    }

    return make_shared<GeoLongitudeSlice>(planetModel, leftLon, rightLon);
  }
  // System.err.println(" not longitude slice");
  if (abs(leftLon - rightLon) < Vector::MINIMUM_ANGULAR_RESOLUTION) {
    if (abs(topLat - bottomLat) < Vector::MINIMUM_ANGULAR_RESOLUTION) {
      return make_shared<GeoDegeneratePoint>(planetModel, topLat, leftLon);
    }
    return make_shared<GeoDegenerateVerticalLine>(planetModel, topLat,
                                                  bottomLat, leftLon);
  }
  // System.err.println(" not vertical line");
  if (extent >= M_PI) {
    if (abs(topLat - bottomLat) < Vector::MINIMUM_ANGULAR_RESOLUTION) {
      if (abs(topLat - M_PI * 0.5) < Vector::MINIMUM_ANGULAR_RESOLUTION) {
        return make_shared<GeoDegeneratePoint>(planetModel, topLat, 0.0);
      } else if (abs(bottomLat + M_PI * 0.5) <
                 Vector::MINIMUM_ANGULAR_RESOLUTION) {
        return make_shared<GeoDegeneratePoint>(planetModel, bottomLat, 0.0);
      }
      // System.err.println(" wide degenerate line");
      return make_shared<GeoWideDegenerateHorizontalLine>(planetModel, topLat,
                                                          leftLon, rightLon);
    }
    if (abs(topLat - M_PI * 0.5) < Vector::MINIMUM_ANGULAR_RESOLUTION) {
      return make_shared<GeoWideNorthRectangle>(planetModel, bottomLat, leftLon,
                                                rightLon);
    } else if (abs(bottomLat + M_PI * 0.5) <
               Vector::MINIMUM_ANGULAR_RESOLUTION) {
      return make_shared<GeoWideSouthRectangle>(planetModel, topLat, leftLon,
                                                rightLon);
    }
    // System.err.println(" wide rect");
    return make_shared<GeoWideRectangle>(planetModel, topLat, bottomLat,
                                         leftLon, rightLon);
  }
  if (abs(topLat - bottomLat) < Vector::MINIMUM_ANGULAR_RESOLUTION) {
    if (abs(topLat - M_PI * 0.5) < Vector::MINIMUM_ANGULAR_RESOLUTION) {
      return make_shared<GeoDegeneratePoint>(planetModel, topLat, 0.0);
    } else if (abs(bottomLat + M_PI * 0.5) <
               Vector::MINIMUM_ANGULAR_RESOLUTION) {
      return make_shared<GeoDegeneratePoint>(planetModel, bottomLat, 0.0);
    }
    // System.err.println(" horizontal line");
    return make_shared<GeoDegenerateHorizontalLine>(planetModel, topLat,
                                                    leftLon, rightLon);
  }
  if (abs(topLat - M_PI * 0.5) < Vector::MINIMUM_ANGULAR_RESOLUTION) {
    return make_shared<GeoNorthRectangle>(planetModel, bottomLat, leftLon,
                                          rightLon);
  } else if (abs(bottomLat + M_PI * 0.5) < Vector::MINIMUM_ANGULAR_RESOLUTION) {
    return make_shared<GeoSouthRectangle>(planetModel, topLat, leftLon,
                                          rightLon);
  }
  // System.err.println(" rectangle");
  return make_shared<GeoRectangle>(planetModel, topLat, bottomLat, leftLon,
                                   rightLon);
}

shared_ptr<GeoBBox>
GeoBBoxFactory::makeGeoBBox(shared_ptr<PlanetModel> planetModel,
                            shared_ptr<LatLonBounds> bounds)
{
  constexpr double topLat = (bounds->checkNoTopLatitudeBound())
                                ? M_PI * 0.5
                                : bounds->getMaxLatitude();
  constexpr double bottomLat = (bounds->checkNoBottomLatitudeBound())
                                   ? -M_PI * 0.5
                                   : bounds->getMinLatitude();
  constexpr double leftLon =
      (bounds->checkNoLongitudeBound()) ? -M_PI : bounds->getLeftLongitude();
  constexpr double rightLon =
      (bounds->checkNoLongitudeBound()) ? M_PI : bounds->getRightLongitude();
  return makeGeoBBox(planetModel, topLat, bottomLat, leftLon, rightLon);
}
} // namespace org::apache::lucene::spatial3d::geom