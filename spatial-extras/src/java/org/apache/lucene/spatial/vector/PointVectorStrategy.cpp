using namespace std;

#include "PointVectorStrategy.h"

namespace org::apache::lucene::spatial::deque
{
using DoubleDocValuesField =
    org::apache::lucene::document::DoubleDocValuesField;
using DoublePoint = org::apache::lucene::document::DoublePoint;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using StoredField = org::apache::lucene::document::StoredField;
using DocValuesType = org::apache::lucene::index::DocValuesType;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using ConstantScoreQuery = org::apache::lucene::search::ConstantScoreQuery;
using ConstantScoreScorer = org::apache::lucene::search::ConstantScoreScorer;
using ConstantScoreWeight = org::apache::lucene::search::ConstantScoreWeight;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;
using Weight = org::apache::lucene::search::Weight;
using SpatialStrategy = org::apache::lucene::spatial::SpatialStrategy;
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;
using SpatialOperation = org::apache::lucene::spatial::query::SpatialOperation;
using UnsupportedSpatialOperation =
    org::apache::lucene::spatial::query::UnsupportedSpatialOperation;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Circle;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;
shared_ptr<org::apache::lucene::document::FieldType>
    PointVectorStrategy::DEFAULT_FIELDTYPE;

PointVectorStrategy::StaticConstructor::StaticConstructor()
{
  // Default: pointValues + docValues
  shared_ptr<FieldType> type = make_shared<FieldType>();
  type->setDimensions(1, Double::BYTES);          // pointValues (assume Double)
  type->setDocValuesType(DocValuesType::NUMERIC); // docValues
  type->setStored(false);
  type->freeze();
  DEFAULT_FIELDTYPE = type;
}

PointVectorStrategy::StaticConstructor PointVectorStrategy::staticConstructor;
const wstring PointVectorStrategy::SUFFIX_X = L"__x";
const wstring PointVectorStrategy::SUFFIX_Y = L"__y";

shared_ptr<PointVectorStrategy>
PointVectorStrategy::newInstance(shared_ptr<SpatialContext> ctx,
                                 const wstring &fieldNamePrefix)
{
  return make_shared<PointVectorStrategy>(ctx, fieldNamePrefix,
                                          DEFAULT_FIELDTYPE);
}

PointVectorStrategy::PointVectorStrategy(shared_ptr<SpatialContext> ctx,
                                         const wstring &fieldNamePrefix,
                                         shared_ptr<FieldType> fieldType)
    : org::apache::lucene::spatial::SpatialStrategy(ctx, fieldNamePrefix),
      fieldNameX(fieldNamePrefix + SUFFIX_X),
      fieldNameY(fieldNamePrefix + SUFFIX_Y), fieldsLen(numPairs * 2)
{

  int numPairs = 0;
  if ((this->hasStored = fieldType->stored())) {
    numPairs++;
  }
  if ((this->hasDocVals = fieldType->docValuesType() != DocValuesType::NONE)) {
    numPairs++;
  }
  if ((this->hasPointVals = fieldType->pointDimensionCount() > 0)) {
    numPairs++;
  }
}

wstring PointVectorStrategy::getFieldNameX() { return fieldNameX; }

wstring PointVectorStrategy::getFieldNameY() { return fieldNameY; }

std::deque<std::shared_ptr<Field>>
PointVectorStrategy::createIndexableFields(shared_ptr<Shape> shape)
{
  if (std::dynamic_pointer_cast<Point>(shape) != nullptr) {
    return createIndexableFields(std::static_pointer_cast<Point>(shape));
  }
  throw make_shared<UnsupportedOperationException>(
      L"Can only index Point, not " + shape);
}

std::deque<std::shared_ptr<Field>>
PointVectorStrategy::createIndexableFields(shared_ptr<Point> point)
{
  std::deque<std::shared_ptr<Field>> fields(fieldsLen);
  int idx = -1;
  if (hasStored) {
    fields[++idx] = make_shared<StoredField>(fieldNameX, point->getX());
    fields[++idx] = make_shared<StoredField>(fieldNameY, point->getY());
  }
  if (hasDocVals) {
    fields[++idx] =
        make_shared<DoubleDocValuesField>(fieldNameX, point->getX());
    fields[++idx] =
        make_shared<DoubleDocValuesField>(fieldNameY, point->getY());
  }
  if (hasPointVals) {
    fields[++idx] = make_shared<DoublePoint>(fieldNameX, point->getX());
    fields[++idx] = make_shared<DoublePoint>(fieldNameY, point->getY());
  }
  assert(idx == fields.size() - 1);
  return fields;
}

shared_ptr<DoubleValuesSource>
PointVectorStrategy::makeDistanceValueSource(shared_ptr<Point> queryPoint,
                                             double multiplier)
{
  return make_shared<DistanceValueSource>(shared_from_this(), queryPoint,
                                          multiplier);
}

shared_ptr<Query> PointVectorStrategy::makeQuery(shared_ptr<SpatialArgs> args)
{
  if (!SpatialOperation::is(
          args->getOperation(),
          {SpatialOperation::Intersects, SpatialOperation::IsWithin})) {
    throw make_shared<UnsupportedSpatialOperation>(args->getOperation());
  }
  shared_ptr<Shape> shape = args->getShape();
  if (std::dynamic_pointer_cast<Rectangle>(shape) != nullptr) {
    shared_ptr<Rectangle> bbox = std::static_pointer_cast<Rectangle>(shape);
    return make_shared<ConstantScoreQuery>(makeWithin(bbox));
  } else if (std::dynamic_pointer_cast<Circle>(shape) != nullptr) {
    shared_ptr<Circle> circle = std::static_pointer_cast<Circle>(shape);
    shared_ptr<Rectangle> bbox = circle->getBoundingBox();
    return make_shared<DistanceRangeQuery>(
        makeWithin(bbox), makeDistanceValueSource(circle->getCenter()),
        circle->getRadius());
  } else {
    throw make_shared<UnsupportedOperationException>(
        wstring(L"Only Rectangles and Circles are currently supported, ") +
        L"found [" + shape->getClass() + L"]"); // TODO
  }
}

shared_ptr<Query> PointVectorStrategy::makeWithin(shared_ptr<Rectangle> bbox)
{
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  BooleanClause::Occur MUST = BooleanClause::Occur::MUST;
  if (bbox->getCrossesDateLine()) {
    // use null as performance trick since no data will be beyond the world
    // bounds
    bq->add(rangeQuery(fieldNameX, nullopt, bbox->getMaxX()),
            BooleanClause::Occur::SHOULD);
    bq->add(rangeQuery(fieldNameX, bbox->getMinX(), nullopt),
            BooleanClause::Occur::SHOULD);
    bq->setMinimumNumberShouldMatch(1); // must match at least one of the SHOULD
  } else {
    bq->add(rangeQuery(fieldNameX, bbox->getMinX(), bbox->getMaxX()), MUST);
  }
  bq->add(rangeQuery(fieldNameY, bbox->getMinY(), bbox->getMaxY()), MUST);
  return bq->build();
}

shared_ptr<Query> PointVectorStrategy::rangeQuery(const wstring &fieldName,
                                                  optional<double> &min,
                                                  optional<double> &max)
{
  if (hasPointVals) {
    if (!min) {
      min = -numeric_limits<double>::infinity();
    }

    if (!max) {
      max = numeric_limits<double>::infinity();
    }

    return DoublePoint::newRangeQuery(fieldName, min, max);
  }
  // TODO try doc-value range query?
  throw make_shared<UnsupportedOperationException>(
      L"An index is required for this operation.");
}

PointVectorStrategy::DistanceRangeQuery::DistanceRangeQuery(
    shared_ptr<Query> inner, shared_ptr<DoubleValuesSource> distanceSource,
    double limit)
    : inner(inner), distanceSource(distanceSource), limit(limit)
{
}

shared_ptr<Query> PointVectorStrategy::DistanceRangeQuery::rewrite(
    shared_ptr<IndexReader> reader) 
{
  shared_ptr<Query> rewritten = inner->rewrite(reader);
  if (rewritten == inner) {
    return shared_from_this();
  }
  return make_shared<DistanceRangeQuery>(rewritten, distanceSource, limit);
}

shared_ptr<Weight> PointVectorStrategy::DistanceRangeQuery::createWeight(
    shared_ptr<IndexSearcher> searcher, bool needsScores,
    float boost) 
{
  shared_ptr<Weight> w = inner->createWeight(searcher, needsScores, 1.0f);
  return make_shared<ConstantScoreWeightAnonymousInnerClass>(shared_from_this(),
                                                             boost, w);
}

PointVectorStrategy::DistanceRangeQuery::
    ConstantScoreWeightAnonymousInnerClass::
        ConstantScoreWeightAnonymousInnerClass(
            shared_ptr<DistanceRangeQuery> outerInstance, float boost,
            shared_ptr<Weight> w)
    : org::apache::lucene::search::ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
  this->w = w;
}

