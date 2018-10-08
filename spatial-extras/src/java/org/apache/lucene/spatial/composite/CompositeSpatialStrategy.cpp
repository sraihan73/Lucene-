using namespace std;

#include "CompositeSpatialStrategy.h"

namespace org::apache::lucene::spatial::composite
{
using Field = org::apache::lucene::document::Field;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using Query = org::apache::lucene::search::Query;
using SpatialStrategy = org::apache::lucene::spatial::SpatialStrategy;
using RecursivePrefixTreeStrategy =
    org::apache::lucene::spatial::prefix::RecursivePrefixTreeStrategy;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;
using SpatialOperation = org::apache::lucene::spatial::query::SpatialOperation;
using UnsupportedSpatialOperation =
    org::apache::lucene::spatial::query::UnsupportedSpatialOperation;
using SerializedDVStrategy =
    org::apache::lucene::spatial::serialized::SerializedDVStrategy;
using ShapeValuesPredicate =
    org::apache::lucene::spatial::util::ShapeValuesPredicate;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Shape;

CompositeSpatialStrategy::CompositeSpatialStrategy(
    const wstring &fieldName,
    shared_ptr<RecursivePrefixTreeStrategy> indexStrategy,
    shared_ptr<SerializedDVStrategy> geometryStrategy)
    : org::apache::lucene::spatial::SpatialStrategy(
          indexStrategy->getSpatialContext(), fieldName),
      indexStrategy(indexStrategy), geometryStrategy(geometryStrategy)
{
}

shared_ptr<RecursivePrefixTreeStrategy>
CompositeSpatialStrategy::getIndexStrategy()
{
  return indexStrategy;
}

shared_ptr<SerializedDVStrategy> CompositeSpatialStrategy::getGeometryStrategy()
{
  return geometryStrategy;
}

bool CompositeSpatialStrategy::isOptimizePredicates()
{
  return optimizePredicates;
}

void CompositeSpatialStrategy::setOptimizePredicates(bool optimizePredicates)
{
  this->optimizePredicates = optimizePredicates;
}

std::deque<std::shared_ptr<Field>>
CompositeSpatialStrategy::createIndexableFields(shared_ptr<Shape> shape)
{
  deque<std::shared_ptr<Field>> fields = deque<std::shared_ptr<Field>>();
  Collections::addAll(fields, indexStrategy->createIndexableFields(shape));
  Collections::addAll(fields, geometryStrategy->createIndexableFields(shape));
  return fields.toArray(std::deque<std::shared_ptr<Field>>(fields.size()));
}

shared_ptr<DoubleValuesSource>
CompositeSpatialStrategy::makeDistanceValueSource(shared_ptr<Point> queryPoint,
                                                  double multiplier)
{
  // TODO consider indexing center-point in DV?  Guarantee contained by the
  // shape, which could then be used for
  // other purposes like faster WITHIN predicate?
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<Query>
CompositeSpatialStrategy::makeQuery(shared_ptr<SpatialArgs> args)
{
  shared_ptr<SpatialOperation> *const pred = args->getOperation();

  if (pred == SpatialOperation::BBoxIntersects ||
      pred == SpatialOperation::BBoxWithin) {
    throw make_shared<UnsupportedSpatialOperation>(pred);
  }

  if (pred == SpatialOperation::IsDisjointTo) {
    //      final Query intersectQuery = makeQuery(new
    //      SpatialArgs(SpatialOperation.Intersects, args.getShape()));
    //      DocValues.getDocsWithField(reader, geometryStrategy.getFieldName());
    // TODO resurrect Disjoint spatial query utility accepting a field name
    // known to have DocValues.
    // update class docs when it's added.
    throw make_shared<UnsupportedSpatialOperation>(pred);
  }

  shared_ptr<ShapeValuesPredicate> *const predicateValueSource =
      make_shared<ShapeValuesPredicate>(
          geometryStrategy->makeShapeValueSource(), pred, args->getShape());
  // System.out.println("PredOpt: " + optimizePredicates);
  if (pred == SpatialOperation::Intersects && optimizePredicates) {
    // We have a smart Intersects impl

    shared_ptr<SpatialPrefixTree> *const grid = indexStrategy->getGrid();
    constexpr int detailLevel = grid->getLevelForDistance(
        args->resolveDistErr(ctx, 0.0)); // default to max precision
    return make_shared<IntersectsRPTVerifyQuery>(
        args->getShape(), indexStrategy->getFieldName(), grid, detailLevel,
        indexStrategy->getPrefixGridScanLevel(), predicateValueSource);
  } else {
    // The general path; all index matches get verified

    shared_ptr<SpatialArgs> indexArgs;
    if (pred == SpatialOperation::Contains) {
      // note: we could map_obj IsWithin as well but it's pretty darned slow since
      // it touches all world grids
      indexArgs = args;
    } else {
      // TODO add args.clone method with new predicate? Or simply make
      // non-final?
      indexArgs = make_shared<SpatialArgs>(SpatialOperation::Intersects,
                                           args->getShape());
      indexArgs->setDistErr(args->getDistErr());
      indexArgs->setDistErrPct(args->getDistErrPct());
    }

    if (!indexArgs->getDistErr() && !indexArgs->getDistErrPct()) {
      indexArgs->setDistErrPct(0.10);
    }

    shared_ptr<Query> *const indexQuery = indexStrategy->makeQuery(indexArgs);
    return make_shared<CompositeVerifyQuery>(indexQuery, predicateValueSource);
  }
}
} // namespace org::apache::lucene::spatial::composite