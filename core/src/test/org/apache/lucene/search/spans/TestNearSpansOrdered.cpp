using namespace std;

#include "TestNearSpansOrdered.h"

namespace org::apache::lucene::search::spans
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using CheckHits = org::apache::lucene::search::CheckHits;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Scorer = org::apache::lucene::search::Scorer;
using TopDocs = org::apache::lucene::search::TopDocs;
using Weight = org::apache::lucene::search::Weight;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
//    import static org.apache.lucene.search.spans.SpanTestUtil.assertFinished;
//    import static org.apache.lucene.search.spans.SpanTestUtil.assertNext;
const wstring TestNearSpansOrdered::FIELD = L"field";

void TestNearSpansOrdered::tearDown() 
{
  delete reader;
  delete directory;
  LuceneTestCase::tearDown();
}

void TestNearSpansOrdered::setUp() 
{
  LuceneTestCase::setUp();
  directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), directory,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergePolicy(newLogMergePolicy()));
  for (int i = 0; i < docFields.size(); i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newTextField(FIELD, docFields[i], Field::Store::NO));
    writer->addDocument(doc);
  }
  writer->forceMerge(1);
  reader = writer->getReader();
  delete writer;
  searcher = newSearcher(getOnlyLeafReader(reader));
}

shared_ptr<SpanNearQuery>
TestNearSpansOrdered::makeQuery(const wstring &s1, const wstring &s2,
                                const wstring &s3, int slop, bool inOrder)
{
  return make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{
          make_shared<SpanTermQuery>(make_shared<Term>(FIELD, s1)),
          make_shared<SpanTermQuery>(make_shared<Term>(FIELD, s2)),
          make_shared<SpanTermQuery>(make_shared<Term>(FIELD, s3))},
      slop, inOrder);
}

shared_ptr<SpanNearQuery> TestNearSpansOrdered::makeQuery()
{
  return makeQuery(L"w1", L"w2", L"w3", 1, true);
}

shared_ptr<SpanNearQuery>
TestNearSpansOrdered::makeOverlappedQuery(const wstring &sqt1,
                                          const wstring &sqt2, bool sqOrdered,
                                          const wstring &t3, bool ordered)
{
  return make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{
          make_shared<SpanNearQuery>(
              std::deque<std::shared_ptr<SpanQuery>>{
                  make_shared<SpanTermQuery>(make_shared<Term>(FIELD, sqt1)),
                  make_shared<SpanTermQuery>(make_shared<Term>(FIELD, sqt2))},
              1, sqOrdered),
          make_shared<SpanTermQuery>(make_shared<Term>(FIELD, t3))},
      0, ordered);
}

void TestNearSpansOrdered::testSpanNearQuery() 
{
  shared_ptr<SpanNearQuery> q = makeQuery();
  CheckHits::checkHits(random(), q, FIELD, searcher, std::deque<int>{0, 1});
}

wstring TestNearSpansOrdered::s(shared_ptr<Spans> span)
{
  return s(span->docID(), span->startPosition(), span->endPosition());
}

wstring TestNearSpansOrdered::s(int doc, int start, int end)
{
  return L"s(" + to_wstring(doc) + L"," + to_wstring(start) + L"," +
         to_wstring(end) + L")";
}

void TestNearSpansOrdered::testNearSpansNext() 
{
  shared_ptr<SpanNearQuery> q = makeQuery();
  shared_ptr<Spans> span =
      q->createWeight(searcher, false, 1.0f)
          ->getSpans(searcher->getIndexReader()->leaves()[0],
                     SpanWeight::Postings::POSITIONS);
  assertNext(span, 0, 0, 3);
  assertNext(span, 1, 0, 4);
  assertFinished(span);
}

void TestNearSpansOrdered::testNearSpansAdvanceLikeNext() 
{
  shared_ptr<SpanNearQuery> q = makeQuery();
  shared_ptr<Spans> span =
      q->createWeight(searcher, false, 1.0f)
          ->getSpans(searcher->getIndexReader()->leaves()[0],
                     SpanWeight::Postings::POSITIONS);
  assertEquals(0, span->advance(0));
  assertEquals(0, span->nextStartPosition());
  assertEquals(s(0, 0, 3), s(span));
  assertEquals(1, span->advance(1));
  assertEquals(0, span->nextStartPosition());
  assertEquals(s(1, 0, 4), s(span));
  assertEquals(Spans::NO_MORE_DOCS, span->advance(2));
}

