using namespace std;

#include "TestFieldMaskingSpanQuery.h"

namespace org::apache::lucene::search::spans
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using CheckHits = org::apache::lucene::search::CheckHits;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using QueryUtils = org::apache::lucene::search::QueryUtils;
using TFIDFSimilarity =
    org::apache::lucene::search::similarities::TFIDFSimilarity;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::AfterClass;
using org::junit::BeforeClass;
//    import static org.apache.lucene.search.spans.SpanTestUtil.assertFinished;
//    import static org.apache.lucene.search.spans.SpanTestUtil.assertNext;

shared_ptr<Document>
TestFieldMaskingSpanQuery::doc(std::deque<std::shared_ptr<Field>> &fields)
{
  shared_ptr<Document> doc = make_shared<Document>();
  for (int i = 0; i < fields.size(); i++) {
    doc->push_back(fields[i]);
  }
  return doc;
}

shared_ptr<Field> TestFieldMaskingSpanQuery::field(const wstring &name,
                                                   const wstring &value)
{
  return newTextField(name, value, Field::Store::NO);
}

shared_ptr<org::apache::lucene::search::IndexSearcher>
    TestFieldMaskingSpanQuery::searcher;
shared_ptr<org::apache::lucene::store::Directory>
    TestFieldMaskingSpanQuery::directory;
shared_ptr<org::apache::lucene::index::IndexReader>
    TestFieldMaskingSpanQuery::reader;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestFieldMaskingSpanQuery::beforeClass() 
{
  directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), directory,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergePolicy(newLogMergePolicy()));

  writer->addDocument(doc(std::deque<std::shared_ptr<Field>>{
      field(L"id", L"0"), field(L"gender", L"male"), field(L"first", L"james"),
      field(L"last", L"jones")}));

  writer->addDocument(doc(std::deque<std::shared_ptr<Field>>{
      field(L"id", L"1"), field(L"gender", L"male"), field(L"first", L"james"),
      field(L"last", L"smith"), field(L"gender", L"female"),
      field(L"first", L"sally"), field(L"last", L"jones")}));

  writer->addDocument(doc(std::deque<std::shared_ptr<Field>>{
      field(L"id", L"2"), field(L"gender", L"female"),
      field(L"first", L"greta"), field(L"last", L"jones"),
      field(L"gender", L"female"), field(L"first", L"sally"),
      field(L"last", L"smith"), field(L"gender", L"male"),
      field(L"first", L"james"), field(L"last", L"jones")}));

  writer->addDocument(doc(std::deque<std::shared_ptr<Field>>{
      field(L"id", L"3"), field(L"gender", L"female"), field(L"first", L"lisa"),
      field(L"last", L"jones"), field(L"gender", L"male"),
      field(L"first", L"bob"), field(L"last", L"costas")}));

  writer->addDocument(doc(std::deque<std::shared_ptr<Field>>{
      field(L"id", L"4"), field(L"gender", L"female"),
      field(L"first", L"sally"), field(L"last", L"smith"),
      field(L"gender", L"female"), field(L"first", L"linda"),
      field(L"last", L"dixit"), field(L"gender", L"male"),
      field(L"first", L"bubba"), field(L"last", L"jones")}));
  writer->forceMerge(1);
  reader = writer->getReader();
  delete writer;
  searcher = make_shared<IndexSearcher>(getOnlyLeafReader(reader));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
void TestFieldMaskingSpanQuery::afterClass() 
{
  searcher.reset();
  delete reader;
  reader.reset();
  delete directory;
  directory.reset();
}

void TestFieldMaskingSpanQuery::check(
    shared_ptr<SpanQuery> q, std::deque<int> &docs) 
{
  CheckHits::checkHitCollector(random(), q, L"", searcher, docs);
}

void TestFieldMaskingSpanQuery::testRewrite0() 
{
  shared_ptr<SpanQuery> q = make_shared<FieldMaskingSpanQuery>(
      make_shared<SpanTermQuery>(make_shared<Term>(L"last", L"sally")),
      L"first");
  shared_ptr<SpanQuery> qr =
      std::static_pointer_cast<SpanQuery>(searcher->rewrite(q));

  QueryUtils::checkEqual(q, qr);

  shared_ptr<Set<std::shared_ptr<Term>>> terms =
      unordered_set<std::shared_ptr<Term>>();
  qr->createWeight(searcher, false, 1.0f)->extractTerms(terms);
  assertEquals(1, terms->size());
}

void TestFieldMaskingSpanQuery::testRewrite1() 
{
  // mask an anon SpanQuery class that rewrites to something else.
  shared_ptr<SpanQuery> q = make_shared<FieldMaskingSpanQuery>(
      make_shared<SpanTermQueryAnonymousInnerClass>(
          shared_from_this(), make_shared<Term>(L"last", L"sally")),
      L"first");

  shared_ptr<SpanQuery> qr =
      std::static_pointer_cast<SpanQuery>(searcher->rewrite(q));

  QueryUtils::checkUnequal(q, qr);

  shared_ptr<Set<std::shared_ptr<Term>>> terms =
      unordered_set<std::shared_ptr<Term>>();
  qr->createWeight(searcher, false, 1.0f)->extractTerms(terms);
  assertEquals(2, terms->size());
}

