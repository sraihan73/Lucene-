using namespace std;

#include "QueryTermExtractor.h"

namespace org::apache::lucene::search::highlight
{
using IndexReader = org::apache::lucene::index::IndexReader;
using MultiReader = org::apache::lucene::index::MultiReader;
using Term = org::apache::lucene::index::Term;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
const shared_ptr<org::apache::lucene::search::IndexSearcher>
    QueryTermExtractor::EMPTY_INDEXSEARCHER;

QueryTermExtractor::StaticConstructor::StaticConstructor()
{
  try {
    shared_ptr<IndexReader> emptyReader = make_shared<MultiReader>();
    EMPTY_INDEXSEARCHER = make_shared<IndexSearcher>(emptyReader);
    EMPTY_INDEXSEARCHER->setQueryCache(nullptr);
  } catch (const IOException &bogus) {
    throw runtime_error(bogus);
  }
}

QueryTermExtractor::StaticConstructor QueryTermExtractor::staticConstructor;

std::deque<std::shared_ptr<WeightedTerm>>
QueryTermExtractor::getTerms(shared_ptr<Query> query)
{
  return getTerms(query, false);
}

std::deque<std::shared_ptr<WeightedTerm>>
QueryTermExtractor::getIdfWeightedTerms(shared_ptr<Query> query,
                                        shared_ptr<IndexReader> reader,
                                        const wstring &fieldName)
{
  std::deque<std::shared_ptr<WeightedTerm>> terms =
      getTerms(query, false, fieldName);
  int totalNumDocs = reader->maxDoc();
  for (int i = 0; i < terms.size(); i++) {
    try {
      int docFreq =
          reader->docFreq(make_shared<Term>(fieldName, terms[i]->term));
      // IDF algorithm taken from ClassicSimilarity class
      float idf = static_cast<float>(
          log(totalNumDocs / static_cast<double>(docFreq + 1)) + 1.0);
      terms[i]->weight *= idf;
    } catch (const IOException &e) {
      // ignore
    }
  }
  return terms;
}

std::deque<std::shared_ptr<WeightedTerm>>
QueryTermExtractor::getTerms(shared_ptr<Query> query, bool prohibited,
                             const wstring &fieldName)
{
  unordered_set<std::shared_ptr<WeightedTerm>> terms =
      unordered_set<std::shared_ptr<WeightedTerm>>();
  getTerms(query, 1.0f, terms, prohibited, fieldName);
  return terms.toArray(std::deque<std::shared_ptr<WeightedTerm>>(0));
}

std::deque<std::shared_ptr<WeightedTerm>>
QueryTermExtractor::getTerms(shared_ptr<Query> query, bool prohibited)
{
  return getTerms(query, prohibited, L"");
}

void QueryTermExtractor::getTerms(
    shared_ptr<Query> query, float boost,
    unordered_set<std::shared_ptr<WeightedTerm>> &terms, bool prohibited,
    const wstring &fieldName)
{
  try {
    if (std::dynamic_pointer_cast<BoostQuery>(query) != nullptr) {
      shared_ptr<BoostQuery> boostQuery =
          std::static_pointer_cast<BoostQuery>(query);
      getTerms(boostQuery->getQuery(), boost * boostQuery->getBoost(), terms,
               prohibited, fieldName);
    } else if (std::dynamic_pointer_cast<BooleanQuery>(query) != nullptr) {
      getTermsFromBooleanQuery(std::static_pointer_cast<BooleanQuery>(query),
                               boost, terms, prohibited, fieldName);
    } else {
      unordered_set<std::shared_ptr<Term>> nonWeightedTerms =
          unordered_set<std::shared_ptr<Term>>();
      try {
        EMPTY_INDEXSEARCHER
            ->createWeight(EMPTY_INDEXSEARCHER->rewrite(query), false, 1)
            ->extractTerms(nonWeightedTerms);
      } catch (const IOException &bogus) {
        // C++ TODO: This exception's constructor requires only one argument:
        // ORIGINAL LINE: throw new RuntimeException("Should not happen on an
        // empty index", bogus);
        throw runtime_error(L"Should not happen on an empty index");
      }
      for (unordered_set<std::shared_ptr<Term>>::const_iterator iter =
               nonWeightedTerms.begin();
           iter != nonWeightedTerms.end(); ++iter) {
        shared_ptr<Term> term = *iter;
        if ((fieldName == L"") || (term->field() == fieldName)) {
          terms.insert(make_shared<WeightedTerm>(boost, term->text()));
        }
      }
    }
  } catch (const UnsupportedOperationException &ignore) {
    // this is non-fatal for our purposes
  }
}

void QueryTermExtractor::getTermsFromBooleanQuery(
    shared_ptr<BooleanQuery> query, float boost,
    unordered_set<std::shared_ptr<WeightedTerm>> &terms, bool prohibited,
    const wstring &fieldName)
{
  for (auto clause : query) {
    if (prohibited || clause->getOccur() != BooleanClause::Occur::MUST_NOT) {
      getTerms(clause->getQuery(), boost, terms, prohibited, fieldName);
    }
  }
}
} // namespace org::apache::lucene::search::highlight