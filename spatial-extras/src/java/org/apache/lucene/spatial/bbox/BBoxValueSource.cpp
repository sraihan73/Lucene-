using namespace std;

#include "BBoxValueSource.h"

namespace org::apache::lucene::spatial::bbox
{
using DocValues = org::apache::lucene::index::DocValues;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using ShapeValues = org::apache::lucene::spatial::ShapeValues;
using ShapeValuesSource = org::apache::lucene::spatial::ShapeValuesSource;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;

BBoxValueSource::BBoxValueSource(shared_ptr<BBoxStrategy> strategy)
    : strategy(strategy)
{
}

wstring BBoxValueSource::toString()
{
  return L"bboxShape(" + strategy->getFieldName() + L")";
}

shared_ptr<ShapeValues> BBoxValueSource::getValues(
    shared_ptr<LeafReaderContext> readerContext) 
{
  shared_ptr<LeafReader> reader = readerContext->reader();
  shared_ptr<NumericDocValues> *const minX =
      DocValues::getNumeric(reader, strategy->field_minX);
  shared_ptr<NumericDocValues> *const minY =
      DocValues::getNumeric(reader, strategy->field_minY);
  shared_ptr<NumericDocValues> *const maxX =
      DocValues::getNumeric(reader, strategy->field_maxX);
  shared_ptr<NumericDocValues> *const maxY =
      DocValues::getNumeric(reader, strategy->field_maxY);

  // reused
  shared_ptr<Rectangle> *const rect =
      strategy->getSpatialContext()->makeRectangle(0, 0, 0, 0);

  return make_shared<ShapeValuesAnonymousInnerClass>(shared_from_this(), minX,
                                                     minY, maxX, maxY, rect);
}

BBoxValueSource::ShapeValuesAnonymousInnerClass::ShapeValuesAnonymousInnerClass(
    shared_ptr<BBoxValueSource> outerInstance,
    shared_ptr<NumericDocValues> minX, shared_ptr<NumericDocValues> minY,
    shared_ptr<NumericDocValues> maxX, shared_ptr<NumericDocValues> maxY,
    shared_ptr<Rectangle> rect)
{
  this->outerInstance = outerInstance;
  this->minX = minX;
  this->minY = minY;
  this->maxX = maxX;
  this->maxY = maxY;
  this->rect = rect;
}

bool BBoxValueSource::ShapeValuesAnonymousInnerClass::advanceExact(
    int doc) 
{
  return minX->advanceExact(doc) && minY->advanceExact(doc) &&
         maxX->advanceExact(doc) && maxY->advanceExact(doc);
}

shared_ptr<Shape>
BBoxValueSource::ShapeValuesAnonymousInnerClass::value() 
{
  double minXValue = Double::longBitsToDouble(minX->longValue());
  double minYValue = Double::longBitsToDouble(minY->longValue());
  double maxXValue = Double::longBitsToDouble(maxX->longValue());
  double maxYValue = Double::longBitsToDouble(maxY->longValue());
  rect->reset(minXValue, maxXValue, minYValue, maxYValue);
  return rect;
}

bool BBoxValueSource::isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return DocValues::isCacheable(ctx,
                                {strategy->field_minX, strategy->field_minY,
                                 strategy->field_maxX, strategy->field_maxY});
}

bool BBoxValueSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }

  shared_ptr<BBoxValueSource> that =
      any_cast<std::shared_ptr<BBoxValueSource>>(o);

  if (!strategy->equals(that->strategy)) {
    return false;
  }

  return true;
}

int BBoxValueSource::hashCode() { return strategy->hashCode(); }
} // namespace org::apache::lucene::spatial::bbox