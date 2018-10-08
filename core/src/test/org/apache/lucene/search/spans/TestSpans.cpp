using namespace std;

#include "TestSpans.h"

namespace org::apache::lucene::search::spans
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using ReaderUtil = org::apache::lucene::index::ReaderUtil;
using Term = org::apache::lucene::index::Term;
using CheckHits = org::apache::lucene::search::CheckHits;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using FuzzyQuery = org::apache::lucene::search::FuzzyQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using PrefixQuery = org::apache::lucene::search::PrefixQuery;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using TermQuery = org::apache::lucene::search::TermQuery;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
//    import static org.apache.lucene.search.spans.SpanTestUtil.assertFinished;
//    import static org.apache.lucene.search.spans.SpanTestUtil.assertNext;
//    import static
//    org.apache.lucene.search.spans.SpanTestUtil.spanNearOrderedQuery; import
//    static org.apache.lucene.search.spans.SpanTestUtil.spanNearUnorderedQuery;
//    import static org.apache.lucene.search.spans.SpanTestUtil.spanNotQuery;
//    import static org.apache.lucene.search.spans.SpanTestUtil.spanOrQuery;
//    import static org.apache.lucene.search.spans.SpanTestUtil.spanTermQuery;
const wstring TestSpans::field = L"field";

void TestSpans::setUp() 
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

void TestSpans::tearDown() 
{
  delete reader;
  delete directory;
  LuceneTestCase::tearDown();
}

void TestSpans::checkHits(shared_ptr<Query> query,
                          std::deque<int> &results) 
{
  CheckHits::checkHits(random(), query, field, searcher, results);
}

void TestSpans::orderedSlopTest3SQ(
    shared_ptr<SpanQuery> q1, shared_ptr<SpanQuery> q2,
    shared_ptr<SpanQuery> q3, int slop,
    std::deque<int> &expectedDocs) 
{
  shared_ptr<SpanQuery> query = spanNearOrderedQuery(slop, q1, q2, q3);
  checkHits(query, expectedDocs);
}

void TestSpans::orderedSlopTest3(
    int slop, std::deque<int> &expectedDocs) 
{
  orderedSlopTest3SQ(spanTermQuery(field, L"w1"), spanTermQuery(field, L"w2"),
                     spanTermQuery(field, L"w3"), slop, expectedDocs);
}

void TestSpans::orderedSlopTest3Equal(
    int slop, std::deque<int> &expectedDocs) 
{
  orderedSlopTest3SQ(spanTermQuery(field, L"w1"), spanTermQuery(field, L"w3"),
                     spanTermQuery(field, L"w3"), slop, expectedDocs);
}

void TestSpans::orderedSlopTest1Equal(
    int slop, std::deque<int> &expectedDocs) 
{
  orderedSlopTest3SQ(spanTermQuery(field, L"u2"), spanTermQuery(field, L"u2"),
                     spanTermQuery(field, L"u1"), slop, expectedDocs);
}

void TestSpans::testSpanNearOrdered01() 
{
  orderedSlopTest3(0, std::deque<int>{0});
}

void TestSpans::testSpanNearOrdered02() 
{
  orderedSlopTest3(1, std::deque<int>{0, 1});
}

void TestSpans::testSpanNearOrdered03() 
{
  orderedSlopTest3(2, std::deque<int>{0, 1, 2});
}

void TestSpans::testSpanNearOrdered04() 
{
  orderedSlopTest3(3, std::deque<int>{0, 1, 2, 3});
}

void TestSpans::testSpanNearOrdered05() 
{
  orderedSlopTest3(4, std::deque<int>{0, 1, 2, 3});
}

void TestSpans::testSpanNearOrderedEqual01() 
{
  orderedSlopTest3Equal(0, std::deque<int>());
}

void TestSpans::testSpanNearOrderedEqual02() 
{
  orderedSlopTest3Equal(1, std::deque<int>{1});
}

void TestSpans::testSpanNearOrderedEqual03() 
{
  orderedSlopTest3Equal(2, std::deque<int>{1});
}

