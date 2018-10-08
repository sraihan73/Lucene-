using namespace std;

#include "ParentChildrenBlockJoinQuery.h"

namespace org::apache::lucene::search::join
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using ReaderUtil = org::apache::lucene::index::ReaderUtil;
using Term = org::apache::lucene::index::Term;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using Weight = org::apache::lucene::search::Weight;
using BitSet = org::apache::lucene::util::BitSet;

ParentChildrenBlockJoinQuery::ParentChildrenBlockJoinQuery(
    shared_ptr<BitSetProducer> parentFilter, shared_ptr<Query> childQuery,
    int parentDocId)
    : parentFilter(parentFilter), childQuery(childQuery),
      parentDocId(parentDocId)
{
}

bool ParentChildrenBlockJoinQuery::equals(any obj)
{
  if (sameClassAs(obj) == false) {
    return false;
  }
  shared_ptr<ParentChildrenBlockJoinQuery> other =
      any_cast<std::shared_ptr<ParentChildrenBlockJoinQuery>>(obj);
  return parentFilter->equals(other->parentFilter) &&
         childQuery->equals(other->childQuery) &&
         parentDocId == other->parentDocId;
}

int ParentChildrenBlockJoinQuery::hashCode()
{
  int hash = classHash();
  hash = 31 * hash + parentFilter->hashCode();
  hash = 31 * hash + childQuery->hashCode();
  hash = 31 * hash + parentDocId;
  return hash;
}

wstring ParentChildrenBlockJoinQuery::toString(const wstring &field)
{
  return L"ParentChildrenBlockJoinQuery (" + childQuery + L")";
}

shared_ptr<Query> ParentChildrenBlockJoinQuery::rewrite(
    shared_ptr<IndexReader> reader) 
{
  shared_ptr<Query> *const childRewrite = childQuery->rewrite(reader);
  if (childRewrite != childQuery) {
    return make_shared<ParentChildrenBlockJoinQuery>(parentFilter, childRewrite,
                                                     parentDocId);
  } else {
    return Query::rewrite(reader);
  }
}

shared_ptr<Weight>
ParentChildrenBlockJoinQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                           bool needsScores,
                                           float boost) 
{
  shared_ptr<Weight> *const childWeight =
      childQuery->createWeight(searcher, needsScores, boost);
  constexpr int readerIndex =
      ReaderUtil::subIndex(parentDocId, searcher->getIndexReader()->leaves());
  return make_shared<WeightAnonymousInnerClass>(shared_from_this(), childWeight,
                                                readerIndex);
}

ParentChildrenBlockJoinQuery::WeightAnonymousInnerClass::
    WeightAnonymousInnerClass(
        shared_ptr<ParentChildrenBlockJoinQuery> outerInstance,
        shared_ptr<Weight> childWeight, int readerIndex)
    : org::apache::lucene::search::Weight(outerInstance)
{
  this->outerInstance = outerInstance;
  this->childWeight = childWeight;
  this->readerIndex = readerIndex;
}

void ParentChildrenBlockJoinQuery::WeightAnonymousInnerClass::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  childWeight->extractTerms(terms);
}

shared_ptr<Explanation>
ParentChildrenBlockJoinQuery::WeightAnonymousInnerClass::explain(
    shared_ptr<LeafReaderContext> context, int doc) 
{
  return Explanation::noMatch(L"Not implemented, use ToParentBlockJoinQuery "
                              L"explain why a document matched");
}

