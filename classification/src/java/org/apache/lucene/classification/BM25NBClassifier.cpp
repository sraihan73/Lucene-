using namespace std;

#include "BM25NBClassifier.h"
#include "../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/MultiFields.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/Term.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/Terms.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/TermsEnum.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/BooleanClause.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/BooleanQuery.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/Query.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/TermQuery.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/TopDocs.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/similarities/BM25Similarity.h"
#include "../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "ClassificationResult.h"

namespace org::apache::lucene::classification
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using IndexReader = org::apache::lucene::index::IndexReader;
using MultiFields = org::apache::lucene::index::MultiFields;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using BM25Similarity =
    org::apache::lucene::search::similarities::BM25Similarity;
using BytesRef = org::apache::lucene::util::BytesRef;

BM25NBClassifier::BM25NBClassifier(shared_ptr<IndexReader> indexReader,
                                   shared_ptr<Analyzer> analyzer,
                                   shared_ptr<Query> query,
                                   const wstring &classFieldName,
                                   deque<wstring> &textFieldNames)
    : indexReader(indexReader), textFieldNames(textFieldNames),
      classFieldName(classFieldName), analyzer(analyzer),
      indexSearcher(make_shared<IndexSearcher>(this->indexReader)), query(query)
{
  this->indexSearcher->setSimilarity(make_shared<BM25Similarity>());
}

shared_ptr<ClassificationResult<std::shared_ptr<BytesRef>>>
BM25NBClassifier::assignClass(const wstring &inputDocument) 
{
  return assignClassNormalizedList(inputDocument)[0];
}

deque<ClassificationResult<std::shared_ptr<BytesRef>>>
BM25NBClassifier::getClasses(const wstring &text) 
{
  deque<ClassificationResult<std::shared_ptr<BytesRef>>> assignedClasses =
      assignClassNormalizedList(text);
  sort(assignedClasses.begin(), assignedClasses.end());
  return assignedClasses;
}

deque<ClassificationResult<std::shared_ptr<BytesRef>>>
BM25NBClassifier::getClasses(const wstring &text, int max) 
{
  deque<ClassificationResult<std::shared_ptr<BytesRef>>> assignedClasses =
      assignClassNormalizedList(text);
  sort(assignedClasses.begin(), assignedClasses.end());
  return assignedClasses.subList(0, max);
}

deque<ClassificationResult<std::shared_ptr<BytesRef>>>
BM25NBClassifier::assignClassNormalizedList(const wstring &inputDocument) throw(
    IOException)
{
  deque<ClassificationResult<std::shared_ptr<BytesRef>>> assignedClasses =
      deque<ClassificationResult<std::shared_ptr<BytesRef>>>();

  shared_ptr<Terms> classes =
      MultiFields::getTerms(indexReader, classFieldName);
  shared_ptr<TermsEnum> classesEnum = classes->begin();
  shared_ptr<BytesRef> next;
  std::deque<wstring> tokenizedText = tokenize(inputDocument);
  while ((next = classesEnum->next()) != nullptr) {
    if (next->length > 0) {
      shared_ptr<Term> term = make_shared<Term>(this->classFieldName, next);
      assignedClasses.push_back(make_shared<ClassificationResult<>>(
          term->bytes(), calculateLogPrior(term) +
                             calculateLogLikelihood(tokenizedText, term)));
    }
  }

  return normClassificationResults(assignedClasses);
}

deque<ClassificationResult<std::shared_ptr<BytesRef>>>
BM25NBClassifier::normClassificationResults(
    deque<ClassificationResult<std::shared_ptr<BytesRef>>> &assignedClasses)
{
  // normalization; the values transforms to a 0-1 range
  deque<ClassificationResult<std::shared_ptr<BytesRef>>> returnList =
      deque<ClassificationResult<std::shared_ptr<BytesRef>>>();
  if (!assignedClasses.empty()) {
    sort(assignedClasses.begin(), assignedClasses.end());
    // this is a negative number closest to 0 = a
    double smax = assignedClasses[0]->getScore();

    double sumLog = 0;
    // log(sum(exp(x_n-a)))
    for (auto cr : assignedClasses) {
      // getScore-smax <=0 (both negative, smax is the smallest abs()
      sumLog += exp(cr->getScore() - smax);
    }
    // loga=a+log(sum(exp(x_n-a))) = log(sum(exp(x_n)))
    double loga = smax;
    loga += log(sumLog);

    // 1/sum*x = exp(log(x))*1/sum = exp(log(x)-log(sum))
    for (auto cr : assignedClasses) {
      double scoreDiff = cr->getScore() - loga;
      returnList.push_back(make_shared<ClassificationResult<>>(
          cr->getAssignedClass(), exp(scoreDiff)));
    }
  }
  return returnList;
}

std::deque<wstring>
BM25NBClassifier::tokenize(const wstring &text) 
{
  shared_ptr<deque<wstring>> result = deque<wstring>();
  for (auto textFieldName : textFieldNames) {
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream
    // tokenStream = analyzer.tokenStream(textFieldName, text))
    {
      org::apache::lucene::analysis::TokenStream tokenStream =
          analyzer->tokenStream(textFieldName, text);
      shared_ptr<CharTermAttribute> charTermAttribute =
          tokenStream->addAttribute(CharTermAttribute::typeid);
      tokenStream->reset();
      while (tokenStream->incrementToken()) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        result->add(charTermAttribute->toString());
      }
      tokenStream->end();
    }
  }
  return result->toArray(std::deque<wstring>(result->size()));
}

double BM25NBClassifier::calculateLogLikelihood(
    std::deque<wstring> &tokens, shared_ptr<Term> term) 
{
  double result = 0;
  for (auto word : tokens) {
    result += log(getTermProbForClass(term, {word}));
  }
  return result;
}

double
BM25NBClassifier::getTermProbForClass(shared_ptr<Term> classTerm,
                                      deque<wstring> &words) 
{
  shared_ptr<BooleanQuery::Builder> builder =
      make_shared<BooleanQuery::Builder>();
  builder->add(make_shared<BooleanClause>(make_shared<TermQuery>(classTerm),
                                          BooleanClause::Occur::MUST));
  for (auto textFieldName : textFieldNames) {
    for (wstring word : words) {
      builder->add(make_shared<BooleanClause>(
          make_shared<TermQuery>(make_shared<Term>(textFieldName, word)),
          BooleanClause::Occur::SHOULD));
    }
  }
  if (query != nullptr) {
    builder->add(query, BooleanClause::Occur::MUST);
  }
  shared_ptr<TopDocs> search = indexSearcher->search(builder->build(), 1);
  return search->totalHits > 0 ? search->getMaxScore() : 1;
}

double
BM25NBClassifier::calculateLogPrior(shared_ptr<Term> term) 
{
  shared_ptr<TermQuery> termQuery = make_shared<TermQuery>(term);
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(termQuery, BooleanClause::Occur::MUST);
  if (query != nullptr) {
    bq->add(query, BooleanClause::Occur::MUST);
  }
  shared_ptr<TopDocs> topDocs = indexSearcher->search(bq->build(), 1);
  return topDocs->totalHits > 0 ? log(topDocs->getMaxScore()) : 0;
}
} // namespace org::apache::lucene::classification