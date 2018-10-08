using namespace std;

#include "SpanContainingQuery.h"

namespace org::apache::lucene::search::spans
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

SpanContainingQuery::SpanContainingQuery(shared_ptr<SpanQuery> big,
                                         shared_ptr<SpanQuery> little)
    : SpanContainQuery(big, little)
{
}

wstring SpanContainingQuery::toString(const wstring &field)
{
  return toString(field, L"SpanContaining");
}

shared_ptr<SpanWeight>
SpanContainingQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                  bool needsScores,
                                  float boost) 
{
  shared_ptr<SpanWeight> bigWeight = big->createWeight(searcher, false, boost);
  shared_ptr<SpanWeight> littleWeight =
      little->createWeight(searcher, false, boost);
  return make_shared<SpanContainingWeight>(
      shared_from_this(), searcher,
      needsScores ? getTermContexts({bigWeight, littleWeight}) : nullptr,
      bigWeight, littleWeight, boost);
}

SpanContainingQuery::SpanContainingWeight::SpanContainingWeight(
    shared_ptr<SpanContainingQuery> outerInstance,
    shared_ptr<IndexSearcher> searcher,
    unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>> &terms,
    shared_ptr<SpanWeight> bigWeight, shared_ptr<SpanWeight> littleWeight,
    float boost) 
    : SpanContainWeight(outerInstance, searcher, terms, bigWeight, littleWeight,
                        boost),
      outerInstance(outerInstance)
{
}

shared_ptr<Spans> SpanContainingQuery::SpanContainingWeight::getSpans(
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
                                                      little, big);
}

SpanContainingQuery::SpanContainingWeight::ContainSpansAnonymousInnerClass::
    ContainSpansAnonymousInnerClass(
        shared_ptr<SpanContainingWeight> outerInstance,
        shared_ptr<org::apache::lucene::search::spans::Spans> big,
        shared_ptr<org::apache::lucene::search::spans::Spans> little,
        shared_ptr<org::apache::lucene::search::spans::Spans> big)
    : ContainSpans(big, little, big)
{
  this->outerInstance = outerInstance;
}

bool SpanContainingQuery::SpanContainingWeight::
    ContainSpansAnonymousInnerClass::twoPhaseCurrentDocMatches() throw(
        IOException)
{
  oneExhaustedInCurrentDoc = false;
  assert(littleSpans::startPosition() == -1);
  while (bigSpans::nextStartPosition() != NO_MORE_POSITIONS) {
    while (littleSpans::startPosition() < bigSpans::startPosition()) {
      if (littleSpans::nextStartPosition() == NO_MORE_POSITIONS) {
        oneExhaustedInCurrentDoc = true;
        return false;
      }
    }
    if (bigSpans::endPosition() >= littleSpans::endPosition()) {
      atFirstInCurrentDoc = true;
      return true;
    }
  }
  oneExhaustedInCurrentDoc = true;
  return false;
}

int SpanContainingQuery::SpanContainingWeight::ContainSpansAnonymousInnerClass::
    nextStartPosition() 
{
  if (atFirstInCurrentDoc) {
    atFirstInCurrentDoc = false;
    return bigSpans::startPosition();
  }
  while (bigSpans::nextStartPosition() != NO_MORE_POSITIONS) {
    while (littleSpans::startPosition() < bigSpans::startPosition()) {
      if (littleSpans::nextStartPosition() == NO_MORE_POSITIONS) {
        oneExhaustedInCurrentDoc = true;
        return NO_MORE_POSITIONS;
      }
    }
    if (bigSpans::endPosition() >= littleSpans::endPosition()) {
      return bigSpans::startPosition();
    }
  }
  oneExhaustedInCurrentDoc = true;
  return NO_MORE_POSITIONS;
}

bool SpanContainingQuery::SpanContainingWeight::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return bigWeight->isCacheable(ctx) && littleWeight->isCacheable(ctx);
}
} // namespace org::apache::lucene::search::spans