using namespace std;

#include "TestPhraseQuery.h"

namespace org::apache::lucene::search
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenFilter = org::apache::lucene::analysis::MockTokenFilter;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Token = org::apache::lucene::analysis::Token;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using TextField = org::apache::lucene::document::TextField;
using IndexReader = org::apache::lucene::index::IndexReader;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using BM25Similarity =
    org::apache::lucene::search::similarities::BM25Similarity;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::AfterClass;
using org::junit::BeforeClass;
shared_ptr<IndexSearcher> TestPhraseQuery::searcher;
shared_ptr<org::apache::lucene::index::IndexReader> TestPhraseQuery::reader;
shared_ptr<org::apache::lucene::store::Directory> TestPhraseQuery::directory;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestPhraseQuery::beforeClass() 
{
  directory = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<AnalyzerAnonymousInnerClass>();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), directory, analyzer);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"field", L"one two three four five", Field::Store::YES));
  doc->push_back(newTextField(L"repeated",
                              L"this is a repeated field - first part",
                              Field::Store::YES));
  shared_ptr<Field> repeatedField = newTextField(
      L"repeated", L"second part of a repeated field", Field::Store::YES);
  doc->push_back(repeatedField);
  doc->push_back(
      newTextField(L"palindrome", L"one two three two one", Field::Store::YES));
  writer->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(newTextField(L"nonexist", L"phrase exist notexist exist found",
                              Field::Store::YES));
  writer->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(newTextField(L"nonexist", L"phrase exist notexist exist found",
                              Field::Store::YES));
  writer->addDocument(doc);

  reader = writer->getReader();
  delete writer;

  searcher = make_shared<IndexSearcher>(reader);
}

TestPhraseQuery::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass() {}

shared_ptr<Analyzer::TokenStreamComponents>
TestPhraseQuery::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false));
}

int TestPhraseQuery::AnalyzerAnonymousInnerClass::getPositionIncrementGap(
    const wstring &fieldName)
{
  return 100;
}

void TestPhraseQuery::setUp() ; }

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
void TestPhraseQuery::afterClass() 
{
  searcher.reset();
  delete reader;
  reader.reset();
  delete directory;
  directory.reset();
}

void TestPhraseQuery::testNotCloseEnough() 
{
  query = make_shared<PhraseQuery>(2, L"field", L"one", L"five");
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(query, 1000)->scoreDocs;
  TestUtil::assertEquals(0, hits.size());
  QueryUtils::check(random(), query, searcher);
}

void TestPhraseQuery::testBarelyCloseEnough() 
{
  query = make_shared<PhraseQuery>(3, L"field", L"one", L"five");
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(query, 1000)->scoreDocs;
  TestUtil::assertEquals(1, hits.size());
  QueryUtils::check(random(), query, searcher);
}

void TestPhraseQuery::testExact() 
{
  // slop is zero by default
  query = make_shared<PhraseQuery>(L"field", L"four", L"five");
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"exact match", 1, hits.size());
  QueryUtils::check(random(), query, searcher);

  query = make_shared<PhraseQuery>(L"field", L"two", L"one");
  hits = searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"reverse not exact", 0, hits.size());
  QueryUtils::check(random(), query, searcher);
}

void TestPhraseQuery::testSlop1() 
{
  // Ensures slop of 1 works with terms in order.
  query = make_shared<PhraseQuery>(1, L"field", L"one", L"two");
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"in order", 1, hits.size());
  QueryUtils::check(random(), query, searcher);

  // Ensures slop of 1 does not work for phrases out of order;
  // must be at least 2.
  query = make_shared<PhraseQuery>(1, L"field", L"two", L"one");
  hits = searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"reversed, slop not 2 or more", 0, hits.size());
  QueryUtils::check(random(), query, searcher);
}

void TestPhraseQuery::testOrderDoesntMatter() 
{
  // must be at least two for reverse order match
  query = make_shared<PhraseQuery>(2, L"field", L"two", L"one");
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"just sloppy enough", 1, hits.size());
  QueryUtils::check(random(), query, searcher);

  query = make_shared<PhraseQuery>(2, L"field", L"three", L"one");
  hits = searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"not sloppy enough", 0, hits.size());
  QueryUtils::check(random(), query, searcher);
}

