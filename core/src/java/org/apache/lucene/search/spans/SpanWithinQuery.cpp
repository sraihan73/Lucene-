using namespace std;

#include "SpanWithinQuery.h"

namespace org::apache::lucene::search::spans
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

SpanWithinQuery::SpanWithinQuery(shared_ptr<SpanQuery> big,
                                 shared_ptr<SpanQuery> little)
    : SpanContainQuery(big, little)
{
}

wstring SpanWithinQuery::toString(const wstring &field)
{
  return toString(field, L"SpanWithin");
}

shared_ptr<SpanWeight>
SpanWithinQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                              bool needsScores, float boost) 
{
  shared_ptr<SpanWeight> bigWeight = big->createWeight(searcher, false, boost);
  shared_ptr<SpanWeight> littleWeight =
      little->createWeight(searcher, false, boost);
  return make_shared<SpanWithinWeight>(
      shared_from_this(), searcher,
      needsScores ? getTermContexts({bigWeight, littleWeight}) : nullptr,
      bigWeight, littleWeight, boost);
}

SpanWithinQuery::SpanWithinWeight::SpanWithinWeight(
    shared_ptr<SpanWithinQuery> outerInstance,
    shared_ptr<IndexSearcher> searcher,
    unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>> &terms,
    shared_ptr<SpanWeight> bigWeight, shared_ptr<SpanWeight> littleWeight,
    float boost) 
    : SpanContainWeight(outerInstance, searcher, terms, bigWeight, littleWeight,
                        boost),
      outerInstance(outerInstance)
{
}

shared_ptr<Spans> SpanWithinQuery::SpanWithinWeight::getSpans(
    shared_ptr<LeafReaderContext> context,
    Postings requiredPostings) 
{
  deque<std::shared_ptr<Spans>> containerContained =
      prepareConjunction(context, requiredPostings);
  if (containerContained.empty()) {
    return nullptr;
  }

  shared_ptr<Spans> big = containerContained[0];
  shared_ptr<Spans> little = containerContained[1];

  return make_shared<ContainSpansAnonymousInnerClass>(shared_from_this(), big,
                                                      little, little);
}

SpanWithinQuery::SpanWithinWeight::ContainSpansAnonymousInnerClass::
    ContainSpansAnonymousInnerClass(
        shared_ptr<SpanWithinWeight> outerInstance,
        shared_ptr<org::apache::lucene::search::spans::Spans> big,
        shared_ptr<org::apache::lucene::search::spans::Spans> little,
        shared_ptr<org::apache::lucene::search::spans::Spans> little)
    : ContainSpans(big, little, little)
{
  this->outerInstance = outerInstance;
}

bool SpanWithinQuery::SpanWithinWeight::ContainSpansAnonymousInnerClass::
    twoPhaseCurrentDocMatches() 
{
  oneExhaustedInCurrentDoc = false;
  assert(littleSpans::startPosition() == -1);
  while (littleSpans::nextStartPosition() != NO_MORE_POSITIONS) {
    while (bigSpans::endPosition() < littleSpans::endPosition()) {
      if (bigSpans::nextStartPosition() == NO_MORE_POSITIONS) {
        oneExhaustedInCurrentDoc = true;
        return false;
      }
    }
    if (bigSpans::startPosition() <= littleSpans::startPosition()) {
      atFirstInCurrentDoc = true;
      return true;
    }
  }
  oneExhaustedInCurrentDoc = true;
  return false;
}

int SpanWithinQuery::SpanWithinWeight::ContainSpansAnonymousInnerClass::
    nextStartPosition() 
{
  if (atFirstInCurrentDoc) {
    atFirstInCurrentDoc = false;
    return littleSpans::startPosition();
  }
  while (littleSpans::nextStartPosition() != NO_MORE_POSITIONS) {
    while (bigSpans::endPosition() < littleSpans::endPosition()) {
      if (bigSpans::nextStartPosition() == NO_MORE_POSITIONS) {
        oneExhaustedInCurrentDoc = true;
        return NO_MORE_POSITIONS;
      }
    }
    if (bigSpans::startPosition() <= littleSpans::startPosition()) {
      return littleSpans::startPosition();
    }
  }
  oneExhaustedInCurrentDoc = true;
  return NO_MORE_POSITIONS;
}

bool SpanWithinQuery::SpanWithinWeight::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return littleWeight->isCacheable(ctx) && bigWeight->isCacheable(ctx);
}
} // namespace org::apache::lucene::search::spans