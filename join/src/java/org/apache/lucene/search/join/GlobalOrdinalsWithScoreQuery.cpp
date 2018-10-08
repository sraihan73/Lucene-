using namespace std;

#include "GlobalOrdinalsWithScoreQuery.h"

namespace org::apache::lucene::search::join
{
using DocValues = org::apache::lucene::index::DocValues;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using OrdinalMap = org::apache::lucene::index::OrdinalMap;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using Term = org::apache::lucene::index::Term;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Explanation = org::apache::lucene::search::Explanation;
using FilterWeight = org::apache::lucene::search::FilterWeight;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;
using Weight = org::apache::lucene::search::Weight;
using BytesRef = org::apache::lucene::util::BytesRef;
using LongValues = org::apache::lucene::util::LongValues;

GlobalOrdinalsWithScoreQuery::GlobalOrdinalsWithScoreQuery(
    shared_ptr<GlobalOrdinalsWithScoreCollector> collector, ScoreMode scoreMode,
    const wstring &joinField, shared_ptr<OrdinalMap> globalOrds,
    shared_ptr<Query> toQuery, shared_ptr<Query> fromQuery, int min, int max,
    any indexReaderContextId)
    : collector(collector), joinField(joinField), globalOrds(globalOrds),
      toQuery(toQuery), scoreMode(scoreMode), fromQuery(fromQuery), min(min),
      max(max), indexReaderContextId(indexReaderContextId)
{
}

shared_ptr<Weight>
GlobalOrdinalsWithScoreQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                           bool needsScores,
                                           float boost) 
{
  if (searcher->getTopReaderContext()->id() != indexReaderContextId) {
    throw make_shared<IllegalStateException>(
        L"Creating the weight against a different index reader than this query "
        L"has been built for.");
  }
  bool doNoMinMax = min <= 0 && max == numeric_limits<int>::max();
  if (needsScores == false && doNoMinMax) {
    // We don't need scores then quickly change the query to not uses the
    // scores:
    shared_ptr<GlobalOrdinalsQuery> globalOrdinalsQuery =
        make_shared<GlobalOrdinalsQuery>(collector->collectedOrds, joinField,
                                         globalOrds, toQuery, fromQuery,
                                         indexReaderContextId);
    return globalOrdinalsQuery->createWeight(searcher, false, boost);
  }
  return make_shared<W>(shared_from_this(), shared_from_this(),
                        toQuery->createWeight(searcher, false, 1.0f));
}

bool GlobalOrdinalsWithScoreQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool GlobalOrdinalsWithScoreQuery::equalsTo(
    shared_ptr<GlobalOrdinalsWithScoreQuery> other)
{
  return min == other->min && max == other->max &&
         scoreMode.equals(other->scoreMode) && joinField == other->joinField &&
         fromQuery->equals(other->fromQuery) &&
         toQuery->equals(other->toQuery) &&
         indexReaderContextId.equals(other->indexReaderContextId);
}

int GlobalOrdinalsWithScoreQuery::hashCode()
{
  int result = classHash();
  result = 31 * result + scoreMode.hashCode();
  result = 31 * result + joinField.hashCode();
  result = 31 * result + toQuery->hashCode();
  result = 31 * result + fromQuery->hashCode();
  result = 31 * result + min;
  result = 31 * result + max;
  result = 31 * result + indexReaderContextId.hashCode();
  return result;
}

wstring GlobalOrdinalsWithScoreQuery::toString(const wstring &field)
{
  return wstring(L"GlobalOrdinalsQuery{") + L"joinField=" + joinField +
         L"min=" + to_wstring(min) + L"max=" + to_wstring(max) + L"fromQuery=" +
         fromQuery + L'}';
}

GlobalOrdinalsWithScoreQuery::W::W(
    shared_ptr<GlobalOrdinalsWithScoreQuery> outerInstance,
    shared_ptr<Query> query, shared_ptr<Weight> approximationWeight)
    : org::apache::lucene::search::FilterWeight(query, approximationWeight),
      outerInstance(outerInstance)
{
}

void GlobalOrdinalsWithScoreQuery::W::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
}

shared_ptr<Explanation>
GlobalOrdinalsWithScoreQuery::W::explain(shared_ptr<LeafReaderContext> context,
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
  if (outerInstance->collector->match(ord) == false) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return Explanation::noMatch(L"Not a match, join value " +
                                Term::toString(joinValue));
  }

  float score = outerInstance->collector->score(ord);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Explanation::match(score, L"A match, join value " +
                                       Term::toString(joinValue));
}