void TestNearSpansOrdered::testNearSpansNextThenAdvance() 
{
  shared_ptr<SpanNearQuery> q = makeQuery();
  shared_ptr<Spans> span =
      q->createWeight(searcher, false, 1.0f)
          ->getSpans(searcher->getIndexReader()->leaves()[0],
                     SpanWeight::Postings::POSITIONS);
  assertNotSame(Spans::NO_MORE_DOCS, span->nextDoc());
  assertEquals(0, span->nextStartPosition());
  assertEquals(s(0, 0, 3), s(span));
  assertNotSame(Spans::NO_MORE_DOCS, span->advance(1));
  assertEquals(0, span->nextStartPosition());
  assertEquals(s(1, 0, 4), s(span));
  assertEquals(Spans::NO_MORE_DOCS, span->nextDoc());
}

void TestNearSpansOrdered::testNearSpansNextThenAdvancePast() throw(
    runtime_error)
{
  shared_ptr<SpanNearQuery> q = makeQuery();
  shared_ptr<Spans> span =
      q->createWeight(searcher, false, 1.0f)
          ->getSpans(searcher->getIndexReader()->leaves()[0],
                     SpanWeight::Postings::POSITIONS);
  assertNotSame(Spans::NO_MORE_DOCS, span->nextDoc());
  assertEquals(0, span->nextStartPosition());
  assertEquals(s(0, 0, 3), s(span));
  assertEquals(Spans::NO_MORE_DOCS, span->advance(2));
}

void TestNearSpansOrdered::testNearSpansAdvancePast() 
{
  shared_ptr<SpanNearQuery> q = makeQuery();
  shared_ptr<Spans> span =
      q->createWeight(searcher, false, 1.0f)
          ->getSpans(searcher->getIndexReader()->leaves()[0],
                     SpanWeight::Postings::POSITIONS);
  assertEquals(Spans::NO_MORE_DOCS, span->advance(2));
}

void TestNearSpansOrdered::testNearSpansAdvanceTo0() 
{
  shared_ptr<SpanNearQuery> q = makeQuery();
  shared_ptr<Spans> span =
      q->createWeight(searcher, false, 1.0f)
          ->getSpans(searcher->getIndexReader()->leaves()[0],
                     SpanWeight::Postings::POSITIONS);
  assertEquals(0, span->advance(0));
  assertEquals(0, span->nextStartPosition());
  assertEquals(s(0, 0, 3), s(span));
}

void TestNearSpansOrdered::testNearSpansAdvanceTo1() 
{
  shared_ptr<SpanNearQuery> q = makeQuery();
  shared_ptr<Spans> span =
      q->createWeight(searcher, false, 1.0f)
          ->getSpans(searcher->getIndexReader()->leaves()[0],
                     SpanWeight::Postings::POSITIONS);
  assertEquals(1, span->advance(1));
  assertEquals(0, span->nextStartPosition());
  assertEquals(s(1, 0, 4), s(span));
}

void TestNearSpansOrdered::testSpanNearScorerSkipTo1() 
{
  shared_ptr<SpanNearQuery> q = makeQuery();
  shared_ptr<Weight> w = searcher->createWeight(searcher->rewrite(q), true, 1);
  shared_ptr<IndexReaderContext> topReaderContext =
      searcher->getTopReaderContext();
  shared_ptr<LeafReaderContext> leave = topReaderContext->leaves()[0];
  shared_ptr<Scorer> s = w->scorer(leave);
  assertEquals(1, s->begin().advance(1));
}

void TestNearSpansOrdered::testOverlappedOrderedSpan() 
{
  shared_ptr<SpanNearQuery> q =
      makeOverlappedQuery(L"w5", L"w3", false, L"w4", true);
  CheckHits::checkHits(random(), q, FIELD, searcher, std::deque<int>());
}

void TestNearSpansOrdered::testOverlappedNonOrderedSpan() 
{
  shared_ptr<SpanNearQuery> q =
      makeOverlappedQuery(L"w3", L"w5", true, L"w4", false);
  CheckHits::checkHits(random(), q, FIELD, searcher, std::deque<int>{0});
}

void TestNearSpansOrdered::testNonOverlappedOrderedSpan() 
{
  shared_ptr<SpanNearQuery> q =
      makeOverlappedQuery(L"w3", L"w4", true, L"w5", true);
  CheckHits::checkHits(random(), q, FIELD, searcher, std::deque<int>{0});
}