void TestPhraseQuery::testMultipleTerms() 
{
  query = make_shared<PhraseQuery>(2, L"field", L"one", L"three", L"five");
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"two total moves", 1, hits.size());
  QueryUtils::check(random(), query, searcher);

  // it takes six moves to match this phrase
  query = make_shared<PhraseQuery>(5, L"field", L"five", L"three", L"one");
  hits = searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"slop of 5 not close enough", 0, hits.size());
  QueryUtils::check(random(), query, searcher);

  query = make_shared<PhraseQuery>(6, L"field", L"five", L"three", L"one");
  hits = searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"slop of 6 just right", 1, hits.size());
  QueryUtils::check(random(), query, searcher);
}

void TestPhraseQuery::testPhraseQueryWithStopAnalyzer() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<Analyzer> stopAnalyzer = make_shared<MockAnalyzer>(
      random(), MockTokenizer::SIMPLE, true, MockTokenFilter::ENGLISH_STOPSET);
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), directory, newIndexWriterConfig(stopAnalyzer));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"field", L"the stop words are here", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> reader = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  // valid exact phrase query
  shared_ptr<PhraseQuery> query =
      make_shared<PhraseQuery>(L"field", L"stop", L"words");
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(query, 1000)->scoreDocs;
  TestUtil::assertEquals(1, hits.size());
  QueryUtils::check(random(), query, searcher);

  delete reader;
  delete directory;
}

void TestPhraseQuery::testPhraseQueryInConjunctionScorer() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), directory);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"source", L"marketing info", Field::Store::YES));
  writer->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(newTextField(L"contents", L"foobar", Field::Store::YES));
  doc->push_back(newTextField(L"source", L"marketing info", Field::Store::YES));
  writer->addDocument(doc);

  shared_ptr<IndexReader> reader = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  shared_ptr<PhraseQuery> phraseQuery =
      make_shared<PhraseQuery>(L"source", L"marketing", L"info");
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(phraseQuery, 1000)->scoreDocs;
  TestUtil::assertEquals(2, hits.size());
  QueryUtils::check(random(), phraseQuery, searcher);

  shared_ptr<TermQuery> termQuery =
      make_shared<TermQuery>(make_shared<Term>(L"contents", L"foobar"));
  shared_ptr<BooleanQuery::Builder> booleanQuery =
      make_shared<BooleanQuery::Builder>();
  booleanQuery->add(termQuery, BooleanClause::Occur::MUST);
  booleanQuery->add(phraseQuery, BooleanClause::Occur::MUST);
  hits = searcher->search(booleanQuery->build(), 1000)->scoreDocs;
  TestUtil::assertEquals(1, hits.size());
  QueryUtils::check(random(), termQuery, searcher);

  delete reader;

  writer = make_shared<RandomIndexWriter>(
      random(), directory,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setOpenMode(OpenMode::CREATE));
  doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"contents", L"map_obj entry woo", Field::Store::YES));
  writer->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"contents", L"woo map_obj entry", Field::Store::YES));
  writer->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(newTextField(L"contents", L"map_obj foobarword entry woo",
                              Field::Store::YES));
  writer->addDocument(doc);

  reader = writer->getReader();
  delete writer;

  searcher = newSearcher(reader);

  termQuery = make_shared<TermQuery>(make_shared<Term>(L"contents", L"woo"));
  phraseQuery = make_shared<PhraseQuery>(L"contents", L"map_obj", L"entry");

  hits = searcher->search(termQuery, 1000)->scoreDocs;
  TestUtil::assertEquals(3, hits.size());
  hits = searcher->search(phraseQuery, 1000)->scoreDocs;
  TestUtil::assertEquals(2, hits.size());

  booleanQuery = make_shared<BooleanQuery::Builder>();
  booleanQuery->add(termQuery, BooleanClause::Occur::MUST);
  booleanQuery->add(phraseQuery, BooleanClause::Occur::MUST);
  hits = searcher->search(booleanQuery->build(), 1000)->scoreDocs;
  TestUtil::assertEquals(2, hits.size());

  booleanQuery = make_shared<BooleanQuery::Builder>();
  booleanQuery->add(phraseQuery, BooleanClause::Occur::MUST);
  booleanQuery->add(termQuery, BooleanClause::Occur::MUST);
  hits = searcher->search(booleanQuery->build(), 1000)->scoreDocs;
  TestUtil::assertEquals(2, hits.size());
  QueryUtils::check(random(), booleanQuery->build(), searcher);

  delete reader;
  delete directory;
}

