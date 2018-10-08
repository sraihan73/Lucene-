using namespace std;

#include "DistanceValueSource.h"

namespace org::apache::lucene::spatial::deque
{
using DocValues = org::apache::lucene::index::DocValues;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using org::locationtech::spatial4j::distance::DistanceCalculator;
using org::locationtech::spatial4j::shape::Point;

DistanceValueSource::DistanceValueSource(
    shared_ptr<PointVectorStrategy> strategy, shared_ptr<Point> from,
    double multiplier)
    : from(from), multiplier(multiplier), nullValue(180 * multiplier)
{
  this->strategy = strategy;
}

wstring DistanceValueSource::toString()
{
  return L"DistanceValueSource(" + strategy + L", " + from + L")";
}

shared_ptr<DoubleValues> DistanceValueSource::getValues(
    shared_ptr<LeafReaderContext> readerContext,
    shared_ptr<DoubleValues> scores) 
{
  shared_ptr<LeafReader> reader = readerContext->reader();

  shared_ptr<NumericDocValues> *const ptX =
      DocValues::getNumeric(reader, strategy->getFieldNameX());
  shared_ptr<NumericDocValues> *const ptY =
      DocValues::getNumeric(reader, strategy->getFieldNameY());

  return DoubleValues::withDefault(make_shared<DoubleValuesAnonymousInnerClass>(
                                       shared_from_this(), ptX, ptY),
                                   nullValue);
}

DistanceValueSource::DoubleValuesAnonymousInnerClass::
    DoubleValuesAnonymousInnerClass(
        shared_ptr<DistanceValueSource> outerInstance,
        shared_ptr<NumericDocValues> ptX, shared_ptr<NumericDocValues> ptY)
{
  this->outerInstance = outerInstance;
  this->ptX = ptX;
  this->ptY = ptY;
  outerInstance->from = outerInstance->from;
  calculator = outerInstance->strategy->getSpatialContext()->getDistCalc();
}

double
DistanceValueSource::DoubleValuesAnonymousInnerClass::doubleValue() throw(
    IOException)
{
  double x = Double::longBitsToDouble(ptX->longValue());
  double y = Double::longBitsToDouble(ptY->longValue());
  return calculator::distance(outerInstance->from, x, y) *
         outerInstance->multiplier;
}

bool DistanceValueSource::DoubleValuesAnonymousInnerClass::advanceExact(
    int doc) 
{
  return ptX->advanceExact(doc) && ptY->advanceExact(doc);
}

bool DistanceValueSource::needsScores() { return false; }

bool DistanceValueSource::isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return DocValues::isCacheable(
      ctx, {strategy->getFieldNameX(), strategy->getFieldNameY()});
}

shared_ptr<DoubleValuesSource> DistanceValueSource::rewrite(
    shared_ptr<IndexSearcher> searcher) 
{
  return shared_from_this();
}

bool DistanceValueSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }

  shared_ptr<DistanceValueSource> that =
      any_cast<std::shared_ptr<DistanceValueSource>>(o);

  if (!from->equals(that->from)) {
    return false;
  }
  if (!strategy->equals(that->strategy)) {
    return false;
  }
  if (multiplier != that->multiplier) {
    return false;
  }

  return true;
}

int DistanceValueSource::hashCode() { return from->hashCode(); }
} // namespace org::apache::lucene::spatial::deque