using namespace std;

#include "WithinPrefixTreeQuery.h"

namespace org::apache::lucene::spatial::prefix
{
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::distance::DistanceUtils;
using org::locationtech::spatial4j::shape::Circle;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::SpatialRelation;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DocIdSet = org::apache::lucene::search::DocIdSet;
using Cell = org::apache::lucene::spatial::prefix::tree::Cell;
using CellIterator = org::apache::lucene::spatial::prefix::tree::CellIterator;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using BitDocIdSet = org::apache::lucene::util::BitDocIdSet;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;

WithinPrefixTreeQuery::WithinPrefixTreeQuery(shared_ptr<Shape> queryShape,
                                             const wstring &fieldName,
                                             shared_ptr<SpatialPrefixTree> grid,
                                             int detailLevel,
                                             int prefixGridScanLevel,
                                             double queryBuffer)
    : AbstractVisitingPrefixTreeQuery(queryShape, fieldName, grid, detailLevel,
                                      prefixGridScanLevel),
      bufferedQueryShape(
          queryBuffer == -1 ? nullptr : bufferShape(queryShape, queryBuffer))
{
}

bool WithinPrefixTreeQuery::equals(any o)
{
  if (!AbstractVisitingPrefixTreeQuery::equals(o)) {
    return false; // checks getClass == o.getClass & instanceof
  }

  shared_ptr<WithinPrefixTreeQuery> that =
      any_cast<std::shared_ptr<WithinPrefixTreeQuery>>(o);

  if (bufferedQueryShape != nullptr
          ? !bufferedQueryShape->equals(that->bufferedQueryShape)
          : that->bufferedQueryShape != nullptr) {
    return false;
  }

  return true;
}

int WithinPrefixTreeQuery::hashCode()
{
  int result = AbstractVisitingPrefixTreeQuery::hashCode();
  result = 31 * result +
           (bufferedQueryShape != nullptr ? bufferedQueryShape->hashCode() : 0);
  return result;
}

wstring WithinPrefixTreeQuery::toString(const wstring &field)
{
  return getClass().getSimpleName() + L"(" + L"fieldName=" + fieldName + L"," +
         L"queryShape=" + queryShape + L"," + L"detailLevel=" +
         to_wstring(detailLevel) + L"," + L"prefixGridScanLevel=" +
         to_wstring(prefixGridScanLevel) + L")";
}

shared_ptr<Shape> WithinPrefixTreeQuery::bufferShape(shared_ptr<Shape> shape,
                                                     double distErr)
{
  if (distErr <= 0) {
    throw invalid_argument(L"distErr must be > 0");
  }
  shared_ptr<SpatialContext> ctx = grid->getSpatialContext();
  if (std::dynamic_pointer_cast<Point>(shape) != nullptr) {
    return ctx->makeCircle(std::static_pointer_cast<Point>(shape), distErr);
  } else if (std::dynamic_pointer_cast<Circle>(shape) != nullptr) {
    shared_ptr<Circle> circle = std::static_pointer_cast<Circle>(shape);
    double newDist = circle->getRadius() + distErr;
    if (ctx->isGeo() && newDist > 180) {
      newDist = 180;
    }
    return ctx->makeCircle(circle->getCenter(), newDist);
  } else {
    shared_ptr<Rectangle> bbox = shape->getBoundingBox();
    double newMinX = bbox->getMinX() - distErr;
    double newMaxX = bbox->getMaxX() + distErr;
    double newMinY = bbox->getMinY() - distErr;
    double newMaxY = bbox->getMaxY() + distErr;
    if (ctx->isGeo()) {
      if (newMinY < -90) {
        newMinY = -90;
      }
      if (newMaxY > 90) {
        newMaxY = 90;
      }
      if (newMinY == -90 || newMaxY == 90 ||
          bbox->getWidth() + 2 * distErr > 360) {
        newMinX = -180;
        newMaxX = 180;
      } else {
        newMinX = DistanceUtils::normLonDEG(newMinX);
        newMaxX = DistanceUtils::normLonDEG(newMaxX);
      }
    } else {
      // restrict to world bounds
      newMinX = max(newMinX, ctx->getWorldBounds().getMinX());
      newMaxX = min(newMaxX, ctx->getWorldBounds().getMaxX());
      newMinY = max(newMinY, ctx->getWorldBounds().getMinY());
      newMaxY = min(newMaxY, ctx->getWorldBounds().getMaxY());
    }
    return ctx->makeRectangle(newMinX, newMaxX, newMinY, newMaxY);
  }
}

shared_ptr<DocIdSet> WithinPrefixTreeQuery::getDocIdSet(
    shared_ptr<LeafReaderContext> context) 
{
  return make_shared<VisitorTemplateAnonymousInnerClass>(shared_from_this(),
                                                         context)
      .getDocIdSet();
}

WithinPrefixTreeQuery::VisitorTemplateAnonymousInnerClass::
    VisitorTemplateAnonymousInnerClass(
        shared_ptr<WithinPrefixTreeQuery> outerInstance,
        shared_ptr<LeafReaderContext> context)
    : VisitorTemplate(outerInstance, context)
{
  this->outerInstance = outerInstance;
}

void WithinPrefixTreeQuery::VisitorTemplateAnonymousInnerClass::start()
{
  inside = make_shared<FixedBitSet>(maxDoc);
  outside = make_shared<FixedBitSet>(maxDoc);
}

shared_ptr<DocIdSet>
WithinPrefixTreeQuery::VisitorTemplateAnonymousInnerClass::finish()
{
  inside::andNot(outside);
  return make_shared<BitDocIdSet>(inside);
}

shared_ptr<CellIterator>
WithinPrefixTreeQuery::VisitorTemplateAnonymousInnerClass::findSubCellsToVisit(
    shared_ptr<Cell> cell)
{
  // use buffered query shape instead of orig.  Works with null too.
  return cell->getNextLevelCells(outerInstance->bufferedQueryShape);
}

bool WithinPrefixTreeQuery::VisitorTemplateAnonymousInnerClass::visitPrefix(
    shared_ptr<Cell> cell) 
{
  // cell.relate is based on the bufferedQueryShape; we need to examine what
  // the relation is against the queryShape
  shared_ptr<SpatialRelation> visitRelation =
      cell->getShape()->relate(outerInstance->queryShape);
  if (cell->getLevel() == outerInstance->detailLevel) {
    collectDocs(visitRelation->intersects() ? inside : outside);
    return false;
  } else if (visitRelation == SpatialRelation::WITHIN) {
    collectDocs(inside);
    return false;
  } else if (visitRelation == SpatialRelation::DISJOINT) {
    collectDocs(outside);
    return false;
  }
  return true;
}

void WithinPrefixTreeQuery::VisitorTemplateAnonymousInnerClass::visitLeaf(
    shared_ptr<Cell> cell) 
{
  if (allCellsIntersectQuery(cell)) {
    collectDocs(inside);
  } else {
    collectDocs(outside);
  }
}

bool WithinPrefixTreeQuery::VisitorTemplateAnonymousInnerClass::
    allCellsIntersectQuery(shared_ptr<Cell> cell)
{
  shared_ptr<SpatialRelation> relate =
      cell->getShape()->relate(outerInstance->queryShape);
  if (cell->getLevel() == outerInstance->detailLevel) {
    return relate->intersects();
  }
  if (relate == SpatialRelation::WITHIN) {
    return true;
  }
  if (relate == SpatialRelation::DISJOINT) {
    return false;
  }
  // Note: Generating all these cells just to determine intersection is not
  // ideal. The real solution is LUCENE-4869.
  shared_ptr<CellIterator> subCells = cell->getNextLevelCells(nullptr);
  while (subCells->hasNext()) {
    shared_ptr<Cell> subCell = subCells->next();
    if (!allCellsIntersectQuery(subCell)) // recursion
    {
      return false;
    }
    subCells++;
  }
  return true;
}

void WithinPrefixTreeQuery::VisitorTemplateAnonymousInnerClass::visitScanned(
    shared_ptr<Cell> cell) 
{
  visitLeaf(cell); // collects as we want, even if not a leaf
                   //        if (cell.isLeaf()) {
                   //          visitLeaf(cell);
                   //        } else {
                   //          visitPrefix(cell);
                   //        }
}
} // namespace org::apache::lucene::spatial::prefix