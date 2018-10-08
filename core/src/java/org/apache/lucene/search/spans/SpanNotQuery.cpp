using namespace std;

#include "SpanNotQuery.h"

namespace org::apache::lucene::search::spans
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;

SpanNotQuery::SpanNotQuery(shared_ptr<SpanQuery> include,
                           shared_ptr<SpanQuery> exclude)
    : SpanNotQuery(include, exclude, 0, 0)
{
}

SpanNotQuery::SpanNotQuery(shared_ptr<SpanQuery> include,
                           shared_ptr<SpanQuery> exclude, int dist)
    : SpanNotQuery(include, exclude, dist, dist)
{
}

SpanNotQuery::SpanNotQuery(shared_ptr<SpanQuery> include,
                           shared_ptr<SpanQuery> exclude, int pre, int post)
    : pre(pre), post(post)
{
  this->include = Objects::requireNonNull(include);
  this->exclude = Objects::requireNonNull(exclude);

  if (include->getField() != L"" && exclude->getField() != L"" &&
      include->getField() != exclude->getField()) {
    throw invalid_argument(L"Clauses must have same field.");
  }
}

shared_ptr<SpanQuery> SpanNotQuery::getInclude() { return include; }

shared_ptr<SpanQuery> SpanNotQuery::getExclude() { return exclude; }

wstring SpanNotQuery::getField() { return include->getField(); }

wstring SpanNotQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  buffer->append(L"spanNot(");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  buffer->append(include->toString(field));
  buffer->append(L", ");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  buffer->append(exclude->toString(field));
  buffer->append(L", ");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  buffer->append(Integer::toString(pre));
  buffer->append(L", ");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  buffer->append(Integer::toString(post));
  buffer->append(L")");
  return buffer->toString();
}

shared_ptr<SpanWeight>
SpanNotQuery::createWeight(shared_ptr<IndexSearcher> searcher, bool needsScores,
                           float boost) 
{
  shared_ptr<SpanWeight> includeWeight =
      include->createWeight(searcher, false, boost);
  shared_ptr<SpanWeight> excludeWeight =
      exclude->createWeight(searcher, false, boost);
  return make_shared<SpanNotWeight>(
      shared_from_this(), searcher,
      needsScores ? getTermContexts({includeWeight, excludeWeight}) : nullptr,
      includeWeight, excludeWeight, boost);
}

SpanNotQuery::SpanNotWeight::SpanNotWeight(
    shared_ptr<SpanNotQuery> outerInstance, shared_ptr<IndexSearcher> searcher,
    unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>> &terms,
    shared_ptr<SpanWeight> includeWeight, shared_ptr<SpanWeight> excludeWeight,
    float boost) 
    : SpanWeight(SpanNotQuery::this, searcher, terms, boost),
      includeWeight(includeWeight), excludeWeight(excludeWeight),
      outerInstance(outerInstance)
{
}

void SpanNotQuery::SpanNotWeight::extractTermContexts(
    unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>
        &contexts)
{
  includeWeight->extractTermContexts(contexts);
}

shared_ptr<Spans> SpanNotQuery::SpanNotWeight::getSpans(
    shared_ptr<LeafReaderContext> context,
    Postings requiredPostings) 
{
  shared_ptr<Spans> includeSpans =
      includeWeight->getSpans(context, requiredPostings);
  if (includeSpans == nullptr) {
    return nullptr;
  }

  shared_ptr<Spans> excludeSpans =
      excludeWeight->getSpans(context, requiredPostings);
  if (excludeSpans == nullptr) {
    return includeSpans;
  }

  shared_ptr<TwoPhaseIterator> excludeTwoPhase =
      excludeSpans->asTwoPhaseIterator();
  shared_ptr<DocIdSetIterator> excludeApproximation =
      excludeTwoPhase == nullptr ? nullptr : excludeTwoPhase->approximation();

  return make_shared<FilterSpansAnonymousInnerClass>(
      shared_from_this(), includeSpans, excludeSpans, excludeTwoPhase,
      excludeApproximation);
}

