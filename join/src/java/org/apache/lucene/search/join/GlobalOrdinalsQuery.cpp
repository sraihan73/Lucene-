using namespace std;

#include "GlobalOrdinalsQuery.h"

namespace org::apache::lucene::search::join
{
using DocValues = org::apache::lucene::index::DocValues;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using OrdinalMap = org::apache::lucene::index::OrdinalMap;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using Term = org::apache::lucene::index::Term;
using ConstantScoreWeight = org::apache::lucene::search::ConstantScoreWeight;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;
using Weight = org::apache::lucene::search::Weight;
using BytesRef = org::apache::lucene::util::BytesRef;
using LongBitSet = org::apache::lucene::util::LongBitSet;
using LongValues = org::apache::lucene::util::LongValues;

GlobalOrdinalsQuery::GlobalOrdinalsQuery(shared_ptr<LongBitSet> foundOrds,
                                         const wstring &joinField,
                                         shared_ptr<OrdinalMap> globalOrds,
                                         shared_ptr<Query> toQuery,
                                         shared_ptr<Query> fromQuery,
                                         any indexReaderContextId)
    : foundOrds(foundOrds), joinField(joinField), globalOrds(globalOrds),
      toQuery(toQuery), fromQuery(fromQuery),
      indexReaderContextId(indexReaderContextId)
{
}

shared_ptr<Weight>
GlobalOrdinalsQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                  bool needsScores,
                                  float boost) 
{
  if (searcher->getTopReaderContext()->id() != indexReaderContextId) {
    throw make_shared<IllegalStateException>(
        L"Creating the weight against a different index reader than this query "
        L"has been built for.");
  }
  return make_shared<W>(shared_from_this(), shared_from_this(),
                        toQuery->createWeight(searcher, false, 1.0f), boost);
}

bool GlobalOrdinalsQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool GlobalOrdinalsQuery::equalsTo(shared_ptr<GlobalOrdinalsQuery> other)
{
  return fromQuery->equals(other->fromQuery) && joinField == other->joinField &&
         toQuery->equals(other->toQuery) &&
         indexReaderContextId.equals(other->indexReaderContextId);
}

int GlobalOrdinalsQuery::hashCode()
{
  int result = classHash();
  result = 31 * result + joinField.hashCode();
  result = 31 * result + toQuery->hashCode();
  result = 31 * result + fromQuery->hashCode();
  result = 31 * result + indexReaderContextId.hashCode();
  return result;
}

wstring GlobalOrdinalsQuery::toString(const wstring &field)
{
  return wstring(L"GlobalOrdinalsQuery{") + L"joinField=" + joinField + L'}';
}

GlobalOrdinalsQuery::W::W(shared_ptr<GlobalOrdinalsQuery> outerInstance,
                          shared_ptr<Query> query,
                          shared_ptr<Weight> approximationWeight, float boost)
    : org::apache::lucene::search::ConstantScoreWeight(query, boost),
      approximationWeight(approximationWeight), outerInstance(outerInstance)
{
}

void GlobalOrdinalsQuery::W::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
}

shared_ptr<Explanation>
GlobalOrdinalsQuery::W::explain(shared_ptr<LeafReaderContext> context,
                                int doc) 
{
  shared_ptr<SortedDocValues> values =
      DocValues::getSorted(context->reader(), outerInstance->joinField);
  if (values == nullptr) {
    return Explanation::noMatch(L"Not a match");
  }

  if (values->advance(doc) != doc) {
    return Explanation::noMatch(L"Not a match");
  }
  int segmentOrd = values->ordValue();
  shared_ptr<BytesRef> joinValue = values->lookupOrd(segmentOrd);

  int ord;
  if (outerInstance->globalOrds != nullptr) {
    ord =
        static_cast<int>(outerInstance->globalOrds->getGlobalOrds(context->ord)
                             ->get(segmentOrd));
  } else {
    ord = segmentOrd;
  }
  if (outerInstance->foundOrds->get(ord) == false) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return Explanation::noMatch(L"Not a match, join value " +
                                Term::toString(joinValue));
  }

  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Explanation::match(score(), L"A match, join value " +
                                         Term::toString(joinValue));
}

