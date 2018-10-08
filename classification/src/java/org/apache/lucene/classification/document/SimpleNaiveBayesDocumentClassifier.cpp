using namespace std;

#include "SimpleNaiveBayesDocumentClassifier.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexableField.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/MultiFields.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/Term.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/Terms.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/TermsEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/BooleanClause.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/BooleanQuery.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/Query.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/TermQuery.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/TotalHitCountCollector.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../ClassificationResult.h"

namespace org::apache::lucene::classification::document
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using ClassificationResult =
    org::apache::lucene::classification::ClassificationResult;
using SimpleNaiveBayesClassifier =
    org::apache::lucene::classification::SimpleNaiveBayesClassifier;
using Document = org::apache::lucene::document::Document;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexableField = org::apache::lucene::index::IndexableField;
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

SimpleNaiveBayesDocumentClassifier::SimpleNaiveBayesDocumentClassifier(
    shared_ptr<IndexReader> indexReader, shared_ptr<Query> query,
    const wstring &classFieldName,
    unordered_map<wstring, std::shared_ptr<Analyzer>> &field2analyzer,
    deque<wstring> &textFieldNames)
    : org::apache::lucene::classification::SimpleNaiveBayesClassifier(
          indexReader, nullptr, query, classFieldName, textFieldNames)
{
  this->field2analyzer = field2analyzer;
}

shared_ptr<ClassificationResult<std::shared_ptr<BytesRef>>>
SimpleNaiveBayesDocumentClassifier::assignClass(
    shared_ptr<Document> document) 
{
  deque<ClassificationResult<std::shared_ptr<BytesRef>>> assignedClasses =
      assignNormClasses(document);
  shared_ptr<ClassificationResult<std::shared_ptr<BytesRef>>> assignedClass =
      nullptr;
  double maxscore = -numeric_limits<double>::max();
  for (auto c : assignedClasses) {
    if (c->getScore() > maxscore) {
      assignedClass = c;
      maxscore = c->getScore();
    }
  }
  return assignedClass;
}

deque<ClassificationResult<std::shared_ptr<BytesRef>>>
SimpleNaiveBayesDocumentClassifier::getClasses(
    shared_ptr<Document> document) 
{
  deque<ClassificationResult<std::shared_ptr<BytesRef>>> assignedClasses =
      assignNormClasses(document);
  sort(assignedClasses.begin(), assignedClasses.end());
  return assignedClasses;
}

deque<ClassificationResult<std::shared_ptr<BytesRef>>>
SimpleNaiveBayesDocumentClassifier::getClasses(shared_ptr<Document> document,
                                               int max) 
{
  deque<ClassificationResult<std::shared_ptr<BytesRef>>> assignedClasses =
      assignNormClasses(document);
  sort(assignedClasses.begin(), assignedClasses.end());
  return assignedClasses.subList(0, max);
}

deque<ClassificationResult<std::shared_ptr<BytesRef>>>
SimpleNaiveBayesDocumentClassifier::assignNormClasses(
    shared_ptr<Document> inputDocument) 
{
  deque<ClassificationResult<std::shared_ptr<BytesRef>>> assignedClasses =
      deque<ClassificationResult<std::shared_ptr<BytesRef>>>();
  unordered_map<wstring, deque<std::deque<wstring>>> fieldName2tokensArray =
      make_shared<LinkedHashMap<wstring, deque<std::deque<wstring>>>>();
  unordered_map<wstring, float> fieldName2boost =
      make_shared<LinkedHashMap<wstring, float>>();
  shared_ptr<Terms> classes =
      MultiFields::getTerms(indexReader, classFieldName);
  if (classes != nullptr) {
    shared_ptr<TermsEnum> classesEnum = classes->begin();
    shared_ptr<BytesRef> c;

    analyzeSeedDocument(inputDocument, fieldName2tokensArray, fieldName2boost);

    int docsWithClassSize = countDocsWithClass();
    while ((c = classesEnum->next()) != nullptr) {
      double classScore = 0;
      shared_ptr<Term> term = make_shared<Term>(this->classFieldName, c);
      for (auto fieldName : textFieldNames) {
        deque<std::deque<wstring>> tokensArrays =
            fieldName2tokensArray[fieldName];
        double fieldScore = 0;
        for (auto fieldTokensArray : tokensArrays) {
          fieldScore += calculateLogPrior(term, docsWithClassSize) +
                        calculateLogLikelihood(fieldTokensArray, fieldName,
                                               term, docsWithClassSize) *
                            fieldName2boost[fieldName];
        }
        classScore += fieldScore;
      }
      assignedClasses.push_back(
          make_shared<ClassificationResult<>>(term->bytes(), classScore));
    }
  }
  return normClassificationResults(assignedClasses);
}

