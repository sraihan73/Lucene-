using namespace std;

#include "TestSpanCollection.h"

namespace org::apache::lucene::search::spans
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using IndexReader = org::apache::lucene::index::IndexReader;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;
const wstring TestSpanCollection::FIELD = L"field";
shared_ptr<org::apache::lucene::document::FieldType>
    TestSpanCollection::OFFSETS =
        make_shared<org::apache::lucene::document::FieldType>(
            org::apache::lucene::document::TextField::TYPE_STORED);

TestSpanCollection::StaticConstructor::StaticConstructor()
{
  OFFSETS->setIndexOptions(
      IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS);
}

TestSpanCollection::StaticConstructor TestSpanCollection::staticConstructor;

void TestSpanCollection::tearDown() 
{
  delete reader;
  delete directory;
  LuceneTestCase::tearDown();
}

void TestSpanCollection::setUp() 
{
  LuceneTestCase::setUp();
  directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), directory,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergePolicy(newLogMergePolicy()));
  for (int i = 0; i < docFields.size(); i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newField(FIELD, docFields[i], OFFSETS));
    writer->addDocument(doc);
  }
  writer->forceMerge(1);
  reader = writer->getReader();
  delete writer;
  searcher = newSearcher(getOnlyLeafReader(reader));
}

void TestSpanCollection::TermCollector::collectLeaf(
    shared_ptr<PostingsEnum> postings, int position,
    shared_ptr<Term> term) 
{
  terms->add(term);
}

void TestSpanCollection::TermCollector::reset() { terms->clear(); }

void TestSpanCollection::checkCollectedTerms(
    shared_ptr<Spans> spans, shared_ptr<TermCollector> collector,
    deque<Term> &expectedTerms) 
{
  collector->reset();
  spans->collect(collector);
  for (shared_ptr<Term> t : expectedTerms) {
    assertTrue(L"Missing term " + t, collector->terms->contains(t));
  }
  assertEquals(L"Unexpected terms found", expectedTerms->length,
               collector->terms->size());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testNestedNearQuery() throws
// java.io.IOException
void TestSpanCollection::testNestedNearQuery() 
{

  // near(w1, near(w2, or(w3, w4)))

  shared_ptr<SpanTermQuery> q1 =
      make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"w1"));
  shared_ptr<SpanTermQuery> q2 =
      make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"w2"));
  shared_ptr<SpanTermQuery> q3 =
      make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"w3"));
  shared_ptr<SpanTermQuery> q4 =
      make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"w4"));

  shared_ptr<SpanOrQuery> q5 = make_shared<SpanOrQuery>(q4, q3);
  shared_ptr<SpanNearQuery> q6 = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{q2, q5}, 1, true);
  shared_ptr<SpanNearQuery> q7 = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{q1, q6}, 1, true);

  shared_ptr<TermCollector> collector = make_shared<TermCollector>();
  shared_ptr<Spans> spans =
      q7->createWeight(searcher, false, 1.0f)
          ->getSpans(searcher->getIndexReader()->leaves()[0],
                     SpanWeight::Postings::POSITIONS);
  assertEquals(0, spans->advance(0));
  spans->nextStartPosition();
  checkCollectedTerms(spans, collector,
                      {make_shared<Term>(FIELD, L"w1"),
                       make_shared<Term>(FIELD, L"w2"),
                       make_shared<Term>(FIELD, L"w3")});

  assertEquals(3, spans->advance(3));
  spans->nextStartPosition();
  checkCollectedTerms(spans, collector,
                      {make_shared<Term>(FIELD, L"w1"),
                       make_shared<Term>(FIELD, L"w2"),
                       make_shared<Term>(FIELD, L"w4")});
  spans->nextStartPosition();
  checkCollectedTerms(spans, collector,
                      {make_shared<Term>(FIELD, L"w1"),
                       make_shared<Term>(FIELD, L"w2"),
                       make_shared<Term>(FIELD, L"w3")});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testOrQuery() throws java.io.IOException
void TestSpanCollection::testOrQuery() 
{
  shared_ptr<SpanTermQuery> q2 =
      make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"w2"));
  shared_ptr<SpanTermQuery> q3 =
      make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"w3"));
  shared_ptr<SpanOrQuery> orQuery = make_shared<SpanOrQuery>(q2, q3);

  shared_ptr<TermCollector> collector = make_shared<TermCollector>();
  shared_ptr<Spans> spans =
      orQuery->createWeight(searcher, false, 1.0f)
          ->getSpans(searcher->getIndexReader()->leaves()[0],
                     SpanWeight::Postings::POSITIONS);

  assertEquals(1, spans->advance(1));
  spans->nextStartPosition();
  checkCollectedTerms(spans, collector, {make_shared<Term>(FIELD, L"w3")});
  spans->nextStartPosition();
  checkCollectedTerms(spans, collector, {make_shared<Term>(FIELD, L"w2")});
  spans->nextStartPosition();
  checkCollectedTerms(spans, collector, {make_shared<Term>(FIELD, L"w3")});

  assertEquals(3, spans->advance(3));
  spans->nextStartPosition();
  checkCollectedTerms(spans, collector, {make_shared<Term>(FIELD, L"w2")});
  spans->nextStartPosition();
  checkCollectedTerms(spans, collector, {make_shared<Term>(FIELD, L"w2")});
  spans->nextStartPosition();
  checkCollectedTerms(spans, collector, {make_shared<Term>(FIELD, L"w3")});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSpanNotQuery() throws
// java.io.IOException
void TestSpanCollection::testSpanNotQuery() 
{

  shared_ptr<SpanTermQuery> q1 =
      make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"w1"));
  shared_ptr<SpanTermQuery> q2 =
      make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"w2"));
  shared_ptr<SpanTermQuery> q3 =
      make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"w3"));

  shared_ptr<SpanNearQuery> nq = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{q1, q2}, 2, true);
  shared_ptr<SpanNotQuery> notq = make_shared<SpanNotQuery>(nq, q3);

  shared_ptr<TermCollector> collector = make_shared<TermCollector>();
  shared_ptr<Spans> spans =
      notq->createWeight(searcher, false, 1.0f)
          ->getSpans(searcher->getIndexReader()->leaves()[0],
                     SpanWeight::Postings::POSITIONS);

  assertEquals(2, spans->advance(2));
  spans->nextStartPosition();
  checkCollectedTerms(
      spans, collector,
      {make_shared<Term>(FIELD, L"w1"), make_shared<Term>(FIELD, L"w2")});
}
} // namespace org::apache::lucene::search::spans