using namespace std;

#include "TestSpanContainQuery.h"

namespace org::apache::lucene::search::spans
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using CheckHits = org::apache::lucene::search::CheckHits;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using namespace org::apache::lucene::search::spans;
//    import static org.apache.lucene.search.spans.SpanTestUtil.*;
const wstring TestSpanContainQuery::field = L"field";

void TestSpanContainQuery::setUp() 
{
  LuceneTestCase::setUp();
  directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), directory,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergePolicy(newLogMergePolicy()));
  for (int i = 0; i < docFields.size(); i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newTextField(field, docFields[i], Field::Store::YES));
    writer->addDocument(doc);
  }
  writer->forceMerge(1);
  reader = writer->getReader();
  delete writer;
  searcher = newSearcher(getOnlyLeafReader(reader));
}

void TestSpanContainQuery::tearDown() 
{
  delete reader;
  delete directory;
  LuceneTestCase::tearDown();
}

void TestSpanContainQuery::checkHits(
    shared_ptr<Query> query, std::deque<int> &results) 
{
  CheckHits::checkHits(random(), query, field, searcher, results);
}

shared_ptr<Spans>
TestSpanContainQuery::makeSpans(shared_ptr<SpanQuery> sq) 
{
  return sq->createWeight(searcher, false, 1.0f)
      ->getSpans(searcher->getIndexReader()->leaves()[0],
                 SpanWeight::Postings::POSITIONS);
}

void TestSpanContainQuery::tstEqualSpans(
    const wstring &mes, shared_ptr<SpanQuery> expectedQ,
    shared_ptr<SpanQuery> actualQ) 
{
  shared_ptr<Spans> expected = makeSpans(expectedQ);
  shared_ptr<Spans> actual = makeSpans(actualQ);
  tstEqualSpans(mes, expected, actual);
}

void TestSpanContainQuery::tstEqualSpans(
    const wstring &mes, shared_ptr<Spans> expected,
    shared_ptr<Spans> actual) 
{
  while (expected->nextDoc() != Spans::NO_MORE_DOCS) {
    assertEquals(expected->docID(), actual->nextDoc());
    assertEquals(expected->docID(), actual->docID());
    while (expected->nextStartPosition() != Spans::NO_MORE_POSITIONS) {
      assertEquals(expected->startPosition(), actual->nextStartPosition());
      assertEquals(L"start", expected->startPosition(),
                   actual->startPosition());
      assertEquals(L"end", expected->endPosition(), actual->endPosition());
    }
  }
}

void TestSpanContainQuery::testSpanContainTerm() 
{
  shared_ptr<SpanQuery> stq = SpanTestUtil::spanTermQuery(field, L"w3");
  shared_ptr<SpanQuery> containingQ =
      SpanTestUtil::spanContainingQuery(stq, stq);
  shared_ptr<SpanQuery> containedQ = SpanTestUtil::spanWithinQuery(stq, stq);
  tstEqualSpans(L"containing", stq, containingQ);
  tstEqualSpans(L"containing", stq, containedQ);
}

void TestSpanContainQuery::testSpanContainPhraseBothWords() 
{
  wstring w2 = L"w2";
  wstring w3 = L"w3";
  shared_ptr<SpanQuery> phraseQ = spanNearOrderedQuery(field, 0, w2, w3);
  shared_ptr<SpanQuery> w23 = spanOrQuery(field, w2, w3);
  shared_ptr<SpanQuery> containingPhraseOr =
      SpanTestUtil::spanContainingQuery(phraseQ, w23);
  shared_ptr<SpanQuery> containedPhraseOr =
      SpanTestUtil::spanWithinQuery(phraseQ, w23);
  tstEqualSpans(L"containing phrase or", phraseQ, containingPhraseOr);
  shared_ptr<Spans> spans = makeSpans(containedPhraseOr);
  SpanTestUtil::assertNext(spans, 0, 1, 2);
  SpanTestUtil::assertNext(spans, 0, 2, 3);
  SpanTestUtil::assertNext(spans, 1, 2, 3);
  SpanTestUtil::assertNext(spans, 1, 3, 4);
  SpanTestUtil::assertFinished(spans);
}

void TestSpanContainQuery::testSpanContainPhraseFirstWord() 
{
  wstring w2 = L"w2";
  wstring w3 = L"w3";
  shared_ptr<SpanQuery> stqw2 = SpanTestUtil::spanTermQuery(field, w2);
  shared_ptr<SpanQuery> phraseQ = spanNearOrderedQuery(field, 0, w2, w3);
  shared_ptr<SpanQuery> containingPhraseW2 =
      SpanTestUtil::spanContainingQuery(phraseQ, stqw2);
  shared_ptr<SpanQuery> containedPhraseW2 =
      SpanTestUtil::spanWithinQuery(phraseQ, stqw2);
  tstEqualSpans(L"containing phrase w2", phraseQ, containingPhraseW2);
  shared_ptr<Spans> spans = makeSpans(containedPhraseW2);
  SpanTestUtil::assertNext(spans, 0, 1, 2);
  SpanTestUtil::assertNext(spans, 1, 2, 3);
  SpanTestUtil::assertFinished(spans);
}

void TestSpanContainQuery::testSpanContainPhraseSecondWord() throw(
    runtime_error)
{
  wstring w2 = L"w2";
  wstring w3 = L"w3";
  shared_ptr<SpanQuery> stqw3 = SpanTestUtil::spanTermQuery(field, w3);
  shared_ptr<SpanQuery> phraseQ = spanNearOrderedQuery(field, 0, w2, w3);
  shared_ptr<SpanQuery> containingPhraseW3 =
      SpanTestUtil::spanContainingQuery(phraseQ, stqw3);
  shared_ptr<SpanQuery> containedPhraseW3 =
      SpanTestUtil::spanWithinQuery(phraseQ, stqw3);
  tstEqualSpans(L"containing phrase w3", phraseQ, containingPhraseW3);
  shared_ptr<Spans> spans = makeSpans(containedPhraseW3);
  SpanTestUtil::assertNext(spans, 0, 2, 3);
  SpanTestUtil::assertNext(spans, 1, 3, 4);
  SpanTestUtil::assertFinished(spans);
}
} // namespace org::apache::lucene::search::spans