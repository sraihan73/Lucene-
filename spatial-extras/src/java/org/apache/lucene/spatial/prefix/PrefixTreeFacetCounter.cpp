using namespace std;

#include "PrefixTreeFacetCounter.h"

namespace org::apache::lucene::spatial::prefix
{
using org::locationtech::spatial4j::shape::Shape;
using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using DocIdSet = org::apache::lucene::search::DocIdSet;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Cell = org::apache::lucene::spatial::prefix::tree::Cell;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using Bits = org::apache::lucene::util::Bits;

void PrefixTreeFacetCounter::FacetVisitor::startOfSegment() {}

PrefixTreeFacetCounter::PrefixTreeFacetCounter() {}

void PrefixTreeFacetCounter::compute(
    shared_ptr<PrefixTreeStrategy> strategy,
    shared_ptr<IndexReaderContext> context, shared_ptr<Bits> topAcceptDocs,
    shared_ptr<Shape> queryShape, int facetLevel,
    shared_ptr<FacetVisitor> facetVisitor) 
{
  // We collect per-leaf
  for (auto leafCtx : context->leaves()) {
    // determine leaf acceptDocs Bits
    shared_ptr<Bits> leafAcceptDocs;
    if (topAcceptDocs == nullptr) {
      leafAcceptDocs = leafCtx->reader()->getLiveDocs(); // filter deleted
    } else {
      leafAcceptDocs =
          make_shared<BitsAnonymousInnerClass>(topAcceptDocs, leafCtx);
    }

    compute(strategy, leafCtx, leafAcceptDocs, queryShape, facetLevel,
            facetVisitor);
  }
}

PrefixTreeFacetCounter::BitsAnonymousInnerClass::BitsAnonymousInnerClass(
    shared_ptr<Bits> topAcceptDocs, shared_ptr<LeafReaderContext> leafCtx)
{
  this->topAcceptDocs = topAcceptDocs;
  this->leafCtx = leafCtx;
}

bool PrefixTreeFacetCounter::BitsAnonymousInnerClass::get(int index)
{
  return topAcceptDocs->get(leafCtx->docBase + index);
}

int PrefixTreeFacetCounter::BitsAnonymousInnerClass::length()
{
  return leafCtx->reader()->maxDoc();
}

void PrefixTreeFacetCounter::compute(
    shared_ptr<PrefixTreeStrategy> strategy,
    shared_ptr<LeafReaderContext> context, shared_ptr<Bits> acceptDocs,
    shared_ptr<Shape> queryShape, int const facetLevel,
    shared_ptr<FacetVisitor> facetVisitor) 
{
  if (acceptDocs != nullptr &&
      acceptDocs->length() != context->reader()->maxDoc()) {
    throw invalid_argument(
        L"acceptDocs bits length " + to_wstring(acceptDocs->length()) +
        L" != leaf maxdoc " + to_wstring(context->reader()->maxDoc()));
  }
  shared_ptr<SpatialPrefixTree> *const tree = strategy->getGrid();

  // scanLevel is an optimization knob of AbstractVisitingPrefixTreeFilter. It's
  // unlikely
  // another scanLevel would be much faster and it tends to be a risky knob (can
  // help a little, can hurt a ton).
  // TODO use RPT's configured scan level?  Do we know better here?  Hard to
  // say.
  constexpr int scanLevel = tree->getMaxLevels();
  // AbstractVisitingPrefixTreeFilter is a Lucene Filter.  We don't need a
  // filter; we use it for its great prefix-tree
  // traversal code.  TODO consider refactoring if/when it makes sense (more use
  // cases than this)
  make_shared<AbstractVisitingPrefixTreeQueryAnonymousInnerClass>(
      queryShape, strategy->getFieldName(), facetLevel, context, acceptDocs,
      facetVisitor)
      .getDocIdSet(context);
}

PrefixTreeFacetCounter::AbstractVisitingPrefixTreeQueryAnonymousInnerClass::
    AbstractVisitingPrefixTreeQueryAnonymousInnerClass(
        shared_ptr<Shape> queryShape, const wstring &getFieldName,
        int facetLevel, shared_ptr<LeafReaderContext> context,
        shared_ptr<Bits> acceptDocs,
        shared_ptr<org::apache::lucene::spatial::prefix::
                       PrefixTreeFacetCounter::FacetVisitor>
            facetVisitor)
    : AbstractVisitingPrefixTreeQuery(queryShape, getFieldName, tree,
                                      facetLevel, scanLevel)
{
  this->context = context;
  this->acceptDocs = acceptDocs;
  this->facetLevel = facetLevel;
  this->facetVisitor = facetVisitor;
}

wstring
PrefixTreeFacetCounter::AbstractVisitingPrefixTreeQueryAnonymousInnerClass::
    toString(const wstring &field)
{
  return L"anonPrefixTreeQuery"; // un-used
}

shared_ptr<DocIdSet>
PrefixTreeFacetCounter::AbstractVisitingPrefixTreeQueryAnonymousInnerClass::
    getDocIdSet(shared_ptr<LeafReaderContext> contexts) 
{
  assert(facetLevel ==
         AbstractVisitingPrefixTreeQuery::detailLevel); // same thing, FYI.
                                                        // (constant)

  return make_shared<VisitorTemplateAnonymousInnerClass>(shared_from_this(),
                                                         context)
      .getDocIdSet();
}

PrefixTreeFacetCounter::AbstractVisitingPrefixTreeQueryAnonymousInnerClass::
    VisitorTemplateAnonymousInnerClass::VisitorTemplateAnonymousInnerClass(
        shared_ptr<AbstractVisitingPrefixTreeQueryAnonymousInnerClass>
            outerInstance,
        shared_ptr<LeafReaderContext> context)
    : VisitorTemplate(context)
{
  this->outerInstance = outerInstance;
}

void PrefixTreeFacetCounter::
    AbstractVisitingPrefixTreeQueryAnonymousInnerClass::
        VisitorTemplateAnonymousInnerClass::start() 
{
  outerInstance->facetVisitor.startOfSegment();
}

shared_ptr<DocIdSet>
PrefixTreeFacetCounter::AbstractVisitingPrefixTreeQueryAnonymousInnerClass::
    VisitorTemplateAnonymousInnerClass::finish() 
{
  return nullptr; // unused;
}

bool PrefixTreeFacetCounter::
    AbstractVisitingPrefixTreeQueryAnonymousInnerClass::
        VisitorTemplateAnonymousInnerClass::visitPrefix(
            shared_ptr<Cell> cell) 
{
  // At facetLevel...
  if (cell->getLevel() == outerInstance->facetLevel) {
    // Count docs
    visitLeaf(cell); // we're not a leaf but we treat it as such at facet level
    return false;    // don't descend further; this is enough detail
  }

  // We optimize for discriminating filters (reflected in acceptDocs) and
  // short-circuit if no matching docs. We could do this at all levels or never
  // but the closer we get to the facet level, the higher the probability this
  // is worthwhile. We do when docFreq == 1 because it's a cheap check,
  // especially due to "pulsing" in the codec.
  // TODO this opt should move to VisitorTemplate (which contains an
  // optimization TODO to this effect)
  if (cell->getLevel() == outerInstance->facetLevel - 1 ||
      termsEnum::docFreq() == 1) {
    if (!hasDocsAtThisTerm()) {
      return false;
    }
  }
  return true;
}

void PrefixTreeFacetCounter::
    AbstractVisitingPrefixTreeQueryAnonymousInnerClass::
        VisitorTemplateAnonymousInnerClass::visitLeaf(
            shared_ptr<Cell> cell) 
{
  constexpr int count = countDocsAtThisTerm();
  if (count > 0) {
    outerInstance->facetVisitor.visit(cell, count);
  }
}

int PrefixTreeFacetCounter::AbstractVisitingPrefixTreeQueryAnonymousInnerClass::
    VisitorTemplateAnonymousInnerClass::countDocsAtThisTerm() 
{
  if (outerInstance->acceptDocs == nullptr) {
    return termsEnum::docFreq();
  }
  int count = 0;
  postingsEnum = termsEnum::postings(postingsEnum, PostingsEnum::NONE);
  while (postingsEnum::nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
    if (outerInstance->acceptDocs->get(postingsEnum::docID()) == false) {
      continue;
    }
    count++;
  }
  return count;
}

bool PrefixTreeFacetCounter::
    AbstractVisitingPrefixTreeQueryAnonymousInnerClass::
        VisitorTemplateAnonymousInnerClass::hasDocsAtThisTerm() throw(
            IOException)
{
  if (outerInstance->acceptDocs == nullptr) {
    return true;
  }
  postingsEnum = termsEnum::postings(postingsEnum, PostingsEnum::NONE);
  int nextDoc = postingsEnum::nextDoc();
  while (nextDoc != DocIdSetIterator::NO_MORE_DOCS &&
         outerInstance->acceptDocs->get(nextDoc) == false) {
    nextDoc = postingsEnum::nextDoc();
  }
  return nextDoc != DocIdSetIterator::NO_MORE_DOCS;
}
} // namespace org::apache::lucene::spatial::prefix