shared_ptr<Scorer> GlobalOrdinalsQuery::W::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<SortedDocValues> values =
      DocValues::getSorted(context->reader(), outerInstance->joinField);
  if (values == nullptr) {
    return nullptr;
  }

  shared_ptr<Scorer> approximationScorer = approximationWeight->scorer(context);
  if (approximationScorer == nullptr) {
    return nullptr;
  }
  if (outerInstance->globalOrds != nullptr) {
    return make_shared<OrdinalMapScorer>(
        shared_from_this(), score(), outerInstance->foundOrds, values,
        approximationScorer->begin(),
        outerInstance->globalOrds->getGlobalOrds(context->ord));
  }
  {
    return make_shared<SegmentOrdinalScorer>(shared_from_this(), score(),
                                             outerInstance->foundOrds, values,
                                             approximationScorer->begin());
  }
}

bool GlobalOrdinalsQuery::W::isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  // disable caching because this query relies on a top reader context
  // and holds a bitset of matching ordinals that cannot be accounted in
  // the memory used by the cache
  return false;
}

GlobalOrdinalsQuery::OrdinalMapScorer::OrdinalMapScorer(
    shared_ptr<Weight> weight, float score, shared_ptr<LongBitSet> foundOrds,
    shared_ptr<SortedDocValues> values,
    shared_ptr<DocIdSetIterator> approximationScorer,
    shared_ptr<LongValues> segmentOrdToGlobalOrdLookup)
    : BaseGlobalOrdinalScorer(weight, values, approximationScorer),
      foundOrds(foundOrds),
      segmentOrdToGlobalOrdLookup(segmentOrdToGlobalOrdLookup)
{
  this->score_ = score;
}

shared_ptr<TwoPhaseIterator>
GlobalOrdinalsQuery::OrdinalMapScorer::createTwoPhaseIterator(
    shared_ptr<DocIdSetIterator> approximation)
{
  return make_shared<TwoPhaseIteratorAnonymousInnerClass>(shared_from_this(),
                                                          approximation);
}

GlobalOrdinalsQuery::OrdinalMapScorer::TwoPhaseIteratorAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass(
        shared_ptr<OrdinalMapScorer> outerInstance,
        shared_ptr<DocIdSetIterator> approximation)
    : org::apache::lucene::search::TwoPhaseIterator(approximation)
{
  this->outerInstance = outerInstance;
  this->approximation = approximation;
}

bool GlobalOrdinalsQuery::OrdinalMapScorer::
    TwoPhaseIteratorAnonymousInnerClass::matches() 
{
  if (outerInstance->values->advanceExact(approximation->docID())) {
    constexpr int64_t segmentOrd = outerInstance->values->ordValue();
    constexpr int64_t globalOrd =
        outerInstance->segmentOrdToGlobalOrdLookup->get(segmentOrd);
    if (outerInstance->foundOrds->get(globalOrd)) {
      return true;
    }
  }
  return false;
}

float GlobalOrdinalsQuery::OrdinalMapScorer::
    TwoPhaseIteratorAnonymousInnerClass::matchCost()
{
  return 100; // TODO: use cost of values.getOrd() and foundOrds.get()
}

GlobalOrdinalsQuery::SegmentOrdinalScorer::SegmentOrdinalScorer(
    shared_ptr<Weight> weight, float score, shared_ptr<LongBitSet> foundOrds,
    shared_ptr<SortedDocValues> values,
    shared_ptr<DocIdSetIterator> approximationScorer)
    : BaseGlobalOrdinalScorer(weight, values, approximationScorer),
      foundOrds(foundOrds)
{
  this->score_ = score;
}

shared_ptr<TwoPhaseIterator>
GlobalOrdinalsQuery::SegmentOrdinalScorer::createTwoPhaseIterator(
    shared_ptr<DocIdSetIterator> approximation)
{
  return make_shared<TwoPhaseIteratorAnonymousInnerClass>(shared_from_this(),
                                                          approximation);
}

GlobalOrdinalsQuery::SegmentOrdinalScorer::TwoPhaseIteratorAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass(
        shared_ptr<SegmentOrdinalScorer> outerInstance,
        shared_ptr<DocIdSetIterator> approximation)
    : org::apache::lucene::search::TwoPhaseIterator(approximation)
{
  this->outerInstance = outerInstance;
  this->approximation = approximation;
}

bool GlobalOrdinalsQuery::SegmentOrdinalScorer::
    TwoPhaseIteratorAnonymousInnerClass::matches() 
{
  if (outerInstance->values->advanceExact(approximation->docID()) &&
      outerInstance->foundOrds->get(outerInstance->values->ordValue())) {
    return true;
  }
  return false;
}

float GlobalOrdinalsQuery::SegmentOrdinalScorer::
    TwoPhaseIteratorAnonymousInnerClass::matchCost()
{
  return 100; // TODO: use cost of values.getOrd() and foundOrds.get()
}
} // namespace org::apache::lucene::search::join