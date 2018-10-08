using namespace std;

#include "TestReqExclBulkScorer.h"

namespace org::apache::lucene::search
{
using Bits = org::apache::lucene::util::Bits;
using DocIdSetBuilder = org::apache::lucene::util::DocIdSetBuilder;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestReqExclBulkScorer::testRandom() 
{
  constexpr int iters = atLeast(10);
  for (int iter = 0; iter < iters; ++iter) {
    doTestRandom();
  }
}

void TestReqExclBulkScorer::doTestRandom() 
{
  constexpr int maxDoc = TestUtil::nextInt(random(), 1, 1000);
  shared_ptr<DocIdSetBuilder> reqBuilder = make_shared<DocIdSetBuilder>(maxDoc);
  shared_ptr<DocIdSetBuilder> exclBuilder =
      make_shared<DocIdSetBuilder>(maxDoc);
  constexpr int numIncludedDocs = TestUtil::nextInt(random(), 1, maxDoc);
  constexpr int numExcludedDocs = TestUtil::nextInt(random(), 1, maxDoc);
  shared_ptr<DocIdSetBuilder::BulkAdder> reqAdder =
      reqBuilder->grow(numIncludedDocs);
  for (int i = 0; i < numIncludedDocs; ++i) {
    reqAdder->add(random()->nextInt(maxDoc));
  }
  shared_ptr<DocIdSetBuilder::BulkAdder> exclAdder =
      exclBuilder->grow(numExcludedDocs);
  for (int i = 0; i < numExcludedDocs; ++i) {
    exclAdder->add(random()->nextInt(maxDoc));
  }

  shared_ptr<DocIdSet> *const req = reqBuilder->build();
  shared_ptr<DocIdSet> *const excl = exclBuilder->build();

  shared_ptr<BulkScorer> *const reqBulkScorer =
      make_shared<BulkScorerAnonymousInnerClass>(shared_from_this(), req);

  shared_ptr<ReqExclBulkScorer> reqExcl =
      make_shared<ReqExclBulkScorer>(reqBulkScorer, excl->begin());
  shared_ptr<FixedBitSet> *const actualMatches =
      make_shared<FixedBitSet>(maxDoc);
  if (random()->nextBoolean()) {
    reqExcl->score(make_shared<LeafCollectorAnonymousInnerClass>(
                       shared_from_this(), actualMatches),
                   nullptr);
  } else {
    int next = 0;
    while (next < maxDoc) {
      constexpr int min = next;
      constexpr int max = min + random()->nextInt(10);
      next = reqExcl->score(make_shared<LeafCollectorAnonymousInnerClass2>(
                                shared_from_this(), actualMatches),
                            nullptr, min, max);
      assertTrue(next >= max);
    }
  }

  shared_ptr<FixedBitSet> *const expectedMatches =
      make_shared<FixedBitSet>(maxDoc);
  expectedMatches->or (req->begin());
  shared_ptr<FixedBitSet> excludedSet = make_shared<FixedBitSet>(maxDoc);
  excludedSet->or (excl->begin());
  expectedMatches->andNot(excludedSet);

  assertArrayEquals(expectedMatches->getBits(), actualMatches->getBits());
}

TestReqExclBulkScorer::BulkScorerAnonymousInnerClass::
    BulkScorerAnonymousInnerClass(
        shared_ptr<TestReqExclBulkScorer> outerInstance,
        shared_ptr<org::apache::lucene::search::DocIdSet> req)
{
  this->outerInstance = outerInstance;
  this->req = req;
  iterator = req->begin();
}

int TestReqExclBulkScorer::BulkScorerAnonymousInnerClass::score(
    shared_ptr<LeafCollector> collector, shared_ptr<Bits> acceptDocs, int min,
    int max) 
{
  int doc = iterator::docID();
  if (iterator::docID() < min) {
    doc = iterator::advance(min);
  }
  while (doc < max) {
    if (acceptDocs == nullptr || acceptDocs->get(doc)) {
      collector->collect(doc);
    }
    doc = iterator::nextDoc();
  }
  return doc;
}

int64_t TestReqExclBulkScorer::BulkScorerAnonymousInnerClass::cost()
{
  return iterator::cost();
}

TestReqExclBulkScorer::LeafCollectorAnonymousInnerClass::
    LeafCollectorAnonymousInnerClass(
        shared_ptr<TestReqExclBulkScorer> outerInstance,
        shared_ptr<FixedBitSet> actualMatches)
{
  this->outerInstance = outerInstance;
  this->actualMatches = actualMatches;
}

void TestReqExclBulkScorer::LeafCollectorAnonymousInnerClass::setScorer(
    shared_ptr<Scorer> scorer) 
{
}

void TestReqExclBulkScorer::LeafCollectorAnonymousInnerClass::collect(
    int doc) 
{
  actualMatches->set(doc);
}

TestReqExclBulkScorer::LeafCollectorAnonymousInnerClass2::
    LeafCollectorAnonymousInnerClass2(
        shared_ptr<TestReqExclBulkScorer> outerInstance,
        shared_ptr<FixedBitSet> actualMatches)
{
  this->outerInstance = outerInstance;
  this->actualMatches = actualMatches;
}

void TestReqExclBulkScorer::LeafCollectorAnonymousInnerClass2::setScorer(
    shared_ptr<Scorer> scorer) 
{
}

void TestReqExclBulkScorer::LeafCollectorAnonymousInnerClass2::collect(
    int doc) 
{
  actualMatches->set(doc);
}
} // namespace org::apache::lucene::search