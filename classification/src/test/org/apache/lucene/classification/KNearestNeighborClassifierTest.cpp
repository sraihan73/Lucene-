using namespace std;

#include "KNearestNeighborClassifierTest.h"
#include "../../../../../../../analysis/common/src/java/org/apache/lucene/analysis/en/EnglishAnalyzer.h"
#include "../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/LeafReader.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/MultiFields.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/Term.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/Terms.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/TermsEnum.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/TermQuery.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/similarities/BM25Similarity.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/similarities/LMDirichletSimilarity.h"
#include "../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockAnalyzer.h"
#include "../../../../../java/org/apache/lucene/classification/ClassificationResult.h"
#include "../../../../../java/org/apache/lucene/classification/KNearestNeighborClassifier.h"
#include "../../../../../java/org/apache/lucene/classification/utils/ConfusionMatrixGenerator.h"

namespace org::apache::lucene::classification
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using EnglishAnalyzer = org::apache::lucene::analysis::en::EnglishAnalyzer;
using ConfusionMatrixGenerator =
    org::apache::lucene::classification::utils::ConfusionMatrixGenerator;
using LeafReader = org::apache::lucene::index::LeafReader;
using MultiFields = org::apache::lucene::index::MultiFields;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using TermQuery = org::apache::lucene::search::TermQuery;
using BM25Similarity =
    org::apache::lucene::search::similarities::BM25Similarity;
using LMDirichletSimilarity =
    org::apache::lucene::search::similarities::LMDirichletSimilarity;
using BytesRef = org::apache::lucene::util::BytesRef;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testBasicUsage() throws Exception
void KNearestNeighborClassifierTest::testBasicUsage() 
{
  shared_ptr<LeafReader> leafReader = nullptr;
  try {
    shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
    leafReader = getSampleIndex(analyzer);
    checkCorrectClassification(make_shared<KNearestNeighborClassifier>(
                                   leafReader, nullptr, analyzer, nullptr, 1, 0,
                                   0, categoryFieldName, textFieldName),
                               TECHNOLOGY_INPUT, TECHNOLOGY_RESULT);
    checkCorrectClassification(
        make_shared<KNearestNeighborClassifier>(
            leafReader, make_shared<LMDirichletSimilarity>(), analyzer, nullptr,
            1, 0, 0, categoryFieldName, textFieldName),
        TECHNOLOGY_INPUT, TECHNOLOGY_RESULT);
    shared_ptr<ClassificationResult<std::shared_ptr<BytesRef>>> resultDS =
        checkCorrectClassification(
            make_shared<KNearestNeighborClassifier>(
                leafReader, make_shared<BM25Similarity>(), analyzer, nullptr, 3,
                2, 1, categoryFieldName, textFieldName),
            TECHNOLOGY_INPUT, TECHNOLOGY_RESULT);
    shared_ptr<ClassificationResult<std::shared_ptr<BytesRef>>> resultLMS =
        checkCorrectClassification(
            make_shared<KNearestNeighborClassifier>(
                leafReader, make_shared<LMDirichletSimilarity>(), analyzer,
                nullptr, 3, 2, 1, categoryFieldName, textFieldName),
            TECHNOLOGY_INPUT, TECHNOLOGY_RESULT);
    assertTrue(resultDS->getScore() != resultLMS->getScore());
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
// ORIGINAL LINE: @Test public void testRankedClasses() throws Exception
void KNearestNeighborClassifierTest::testRankedClasses() 
{
  shared_ptr<LeafReader> leafReader = nullptr;
  try {
    shared_ptr<Analyzer> analyzer = make_shared<EnglishAnalyzer>();
    leafReader = getSampleIndex(analyzer);
    shared_ptr<KNearestNeighborClassifier> knnClassifier =
        make_shared<KNearestNeighborClassifier>(
            leafReader, nullptr, analyzer, nullptr, 6, 1, 1, categoryFieldName,
            textFieldName);
    deque<ClassificationResult<std::shared_ptr<BytesRef>>> classes =
        knnClassifier->getClasses(STRONG_TECHNOLOGY_INPUT);
    assertTrue(classes[0]->getScore() > classes[1]->getScore());
    checkCorrectClassification(knnClassifier, STRONG_TECHNOLOGY_INPUT,
                               TECHNOLOGY_RESULT);
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
// ORIGINAL LINE: @Test public void testUnbalancedClasses() throws Exception
void KNearestNeighborClassifierTest::testUnbalancedClasses() throw(
    runtime_error)
{
  shared_ptr<LeafReader> leafReader = nullptr;
  try {
    shared_ptr<Analyzer> analyzer = make_shared<EnglishAnalyzer>();
    leafReader = getSampleIndex(analyzer);
    shared_ptr<KNearestNeighborClassifier> knnClassifier =
        make_shared<KNearestNeighborClassifier>(
            leafReader, nullptr, analyzer, nullptr, 3, 1, 1, categoryFieldName,
            textFieldName);
    deque<ClassificationResult<std::shared_ptr<BytesRef>>> classes =
        knnClassifier->getClasses(SUPER_STRONG_TECHNOLOGY_INPUT);
    assertTrue(classes[0]->getScore() > classes[1]->getScore());
    checkCorrectClassification(knnClassifier, SUPER_STRONG_TECHNOLOGY_INPUT,
                               TECHNOLOGY_RESULT);
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
void KNearestNeighborClassifierTest::testBasicUsageWithQuery() throw(
    runtime_error)
{
  shared_ptr<LeafReader> leafReader = nullptr;
  try {
    shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
    leafReader = getSampleIndex(analyzer);
    shared_ptr<TermQuery> query =
        make_shared<TermQuery>(make_shared<Term>(textFieldName, L"it"));
    checkCorrectClassification(make_shared<KNearestNeighborClassifier>(
                                   leafReader, nullptr, analyzer, query, 1, 0,
                                   0, categoryFieldName, textFieldName),
                               TECHNOLOGY_INPUT, TECHNOLOGY_RESULT);
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
// ORIGINAL LINE: @Test public void testPerformance() throws Exception
void KNearestNeighborClassifierTest::testPerformance() 
{
  shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<LeafReader> leafReader = getRandomIndex(analyzer, 100);
  try {
    shared_ptr<KNearestNeighborClassifier> kNearestNeighborClassifier =
        make_shared<KNearestNeighborClassifier>(
            leafReader, nullptr, analyzer, nullptr, 1, 1, 1, categoryFieldName,
            textFieldName);

    shared_ptr<ConfusionMatrixGenerator::ConfusionMatrix> confusionMatrix =
        ConfusionMatrixGenerator::getConfusionMatrix(
            leafReader, kNearestNeighborClassifier, categoryFieldName,
            textFieldName, -1);
    assertNotNull(confusionMatrix);

    double avgClassificationTime = confusionMatrix->getAvgClassificationTime();
    assertTrue(avgClassificationTime >= 0);

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