void TestPhraseQuery::testSlopScoring() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), directory,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergePolicy(newLogMergePolicy())
          ->setSimilarity(make_shared<BM25Similarity>()));

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"field", L"foo firstname lastname foo", Field::Store::YES));
  writer->addDocument(doc);

  shared_ptr<Document> doc2 = make_shared<Document>();
  doc2->push_back(newTextField(L"field", L"foo firstname zzz lastname foo",
                               Field::Store::YES));
  writer->addDocument(doc2);

  shared_ptr<Document> doc3 = make_shared<Document>();
  doc3->push_back(newTextField(L"field", L"foo firstname zzz yyy lastname foo",
                               Field::Store::YES));
  writer->addDocument(doc3);

  shared_ptr<IndexReader> reader = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  searcher->setSimilarity(make_shared<ClassicSimilarity>());
  shared_ptr<PhraseQuery> query = make_shared<PhraseQuery>(
      numeric_limits<int>::max(), L"field", L"firstname", L"lastname");
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(query, 1000)->scoreDocs;
  TestUtil::assertEquals(3, hits.size());
  // Make sure that those matches where the terms appear closer to
  // each other get a higher score:
  assertEquals(1.0, hits[0]->score, 0.01);
  TestUtil::assertEquals(0, hits[0]->doc);
  assertEquals(0.63, hits[1]->score, 0.01);
  TestUtil::assertEquals(1, hits[1]->doc);
  assertEquals(0.47, hits[2]->score, 0.01);
  TestUtil::assertEquals(2, hits[2]->doc);
  QueryUtils::check(random(), query, searcher);
  delete reader;
  delete directory;
}

void TestPhraseQuery::testToString() 
{
  shared_ptr<PhraseQuery> q =
      make_shared<PhraseQuery>(L"field", std::deque<wstring>(0));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"\"\"", q->toString());

  shared_ptr<PhraseQuery::Builder> builder =
      make_shared<PhraseQuery::Builder>();
  builder->add(make_shared<Term>(L"field", L"hi"), 1);
  q = builder->build();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"field:\"? hi\"", q->toString());

  builder = make_shared<PhraseQuery::Builder>();
  builder->add(make_shared<Term>(L"field", L"hi"), 1);
  builder->add(make_shared<Term>(L"field", L"test"), 5);
  q = builder->build(); // Query "this hi this is a test is"

  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"field:\"? hi ? ? ? test\"", q->toString());

  builder = make_shared<PhraseQuery::Builder>();
  builder->add(make_shared<Term>(L"field", L"hi"), 1);
  builder->add(make_shared<Term>(L"field", L"hello"), 1);
  builder->add(make_shared<Term>(L"field", L"test"), 5);
  q = builder->build();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"field:\"? hi|hello ? ? ? test\"", q->toString());

  builder = make_shared<PhraseQuery::Builder>();
  builder->add(make_shared<Term>(L"field", L"hi"), 1);
  builder->add(make_shared<Term>(L"field", L"hello"), 1);
  builder->add(make_shared<Term>(L"field", L"test"), 5);
  builder->setSlop(5);
  q = builder->build();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"field:\"? hi|hello ? ? ? test\"~5", q->toString());
}

void TestPhraseQuery::testWrappedPhrase() 
{
  query = make_shared<PhraseQuery>(100, L"repeated", L"first", L"part",
                                   L"second", L"part");

  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"slop of 100 just right", 1, hits.size());
  QueryUtils::check(random(), query, searcher);

  query = make_shared<PhraseQuery>(99, L"repeated", L"first", L"part",
                                   L"second", L"part");

  hits = searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"slop of 99 not enough", 0, hits.size());
  QueryUtils::check(random(), query, searcher);
}

void TestPhraseQuery::testNonExistingPhrase() 
{
  // phrase without repetitions that exists in 2 docs
  query = make_shared<PhraseQuery>(2, L"nonexist", L"phrase", L"notexist",
                                   L"found");

  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"phrase without repetitions exists in 2 docs", 2, hits.size());
  QueryUtils::check(random(), query, searcher);

  // phrase with repetitions that exists in 2 docs
  query =
      make_shared<PhraseQuery>(1, L"nonexist", L"phrase", L"exist", L"exist");

  hits = searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"phrase with repetitions exists in two docs", 2, hits.size());
  QueryUtils::check(random(), query, searcher);

  // phrase I with repetitions that does not exist in any doc
  query = make_shared<PhraseQuery>(1000, L"nonexist", L"phrase", L"notexist",
                                   L"phrase");

  hits = searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"nonexisting phrase with repetitions does not exist in any doc",
               0, hits.size());
  QueryUtils::check(random(), query, searcher);

  // phrase II with repetitions that does not exist in any doc
  query = make_shared<PhraseQuery>(1000, L"nonexist", L"phrase", L"exist",
                                   L"exist", L"exist");

  hits = searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"nonexisting phrase with repetitions does not exist in any doc",
               0, hits.size());
  QueryUtils::check(random(), query, searcher);
}

