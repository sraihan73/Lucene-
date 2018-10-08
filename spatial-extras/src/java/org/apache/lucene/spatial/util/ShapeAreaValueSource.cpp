using namespace std;

#include "ShapeAreaValueSource.h"

namespace org::apache::lucene::spatial::util
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ShapeValues = org::apache::lucene::spatial::ShapeValues;
using ShapeValuesSource = org::apache::lucene::spatial::ShapeValuesSource;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Shape;

ShapeAreaValueSource::ShapeAreaValueSource(
    shared_ptr<ShapeValuesSource> shapeValueSource,
    shared_ptr<SpatialContext> ctx, bool geoArea, double multiplier)
    : shapeValueSource(shapeValueSource), ctx(ctx), geoArea(geoArea)
{
  this->multiplier = multiplier;
}

wstring ShapeAreaValueSource::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"area(" + shapeValueSource->toString() + L",geo=" +
         StringHelper::toString(geoArea) + L")";
}

shared_ptr<DoubleValues> ShapeAreaValueSource::getValues(
    shared_ptr<LeafReaderContext> readerContext,
    shared_ptr<DoubleValues> scores) 
{
  shared_ptr<ShapeValues> *const shapeValues =
      shapeValueSource->getValues(readerContext);
  return DoubleValues::withDefault(make_shared<DoubleValuesAnonymousInnerClass>(
                                       shared_from_this(), shapeValues),
                                   0);
}

ShapeAreaValueSource::DoubleValuesAnonymousInnerClass::
    DoubleValuesAnonymousInnerClass(
        shared_ptr<ShapeAreaValueSource> outerInstance,
        shared_ptr<ShapeValues> shapeValues)
{
  this->outerInstance = outerInstance;
  this->shapeValues = shapeValues;
}

double
ShapeAreaValueSource::DoubleValuesAnonymousInnerClass::doubleValue() throw(
    IOException)
{
  return shapeValues->value()->getArea(
             outerInstance->geoArea ? outerInstance->ctx : nullptr) *
         outerInstance->multiplier;
}

bool ShapeAreaValueSource::DoubleValuesAnonymousInnerClass::advanceExact(
    int doc) 
{
  return shapeValues->advanceExact(doc);
}

bool ShapeAreaValueSource::needsScores() { return false; }

bool ShapeAreaValueSource::isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return shapeValueSource->isCacheable(ctx);
}

shared_ptr<DoubleValuesSource> ShapeAreaValueSource::rewrite(
    shared_ptr<IndexSearcher> searcher) 
{
  return shared_from_this();
}

bool ShapeAreaValueSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }

  shared_ptr<ShapeAreaValueSource> that =
      any_cast<std::shared_ptr<ShapeAreaValueSource>>(o);

  if (geoArea != that->geoArea) {
    return false;
  }
  if (!shapeValueSource->equals(that->shapeValueSource)) {
    return false;
  }

  return true;
}

int ShapeAreaValueSource::hashCode()
{
  int result = shapeValueSource->hashCode();
  result = 31 * result + (geoArea ? 1 : 0);
  return result;
}
} // namespace org::apache::lucene::spatial::util