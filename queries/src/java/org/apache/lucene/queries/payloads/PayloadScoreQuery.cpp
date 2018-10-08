using namespace std;

#include "PayloadScoreQuery.h"

namespace org::apache::lucene::queries::payloads
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using SimScorer =
    org::apache::lucene::search::similarities::Similarity::SimScorer;
using FilterSpans = org::apache::lucene::search::spans::FilterSpans;
using SpanCollector = org::apache::lucene::search::spans::SpanCollector;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using SpanScorer = org::apache::lucene::search::spans::SpanScorer;
using SpanWeight = org::apache::lucene::search::spans::SpanWeight;
using Spans = org::apache::lucene::search::spans::Spans;
using BytesRef = org::apache::lucene::util::BytesRef;

PayloadScoreQuery::PayloadScoreQuery(shared_ptr<SpanQuery> wrappedQuery,
                                     shared_ptr<PayloadFunction> function,
                                     shared_ptr<PayloadDecoder> decoder,
                                     bool includeSpanScore)
    : wrappedQuery(Objects::requireNonNull(wrappedQuery)),
      function(Objects::requireNonNull(function)), decoder(decoder),
      includeSpanScore(includeSpanScore)
{
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public
// PayloadScoreQuery(org.apache.lucene.search.spans.SpanQuery wrappedQuery,
// PayloadFunction function, bool includeSpanScore)
PayloadScoreQuery::PayloadScoreQuery(shared_ptr<SpanQuery> wrappedQuery,
                                     shared_ptr<PayloadFunction> function,
                                     bool includeSpanScore)
    : PayloadScoreQuery(wrappedQuery, function, nullptr, includeSpanScore)
{
}

PayloadScoreQuery::PayloadScoreQuery(shared_ptr<SpanQuery> wrappedQuery,
                                     shared_ptr<PayloadFunction> function,
                                     shared_ptr<PayloadDecoder> decoder)
    : PayloadScoreQuery(wrappedQuery, function, decoder, true)
{
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public
// PayloadScoreQuery(org.apache.lucene.search.spans.SpanQuery wrappedQuery,
// PayloadFunction function)
PayloadScoreQuery::PayloadScoreQuery(shared_ptr<SpanQuery> wrappedQuery,
                                     shared_ptr<PayloadFunction> function)
    : PayloadScoreQuery(wrappedQuery, function, true)
{
}

wstring PayloadScoreQuery::getField() { return wrappedQuery->getField(); }

shared_ptr<Query>
PayloadScoreQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  shared_ptr<Query> matchRewritten = wrappedQuery->rewrite(reader);
  if (wrappedQuery != matchRewritten &&
      std::dynamic_pointer_cast<SpanQuery>(matchRewritten) != nullptr) {
    return make_shared<PayloadScoreQuery>(
        std::static_pointer_cast<SpanQuery>(matchRewritten), function, decoder,
        includeSpanScore);
  }
  return SpanQuery::rewrite(reader);
}

wstring PayloadScoreQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  buffer->append(L"PayloadScoreQuery(");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  buffer->append(wrappedQuery->toString(field));
  buffer->append(L", function: ");
  buffer->append(function->getClass().getSimpleName());
  buffer->append(L", includeSpanScore: ");
  buffer->append(includeSpanScore);
  buffer->append(L")");
  return buffer->toString();
}

shared_ptr<SpanWeight>
PayloadScoreQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                bool needsScores,
                                float boost) 
{
  shared_ptr<SpanWeight> innerWeight =
      wrappedQuery->createWeight(searcher, needsScores, boost);
  if (!needsScores) {
    return innerWeight;
  }
  return make_shared<PayloadSpanWeight>(shared_from_this(), searcher,
                                        innerWeight, boost);
}

bool PayloadScoreQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool PayloadScoreQuery::equalsTo(shared_ptr<PayloadScoreQuery> other)
{
  return wrappedQuery->equals(other->wrappedQuery) &&
         function->equals(other->function) &&
         (includeSpanScore == other->includeSpanScore) &&
         Objects::equals(decoder, other->decoder);
}

int PayloadScoreQuery::hashCode()
{
  return Objects::hash(wrappedQuery, function, decoder, includeSpanScore);
}

PayloadScoreQuery::PayloadSpanWeight::PayloadSpanWeight(
    shared_ptr<PayloadScoreQuery> outerInstance,
    shared_ptr<IndexSearcher> searcher, shared_ptr<SpanWeight> innerWeight,
    float boost) 
    : org::apache::lucene::search::spans::SpanWeight(PayloadScoreQuery::this,
                                                     searcher, nullptr, boost),
      innerWeight(innerWeight), outerInstance(outerInstance)
{
}

void PayloadScoreQuery::PayloadSpanWeight::extractTermContexts(
    unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>
        &contexts)
{
  innerWeight->extractTermContexts(contexts);
}

shared_ptr<Spans> PayloadScoreQuery::PayloadSpanWeight::getSpans(
    shared_ptr<LeafReaderContext> ctx,
    Postings requiredPostings) 
{
  return innerWeight->getSpans(ctx,
                               requiredPostings.atLeast(Postings::PAYLOADS));
}

