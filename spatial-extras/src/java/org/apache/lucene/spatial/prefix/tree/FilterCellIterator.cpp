using namespace std;

#include "FilterCellIterator.h"

namespace org::apache::lucene::spatial::prefix::tree
{
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::SpatialRelation;

FilterCellIterator::FilterCellIterator(
    shared_ptr<Iterator<std::shared_ptr<Cell>>> baseIter,
    shared_ptr<Shape> shapeFilter)
    : baseIter(baseIter), shapeFilter(shapeFilter)
{
}

bool FilterCellIterator::hasNext()
{
  thisCell_.reset();
  if (nextCell != nullptr) // calling hasNext twice in a row
  {
    return true;
  }
  while (baseIter->hasNext()) {
    nextCell = baseIter->next();
    if (shapeFilter == nullptr) {
      return true;
    } else {
      shared_ptr<SpatialRelation> rel =
          nextCell->getShape()->relate(shapeFilter);
      if (rel->intersects()) {
        nextCell->setShapeRel(rel);
        if (rel == SpatialRelation::WITHIN) {
          nextCell->setLeaf();
        }
        return true;
      }
    }
    baseIter++;
  }
  return false;
}
} // namespace org::apache::lucene::spatial::prefix::tree