using namespace std;

#include "ConfusionMatrixGeneratorTest.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/LeafReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockAnalyzer.h"
#include "../../../../../../java/org/apache/lucene/classification/BM25NBClassifier.h"
#include "../../../../../../java/org/apache/lucene/classification/BooleanPerceptronClassifier.h"
#include "../../../../../../java/org/apache/lucene/classification/CachingNaiveBayesClassifier.h"
#include "../../../../../../java/org/apache/lucene/classification/ClassificationResult.h"
#include "../../../../../../java/org/apache/lucene/classification/KNearestFuzzyClassifier.h"
#include "../../../../../../java/org/apache/lucene/classification/KNearestNeighborClassifier.h"
#include "../../../../../../java/org/apache/lucene/classification/SimpleNaiveBayesClassifier.h"
#include "../../../../../../java/org/apache/lucene/classification/utils/ConfusionMatrixGenerator.h"

namespace org::apache::lucene::classification::utils
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using BM25NBClassifier = org::apache::lucene::classification::BM25NBClassifier;
using BooleanPerceptronClassifier =
    org::apache::lucene::classification::BooleanPerceptronClassifier;
using CachingNaiveBayesClassifier =
    org::apache::lucene::classification::CachingNaiveBayesClassifier;
using ClassificationResult =
    org::apache::lucene::classification::ClassificationResult;
using ClassificationTestBase =
    org::apache::lucene::classification::ClassificationTestBase;
using Classifier = org::apache::lucene::classification::Classifier;
using KNearestFuzzyClassifier =
    org::apache::lucene::classification::KNearestFuzzyClassifier;
using KNearestNeighborClassifier =
    org::apache::lucene::classification::KNearestNeighborClassifier;
using SimpleNaiveBayesClassifier =
    org::apache::lucene::classification::SimpleNaiveBayesClassifier;
using LeafReader = org::apache::lucene::index::LeafReader;
using BytesRef = org::apache::lucene::util::BytesRef;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGetConfusionMatrix() throws Exception
void ConfusionMatrixGeneratorTest::testGetConfusionMatrix() 
{
  shared_ptr<LeafReader> reader = nullptr;
  try {
    shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
    reader = getSampleIndex(analyzer);
    shared_ptr<Classifier<std::shared_ptr<BytesRef>>> classifier =
        make_shared<ClassifierAnonymousInnerClass>(shared_from_this());
    shared_ptr<ConfusionMatrixGenerator::ConfusionMatrix> confusionMatrix =
        ConfusionMatrixGenerator::getConfusionMatrix(
            reader, classifier, categoryFieldName, textFieldName, -1);
    assertNotNull(confusionMatrix);
    assertNotNull(confusionMatrix->getLinearizedMatrix());
    assertEquals(7, confusionMatrix->getNumberOfEvaluatedDocs());
    double avgClassificationTime = confusionMatrix->getAvgClassificationTime();
    assertTrue(avgClassificationTime >= 0);
    double accuracy = confusionMatrix->getAccuracy();
    assertTrue(accuracy >= 0);
    assertTrue(accuracy <= 1);
    double precision = confusionMatrix->getPrecision();
    assertTrue(precision >= 0);
    assertTrue(precision <= 1);
    double recall = confusionMatrix->getRecall();
    assertTrue(recall >= 0);
    assertTrue(recall <= 1);
    double f1Measure = confusionMatrix->getF1Measure();
    assertTrue(f1Measure >= 0);
    assertTrue(f1Measure <= 1);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (reader != nullptr) {
      delete reader;
    }
  }
}

