using namespace std;

#include "SpanPayloadCheckQuery.h"

namespace org::apache::lucene::queries::payloads
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using Terms = org::apache::lucene::index::Terms;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using FilterSpans = org::apache::lucene::search::spans::FilterSpans;
using AcceptStatus =
    org::apache::lucene::search::spans::FilterSpans::AcceptStatus;
using SpanCollector = org::apache::lucene::search::spans::SpanCollector;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using SpanScorer = org::apache::lucene::search::spans::SpanScorer;
using SpanWeight = org::apache::lucene::search::spans::SpanWeight;
using Spans = org::apache::lucene::search::spans::Spans;
using BytesRef = org::apache::lucene::util::BytesRef;

SpanPayloadCheckQuery::SpanPayloadCheckQuery(
    shared_ptr<SpanQuery> match,
    deque<std::shared_ptr<BytesRef>> &payloadToMatch)
    : payloadToMatch(payloadToMatch), match(match)
{
}

wstring SpanPayloadCheckQuery::getField() { return match->getField(); }

shared_ptr<SpanWeight>
SpanPayloadCheckQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                    bool needsScores,
                                    float boost) 
{
  shared_ptr<SpanWeight> matchWeight =
      match->createWeight(searcher, false, boost);
  return make_shared<SpanPayloadCheckWeight>(
      shared_from_this(), searcher,
      needsScores ? getTermContexts({matchWeight}) : nullptr, matchWeight,
      boost);
}

shared_ptr<Query> SpanPayloadCheckQuery::rewrite(
    shared_ptr<IndexReader> reader) 
{
  shared_ptr<Query> matchRewritten = match->rewrite(reader);
  if (match != matchRewritten &&
      std::dynamic_pointer_cast<SpanQuery>(matchRewritten) != nullptr) {
    return make_shared<SpanPayloadCheckQuery>(
        std::static_pointer_cast<SpanQuery>(matchRewritten), payloadToMatch);
  }
  return SpanQuery::rewrite(reader);
}

SpanPayloadCheckQuery::SpanPayloadCheckWeight::SpanPayloadCheckWeight(
    shared_ptr<SpanPayloadCheckQuery> outerInstance,
    shared_ptr<IndexSearcher> searcher,
    unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>
        &termContexts,
    shared_ptr<SpanWeight> matchWeight, float boost) 
    : org::apache::lucene::search::spans::SpanWeight(
          SpanPayloadCheckQuery::this, searcher, termContexts, boost),
      matchWeight(matchWeight), outerInstance(outerInstance)
{
}

void SpanPayloadCheckQuery::SpanPayloadCheckWeight::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  matchWeight->extractTerms(terms);
}

void SpanPayloadCheckQuery::SpanPayloadCheckWeight::extractTermContexts(
    unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>
        &contexts)
{
  matchWeight->extractTermContexts(contexts);
}

shared_ptr<Spans> SpanPayloadCheckQuery::SpanPayloadCheckWeight::getSpans(
    shared_ptr<LeafReaderContext> context,
    Postings requiredPostings) 
{
  shared_ptr<PayloadChecker> *const collector =
      make_shared<PayloadChecker>(outerInstance);
  shared_ptr<Spans> matchSpans = matchWeight->getSpans(
      context, requiredPostings.atLeast(Postings::PAYLOADS));
  return (matchSpans == nullptr)
             ? nullptr
             : make_shared<FilterSpansAnonymousInnerClass>(
                   shared_from_this(), matchSpans, collector);
}

SpanPayloadCheckQuery::SpanPayloadCheckWeight::FilterSpansAnonymousInnerClass::
    FilterSpansAnonymousInnerClass(
        shared_ptr<SpanPayloadCheckWeight> outerInstance,
        shared_ptr<Spans> matchSpans,
        shared_ptr<org::apache::lucene::queries::payloads::
                       SpanPayloadCheckQuery::PayloadChecker>
            collector)
    : org::apache::lucene::search::spans::FilterSpans(matchSpans)
{
  this->outerInstance = outerInstance;
  this->collector = collector;
}

FilterSpans::AcceptStatus
SpanPayloadCheckQuery::SpanPayloadCheckWeight::FilterSpansAnonymousInnerClass::
    accept(shared_ptr<Spans> candidate) 
{
  collector->reset();
  candidate->collect(collector);
  return collector->match();
}

shared_ptr<SpanScorer> SpanPayloadCheckQuery::SpanPayloadCheckWeight::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  if (field == L"") {
    return nullptr;
  }

  shared_ptr<Terms> terms = context->reader()->terms(field);
  if (terms != nullptr && terms->hasPositions() == false) {
    throw make_shared<IllegalStateException>(
        L"field \"" + field +
        L"\" was indexed without position data; cannot run SpanQuery (query=" +
        parentQuery + L")");
  }

  shared_ptr<Spans> *const spans = getSpans(context, Postings::PAYLOADS);
  if (spans == nullptr) {
    return nullptr;
  }
  shared_ptr<Similarity::SimScorer> *const docScorer = getSimScorer(context);
  return make_shared<SpanScorer>(shared_from_this(), spans, docScorer);
}

bool SpanPayloadCheckQuery::SpanPayloadCheckWeight::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return matchWeight->isCacheable(ctx);
}

SpanPayloadCheckQuery::PayloadChecker::PayloadChecker(
    shared_ptr<SpanPayloadCheckQuery> outerInstance)
    : outerInstance(outerInstance)
{
}

void SpanPayloadCheckQuery::PayloadChecker::collectLeaf(
    shared_ptr<PostingsEnum> postings, int position,
    shared_ptr<Term> term) 
{
  if (!matches) {
    return;
  }
  if (upto >= outerInstance->payloadToMatch.size()) {
    matches = false;
    return;
  }
  shared_ptr<BytesRef> payload = postings->getPayload();
  if (outerInstance->payloadToMatch[upto] == nullptr) {
    matches = payload == nullptr;
    upto++;
    return;
  }
  if (payload == nullptr) {
    matches = false;
    upto++;
    return;
  }
  matches = outerInstance->payloadToMatch[upto]->bytesEquals(payload);
  upto++;
}

FilterSpans::AcceptStatus SpanPayloadCheckQuery::PayloadChecker::match()
{
  return matches && upto == outerInstance->payloadToMatch.size()
             ? FilterSpans::AcceptStatus::YES
             : FilterSpans::AcceptStatus::NO;
}

void SpanPayloadCheckQuery::PayloadChecker::reset()
{
  this->upto = 0;
  this->matches = true;
}

wstring SpanPayloadCheckQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  buffer->append(L"SpanPayloadCheckQuery(");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  buffer->append(match->toString(field));
  buffer->append(L", payloadRef: ");
  for (auto bytes : payloadToMatch) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    buffer->append(Term::toString(bytes));
    buffer->append(L';');
  }
  buffer->append(L")");
  return buffer->toString();
}

bool SpanPayloadCheckQuery::equals(any other)
{
  return sameClassAs(other) &&
         payloadToMatch.equals(
             (any_cast<std::shared_ptr<SpanPayloadCheckQuery>>(other))
                 .payloadToMatch) &&
         match->equals(
             (any_cast<std::shared_ptr<SpanPayloadCheckQuery>>(other)).match);
}

int SpanPayloadCheckQuery::hashCode()
{
  int result = classHash();
  result = 31 * result + Objects::hashCode(match);
  result = 31 * result + Objects::hashCode(payloadToMatch);
  return result;
}
} // namespace org::apache::lucene::queries::payloads