using namespace std;

#include "SpanPositionCheckQuery.h"

namespace org::apache::lucene::search::spans
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using AcceptStatus =
    org::apache::lucene::search::spans::FilterSpans::AcceptStatus;

SpanPositionCheckQuery::SpanPositionCheckQuery(shared_ptr<SpanQuery> match)
{
  this->match = Objects::requireNonNull(match);
}

shared_ptr<SpanQuery> SpanPositionCheckQuery::getMatch() { return match; }

wstring SpanPositionCheckQuery::getField() { return match->getField(); }

shared_ptr<SpanWeight>
SpanPositionCheckQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                     bool needsScores,
                                     float boost) 
{
  shared_ptr<SpanWeight> matchWeight =
      match->createWeight(searcher, false, boost);
  return make_shared<SpanPositionCheckWeight>(
      shared_from_this(), matchWeight, searcher,
      needsScores ? getTermContexts({matchWeight}) : nullptr, boost);
}

SpanPositionCheckQuery::SpanPositionCheckWeight::SpanPositionCheckWeight(
    shared_ptr<SpanPositionCheckQuery> outerInstance,
    shared_ptr<SpanWeight> matchWeight, shared_ptr<IndexSearcher> searcher,
    unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>> &terms,
    float boost) 
    : SpanWeight(SpanPositionCheckQuery::this, searcher, terms, boost),
      matchWeight(matchWeight), outerInstance(outerInstance)
{
}

void SpanPositionCheckQuery::SpanPositionCheckWeight::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  matchWeight->extractTerms(terms);
}

bool SpanPositionCheckQuery::SpanPositionCheckWeight::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return matchWeight->isCacheable(ctx);
}

void SpanPositionCheckQuery::SpanPositionCheckWeight::extractTermContexts(
    unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>
        &contexts)
{
  matchWeight->extractTermContexts(contexts);
}

shared_ptr<Spans> SpanPositionCheckQuery::SpanPositionCheckWeight::getSpans(
    shared_ptr<LeafReaderContext> context,
    Postings requiredPostings) 
{
  shared_ptr<Spans> matchSpans =
      matchWeight->getSpans(context, requiredPostings);
  return (matchSpans == nullptr) ? nullptr
                                 : make_shared<FilterSpansAnonymousInnerClass>(
                                       shared_from_this(), matchSpans);
}

SpanPositionCheckQuery::SpanPositionCheckWeight::
    FilterSpansAnonymousInnerClass::FilterSpansAnonymousInnerClass(
        shared_ptr<SpanPositionCheckWeight> outerInstance,
        shared_ptr<org::apache::lucene::search::spans::Spans> matchSpans)
    : FilterSpans(matchSpans)
{
  this->outerInstance = outerInstance;
}

AcceptStatus SpanPositionCheckQuery::SpanPositionCheckWeight::
    FilterSpansAnonymousInnerClass::accept(shared_ptr<Spans> candidate) throw(
        IOException)
{
  return outerInstance->outerInstance->acceptPosition(candidate);
}

shared_ptr<Query> SpanPositionCheckQuery::rewrite(
    shared_ptr<IndexReader> reader) 
{
  shared_ptr<SpanQuery> rewritten =
      std::static_pointer_cast<SpanQuery>(match->rewrite(reader));
  if (rewritten != match) {
    try {
      shared_ptr<SpanPositionCheckQuery> clone =
          std::static_pointer_cast<SpanPositionCheckQuery>(this->clone());
      clone->match = rewritten;
      return clone;
    } catch (const CloneNotSupportedException &e) {
      throw make_shared<AssertionError>(e);
    }
  }

  return SpanQuery::rewrite(reader);
}

bool SpanPositionCheckQuery::equals(any other)
{
  return sameClassAs(other) &&
         match->equals(
             (any_cast<std::shared_ptr<SpanPositionCheckQuery>>(other)).match);
}

int SpanPositionCheckQuery::hashCode()
{
  return classHash() ^ match->hashCode();
}
} // namespace org::apache::lucene::search::spans