void TestPhraseQuery::testPalyndrome2() 
{

  // search on non palyndrome, find phrase with no slop, using exact phrase
  // scorer
  query = make_shared<PhraseQuery>(L"field", L"two",
                                   L"three"); // to use exact phrase scorer
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"phrase found with exact phrase scorer", 1, hits.size());
  float score0 = hits[0]->score;
  // System.out.println("(exact) field: two three: "+score0);
  QueryUtils::check(random(), query, searcher);

  // search on non palyndrome, find phrase with slop 2, though no slop required
  // here.
  query = make_shared<PhraseQuery>(L"field", L"two",
                                   L"three"); // to use sloppy scorer
  hits = searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"just sloppy enough", 1, hits.size());
  float score1 = hits[0]->score;
  // System.out.println("(sloppy) field: two three: "+score1);
  assertEquals(L"exact scorer and sloppy scorer score the same when slop does "
               L"not matter",
               score0, score1, SCORE_COMP_THRESH);
  QueryUtils::check(random(), query, searcher);

  // search ordered in palyndrome, find it twice
  query = make_shared<PhraseQuery>(
      2, L"palindrome", L"two",
      L"three"); // must be at least two for both ordered and reversed to match
  hits = searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"just sloppy enough", 1, hits.size());
  // float score2 = hits[0].score;
  // System.out.println("palindrome: two three: "+score2);
  QueryUtils::check(random(), query, searcher);

  // commented out for sloppy-phrase efficiency (issue 736) - see
  // SloppyPhraseScorer.phraseFreq(). assertTrue("ordered scores higher in
  // palindrome",score1+SCORE_COMP_THRESH<score2);

  // search reveresed in palyndrome, find it twice
  query = make_shared<PhraseQuery>(
      2, L"palindrome", L"three",
      L"two"); // must be at least two for both ordered and reversed to match
  hits = searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"just sloppy enough", 1, hits.size());
  // float score3 = hits[0].score;
  // System.out.println("palindrome: three two: "+score3);
  QueryUtils::check(random(), query, searcher);

  // commented out for sloppy-phrase efficiency (issue 736) - see
  // SloppyPhraseScorer.phraseFreq(). assertTrue("reversed scores higher in
  // palindrome",score1+SCORE_COMP_THRESH<score3); assertEquals("ordered or
  // reversed does not matter",score2, score3, SCORE_COMP_THRESH);
}

void TestPhraseQuery::testPalyndrome3() 
{

  // search on non palyndrome, find phrase with no slop, using exact phrase
  // scorer slop=0 to use exact phrase scorer
  query = make_shared<PhraseQuery>(0, L"field", L"one", L"two", L"three");
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"phrase found with exact phrase scorer", 1, hits.size());
  float score0 = hits[0]->score;
  // System.out.println("(exact) field: one two three: "+score0);
  QueryUtils::check(random(), query, searcher);

  // just make sure no exc:
  searcher->explain(query, 0);

  // search on non palyndrome, find phrase with slop 3, though no slop required
  // here. slop=4 to use sloppy scorer
  query = make_shared<PhraseQuery>(4, L"field", L"one", L"two", L"three");
  hits = searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"just sloppy enough", 1, hits.size());
  float score1 = hits[0]->score;
  // System.out.println("(sloppy) field: one two three: "+score1);
  assertEquals(L"exact scorer and sloppy scorer score the same when slop does "
               L"not matter",
               score0, score1, SCORE_COMP_THRESH);
  QueryUtils::check(random(), query, searcher);

  // search ordered in palyndrome, find it twice
  // slop must be at least four for both ordered and reversed to match
  query = make_shared<PhraseQuery>(4, L"palindrome", L"one", L"two", L"three");
  hits = searcher->search(query, 1000)->scoreDocs;

  // just make sure no exc:
  searcher->explain(query, 0);

  assertEquals(L"just sloppy enough", 1, hits.size());
  // float score2 = hits[0].score;
  // System.out.println("palindrome: one two three: "+score2);
  QueryUtils::check(random(), query, searcher);

  // commented out for sloppy-phrase efficiency (issue 736) - see
  // SloppyPhraseScorer.phraseFreq(). assertTrue("ordered scores higher in
  // palindrome",score1+SCORE_COMP_THRESH<score2);

  // search reveresed in palyndrome, find it twice
  // must be at least four for both ordered and reversed to match
  query = make_shared<PhraseQuery>(4, L"palindrome", L"three", L"two", L"one");
  hits = searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"just sloppy enough", 1, hits.size());
  // float score3 = hits[0].score;
  // System.out.println("palindrome: three two one: "+score3);
  QueryUtils::check(random(), query, searcher);

  // commented out for sloppy-phrase efficiency (issue 736) - see
  // SloppyPhraseScorer.phraseFreq(). assertTrue("reversed scores higher in
  // palindrome",score1+SCORE_COMP_THRESH<score3); assertEquals("ordered or
  // reversed does not matter",score2, score3, SCORE_COMP_THRESH);
}

