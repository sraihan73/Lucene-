using namespace std;

#include "TestLongNormValueSource.h"

namespace org::apache::lucene::queries::function
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using TextField = org::apache::lucene::document::TextField;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using NormValueSource =
    org::apache::lucene::queries::function::valuesource::NormValueSource;
using CheckHits = org::apache::lucene::search::CheckHits;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using TopDocs = org::apache::lucene::search::TopDocs;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::AfterClass;
using org::junit::BeforeClass;
shared_ptr<org::apache::lucene::store::Directory> TestLongNormValueSource::dir;
shared_ptr<org::apache::lucene::index::IndexReader>
    TestLongNormValueSource::reader;
shared_ptr<org::apache::lucene::search::IndexSearcher>
    TestLongNormValueSource::searcher;
shared_ptr<org::apache::lucene::analysis::Analyzer>
    TestLongNormValueSource::analyzer;
shared_ptr<org::apache::lucene::search::similarities::Similarity>
    TestLongNormValueSource::sim = make_shared<
        org::apache::lucene::search::similarities::ClassicSimilarity>();

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestLongNormValueSource::beforeClass() 
{
  dir = newDirectory();
  analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwConfig = newIndexWriterConfig(analyzer);
  iwConfig->setMergePolicy(newLogMergePolicy());
  iwConfig->setSimilarity(sim);
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, iwConfig);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"text", L"this is a test test test",
                                        Field::Store::NO));
  iw->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(
      make_shared<TextField>(L"text", L"second test", Field::Store::NO));
  iw->addDocument(doc);

  reader = iw->getReader();
  searcher = newSearcher(reader);
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
void TestLongNormValueSource::afterClass() 
{
  searcher.reset();
  delete reader;
  reader.reset();
  delete dir;
  dir.reset();
  delete analyzer;
  analyzer.reset();
}

void TestLongNormValueSource::testNorm() 
{
  shared_ptr<Similarity> saved = searcher->getSimilarity(true);
  try {
    // no norm field (so agnostic to indexed similarity)
    searcher->setSimilarity(sim);
    assertHits(
        make_shared<FunctionQuery>(make_shared<NormValueSource>(L"text")),
        std::deque<float>{0.0f, 0.0f});
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    searcher->setSimilarity(saved);
  }
}

void TestLongNormValueSource::assertHits(
    shared_ptr<Query> q, std::deque<float> &scores) 
{
  std::deque<std::shared_ptr<ScoreDoc>> expected(scores.size());
  std::deque<int> expectedDocs(scores.size());
  for (int i = 0; i < expected.size(); i++) {
    expectedDocs[i] = i;
    expected[i] = make_shared<ScoreDoc>(i, scores[i]);
  }
  shared_ptr<TopDocs> docs =
      searcher->search(q, 2,
                       make_shared<Sort>(make_shared<SortField>(
                           L"id", SortField::Type::STRING)));

  /*
  for (int i=0;i<docs.scoreDocs.length;i++) {
    System.out.println(searcher.explain(q, docs.scoreDocs[i].doc));
  }
  */

  CheckHits::checkHits(random(), q, L"", searcher, expectedDocs);
  CheckHits::checkHitsQuery(q, expected, docs->scoreDocs, expectedDocs);
  CheckHits::checkExplanations(q, L"", searcher);
}
} // namespace org::apache::lucene::queries::function