void TestSpans::testSpanNearOrderedEqual04() 
{
  orderedSlopTest3Equal(3, std::deque<int>{1, 3});
}

void TestSpans::testSpanNearOrderedEqual11() 
{
  orderedSlopTest1Equal(0, std::deque<int>{4});
}

void TestSpans::testSpanNearOrderedEqual12() 
{
  orderedSlopTest1Equal(0, std::deque<int>{4});
}

void TestSpans::testSpanNearOrderedEqual13() 
{
  orderedSlopTest1Equal(1, std::deque<int>{4, 5, 6});
}

void TestSpans::testSpanNearOrderedEqual14() 
{
  orderedSlopTest1Equal(2, std::deque<int>{4, 5, 6, 7});
}

void TestSpans::testSpanNearOrderedEqual15() 
{
  orderedSlopTest1Equal(3, std::deque<int>{4, 5, 6, 7});
}

void TestSpans::testSpanNearOrderedOverlap() 
{
  shared_ptr<SpanQuery> *const query =
      spanNearOrderedQuery(field, 1, L"t1", L"t2", L"t3");

  shared_ptr<Spans> spans =
      query->createWeight(searcher, false, 1.0f)
          ->getSpans(searcher->getIndexReader()->leaves()[0],
                     SpanWeight::Postings::POSITIONS);

  assertEquals(L"first doc", 11, spans->nextDoc());
  assertEquals(L"first start", 0, spans->nextStartPosition());
  assertEquals(L"first end", 4, spans->endPosition());

  assertEquals(L"second start", 2, spans->nextStartPosition());
  assertEquals(L"second end", 6, spans->endPosition());

  assertFinished(spans);
}

void TestSpans::testSpanNearUnOrdered() 
{
  // See http://www.gossamer-threads.com/lists/lucene/java-dev/52270 for
  // discussion about this test
  shared_ptr<SpanQuery> senq = spanNearUnorderedQuery(field, 0, L"u1", L"u2");
  shared_ptr<Spans> spans =
      senq->createWeight(searcher, false, 1.0f)
          ->getSpans(searcher->getIndexReader()->leaves()[0],
                     SpanWeight::Postings::POSITIONS);
  assertNext(spans, 4, 1, 3);
  assertNext(spans, 5, 2, 4);
  assertNext(spans, 8, 2, 4);
  assertNext(spans, 9, 0, 2);
  assertNext(spans, 10, 0, 2);
  assertFinished(spans);

  senq = spanNearUnorderedQuery(1, senq, spanTermQuery(field, L"u2"));
  spans = senq->createWeight(searcher, false, 1.0f)
              ->getSpans(searcher->getIndexReader()->leaves()[0],
                         SpanWeight::Postings::POSITIONS);
  assertNext(spans, 4, 0, 3);
  assertNext(spans, 4, 1, 3); // unordered spans can be subsets
  assertNext(spans, 5, 0, 4);
  assertNext(spans, 5, 2, 4);
  assertNext(spans, 8, 0, 4);
  assertNext(spans, 8, 2, 4);
  assertNext(spans, 9, 0, 2);
  assertNext(spans, 9, 0, 4);
  assertNext(spans, 10, 0, 2);
  assertFinished(spans);
}

shared_ptr<Spans>
TestSpans::orSpans(std::deque<wstring> &terms) 
{
  return spanOrQuery(field, terms)
      .createWeight(searcher, false, 1.0f)
      .getSpans(searcher->getIndexReader()->leaves()[0],
                SpanWeight::Postings::POSITIONS);
}

void TestSpans::testSpanOrEmpty() 
{
  shared_ptr<Spans> spans = orSpans(std::deque<wstring>(0));
  assertFinished(spans);
}

void TestSpans::testSpanOrSingle() 
{
  shared_ptr<Spans> spans = orSpans(std::deque<wstring>{L"w5"});
  assertNext(spans, 0, 4, 5);
  assertFinished(spans);
}

