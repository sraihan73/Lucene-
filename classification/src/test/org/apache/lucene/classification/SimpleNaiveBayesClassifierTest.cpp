using namespace std;

#include "SimpleNaiveBayesClassifierTest.h"
#include "../../../../../../../analysis/common/src/java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../../analysis/common/src/java/org/apache/lucene/analysis/ngram/EdgeNGramTokenFilter.h"
#include "../../../../../../../analysis/common/src/java/org/apache/lucene/analysis/reverse/ReverseStringFilter.h"
#include "../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/LeafReader.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/MultiFields.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/Term.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/Terms.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/TermsEnum.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/TermQuery.h"
#include "../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockAnalyzer.h"
#include "../../../../../java/org/apache/lucene/classification/SimpleNaiveBayesClassifier.h"
#include "../../../../../java/org/apache/lucene/classification/utils/ConfusionMatrixGenerator.h"

namespace org::apache::lucene::classification
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using EdgeNGramTokenFilter =
    org::apache::lucene::analysis::ngram::EdgeNGramTokenFilter;
using ReverseStringFilter =
    org::apache::lucene::analysis::reverse::ReverseStringFilter;
using ConfusionMatrixGenerator =
    org::apache::lucene::classification::utils::ConfusionMatrixGenerator;
using LeafReader = org::apache::lucene::index::LeafReader;
using MultiFields = org::apache::lucene::index::MultiFields;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using TermQuery = org::apache::lucene::search::TermQuery;
using BytesRef = org::apache::lucene::util::BytesRef;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testBasicUsage() throws Exception
void SimpleNaiveBayesClassifierTest::testBasicUsage() 
{
  shared_ptr<LeafReader> leafReader = nullptr;
  try {
    shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
    leafReader = getSampleIndex(analyzer);
    shared_ptr<SimpleNaiveBayesClassifier> classifier =
        make_shared<SimpleNaiveBayesClassifier>(
            leafReader, analyzer, nullptr, categoryFieldName, textFieldName);
    checkCorrectClassification(classifier, TECHNOLOGY_INPUT, TECHNOLOGY_RESULT);
    checkCorrectClassification(classifier, POLITICS_INPUT, POLITICS_RESULT);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (leafReader != nullptr) {
      delete leafReader;
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testBasicUsageWithQuery() throws Exception
void SimpleNaiveBayesClassifierTest::testBasicUsageWithQuery() throw(
    runtime_error)
{
  shared_ptr<LeafReader> leafReader = nullptr;
  try {
    shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
    leafReader = getSampleIndex(analyzer);
    shared_ptr<TermQuery> query =
        make_shared<TermQuery>(make_shared<Term>(textFieldName, L"a"));
    shared_ptr<SimpleNaiveBayesClassifier> classifier =
        make_shared<SimpleNaiveBayesClassifier>(
            leafReader, analyzer, query, categoryFieldName, textFieldName);
    checkCorrectClassification(classifier, TECHNOLOGY_INPUT, TECHNOLOGY_RESULT);
    checkCorrectClassification(classifier, POLITICS_INPUT, POLITICS_RESULT);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (leafReader != nullptr) {
      delete leafReader;
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testNGramUsage() throws Exception
void SimpleNaiveBayesClassifierTest::testNGramUsage() 
{
  shared_ptr<LeafReader> leafReader = nullptr;
  try {
    shared_ptr<Analyzer> analyzer = make_shared<NGramAnalyzer>();
    leafReader = getSampleIndex(analyzer);
    shared_ptr<SimpleNaiveBayesClassifier> classifier =
        make_shared<SimpleNaiveBayesClassifier>(
            leafReader, analyzer, nullptr, categoryFieldName, textFieldName);
    checkCorrectClassification(classifier, TECHNOLOGY_INPUT, TECHNOLOGY_RESULT);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (leafReader != nullptr) {
      delete leafReader;
    }
  }
}

shared_ptr<Analyzer::TokenStreamComponents>
SimpleNaiveBayesClassifierTest::NGramAnalyzer::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> *const tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer,
      make_shared<ReverseStringFilter>(make_shared<EdgeNGramTokenFilter>(
          make_shared<ReverseStringFilter>(tokenizer), 10, 20, false)));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testPerformance() throws Exception
void SimpleNaiveBayesClassifierTest::testPerformance() 
{
  shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<LeafReader> leafReader = getRandomIndex(analyzer, 100);
  try {
    shared_ptr<SimpleNaiveBayesClassifier> simpleNaiveBayesClassifier =
        make_shared<SimpleNaiveBayesClassifier>(
            leafReader, analyzer, nullptr, categoryFieldName, textFieldName);

    shared_ptr<ConfusionMatrixGenerator::ConfusionMatrix> confusionMatrix =
        ConfusionMatrixGenerator::getConfusionMatrix(
            leafReader, simpleNaiveBayesClassifier, categoryFieldName,
            textFieldName, -1);
    assertNotNull(confusionMatrix);

    double avgClassificationTime = confusionMatrix->getAvgClassificationTime();
    assertTrue(avgClassificationTime >= 0);

    double f1 = confusionMatrix->getF1Measure();
    assertTrue(f1 >= 0);
    assertTrue(f1 <= 1);

    double accuracy = confusionMatrix->getAccuracy();
    assertTrue(accuracy >= 0);
    assertTrue(accuracy <= 1);

    double recall = confusionMatrix->getRecall();
    assertTrue(recall >= 0);
    assertTrue(recall <= 1);

    double precision = confusionMatrix->getPrecision();
    assertTrue(precision >= 0);
    assertTrue(precision <= 1);

    shared_ptr<Terms> terms =
        MultiFields::getTerms(leafReader, categoryFieldName);
    shared_ptr<TermsEnum> iterator = terms->begin();
    shared_ptr<BytesRef> term;
    while ((term = iterator->next()) != nullptr) {
      wstring s = term->utf8ToString();
      recall = confusionMatrix->getRecall(s);
      assertTrue(recall >= 0);
      assertTrue(recall <= 1);
      precision = confusionMatrix->getPrecision(s);
      assertTrue(precision >= 0);
      assertTrue(precision <= 1);
      double f1Measure = confusionMatrix->getF1Measure(s);
      assertTrue(f1Measure >= 0);
      assertTrue(f1Measure <= 1);
    }

  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete leafReader;
  }
}
} // namespace org::apache::lucene::classification