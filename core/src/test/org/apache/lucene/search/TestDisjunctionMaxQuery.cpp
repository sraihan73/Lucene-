using namespace std;

#include "TestDisjunctionMaxQuery.h"

namespace org::apache::lucene::search
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

TestDisjunctionMaxQuery::TestSimilarity::TestSimilarity() {}

float TestDisjunctionMaxQuery::TestSimilarity::tf(float freq)
{
  if (freq > 0.0f) {
    return 1.0f;
  } else {
    return 0.0f;
  }
}

float TestDisjunctionMaxQuery::TestSimilarity::lengthNorm(int length)
{
  // Disable length norm
  return 1;
}

float TestDisjunctionMaxQuery::TestSimilarity::idf(int64_t docFreq,
                                                   int64_t docCount)
{
  return 1.0f;
}

const shared_ptr<org::apache::lucene::document::FieldType>
    TestDisjunctionMaxQuery::nonAnalyzedType =
        make_shared<org::apache::lucene::document::FieldType>(
            org::apache::lucene::document::TextField::TYPE_STORED);

TestDisjunctionMaxQuery::StaticConstructor::StaticConstructor()
{
  nonAnalyzedType->setTokenized(false);
}

TestDisjunctionMaxQuery::StaticConstructor
    TestDisjunctionMaxQuery::staticConstructor;

void TestDisjunctionMaxQuery::setUp() 
{
  LuceneTestCase::setUp();

  index = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), index,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setSimilarity(sim)
          ->setMergePolicy(newLogMergePolicy()));

  // hed is the most important field, dek is secondary

  {
    // d1 is an "ok" match for: albino elephant
    shared_ptr<Document> d1 = make_shared<Document>();
    d1->push_back(
        newField(L"id", L"d1", nonAnalyzedType)); // Field.Keyword("id",
                                                  // "d1"));
    d1->push_back(
        newTextField(L"hed", L"elephant",
                     Field::Store::YES)); // Field.Text("hed", "elephant"));
    d1->push_back(
        newTextField(L"dek", L"elephant",
                     Field::Store::YES)); // Field.Text("dek", "elephant"));
    writer->addDocument(d1);
  }

  {
    // d2 is a "good" match for: albino elephant
    shared_ptr<Document> d2 = make_shared<Document>();
    d2->push_back(
        newField(L"id", L"d2", nonAnalyzedType)); // Field.Keyword("id",
                                                  // "d2"));
    d2->push_back(
        newTextField(L"hed", L"elephant",
                     Field::Store::YES)); // Field.Text("hed", "elephant"));
    d2->push_back(newTextField(L"dek", L"albino",
                               Field::Store::YES)); // Field.Text("dek",
                                                    // "albino"));
    d2->push_back(
        newTextField(L"dek", L"elephant",
                     Field::Store::YES)); // Field.Text("dek", "elephant"));
    writer->addDocument(d2);
  }

  {
    // d3 is a "better" match for: albino elephant
    shared_ptr<Document> d3 = make_shared<Document>();
    d3->push_back(
        newField(L"id", L"d3", nonAnalyzedType)); // Field.Keyword("id",
                                                  // "d3"));
    d3->push_back(newTextField(L"hed", L"albino",
                               Field::Store::YES)); // Field.Text("hed",
                                                    // "albino"));
    d3->push_back(
        newTextField(L"hed", L"elephant",
                     Field::Store::YES)); // Field.Text("hed", "elephant"));
    writer->addDocument(d3);
  }

  {
    // d4 is the "best" match for: albino elephant
    shared_ptr<Document> d4 = make_shared<Document>();
    d4->push_back(
        newField(L"id", L"d4", nonAnalyzedType)); // Field.Keyword("id",
                                                  // "d4"));
    d4->push_back(newTextField(L"hed", L"albino",
                               Field::Store::YES)); // Field.Text("hed",
                                                    // "albino"));
    d4->push_back(newField(L"hed", L"elephant",
                           nonAnalyzedType)); // Field.Text("hed", "elephant"));
    d4->push_back(newTextField(L"dek", L"albino",
                               Field::Store::YES)); // Field.Text("dek",
                                                    // "albino"));
    writer->addDocument(d4);
  }

  writer->forceMerge(1);
  r = getOnlyLeafReader(writer->getReader());
  delete writer;
  s = make_shared<IndexSearcher>(r);
  s->setSimilarity(sim);
}

void TestDisjunctionMaxQuery::tearDown() 
{
  delete r;
  delete index;
  LuceneTestCase::tearDown();
}

