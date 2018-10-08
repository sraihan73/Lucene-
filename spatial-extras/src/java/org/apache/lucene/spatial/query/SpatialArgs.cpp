using namespace std;

#include "SpatialArgs.h"

namespace org::apache::lucene::spatial::query
{
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;

SpatialArgs::SpatialArgs(shared_ptr<SpatialOperation> operation,
                         shared_ptr<Shape> shape)
{
  if (operation == nullptr || shape == nullptr) {
    throw make_shared<NullPointerException>(
        L"operation and shape are required");
  }
  this->operation = operation;
  this->shape = shape;
}

double SpatialArgs::calcDistanceFromErrPct(shared_ptr<Shape> shape,
                                           double distErrPct,
                                           shared_ptr<SpatialContext> ctx)
{
  if (distErrPct < 0 || distErrPct > 0.5) {
    throw invalid_argument(L"distErrPct " + to_wstring(distErrPct) +
                           L" must be between [0 to 0.5]");
  }
  if (distErrPct == 0 || std::dynamic_pointer_cast<Point>(shape) != nullptr) {
    return 0;
  }
  shared_ptr<Rectangle> bbox = shape->getBoundingBox();
  // Compute the distance from the center to a corner.  Because the distance
  // to a bottom corner vs a top corner can vary in a geospatial scenario,
  // take the closest one (greater precision).
  shared_ptr<Point> ctr = bbox->getCenter();
  double y = (ctr->getY() >= 0 ? bbox->getMaxY() : bbox->getMinY());
  double diagonalDist = ctx->getDistCalc().distance(ctr, bbox->getMaxX(), y);
  return diagonalDist * distErrPct;
}

double SpatialArgs::resolveDistErr(shared_ptr<SpatialContext> ctx,
                                   double defaultDistErrPct)
{
  if (distErr) {
    return distErr;
  }
  double distErrPct = (this->distErrPct ? this->distErrPct : defaultDistErrPct);
  return calcDistanceFromErrPct(shape, distErrPct, ctx);
}

void SpatialArgs::validate() 
{
  if (distErr && distErrPct) {
    throw invalid_argument(L"Only distErr or distErrPct can be specified.");
  }
}

wstring SpatialArgs::toString()
{
  return SpatialArgsParser::writeSpatialArgs(shared_from_this());
}

shared_ptr<SpatialOperation> SpatialArgs::getOperation() { return operation; }

void SpatialArgs::setOperation(shared_ptr<SpatialOperation> operation)
{
  this->operation = operation;
}

shared_ptr<Shape> SpatialArgs::getShape() { return shape; }

void SpatialArgs::setShape(shared_ptr<Shape> shape) { this->shape = shape; }

optional<double> SpatialArgs::getDistErrPct() { return distErrPct; }

void SpatialArgs::setDistErrPct(optional<double> &distErrPct)
{
  if (distErrPct) {
    this->distErrPct = distErrPct;
  }
}

optional<double> SpatialArgs::getDistErr() { return distErr; }

void SpatialArgs::setDistErr(optional<double> &distErr)
{
  this->distErr = distErr;
}
} // namespace org::apache::lucene::spatial::query