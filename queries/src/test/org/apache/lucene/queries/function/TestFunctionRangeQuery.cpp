using namespace std;

#include "TestFunctionRangeQuery.h"

namespace org::apache::lucene::queries::function
{
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TopDocs = org::apache::lucene::search::TopDocs;
using org::junit::After;
using org::junit::Before;
using org::junit::BeforeClass;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestFunctionRangeQuery::beforeClass() 
{
  createIndex(true); // doMultiSegment
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Before protected void before() throws java.io.IOException
void TestFunctionRangeQuery::before() 
{
  indexReader = DirectoryReader::open(dir);
  indexSearcher = newSearcher(indexReader);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @After public void after() throws java.io.IOException
void TestFunctionRangeQuery::after()  { delete indexReader; }

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRangeInt() throws java.io.IOException
void TestFunctionRangeQuery::testRangeInt() 
{
  doTestRange(INT_VALUESOURCE);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRangeIntMultiValued() throws
// java.io.IOException
void TestFunctionRangeQuery::testRangeIntMultiValued() 
{
  doTestRange(INT_MV_MAX_VALUESOURCE);
  doTestRange(INT_MV_MIN_VALUESOURCE);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRangeFloat() throws java.io.IOException
void TestFunctionRangeQuery::testRangeFloat() 
{
  doTestRange(FLOAT_VALUESOURCE);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRangeFloatMultiValued() throws
// java.io.IOException
void TestFunctionRangeQuery::testRangeFloatMultiValued() 
{
  doTestRange(FLOAT_MV_MAX_VALUESOURCE);
  doTestRange(FLOAT_MV_MIN_VALUESOURCE);
}

void TestFunctionRangeQuery::doTestRange(
    shared_ptr<ValueSource> valueSource) 
{
  shared_ptr<Query> rangeQuery =
      make_shared<FunctionRangeQuery>(valueSource, 2, 4, true, false);
  std::deque<std::shared_ptr<ScoreDoc>> scoreDocs =
      indexSearcher->search(rangeQuery, N_DOCS)->scoreDocs;
  expectScores(scoreDocs, {3, 2});

  rangeQuery = make_shared<FunctionRangeQuery>(valueSource, 2, 4, false, true);
  scoreDocs = indexSearcher->search(rangeQuery, N_DOCS)->scoreDocs;
  expectScores(scoreDocs, {4, 3});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testDeleted() throws java.io.IOException
void TestFunctionRangeQuery::testDeleted() 
{
  doTestDeleted(INT_VALUESOURCE);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testDeletedMultiValued() throws
// java.io.IOException
void TestFunctionRangeQuery::testDeletedMultiValued() 
{
  doTestDeleted(INT_MV_MAX_VALUESOURCE);
  doTestDeleted(INT_MV_MIN_VALUESOURCE);
}

void TestFunctionRangeQuery::doTestDeleted(
    shared_ptr<ValueSource> valueSource) 
{
  // We delete doc with #3. Note we don't commit it to disk; we search using a
  // near real-time reader.
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
  try {
    writer->deleteDocuments({make_shared<FunctionRangeQuery>(
        valueSource, 3, 3, true, true)}); // delete the one with #3
    assert(writer->hasDeletions());
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.index.IndexReader
    // indexReader2 = org.apache.lucene.index.DirectoryReader.open(writer))
    {
      org::apache::lucene::index::IndexReader indexReader2 =
          org::apache::lucene::index::DirectoryReader::open(writer);
      shared_ptr<IndexSearcher> indexSearcher2 =
          make_shared<IndexSearcher>(indexReader2);
      shared_ptr<TopDocs> topDocs = indexSearcher2->search(
          make_shared<FunctionRangeQuery>(valueSource, 3, 4, true, true),
          N_DOCS);
      expectScores(topDocs->scoreDocs, {4}); // missing #3 because it's deleted
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    writer->rollback();
    delete writer;
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testExplain() throws java.io.IOException
void TestFunctionRangeQuery::testExplain() 
{
  shared_ptr<Query> rangeQuery =
      make_shared<FunctionRangeQuery>(INT_VALUESOURCE, 2, 2, true, true);
  std::deque<std::shared_ptr<ScoreDoc>> scoreDocs =
      indexSearcher->search(rangeQuery, N_DOCS)->scoreDocs;
  shared_ptr<Explanation> explain =
      indexSearcher->explain(rangeQuery, scoreDocs[0]->doc);
  // Just validate it looks reasonable
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"2.0 = frange(int(" + INT_FIELD + L")):[2 TO 2]\n" +
                   L"  2.0 = int(" + INT_FIELD + L")=2\n",
               explain->toString());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testExplainMultiValued() throws
// java.io.IOException
void TestFunctionRangeQuery::testExplainMultiValued() 
{
  shared_ptr<Query> rangeQuery =
      make_shared<FunctionRangeQuery>(INT_MV_MIN_VALUESOURCE, 2, 2, true, true);
  std::deque<std::shared_ptr<ScoreDoc>> scoreDocs =
      indexSearcher->search(rangeQuery, N_DOCS)->scoreDocs;
  shared_ptr<Explanation> explain =
      indexSearcher->explain(rangeQuery, scoreDocs[0]->doc);
  // Just validate it looks reasonable
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"2.0 = frange(int(" + INT_FIELD_MV_MIN + L",MIN)):[2 TO 2]\n" +
                   L"  2.0 = int(" + INT_FIELD_MV_MIN + L",MIN)=2\n",
               explain->toString());
}

void TestFunctionRangeQuery::expectScores(
    std::deque<std::shared_ptr<ScoreDoc>> &scoreDocs, deque<int> &docScores)
{
  assertEquals(docScores->length, scoreDocs.size());
  for (int i = 0; i < docScores->length; i++) {
    assertEquals(docScores[i], scoreDocs[i]->score, 0.0);
  }
}
} // namespace org::apache::lucene::queries::function