void TestPhraseQuery::testEmptyPhraseQuery() 
{
  shared_ptr<BooleanQuery::Builder> *const q2 =
      make_shared<BooleanQuery::Builder>();
  q2->add(make_shared<PhraseQuery>(L"field", std::deque<wstring>(0)),
          BooleanClause::Occur::MUST);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  q2->build()->toString();
}

void TestPhraseQuery::testRewrite() 
{
  shared_ptr<PhraseQuery> pq = make_shared<PhraseQuery>(L"foo", L"bar");
  shared_ptr<Query> rewritten = pq->rewrite(searcher->getIndexReader());
  assertTrue(std::dynamic_pointer_cast<TermQuery>(rewritten) != nullptr);
}

void TestPhraseQuery::testZeroPosIncr() 
{
  shared_ptr<Directory> dir = newDirectory();
  std::deque<std::shared_ptr<Token>> tokens(3);
  tokens[0] = make_shared<Token>();
  tokens[0]->append(L"a");
  tokens[0]->setPositionIncrement(1);
  tokens[1] = make_shared<Token>();
  tokens[1]->append(L"aa");
  tokens[1]->setPositionIncrement(0);
  tokens[2] = make_shared<Token>();
  tokens[2]->append(L"b");
  tokens[2]->setPositionIncrement(1);

  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<TextField>(L"field", make_shared<CannedTokenStream>(tokens)));
  writer->addDocument(doc);
  shared_ptr<IndexReader> r = writer->getReader();
  delete writer;
  shared_ptr<IndexSearcher> searcher = newSearcher(r);

  // Sanity check; simple "a b" phrase:
  shared_ptr<PhraseQuery::Builder> pqBuilder =
      make_shared<PhraseQuery::Builder>();
  pqBuilder->add(make_shared<Term>(L"field", L"a"), 0);
  pqBuilder->add(make_shared<Term>(L"field", L"b"), 1);
  TestUtil::assertEquals(1, searcher->search(pqBuilder->build(), 1)->totalHits);

  // Now with "a|aa b"
  pqBuilder = make_shared<PhraseQuery::Builder>();
  pqBuilder->add(make_shared<Term>(L"field", L"a"), 0);
  pqBuilder->add(make_shared<Term>(L"field", L"aa"), 0);
  pqBuilder->add(make_shared<Term>(L"field", L"b"), 1);
  TestUtil::assertEquals(1, searcher->search(pqBuilder->build(), 1)->totalHits);

  // Now with "a|z b" which should not match; this isn't a MultiPhraseQuery
  pqBuilder = make_shared<PhraseQuery::Builder>();
  pqBuilder->add(make_shared<Term>(L"field", L"a"), 0);
  pqBuilder->add(make_shared<Term>(L"field", L"z"), 0);
  pqBuilder->add(make_shared<Term>(L"field", L"b"), 1);
  TestUtil::assertEquals(0, searcher->search(pqBuilder->build(), 1)->totalHits);

  delete r;
  delete dir;
}