void SimpleNaiveBayesDocumentClassifier::analyzeSeedDocument(
    shared_ptr<Document> inputDocument,
    unordered_map<wstring, deque<std::deque<wstring>>> &fieldName2tokensArray,
    unordered_map<wstring, float> &fieldName2boost) 
{
  for (int i = 0; i < textFieldNames.size(); i++) {
    wstring fieldName = textFieldNames[i];
    float boost = 1;
    deque<std::deque<wstring>> tokenizedValues = deque<std::deque<wstring>>();
    if (fieldName.find(L"^") != wstring::npos) {
      std::deque<wstring> field2boost = fieldName.split(L"\\^");
      fieldName = field2boost[0];
      boost = stof(field2boost[1]);
    }
    std::deque<std::shared_ptr<IndexableField>> fieldValues =
        inputDocument->getFields(fieldName);
    for (auto fieldValue : fieldValues) {
      shared_ptr<TokenStream> fieldTokens =
          fieldValue->tokenStream(field2analyzer[fieldName], nullptr);
      std::deque<wstring> fieldTokensArray = getTokenArray(fieldTokens);
      tokenizedValues.push_back(fieldTokensArray);
    }
    fieldName2tokensArray.emplace(fieldName, tokenizedValues);
    fieldName2boost.emplace(fieldName, boost);
    textFieldNames[i] = fieldName;
  }
}

std::deque<wstring> SimpleNaiveBayesDocumentClassifier::getTokenArray(
    shared_ptr<TokenStream> tokenizedText) 
{
  shared_ptr<deque<wstring>> tokens = deque<wstring>();
  shared_ptr<CharTermAttribute> charTermAttribute =
      tokenizedText->addAttribute(CharTermAttribute::typeid);
  tokenizedText->reset();
  while (tokenizedText->incrementToken()) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    tokens->add(charTermAttribute->toString());
  }
  tokenizedText->end();
  delete tokenizedText;
  return tokens->toArray(std::deque<wstring>(tokens->size()));
}

double SimpleNaiveBayesDocumentClassifier::calculateLogLikelihood(
    std::deque<wstring> &tokenizedText, const wstring &fieldName,
    shared_ptr<Term> term, int docsWithClass) 
{
  // for each word
  double result = 0;
  for (auto word : tokenizedText) {
    // search with text:word AND class:c
    int hits = getWordFreqForClass(word, fieldName, term);

    // num : count the no of times the word appears in documents of class c (+1)
    double num = hits + 1; // +1 is added because of add 1 smoothing

    // den : for the whole dictionary, count the no of times a word appears in
    // documents of class c (+|V|)
    double den = getTextTermFreqForClass(term, fieldName) + docsWithClass;

    // P(w|c) = num/den
    double wordProbability = num / den;
    result += log(wordProbability);
  }

  // log(P(d|c)) = log(P(w1|c))+...+log(P(wn|c))
  double normScore =
      result /
      (tokenizedText
           .size()); // this is normalized because if not, long text fields will
                     // always be more important than short fields
  return normScore;
}

double SimpleNaiveBayesDocumentClassifier::getTextTermFreqForClass(
    shared_ptr<Term> term, const wstring &fieldName) 
{
  double avgNumberOfUniqueTerms;
  shared_ptr<Terms> terms = MultiFields::getTerms(indexReader, fieldName);
  int64_t numPostings = terms->getSumDocFreq(); // number of term/doc pairs
  avgNumberOfUniqueTerms =
      numPostings / static_cast<double>(
                        terms->getDocCount()); // avg # of unique terms per doc
  int docsWithC = indexReader->docFreq(term);
  return avgNumberOfUniqueTerms * docsWithC; // avg # of unique terms in text
                                             // fields per doc * # docs with c
}

int SimpleNaiveBayesDocumentClassifier::getWordFreqForClass(
    const wstring &word, const wstring &fieldName,
    shared_ptr<Term> term) 
{
  shared_ptr<BooleanQuery::Builder> booleanQuery =
      make_shared<BooleanQuery::Builder>();
  shared_ptr<BooleanQuery::Builder> subQuery =
      make_shared<BooleanQuery::Builder>();
  subQuery->add(make_shared<BooleanClause>(
      make_shared<TermQuery>(make_shared<Term>(fieldName, word)),
      BooleanClause::Occur::SHOULD));
  booleanQuery->add(make_shared<BooleanClause>(subQuery->build(),
                                               BooleanClause::Occur::MUST));
  booleanQuery->add(make_shared<BooleanClause>(make_shared<TermQuery>(term),
                                               BooleanClause::Occur::MUST));
  if (query != nullptr) {
    booleanQuery->add(query, BooleanClause::Occur::MUST);
  }
  shared_ptr<TotalHitCountCollector> totalHitCountCollector =
      make_shared<TotalHitCountCollector>();
  indexSearcher->search(booleanQuery->build(), totalHitCountCollector);
  return totalHitCountCollector->getTotalHits();
}

double SimpleNaiveBayesDocumentClassifier::calculateLogPrior(
    shared_ptr<Term> term, int docsWithClassSize) 
{
  return log(static_cast<double>(docCount(term))) - log(docsWithClassSize);
}

int SimpleNaiveBayesDocumentClassifier::docCount(shared_ptr<Term> term) throw(
    IOException)
{
  return indexReader->docFreq(term);
}
} // namespace org::apache::lucene::classification::document