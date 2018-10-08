using namespace std;

#include "IntersectsPrefixTreeQuery.h"

namespace org::apache::lucene::spatial::prefix
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DocIdSet = org::apache::lucene::search::DocIdSet;
using Cell = org::apache::lucene::spatial::prefix::tree::Cell;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using DocIdSetBuilder = org::apache::lucene::util::DocIdSetBuilder;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::SpatialRelation;

IntersectsPrefixTreeQuery::IntersectsPrefixTreeQuery(
    shared_ptr<Shape> queryShape, const wstring &fieldName,
    shared_ptr<SpatialPrefixTree> grid, int detailLevel,
    int prefixGridScanLevel)
    : AbstractVisitingPrefixTreeQuery(queryShape, fieldName, grid, detailLevel,
                                      prefixGridScanLevel)
{
}

shared_ptr<DocIdSet> IntersectsPrefixTreeQuery::getDocIdSet(
    shared_ptr<LeafReaderContext> context) 
{
  /* Possible optimizations (in IN ADDITION TO THOSE LISTED IN VISITORTEMPLATE):

  * If docFreq is 1 (or < than some small threshold), then check to see if we've
  already collected it; if so short-circuit. Don't do this just for point data,
  as there is no benefit, or only marginal benefit when multi-valued.

  * Point query shape optimization when the only indexed data is a point (no
  leaves).  Result is a term query.

   */
  return make_shared<VisitorTemplateAnonymousInnerClass>(shared_from_this(),
                                                         context)
      .getDocIdSet();
}

IntersectsPrefixTreeQuery::VisitorTemplateAnonymousInnerClass::
    VisitorTemplateAnonymousInnerClass(
        shared_ptr<IntersectsPrefixTreeQuery> outerInstance,
        shared_ptr<LeafReaderContext> context)
    : VisitorTemplate(outerInstance, context)
{
  this->outerInstance = outerInstance;
}

void IntersectsPrefixTreeQuery::VisitorTemplateAnonymousInnerClass::
    start() 
{
  results = make_shared<DocIdSetBuilder>(maxDoc, terms);
}

shared_ptr<DocIdSet>
IntersectsPrefixTreeQuery::VisitorTemplateAnonymousInnerClass::finish()
{
  return results::build();
}

bool IntersectsPrefixTreeQuery::VisitorTemplateAnonymousInnerClass::visitPrefix(
    shared_ptr<Cell> cell) 
{
  if (cell->getShapeRel() == SpatialRelation::WITHIN ||
      cell->getLevel() == outerInstance->detailLevel) {
    collectDocs(results);
    return false;
  }
  return true;
}

void IntersectsPrefixTreeQuery::VisitorTemplateAnonymousInnerClass::visitLeaf(
    shared_ptr<Cell> cell) 
{
  collectDocs(results);
}

wstring IntersectsPrefixTreeQuery::toString(const wstring &field)
{
  return getClass().getSimpleName() + L"(" + L"fieldName=" + fieldName + L"," +
         L"queryShape=" + queryShape + L"," + L"detailLevel=" +
         to_wstring(detailLevel) + L"," + L"prefixGridScanLevel=" +
         to_wstring(prefixGridScanLevel) + L")";
}
} // namespace org::apache::lucene::spatial::prefix