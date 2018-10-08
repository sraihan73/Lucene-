using namespace std;

#include "PrefixTreeStrategy.h"

namespace org::apache::lucene::spatial::prefix
{
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using SpatialStrategy = org::apache::lucene::spatial::SpatialStrategy;
using Cell = org::apache::lucene::spatial::prefix::tree::Cell;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;
using ShapeFieldCacheDistanceValueSource =
    org::apache::lucene::spatial::util::ShapeFieldCacheDistanceValueSource;
using Bits = org::apache::lucene::util::Bits;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Shape;

PrefixTreeStrategy::PrefixTreeStrategy(shared_ptr<SpatialPrefixTree> grid,
                                       const wstring &fieldName)
    : org::apache::lucene::spatial::SpatialStrategy(grid->getSpatialContext(),
                                                    fieldName),
      grid(grid)
{
}

shared_ptr<SpatialPrefixTree> PrefixTreeStrategy::getGrid() { return grid; }

void PrefixTreeStrategy::setDefaultFieldValuesArrayLen(
    int defaultFieldValuesArrayLen)
{
  this->defaultFieldValuesArrayLen = defaultFieldValuesArrayLen;
}

double PrefixTreeStrategy::getDistErrPct() { return distErrPct; }

void PrefixTreeStrategy::setDistErrPct(double distErrPct)
{
  this->distErrPct = distErrPct;
}

bool PrefixTreeStrategy::isPointsOnly() { return pointsOnly; }

void PrefixTreeStrategy::setPointsOnly(bool pointsOnly)
{
  this->pointsOnly = pointsOnly;
}

std::deque<std::shared_ptr<Field>>
PrefixTreeStrategy::createIndexableFields(shared_ptr<Shape> shape)
{
  double distErr = SpatialArgs::calcDistanceFromErrPct(shape, distErrPct, ctx);
  return createIndexableFields(shape, distErr);
}

std::deque<std::shared_ptr<Field>>
PrefixTreeStrategy::createIndexableFields(shared_ptr<Shape> shape,
                                          double distErr)
{
  int detailLevel = grid->getLevelForDistance(distErr);
  return createIndexableFields(shape, detailLevel);
}

std::deque<std::shared_ptr<Field>>
PrefixTreeStrategy::createIndexableFields(shared_ptr<Shape> shape,
                                          int detailLevel)
{
  // TODO re-use TokenStream LUCENE-5776: Subclass Field, put cell iterator
  // there, override tokenStream()
  Iterator<std::shared_ptr<Cell>> cells =
      createCellIteratorToIndex(shape, detailLevel, nullptr);
  shared_ptr<CellToBytesRefIterator> cellToBytesRefIterator =
      newCellToBytesRefIterator();
  cellToBytesRefIterator->reset(cells);
  shared_ptr<BytesRefIteratorTokenStream> tokenStream =
      make_shared<BytesRefIteratorTokenStream>();
  tokenStream->setBytesRefIterator(cellToBytesRefIterator);
  shared_ptr<Field> field =
      make_shared<Field>(getFieldName(), tokenStream, FIELD_TYPE);
  return std::deque<std::shared_ptr<Field>>{field};
}

shared_ptr<CellToBytesRefIterator>
PrefixTreeStrategy::newCellToBytesRefIterator()
{
  // subclasses could return one that never emits leaves, or does both, or who
  // knows.
  return make_shared<CellToBytesRefIterator>();
}

shared_ptr<Iterator<std::shared_ptr<Cell>>>
PrefixTreeStrategy::createCellIteratorToIndex(
    shared_ptr<Shape> shape, int detailLevel,
    shared_ptr<Iterator<std::shared_ptr<Cell>>> reuse)
{
  if (pointsOnly && !isPointShape(shape)) {
    throw invalid_argument(L"pointsOnly is true yet a " + shape->getClass() +
                           L" is given for indexing");
  }
  return grid->getTreeCellIterator(
      shape, detailLevel); // TODO should take a re-use iterator
}

const shared_ptr<org::apache::lucene::document::FieldType>
    PrefixTreeStrategy::FIELD_TYPE =
        make_shared<org::apache::lucene::document::FieldType>();

PrefixTreeStrategy::StaticConstructor::StaticConstructor()
{
  FIELD_TYPE->setTokenized(true);
  FIELD_TYPE->setOmitNorms(true);
  FIELD_TYPE->setIndexOptions(IndexOptions::DOCS);
  FIELD_TYPE->freeze();
}

PrefixTreeStrategy::StaticConstructor PrefixTreeStrategy::staticConstructor;

shared_ptr<DoubleValuesSource>
PrefixTreeStrategy::makeDistanceValueSource(shared_ptr<Point> queryPoint,
                                            double multiplier)
{
  shared_ptr<PointPrefixTreeFieldCacheProvider> p = provider[getFieldName()];
  if (p == nullptr) {
    // C++ TODO: Multithread locking on 'this' is not converted to native C++:
    synchronized(shared_from_this())
    { // double checked locking idiom is okay since provider is threadsafe
      p = provider[getFieldName()];
      if (p == nullptr) {
        p = make_shared<PointPrefixTreeFieldCacheProvider>(
            grid, getFieldName(), defaultFieldValuesArrayLen);
        provider.emplace(getFieldName(), p);
      }
    }
  }

  return make_shared<ShapeFieldCacheDistanceValueSource>(ctx, p, queryPoint,
                                                         multiplier);
}

shared_ptr<HeatmapFacetCounter::Heatmap> PrefixTreeStrategy::calcFacets(
    shared_ptr<IndexReaderContext> context, shared_ptr<Bits> topAcceptDocs,
    shared_ptr<Shape> inputShape, int const facetLevel,
    int maxCells) 
{
  return HeatmapFacetCounter::calcFacets(shared_from_this(), context,
                                         topAcceptDocs, inputShape, facetLevel,
                                         maxCells);
}

bool PrefixTreeStrategy::isPointShape(shared_ptr<Shape> shape)
{
  return std::dynamic_pointer_cast<Point>(shape) != nullptr;
}
} // namespace org::apache::lucene::spatial::prefix