void TestSpans::testSpanOrDouble() 
{
  shared_ptr<Spans> spans = orSpans(std::deque<wstring>{L"w5", L"yy"});
  assertNext(spans, 0, 4, 5);
  assertNext(spans, 2, 3, 4);
  assertNext(spans, 3, 4, 5);
  assertNext(spans, 7, 3, 4);
  assertFinished(spans);
}

void TestSpans::testSpanOrDoubleAdvance() 
{
  shared_ptr<Spans> spans = orSpans(std::deque<wstring>{L"w5", L"yy"});
  assertEquals(L"initial advance", 3, spans->advance(3));
  assertNext(spans, 3, 4, 5);
  assertNext(spans, 7, 3, 4);
  assertFinished(spans);
}

void TestSpans::testSpanOrUnused() 
{
  shared_ptr<Spans> spans =
      orSpans(std::deque<wstring>{L"w5", L"unusedTerm", L"yy"});
  assertNext(spans, 0, 4, 5);
  assertNext(spans, 2, 3, 4);
  assertNext(spans, 3, 4, 5);
  assertNext(spans, 7, 3, 4);
  assertFinished(spans);
}

void TestSpans::testSpanOrTripleSameDoc() 
{
  shared_ptr<Spans> spans = orSpans(std::deque<wstring>{L"t1", L"t2", L"t3"});
  assertNext(spans, 11, 0, 1);
  assertNext(spans, 11, 1, 2);
  assertNext(spans, 11, 2, 3);
  assertNext(spans, 11, 3, 4);
  assertNext(spans, 11, 4, 5);
  assertNext(spans, 11, 5, 6);
  assertFinished(spans);
}

void TestSpans::testSpanScorerZeroSloppyFreq() 
{
  shared_ptr<IndexReaderContext> topReaderContext =
      searcher->getTopReaderContext();
  deque<std::shared_ptr<LeafReaderContext>> leaves =
      topReaderContext->leaves();
  int subIndex = ReaderUtil::subIndex(11, leaves);
  for (int i = 0, c = leaves.size(); i < c; i++) {
    shared_ptr<LeafReaderContext> *const ctx = leaves[i];

    shared_ptr<Similarity> *const sim =
        make_shared<ClassicSimilarityAnonymousInnerClass>(shared_from_this());

    shared_ptr<Similarity> *const oldSim = searcher->getSimilarity(true);
    shared_ptr<Scorer> spanScorer;
    try {
      searcher->setSimilarity(sim);
      shared_ptr<SpanQuery> snq = spanNearOrderedQuery(field, 1, L"t1", L"t2");
      spanScorer = searcher->createNormalizedWeight(snq, true)->scorer(ctx);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      searcher->setSimilarity(oldSim);
    }
    if (i == subIndex) {
      assertTrue(L"first doc", spanScorer->begin().nextDoc() !=
                                   DocIdSetIterator::NO_MORE_DOCS);
      assertEquals(L"first doc number", spanScorer->docID() + ctx->docBase, 11);
      float score = spanScorer->score();
      assertTrue(L"first doc score should be zero, " + to_wstring(score),
                 score == 0.0f);
    } else {
      assertTrue(L"no second doc",
                 spanScorer == nullptr || spanScorer->begin().nextDoc() ==
                                              DocIdSetIterator::NO_MORE_DOCS);
    }
  }
}

TestSpans::ClassicSimilarityAnonymousInnerClass::
    ClassicSimilarityAnonymousInnerClass(shared_ptr<TestSpans> outerInstance)
{
  this->outerInstance = outerInstance;
}

float TestSpans::ClassicSimilarityAnonymousInnerClass::sloppyFreq(int distance)
{
  return 0.0f;
}

void TestSpans::addDoc(shared_ptr<IndexWriter> writer, const wstring &id,
                       const wstring &text) 
{
  shared_ptr<Document> *const doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", id, Field::Store::YES));
  doc->push_back(newTextField(L"text", text, Field::Store::YES));
  writer->addDocument(doc);
}

int64_t TestSpans::hitCount(shared_ptr<IndexSearcher> searcher,
                              const wstring &word) 
{
  return searcher
      ->search(make_shared<TermQuery>(make_shared<Term>(L"text", word)), 10)
      ->totalHits;
}