void TestNearSpansOrdered::testOrderedSpanIteration() 
{
  shared_ptr<SpanNearQuery> q = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{
          make_shared<SpanOrQuery>(
              make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"w1")),
              make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"w2"))),
          make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"w4"))},
      10, true);
  shared_ptr<Spans> spans =
      q->createWeight(searcher, false, 1.0f)
          ->getSpans(searcher->getIndexReader()->leaves()[0],
                     SpanWeight::Postings::POSITIONS);
  assertNext(spans, 0, 0, 4);
  assertNext(spans, 0, 1, 4);
  assertFinished(spans);
}

void TestNearSpansOrdered::testOrderedSpanIterationSameTerms1() throw(
    runtime_error)
{
  shared_ptr<SpanNearQuery> q = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{
          make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"t1")),
          make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"t2"))},
      1, true);
  shared_ptr<Spans> spans =
      q->createWeight(searcher, false, 1.0f)
          ->getSpans(searcher->getIndexReader()->leaves()[0],
                     SpanWeight::Postings::POSITIONS);
  assertNext(spans, 4, 0, 2);
  assertFinished(spans);
}

void TestNearSpansOrdered::testOrderedSpanIterationSameTerms2() throw(
    runtime_error)
{
  shared_ptr<SpanNearQuery> q = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{
          make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"t2")),
          make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"t1"))},
      1, true);
  shared_ptr<Spans> spans =
      q->createWeight(searcher, false, 1.0f)
          ->getSpans(searcher->getIndexReader()->leaves()[0],
                     SpanWeight::Postings::POSITIONS);
  assertNext(spans, 4, 1, 4);
  assertNext(spans, 4, 2, 4);
  assertFinished(spans);
}

void TestNearSpansOrdered::testSpanNearScorerExplain() 
{
  shared_ptr<SpanNearQuery> q = makeQuery();
  shared_ptr<Explanation> e = searcher->explain(q, 1);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(L"Scorer explanation value for doc#1 isn't positive: " +
                 e->toString(),
             0.0f <= e->getValue());
}

void TestNearSpansOrdered::testGaps() 
{
  shared_ptr<SpanNearQuery> q =
      SpanNearQuery::newOrderedNearQuery(FIELD)
          ->addClause(
              make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"w1")))
          ->addGap(1)
          ->addClause(
              make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"w2")))
          ->build();
  shared_ptr<Spans> spans =
      q->createWeight(searcher, false, 1.0f)
          ->getSpans(searcher->getIndexReader()->leaves()[0],
                     SpanWeight::Postings::POSITIONS);
  assertNext(spans, 1, 0, 3);
  assertNext(spans, 2, 0, 3);
  assertFinished(spans);

  q = SpanNearQuery::newOrderedNearQuery(FIELD)
          ->addClause(
              make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"w1")))
          ->addGap(1)
          ->addClause(
              make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"w2")))
          ->addGap(1)
          ->addClause(
              make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"w3")))
          ->setSlop(1)
          ->build();
  spans = q->createWeight(searcher, false, 1.0f)
              ->getSpans(searcher->getIndexReader()->leaves()[0],
                         SpanWeight::Postings::POSITIONS);
  assertNext(spans, 2, 0, 5);
  assertNext(spans, 3, 0, 6);
  assertFinished(spans);
}

void TestNearSpansOrdered::testMultipleGaps() 
{
  shared_ptr<SpanQuery> q =
      SpanNearQuery::newOrderedNearQuery(FIELD)
          ->addClause(
              make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"g")))
          ->addGap(2)
          ->addClause(
              make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"g")))
          ->build();
  shared_ptr<Spans> spans =
      q->createWeight(searcher, false, 1.0f)
          ->getSpans(searcher->getIndexReader()->leaves()[0],
                     SpanWeight::Postings::POSITIONS);
  assertNext(spans, 5, 0, 4);
  assertNext(spans, 5, 9, 13);
  assertFinished(spans);
}

void TestNearSpansOrdered::testNestedGaps() 
{
  shared_ptr<SpanQuery> q =
      SpanNearQuery::newOrderedNearQuery(FIELD)
          ->addClause(make_shared<SpanOrQuery>(
              make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"open")),
              SpanNearQuery::newOrderedNearQuery(FIELD)
                  ->addClause(make_shared<SpanTermQuery>(
                      make_shared<Term>(FIELD, L"go")))
                  ->addGap(1)
                  ->build()))
          ->addClause(
              make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"webpage")))
          ->build();

  shared_ptr<TopDocs> topDocs = searcher->search(q, 1);
  assertEquals(6, topDocs->scoreDocs[0]->doc);
}
} // namespace org::apache::lucene::search::spans