TestFieldMaskingSpanQuery::SpanTermQueryAnonymousInnerClass::
    SpanTermQueryAnonymousInnerClass(
        shared_ptr<TestFieldMaskingSpanQuery> outerInstance,
        shared_ptr<Term> org)
    : SpanTermQuery(Term)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Query>
TestFieldMaskingSpanQuery::SpanTermQueryAnonymousInnerClass::rewrite(
    shared_ptr<IndexReader> reader)
{
  return make_shared<SpanOrQuery>(
      make_shared<SpanTermQuery>(make_shared<Term>(L"first", L"sally")),
      make_shared<SpanTermQuery>(make_shared<Term>(L"first", L"james")));
}

void TestFieldMaskingSpanQuery::testRewrite2() 
{
  shared_ptr<SpanQuery> q1 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"last", L"smith"));
  shared_ptr<SpanQuery> q2 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"last", L"jones"));
  shared_ptr<SpanQuery> q = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{
          q1, make_shared<FieldMaskingSpanQuery>(q2, L"last")},
      1, true);
  shared_ptr<Query> qr = searcher->rewrite(q);

  QueryUtils::checkEqual(q, qr);

  unordered_set<std::shared_ptr<Term>> set =
      unordered_set<std::shared_ptr<Term>>();
  qr->createWeight(searcher, true, 1.0f)->extractTerms(set);
  assertEquals(2, set.size());
}

void TestFieldMaskingSpanQuery::testEquality1()
{
  shared_ptr<SpanQuery> q1 = make_shared<FieldMaskingSpanQuery>(
      make_shared<SpanTermQuery>(make_shared<Term>(L"last", L"sally")),
      L"first");
  shared_ptr<SpanQuery> q2 = make_shared<FieldMaskingSpanQuery>(
      make_shared<SpanTermQuery>(make_shared<Term>(L"last", L"sally")),
      L"first");
  shared_ptr<SpanQuery> q3 = make_shared<FieldMaskingSpanQuery>(
      make_shared<SpanTermQuery>(make_shared<Term>(L"last", L"sally")),
      L"XXXXX");
  shared_ptr<SpanQuery> q4 = make_shared<FieldMaskingSpanQuery>(
      make_shared<SpanTermQuery>(make_shared<Term>(L"last", L"XXXXX")),
      L"first");
  shared_ptr<SpanQuery> q5 = make_shared<FieldMaskingSpanQuery>(
      make_shared<SpanTermQuery>(make_shared<Term>(L"xXXX", L"sally")),
      L"first");
  QueryUtils::checkEqual(q1, q2);
  QueryUtils::checkUnequal(q1, q3);
  QueryUtils::checkUnequal(q1, q4);
  QueryUtils::checkUnequal(q1, q5);
}

void TestFieldMaskingSpanQuery::testNoop0() 
{
  shared_ptr<SpanQuery> q1 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"last", L"sally"));
  shared_ptr<SpanQuery> q = make_shared<FieldMaskingSpanQuery>(q1, L"first");
  check(q, std::deque<int>());
}

void TestFieldMaskingSpanQuery::testNoop1() 
{
  shared_ptr<SpanQuery> q1 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"last", L"smith"));
  shared_ptr<SpanQuery> q2 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"last", L"jones"));
  shared_ptr<SpanQuery> q = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{
          q1, make_shared<FieldMaskingSpanQuery>(q2, L"last")},
      0, true);
  check(q, std::deque<int>{1, 2});
  q = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{
          make_shared<FieldMaskingSpanQuery>(q1, L"last"),
          make_shared<FieldMaskingSpanQuery>(q2, L"last")},
      0, true);
  check(q, std::deque<int>{1, 2});
}

void TestFieldMaskingSpanQuery::testSimple1() 
{
  shared_ptr<SpanQuery> q1 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"first", L"james"));
  shared_ptr<SpanQuery> q2 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"last", L"jones"));
  shared_ptr<SpanQuery> q = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{
          q1, make_shared<FieldMaskingSpanQuery>(q2, L"first")},
      -1, false);
  check(q, std::deque<int>{0, 2});
  q = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{
          make_shared<FieldMaskingSpanQuery>(q2, L"first"), q1},
      -1, false);
  check(q, std::deque<int>{0, 2});
  q = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{
          q2, make_shared<FieldMaskingSpanQuery>(q1, L"last")},
      -1, false);
  check(q, std::deque<int>{0, 2});
  q = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{
          make_shared<FieldMaskingSpanQuery>(q1, L"last"), q2},
      -1, false);
  check(q, std::deque<int>{0, 2});
}