shared_ptr<SpanQuery> TestSpans::createSpan(const wstring &value)
{
  return spanTermQuery(L"text", value);
}

shared_ptr<SpanQuery>
TestSpans::createSpan(int slop, bool ordered,
                      std::deque<std::shared_ptr<SpanQuery>> &clauses)
{
  if (ordered) {
    return spanNearOrderedQuery(slop, clauses);
  } else {
    return spanNearUnorderedQuery(slop, clauses);
  }
}

shared_ptr<SpanQuery> TestSpans::createSpan(int slop, bool ordered,
                                            const wstring &term1,
                                            const wstring &term2)
{
  return createSpan(slop, ordered,
                    std::deque<std::shared_ptr<SpanQuery>>{createSpan(term1),
                                                            createSpan(term2)});
}

void TestSpans::testNPESpanQuery() 
{
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<IndexWriter> *const writer = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));

  // Add documents
  addDoc(writer, L"1", L"the big dogs went running to the market");
  addDoc(writer, L"2",
         L"the cat chased the mouse, then the cat ate the mouse quickly");

  // Commit
  delete writer;

  // Get searcher
  shared_ptr<IndexReader> *const reader = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> *const searcher = newSearcher(reader);

  // Control (make sure docs indexed)
  assertEquals(2, hitCount(searcher, L"the"));
  assertEquals(1, hitCount(searcher, L"cat"));
  assertEquals(1, hitCount(searcher, L"dogs"));
  assertEquals(0, hitCount(searcher, L"rabbit"));

  // This throws exception (it shouldn't)
  assertEquals(
      1, searcher
             ->search(createSpan(0, true,
                                 std::deque<std::shared_ptr<SpanQuery>>{
                                     createSpan(4, false, L"chased", L"cat"),
                                     createSpan(L"ate")}),
                      10)
             ->totalHits);
  delete reader;
  delete dir;
}

void TestSpans::testSpanNotWithMultiterm() 
{
  shared_ptr<SpanQuery> q = spanNotQuery(
      spanTermQuery(field, L"r1"),
      make_shared<SpanMultiTermQueryWrapper<>>(
          make_shared<PrefixQuery>(make_shared<Term>(field, L"s1"))),
      3, 3);
  checkHits(q, std::deque<int>{14});

  q = spanNotQuery(
      spanTermQuery(field, L"r1"),
      make_shared<SpanMultiTermQueryWrapper<>>(
          make_shared<FuzzyQuery>(make_shared<Term>(field, L"s12"), 1, 2)),
      3, 3);
  checkHits(q, std::deque<int>{14});

  q = spanNotQuery(
      make_shared<SpanMultiTermQueryWrapper<>>(
          make_shared<PrefixQuery>(make_shared<Term>(field, L"r"))),
      spanTermQuery(field, L"s21"), 3, 3);
  checkHits(q, std::deque<int>{13});
}

