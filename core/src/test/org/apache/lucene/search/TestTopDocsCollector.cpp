using namespace std;

#include "TestTopDocsCollector.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

TestTopDocsCollector::MyTopsDocCollector::MyTopsDocCollector(int size)
    : TopDocsCollector<ScoreDoc>(new HitQueue(size, false))
{
}

shared_ptr<TopDocs> TestTopDocsCollector::MyTopsDocCollector::newTopDocs(
    std::deque<std::shared_ptr<ScoreDoc>> &results, int start)
{
  if (results.empty()) {
    return EMPTY_TOPDOCS;
  }

  float maxScore = NAN;
  if (start == 0) {
    maxScore = results[0]->score;
  } else {
    for (int i = pq->size(); i > 1; i--) {
      pq->pop();
    }
    maxScore = pq->pop()->score;
  }

  return make_shared<TopDocs>(totalHits, results, maxScore);
}

shared_ptr<LeafCollector>
TestTopDocsCollector::MyTopsDocCollector::getLeafCollector(
    shared_ptr<LeafReaderContext> context) 
{
  constexpr int base = context->docBase;
  return make_shared<LeafCollectorAnonymousInnerClass>(shared_from_this(),
                                                       base);
}

TestTopDocsCollector::MyTopsDocCollector::LeafCollectorAnonymousInnerClass::
    LeafCollectorAnonymousInnerClass(
        shared_ptr<MyTopsDocCollector> outerInstance, int base)
{
  this->outerInstance = outerInstance;
  this->base = base;
}

void TestTopDocsCollector::MyTopsDocCollector::
    LeafCollectorAnonymousInnerClass::collect(int doc)
{
  ++outerInstance->totalHits;
  outerInstance->pq->insertWithOverflow(
      make_shared<ScoreDoc>(doc + base, scores[outerInstance->idx++]));
}

void TestTopDocsCollector::MyTopsDocCollector::
    LeafCollectorAnonymousInnerClass::setScorer(shared_ptr<Scorer> scorer)
{
  // Don't do anything. Assign scores in random
}

bool TestTopDocsCollector::MyTopsDocCollector::needsScores() { return false; }

std::deque<float> const TestTopDocsCollector::scores = std::deque<float>{
    0.7767749f, 1.7839992f, 8.9925785f, 7.9608946f,  0.07948637f, 2.6356435f,
    7.4950366f, 7.1490803f, 8.108544f,  4.961808f,   2.2423935f,  7.285586f,
    4.6699767f, 2.9655676f, 6.953706f,  5.383931f,   6.9916306f,  8.365894f,
    7.888485f,  8.723962f,  3.1796896f, 0.39971232f, 1.3077754f,  6.8489285f,
    9.17561f,   5.060466f,  7.9793315f, 8.601509f,   4.1858315f,  0.28146625f};

shared_ptr<TopDocsCollector<std::shared_ptr<ScoreDoc>>>
TestTopDocsCollector::doSearch(int numResults) 
{
  shared_ptr<Query> q = make_shared<MatchAllDocsQuery>();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  shared_ptr<TopDocsCollector<std::shared_ptr<ScoreDoc>>> tdc =
      make_shared<MyTopsDocCollector>(numResults);
  searcher->search(q, tdc);
  return tdc;
}

void TestTopDocsCollector::setUp() 
{
  LuceneTestCase::setUp();

  // populate an index with 30 documents, this should be enough for the test.
  // The documents have no content - the test uses MatchAllDocsQuery().
  dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  for (int i = 0; i < 30; i++) {
    writer->addDocument(make_shared<Document>());
  }
  reader = writer->getReader();
  delete writer;
}

void TestTopDocsCollector::tearDown() 
{
  delete reader;
  delete dir;
  dir.reset();
  LuceneTestCase::tearDown();
}

void TestTopDocsCollector::testInvalidArguments() 
{
  int numResults = 5;
  shared_ptr<TopDocsCollector<std::shared_ptr<ScoreDoc>>> tdc =
      doSearch(numResults);

  // start < 0
  assertEquals(0, tdc->topDocs(-1)->scoreDocs.size());

  // start > pq.size()
  assertEquals(0, tdc->topDocs(numResults + 1)->scoreDocs.size());

  // start == pq.size()
  assertEquals(0, tdc->topDocs(numResults)->scoreDocs.size());

  // howMany < 0
  assertEquals(0, tdc->topDocs(0, -1)->scoreDocs.size());

  // howMany == 0
  assertEquals(0, tdc->topDocs(0, 0)->scoreDocs.size());
}

void TestTopDocsCollector::testZeroResults() 
{
  shared_ptr<TopDocsCollector<std::shared_ptr<ScoreDoc>>> tdc =
      make_shared<MyTopsDocCollector>(5);
  assertEquals(0, tdc->topDocs(0, 1)->scoreDocs.size());
}

void TestTopDocsCollector::testFirstResultsPage() 
{
  shared_ptr<TopDocsCollector<std::shared_ptr<ScoreDoc>>> tdc = doSearch(15);
  assertEquals(10, tdc->topDocs(0, 10)->scoreDocs.size());
}

void TestTopDocsCollector::testSecondResultsPages() 
{
  shared_ptr<TopDocsCollector<std::shared_ptr<ScoreDoc>>> tdc = doSearch(15);
  // ask for more results than are available
  assertEquals(5, tdc->topDocs(10, 10)->scoreDocs.size());

  // ask for 5 results (exactly what there should be
  tdc = doSearch(15);
  assertEquals(5, tdc->topDocs(10, 5)->scoreDocs.size());

  // ask for less results than there are
  tdc = doSearch(15);
  assertEquals(4, tdc->topDocs(10, 4)->scoreDocs.size());
}

void TestTopDocsCollector::testGetAllResults() 
{
  shared_ptr<TopDocsCollector<std::shared_ptr<ScoreDoc>>> tdc = doSearch(15);
  assertEquals(15, tdc->topDocs()->scoreDocs.size());
}

void TestTopDocsCollector::testGetResultsFromStart() 
{
  shared_ptr<TopDocsCollector<std::shared_ptr<ScoreDoc>>> tdc = doSearch(15);
  // should bring all results
  assertEquals(15, tdc->topDocs(0)->scoreDocs.size());

  tdc = doSearch(15);
  // get the last 5 only.
  assertEquals(5, tdc->topDocs(10)->scoreDocs.size());
}

void TestTopDocsCollector::testMaxScore() 
{
  // ask for all results
  shared_ptr<TopDocsCollector<std::shared_ptr<ScoreDoc>>> tdc = doSearch(15);
  shared_ptr<TopDocs> td = tdc->topDocs();
  assertEquals(MAX_SCORE, td->getMaxScore(), 0.0f);

  // ask for 5 last results
  tdc = doSearch(15);
  td = tdc->topDocs(10);
  assertEquals(MAX_SCORE, td->getMaxScore(), 0.0f);
}

void TestTopDocsCollector::testResultsOrder() 
{
  shared_ptr<TopDocsCollector<std::shared_ptr<ScoreDoc>>> tdc = doSearch(15);
  std::deque<std::shared_ptr<ScoreDoc>> sd = tdc->topDocs()->scoreDocs;

  assertEquals(MAX_SCORE, sd[0]->score, 0.0f);
  for (int i = 1; i < sd.size(); i++) {
    assertTrue(sd[i - 1]->score >= sd[i]->score);
  }
}
} // namespace org::apache::lucene::search