void TestDisjunctionMaxQuery::testSkipToFirsttimeMiss() 
{
  shared_ptr<DisjunctionMaxQuery> *const dq = make_shared<DisjunctionMaxQuery>(
      Arrays::asList(tq(L"id", L"d1"), tq(L"dek", L"DOES_NOT_EXIST")), 0.0f);

  QueryUtils::check(random(), dq, s);
  assertTrue(std::dynamic_pointer_cast<LeafReaderContext>(
                 s->getTopReaderContext()) != nullptr);
  shared_ptr<Weight> *const dw = s->createWeight(s->rewrite(dq), true, 1);
  shared_ptr<LeafReaderContext> context =
      std::static_pointer_cast<LeafReaderContext>(s->getTopReaderContext());
  shared_ptr<Scorer> *const ds = dw->scorer(context);
  constexpr bool skipOk =
      ds->begin().advance(3) != DocIdSetIterator::NO_MORE_DOCS;
  if (skipOk) {
    fail(L"firsttime skipTo found a match? ... " +
         r->document(ds->docID())[L"id"]);
  }
}

void TestDisjunctionMaxQuery::testSkipToFirsttimeHit() 
{
  shared_ptr<DisjunctionMaxQuery> *const dq = make_shared<DisjunctionMaxQuery>(
      Arrays::asList(tq(L"dek", L"albino"), tq(L"dek", L"DOES_NOT_EXIST")),
      0.0f);

  assertTrue(std::dynamic_pointer_cast<LeafReaderContext>(
                 s->getTopReaderContext()) != nullptr);
  QueryUtils::check(random(), dq, s);
  shared_ptr<Weight> *const dw = s->createWeight(s->rewrite(dq), true, 1);
  shared_ptr<LeafReaderContext> context =
      std::static_pointer_cast<LeafReaderContext>(s->getTopReaderContext());
  shared_ptr<Scorer> *const ds = dw->scorer(context);
  assertTrue(L"firsttime skipTo found no match",
             ds->begin().advance(3) != DocIdSetIterator::NO_MORE_DOCS);
  assertEquals(L"found wrong docid", L"d4", r->document(ds->docID())[L"id"]);
}

void TestDisjunctionMaxQuery::testSimpleEqualScores1() 
{

  shared_ptr<DisjunctionMaxQuery> q = make_shared<DisjunctionMaxQuery>(
      Arrays::asList(tq(L"hed", L"albino"), tq(L"hed", L"elephant")), 0.0f);
  QueryUtils::check(random(), q, s);

  std::deque<std::shared_ptr<ScoreDoc>> h = s->search(q, 1000)->scoreDocs;

  try {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(L"all docs should match " + q->toString(), 4, h.size());

    float score = h[0]->score;
    for (int i = 1; i < h.size(); i++) {
      assertEquals(L"score #" + to_wstring(i) + L" is not the same", score,
                   h[i]->score, SCORE_COMP_THRESH);
    }
  } catch (const Error &e) {
    printHits(L"testSimpleEqualScores1", h, s);
    throw e;
  }
}

void TestDisjunctionMaxQuery::testSimpleEqualScores2() 
{

  shared_ptr<DisjunctionMaxQuery> q = make_shared<DisjunctionMaxQuery>(
      Arrays::asList(tq(L"dek", L"albino"), tq(L"dek", L"elephant")), 0.0f);
  QueryUtils::check(random(), q, s);

  std::deque<std::shared_ptr<ScoreDoc>> h = s->search(q, 1000)->scoreDocs;

  try {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(L"3 docs should match " + q->toString(), 3, h.size());
    float score = h[0]->score;
    for (int i = 1; i < h.size(); i++) {
      assertEquals(L"score #" + to_wstring(i) + L" is not the same", score,
                   h[i]->score, SCORE_COMP_THRESH);
    }
  } catch (const Error &e) {
    printHits(L"testSimpleEqualScores2", h, s);
    throw e;
  }
}

void TestDisjunctionMaxQuery::testSimpleEqualScores3() 
{

  shared_ptr<DisjunctionMaxQuery> q = make_shared<DisjunctionMaxQuery>(
      Arrays::asList(tq(L"hed", L"albino"), tq(L"hed", L"elephant"),
                     tq(L"dek", L"albino"), tq(L"dek", L"elephant")),
      0.0f);
  QueryUtils::check(random(), q, s);

  std::deque<std::shared_ptr<ScoreDoc>> h = s->search(q, 1000)->scoreDocs;

  try {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(L"all docs should match " + q->toString(), 4, h.size());
    float score = h[0]->score;
    for (int i = 1; i < h.size(); i++) {
      assertEquals(L"score #" + to_wstring(i) + L" is not the same", score,
                   h[i]->score, SCORE_COMP_THRESH);
    }
  } catch (const Error &e) {
    printHits(L"testSimpleEqualScores3", h, s);
    throw e;
  }
}

