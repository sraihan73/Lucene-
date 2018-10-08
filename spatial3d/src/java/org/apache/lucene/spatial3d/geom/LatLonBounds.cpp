using namespace std;

#include "LatLonBounds.h"

namespace org::apache::lucene::spatial3d::geom
{

LatLonBounds::LatLonBounds() {}

optional<double> LatLonBounds::getMaxLatitude() { return maxLatitude; }

optional<double> LatLonBounds::getMinLatitude() { return minLatitude; }

optional<double> LatLonBounds::getLeftLongitude() { return leftLongitude; }

optional<double> LatLonBounds::getRightLongitude() { return rightLongitude; }

bool LatLonBounds::checkNoLongitudeBound() { return noLongitudeBound_; }

bool LatLonBounds::checkNoTopLatitudeBound() { return noTopLatitudeBound_; }

bool LatLonBounds::checkNoBottomLatitudeBound()
{
  return noBottomLatitudeBound_;
}

shared_ptr<Bounds> LatLonBounds::addPlane(shared_ptr<PlanetModel> planetModel,
                                          shared_ptr<Plane> plane,
                                          deque<Membership> &bounds)
{
  plane->recordBounds(planetModel, shared_from_this(), bounds);
  return shared_from_this();
}

shared_ptr<Bounds> LatLonBounds::addHorizontalPlane(
    shared_ptr<PlanetModel> planetModel, double const latitude,
    shared_ptr<Plane> horizontalPlane, deque<Membership> &bounds)
{
  if (!noTopLatitudeBound_ || !noBottomLatitudeBound_) {
    addLatitudeBound(latitude);
  }
  return shared_from_this();
}

shared_ptr<Bounds> LatLonBounds::addVerticalPlane(
    shared_ptr<PlanetModel> planetModel, double const longitude,
    shared_ptr<Plane> verticalPlane, deque<Membership> &bounds)
{
  if (!noLongitudeBound_) {
    addLongitudeBound(longitude);
  }
  return shared_from_this();
}

shared_ptr<Bounds> LatLonBounds::isWide() { return noLongitudeBound(); }

shared_ptr<Bounds> LatLonBounds::addXValue(shared_ptr<GeoPoint> point)
{
  if (!noLongitudeBound_) {
    // Get a longitude value
    addLongitudeBound(point->getLongitude());
  }
  return shared_from_this();
}

shared_ptr<Bounds> LatLonBounds::addYValue(shared_ptr<GeoPoint> point)
{
  if (!noLongitudeBound_) {
    // Get a longitude value
    addLongitudeBound(point->getLongitude());
  }
  return shared_from_this();
}

shared_ptr<Bounds> LatLonBounds::addZValue(shared_ptr<GeoPoint> point)
{
  if (!noTopLatitudeBound_ || !noBottomLatitudeBound_) {
    // Compute a latitude value
    double latitude = point->getLatitude();
    addLatitudeBound(latitude);
  }
  return shared_from_this();
}

shared_ptr<Bounds> LatLonBounds::addIntersection(
    shared_ptr<PlanetModel> planetModel, shared_ptr<Plane> plane1,
    shared_ptr<Plane> plane2, deque<Membership> &bounds)
{
  plane1->recordBounds(planetModel, shared_from_this(), plane2, {bounds});
  return shared_from_this();
}

shared_ptr<Bounds> LatLonBounds::addPoint(shared_ptr<GeoPoint> point)
{
  if (!noLongitudeBound_) {
    // Get a longitude value
    addLongitudeBound(point->getLongitude());
  }
  if (!noTopLatitudeBound_ || !noBottomLatitudeBound_) {
    // Compute a latitude value
    addLatitudeBound(point->getLatitude());
  }
  return shared_from_this();
}

shared_ptr<Bounds> LatLonBounds::noLongitudeBound()
{
  noLongitudeBound_ = true;
  leftLongitude = nullopt;
  rightLongitude = nullopt;
  return shared_from_this();
}

shared_ptr<Bounds> LatLonBounds::noTopLatitudeBound()
{
  noTopLatitudeBound_ = true;
  maxLatitude = nullopt;
  return shared_from_this();
}

shared_ptr<Bounds> LatLonBounds::noBottomLatitudeBound()
{
  noBottomLatitudeBound_ = true;
  minLatitude = nullopt;
  return shared_from_this();
}

shared_ptr<Bounds> LatLonBounds::noBound(shared_ptr<PlanetModel> planetModel)
{
  return noLongitudeBound()->noTopLatitudeBound()->noBottomLatitudeBound();
}

void LatLonBounds::addLatitudeBound(double latitude)
{
  if (!noTopLatitudeBound_ && (!maxLatitude || latitude > maxLatitude)) {
    maxLatitude = latitude;
  }
  if (!noBottomLatitudeBound_ && (!minLatitude || latitude < minLatitude)) {
    minLatitude = latitude;
  }
}

void LatLonBounds::addLongitudeBound(double longitude)
{
  // If this point is within the current bounds, we're done; otherwise
  // expand one side or the other.
  if (!leftLongitude && !rightLongitude) {
    leftLongitude = longitude;
    rightLongitude = longitude;
  } else {
    // Compute whether we're to the right of the left value.  But the left value
    // may be greater than the right value.
    double currentLeftLongitude = leftLongitude;
    double currentRightLongitude = rightLongitude;
    if (currentRightLongitude < currentLeftLongitude) {
      currentRightLongitude += 2.0 * M_PI;
    }
    // We have a range to look at that's going in the right way.
    // Now, do the same trick with the computed longitude.
    if (longitude < currentLeftLongitude) {
      longitude += 2.0 * M_PI;
    }

    if (longitude < currentLeftLongitude || longitude > currentRightLongitude) {
      // Outside of current bounds.  Consider carefully how we'll expand.
      double leftExtensionAmt;
      double rightExtensionAmt;
      if (longitude < currentLeftLongitude) {
        leftExtensionAmt = currentLeftLongitude - longitude;
      } else {
        leftExtensionAmt = currentLeftLongitude + 2.0 * M_PI - longitude;
      }
      if (longitude > currentRightLongitude) {
        rightExtensionAmt = longitude - currentRightLongitude;
      } else {
        rightExtensionAmt = longitude + 2.0 * M_PI - currentRightLongitude;
      }
      if (leftExtensionAmt < rightExtensionAmt) {
        currentLeftLongitude = leftLongitude - leftExtensionAmt;
        while (currentLeftLongitude <= -M_PI) {
          currentLeftLongitude += 2.0 * M_PI;
        }
        leftLongitude = currentLeftLongitude;
      } else {
        currentRightLongitude = rightLongitude + rightExtensionAmt;
        while (currentRightLongitude > M_PI) {
          currentRightLongitude -= 2.0 * M_PI;
        }
        rightLongitude = currentRightLongitude;
      }
    }
  }
  double testRightLongitude = rightLongitude;
  if (testRightLongitude < leftLongitude) {
    testRightLongitude += M_PI * 2.0;
  }
  if (testRightLongitude - leftLongitude >= M_PI) {
    noLongitudeBound_ = true;
    leftLongitude = nullopt;
    rightLongitude = nullopt;
  }
}
} // namespace org::apache::lucene::spatial3d::geom