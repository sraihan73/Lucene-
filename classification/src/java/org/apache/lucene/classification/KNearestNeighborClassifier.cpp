using namespace std;

#include "KNearestNeighborClassifier.h"
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
#include "../../../../../../../queries/src/java/org/apache/lucene/queries/mlt/MoreLikeThis.h"
#include "ClassificationResult.h"

namespace org::apache::lucene::classification
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using IndexableField = org::apache::lucene::index::IndexableField;
using Term = org::apache::lucene::index::Term;
using MoreLikeThis = org::apache::lucene::queries::mlt::MoreLikeThis;
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

KNearestNeighborClassifier::KNearestNeighborClassifier(
    shared_ptr<IndexReader> indexReader, shared_ptr<Similarity> similarity,
    shared_ptr<Analyzer> analyzer, shared_ptr<Query> query, int k,
    int minDocsFreq, int minTermFreq, const wstring &classFieldName,
    deque<wstring> &textFieldNames)
    : mlt(make_shared<MoreLikeThis>(indexReader)),
      textFieldNames(textFieldNames), classFieldName(classFieldName),
      indexSearcher(make_shared<IndexSearcher>(indexReader)), k(k), query(query)
{
  this->mlt->setAnalyzer(analyzer);
  this->mlt->setFieldNames(textFieldNames);
  if (similarity != nullptr) {
    this->indexSearcher->setSimilarity(similarity);
  } else {
    this->indexSearcher->setSimilarity(make_shared<BM25Similarity>());
  }
  if (minDocsFreq > 0) {
    mlt->setMinDocFreq(minDocsFreq);
  }
  if (minTermFreq > 0) {
    mlt->setMinTermFreq(minTermFreq);
  }
}

shared_ptr<ClassificationResult<std::shared_ptr<BytesRef>>>
KNearestNeighborClassifier::assignClass(const wstring &text) 
{
  return classifyFromTopDocs(knnSearch(text));
}

shared_ptr<ClassificationResult<std::shared_ptr<BytesRef>>>
KNearestNeighborClassifier::classifyFromTopDocs(
    shared_ptr<TopDocs> knnResults) 
{
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
KNearestNeighborClassifier::getClasses(const wstring &text) 
{
  shared_ptr<TopDocs> knnResults = knnSearch(text);
  deque<ClassificationResult<std::shared_ptr<BytesRef>>> assignedClasses =
      buildListFromTopDocs(knnResults);
  sort(assignedClasses.begin(), assignedClasses.end());
  return assignedClasses;
}

deque<ClassificationResult<std::shared_ptr<BytesRef>>>
KNearestNeighborClassifier::getClasses(const wstring &text,
                                       int max) 
{
  shared_ptr<TopDocs> knnResults = knnSearch(text);
  deque<ClassificationResult<std::shared_ptr<BytesRef>>> assignedClasses =
      buildListFromTopDocs(knnResults);
  sort(assignedClasses.begin(), assignedClasses.end());
  return assignedClasses.subList(0, max);
}

shared_ptr<TopDocs>
KNearestNeighborClassifier::knnSearch(const wstring &text) 
{
  shared_ptr<BooleanQuery::Builder> mltQuery =
      make_shared<BooleanQuery::Builder>();
  for (auto fieldName : textFieldNames) {
    wstring boost = L"";
    mlt->setBoost(true); // terms boost actually helps in MLT queries
    if (fieldName.contains(L"^")) {
      std::deque<wstring> field2boost = fieldName.split(L"\\^");
      fieldName = field2boost[0];
      boost = field2boost[1];
    }
    if (boost != L"") {
      mlt->setBoostFactor(stof(boost)); // if we have a field boost, we add it
    }
    mltQuery->add(make_shared<BooleanClause>(
        mlt->like(fieldName, make_shared<StringReader>(text)),
        BooleanClause::Occur::SHOULD));
    mlt->setBoostFactor(1); // restore neutral boost for next field
  }
  shared_ptr<Query> classFieldQuery =
      make_shared<WildcardQuery>(make_shared<Term>(classFieldName, L"*"));
  mltQuery->add(
      make_shared<BooleanClause>(classFieldQuery, BooleanClause::Occur::MUST));
  if (query != nullptr) {
    mltQuery->add(query, BooleanClause::Occur::MUST);
  }
  return indexSearcher->search(mltQuery->build(), k);
}

deque<ClassificationResult<std::shared_ptr<BytesRef>>>
KNearestNeighborClassifier::buildListFromTopDocs(
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
    std::deque<std::shared_ptr<IndexableField>> storableFields =
        indexSearcher->doc(scoreDoc->doc)->getFields(classFieldName);
    for (auto singleStorableField : storableFields) {
      if (singleStorableField != nullptr) {
        shared_ptr<BytesRef> cl =
            make_shared<BytesRef>(singleStorableField->stringValue());
        // update count
        optional<int> count = classCounts[cl];
        if (count) {
          classCounts.emplace(cl, count + 1);
        } else {
          classCounts.emplace(cl, 1);
        }
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

wstring KNearestNeighborClassifier::toString()
{
  return wstring(L"KNearestNeighborClassifier{") +
         // C++ TODO: There is no native C++ equivalent to 'toString':
         L"textFieldNames=" + Arrays->toString(textFieldNames) +
         L", classFieldName='" + classFieldName +
         StringHelper::toString(L'\'') + L", k=" + to_wstring(k) + L", query=" +
         query + L", similarity=" + indexSearcher->getSimilarity(true) + L'}';
}
} // namespace org::apache::lucene::classification