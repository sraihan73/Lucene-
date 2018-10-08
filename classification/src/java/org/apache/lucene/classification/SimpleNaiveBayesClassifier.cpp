using namespace std;

#include "SimpleNaiveBayesClassifier.h"
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
#include "../../../../../../../core/src/java/org/apache/lucene/search/TotalHitCountCollector.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/WildcardQuery.h"
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
using TotalHitCountCollector =
    org::apache::lucene::search::TotalHitCountCollector;
using WildcardQuery = org::apache::lucene::search::WildcardQuery;
using BytesRef = org::apache::lucene::util::BytesRef;

SimpleNaiveBayesClassifier::SimpleNaiveBayesClassifier(
    shared_ptr<IndexReader> indexReader, shared_ptr<Analyzer> analyzer,
    shared_ptr<Query> query, const wstring &classFieldName,
    deque<wstring> &textFieldNames)
    : indexReader(indexReader), textFieldNames(textFieldNames),
      classFieldName(classFieldName), analyzer(analyzer),
      indexSearcher(make_shared<IndexSearcher>(this->indexReader)), query(query)
{
}

shared_ptr<ClassificationResult<std::shared_ptr<BytesRef>>>
SimpleNaiveBayesClassifier::assignClass(const wstring &inputDocument) throw(
    IOException)
{
  deque<ClassificationResult<std::shared_ptr<BytesRef>>> assignedClasses =
      assignClassNormalizedList(inputDocument);
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
SimpleNaiveBayesClassifier::getClasses(const wstring &text) 
{
  deque<ClassificationResult<std::shared_ptr<BytesRef>>> assignedClasses =
      assignClassNormalizedList(text);
  sort(assignedClasses.begin(), assignedClasses.end());
  return assignedClasses;
}

deque<ClassificationResult<std::shared_ptr<BytesRef>>>
SimpleNaiveBayesClassifier::getClasses(const wstring &text,
                                       int max) 
{
  deque<ClassificationResult<std::shared_ptr<BytesRef>>> assignedClasses =
      assignClassNormalizedList(text);
  sort(assignedClasses.begin(), assignedClasses.end());
  return assignedClasses.subList(0, max);
}

deque<ClassificationResult<std::shared_ptr<BytesRef>>>
SimpleNaiveBayesClassifier::assignClassNormalizedList(
    const wstring &inputDocument) 
{
  deque<ClassificationResult<std::shared_ptr<BytesRef>>> assignedClasses =
      deque<ClassificationResult<std::shared_ptr<BytesRef>>>();

  shared_ptr<Terms> classes =
      MultiFields::getTerms(indexReader, classFieldName);
  if (classes != nullptr) {
    shared_ptr<TermsEnum> classesEnum = classes->begin();
    shared_ptr<BytesRef> next;
    std::deque<wstring> tokenizedText = tokenize(inputDocument);
    int docsWithClassSize = countDocsWithClass();
    while ((next = classesEnum->next()) != nullptr) {
      if (next->length > 0) {
        shared_ptr<Term> term = make_shared<Term>(this->classFieldName, next);
        double clVal =
            calculateLogPrior(term, docsWithClassSize) +
            calculateLogLikelihood(tokenizedText, term, docsWithClassSize);
        assignedClasses.push_back(
            make_shared<ClassificationResult<>>(term->bytes(), clVal));
      }
    }
  }
  // normalization; the values transforms to a 0-1 range
  return normClassificationResults(assignedClasses);
}

int SimpleNaiveBayesClassifier::countDocsWithClass() 
{
  shared_ptr<Terms> terms =
      MultiFields::getTerms(this->indexReader, this->classFieldName);
  int docCount;
  if (terms == nullptr ||
      terms->getDocCount() == -1) { // in case codec doesn't support getDocCount
    shared_ptr<TotalHitCountCollector> classQueryCountCollector =
        make_shared<TotalHitCountCollector>();
    shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
    q->add(make_shared<BooleanClause>(
        make_shared<WildcardQuery>(make_shared<Term>(
            classFieldName,
            StringHelper::toString(WildcardQuery::WILDCARD_STRING))),
        BooleanClause::Occur::MUST));
    if (query != nullptr) {
      q->add(query, BooleanClause::Occur::MUST);
    }
    indexSearcher->search(q->build(), classQueryCountCollector);
    docCount = classQueryCountCollector->getTotalHits();
  } else {
    docCount = terms->getDocCount();
  }
  return docCount;
}

std::deque<wstring>
SimpleNaiveBayesClassifier::tokenize(const wstring &text) 
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

double SimpleNaiveBayesClassifier::calculateLogLikelihood(
    std::deque<wstring> &tokenizedText, shared_ptr<Term> term,
    int docsWithClass) 
{
  // for each word
  double result = 0;
  for (auto word : tokenizedText) {
    // search with text:word AND class:c
    int hits = getWordFreqForClass(word, term);

    // num : count the no of times the word appears in documents of class c (+1)
    double num = hits + 1; // +1 is added because of add 1 smoothing

    // den : for the whole dictionary, count the no of times a word appears in
    // documents of class c (+|V|)
    double den = getTextTermFreqForClass(term) + docsWithClass;

    // P(w|c) = num/den
    double wordProbability = num / den;
    result += log(wordProbability);
  }

  // log(P(d|c)) = log(P(w1|c))+...+log(P(wn|c))
  return result;
}

double SimpleNaiveBayesClassifier::getTextTermFreqForClass(
    shared_ptr<Term> term) 
{
  double avgNumberOfUniqueTerms = 0;
  for (auto textFieldName : textFieldNames) {
    shared_ptr<Terms> terms = MultiFields::getTerms(indexReader, textFieldName);
    int64_t numPostings = terms->getSumDocFreq(); // number of term/doc pairs
    avgNumberOfUniqueTerms +=
        numPostings /
        static_cast<double>(
            terms->getDocCount()); // avg # of unique terms per doc
  }
  int docsWithC = indexReader->docFreq(term);
  return avgNumberOfUniqueTerms * docsWithC; // avg # of unique terms in text
                                             // fields per doc * # docs with c
}

int SimpleNaiveBayesClassifier::getWordFreqForClass(
    const wstring &word, shared_ptr<Term> term) 
{
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

double SimpleNaiveBayesClassifier::calculateLogPrior(
    shared_ptr<Term> term, int docsWithClassSize) 
{
  return log(static_cast<double>(docCount(term))) - log(docsWithClassSize);
}

int SimpleNaiveBayesClassifier::docCount(shared_ptr<Term> term) throw(
    IOException)
{
  return indexReader->docFreq(term);
}

deque<ClassificationResult<std::shared_ptr<BytesRef>>>
SimpleNaiveBayesClassifier::normClassificationResults(
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
} // namespace org::apache::lucene::classification