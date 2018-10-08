using namespace std;

#include "SpanWeight.h"

namespace org::apache::lucene::search::spans
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using Weight = org::apache::lucene::search::Weight;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using SimScorer =
    org::apache::lucene::search::similarities::Similarity::SimScorer;

deque<Postings> Postings::valueList;

Postings::StaticConstructor::StaticConstructor() {}

Postings::StaticConstructor Postings::staticConstructor;
int Postings::nextOrdinal = 0;
Postings::Postings(const wstring &name, InnerEnum innerEnum)
    : nameValue(name), ordinalValue(nextOrdinal++), innerEnumValue(innerEnum)
{
}

bool Postings::operator==(const Postings &other)
{
  return this->ordinalValue == other.ordinalValue;
}

bool Postings::operator!=(const Postings &other)
{
  return this->ordinalValue != other.ordinalValue;
}

deque<Postings> Postings::values() { return valueList; }

int Postings::ordinal() { return ordinalValue; }

wstring Postings::toString() { return nameValue; }

Postings Postings::valueOf(const wstring &name)
{
  for (auto enumInstance : Postings::valueList) {
    if (enumInstance.nameValue == name) {
      return enumInstance;
    }
  }
}

SpanWeight::SpanWeight(
    shared_ptr<SpanQuery> query, shared_ptr<IndexSearcher> searcher,
    unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>
        &termContexts,
    float boost) 
    : org::apache::lucene::search::Weight(query),
      similarity(searcher->getSimilarity(termContexts.size() > 0)),
      simWeight(buildSimWeight(query, searcher, termContexts, boost)),
      field(query->getField())
{
}

shared_ptr<Similarity::SimWeight> SpanWeight::buildSimWeight(
    shared_ptr<SpanQuery> query, shared_ptr<IndexSearcher> searcher,
    unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>
        &termContexts,
    float boost) 
{
  if (termContexts.empty() || termContexts.empty() ||
      query->getField() == L"") {
    return nullptr;
  }
  std::deque<std::shared_ptr<TermStatistics>> termStats(termContexts.size());
  int i = 0;
  for (auto term : termContexts) {
    termStats[i] =
        searcher->termStatistics(term->first, termContexts[term->first]);
    i++;
  }
  shared_ptr<CollectionStatistics> collectionStats =
      searcher->collectionStatistics(query->getField());
  return similarity->computeWeight(boost, collectionStats, termStats);
}

shared_ptr<SpanScorer>
SpanWeight::scorer(shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<Spans> *const spans = getSpans(context, Postings::POSITIONS);
  if (spans == nullptr) {
    return nullptr;
  }
  shared_ptr<Similarity::SimScorer> *const docScorer = getSimScorer(context);
  return make_shared<SpanScorer>(shared_from_this(), spans, docScorer);
}

shared_ptr<Similarity::SimScorer> SpanWeight::getSimScorer(
    shared_ptr<LeafReaderContext> context) 
{
  return simWeight == nullptr ? nullptr
                              : similarity->simScorer(simWeight, context);
}

shared_ptr<Explanation>
SpanWeight::explain(shared_ptr<LeafReaderContext> context,
                    int doc) 
{
  shared_ptr<SpanScorer> scorer = this->scorer(context);
  if (scorer != nullptr) {
    int newDoc = scorer->begin().advance(doc);
    if (newDoc == doc) {
      float freq = scorer->sloppyFreq();
      shared_ptr<Similarity::SimScorer> docScorer =
          similarity->simScorer(simWeight, context);
      shared_ptr<Explanation> freqExplanation =
          Explanation::match(freq, L"phraseFreq=" + to_wstring(freq));
      shared_ptr<Explanation> scoreExplanation =
          docScorer->explain(doc, freqExplanation);
      return Explanation::match(
          scoreExplanation->getValue(),
          L"weight(" + getQuery() + L" in " + to_wstring(doc) + L") [" +
              similarity->getClass().getSimpleName() + L"], result of:",
          scoreExplanation);
    }
  }

  return Explanation::noMatch(L"no matching term");
}
} // namespace org::apache::lucene::search::spans