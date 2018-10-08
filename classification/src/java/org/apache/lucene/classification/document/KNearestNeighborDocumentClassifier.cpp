using namespace std;

#include "KNearestNeighborDocumentClassifier.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/Term.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/BooleanClause.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/BooleanQuery.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/Query.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/TopDocs.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/WildcardQuery.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/similarities/Similarity.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../ClassificationResult.h"

namespace org::apache::lucene::classification::document
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using ClassificationResult =
    org::apache::lucene::classification::ClassificationResult;
using KNearestNeighborClassifier =
    org::apache::lucene::classification::KNearestNeighborClassifier;
using Document = org::apache::lucene::document::Document;
using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using TopDocs = org::apache::lucene::search::TopDocs;
using WildcardQuery = org::apache::lucene::search::WildcardQuery;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using BytesRef = org::apache::lucene::util::BytesRef;

KNearestNeighborDocumentClassifier::KNearestNeighborDocumentClassifier(
    shared_ptr<IndexReader> indexReader, shared_ptr<Similarity> similarity,
    shared_ptr<Query> query, int k, int minDocsFreq, int minTermFreq,
    const wstring &classFieldName,
    unordered_map<wstring, std::shared_ptr<Analyzer>> &field2analyzer,
    deque<wstring> &textFieldNames)
    : org::apache::lucene::classification::KNearestNeighborClassifier(
          indexReader, similarity, nullptr, query, k, minDocsFreq, minTermFreq,
          classFieldName, textFieldNames)
{
  this->field2analyzer = field2analyzer;
}

shared_ptr<ClassificationResult<std::shared_ptr<BytesRef>>>
KNearestNeighborDocumentClassifier::assignClass(
    shared_ptr<Document> document) 
{
  return classifyFromTopDocs(knnSearch(document));
}

deque<ClassificationResult<std::shared_ptr<BytesRef>>>
KNearestNeighborDocumentClassifier::getClasses(
    shared_ptr<Document> document) 
{
  shared_ptr<TopDocs> knnResults = knnSearch(document);
  deque<ClassificationResult<std::shared_ptr<BytesRef>>> assignedClasses =
      buildListFromTopDocs(knnResults);
  sort(assignedClasses.begin(), assignedClasses.end());
  return assignedClasses;
}

deque<ClassificationResult<std::shared_ptr<BytesRef>>>
KNearestNeighborDocumentClassifier::getClasses(shared_ptr<Document> document,
                                               int max) 
{
  shared_ptr<TopDocs> knnResults = knnSearch(document);
  deque<ClassificationResult<std::shared_ptr<BytesRef>>> assignedClasses =
      buildListFromTopDocs(knnResults);
  sort(assignedClasses.begin(), assignedClasses.end());
  max = min(max, assignedClasses.size());
  return assignedClasses.subList(0, max);
}

shared_ptr<TopDocs> KNearestNeighborDocumentClassifier::knnSearch(
    shared_ptr<Document> document) 
{
  shared_ptr<BooleanQuery::Builder> mltQuery =
      make_shared<BooleanQuery::Builder>();

  for (auto fieldName : textFieldNames) {
    wstring boost = L"";
    if (fieldName.contains(L"^")) {
      std::deque<wstring> field2boost = fieldName.split(L"\\^");
      fieldName = field2boost[0];
      boost = field2boost[1];
    }
    std::deque<wstring> fieldValues = document->getValues(fieldName);
    mlt->setBoost(true); // we want always to use the boost coming from TF * IDF
                         // of the term
    if (boost != L"") {
      mlt->setBoostFactor(stof(boost)); // this is an additional multiplicative
                                        // boost coming from the field boost
    }
    mlt->setAnalyzer(field2analyzer[fieldName]);
    for (auto fieldContent : fieldValues) {
      mltQuery->add(make_shared<BooleanClause>(
          mlt->like(fieldName, make_shared<StringReader>(fieldContent)),
          BooleanClause::Occur::SHOULD));
    }
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
} // namespace org::apache::lucene::classification::document