shared_ptr<Scorer> PointVectorStrategy::DistanceRangeQuery::
    ConstantScoreWeightAnonymousInnerClass::scorer(
        shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<Scorer> in_ = w->scorer(context);
  if (in_ == nullptr) {
    return nullptr;
  }
  shared_ptr<DoubleValues> v = outerInstance->distanceSource->getValues(
      context, DoubleValuesSource::fromScorer(in_));
  shared_ptr<DocIdSetIterator> approximation = in_->begin();
  shared_ptr<TwoPhaseIterator> twoPhase =
      make_shared<TwoPhaseIteratorAnonymousInnerClass>(shared_from_this(),
                                                       approximation, v);
  return make_shared<ConstantScoreScorer>(shared_from_this(), score(),
                                          twoPhase);
}

PointVectorStrategy::DistanceRangeQuery::
    ConstantScoreWeightAnonymousInnerClass::
        TwoPhaseIteratorAnonymousInnerClass::
            TwoPhaseIteratorAnonymousInnerClass(
                shared_ptr<ConstantScoreWeightAnonymousInnerClass>
                    outerInstance,
                shared_ptr<DocIdSetIterator> approximation,
                shared_ptr<DoubleValues> v)
    : org::apache::lucene::search::TwoPhaseIterator(approximation)
{
  this->outerInstance = outerInstance;
  this->v = v;
  this->approximation = approximation;
}

