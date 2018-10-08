using namespace std;

#include "TestMultiTermConstantScore.h"

namespace org::apache::lucene::search
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using junit::framework::Assert;
using org::junit::AfterClass;
using org::junit::BeforeClass;
using org::junit::Test;
shared_ptr<org::apache::lucene::store::Directory>
    TestMultiTermConstantScore::small;
shared_ptr<org::apache::lucene::index::IndexReader>
    TestMultiTermConstantScore::reader;

void TestMultiTermConstantScore::assertEquals(const wstring &m, int e, int a)
{
  Assert::assertEquals(m, e, a);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestMultiTermConstantScore::beforeClass() 
{
  std::deque<wstring> data = {L"A 1 2 3 4 5 6", L"Z       4 5 6", L"",
                               L"B   2   4 5 6", L"Y     3   5 6", L"",
                               L"C     3     6", L"X       4 5 6"};

  small = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), small,
      newIndexWriterConfig(
          make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false))
          ->setMergePolicy(newLogMergePolicy()));

  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType->setTokenized(false);
  for (int i = 0; i < data.size(); i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newField(L"id", to_wstring(i),
                 customType)); // Field.Keyword("id",std::wstring.valueOf(i)));
    doc->push_back(
        newField(L"all", L"all", customType)); // Field.Keyword("all","all"));
    if (L"" != data[i]) {
      doc->push_back(newTextField(
          L"data", data[i], Field::Store::YES)); // Field.Text("data",data[i]));
    }
    writer->addDocument(doc);
  }

  reader = writer->getReader();
  delete writer;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
void TestMultiTermConstantScore::afterClass() 
{
  delete reader;
  delete small;
  reader.reset();
  small.reset();
}

shared_ptr<Query> TestMultiTermConstantScore::csrq(const wstring &f,
                                                   const wstring &l,
                                                   const wstring &h, bool il,
                                                   bool ih)
{
  shared_ptr<TermRangeQuery> query =
      TermRangeQuery::newStringRange(f, l, h, il, ih);
  query->setRewriteMethod(MultiTermQuery::CONSTANT_SCORE_REWRITE);
  if (VERBOSE) {
    wcout << L"TEST: query=" << query << endl;
  }
  return query;
}

shared_ptr<Query> TestMultiTermConstantScore::csrq(
    const wstring &f, const wstring &l, const wstring &h, bool il, bool ih,
    shared_ptr<MultiTermQuery::RewriteMethod> method)
{
  shared_ptr<TermRangeQuery> query =
      TermRangeQuery::newStringRange(f, l, h, il, ih);
  query->setRewriteMethod(method);
  if (VERBOSE) {
    wcout << L"TEST: query=" << query << L" method=" << method << endl;
  }
  return query;
}

shared_ptr<Query> TestMultiTermConstantScore::cspq(shared_ptr<Term> prefix)
{
  shared_ptr<PrefixQuery> query = make_shared<PrefixQuery>(prefix);
  query->setRewriteMethod(MultiTermQuery::CONSTANT_SCORE_REWRITE);
  return query;
}