shared_ptr<Scorer>
ParentChildrenBlockJoinQuery::WeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  // Childs docs only reside in a single segment, so no need to evaluate all
  // segments
  if (context->ord != readerIndex) {
    return nullptr;
  }

  constexpr int localParentDocId =
      outerInstance->parentDocId - context->docBase;
  // If parentDocId == 0 then a parent doc doesn't have child docs, because
  // child docs are stored before the parent doc and because parent doc is 0 we
  // can safely assume that there are no child docs.
  if (localParentDocId == 0) {
    return nullptr;
  }

  shared_ptr<BitSet> *const parents =
      outerInstance->parentFilter->getBitSet(context);
  constexpr int firstChildDocId = parents->prevSetBit(localParentDocId - 1) + 1;
  // A parent doc doesn't have child docs, so we can early exit here:
  if (firstChildDocId == localParentDocId) {
    return nullptr;
  }

  shared_ptr<Scorer> *const childrenScorer = childWeight->scorer(context);
  if (childrenScorer == nullptr) {
    return nullptr;
  }
  shared_ptr<DocIdSetIterator> childrenIterator = childrenScorer->begin();
  shared_ptr<DocIdSetIterator> *const it =
      make_shared<DocIdSetIteratorAnonymousInnerClass>(
          shared_from_this(), localParentDocId, firstChildDocId,
          childrenIterator);
  return make_shared<ScorerAnonymousInnerClass>(shared_from_this(),
                                                childrenScorer, it);
}

ParentChildrenBlockJoinQuery::WeightAnonymousInnerClass::
    DocIdSetIteratorAnonymousInnerClass::DocIdSetIteratorAnonymousInnerClass(
        shared_ptr<WeightAnonymousInnerClass> outerInstance,
        int localParentDocId, int firstChildDocId,
        shared_ptr<DocIdSetIterator> childrenIterator)
{
  this->outerInstance = outerInstance;
  this->localParentDocId = localParentDocId;
  this->firstChildDocId = firstChildDocId;
  this->childrenIterator = childrenIterator;
  doc = -1;
}

int ParentChildrenBlockJoinQuery::WeightAnonymousInnerClass::
    DocIdSetIteratorAnonymousInnerClass::docID()
{
  return doc;
}

int ParentChildrenBlockJoinQuery::WeightAnonymousInnerClass::
    DocIdSetIteratorAnonymousInnerClass::nextDoc() 
{
  return advance(doc + 1);
}

int ParentChildrenBlockJoinQuery::WeightAnonymousInnerClass::
    DocIdSetIteratorAnonymousInnerClass::advance(int target) 
{
  target = max(firstChildDocId, target);
  if (target >= localParentDocId) {
    // We're outside the child nested scope, so it is done
    return doc = DocIdSetIterator::NO_MORE_DOCS;
  } else {
    int advanced = childrenIterator->advance(target);
    if (advanced >= localParentDocId) {
      // We're outside the child nested scope, so it is done
      return doc = DocIdSetIterator::NO_MORE_DOCS;
    } else {
      return doc = advanced;
    }
  }
}

int64_t ParentChildrenBlockJoinQuery::WeightAnonymousInnerClass::
    DocIdSetIteratorAnonymousInnerClass::cost()
{
  return min(childrenIterator->cost(), localParentDocId - firstChildDocId);
}

ParentChildrenBlockJoinQuery::WeightAnonymousInnerClass::
    ScorerAnonymousInnerClass::ScorerAnonymousInnerClass(
        shared_ptr<WeightAnonymousInnerClass> outerInstance,
        shared_ptr<Scorer> childrenScorer, shared_ptr<DocIdSetIterator> it)
    : org::apache::lucene::search::Scorer(outerInstance)
{
  this->outerInstance = outerInstance;
  this->childrenScorer = childrenScorer;
  this->it = it;
}

int ParentChildrenBlockJoinQuery::WeightAnonymousInnerClass::
    ScorerAnonymousInnerClass::docID()
{
  return it->docID();
}

float ParentChildrenBlockJoinQuery::WeightAnonymousInnerClass::
    ScorerAnonymousInnerClass::score() 
{
  return childrenScorer->score();
}

shared_ptr<DocIdSetIterator> ParentChildrenBlockJoinQuery::
    WeightAnonymousInnerClass::ScorerAnonymousInnerClass::iterator()
{
  return it;
}

bool ParentChildrenBlockJoinQuery::WeightAnonymousInnerClass::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return false; // TODO delegate to BitSetProducer?
}
} // namespace org::apache::lucene::search::join