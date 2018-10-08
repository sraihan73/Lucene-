using namespace std;

#include "DistanceToShapeValueSource.h"

namespace org::apache::lucene::spatial::util
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ShapeValues = org::apache::lucene::spatial::ShapeValues;
using ShapeValuesSource = org::apache::lucene::spatial::ShapeValuesSource;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::distance::DistanceCalculator;
using org::locationtech::spatial4j::shape::Point;

DistanceToShapeValueSource::DistanceToShapeValueSource(
    shared_ptr<ShapeValuesSource> shapeValueSource,
    shared_ptr<Point> queryPoint, double multiplier,
    shared_ptr<SpatialContext> ctx)
    : shapeValueSource(shapeValueSource), queryPoint(queryPoint),
      multiplier(multiplier), distCalc(ctx->getDistCalc()),
      nullValue(
          (ctx->isGeo() ? 180 * multiplier : numeric_limits<double>::max()))
{
}

wstring DistanceToShapeValueSource::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"distance(" + queryPoint + L" to " + shapeValueSource->toString() +
         L")*" + to_wstring(multiplier) + L")";
}

shared_ptr<DoubleValues> DistanceToShapeValueSource::getValues(
    shared_ptr<LeafReaderContext> readerContext,
    shared_ptr<DoubleValues> scores) 
{

  shared_ptr<ShapeValues> *const shapeValues =
      shapeValueSource->getValues(readerContext);

  return DoubleValues::withDefault(make_shared<DoubleValuesAnonymousInnerClass>(
                                       shared_from_this(), shapeValues),
                                   nullValue);
}

DistanceToShapeValueSource::DoubleValuesAnonymousInnerClass::
    DoubleValuesAnonymousInnerClass(
        shared_ptr<DistanceToShapeValueSource> outerInstance,
        shared_ptr<ShapeValues> shapeValues)
{
  this->outerInstance = outerInstance;
  this->shapeValues = shapeValues;
}

double DistanceToShapeValueSource::DoubleValuesAnonymousInnerClass::
    doubleValue() 
{
  return outerInstance->distCalc->distance(outerInstance->queryPoint,
                                           shapeValues->value()->getCenter()) *
         outerInstance->multiplier;
}

bool DistanceToShapeValueSource::DoubleValuesAnonymousInnerClass::advanceExact(
    int doc) 
{
  return shapeValues->advanceExact(doc);
}

bool DistanceToShapeValueSource::needsScores() { return false; }

bool DistanceToShapeValueSource::isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return shapeValueSource->isCacheable(ctx);
}

shared_ptr<DoubleValuesSource> DistanceToShapeValueSource::rewrite(
    shared_ptr<IndexSearcher> searcher) 
{
  return shared_from_this();
}

bool DistanceToShapeValueSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }

  shared_ptr<DistanceToShapeValueSource> that =
      any_cast<std::shared_ptr<DistanceToShapeValueSource>>(o);

  if (!queryPoint->equals(that->queryPoint)) {
    return false;
  }
  if (Double::compare(that->multiplier, multiplier) != 0) {
    return false;
  }
  if (!shapeValueSource->equals(that->shapeValueSource)) {
    return false;
  }
  if (!distCalc->equals(that->distCalc)) {
    return false;
  }

  return true;
}

int DistanceToShapeValueSource::hashCode()
{
  int result;
  int64_t temp;
  result = shapeValueSource->hashCode();
  result = 31 * result + queryPoint->hashCode();
  temp = Double::doubleToLongBits(multiplier);
  result = 31 * result +
           static_cast<int>(temp ^
                            (static_cast<int64_t>(
                                static_cast<uint64_t>(temp) >> 32)));
  return result;
}
} // namespace org::apache::lucene::spatial::util