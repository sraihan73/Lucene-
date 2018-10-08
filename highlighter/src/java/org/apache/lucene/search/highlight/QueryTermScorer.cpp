using namespace std;

#include "QueryTermScorer.h"

namespace org::apache::lucene::search::highlight
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using IndexReader = org::apache::lucene::index::IndexReader;
using Query = org::apache::lucene::search::Query;

QueryTermScorer::QueryTermScorer(shared_ptr<Query> query)
    : QueryTermScorer(QueryTermExtractor::getTerms(query))
{
}

QueryTermScorer::QueryTermScorer(shared_ptr<Query> query,
                                 const wstring &fieldName)
    : QueryTermScorer(QueryTermExtractor::getTerms(query, false, fieldName))
{
}

QueryTermScorer::QueryTermScorer(shared_ptr<Query> query,
                                 shared_ptr<IndexReader> reader,
                                 const wstring &fieldName)
    : QueryTermScorer(
          QueryTermExtractor::getIdfWeightedTerms(query, reader, fieldName))
{
}

QueryTermScorer::QueryTermScorer(
    std::deque<std::shared_ptr<WeightedTerm>> &weightedTerms)
{
  termsToFind = unordered_map<wstring, std::shared_ptr<WeightedTerm>>();
  for (int i = 0; i < weightedTerms.size(); i++) {
    shared_ptr<WeightedTerm> existingTerm = termsToFind[weightedTerms[i]->term];
    if ((existingTerm == nullptr) ||
        (existingTerm->weight < weightedTerms[i]->weight)) {
      // if a term is defined more than once, always use the highest scoring
      // weight
      termsToFind.emplace(weightedTerms[i]->term, weightedTerms[i]);
      maxTermWeight = max(maxTermWeight, weightedTerms[i]->getWeight());
    }
  }
}

shared_ptr<TokenStream>
QueryTermScorer::init(shared_ptr<TokenStream> tokenStream)
{
  termAtt = tokenStream->addAttribute(CharTermAttribute::typeid);
  return nullptr;
}

void QueryTermScorer::startFragment(shared_ptr<TextFragment> newFragment)
{
  uniqueTermsInFragment = unordered_set<wstring>();
  currentTextFragment = newFragment;
  totalScore = 0;
}

float QueryTermScorer::getTokenScore()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring termText = termAtt->toString();

  shared_ptr<WeightedTerm> queryTerm = termsToFind[termText];
  if (queryTerm == nullptr) {
    // not a query term - return
    return 0;
  }
  // found a query term - is it unique in this doc?
  if (!find(uniqueTermsInFragment.begin(), uniqueTermsInFragment.end(),
            termText) != uniqueTermsInFragment.end()) {
    totalScore += queryTerm->getWeight();
    uniqueTermsInFragment.insert(termText);
  }
  return queryTerm->getWeight();
}

float QueryTermScorer::getFragmentScore() { return totalScore; }

void QueryTermScorer::allFragmentsProcessed()
{
  // this class has no special operations to perform at end of processing
}

float QueryTermScorer::getMaxTermWeight() { return maxTermWeight; }
} // namespace org::apache::lucene::search::highlight