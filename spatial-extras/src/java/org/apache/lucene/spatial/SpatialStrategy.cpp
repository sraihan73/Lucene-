using namespace std;

#include "SpatialStrategy.h"

namespace org::apache::lucene::spatial
{
using Field = org::apache::lucene::document::Field;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using Query = org::apache::lucene::search::Query;
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;
using ReciprocalDoubleValuesSource =
    org::apache::lucene::spatial::util::ReciprocalDoubleValuesSource;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;

SpatialStrategy::SpatialStrategy(shared_ptr<SpatialContext> ctx,
                                 const wstring &fieldName)
    : ctx(ctx), fieldName(fieldName)
{
  if (ctx == nullptr) {
    throw invalid_argument(L"ctx is required");
  }
  if (fieldName == L"" || fieldName.length() == 0) {
    throw invalid_argument(L"fieldName is required");
  }
}

shared_ptr<SpatialContext> SpatialStrategy::getSpatialContext() { return ctx; }

wstring SpatialStrategy::getFieldName() { return fieldName; }

shared_ptr<DoubleValuesSource>
SpatialStrategy::makeDistanceValueSource(shared_ptr<Point> queryPoint)
{
  return makeDistanceValueSource(queryPoint, 1.0);
}

shared_ptr<DoubleValuesSource>
SpatialStrategy::makeRecipDistanceValueSource(shared_ptr<Shape> queryShape)
{
  shared_ptr<Rectangle> bbox = queryShape->getBoundingBox();
  double diagonalDist = ctx->getDistCalc().distance(
      ctx->makePoint(bbox->getMinX(), bbox->getMinY()), bbox->getMaxX(),
      bbox->getMaxY());
  double distToEdge = diagonalDist * 0.5;
  float c = static_cast<float>(distToEdge) * 0.1f; // one tenth
  shared_ptr<DoubleValuesSource> distance =
      makeDistanceValueSource(queryShape->getCenter(), 1.0);
  return make_shared<ReciprocalDoubleValuesSource>(c, distance);
}

wstring SpatialStrategy::toString()
{
  return getClass().getSimpleName() + L" field:" + fieldName + L" ctx=" + ctx;
}
} // namespace org::apache::lucene::spatial