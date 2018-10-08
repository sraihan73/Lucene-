using namespace std;

#include "TestUsageTrackingFilterCachingPolicy.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using IntPoint = org::apache::lucene::document::IntPoint;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestUsageTrackingFilterCachingPolicy::testCostlyFilter()
{
  assertTrue(UsageTrackingQueryCachingPolicy::isCostly(
      make_shared<PrefixQuery>(make_shared<Term>(L"field", L"prefix"))));
  assertTrue(UsageTrackingQueryCachingPolicy::isCostly(
      IntPoint::newRangeQuery(L"intField", 1, 1000)));
  assertFalse(UsageTrackingQueryCachingPolicy::isCostly(
      make_shared<TermQuery>(make_shared<Term>(L"field", L"value"))));
}

void TestUsageTrackingFilterCachingPolicy::testNeverCacheMatchAll() throw(
    runtime_error)
{
  shared_ptr<Query> q = make_shared<MatchAllDocsQuery>();
  shared_ptr<UsageTrackingQueryCachingPolicy> policy =
      make_shared<UsageTrackingQueryCachingPolicy>();
  for (int i = 0; i < 1000; ++i) {
    policy->onUse(q);
  }
  assertFalse(policy->shouldCache(q));
}

void TestUsageTrackingFilterCachingPolicy::testNeverCacheTermFilter() throw(
    IOException)
{
  shared_ptr<Query> q =
      make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar"));
  shared_ptr<UsageTrackingQueryCachingPolicy> policy =
      make_shared<UsageTrackingQueryCachingPolicy>();
  for (int i = 0; i < 1000; ++i) {
    policy->onUse(q);
  }
  assertFalse(policy->shouldCache(q));
}

void TestUsageTrackingFilterCachingPolicy::testBooleanQueries() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  w->addDocument(make_shared<Document>());
  shared_ptr<IndexReader> reader = w->getReader();
  delete w;

  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(reader);
  shared_ptr<UsageTrackingQueryCachingPolicy> policy =
      make_shared<UsageTrackingQueryCachingPolicy>();
  shared_ptr<LRUQueryCache> cache = make_shared<LRUQueryCache>(
      10, numeric_limits<int64_t>::max(),
      make_shared<LRUQueryCache::MinSegmentSizePredicate>(1, 0.0f));
  searcher->setQueryCache(cache);
  searcher->setQueryCachingPolicy(policy);

  shared_ptr<DummyQuery> q1 = make_shared<DummyQuery>(1);
  shared_ptr<DummyQuery> q2 = make_shared<DummyQuery>(2);
  shared_ptr<BooleanQuery> bq = (make_shared<BooleanQuery::Builder>())
                                    ->add(q1, Occur::SHOULD)
                                    ->add(q2, Occur::SHOULD)
                                    ->build();

  for (int i = 0; i < 3; ++i) {
    searcher->count(bq);
  }
  assertEquals(0, cache->getCacheSize()); // nothing cached yet, too early

  searcher->count(bq);
  assertEquals(1,
               cache->getCacheSize()); // the bq got cached, but not q1 and q2

  for (int i = 0; i < 10; ++i) {
    searcher->count(bq);
  }
  assertEquals(1, cache->getCacheSize()); // q1 and q2 still not cached since we
                                          // do not pull scorers on them

  searcher->count(q1);
  assertEquals(2, cache->getCacheSize()); // q1 used on its own -> cached

  delete reader;
  delete dir;
}

TestUsageTrackingFilterCachingPolicy::DummyQuery::DummyQuery(int id) : id(id) {}

wstring
TestUsageTrackingFilterCachingPolicy::DummyQuery::toString(const wstring &field)
{
  return L"dummy";
}

bool TestUsageTrackingFilterCachingPolicy::DummyQuery::equals(any obj)
{
  return sameClassAs(obj) &&
         (any_cast<std::shared_ptr<DummyQuery>>(obj))->id == id;
}

int TestUsageTrackingFilterCachingPolicy::DummyQuery::hashCode() { return id; }

shared_ptr<Weight>
TestUsageTrackingFilterCachingPolicy::DummyQuery::createWeight(
    shared_ptr<IndexSearcher> searcher, bool needsScores,
    float boost) 
{
  return make_shared<ConstantScoreWeightAnonymousInnerClass>(
      shared_from_this(), DummyQuery::this, boost);
}

// C++ TODO: You cannot use 'shared_from_this' in a constructor:
TestUsageTrackingFilterCachingPolicy::DummyQuery::
    ConstantScoreWeightAnonymousInnerClass::
        ConstantScoreWeightAnonymousInnerClass(
            shared_ptr<DummyQuery> outerInstance,
            shared_ptr<org::apache::lucene::search::
                           TestUsageTrackingFilterCachingPolicy::DummyQuery>
                shared_from_this(),
            float boost)
    : ConstantScoreWeight(this, boost)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Scorer> TestUsageTrackingFilterCachingPolicy::DummyQuery::
    ConstantScoreWeightAnonymousInnerClass::scorer(
        shared_ptr<LeafReaderContext> context) 
{
  return make_shared<ConstantScoreScorer>(shared_from_this(), score(),
                                          DocIdSetIterator::all(1));
}

bool TestUsageTrackingFilterCachingPolicy::DummyQuery::
    ConstantScoreWeightAnonymousInnerClass::isCacheable(
        shared_ptr<LeafReaderContext> ctx)
{
  return true;
}
} // namespace org::apache::lucene::search