shared_ptr<Query> TestMultiTermConstantScore::cswcq(shared_ptr<Term> wild)
{
  shared_ptr<WildcardQuery> query = make_shared<WildcardQuery>(wild);
  query->setRewriteMethod(MultiTermQuery::CONSTANT_SCORE_REWRITE);
  return query;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testBasics() throws java.io.IOException
void TestMultiTermConstantScore::testBasics() 
{
  QueryUtils::check(csrq(L"data", L"1", L"6", T, T));
  QueryUtils::check(csrq(L"data", L"A", L"Z", T, T));
  QueryUtils::checkUnequal(csrq(L"data", L"1", L"6", T, T),
                           csrq(L"data", L"A", L"Z", T, T));

  QueryUtils::check(cspq(make_shared<Term>(L"data", L"p*u?")));
  QueryUtils::checkUnequal(cspq(make_shared<Term>(L"data", L"pre*")),
                           cspq(make_shared<Term>(L"data", L"pres*")));

  QueryUtils::check(cswcq(make_shared<Term>(L"data", L"p")));
  QueryUtils::checkUnequal(cswcq(make_shared<Term>(L"data", L"pre*n?t")),
                           cswcq(make_shared<Term>(L"data", L"pr*t?j")));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testEqualScores() throws java.io.IOException
void TestMultiTermConstantScore::testEqualScores() 
{
  // NOTE: uses index build in *this* setUp

  shared_ptr<IndexSearcher> search = newSearcher(reader);

  std::deque<std::shared_ptr<ScoreDoc>> result;

  // some hits match more terms then others, score should be the same

  result = search->search(csrq(L"data", L"1", L"6", T, T), 1000).scoreDocs;
  int numHits = result.size();
  assertEquals(L"wrong number of results", 6, numHits);
  float score = result[0]->score;
  for (int i = 1; i < numHits; i++) {
    assertEquals(L"score for " + to_wstring(i) + L" was not the same", score,
                 result[i]->score, SCORE_COMP_THRESH);
  }

  result = search
               ->search(csrq(L"data", L"1", L"6", T, T,
                             MultiTermQuery::CONSTANT_SCORE_BOOLEAN_REWRITE),
                        1000)
               .scoreDocs;
  numHits = result.size();
  assertEquals(L"wrong number of results", 6, numHits);
  for (int i = 0; i < numHits; i++) {
    assertEquals(L"score for " + to_wstring(i) + L" was not the same", score,
                 result[i]->score, SCORE_COMP_THRESH);
  }

  result = search
               ->search(csrq(L"data", L"1", L"6", T, T,
                             MultiTermQuery::CONSTANT_SCORE_REWRITE),
                        1000)
               .scoreDocs;
  numHits = result.size();
  assertEquals(L"wrong number of results", 6, numHits);
  for (int i = 0; i < numHits; i++) {
    assertEquals(L"score for " + to_wstring(i) + L" was not the same", score,
                 result[i]->score, SCORE_COMP_THRESH);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testEqualScoresWhenNoHits() throws
// java.io.IOException
void TestMultiTermConstantScore::testEqualScoresWhenNoHits() 
{
  // NOTE: uses index build in *this* setUp

  shared_ptr<IndexSearcher> search = newSearcher(reader);

  std::deque<std::shared_ptr<ScoreDoc>> result;

  shared_ptr<TermQuery> dummyTerm =
      make_shared<TermQuery>(make_shared<Term>(L"data", L"1"));

  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(dummyTerm, BooleanClause::Occur::SHOULD); // hits one doc
  bq->add(csrq(L"data", L"#", L"#", T, T),
          BooleanClause::Occur::SHOULD); // hits no docs
  result = search->search(bq->build(), 1000).scoreDocs;
  int numHits = result.size();
  assertEquals(L"wrong number of results", 1, numHits);
  float score = result[0]->score;
  for (int i = 1; i < numHits; i++) {
    assertEquals(L"score for " + to_wstring(i) + L" was not the same", score,
                 result[i]->score, SCORE_COMP_THRESH);
  }

  bq = make_shared<BooleanQuery::Builder>();
  bq->add(dummyTerm, BooleanClause::Occur::SHOULD); // hits one doc
  bq->add(csrq(L"data", L"#", L"#", T, T,
               MultiTermQuery::CONSTANT_SCORE_BOOLEAN_REWRITE),
          BooleanClause::Occur::SHOULD); // hits no docs
  result = search->search(bq->build(), 1000).scoreDocs;
  numHits = result.size();
  assertEquals(L"wrong number of results", 1, numHits);
  for (int i = 0; i < numHits; i++) {
    assertEquals(L"score for " + to_wstring(i) + L" was not the same", score,
                 result[i]->score, SCORE_COMP_THRESH);
  }

  bq = make_shared<BooleanQuery::Builder>();
  bq->add(dummyTerm, BooleanClause::Occur::SHOULD); // hits one doc
  bq->add(
      csrq(L"data", L"#", L"#", T, T, MultiTermQuery::CONSTANT_SCORE_REWRITE),
      BooleanClause::Occur::SHOULD); // hits no docs
  result = search->search(bq->build(), 1000).scoreDocs;
  numHits = result.size();
  assertEquals(L"wrong number of results", 1, numHits);
  for (int i = 0; i < numHits; i++) {
    assertEquals(L"score for " + to_wstring(i) + L" was not the same", score,
                 result[i]->score, SCORE_COMP_THRESH);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testBooleanOrderUnAffected() throws
// java.io.IOException
void TestMultiTermConstantScore::testBooleanOrderUnAffected() 
{
  // NOTE: uses index build in *this* setUp

  shared_ptr<IndexSearcher> search = newSearcher(reader);

  // first do a regular TermRangeQuery which uses term expansion so
  // docs with more terms in range get higher scores

  shared_ptr<Query> rq =
      TermRangeQuery::newStringRange(L"data", L"1", L"4", T, T);

  std::deque<std::shared_ptr<ScoreDoc>> expected =
      search->search(rq, 1000).scoreDocs;
  int numHits = expected.size();

  // now do a bool where which also contains a
  // ConstantScoreRangeQuery and make sure hte order is the same

  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  q->add(rq, BooleanClause::Occur::MUST);                              // T, F);
  q->add(csrq(L"data", L"1", L"6", T, T), BooleanClause::Occur::MUST); // T, F);

  std::deque<std::shared_ptr<ScoreDoc>> actual =
      search->search(q->build(), 1000).scoreDocs;

  assertEquals(L"wrong numebr of hits", numHits, actual.size());
  for (int i = 0; i < numHits; i++) {
    assertEquals(L"mismatch in docid for hit#" + to_wstring(i),
                 expected[i]->doc, actual[i]->doc);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRangeQueryId() throws
// java.io.IOException
void TestMultiTermConstantScore::testRangeQueryId() 
{
  // NOTE: uses index build in *super* setUp

  shared_ptr<IndexReader> reader = signedIndexReader;
  shared_ptr<IndexSearcher> search = newSearcher(reader);

  if (VERBOSE) {
    wcout << L"TEST: reader=" << reader << endl;
  }

  int medId = ((maxId - minId) / 2);

  wstring minIP = pad(minId);
  wstring maxIP = pad(maxId);
  wstring medIP = pad(medId);

  int numDocs = reader->numDocs();

  assertEquals(L"num of docs", numDocs, 1 + maxId - minId);

  std::deque<std::shared_ptr<ScoreDoc>> result;

  // test id, bounded on both ends

  result = search->search(csrq(L"id", minIP, maxIP, T, T), numDocs).scoreDocs;
  assertEquals(L"find all", numDocs, result.size());

  result = search
               ->search(csrq(L"id", minIP, maxIP, T, T,
                             MultiTermQuery::CONSTANT_SCORE_REWRITE),
                        numDocs)
               .scoreDocs;
  assertEquals(L"find all", numDocs, result.size());

  result = search->search(csrq(L"id", minIP, maxIP, T, F), numDocs).scoreDocs;
  assertEquals(L"all but last", numDocs - 1, result.size());

  result = search
               ->search(csrq(L"id", minIP, maxIP, T, F,
                             MultiTermQuery::CONSTANT_SCORE_REWRITE),
                        numDocs)
               .scoreDocs;
  assertEquals(L"all but last", numDocs - 1, result.size());

  result = search->search(csrq(L"id", minIP, maxIP, F, T), numDocs).scoreDocs;
  assertEquals(L"all but first", numDocs - 1, result.size());

  result = search
               ->search(csrq(L"id", minIP, maxIP, F, T,
                             MultiTermQuery::CONSTANT_SCORE_REWRITE),
                        numDocs)
               .scoreDocs;
  assertEquals(L"all but first", numDocs - 1, result.size());

  result = search->search(csrq(L"id", minIP, maxIP, F, F), numDocs).scoreDocs;
  assertEquals(L"all but ends", numDocs - 2, result.size());

  result = search
               ->search(csrq(L"id", minIP, maxIP, F, F,
                             MultiTermQuery::CONSTANT_SCORE_REWRITE),
                        numDocs)
               .scoreDocs;
  assertEquals(L"all but ends", numDocs - 2, result.size());

  result = search->search(csrq(L"id", medIP, maxIP, T, T), numDocs).scoreDocs;
  assertEquals(L"med and up", 1 + maxId - medId, result.size());

  result = search
               ->search(csrq(L"id", medIP, maxIP, T, T,
                             MultiTermQuery::CONSTANT_SCORE_REWRITE),
                        numDocs)
               .scoreDocs;
  assertEquals(L"med and up", 1 + maxId - medId, result.size());

  result = search->search(csrq(L"id", minIP, medIP, T, T), numDocs).scoreDocs;
  assertEquals(L"up to med", 1 + medId - minId, result.size());

  result = search
               ->search(csrq(L"id", minIP, medIP, T, T,
                             MultiTermQuery::CONSTANT_SCORE_REWRITE),
                        numDocs)
               .scoreDocs;
  assertEquals(L"up to med", 1 + medId - minId, result.size());

  // unbounded id

  result = search->search(csrq(L"id", minIP, L"", T, F), numDocs).scoreDocs;
  assertEquals(L"min and up", numDocs, result.size());

  result = search->search(csrq(L"id", L"", maxIP, F, T), numDocs).scoreDocs;
  assertEquals(L"max and down", numDocs, result.size());

  result = search->search(csrq(L"id", minIP, L"", F, F), numDocs).scoreDocs;
  assertEquals(L"not min, but up", numDocs - 1, result.size());

  result = search->search(csrq(L"id", L"", maxIP, F, F), numDocs).scoreDocs;
  assertEquals(L"not max, but down", numDocs - 1, result.size());

  result = search->search(csrq(L"id", medIP, maxIP, T, F), numDocs).scoreDocs;
  assertEquals(L"med and up, not max", maxId - medId, result.size());

  result = search->search(csrq(L"id", minIP, medIP, F, T), numDocs).scoreDocs;
  assertEquals(L"not min, up to med", medId - minId, result.size());

  // very small sets

  result = search->search(csrq(L"id", minIP, minIP, F, F), numDocs).scoreDocs;
  assertEquals(L"min,min,F,F", 0, result.size());

  result = search
               ->search(csrq(L"id", minIP, minIP, F, F,
                             MultiTermQuery::CONSTANT_SCORE_REWRITE),
                        numDocs)
               .scoreDocs;
  assertEquals(L"min,min,F,F", 0, result.size());

  result = search->search(csrq(L"id", medIP, medIP, F, F), numDocs).scoreDocs;
  assertEquals(L"med,med,F,F", 0, result.size());

  result = search
               ->search(csrq(L"id", medIP, medIP, F, F,
                             MultiTermQuery::CONSTANT_SCORE_REWRITE),
                        numDocs)
               .scoreDocs;
  assertEquals(L"med,med,F,F", 0, result.size());

  result = search->search(csrq(L"id", maxIP, maxIP, F, F), numDocs).scoreDocs;
  assertEquals(L"max,max,F,F", 0, result.size());

  result = search
               ->search(csrq(L"id", maxIP, maxIP, F, F,
                             MultiTermQuery::CONSTANT_SCORE_REWRITE),
                        numDocs)
               .scoreDocs;
  assertEquals(L"max,max,F,F", 0, result.size());

  result = search->search(csrq(L"id", minIP, minIP, T, T), numDocs).scoreDocs;
  assertEquals(L"min,min,T,T", 1, result.size());

  result = search
               ->search(csrq(L"id", minIP, minIP, T, T,
                             MultiTermQuery::CONSTANT_SCORE_REWRITE),
                        numDocs)
               .scoreDocs;
  assertEquals(L"min,min,T,T", 1, result.size());

  result = search->search(csrq(L"id", L"", minIP, F, T), numDocs).scoreDocs;
  assertEquals(L"nul,min,F,T", 1, result.size());

  result = search
               ->search(csrq(L"id", L"", minIP, F, T,
                             MultiTermQuery::CONSTANT_SCORE_REWRITE),
                        numDocs)
               .scoreDocs;
  assertEquals(L"nul,min,F,T", 1, result.size());

  result = search->search(csrq(L"id", maxIP, maxIP, T, T), numDocs).scoreDocs;
  assertEquals(L"max,max,T,T", 1, result.size());

  result = search
               ->search(csrq(L"id", maxIP, maxIP, T, T,
                             MultiTermQuery::CONSTANT_SCORE_REWRITE),
                        numDocs)
               .scoreDocs;
  assertEquals(L"max,max,T,T", 1, result.size());

  result = search->search(csrq(L"id", maxIP, L"", T, F), numDocs).scoreDocs;
  assertEquals(L"max,nul,T,T", 1, result.size());

  result = search
               ->search(csrq(L"id", maxIP, L"", T, F,
                             MultiTermQuery::CONSTANT_SCORE_REWRITE),
                        numDocs)
               .scoreDocs;
  assertEquals(L"max,nul,T,T", 1, result.size());

  result = search->search(csrq(L"id", medIP, medIP, T, T), numDocs).scoreDocs;
  assertEquals(L"med,med,T,T", 1, result.size());

  result = search
               ->search(csrq(L"id", medIP, medIP, T, T,
                             MultiTermQuery::CONSTANT_SCORE_REWRITE),
                        numDocs)
               .scoreDocs;
  assertEquals(L"med,med,T,T", 1, result.size());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRangeQueryRand() throws
// java.io.IOException
void TestMultiTermConstantScore::testRangeQueryRand() 
{
  // NOTE: uses index build in *super* setUp

  shared_ptr<IndexReader> reader = signedIndexReader;
  shared_ptr<IndexSearcher> search = newSearcher(reader);

  wstring minRP = pad(signedIndexDir->minR);
  wstring maxRP = pad(signedIndexDir->maxR);

  int numDocs = reader->numDocs();

  assertEquals(L"num of docs", numDocs, 1 + maxId - minId);

  std::deque<std::shared_ptr<ScoreDoc>> result;

  // test extremes, bounded on both ends

  result = search->search(csrq(L"rand", minRP, maxRP, T, T), numDocs).scoreDocs;
  assertEquals(L"find all", numDocs, result.size());

  result = search->search(csrq(L"rand", minRP, maxRP, T, F), numDocs).scoreDocs;
  assertEquals(L"all but biggest", numDocs - 1, result.size());

  result = search->search(csrq(L"rand", minRP, maxRP, F, T), numDocs).scoreDocs;
  assertEquals(L"all but smallest", numDocs - 1, result.size());

  result = search->search(csrq(L"rand", minRP, maxRP, F, F), numDocs).scoreDocs;
  assertEquals(L"all but extremes", numDocs - 2, result.size());

  // unbounded

  result = search->search(csrq(L"rand", minRP, L"", T, F), numDocs).scoreDocs;
  assertEquals(L"smallest and up", numDocs, result.size());

  result = search->search(csrq(L"rand", L"", maxRP, F, T), numDocs).scoreDocs;
  assertEquals(L"biggest and down", numDocs, result.size());

  result = search->search(csrq(L"rand", minRP, L"", F, F), numDocs).scoreDocs;
  assertEquals(L"not smallest, but up", numDocs - 1, result.size());

  result = search->search(csrq(L"rand", L"", maxRP, F, F), numDocs).scoreDocs;
  assertEquals(L"not biggest, but down", numDocs - 1, result.size());

  // very small sets

  result = search->search(csrq(L"rand", minRP, minRP, F, F), numDocs).scoreDocs;
  assertEquals(L"min,min,F,F", 0, result.size());
  result = search->search(csrq(L"rand", maxRP, maxRP, F, F), numDocs).scoreDocs;
  assertEquals(L"max,max,F,F", 0, result.size());

  result = search->search(csrq(L"rand", minRP, minRP, T, T), numDocs).scoreDocs;
  assertEquals(L"min,min,T,T", 1, result.size());
  result = search->search(csrq(L"rand", L"", minRP, F, T), numDocs).scoreDocs;
  assertEquals(L"nul,min,F,T", 1, result.size());

  result = search->search(csrq(L"rand", maxRP, maxRP, T, T), numDocs).scoreDocs;
  assertEquals(L"max,max,T,T", 1, result.size());
  result = search->search(csrq(L"rand", maxRP, L"", T, F), numDocs).scoreDocs;
  assertEquals(L"max,nul,T,T", 1, result.size());
}
} // namespace org::apache::lucene::search