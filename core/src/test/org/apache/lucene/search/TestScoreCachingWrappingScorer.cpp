using namespace std;

#include "TestScoreCachingWrappingScorer.h"

namespace org::apache::lucene::search
{
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

TestScoreCachingWrappingScorer::SimpleScorer::SimpleScorer(
    shared_ptr<Weight> weight)
    : Scorer(weight)
{
}

float TestScoreCachingWrappingScorer::SimpleScorer::score()
{
  // advance idx on purpose, so that consecutive calls to score will get
  // different results. This is to emulate computation of a score. If
  // ScoreCachingWrappingScorer is used, this should not be called more than
  // once per document.
  return idx == scores.size() ? NAN : scores[idx++];
}

int TestScoreCachingWrappingScorer::SimpleScorer::docID() { return doc; }

shared_ptr<DocIdSetIterator>
TestScoreCachingWrappingScorer::SimpleScorer::iterator()
{
  return make_shared<DocIdSetIteratorAnonymousInnerClass>(shared_from_this());
}

TestScoreCachingWrappingScorer::SimpleScorer::
    DocIdSetIteratorAnonymousInnerClass::DocIdSetIteratorAnonymousInnerClass(
        shared_ptr<SimpleScorer> outerInstance)
{
  this->outerInstance = outerInstance;
}

int TestScoreCachingWrappingScorer::SimpleScorer::
    DocIdSetIteratorAnonymousInnerClass::docID()
{
  return outerInstance->doc;
}

int TestScoreCachingWrappingScorer::SimpleScorer::
    DocIdSetIteratorAnonymousInnerClass::nextDoc()
{
  return ++outerInstance->doc < scores->size() ? outerInstance->doc
                                               : NO_MORE_DOCS;
}

int TestScoreCachingWrappingScorer::SimpleScorer::
    DocIdSetIteratorAnonymousInnerClass::advance(int target)
{
  outerInstance->doc = target;
  return outerInstance->doc < scores->size() ? outerInstance->doc
                                             : NO_MORE_DOCS;
}

int64_t TestScoreCachingWrappingScorer::SimpleScorer::
    DocIdSetIteratorAnonymousInnerClass::cost()
{
  return scores->size();
}

TestScoreCachingWrappingScorer::ScoreCachingCollector::ScoreCachingCollector(
    int numToCollect)
{
  mscores = std::deque<float>(numToCollect);
}

void TestScoreCachingWrappingScorer::ScoreCachingCollector::collect(
    int doc) 
{
  // just a sanity check to avoid IOOB.
  if (idx == mscores.size()) {
    return;
  }

  // just call score() a couple of times and record the score.
  mscores[idx] = scorer->score();
  mscores[idx] = scorer->score();
  mscores[idx] = scorer->score();
  ++idx;
}

void TestScoreCachingWrappingScorer::ScoreCachingCollector::setScorer(
    shared_ptr<Scorer> scorer)
{
  this->scorer = make_shared<ScoreCachingWrappingScorer>(scorer);
}

bool TestScoreCachingWrappingScorer::ScoreCachingCollector::needsScores()
{
  return true;
}

std::deque<float> const TestScoreCachingWrappingScorer::scores =
    std::deque<float>{0.7767749f,  1.7839992f, 8.9925785f, 7.9608946f,
                       0.07948637f, 2.6356435f, 7.4950366f, 7.1490803f,
                       8.108544f,   4.961808f,  2.2423935f, 7.285586f,
                       4.6699767f};

void TestScoreCachingWrappingScorer::testGetScores() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), directory);
  writer->commit();
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;
  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<Weight> fake =
      (make_shared<TermQuery>(make_shared<Term>(L"fake", L"weight")))
          ->createWeight(searcher, true, 1.0f);
  shared_ptr<Scorer> s = make_shared<SimpleScorer>(fake);
  shared_ptr<ScoreCachingCollector> scc =
      make_shared<ScoreCachingCollector>(scores.size());
  scc->setScorer(s);

  // We need to iterate on the scorer so that its doc() advances.
  int doc;
  while ((doc = s->begin().nextDoc()) != DocIdSetIterator::NO_MORE_DOCS) {
    scc->collect(doc);
  }

  for (int i = 0; i < scores.size(); i++) {
    assertEquals(scores[i], scc->mscores[i], 0.0f);
  }
  delete ir;
  delete directory;
}
} // namespace org::apache::lucene::search