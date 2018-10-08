using namespace std;

#include "KNearestNeighborDocumentClassifierTest.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/Term.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/TermQuery.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../java/org/apache/lucene/classification/document/KNearestNeighborDocumentClassifier.h"

namespace org::apache::lucene::classification::document
{
using Document = org::apache::lucene::document::Document;
using Term = org::apache::lucene::index::Term;
using TermQuery = org::apache::lucene::search::TermQuery;
using BytesRef = org::apache::lucene::util::BytesRef;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testBasicDocumentClassification() throws
// Exception
void KNearestNeighborDocumentClassifierTest::
    testBasicDocumentClassification() 
{
  try {
    shared_ptr<Document> videoGameDocument = getVideoGameDocument();
    shared_ptr<Document> batmanDocument = getBatmanDocument();
    shared_ptr<KNearestNeighborDocumentClassifier> classifier =
        make_shared<KNearestNeighborDocumentClassifier>(
            indexReader, nullptr, nullptr, 1, 4, 1, categoryFieldName,
            field2analyzer,
            std::deque<wstring>{textFieldName, titleFieldName,
                                 authorFieldName});
    checkCorrectDocumentClassification(classifier, videoGameDocument,
                                       VIDEOGAME_RESULT);
    checkCorrectDocumentClassification(classifier, batmanDocument,
                                       BATMAN_RESULT);
    // considering only the text we have wrong classification because the text
    // was ambiguos on purpose
    checkCorrectDocumentClassification(
        make_shared<KNearestNeighborDocumentClassifier>(
            indexReader, nullptr, nullptr, 1, 1, 1, categoryFieldName,
            field2analyzer, std::deque<wstring>{textFieldName}),
        videoGameDocument, BATMAN_RESULT);
    checkCorrectDocumentClassification(
        make_shared<KNearestNeighborDocumentClassifier>(
            indexReader, nullptr, nullptr, 1, 1, 1, categoryFieldName,
            field2analyzer, std::deque<wstring>{textFieldName}),
        batmanDocument, VIDEOGAME_RESULT);

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
void KNearestNeighborDocumentClassifierTest::
    testBasicDocumentClassificationScore() 
{
  try {
    shared_ptr<Document> videoGameDocument = getVideoGameDocument();
    shared_ptr<Document> batmanDocument = getBatmanDocument();
    shared_ptr<KNearestNeighborDocumentClassifier> classifier =
        make_shared<KNearestNeighborDocumentClassifier>(
            indexReader, nullptr, nullptr, 1, 4, 1, categoryFieldName,
            field2analyzer,
            std::deque<wstring>{textFieldName, titleFieldName,
                                 authorFieldName});
    double score1 = checkCorrectDocumentClassification(
        classifier, videoGameDocument, VIDEOGAME_RESULT);
    assertEquals(1.0, score1, 0);
    double score2 = checkCorrectDocumentClassification(
        classifier, batmanDocument, BATMAN_RESULT);
    assertEquals(1.0, score2, 0);
    // considering only the text we have wrong classification because the text
    // was ambiguos on purpose
    double score3 = checkCorrectDocumentClassification(
        make_shared<KNearestNeighborDocumentClassifier>(
            indexReader, nullptr, nullptr, 1, 1, 1, categoryFieldName,
            field2analyzer, std::deque<wstring>{textFieldName}),
        videoGameDocument, BATMAN_RESULT);
    assertEquals(1.0, score3, 0);
    double score4 = checkCorrectDocumentClassification(
        make_shared<KNearestNeighborDocumentClassifier>(
            indexReader, nullptr, nullptr, 1, 1, 1, categoryFieldName,
            field2analyzer, std::deque<wstring>{textFieldName}),
        batmanDocument, VIDEOGAME_RESULT);
    assertEquals(1.0, score4, 0);
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
void KNearestNeighborDocumentClassifierTest::
    testBoostedDocumentClassification() 
{
  try {
    checkCorrectDocumentClassification(
        make_shared<KNearestNeighborDocumentClassifier>(
            indexReader, nullptr, nullptr, 1, 1, 1, categoryFieldName,
            field2analyzer,
            std::deque<wstring>{textFieldName, titleFieldName + L"^100",
                                 authorFieldName}),
        getBatmanAmbiguosDocument(), BATMAN_RESULT);
    // considering without boost wrong classification will appear
    checkCorrectDocumentClassification(
        make_shared<KNearestNeighborDocumentClassifier>(
            indexReader, nullptr, nullptr, 1, 1, 1, categoryFieldName,
            field2analyzer,
            std::deque<wstring>{textFieldName, titleFieldName,
                                 authorFieldName}),
        getBatmanAmbiguosDocument(), VIDEOGAME_RESULT);
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
// ORIGINAL LINE: @Test public void testBasicDocumentClassificationWithQuery()
// throws Exception
void KNearestNeighborDocumentClassifierTest::
    testBasicDocumentClassificationWithQuery() 
{
  try {
    shared_ptr<TermQuery> query =
        make_shared<TermQuery>(make_shared<Term>(authorFieldName, L"ign"));
    checkCorrectDocumentClassification(
        make_shared<KNearestNeighborDocumentClassifier>(
            indexReader, nullptr, query, 1, 1, 1, categoryFieldName,
            field2analyzer,
            std::deque<wstring>{textFieldName, titleFieldName,
                                 authorFieldName}),
        getVideoGameDocument(), VIDEOGAME_RESULT);
    checkCorrectDocumentClassification(
        make_shared<KNearestNeighborDocumentClassifier>(
            indexReader, nullptr, query, 1, 1, 1, categoryFieldName,
            field2analyzer,
            std::deque<wstring>{textFieldName, titleFieldName,
                                 authorFieldName}),
        getBatmanDocument(), VIDEOGAME_RESULT);
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