void TestDisjunctionMaxQuery::testSimpleTiebreaker() 
{

  shared_ptr<DisjunctionMaxQuery> q = make_shared<DisjunctionMaxQuery>(
      Arrays::asList(tq(L"dek", L"albino"), tq(L"dek", L"elephant")), 0.01f);
  QueryUtils::check(random(), q, s);

  std::deque<std::shared_ptr<ScoreDoc>> h = s->search(q, 1000)->scoreDocs;

  try {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(L"3 docs should match " + q->toString(), 3, h.size());
    assertEquals(L"wrong first", L"d2", s->doc(h[0]->doc)[L"id"]);
    float score0 = h[0]->score;
    float score1 = h[1]->score;
    float score2 = h[2]->score;
    assertTrue(L"d2 does not have better score then others: " +
                   to_wstring(score0) + L" >? " + to_wstring(score1),
               score0 > score1);
    assertEquals(L"d4 and d1 don't have equal scores", score1, score2,
                 SCORE_COMP_THRESH);
  } catch (const Error &e) {
    printHits(L"testSimpleTiebreaker", h, s);
    throw e;
  }
}

void TestDisjunctionMaxQuery::testBooleanRequiredEqualScores() throw(
    runtime_error)
{

  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  {
    shared_ptr<DisjunctionMaxQuery> q1 = make_shared<DisjunctionMaxQuery>(
        Arrays::asList(tq(L"hed", L"albino"), tq(L"dek", L"albino")), 0.0f);
    q->add(q1, BooleanClause::Occur::MUST); // true,false);
    QueryUtils::check(random(), q1, s);
  }
  {
    shared_ptr<DisjunctionMaxQuery> q2 = make_shared<DisjunctionMaxQuery>(
        Arrays::asList(tq(L"hed", L"elephant"), tq(L"dek", L"elephant")), 0.0f);
    q->add(q2, BooleanClause::Occur::MUST); // true,false);
    QueryUtils::check(random(), q2, s);
  }

  QueryUtils::check(random(), q->build(), s);

  std::deque<std::shared_ptr<ScoreDoc>> h =
      s->search(q->build(), 1000)->scoreDocs;

  try {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(L"3 docs should match " + q->toString(), 3, h.size());
    float score = h[0]->score;
    for (int i = 1; i < h.size(); i++) {
      assertEquals(L"score #" + to_wstring(i) + L" is not the same", score,
                   h[i]->score, SCORE_COMP_THRESH);
    }
  } catch (const Error &e) {
    printHits(L"testBooleanRequiredEqualScores1", h, s);
    throw e;
  }
}

void TestDisjunctionMaxQuery::testBooleanOptionalNoTiebreaker() throw(
    runtime_error)
{

  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  {
    shared_ptr<DisjunctionMaxQuery> q1 = make_shared<DisjunctionMaxQuery>(
        Arrays::asList(tq(L"hed", L"albino"), tq(L"dek", L"albino")), 0.0f);
    q->add(q1, BooleanClause::Occur::SHOULD); // false,false);
  }
  {
    shared_ptr<DisjunctionMaxQuery> q2 = make_shared<DisjunctionMaxQuery>(
        Arrays::asList(tq(L"hed", L"elephant"), tq(L"dek", L"elephant")), 0.0f);
    q->add(q2, BooleanClause::Occur::SHOULD); // false,false);
  }
  QueryUtils::check(random(), q->build(), s);

  std::deque<std::shared_ptr<ScoreDoc>> h =
      s->search(q->build(), 1000)->scoreDocs;

  try {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(L"4 docs should match " + q->toString(), 4, h.size());
    float score = h[0]->score;
    for (int i = 1; i < h.size() - 1; i++) { // note: -1
      assertEquals(L"score #" + to_wstring(i) + L" is not the same", score,
                   h[i]->score, SCORE_COMP_THRESH);
    }
    assertEquals(L"wrong last", L"d1", s->doc(h[h.size() - 1]->doc)[L"id"]);
    float score1 = h[h.size() - 1]->score;
    assertTrue(L"d1 does not have worse score then others: " +
                   to_wstring(score) + L" >? " + to_wstring(score1),
               score > score1);
  } catch (const Error &e) {
    printHits(L"testBooleanOptionalNoTiebreaker", h, s);
    throw e;
  }
}