shared_ptr<Scorer> GlobalOrdinalsWithScoreQuery::W::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<SortedDocValues> values =
      DocValues::getSorted(context->reader(), outerInstance->joinField);
  if (values == nullptr) {
    return nullptr;
  }

  shared_ptr<Scorer> approximationScorer = in_->scorer(context);
  if (approximationScorer == nullptr) {
    return nullptr;
  } else if (outerInstance->globalOrds != nullptr) {
    return make_shared<OrdinalMapScorer>(
        shared_from_this(), outerInstance->collector, values,
        approximationScorer->begin(),
        outerInstance->globalOrds->getGlobalOrds(context->ord));
  } else {
    return make_shared<SegmentOrdinalScorer>(shared_from_this(),
                                             outerInstance->collector, values,
                                             approximationScorer->begin());
  }
}

GlobalOrdinalsWithScoreQuery::OrdinalMapScorer::OrdinalMapScorer(
    shared_ptr<Weight> weight,
    shared_ptr<GlobalOrdinalsWithScoreCollector> collector,
    shared_ptr<SortedDocValues> values,
    shared_ptr<DocIdSetIterator> approximation,
    shared_ptr<LongValues> segmentOrdToGlobalOrdLookup)
    : BaseGlobalOrdinalScorer(weight, values, approximation),
      segmentOrdToGlobalOrdLookup(segmentOrdToGlobalOrdLookup),
      collector(collector)
{
}

shared_ptr<TwoPhaseIterator>
GlobalOrdinalsWithScoreQuery::OrdinalMapScorer::createTwoPhaseIterator(
    shared_ptr<DocIdSetIterator> approximation)
{
  return make_shared<TwoPhaseIteratorAnonymousInnerClass>(shared_from_this(),
                                                          approximation);
}

GlobalOrdinalsWithScoreQuery::OrdinalMapScorer::
    TwoPhaseIteratorAnonymousInnerClass::TwoPhaseIteratorAnonymousInnerClass(
        shared_ptr<OrdinalMapScorer> outerInstance,
        shared_ptr<DocIdSetIterator> approximation)
    : org::apache::lucene::search::TwoPhaseIterator(approximation)
{
  this->outerInstance = outerInstance;
  this->approximation = approximation;
}

bool GlobalOrdinalsWithScoreQuery::OrdinalMapScorer::
    TwoPhaseIteratorAnonymousInnerClass::matches() 
{
  if (outerInstance->values->advanceExact(approximation->docID())) {
    constexpr int64_t segmentOrd = outerInstance->values->ordValue();
    constexpr int globalOrd = static_cast<int>(
        outerInstance->segmentOrdToGlobalOrdLookup->get(segmentOrd));
    if (outerInstance->collector->match(globalOrd)) {
      outerInstance->score_ = outerInstance->collector->score(globalOrd);
      return true;
    }
  }
  return false;
}

float GlobalOrdinalsWithScoreQuery::OrdinalMapScorer::
    TwoPhaseIteratorAnonymousInnerClass::matchCost()
{
  return 100; // TODO: use cost of values.getOrd() and collector.score()
}

GlobalOrdinalsWithScoreQuery::SegmentOrdinalScorer::SegmentOrdinalScorer(
    shared_ptr<Weight> weight,
    shared_ptr<GlobalOrdinalsWithScoreCollector> collector,
    shared_ptr<SortedDocValues> values,
    shared_ptr<DocIdSetIterator> approximation)
    : BaseGlobalOrdinalScorer(weight, values, approximation),
      collector(collector)
{
}

shared_ptr<TwoPhaseIterator>
GlobalOrdinalsWithScoreQuery::SegmentOrdinalScorer::createTwoPhaseIterator(
    shared_ptr<DocIdSetIterator> approximation)
{
  return make_shared<TwoPhaseIteratorAnonymousInnerClass>(shared_from_this(),
                                                          approximation);
}

GlobalOrdinalsWithScoreQuery::SegmentOrdinalScorer::
    TwoPhaseIteratorAnonymousInnerClass::TwoPhaseIteratorAnonymousInnerClass(
        shared_ptr<SegmentOrdinalScorer> outerInstance,
        shared_ptr<DocIdSetIterator> approximation)
    : org::apache::lucene::search::TwoPhaseIterator(approximation)
{
  this->outerInstance = outerInstance;
  this->approximation = approximation;
}

bool GlobalOrdinalsWithScoreQuery::SegmentOrdinalScorer::
    TwoPhaseIteratorAnonymousInnerClass::matches() 
{
  if (outerInstance->values->advanceExact(approximation->docID())) {
    constexpr int segmentOrd = outerInstance->values->ordValue();
    if (outerInstance->collector->match(segmentOrd)) {
      outerInstance->score_ = outerInstance->collector->score(segmentOrd);
      return true;
    }
  }
  return false;
}

float GlobalOrdinalsWithScoreQuery::SegmentOrdinalScorer::
    TwoPhaseIteratorAnonymousInnerClass::matchCost()
{
  return 100; // TODO: use cost.getOrd() of values and collector.score()
}
} // namespace org::apache::lucene::search::join