using namespace std;

#include "ShapeFieldCacheDistanceValueSource.h"

namespace org::apache::lucene::spatial::util
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::distance::DistanceCalculator;
using org::locationtech::spatial4j::shape::Point;

ShapeFieldCacheDistanceValueSource::ShapeFieldCacheDistanceValueSource(
    shared_ptr<SpatialContext> ctx,
    shared_ptr<ShapeFieldCacheProvider<std::shared_ptr<Point>>> provider,
    shared_ptr<Point> from, double multiplier)
    : ctx(ctx), from(from), provider(provider), multiplier(multiplier)
{
}

wstring ShapeFieldCacheDistanceValueSource::toString()
{
  return getClass().getSimpleName() + L"(" + provider + L", " + from + L")";
}

shared_ptr<DoubleValues> ShapeFieldCacheDistanceValueSource::getValues(
    shared_ptr<LeafReaderContext> readerContext,
    shared_ptr<DoubleValues> scores) 
{

  constexpr double nullValue =
      (ctx->isGeo() ? 180 * multiplier : numeric_limits<double>::max());

  return DoubleValues::withDefault(make_shared<DoubleValuesAnonymousInnerClass>(
                                       shared_from_this(), readerContext),
                                   nullValue);
}

ShapeFieldCacheDistanceValueSource::DoubleValuesAnonymousInnerClass::
    DoubleValuesAnonymousInnerClass(
        shared_ptr<ShapeFieldCacheDistanceValueSource> outerInstance,
        shared_ptr<LeafReaderContext> readerContext)
{
  this->outerInstance = outerInstance;
  this->readerContext = readerContext;
  cache = outerInstance->provider->getCache(readerContext->reader());
  outerInstance->from = outerInstance->from;
  calculator = outerInstance->ctx->getDistCalc();
}

double ShapeFieldCacheDistanceValueSource::DoubleValuesAnonymousInnerClass::
    doubleValue() 
{
  double v = calculator::distance(outerInstance->from, currentVals->get(0));
  for (int i = 1; i < currentVals->size(); i++) {
    v = min(v, calculator::distance(outerInstance->from, currentVals->get(i)));
  }
  return v * outerInstance->multiplier;
}

bool ShapeFieldCacheDistanceValueSource::DoubleValuesAnonymousInnerClass::
    advanceExact(int doc) 
{
  currentVals = cache::getShapes(doc);
  return currentVals != nullptr;
}

bool ShapeFieldCacheDistanceValueSource::needsScores() { return false; }

bool ShapeFieldCacheDistanceValueSource::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return true;
}

shared_ptr<DoubleValuesSource> ShapeFieldCacheDistanceValueSource::rewrite(
    shared_ptr<IndexSearcher> searcher) 
{
  return shared_from_this();
}

bool ShapeFieldCacheDistanceValueSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }

  shared_ptr<ShapeFieldCacheDistanceValueSource> that =
      any_cast<std::shared_ptr<ShapeFieldCacheDistanceValueSource>>(o);

  if (!ctx->equals(that->ctx)) {
    return false;
  }
  if (!from->equals(that->from)) {
    return false;
  }
  if (!provider->equals(that->provider)) {
    return false;
  }
  if (multiplier != that->multiplier) {
    return false;
  }

  return true;
}

int ShapeFieldCacheDistanceValueSource::hashCode() { return from->hashCode(); }
} // namespace org::apache::lucene::spatial::util