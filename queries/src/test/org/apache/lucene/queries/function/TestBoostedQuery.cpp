using namespace std;

#include "TestBoostedQuery.h"

namespace org::apache::lucene::queries::function
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using ConstValueSource =
    org::apache::lucene::queries::function::valuesource::ConstValueSource;
using CheckHits = org::apache::lucene::search::CheckHits;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::AfterClass;
using org::junit::BeforeClass;
shared_ptr<org::apache::lucene::store::Directory> TestBoostedQuery::dir;
shared_ptr<org::apache::lucene::index::IndexReader> TestBoostedQuery::ir;
shared_ptr<org::apache::lucene::search::IndexSearcher> TestBoostedQuery::is;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestBoostedQuery::beforeClass() 
{
  dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwConfig =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwConfig->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, iwConfig);
  shared_ptr<Document> document = make_shared<Document>();
  shared_ptr<Field> idField =
      make_shared<SortedDocValuesField>(L"id", make_shared<BytesRef>());
  document->push_back(idField);
  iw->addDocument(document);
  ir = iw->getReader();
  is = newSearcher(ir);
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
void TestBoostedQuery::afterClass() 
{
  is.reset();
  delete ir;
  ir.reset();
  delete dir;
  dir.reset();
}

void TestBoostedQuery::testBasic() 
{
  shared_ptr<Query> q = make_shared<MatchAllDocsQuery>();
  shared_ptr<TopDocs> docs = is->search(q, 10);
  assertEquals(1, docs->totalHits);
  float score = docs->scoreDocs[0]->score;

  shared_ptr<Query> boostedQ =
      make_shared<BoostedQuery>(q, make_shared<ConstValueSource>(2.0f));
  assertHits(boostedQ, std::deque<float>{score * 2});
}

void TestBoostedQuery::assertHits(
    shared_ptr<Query> q, std::deque<float> &scores) 
{
  std::deque<std::shared_ptr<ScoreDoc>> expected(scores.size());
  std::deque<int> expectedDocs(scores.size());
  for (int i = 0; i < expected.size(); i++) {
    expectedDocs[i] = i;
    expected[i] = make_shared<ScoreDoc>(i, scores[i]);
  }
  shared_ptr<TopDocs> docs =
      is->search(q, 10,
                 make_shared<Sort>(
                     make_shared<SortField>(L"id", SortField::Type::STRING)));
  CheckHits::checkHits(random(), q, L"", is, expectedDocs);
  CheckHits::checkHitsQuery(q, expected, docs->scoreDocs, expectedDocs);
  CheckHits::checkExplanations(q, L"", is);
}
} // namespace org::apache::lucene::queries::function