SpanNotQuery::SpanNotWeight::FilterSpansAnonymousInnerClass::
    FilterSpansAnonymousInnerClass(
        shared_ptr<SpanNotWeight> outerInstance,
        shared_ptr<org::apache::lucene::search::spans::Spans> includeSpans,
        shared_ptr<org::apache::lucene::search::spans::Spans> excludeSpans,
        shared_ptr<TwoPhaseIterator> excludeTwoPhase,
        shared_ptr<DocIdSetIterator> excludeApproximation)
    : FilterSpans(includeSpans)
{
  this->outerInstance = outerInstance;
  this->excludeSpans = excludeSpans;
  this->excludeTwoPhase = excludeTwoPhase;
  this->excludeApproximation = excludeApproximation;
  lastApproxDoc = -1;
  lastApproxResult = false;
}

AcceptStatus
SpanNotQuery::SpanNotWeight::FilterSpansAnonymousInnerClass::accept(
    shared_ptr<Spans> candidate) 
{
  // TODO: this logic is ugly and sneaky, can we clean it up?
  int doc = candidate->docID();
  if (doc > excludeSpans->docID()) {
    // catch up 'exclude' to the current doc
    if (excludeTwoPhase != nullptr) {
      if (excludeApproximation->advance(doc) == doc) {
        lastApproxDoc = doc;
        lastApproxResult = excludeTwoPhase->matches();
      }
    } else {
      excludeSpans->advance(doc);
    }
  } else if (excludeTwoPhase != nullptr && doc == excludeSpans->docID() &&
             doc != lastApproxDoc) {
    // excludeSpans already sitting on our candidate doc, but matches not called
    // yet.
    lastApproxDoc = doc;
    lastApproxResult = excludeTwoPhase->matches();
  }

  if (doc != excludeSpans->docID() ||
      (doc == lastApproxDoc && lastApproxResult == false)) {
    return AcceptStatus::YES;
  }

  if (excludeSpans->startPosition() ==
      -1) { // init exclude start position if needed
    excludeSpans->nextStartPosition();
  }

  while (excludeSpans->endPosition() <=
         candidate->startPosition() - outerInstance->outerInstance->pre) {
    // exclude end position is before a possible exclusion
    if (excludeSpans->nextStartPosition() == NO_MORE_POSITIONS) {
      return AcceptStatus::YES; // no more exclude at current doc.
    }
  }

  // exclude end position far enough in current doc, check start position:
  if (excludeSpans->startPosition() - outerInstance->outerInstance->post >=
      candidate->endPosition()) {
    return AcceptStatus::YES;
  } else {
    return AcceptStatus::NO;
  }
}

void SpanNotQuery::SpanNotWeight::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  includeWeight->extractTerms(terms);
}

bool SpanNotQuery::SpanNotWeight::isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return includeWeight->isCacheable(ctx) && excludeWeight->isCacheable(ctx);
}

shared_ptr<Query>
SpanNotQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  shared_ptr<SpanQuery> rewrittenInclude =
      std::static_pointer_cast<SpanQuery>(include->rewrite(reader));
  shared_ptr<SpanQuery> rewrittenExclude =
      std::static_pointer_cast<SpanQuery>(exclude->rewrite(reader));
  if (rewrittenInclude != include || rewrittenExclude != exclude) {
    return make_shared<SpanNotQuery>(rewrittenInclude, rewrittenExclude, pre,
                                     post);
  }
  return SpanQuery::rewrite(reader);
}

bool SpanNotQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool SpanNotQuery::equalsTo(shared_ptr<SpanNotQuery> other)
{
  return include->equals(other->include) && exclude->equals(other->exclude) &&
         pre == other->pre && post == other->post;
}

int SpanNotQuery::hashCode()
{
  int h = classHash();
  h = Integer::rotateLeft(h, 1);
  h ^= include->hashCode();
  h = Integer::rotateLeft(h, 1);
  h ^= exclude->hashCode();
  h = Integer::rotateLeft(h, 1);
  h ^= pre;
  h = Integer::rotateLeft(h, 1);
  h ^= post;
  return h;
}
} // namespace org::apache::lucene::search::spans