shared_ptr<SpanScorer> PayloadScoreQuery::PayloadSpanWeight::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<Spans> spans = getSpans(context, Postings::PAYLOADS);
  if (spans == nullptr) {
    return nullptr;
  }
  shared_ptr<Similarity::SimScorer> docScorer =
      innerWeight->getSimScorer(context);
  shared_ptr<PayloadSpans> payloadSpans = make_shared<PayloadSpans>(
      outerInstance, spans,
      outerInstance->decoder == nullptr
          ? make_shared<SimilarityPayloadDecoder>(docScorer)
          : outerInstance->decoder);
  return make_shared<PayloadSpanScorer>(outerInstance, shared_from_this(),
                                        payloadSpans, docScorer);
}

bool PayloadScoreQuery::PayloadSpanWeight::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return innerWeight->isCacheable(ctx);
}

void PayloadScoreQuery::PayloadSpanWeight::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  innerWeight->extractTerms(terms);
}

shared_ptr<Explanation> PayloadScoreQuery::PayloadSpanWeight::explain(
    shared_ptr<LeafReaderContext> context, int doc) 
{
  shared_ptr<PayloadSpanScorer> scorer =
      std::static_pointer_cast<PayloadSpanScorer>(this->scorer(context));
  if (scorer == nullptr || scorer->begin().advance(doc) != doc) {
    return Explanation::noMatch(L"No match");
  }

  scorer->score(); // force freq calculation
  shared_ptr<Explanation> payloadExpl = scorer->getPayloadExplanation();

  if (outerInstance->includeSpanScore) {
    shared_ptr<SpanWeight> innerWeight =
        (std::static_pointer_cast<PayloadSpanWeight>(scorer->getWeight()))
            ->innerWeight;
    shared_ptr<Explanation> innerExpl = innerWeight->explain(context, doc);
    return Explanation::match(scorer->scoreCurrentDoc(),
                              L"PayloadSpanQuery, product of:",
                              {innerExpl, payloadExpl});
  }

  return scorer->getPayloadExplanation();
}

PayloadScoreQuery::PayloadSpans::PayloadSpans(
    shared_ptr<PayloadScoreQuery> outerInstance, shared_ptr<Spans> in_,
    shared_ptr<PayloadDecoder> decoder)
    : org::apache::lucene::search::spans::FilterSpans(in_), decoder(decoder),
      outerInstance(outerInstance)
{
}

AcceptStatus PayloadScoreQuery::PayloadSpans::accept(
    shared_ptr<Spans> candidate) 
{
  return AcceptStatus::YES;
}

void PayloadScoreQuery::PayloadSpans::doStartCurrentDoc()
{
  payloadScore = 0;
  payloadsSeen = 0;
}

void PayloadScoreQuery::PayloadSpans::collectLeaf(
    shared_ptr<PostingsEnum> postings, int position,
    shared_ptr<Term> term) 
{
  shared_ptr<BytesRef> payload = postings->getPayload();
  float payloadFactor = decoder->computePayloadFactor(
      docID(), in_->startPosition(), in_->endPosition(), payload);
  payloadScore = outerInstance->function->currentScore(
      docID(), outerInstance->getField(), in_->startPosition(),
      in_->endPosition(), payloadsSeen, payloadScore, payloadFactor);
  payloadsSeen++;
}

void PayloadScoreQuery::PayloadSpans::reset() {}

void PayloadScoreQuery::PayloadSpans::doCurrentSpans() 
{
  in_->collect(shared_from_this());
}

PayloadScoreQuery::PayloadSpanScorer::PayloadSpanScorer(
    shared_ptr<PayloadScoreQuery> outerInstance, shared_ptr<SpanWeight> weight,
    shared_ptr<PayloadSpans> spans,
    shared_ptr<Similarity::SimScorer> docScorer) 
    : org::apache::lucene::search::spans::SpanScorer(weight, spans, docScorer),
      spans(spans), outerInstance(outerInstance)
{
}

float PayloadScoreQuery::PayloadSpanScorer::getPayloadScore()
{
  return outerInstance->function->docScore(docID(), outerInstance->getField(),
                                           spans->payloadsSeen,
                                           spans->payloadScore);
}

shared_ptr<Explanation>
PayloadScoreQuery::PayloadSpanScorer::getPayloadExplanation()
{
  return outerInstance->function->explain(docID(), outerInstance->getField(),
                                          spans->payloadsSeen,
                                          spans->payloadScore);
}

float PayloadScoreQuery::PayloadSpanScorer::getSpanScore() 
{
  return SpanScorer::scoreCurrentDoc();
}

float PayloadScoreQuery::PayloadSpanScorer::scoreCurrentDoc() 
{
  if (outerInstance->includeSpanScore) {
    return getSpanScore() * getPayloadScore();
  }
  return getPayloadScore();
}

PayloadScoreQuery::SimilarityPayloadDecoder::SimilarityPayloadDecoder(
    shared_ptr<Similarity::SimScorer> docScorer)
    : docScorer(docScorer)
{
}

float PayloadScoreQuery::SimilarityPayloadDecoder::computePayloadFactor(
    int docID, int startPosition, int endPosition, shared_ptr<BytesRef> payload)
{
  if (payload == nullptr) {
    return 0;
  }
  return docScorer->computePayloadFactor(docID, startPosition, endPosition,
                                         payload);
}

float PayloadScoreQuery::SimilarityPayloadDecoder::computePayloadFactor(
    shared_ptr<BytesRef> payload)
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::queries::payloads