void TestDisjunctionMaxQuery::testBooleanOptionalWithTiebreaker() throw(
    runtime_error)
{

  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  {
    shared_ptr<DisjunctionMaxQuery> q1 = make_shared<DisjunctionMaxQuery>(
        Arrays::asList(tq(L"hed", L"albino"), tq(L"dek", L"albino")), 0.01f);
    q->add(q1, BooleanClause::Occur::SHOULD); // false,false);
  }
  {
    shared_ptr<DisjunctionMaxQuery> q2 = make_shared<DisjunctionMaxQuery>(
        Arrays::asList(tq(L"hed", L"elephant"), tq(L"dek", L"elephant")),
        0.01f);
    q->add(q2, BooleanClause::Occur::SHOULD); // false,false);
  }
  QueryUtils::check(random(), q->build(), s);

  std::deque<std::shared_ptr<ScoreDoc>> h =
      s->search(q->build(), 1000)->scoreDocs;

  try {

    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(L"4 docs should match " + q->toString(), 4, h.size());

    float score0 = h[0]->score;
    float score1 = h[1]->score;
    float score2 = h[2]->score;
    float score3 = h[3]->score;

    wstring doc0 = s->doc(h[0]->doc)[L"id"];
    wstring doc1 = s->doc(h[1]->doc)[L"id"];
    wstring doc2 = s->doc(h[2]->doc)[L"id"];
    wstring doc3 = s->doc(h[3]->doc)[L"id"];

    assertTrue(L"doc0 should be d2 or d4: " + doc0,
               doc0 == L"d2" || doc0 == L"d4");
    assertTrue(L"doc1 should be d2 or d4: " + doc0,
               doc1 == L"d2" || doc1 == L"d4");
    assertEquals(L"score0 and score1 should match", score0, score1,
                 SCORE_COMP_THRESH);
    assertEquals(L"wrong third", L"d3", doc2);
    assertTrue(L"d3 does not have worse score then d2 and d4: " +
                   to_wstring(score1) + L" >? " + to_wstring(score2),
               score1 > score2);

    assertEquals(L"wrong fourth", L"d1", doc3);
    assertTrue(L"d1 does not have worse score then d3: " + to_wstring(score2) +
                   L" >? " + to_wstring(score3),
               score2 > score3);

  } catch (const Error &e) {
    printHits(L"testBooleanOptionalWithTiebreaker", h, s);
    throw e;
  }
}

void TestDisjunctionMaxQuery::testBooleanOptionalWithTiebreakerAndBoost() throw(
    runtime_error)
{

  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  {
    shared_ptr<DisjunctionMaxQuery> q1 = make_shared<DisjunctionMaxQuery>(
        Arrays::asList(tq(L"hed", L"albino", 1.5f), tq(L"dek", L"albino")),
        0.01f);
    q->add(q1, BooleanClause::Occur::SHOULD); // false,false);
  }
  {
    shared_ptr<DisjunctionMaxQuery> q2 = make_shared<DisjunctionMaxQuery>(
        Arrays::asList(tq(L"hed", L"elephant", 1.5f), tq(L"dek", L"elephant")),
        0.01f);
    q->add(q2, BooleanClause::Occur::SHOULD); // false,false);
  }
  QueryUtils::check(random(), q->build(), s);

  std::deque<std::shared_ptr<ScoreDoc>> h =
      s->search(q->build(), 1000)->scoreDocs;

  try {

    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(L"4 docs should match " + q->toString(), 4, h.size());

    float score0 = h[0]->score;
    float score1 = h[1]->score;
    float score2 = h[2]->score;
    float score3 = h[3]->score;

    wstring doc0 = s->doc(h[0]->doc)[L"id"];
    wstring doc1 = s->doc(h[1]->doc)[L"id"];
    wstring doc2 = s->doc(h[2]->doc)[L"id"];
    wstring doc3 = s->doc(h[3]->doc)[L"id"];

    assertEquals(L"doc0 should be d4: ", L"d4", doc0);
    assertEquals(L"doc1 should be d3: ", L"d3", doc1);
    assertEquals(L"doc2 should be d2: ", L"d2", doc2);
    assertEquals(L"doc3 should be d1: ", L"d1", doc3);

    assertTrue(L"d4 does not have a better score then d3: " +
                   to_wstring(score0) + L" >? " + to_wstring(score1),
               score0 > score1);
    assertTrue(L"d3 does not have a better score then d2: " +
                   to_wstring(score1) + L" >? " + to_wstring(score2),
               score1 > score2);
    assertTrue(L"d3 does not have a better score then d1: " +
                   to_wstring(score2) + L" >? " + to_wstring(score3),
               score2 > score3);

  } catch (const Error &e) {
    printHits(L"testBooleanOptionalWithTiebreakerAndBoost", h, s);
    throw e;
  }
}

