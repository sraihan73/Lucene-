using namespace std;

#include "CachingNaiveBayesClassifier.h"
#include "../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/MultiFields.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/Term.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/Terms.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/TermsEnum.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/BooleanClause.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/BooleanQuery.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/Query.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/TermQuery.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/TotalHitCountCollector.h"
#include "../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "ClassificationResult.h"

namespace org::apache::lucene::classification
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using IndexReader = org::apache::lucene::index::IndexReader;
using MultiFields = org::apache::lucene::index::MultiFields;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using TotalHitCountCollector =
    org::apache::lucene::search::TotalHitCountCollector;
using BytesRef = org::apache::lucene::util::BytesRef;

CachingNaiveBayesClassifier::CachingNaiveBayesClassifier(
    shared_ptr<IndexReader> indexReader, shared_ptr<Analyzer> analyzer,
    shared_ptr<Query> query, const wstring &classFieldName,
    deque<wstring> &textFieldNames)
    : SimpleNaiveBayesClassifier(indexReader, analyzer, query, classFieldName,
                                 textFieldNames)
{
  // building the cache
  try {
    reInitCache(0, true);
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

deque<ClassificationResult<std::shared_ptr<BytesRef>>>
CachingNaiveBayesClassifier::assignClassNormalizedList(
    const wstring &inputDocument) 
{
  std::deque<wstring> tokenizedText = tokenize(inputDocument);

  deque<ClassificationResult<std::shared_ptr<BytesRef>>> assignedClasses =
      calculateLogLikelihood(tokenizedText);

  // normalization
  // The values transforms to a 0-1 range
  deque<ClassificationResult<std::shared_ptr<BytesRef>>> asignedClassesNorm =
      SimpleNaiveBayesClassifier::normClassificationResults(assignedClasses);
  return asignedClassesNorm;
}

deque<ClassificationResult<std::shared_ptr<BytesRef>>>
CachingNaiveBayesClassifier::calculateLogLikelihood(
    std::deque<wstring> &tokenizedText) 
{
  // initialize the return List
  deque<ClassificationResult<std::shared_ptr<BytesRef>>> ret =
      deque<ClassificationResult<std::shared_ptr<BytesRef>>>();
  for (auto cclass : cclasses) {
    shared_ptr<ClassificationResult<std::shared_ptr<BytesRef>>> cr =
        make_shared<ClassificationResult<std::shared_ptr<BytesRef>>>(cclass, 0);
    ret.push_back(cr);
  }
  // for each word
  for (auto word : tokenizedText) {
    // search with text:word for all class:c
    unordered_map<std::shared_ptr<BytesRef>, int> hitsInClasses =
        getWordFreqForClassess(word);
    // for each class
    for (auto cclass : cclasses) {
      optional<int> hitsI = hitsInClasses[cclass];
      // if the word is out of scope hitsI could be null
      int hits = 0;
      if (hitsI) {
        hits = hitsI;
      }
      // num : count the no of times the word appears in documents of class
      // c(+1)
      double num = hits + 1; // +1 is added because of add 1 smoothing

      // den : for the whole dictionary, count the no of times a word appears in
      // documents of class c (+|V|)
      double den = classTermFreq[cclass] + docsWithClassSize;

      // P(w|c) = num/den
      double wordProbability = num / den;

      // modify the value in the result deque item
      int removeIdx = -1;
      int i = 0;
      for (auto cr : ret) {
        if (cr->getAssignedClass()->equals(cclass)) {
          removeIdx = i;
          break;
        }
        i++;
      }

      if (removeIdx >= 0) {
        shared_ptr<ClassificationResult<std::shared_ptr<BytesRef>>> toRemove =
            ret[removeIdx];
        ret.push_back(make_shared<ClassificationResult<>>(
            toRemove->getAssignedClass(),
            toRemove->getScore() + log(wordProbability)));
        ret.erase(ret.begin() + removeIdx);
      }
    }
  }

  // log(P(d|c)) = log(P(w1|c))+...+log(P(wn|c))
  return ret;
}

unordered_map<std::shared_ptr<BytesRef>, int>
CachingNaiveBayesClassifier::getWordFreqForClassess(const wstring &word) throw(
    IOException)
{

  unordered_map<std::shared_ptr<BytesRef>, int> insertPoint;
  insertPoint = termCClassHitCache[word];

  // if we get the answer from the cache
  if (insertPoint.size() > 0) {
    if (!insertPoint.empty()) {
      return insertPoint;
    }
  }

  unordered_map<std::shared_ptr<BytesRef>, int> searched =
      make_shared<ConcurrentHashMap<std::shared_ptr<BytesRef>, int>>();

  // if we dont get the answer, but it's relevant we must search it and insert
  // to the cache
  if (insertPoint.size() > 0 || !justCachedTerms) {
    for (auto cclass : cclasses) {
      shared_ptr<BooleanQuery::Builder> booleanQuery =
          make_shared<BooleanQuery::Builder>();
      shared_ptr<BooleanQuery::Builder> subQuery =
          make_shared<BooleanQuery::Builder>();
      for (auto textFieldName : textFieldNames) {
        subQuery->add(make_shared<BooleanClause>(
            make_shared<TermQuery>(make_shared<Term>(textFieldName, word)),
            BooleanClause::Occur::SHOULD));
      }
      booleanQuery->add(make_shared<BooleanClause>(subQuery->build(),
                                                   BooleanClause::Occur::MUST));
      booleanQuery->add(make_shared<BooleanClause>(
          make_shared<TermQuery>(make_shared<Term>(classFieldName, cclass)),
          BooleanClause::Occur::MUST));
      if (query != nullptr) {
        booleanQuery->add(query, BooleanClause::Occur::MUST);
      }
      shared_ptr<TotalHitCountCollector> totalHitCountCollector =
          make_shared<TotalHitCountCollector>();
      indexSearcher->search(booleanQuery->build(), totalHitCountCollector);

      int ret = totalHitCountCollector->getTotalHits();
      if (ret != 0) {
        searched.emplace(cclass, ret);
      }
    }
    if (insertPoint.size() > 0) {
      // threadsafe and concurrent write
      termCClassHitCache.emplace(word, searched);
    }
  }

  return searched;
}

void CachingNaiveBayesClassifier::reInitCache(
    int minTermOccurrenceInCache, bool justCachedTerms) 
{
  this->justCachedTerms = justCachedTerms;

  this->docsWithClassSize = countDocsWithClass();
  termCClassHitCache.clear();
  cclasses.clear();
  classTermFreq.clear();

  // build the cache for the word
  unordered_map<wstring, int64_t> frequencyMap =
      unordered_map<wstring, int64_t>();
  for (auto textFieldName : textFieldNames) {
    shared_ptr<TermsEnum> termsEnum =
        MultiFields::getTerms(indexReader, textFieldName)->begin();
    while (termsEnum->next() != nullptr) {
      shared_ptr<BytesRef> term = termsEnum->term();
      wstring termText = term->utf8ToString();
      int64_t frequency = termsEnum->docFreq();
      optional<int64_t> lastfreq = frequencyMap[termText];
      if (lastfreq) {
        frequency += lastfreq;
      }
      frequencyMap.emplace(termText, frequency);
    }
  }
  for (auto entry : frequencyMap) {
    if (entry.second > minTermOccurrenceInCache) {
      termCClassHitCache.emplace(
          entry.first,
          make_shared<ConcurrentHashMap<std::shared_ptr<BytesRef>, int>>());
    }
  }

  // fill the class deque
  shared_ptr<Terms> terms = MultiFields::getTerms(indexReader, classFieldName);
  shared_ptr<TermsEnum> termsEnum = terms->begin();
  while ((termsEnum->next()) != nullptr) {
    cclasses.push_back(BytesRef::deepCopyOf(termsEnum->term()));
  }
  // fill the classTermFreq map_obj
  for (auto cclass : cclasses) {
    double avgNumberOfUniqueTerms = 0;
    for (auto textFieldName : textFieldNames) {
      terms = MultiFields::getTerms(indexReader, textFieldName);
      int64_t numPostings =
          terms->getSumDocFreq(); // number of term/doc pairs
      avgNumberOfUniqueTerms +=
          numPostings / static_cast<double>(terms->getDocCount());
    }
    int docsWithC =
        indexReader->docFreq(make_shared<Term>(classFieldName, cclass));
    classTermFreq.emplace(cclass, avgNumberOfUniqueTerms * docsWithC);
  }
}
} // namespace org::apache::lucene::classification