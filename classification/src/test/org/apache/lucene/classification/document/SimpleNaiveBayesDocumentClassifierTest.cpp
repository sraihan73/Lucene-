using namespace std;

#include "SimpleNaiveBayesDocumentClassifierTest.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../java/org/apache/lucene/classification/document/SimpleNaiveBayesDocumentClassifier.h"

namespace org::apache::lucene::classification::document
{
using BytesRef = org::apache::lucene::util::BytesRef;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testBasicDocumentClassification() throws
// Exception
void SimpleNaiveBayesDocumentClassifierTest::
    testBasicDocumentClassification() 
{
  try {
    checkCorrectDocumentClassification(
        make_shared<SimpleNaiveBayesDocumentClassifier>(
            indexReader, nullptr, categoryFieldName, field2analyzer,
            std::deque<wstring>{textFieldName, titleFieldName,
                                 authorFieldName}),
        getVideoGameDocument(), VIDEOGAME_ANALYZED_RESULT);
    checkCorrectDocumentClassification(
        make_shared<SimpleNaiveBayesDocumentClassifier>(
            indexReader, nullptr, categoryFieldName, field2analyzer,
            std::deque<wstring>{textFieldName, titleFieldName,
                                 authorFieldName}),
        getBatmanDocument(), BATMAN_RESULT);

    checkCorrectDocumentClassification(
        make_shared<SimpleNaiveBayesDocumentClassifier>(
            indexReader, nullptr, categoryFieldName, field2analyzer,
            std::deque<wstring>{textFieldName}),
        getVideoGameDocument(), BATMAN_RESULT);
    checkCorrectDocumentClassification(
        make_shared<SimpleNaiveBayesDocumentClassifier>(
            indexReader, nullptr, categoryFieldName, field2analyzer,
            std::deque<wstring>{textFieldName}),
        getBatmanDocument(), VIDEOGAME_ANALYZED_RESULT);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (indexReader != nullptr) {
      delete indexReader;
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testBasicDocumentClassificationScore()
// throws Exception
void SimpleNaiveBayesDocumentClassifierTest::
    testBasicDocumentClassificationScore() 
{
  try {
    double score1 = checkCorrectDocumentClassification(
        make_shared<SimpleNaiveBayesDocumentClassifier>(
            indexReader, nullptr, categoryFieldName, field2analyzer,
            std::deque<wstring>{textFieldName, titleFieldName,
                                 authorFieldName}),
        getVideoGameDocument(), VIDEOGAME_ANALYZED_RESULT);
    assertEquals(0.88, score1, 0.01);
    double score2 = checkCorrectDocumentClassification(
        make_shared<SimpleNaiveBayesDocumentClassifier>(
            indexReader, nullptr, categoryFieldName, field2analyzer,
            std::deque<wstring>{textFieldName, titleFieldName,
                                 authorFieldName}),
        getBatmanDocument(), BATMAN_RESULT);
    assertEquals(0.89, score2, 0.01);
    // taking in consideration only the text
    double score3 = checkCorrectDocumentClassification(
        make_shared<SimpleNaiveBayesDocumentClassifier>(
            indexReader, nullptr, categoryFieldName, field2analyzer,
            std::deque<wstring>{textFieldName}),
        getVideoGameDocument(), BATMAN_RESULT);
    assertEquals(0.55, score3, 0.01);
    double score4 = checkCorrectDocumentClassification(
        make_shared<SimpleNaiveBayesDocumentClassifier>(
            indexReader, nullptr, categoryFieldName, field2analyzer,
            std::deque<wstring>{textFieldName}),
        getBatmanDocument(), VIDEOGAME_ANALYZED_RESULT);
    assertEquals(0.52, score4, 0.01);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (indexReader != nullptr) {
      delete indexReader;
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testBoostedDocumentClassification() throws
// Exception
void SimpleNaiveBayesDocumentClassifierTest::
    testBoostedDocumentClassification() 
{
  try {
    checkCorrectDocumentClassification(
        make_shared<SimpleNaiveBayesDocumentClassifier>(
            indexReader, nullptr, categoryFieldName, field2analyzer,
            std::deque<wstring>{textFieldName, titleFieldName + L"^100",
                                 authorFieldName}),
        getBatmanAmbiguosDocument(), BATMAN_RESULT);
    // considering without boost wrong classification will appear
    checkCorrectDocumentClassification(
        make_shared<SimpleNaiveBayesDocumentClassifier>(
            indexReader, nullptr, categoryFieldName, field2analyzer,
            std::deque<wstring>{textFieldName, titleFieldName,
                                 authorFieldName}),
        getBatmanAmbiguosDocument(), VIDEOGAME_ANALYZED_RESULT);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (indexReader != nullptr) {
      delete indexReader;
    }
  }
}
} // namespace org::apache::lucene::classification::document