void TestDisjunctionMaxQuery::testBooleanSpanQuery() 
{
  int hits = 0;
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<Analyzer> indexerAnalyzer = make_shared<MockAnalyzer>(random());

  shared_ptr<IndexWriterConfig> config =
      make_shared<IndexWriterConfig>(indexerAnalyzer);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(directory, config);
  wstring FIELD = L"content";
  shared_ptr<Document> d = make_shared<Document>();
  d->push_back(
      make_shared<TextField>(FIELD, L"clockwork orange", Field::Store::YES));
  writer->addDocument(d);
  delete writer;

  shared_ptr<IndexReader> indexReader = DirectoryReader::open(directory);
  shared_ptr<IndexSearcher> searcher = newSearcher(indexReader);

  shared_ptr<DisjunctionMaxQuery> query = make_shared<DisjunctionMaxQuery>(
      Arrays::asList(
          make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"clockwork")),
          make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"clckwork"))),
      1.0f);
  shared_ptr<TopScoreDocCollector> collector =
      TopScoreDocCollector::create(1000);
  searcher->search(query, collector);
  hits = collector->topDocs()->scoreDocs.size();
  for (auto scoreDoc : collector->topDocs()->scoreDocs) {
    wcout << scoreDoc->doc << endl;
  }
  delete indexReader;
  assertEquals(hits, 1);
  delete directory;
}

void TestDisjunctionMaxQuery::testNegativeScore() 
{
  shared_ptr<DisjunctionMaxQuery> q = make_shared<DisjunctionMaxQuery>(
      Arrays::asList(make_shared<BoostQuery>(tq(L"hed", L"albino"), -1.0f),
                     make_shared<BoostQuery>(tq(L"hed", L"elephant"), -1.0f)),
      0.0f);

  std::deque<std::shared_ptr<ScoreDoc>> h = s->search(q, 1000)->scoreDocs;

  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"all docs should match " + q->toString(), 4, h.size());

  for (int i = 0; i < h.size(); i++) {
    assertTrue(L"score should be negative", h[i]->score < 0);
  }
}

void TestDisjunctionMaxQuery::testRewriteBoolean() 
{
  shared_ptr<Query> sub1 = tq(L"hed", L"albino");
  shared_ptr<Query> sub2 = tq(L"hed", L"elephant");
  shared_ptr<DisjunctionMaxQuery> q =
      make_shared<DisjunctionMaxQuery>(Arrays::asList(sub1, sub2), 1.0f);
  shared_ptr<Query> rewritten = s->rewrite(q);
  assertTrue(std::dynamic_pointer_cast<BooleanQuery>(rewritten) != nullptr);
  shared_ptr<BooleanQuery> bq =
      std::static_pointer_cast<BooleanQuery>(rewritten);
  assertEquals(bq->clauses().size(), 2);
  assertEquals(bq->clauses()[0],
               make_shared<BooleanClause>(sub1, BooleanClause::Occur::SHOULD));
  assertEquals(bq->clauses()[1],
               make_shared<BooleanClause>(sub2, BooleanClause::Occur::SHOULD));
}

shared_ptr<Query> TestDisjunctionMaxQuery::tq(const wstring &f,
                                              const wstring &t)
{
  return make_shared<TermQuery>(make_shared<Term>(f, t));
}

shared_ptr<Query> TestDisjunctionMaxQuery::tq(const wstring &f,
                                              const wstring &t, float b)
{
  shared_ptr<Query> q = tq(f, t);
  return make_shared<BoostQuery>(q, b);
}

void TestDisjunctionMaxQuery::printHits(
    const wstring &test, std::deque<std::shared_ptr<ScoreDoc>> &h,
    shared_ptr<IndexSearcher> searcher) 
{

  System::err::println(L"------- " + test + L" -------");

  shared_ptr<DecimalFormat> f = make_shared<DecimalFormat>(
      L"0.000000000", DecimalFormatSymbols::getInstance(Locale::ROOT));

  for (int i = 0; i < h.size(); i++) {
    shared_ptr<Document> d = searcher->doc(h[i]->doc);
    float score = h[i]->score;
    System::err::println(L"#" + to_wstring(i) + L": " + f->format(score) +
                         L" - " + d[L"id"]);
  }
}
} // namespace org::apache::lucene::search