void TestFieldMaskingSpanQuery::testSimple2() 
{
  assumeTrue(L"Broken scoring: LUCENE-3723",
             std::dynamic_pointer_cast<TFIDFSimilarity>(
                 searcher->getSimilarity(true)) != nullptr);
  shared_ptr<SpanQuery> q1 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"gender", L"female"));
  shared_ptr<SpanQuery> q2 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"last", L"smith"));
  shared_ptr<SpanQuery> q = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{
          q1, make_shared<FieldMaskingSpanQuery>(q2, L"gender")},
      -1, false);
  check(q, std::deque<int>{2, 4});
  q = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{
          make_shared<FieldMaskingSpanQuery>(q1, L"id"),
          make_shared<FieldMaskingSpanQuery>(q2, L"id")},
      -1, false);
  check(q, std::deque<int>{2, 4});
}

void TestFieldMaskingSpanQuery::testSpans0() 
{
  shared_ptr<SpanQuery> q1 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"gender", L"female"));
  shared_ptr<SpanQuery> q2 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"first", L"james"));
  shared_ptr<SpanQuery> q = make_shared<SpanOrQuery>(
      q1, make_shared<FieldMaskingSpanQuery>(q2, L"gender"));
  check(q, std::deque<int>{0, 1, 2, 3, 4});

  shared_ptr<Spans> span =
      q->createWeight(searcher, false, 1.0f)
          ->getSpans(searcher->getIndexReader()->leaves()[0],
                     SpanWeight::Postings::POSITIONS);
  assertNext(span, 0, 0, 1);
  assertNext(span, 1, 0, 1);
  assertNext(span, 1, 1, 2);
  assertNext(span, 2, 0, 1);
  assertNext(span, 2, 1, 2);
  assertNext(span, 2, 2, 3);
  assertNext(span, 3, 0, 1);
  assertNext(span, 4, 0, 1);
  assertNext(span, 4, 1, 2);
  assertFinished(span);
}

void TestFieldMaskingSpanQuery::testSpans1() 
{
  shared_ptr<SpanQuery> q1 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"first", L"sally"));
  shared_ptr<SpanQuery> q2 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"first", L"james"));
  shared_ptr<SpanQuery> qA = make_shared<SpanOrQuery>(q1, q2);
  shared_ptr<SpanQuery> qB = make_shared<FieldMaskingSpanQuery>(qA, L"id");

  check(qA, std::deque<int>{0, 1, 2, 4});
  check(qB, std::deque<int>{0, 1, 2, 4});

  shared_ptr<Spans> spanA =
      qA->createWeight(searcher, false, 1.0f)
          ->getSpans(searcher->getIndexReader()->leaves()[0],
                     SpanWeight::Postings::POSITIONS);
  shared_ptr<Spans> spanB =
      qB->createWeight(searcher, false, 1.0f)
          ->getSpans(searcher->getIndexReader()->leaves()[0],
                     SpanWeight::Postings::POSITIONS);

  while (spanA->nextDoc() != Spans::NO_MORE_DOCS) {
    assertNotSame(L"spanB not still going", Spans::NO_MORE_DOCS,
                  spanB->nextDoc());
    while (spanA->nextStartPosition() != Spans::NO_MORE_POSITIONS) {
      assertEquals(L"spanB start position", spanA->startPosition(),
                   spanB->nextStartPosition());
      assertEquals(L"spanB end position", spanA->endPosition(),
                   spanB->endPosition());
    }
    assertEquals(L"spanB start position", Spans::NO_MORE_POSITIONS,
                 spanB->nextStartPosition());
  }
  assertEquals(L"spanB end doc", Spans::NO_MORE_DOCS, spanB->nextDoc());
}

void TestFieldMaskingSpanQuery::testSpans2() 
{
  assumeTrue(L"Broken scoring: LUCENE-3723",
             std::dynamic_pointer_cast<TFIDFSimilarity>(
                 searcher->getSimilarity(true)) != nullptr);
  shared_ptr<SpanQuery> qA1 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"gender", L"female"));
  shared_ptr<SpanQuery> qA2 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"first", L"james"));
  shared_ptr<SpanQuery> qA = make_shared<SpanOrQuery>(
      qA1, make_shared<FieldMaskingSpanQuery>(qA2, L"gender"));
  shared_ptr<SpanQuery> qB =
      make_shared<SpanTermQuery>(make_shared<Term>(L"last", L"jones"));
  shared_ptr<SpanQuery> q = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{
          make_shared<FieldMaskingSpanQuery>(qA, L"id"),
          make_shared<FieldMaskingSpanQuery>(qB, L"id")},
      -1, false);
  check(q, std::deque<int>{0, 1, 2, 3});

  shared_ptr<Spans> span =
      q->createWeight(searcher, false, 1.0f)
          ->getSpans(searcher->getIndexReader()->leaves()[0],
                     SpanWeight::Postings::POSITIONS);
  assertNext(span, 0, 0, 1);
  assertNext(span, 1, 1, 2);
  assertNext(span, 2, 0, 1);
  assertNext(span, 2, 2, 3);
  assertNext(span, 3, 0, 1);
  assertFinished(span);
}

wstring TestFieldMaskingSpanQuery::s(int doc, int start, int end)
{
  return L"s(" + to_wstring(doc) + L"," + to_wstring(start) + L"," +
         to_wstring(end) + L")";
}
} // namespace org::apache::lucene::search::spans