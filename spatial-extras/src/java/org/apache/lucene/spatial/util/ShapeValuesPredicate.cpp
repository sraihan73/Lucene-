using namespace std;

#include "ShapeValuesPredicate.h"

namespace org::apache::lucene::spatial::util
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using SegmentCacheable = org::apache::lucene::search::SegmentCacheable;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;
using ShapeValues = org::apache::lucene::spatial::ShapeValues;
using ShapeValuesSource = org::apache::lucene::spatial::ShapeValuesSource;
using SpatialOperation = org::apache::lucene::spatial::query::SpatialOperation;
using org::locationtech::spatial4j::shape::Shape;

ShapeValuesPredicate::ShapeValuesPredicate(
    shared_ptr<ShapeValuesSource> shapeValuesource,
    shared_ptr<SpatialOperation> op, shared_ptr<Shape> queryShape)
    : shapeValuesource(shapeValuesource), op(op), queryShape(queryShape)
{
}

wstring ShapeValuesPredicate::toString()
{
  return shapeValuesource + L" " + op + L" " + queryShape;
}

shared_ptr<TwoPhaseIterator> ShapeValuesPredicate::iterator(
    shared_ptr<LeafReaderContext> ctx,
    shared_ptr<DocIdSetIterator> approximation) 
{
  shared_ptr<ShapeValues> *const shapeValues = shapeValuesource->getValues(ctx);
  return make_shared<TwoPhaseIteratorAnonymousInnerClass>(
      shared_from_this(), approximation, shapeValues);
}

ShapeValuesPredicate::TwoPhaseIteratorAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass(
        shared_ptr<ShapeValuesPredicate> outerInstance,
        shared_ptr<DocIdSetIterator> approximation,
        shared_ptr<ShapeValues> shapeValues)
    : org::apache::lucene::search::TwoPhaseIterator(approximation)
{
  this->outerInstance = outerInstance;
  this->approximation = approximation;
  this->shapeValues = shapeValues;
}

bool ShapeValuesPredicate::TwoPhaseIteratorAnonymousInnerClass::matches() throw(
    IOException)
{
  return shapeValues->advanceExact(approximation->docID()) &&
         outerInstance->op->evaluate(shapeValues->value(),
                                     outerInstance->queryShape);
}

float ShapeValuesPredicate::TwoPhaseIteratorAnonymousInnerClass::matchCost()
{
  return 100; // is this necessary?
}

bool ShapeValuesPredicate::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }

  shared_ptr<ShapeValuesPredicate> that =
      any_cast<std::shared_ptr<ShapeValuesPredicate>>(o);

  if (!shapeValuesource->equals(that->shapeValuesource)) {
    return false;
  }
  if (!op->equals(that->op)) {
    return false;
  }
  if (!queryShape->equals(that->queryShape)) {
    return false;
  }

  return true;
}

int ShapeValuesPredicate::hashCode()
{
  int result = shapeValuesource->hashCode();
  result = 31 * result + op->hashCode();
  result = 31 * result + queryShape->hashCode();
  return result;
}

bool ShapeValuesPredicate::isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return shapeValuesource->isCacheable(ctx);
}
} // namespace org::apache::lucene::spatial::util