void TestPhraseQuery::testRandomPhrases() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());

  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(analyzer)->setMergePolicy(newLogMergePolicy()));
  deque<deque<wstring>> docs = deque<deque<wstring>>();
  shared_ptr<Document> d = make_shared<Document>();
  shared_ptr<Field> f = newTextField(L"f", L"", Field::Store::NO);
  d->push_back(f);

  shared_ptr<Random> r = random();

  int NUM_DOCS = atLeast(10);
  for (int i = 0; i < NUM_DOCS; i++) {
    // must be > 4096 so it spans multiple chunks
    int termCount = TestUtil::nextInt(random(), 4097, 8200);

    deque<wstring> doc = deque<wstring>();

    shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
    while (doc.size() < termCount) {
      if (r->nextInt(5) == 1 || docs.empty()) {
        // make new non-empty-string term
        wstring term;
        while (true) {
          term = TestUtil::randomUnicodeString(r);
          if (term.length() > 0) {
            break;
          }
        }
        // C++ NOTE: The following 'try with resources' block is replaced by its
        // C++ equivalent: ORIGINAL LINE: try
        // (org.apache.lucene.analysis.TokenStream ts =
        // analyzer.tokenStream("ignore", term))
        {
          org::apache::lucene::analysis::TokenStream ts =
              analyzer->tokenStream(L"ignore", term);
          shared_ptr<CharTermAttribute> termAttr =
              ts->addAttribute(CharTermAttribute::typeid);
          ts->reset();
          while (ts->incrementToken()) {
            // C++ TODO: There is no native C++ equivalent to 'toString':
            wstring text = termAttr->toString();
            doc.push_back(text);
            sb->append(text)->append(L' ');
          }
          ts->end();
        }
      } else {
        // pick existing sub-phrase
        deque<wstring> lastDoc = docs[r->nextInt(docs.size())];
        int len = TestUtil::nextInt(r, 1, 10);
        int start = r->nextInt(lastDoc.size() - len);
        for (int k = start; k < start + len; k++) {
          wstring t = lastDoc[k];
          doc.push_back(t);
          sb->append(t)->append(L' ');
        }
      }
    }
    docs.push_back(doc);
    f->setStringValue(sb->toString());
    w->addDocument(d);
  }

  shared_ptr<IndexReader> reader = w->getReader();
  shared_ptr<IndexSearcher> s = newSearcher(reader);
  delete w;

  // now search
  int num = atLeast(10);
  for (int i = 0; i < num; i++) {
    int docID = r->nextInt(docs.size());
    deque<wstring> doc = docs[docID];

    constexpr int numTerm = TestUtil::nextInt(r, 2, 20);
    constexpr int start = r->nextInt(doc.size() - numTerm);
    shared_ptr<PhraseQuery::Builder> builder =
        make_shared<PhraseQuery::Builder>();
    shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
    for (int t = start; t < start + numTerm; t++) {
      builder->add(make_shared<Term>(L"f", doc[t]), t);
      sb->append(doc[t])->append(L' ');
    }
    shared_ptr<PhraseQuery> pq = builder->build();

    shared_ptr<TopDocs> hits = s->search(pq, NUM_DOCS);
    bool found = false;
    for (int j = 0; j < hits->scoreDocs.size(); j++) {
      if (hits->scoreDocs[j]->doc == docID) {
        found = true;
        break;
      }
    }

    assertTrue(L"phrase '" + sb + L"' not found; start=" + to_wstring(start) +
                   L", it=" + to_wstring(i) + L", expected doc " +
                   to_wstring(docID),
               found);
  }

  delete reader;
  delete dir;
}

void TestPhraseQuery::testNegativeSlop() 
{
  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<PhraseQuery>(-2, L"field", L"two", L"one");
  });
}

void TestPhraseQuery::testNegativePosition() 
{
  shared_ptr<PhraseQuery::Builder> builder =
      make_shared<PhraseQuery::Builder>();
  expectThrows(invalid_argument::typeid, [&]() {
    builder->add(make_shared<Term>(L"field", L"two"), -42);
  });
}

void TestPhraseQuery::testBackwardPositions() 
{
  shared_ptr<PhraseQuery::Builder> builder =
      make_shared<PhraseQuery::Builder>();
  builder->add(make_shared<Term>(L"field", L"one"), 1);
  builder->add(make_shared<Term>(L"field", L"two"), 5);
  expectThrows(invalid_argument::typeid, [&]() {
    builder->add(make_shared<Term>(L"field", L"three"), 4);
  });
}
} // namespace org::apache::lucene::search