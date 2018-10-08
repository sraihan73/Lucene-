using namespace std;

#include "KNearestFuzzyClassifier.h"
#include "../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/IndexableField.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/Term.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/BooleanClause.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/BooleanQuery.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/Query.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/TopDocs.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/WildcardQuery.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/similarities/BM25Similarity.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/similarities/Similarity.h"
#include "../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "ClassificationResult.h"
#include "utils/NearestFuzzyQuery.h"

namespace org::apache::lucene::classification
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using NearestFuzzyQuery =
    org::apache::lucene::classification::utils::NearestFuzzyQuery;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexableField = org::apache::lucene::index::IndexableField;
using LeafReader = org::apache::lucene::index::LeafReader;
using Term = org::apache::lucene::index::Term;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TopDocs = org::apache::lucene::search::TopDocs;
using WildcardQuery = org::apache::lucene::search::WildcardQuery;
using BM25Similarity =
    org::apache::lucene::search::similarities::BM25Similarity;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using BytesRef = org::apache::lucene::util::BytesRef;

KNearestFuzzyClassifier::KNearestFuzzyClassifier(
    shared_ptr<IndexReader> indexReader, shared_ptr<Similarity> similarity,
    shared_ptr<Analyzer> analyzer, shared_ptr<Query> query, int k,
    const wstring &classFieldName, deque<wstring> &textFieldNames)
    : textFieldNames(textFieldNames), classFieldName(classFieldName),
      indexSearcher(make_shared<IndexSearcher>(indexReader)), k(k),
      query(query), analyzer(analyzer)
{
  if (similarity != nullptr) {
    this->indexSearcher->setSimilarity(similarity);
  } else {
    this->indexSearcher->setSimilarity(make_shared<BM25Similarity>());
  }
}

shared_ptr<ClassificationResult<std::shared_ptr<BytesRef>>>
KNearestFuzzyClassifier::assignClass(const wstring &text) 
{
  shared_ptr<TopDocs> knnResults = knnSearch(text);
  deque<ClassificationResult<std::shared_ptr<BytesRef>>> assignedClasses =
      buildListFromTopDocs(knnResults);
  shared_ptr<ClassificationResult<std::shared_ptr<BytesRef>>> assignedClass =
      nullptr;
  double maxscore = -numeric_limits<double>::max();
  for (auto cl : assignedClasses) {
    if (cl->getScore() > maxscore) {
      assignedClass = cl;
      maxscore = cl->getScore();
    }
  }
  return assignedClass;
}

deque<ClassificationResult<std::shared_ptr<BytesRef>>>
KNearestFuzzyClassifier::getClasses(const wstring &text) 
{
  shared_ptr<TopDocs> knnResults = knnSearch(text);
  deque<ClassificationResult<std::shared_ptr<BytesRef>>> assignedClasses =
      buildListFromTopDocs(knnResults);
  sort(assignedClasses.begin(), assignedClasses.end());
  return assignedClasses;
}

deque<ClassificationResult<std::shared_ptr<BytesRef>>>
KNearestFuzzyClassifier::getClasses(const wstring &text,
                                    int max) 
{
  shared_ptr<TopDocs> knnResults = knnSearch(text);
  deque<ClassificationResult<std::shared_ptr<BytesRef>>> assignedClasses =
      buildListFromTopDocs(knnResults);
  sort(assignedClasses.begin(), assignedClasses.end());
  return assignedClasses.subList(0, max);
}

shared_ptr<TopDocs>
KNearestFuzzyClassifier::knnSearch(const wstring &text) 
{
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  shared_ptr<NearestFuzzyQuery> nearestFuzzyQuery =
      make_shared<NearestFuzzyQuery>(analyzer);
  for (auto fieldName : textFieldNames) {
    nearestFuzzyQuery->addTerms(text, fieldName);
  }
  bq->add(nearestFuzzyQuery, BooleanClause::Occur::MUST);
  shared_ptr<Query> classFieldQuery =
      make_shared<WildcardQuery>(make_shared<Term>(classFieldName, L"*"));
  bq->add(
      make_shared<BooleanClause>(classFieldQuery, BooleanClause::Occur::MUST));
  if (query != nullptr) {
    bq->add(query, BooleanClause::Occur::MUST);
  }
  return indexSearcher->search(bq->build(), k);
}

deque<ClassificationResult<std::shared_ptr<BytesRef>>>
KNearestFuzzyClassifier::buildListFromTopDocs(
    shared_ptr<TopDocs> topDocs) 
{
  unordered_map<std::shared_ptr<BytesRef>, int> classCounts =
      unordered_map<std::shared_ptr<BytesRef>, int>();
  unordered_map<std::shared_ptr<BytesRef>, double> classBoosts =
      unordered_map<std::shared_ptr<BytesRef>,
                    double>(); // this is a boost based on class ranking
                               // positions in topDocs
  float maxScore = topDocs->getMaxScore();
  for (auto scoreDoc : topDocs->scoreDocs) {
    shared_ptr<IndexableField> storableField =
        indexSearcher->doc(scoreDoc->doc)->getField(classFieldName);
    if (storableField != nullptr) {
      shared_ptr<BytesRef> cl =
          make_shared<BytesRef>(storableField->stringValue());
      // update count
      classCounts.merge(cl, 1, [&](a, b) { return a + b; });
      // update boost, the boost is based on the best score
      optional<double> totalBoost = classBoosts[cl];
      double singleBoost = scoreDoc->score / maxScore;
      if (totalBoost) {
        classBoosts.emplace(cl, totalBoost + singleBoost);
      } else {
        classBoosts.emplace(cl, singleBoost);
      }
    }
  }
  deque<ClassificationResult<std::shared_ptr<BytesRef>>> returnList =
      deque<ClassificationResult<std::shared_ptr<BytesRef>>>();
  deque<ClassificationResult<std::shared_ptr<BytesRef>>> temporaryList =
      deque<ClassificationResult<std::shared_ptr<BytesRef>>>();
  int sumdoc = 0;
  for (auto entry : classCounts) {
    optional<int> count = entry.second;
    optional<double> normBoost =
        classBoosts[entry.first] / count; // the boost is normalized to be 0<b<1
    temporaryList.push_back(make_shared<ClassificationResult<>>(
        entry.first::clone(), (count * normBoost) / static_cast<double>(k)));
    sumdoc += count;
  }

  // correction
  if (sumdoc < k) {
    for (auto cr : temporaryList) {
      returnList.push_back(make_shared<ClassificationResult<>>(
          cr->getAssignedClass(),
          cr->getScore() * k / static_cast<double>(sumdoc)));
    }
  } else {
    returnList = temporaryList;
  }
  return returnList;
}

wstring KNearestFuzzyClassifier::toString()
{
  return wstring(L"KNearestFuzzyClassifier{") +
         // C++ TODO: There is no native C++ equivalent to 'toString':
         L"textFieldNames=" + Arrays->toString(textFieldNames) +
         L", classFieldName='" + classFieldName +
         StringHelper::toString(L'\'') + L", k=" + to_wstring(k) + L", query=" +
         query + L", similarity=" + indexSearcher->getSimilarity(true) + L'}';
}
} // namespace org::apache::lucene::classification