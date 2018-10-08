using namespace std;

#include "BBoxSimilarityValueSource.h"

namespace org::apache::lucene::spatial::bbox
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ShapeValues = org::apache::lucene::spatial::ShapeValues;
using ShapeValuesSource = org::apache::lucene::spatial::ShapeValuesSource;
using org::locationtech::spatial4j::shape::Rectangle;

BBoxSimilarityValueSource::BBoxSimilarityValueSource(
    shared_ptr<ShapeValuesSource> bboxValueSource)
    : bboxValueSource(bboxValueSource)
{
}

shared_ptr<DoubleValuesSource> BBoxSimilarityValueSource::rewrite(
    shared_ptr<IndexSearcher> searcher) 
{
  return shared_from_this();
}

wstring BBoxSimilarityValueSource::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return getClass().getSimpleName() + L"(" + bboxValueSource->toString() +
         L"," + similarityDescription() + L")";
}

shared_ptr<DoubleValues> BBoxSimilarityValueSource::getValues(
    shared_ptr<LeafReaderContext> readerContext,
    shared_ptr<DoubleValues> scores) 
{

  shared_ptr<ShapeValues> *const shapeValues =
      bboxValueSource->getValues(readerContext);
  return DoubleValues::withDefault(make_shared<DoubleValuesAnonymousInnerClass>(
                                       shared_from_this(), shapeValues),
                                   0);
}

BBoxSimilarityValueSource::DoubleValuesAnonymousInnerClass::
    DoubleValuesAnonymousInnerClass(
        shared_ptr<BBoxSimilarityValueSource> outerInstance,
        shared_ptr<ShapeValues> shapeValues)
{
  this->outerInstance = outerInstance;
  this->shapeValues = shapeValues;
}

double
BBoxSimilarityValueSource::DoubleValuesAnonymousInnerClass::doubleValue() throw(
    IOException)
{
  return outerInstance->score(
      std::static_pointer_cast<Rectangle>(shapeValues->value()), nullptr);
}

bool BBoxSimilarityValueSource::DoubleValuesAnonymousInnerClass::advanceExact(
    int doc) 
{
  return shapeValues->advanceExact(doc);
}

bool BBoxSimilarityValueSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false; // same class
  }

  shared_ptr<BBoxSimilarityValueSource> that =
      any_cast<std::shared_ptr<BBoxSimilarityValueSource>>(o);

  if (!bboxValueSource->equals(that->bboxValueSource)) {
    return false;
  }

  return true;
}

int BBoxSimilarityValueSource::hashCode()
{
  return bboxValueSource->hashCode();
}

shared_ptr<Explanation> BBoxSimilarityValueSource::explain(
    shared_ptr<LeafReaderContext> ctx, int docId,
    shared_ptr<Explanation> scoreExplanation) 
{
  shared_ptr<DoubleValues> dv =
      getValues(ctx, DoubleValuesSource::constant(scoreExplanation->getValue())
                         ->getValues(ctx, nullptr));
  if (dv->advanceExact(docId)) {
    shared_ptr<AtomicReference<std::shared_ptr<Explanation>>> explanation =
        make_shared<AtomicReference<std::shared_ptr<Explanation>>>();
    shared_ptr<ShapeValues> *const shapeValues =
        bboxValueSource->getValues(ctx);
    if (shapeValues->advanceExact(docId)) {
      score(std::static_pointer_cast<Rectangle>(shapeValues->value()),
            explanation);
      return explanation->get();
    }
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Explanation::noMatch(this->toString());
}

bool BBoxSimilarityValueSource::isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return bboxValueSource->isCacheable(ctx);
}

bool BBoxSimilarityValueSource::needsScores() { return false; }
} // namespace org::apache::lucene::spatial::bbox