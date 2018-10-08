using namespace std;

#include "TestPositiveScoresOnlyCollector.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

TestPositiveScoresOnlyCollector::SimpleScorer::SimpleScorer(
    shared_ptr<Weight> weight)
    : Scorer(weight)
{
}

float TestPositiveScoresOnlyCollector::SimpleScorer::score()
{
  return idx == scores.size() ? NAN : scores[idx];
}

int TestPositiveScoresOnlyCollector::SimpleScorer::docID() { return idx; }

shared_ptr<DocIdSetIterator>
TestPositiveScoresOnlyCollector::SimpleScorer::iterator()
{
  return make_shared<DocIdSetIteratorAnonymousInnerClass>(shared_from_this());
}

TestPositiveScoresOnlyCollector::SimpleScorer::
    DocIdSetIteratorAnonymousInnerClass::DocIdSetIteratorAnonymousInnerClass(
        shared_ptr<SimpleScorer> outerInstance)
{
  this->outerInstance = outerInstance;
}

int TestPositiveScoresOnlyCollector::SimpleScorer::
    DocIdSetIteratorAnonymousInnerClass::docID()
{
  return outerInstance->idx;
}

int TestPositiveScoresOnlyCollector::SimpleScorer::
    DocIdSetIteratorAnonymousInnerClass::nextDoc()
{
  return ++outerInstance->idx != scores->size() ? outerInstance->idx
                                                : NO_MORE_DOCS;
}

int TestPositiveScoresOnlyCollector::SimpleScorer::
    DocIdSetIteratorAnonymousInnerClass::advance(int target)
{
  outerInstance->idx = target;
  return outerInstance->idx < scores->size() ? outerInstance->idx
                                             : NO_MORE_DOCS;
}

int64_t TestPositiveScoresOnlyCollector::SimpleScorer::
    DocIdSetIteratorAnonymousInnerClass::cost()
{
  return scores->size();
}

std::deque<float> const TestPositiveScoresOnlyCollector::scores =
    std::deque<float>{0.7767749f,   -1.7839992f, 8.9925785f, 7.9608946f,
                       -0.07948637f, 2.6356435f,  7.4950366f, 7.1490803f,
                       -8.108544f,   4.961808f,   2.2423935f, -7.285586f,
                       4.6699767f};

void TestPositiveScoresOnlyCollector::testNegativeScores() 
{

  // The Top*Collectors previously filtered out documents with <= scores. This
  // behavior has changed. This test checks that if PositiveOnlyScoresFilter
  // wraps one of these collectors, documents with <= 0 scores are indeed
  // filtered.

  int numPositiveScores = 0;
  for (int i = 0; i < scores.size(); i++) {
    if (scores[i] > 0) {
      ++numPositiveScores;
    }
  }

  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), directory);
  writer->addDocument(make_shared<Document>());
  writer->commit();
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;
  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<Weight> fake =
      (make_shared<TermQuery>(make_shared<Term>(L"fake", L"weight")))
          ->createWeight(searcher, true, 1.0f);
  shared_ptr<Scorer> s = make_shared<SimpleScorer>(fake);
  shared_ptr<TopDocsCollector<std::shared_ptr<ScoreDoc>>> tdc =
      TopScoreDocCollector::create(scores.size());
  shared_ptr<Collector> c = make_shared<PositiveScoresOnlyCollector>(tdc);
  shared_ptr<LeafCollector> ac = c->getLeafCollector(ir->leaves()[0]);
  ac->setScorer(s);
  while (s->begin().nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
    ac->collect(0);
  }
  shared_ptr<TopDocs> td = tdc->topDocs();
  std::deque<std::shared_ptr<ScoreDoc>> sd = td->scoreDocs;
  assertEquals(numPositiveScores, td->totalHits);
  for (int i = 0; i < sd.size(); i++) {
    assertTrue(L"only positive scores should return: " +
                   to_wstring(sd[i]->score),
               sd[i]->score > 0);
  }
  delete ir;
  delete directory;
}
} // namespace org::apache::lucene::search