bool PointVectorStrategy::DistanceRangeQuery::
    ConstantScoreWeightAnonymousInnerClass::
        TwoPhaseIteratorAnonymousInnerClass::matches() 
{
  return v->advanceExact(approximation->docID()) &&
         v->doubleValue() <= outerInstance->outerInstance.limit;
}

float PointVectorStrategy::DistanceRangeQuery::
    ConstantScoreWeightAnonymousInnerClass::
        TwoPhaseIteratorAnonymousInnerClass::matchCost()
{
  return 100; // distance calculation can be heavy!
}

bool PointVectorStrategy::DistanceRangeQuery::
    ConstantScoreWeightAnonymousInnerClass::isCacheable(
        shared_ptr<LeafReaderContext> ctx)
{
  return outerInstance->distanceSource->isCacheable(ctx);
}

wstring PointVectorStrategy::DistanceRangeQuery::toString(const wstring &field)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"DistanceRangeQuery(" + inner->toString(field) + L"; " +
         distanceSource->toString() + L" < " + to_wstring(limit) + L")";
}

bool PointVectorStrategy::DistanceRangeQuery::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }
  shared_ptr<DistanceRangeQuery> that =
      any_cast<std::shared_ptr<DistanceRangeQuery>>(o);
  return Objects::equals(inner, that->inner) &&
         Objects::equals(distanceSource, that->distanceSource) &&
         limit == that->limit;
}

int PointVectorStrategy::DistanceRangeQuery::hashCode()
{
  return Objects::hash(inner, distanceSource, limit);
}
} // namespace org::apache::lucene::spatial::deque