ConfusionMatrixGeneratorTest::ClassifierAnonymousInnerClass::
    ClassifierAnonymousInnerClass(
        shared_ptr<ConfusionMatrixGeneratorTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<ClassificationResult<std::shared_ptr<BytesRef>>>
ConfusionMatrixGeneratorTest::ClassifierAnonymousInnerClass::assignClass(
    const wstring &text) 
{
  return make_shared<ClassificationResult<std::shared_ptr<BytesRef>>>(
      make_shared<BytesRef>(), 1 / (1 + exp(-random()->nextInt())));
}

deque<ClassificationResult<std::shared_ptr<BytesRef>>>
ConfusionMatrixGeneratorTest::ClassifierAnonymousInnerClass::getClasses(
    const wstring &text) 
{
  return nullptr;
}

deque<ClassificationResult<std::shared_ptr<BytesRef>>>
ConfusionMatrixGeneratorTest::ClassifierAnonymousInnerClass::getClasses(
    const wstring &text, int max) 
{
  return nullptr;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGetConfusionMatrixWithSNB() throws
// Exception
void ConfusionMatrixGeneratorTest::testGetConfusionMatrixWithSNB() throw(
    runtime_error)
{
  shared_ptr<LeafReader> reader = nullptr;
  try {
    shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
    reader = getSampleIndex(analyzer);
    shared_ptr<Classifier<std::shared_ptr<BytesRef>>> classifier =
        make_shared<SimpleNaiveBayesClassifier>(
            reader, analyzer, nullptr, categoryFieldName, textFieldName);
    shared_ptr<ConfusionMatrixGenerator::ConfusionMatrix> confusionMatrix =
        ConfusionMatrixGenerator::getConfusionMatrix(
            reader, classifier, categoryFieldName, textFieldName, -1);
    checkCM(confusionMatrix);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (reader != nullptr) {
      delete reader;
    }
  }
}

void ConfusionMatrixGeneratorTest::checkCM(
    shared_ptr<ConfusionMatrixGenerator::ConfusionMatrix> confusionMatrix)
{
  assertNotNull(confusionMatrix);
  assertNotNull(confusionMatrix->getLinearizedMatrix());
  assertEquals(7, confusionMatrix->getNumberOfEvaluatedDocs());
  assertTrue(confusionMatrix->getAvgClassificationTime() >= 0);
  double accuracy = confusionMatrix->getAccuracy();
  assertTrue(accuracy >= 0);
  assertTrue(accuracy <= 1);
  double precision = confusionMatrix->getPrecision();
  assertTrue(precision >= 0);
  assertTrue(precision <= 1);
  double recall = confusionMatrix->getRecall();
  assertTrue(recall >= 0);
  assertTrue(recall <= 1);
  double f1Measure = confusionMatrix->getF1Measure();
  assertTrue(f1Measure >= 0);
  assertTrue(f1Measure <= 1);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGetConfusionMatrixWithBM25NB() throws
// Exception
void ConfusionMatrixGeneratorTest::testGetConfusionMatrixWithBM25NB() throw(
    runtime_error)
{
  shared_ptr<LeafReader> reader = nullptr;
  try {
    shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
    reader = getSampleIndex(analyzer);
    shared_ptr<Classifier<std::shared_ptr<BytesRef>>> classifier =
        make_shared<BM25NBClassifier>(reader, analyzer, nullptr,
                                      categoryFieldName, textFieldName);
    shared_ptr<ConfusionMatrixGenerator::ConfusionMatrix> confusionMatrix =
        ConfusionMatrixGenerator::getConfusionMatrix(
            reader, classifier, categoryFieldName, textFieldName, -1);
    checkCM(confusionMatrix);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (reader != nullptr) {
      delete reader;
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGetConfusionMatrixWithCNB() throws
// Exception
void ConfusionMatrixGeneratorTest::testGetConfusionMatrixWithCNB() throw(
    runtime_error)
{
  shared_ptr<LeafReader> reader = nullptr;
  try {
    shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
    reader = getSampleIndex(analyzer);
    shared_ptr<Classifier<std::shared_ptr<BytesRef>>> classifier =
        make_shared<CachingNaiveBayesClassifier>(
            reader, analyzer, nullptr, categoryFieldName, textFieldName);
    shared_ptr<ConfusionMatrixGenerator::ConfusionMatrix> confusionMatrix =
        ConfusionMatrixGenerator::getConfusionMatrix(
            reader, classifier, categoryFieldName, textFieldName, -1);
    checkCM(confusionMatrix);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (reader != nullptr) {
      delete reader;
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGetConfusionMatrixWithKNN() throws
// Exception
void ConfusionMatrixGeneratorTest::testGetConfusionMatrixWithKNN() throw(
    runtime_error)
{
  shared_ptr<LeafReader> reader = nullptr;
  try {
    shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
    reader = getSampleIndex(analyzer);
    shared_ptr<Classifier<std::shared_ptr<BytesRef>>> classifier =
        make_shared<KNearestNeighborClassifier>(
            reader, nullptr, analyzer, nullptr, 1, 0, 0, categoryFieldName,
            textFieldName);
    shared_ptr<ConfusionMatrixGenerator::ConfusionMatrix> confusionMatrix =
        ConfusionMatrixGenerator::getConfusionMatrix(
            reader, classifier, categoryFieldName, textFieldName, -1);
    checkCM(confusionMatrix);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (reader != nullptr) {
      delete reader;
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGetConfusionMatrixWithFLTKNN() throws
// Exception
void ConfusionMatrixGeneratorTest::testGetConfusionMatrixWithFLTKNN() throw(
    runtime_error)
{
  shared_ptr<LeafReader> reader = nullptr;
  try {
    shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
    reader = getSampleIndex(analyzer);
    shared_ptr<Classifier<std::shared_ptr<BytesRef>>> classifier =
        make_shared<KNearestFuzzyClassifier>(reader, nullptr, analyzer, nullptr,
                                             1, categoryFieldName,
                                             textFieldName);
    shared_ptr<ConfusionMatrixGenerator::ConfusionMatrix> confusionMatrix =
        ConfusionMatrixGenerator::getConfusionMatrix(
            reader, classifier, categoryFieldName, textFieldName, -1);
    checkCM(confusionMatrix);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (reader != nullptr) {
      delete reader;
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGetConfusionMatrixWithBP() throws
// Exception
void ConfusionMatrixGeneratorTest::testGetConfusionMatrixWithBP() throw(
    runtime_error)
{
  shared_ptr<LeafReader> reader = nullptr;
  try {
    shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
    reader = getSampleIndex(analyzer);
    shared_ptr<Classifier<bool>> classifier =
        make_shared<BooleanPerceptronClassifier>(reader, analyzer, nullptr, 1,
                                                 nullptr, booleanFieldName,
                                                 textFieldName);
    shared_ptr<ConfusionMatrixGenerator::ConfusionMatrix> confusionMatrix =
        ConfusionMatrixGenerator::getConfusionMatrix(
            reader, classifier, booleanFieldName, textFieldName, -1);
    checkCM(confusionMatrix);
    assertTrue(confusionMatrix->getPrecision(L"true") >= 0);
    assertTrue(confusionMatrix->getPrecision(L"true") <= 1);
    assertTrue(confusionMatrix->getPrecision(L"false") >= 0);
    assertTrue(confusionMatrix->getPrecision(L"false") <= 1);
    assertTrue(confusionMatrix->getRecall(L"true") >= 0);
    assertTrue(confusionMatrix->getRecall(L"true") <= 1);
    assertTrue(confusionMatrix->getRecall(L"false") >= 0);
    assertTrue(confusionMatrix->getRecall(L"false") <= 1);
    assertTrue(confusionMatrix->getF1Measure(L"true") >= 0);
    assertTrue(confusionMatrix->getF1Measure(L"true") <= 1);
    assertTrue(confusionMatrix->getF1Measure(L"false") >= 0);
    assertTrue(confusionMatrix->getF1Measure(L"false") <= 1);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (reader != nullptr) {
      delete reader;
    }
  }
}
} // namespace org::apache::lucene::classification::utils