void TestSpans::testSpanNots() 
{

  assertEquals(L"SpanNotIncludeExcludeSame1", 0,
               spanCount(L"s2", 0, L"s2", 0, 0), 0);
  assertEquals(L"SpanNotIncludeExcludeSame2", 0,
               spanCount(L"s2", 0, L"s2", 10, 10), 0);

  // focus on behind
  assertEquals(L"SpanNotS2NotS1_6_0", 1, spanCount(L"s2", 0, L"s1", 6, 0));
  assertEquals(L"SpanNotS2NotS1_5_0", 2, spanCount(L"s2", 0, L"s1", 5, 0));
  assertEquals(L"SpanNotS2NotS1_3_0", 3, spanCount(L"s2", 0, L"s1", 3, 0));
  assertEquals(L"SpanNotS2NotS1_2_0", 4, spanCount(L"s2", 0, L"s1", 2, 0));
  assertEquals(L"SpanNotS2NotS1_0_0", 4, spanCount(L"s2", 0, L"s1", 0, 0));

  // focus on both
  assertEquals(L"SpanNotS2NotS1_3_1", 2, spanCount(L"s2", 0, L"s1", 3, 1));
  assertEquals(L"SpanNotS2NotS1_2_1", 3, spanCount(L"s2", 0, L"s1", 2, 1));
  assertEquals(L"SpanNotS2NotS1_1_1", 3, spanCount(L"s2", 0, L"s1", 1, 1));
  assertEquals(L"SpanNotS2NotS1_10_10", 0, spanCount(L"s2", 0, L"s1", 10, 10));

  // focus on ahead
  assertEquals(L"SpanNotS1NotS2_10_10", 0, spanCount(L"s1", 0, L"s2", 10, 10));
  assertEquals(L"SpanNotS1NotS2_0_1", 3, spanCount(L"s1", 0, L"s2", 0, 1));
  assertEquals(L"SpanNotS1NotS2_0_2", 3, spanCount(L"s1", 0, L"s2", 0, 2));
  assertEquals(L"SpanNotS1NotS2_0_3", 2, spanCount(L"s1", 0, L"s2", 0, 3));
  assertEquals(L"SpanNotS1NotS2_0_4", 1, spanCount(L"s1", 0, L"s2", 0, 4));
  assertEquals(L"SpanNotS1NotS2_0_8", 0, spanCount(L"s1", 0, L"s2", 0, 8));

  // exclude doesn't exist
  assertEquals(L"SpanNotS1NotS3_8_8", 3, spanCount(L"s1", 0, L"s3", 8, 8));

  // include doesn't exist
  assertEquals(L"SpanNotS3NotS1_8_8", 0, spanCount(L"s3", 0, L"s1", 8, 8));

  // Negative values
  assertEquals(L"SpanNotS2S1NotXXNeg_0_0", 1,
               spanCount(L"s2 s1", 10, L"xx", 0, 0));
  assertEquals(L"SpanNotS2S1NotXXNeg_1_1", 1,
               spanCount(L"s2 s1", 10, L"xx", -1, -1));
  assertEquals(L"SpanNotS2S1NotXXNeg_0_2", 2,
               spanCount(L"s2 s1", 10, L"xx", 0, -2));
  assertEquals(L"SpanNotS2S1NotXXNeg_1_2", 2,
               spanCount(L"s2 s1", 10, L"xx", -1, -2));
  assertEquals(L"SpanNotS2S1NotXXNeg_2_1", 2,
               spanCount(L"s2 s1", 10, L"xx", -2, -1));
  assertEquals(L"SpanNotS2S1NotXXNeg_3_1", 2,
               spanCount(L"s2 s1", 10, L"xx", -3, -1));
  assertEquals(L"SpanNotS2S1NotXXNeg_1_3", 2,
               spanCount(L"s2 s1", 10, L"xx", -1, -3));
  assertEquals(L"SpanNotS2S1NotXXNeg_2_2", 3,
               spanCount(L"s2 s1", 10, L"xx", -2, -2));
}

int TestSpans::spanCount(const wstring &include, int slop,
                         const wstring &exclude, int pre,
                         int post) 
{
  std::deque<wstring> includeTerms = include.split(L" +");
  shared_ptr<SpanQuery> iq =
      includeTerms.size() == 1
          ? spanTermQuery(field, include)
          : spanNearOrderedQuery(field, slop, includeTerms);
  shared_ptr<SpanQuery> eq = spanTermQuery(field, exclude);
  shared_ptr<SpanQuery> snq = spanNotQuery(iq, eq, pre, post);
  shared_ptr<Spans> spans =
      snq->createWeight(searcher, false, 1.0f)
          ->getSpans(searcher->getIndexReader()->leaves()[0],
                     SpanWeight::Postings::POSITIONS);

  int i = 0;
  if (spans != nullptr) {
    while (spans->nextDoc() != Spans::NO_MORE_DOCS) {
      while (spans->nextStartPosition() != Spans::NO_MORE_POSITIONS) {
        i++;
      }
    }
  }
  return i;
}
} // namespace org::apache::lucene::search::spans