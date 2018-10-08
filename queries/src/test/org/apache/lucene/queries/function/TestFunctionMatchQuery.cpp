using namespace std;

#include "TestFunctionMatchQuery.h"

namespace org::apache::lucene::queries::function
{
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using QueryUtils = org::apache::lucene::search::QueryUtils;
using TopDocs = org::apache::lucene::search::TopDocs;
using org::junit::AfterClass;
using org::junit::BeforeClass;
shared_ptr<org::apache::lucene::index::IndexReader>
    TestFunctionMatchQuery::reader;
shared_ptr<org::apache::lucene::search::IndexSearcher>
    TestFunctionMatchQuery::searcher;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestFunctionMatchQuery::beforeClass() 
{
  createIndex(true);
  reader = DirectoryReader::open(dir);
  searcher = make_shared<IndexSearcher>(reader);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
void TestFunctionMatchQuery::afterClass() 
{
  delete reader;
}

void TestFunctionMatchQuery::testRangeMatching() 
{
  shared_ptr<DoubleValuesSource> in_ =
      DoubleValuesSource::fromFloatField(FLOAT_FIELD);
  shared_ptr<FunctionMatchQuery> fmq = make_shared<FunctionMatchQuery>(
      in_, [&](any d) { return d >= 2 && d < 4; });
  shared_ptr<TopDocs> docs = searcher->search(fmq, 10);

  assertEquals(2, docs->totalHits);
  assertEquals(9, docs->scoreDocs[0]->doc);
  assertEquals(13, docs->scoreDocs[1]->doc);

  QueryUtils::check(random(), fmq, searcher, rarely());
}
} // namespace org::apache::lucene::queries::function