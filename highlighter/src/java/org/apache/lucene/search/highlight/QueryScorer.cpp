using namespace std;

#include "QueryScorer.h"

namespace org::apache::lucene::search::highlight
{
using CachingTokenFilter = org::apache::lucene::analysis::CachingTokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using IndexReader = org::apache::lucene::index::IndexReader;
using MemoryIndex = org::apache::lucene::index::memory::MemoryIndex;
using Query = org::apache::lucene::search::Query;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;

QueryScorer::QueryScorer(shared_ptr<Query> query)
{
  init(query, L"", nullptr, true);
}

QueryScorer::QueryScorer(shared_ptr<Query> query, const wstring &field)
{
  init(query, field, nullptr, true);
}

QueryScorer::QueryScorer(shared_ptr<Query> query,
                         shared_ptr<IndexReader> reader, const wstring &field)
{
  init(query, field, reader, true);
}

QueryScorer::QueryScorer(shared_ptr<Query> query,
                         shared_ptr<IndexReader> reader, const wstring &field,
                         const wstring &defaultField)
{
  this->defaultField = defaultField;
  init(query, field, reader, true);
}

QueryScorer::QueryScorer(shared_ptr<Query> query, const wstring &field,
                         const wstring &defaultField)
{
  this->defaultField = defaultField;
  init(query, field, nullptr, true);
}

QueryScorer::QueryScorer(
    std::deque<std::shared_ptr<WeightedSpanTerm>> &weightedTerms)
{
  this->fieldWeightedSpanTerms = unordered_map<>(weightedTerms.size());

  for (int i = 0; i < weightedTerms.size(); i++) {
    shared_ptr<WeightedSpanTerm> existingTerm =
        fieldWeightedSpanTerms[weightedTerms[i]->term];

    if ((existingTerm == nullptr) ||
        (existingTerm->weight < weightedTerms[i]->weight)) {
      // if a term is defined more than once, always use the highest
      // scoring weight
      fieldWeightedSpanTerms.emplace(weightedTerms[i]->term, weightedTerms[i]);
      maxTermWeight = max(maxTermWeight, weightedTerms[i]->getWeight());
    }
  }
  skipInitExtractor = true;
}

float QueryScorer::getFragmentScore() { return totalScore; }

float QueryScorer::getMaxTermWeight() { return maxTermWeight; }

float QueryScorer::getTokenScore()
{
  position += posIncAtt->getPositionIncrement();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring termText = termAtt->toString();

  shared_ptr<WeightedSpanTerm> weightedSpanTerm;

  if ((weightedSpanTerm = fieldWeightedSpanTerms[termText]) == nullptr) {
    return 0;
  }

  if (weightedSpanTerm->positionSensitive &&
      !weightedSpanTerm->checkPosition(position)) {
    return 0;
  }

  float score = weightedSpanTerm->getWeight();

  // found a query term - is it unique in this doc?
  if (!foundTerms->contains(termText)) {
    totalScore += score;
    foundTerms->add(termText);
  }

  return score;
}

shared_ptr<TokenStream>
QueryScorer::init(shared_ptr<TokenStream> tokenStream) 
{
  position = -1;
  termAtt = tokenStream->addAttribute(CharTermAttribute::typeid);
  posIncAtt = tokenStream->addAttribute(PositionIncrementAttribute::typeid);
  if (!skipInitExtractor) {
    if (fieldWeightedSpanTerms.size() > 0) {
      fieldWeightedSpanTerms.clear();
    }
    return initExtractor(tokenStream);
  }
  return nullptr;
}

shared_ptr<WeightedSpanTerm>
QueryScorer::getWeightedSpanTerm(const wstring &token)
{
  return fieldWeightedSpanTerms[token];
}

void QueryScorer::init(shared_ptr<Query> query, const wstring &field,
                       shared_ptr<IndexReader> reader,
                       bool expandMultiTermQuery)
{
  this->reader = reader;
  this->expandMultiTermQuery = expandMultiTermQuery;
  this->query = query;
  this->field = field;
}

shared_ptr<TokenStream> QueryScorer::initExtractor(
    shared_ptr<TokenStream> tokenStream) 
{
  shared_ptr<WeightedSpanTermExtractor> qse = newTermExtractor(defaultField);
  qse->setMaxDocCharsToAnalyze(maxCharsToAnalyze);
  qse->setExpandMultiTermQuery(expandMultiTermQuery);
  qse->setWrapIfNotCachingTokenFilter(wrapToCaching);
  qse->setUsePayloads(usePayloads);
  if (reader == nullptr) {
    this->fieldWeightedSpanTerms =
        qse->getWeightedSpanTerms(query, 1.0f, tokenStream, field);
  } else {
    this->fieldWeightedSpanTerms = qse->getWeightedSpanTermsWithScores(
        query, 1.0f, tokenStream, field, reader);
  }
  if (qse->isCachedTokenStream()) {
    return qse->getTokenStream();
  }

  return nullptr;
}

shared_ptr<WeightedSpanTermExtractor>
QueryScorer::newTermExtractor(const wstring &defaultField)
{
  return defaultField == L""
             ? make_shared<WeightedSpanTermExtractor>()
             : make_shared<WeightedSpanTermExtractor>(defaultField);
}

void QueryScorer::startFragment(shared_ptr<TextFragment> newFragment)
{
  foundTerms = unordered_set<>();
  totalScore = 0;
}

bool QueryScorer::isExpandMultiTermQuery() { return expandMultiTermQuery; }

void QueryScorer::setExpandMultiTermQuery(bool expandMultiTermQuery)
{
  this->expandMultiTermQuery = expandMultiTermQuery;
}

bool QueryScorer::isUsePayloads() { return usePayloads; }

void QueryScorer::setUsePayloads(bool usePayloads)
{
  this->usePayloads = usePayloads;
}

void QueryScorer::setWrapIfNotCachingTokenFilter(bool wrap)
{
  this->wrapToCaching = wrap;
}

void QueryScorer::setMaxDocCharsToAnalyze(int maxDocCharsToAnalyze)
{
  this->maxCharsToAnalyze = maxDocCharsToAnalyze;
}
} // namespace org::apache::lucene::search::highlight