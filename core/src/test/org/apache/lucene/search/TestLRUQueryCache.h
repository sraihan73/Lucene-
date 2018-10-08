#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/QueryCachingPolicy.h"

#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/LRUQueryCache.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/index/RandomIndexWriter.h"
#include  "core/src/java/org/apache/lucene/search/SearcherManager.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/search/BooleanQuery.h"
#include  "core/src/java/org/apache/lucene/search/Builder.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/document/Document.h"
#include  "core/src/java/org/apache/lucene/search/TermQuery.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/search/PhraseQuery.h"
#include  "core/src/java/org/apache/lucene/document/TextField.h"
#include  "core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include  "core/src/java/org/apache/lucene/index/IndexWriterConfig.h"
#include  "core/src/java/org/apache/lucene/search/ScorerSupplier.h"
#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::search
{

using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using FilterDirectoryReader = org::apache::lucene::index::FilterDirectoryReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestLRUQueryCache : public LuceneTestCase
{
  GET_CLASS_NAME(TestLRUQueryCache)

private:
  static const std::shared_ptr<QueryCachingPolicy> NEVER_CACHE;

private:
  class QueryCachingPolicyAnonymousInnerClass
      : public std::enable_shared_from_this<
            QueryCachingPolicyAnonymousInnerClass>,
        public QueryCachingPolicy
  {
    GET_CLASS_NAME(QueryCachingPolicyAnonymousInnerClass)
  public:
    QueryCachingPolicyAnonymousInnerClass();

    void onUse(std::shared_ptr<Query> query) override;
    bool shouldCache(std::shared_ptr<Query> query)  override;
  };

public:
  virtual void testConcurrency() ;

private:
  class SearcherFactoryAnonymousInnerClass : public SearcherFactory
  {
    GET_CLASS_NAME(SearcherFactoryAnonymousInnerClass)
  private:
    std::shared_ptr<TestLRUQueryCache> outerInstance;

    std::shared_ptr<org::apache::lucene::search::LRUQueryCache> queryCache;

  public:
    SearcherFactoryAnonymousInnerClass(
        std::shared_ptr<TestLRUQueryCache> outerInstance,
        std::shared_ptr<org::apache::lucene::search::LRUQueryCache> queryCache);

    std::shared_ptr<IndexSearcher> LuceneTestCase::newSearcher(
        std::shared_ptr<IndexReader> reader,
        std::shared_ptr<IndexReader> previous)  override;

  protected:
    std::shared_ptr<SearcherFactoryAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SearcherFactoryAnonymousInnerClass>(
          SearcherFactory::shared_from_this());
    }
  };

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestLRUQueryCache> outerInstance;

    std::shared_ptr<org::apache::lucene::search::LRUQueryCache> queryCache;
    std::shared_ptr<RandomIndexWriter> w;
    std::shared_ptr<org::apache::lucene::search::SearcherManager> mgr;
    std::shared_ptr<AtomicBoolean> indexing;
    std::shared_ptr<AtomicReference<std::runtime_error>> error;
    int numDocs = 0;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestLRUQueryCache> outerInstance,
        std::shared_ptr<org::apache::lucene::search::LRUQueryCache> queryCache,
        std::shared_ptr<RandomIndexWriter> w,
        std::shared_ptr<org::apache::lucene::search::SearcherManager> mgr,
        std::shared_ptr<AtomicBoolean> indexing,
        std::shared_ptr<AtomicReference<std::runtime_error>> error,
        int numDocs);

    void run();

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

private:
  class ThreadAnonymousInnerClass2 : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass2)
  private:
    std::shared_ptr<TestLRUQueryCache> outerInstance;

    std::shared_ptr<org::apache::lucene::search::SearcherManager> mgr;
    std::shared_ptr<AtomicBoolean> indexing;
    std::shared_ptr<AtomicReference<std::runtime_error>> error;

  public:
    ThreadAnonymousInnerClass2(
        std::shared_ptr<TestLRUQueryCache> outerInstance,
        std::shared_ptr<org::apache::lucene::search::SearcherManager> mgr,
        std::shared_ptr<AtomicBoolean> indexing,
        std::shared_ptr<AtomicReference<std::runtime_error>> error);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass2>(
          Thread::shared_from_this());
    }
  };

public:
  virtual void testLRUEviction() ;

  virtual void testClearFilter() ;

  // This test makes sure that by making the same assumptions as LRUQueryCache,
  // RAMUsageTester computes the same memory usage.
  virtual void testRamBytesUsedAgreesWithRamUsageTester() ;

private:
  class AccumulatorAnonymousInnerClass : public RamUsageTester::Accumulator
  {
    GET_CLASS_NAME(AccumulatorAnonymousInnerClass)
  private:
    std::shared_ptr<TestLRUQueryCache> outerInstance;

  public:
    AccumulatorAnonymousInnerClass(
        std::shared_ptr<TestLRUQueryCache> outerInstance);

    int64_t accumulateObject(
        std::any o, int64_t shallowSize,
        std::unordered_map<std::shared_ptr<Field>, std::any> &fieldValues,
        std::shared_ptr<std::deque<std::any>> queue) override;
    int64_t
    accumulateArray(std::any array_, int64_t shallowSize,
                    std::deque<std::any> &values,
                    std::shared_ptr<std::deque<std::any>> queue) override;

  protected:
    std::shared_ptr<AccumulatorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AccumulatorAnonymousInnerClass>(
          org.apache.lucene.util.RamUsageTester
              .Accumulator::shared_from_this());
    }
  };

  /** A query that doesn't match anything */
private:
  class DummyQuery : public Query
  {
    GET_CLASS_NAME(DummyQuery)

  private:
    static int COUNTER;
    const int id;

  public:
    DummyQuery();

    std::shared_ptr<Weight>
    createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
                 float boost)  override;

  private:
    class ConstantScoreWeightAnonymousInnerClass : public ConstantScoreWeight
    {
      GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass)
    private:
      std::shared_ptr<DummyQuery> outerInstance;

    public:
      ConstantScoreWeightAnonymousInnerClass(
          std::shared_ptr<DummyQuery> outerInstance, float boost);

      std::shared_ptr<Scorer>
      scorer(std::shared_ptr<LeafReaderContext> context) throw(
          IOException) override;

      bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

    protected:
      std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ConstantScoreWeightAnonymousInnerClass>(
            ConstantScoreWeight::shared_from_this());
      }
    };

  public:
    bool equals(std::any other) override;

    virtual int hashCode();

    std::wstring toString(const std::wstring &field) override;

  protected:
    std::shared_ptr<DummyQuery> shared_from_this()
    {
      return std::static_pointer_cast<DummyQuery>(Query::shared_from_this());
    }
  };

  // Test what happens when the cache contains only filters and doc id sets
  // that require very little memory. In that case most of the memory is taken
  // by the cache itself, not cache entries, and we want to make sure that
  // memory usage is not grossly underestimated.
public:
  virtual void testRamBytesUsedConstantEntryOverhead() ;

private:
  class AccumulatorAnonymousInnerClass : public RamUsageTester::Accumulator
  {
    GET_CLASS_NAME(AccumulatorAnonymousInnerClass)
  private:
    std::shared_ptr<TestLRUQueryCache> outerInstance;

  public:
    AccumulatorAnonymousInnerClass(
        std::shared_ptr<TestLRUQueryCache> outerInstance);

    int64_t accumulateObject(
        std::any o, int64_t shallowSize,
        std::unordered_map<std::shared_ptr<Field>, std::any> &fieldValues,
        std::shared_ptr<std::deque<std::any>> queue) override;

  protected:
    std::shared_ptr<AccumulatorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AccumulatorAnonymousInnerClass>(
          org.apache.lucene.util.RamUsageTester
              .Accumulator::shared_from_this());
    }
  };

public:
  virtual void testOnUse() ;

private:
  class QueryCachingPolicyAnonymousInnerClass
      : public std::enable_shared_from_this<
            QueryCachingPolicyAnonymousInnerClass>,
        public QueryCachingPolicy
  {
    GET_CLASS_NAME(QueryCachingPolicyAnonymousInnerClass)
  private:
    std::shared_ptr<TestLRUQueryCache> outerInstance;

    std::unordered_map<std::shared_ptr<Query>, int> expectedCounts;

  public:
    QueryCachingPolicyAnonymousInnerClass(
        std::shared_ptr<TestLRUQueryCache> outerInstance,
        std::unordered_map<std::shared_ptr<Query>, int> &expectedCounts);

    bool shouldCache(std::shared_ptr<Query> query)  override;

    void onUse(std::shared_ptr<Query> query) override;
  };

public:
  virtual void testStats() ;

  virtual void testFineGrainedStats() ;

  void onMiss(std::any readerCoreKey, std::shared_ptr<Query> query) override;

  void onQueryCache(std::shared_ptr<Query> query,
                    int64_t ramBytesUsed) override;

  void onQueryEviction(std::shared_ptr<Query> query,
                       int64_t ramBytesUsed) override;

  void onDocIdSetCache(std::any readerCoreKey, int64_t ramBytesUsed) override;

  void onDocIdSetEviction(std::any readerCoreKey, int numEntries,
                          int64_t sumRamBytesUsed) override;

  void onClear() override;

protected:
  std::shared_ptr<TestLRUQueryCache> shared_from_this()
  {
    return std::static_pointer_cast<TestLRUQueryCache>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

public:
const std::shared_ptr<Query> query =
    std::make_shared<TermQuery>(std::make_shared<Term>(L"color", L"red"));
const std::shared_ptr<Query> query2 =
    std::make_shared<TermQuery>(std::make_shared<Term>(L"color", L"blue"));
const std::shared_ptr<Query> query3 =
    std::make_shared<TermQuery>(std::make_shared<Term>(L"color", L"green"));

for (std::shared_ptr<IndexSearcher> searcher :
     Arrays::asList(searcher1, searcher2)) {
  searcher->setQueryCache(queryCache);
  searcher->setQueryCachingPolicy(QueryCachingPolicy::ALWAYS_CACHE);
}

// search on searcher1
for (int i = 0; i < 10; ++i) {
  searcher1::search(std::make_shared<ConstantScoreQuery>(query), 1);
}
TestUtil::assertEquals(9 * segmentCount1, hitCount1::longValue());
TestUtil::assertEquals(0, hitCount2::longValue());
TestUtil::assertEquals(segmentCount1, missCount1::longValue());
TestUtil::assertEquals(0, missCount2::longValue());

// then on searcher2
for (int i = 0; i < 20; ++i) {
  searcher2::search(std::make_shared<ConstantScoreQuery>(query2), 1);
}
TestUtil::assertEquals(9 * segmentCount1, hitCount1::longValue());
TestUtil::assertEquals(19 * segmentCount2, hitCount2::longValue());
TestUtil::assertEquals(segmentCount1, missCount1::longValue());
TestUtil::assertEquals(segmentCount2, missCount2::longValue());

// now on searcher1 again to trigger evictions
for (int i = 0; i < 30; ++i) {
  searcher1::search(std::make_shared<ConstantScoreQuery>(query3), 1);
}
TestUtil::assertEquals(segmentCount1, queryCache::getEvictionCount());
TestUtil::assertEquals(38 * segmentCount1, hitCount1::longValue());
TestUtil::assertEquals(19 * segmentCount2, hitCount2::longValue());
TestUtil::assertEquals(2 * segmentCount1, missCount1::longValue());
TestUtil::assertEquals(segmentCount2, missCount2::longValue());

// check that the recomputed stats are the same as those reported by the cache
TestUtil::assertEquals(queryCache::ramBytesUsed(),
                       (segmentCount1 + segmentCount2) *
                               LRUQueryCache::HASHTABLE_RAM_BYTES_PER_ENTRY +
                           ramBytesUsage::longValue());
TestUtil::assertEquals(queryCache::getCacheSize(), cacheSize::longValue());

reader1->close();
reader2->close();
w1->close();
w2->close();

TestUtil::assertEquals(queryCache::ramBytesUsed(), ramBytesUsage::longValue());
TestUtil::assertEquals(0, cacheSize::longValue());

queryCache->clear();
TestUtil::assertEquals(0, ramBytesUsage::longValue());
TestUtil::assertEquals(0, cacheSize::longValue());

dir1->close();
dir2->close();
} // namespace org::apache::lucene::search

void testUseRewrittenQueryAsCacheKey() throws IOException
{
  std::shared_ptr<Query> *const expectedCacheKey =
      std::make_shared<TermQuery>(std::make_shared<Term>(L"foo", L"bar"));
  std::shared_ptr<BooleanQuery::Builder> *const query =
      std::make_shared<BooleanQuery::Builder>();
  query->add(std::make_shared<BoostQuery>(expectedCacheKey, 42.0f),
             Occur::MUST);

  std::shared_ptr<LRUQueryCache> *const queryCache =
      std::make_shared<LRUQueryCache>(1000000, 10000000, [&](std::any context) {
        LuceneTestCase::random()->nextBoolean();
      });
  std::shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
  std::shared_ptr<RandomIndexWriter> *const w =
      std::make_shared<RandomIndexWriter>(LuceneTestCase::random(), dir);
  std::shared_ptr<Document> doc = std::make_shared<Document>();
  doc->push_back(std::make_shared<StringField>(L"foo", L"bar", Store::YES));
  w->addDocument(doc);
  w->commit();
  std::shared_ptr<IndexReader> *const reader = w->getReader();
  std::shared_ptr<IndexSearcher> *const searcher =
      LuceneTestCase::newSearcher(reader);
  delete w;

  std::shared_ptr<QueryCachingPolicy> *const policy =
      std::make_shared<QueryCachingPolicyAnonymousInnerClass>(
          shared_from_this(), expectedCacheKey, query);

  searcher->setQueryCache(queryCache);
  searcher->setQueryCachingPolicy(policy);
  searcher->search(query->build(), std::make_shared<TotalHitCountCollector>());

  delete reader;
  delete dir;
}

void testBooleanQueryCachesSubClauses() throws IOException
{
  std::shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
  std::shared_ptr<RandomIndexWriter> *const w =
      std::make_shared<RandomIndexWriter>(LuceneTestCase::random(), dir);
  std::shared_ptr<Document> doc = std::make_shared<Document>();
  doc->push_back(std::make_shared<StringField>(L"foo", L"bar", Store::YES));
  doc->push_back(std::make_shared<StringField>(L"foo", L"quux", Store::YES));
  w->addDocument(doc);
  w->commit();
  std::shared_ptr<IndexReader> *const reader = w->getReader();
  std::shared_ptr<IndexSearcher> *const searcher =
      LuceneTestCase::newSearcher(reader);
  delete w;

  std::shared_ptr<LRUQueryCache> *const queryCache =
      std::make_shared<LRUQueryCache>(1000000, 10000000,
                                      [&](std::any context) { true; });
  searcher->setQueryCache(queryCache);
  searcher->setQueryCachingPolicy(QueryCachingPolicy::ALWAYS_CACHE);

  std::shared_ptr<BooleanQuery::Builder> bq =
      std::make_shared<BooleanQuery::Builder>();
  std::shared_ptr<TermQuery> should =
      std::make_shared<TermQuery>(std::make_shared<Term>(L"foo", L"baz"));
  std::shared_ptr<TermQuery> must =
      std::make_shared<TermQuery>(std::make_shared<Term>(L"foo", L"bar"));
  std::shared_ptr<TermQuery> filter =
      std::make_shared<TermQuery>(std::make_shared<Term>(L"foo", L"quux"));
  std::shared_ptr<TermQuery> mustNot =
      std::make_shared<TermQuery>(std::make_shared<Term>(L"foo", L"foo"));
  bq->add(should, Occur::SHOULD);
  bq->add(must, Occur::MUST);
  bq->add(filter, Occur::FILTER);
  bq->add(mustNot, Occur::MUST_NOT);

  // same bq but with FILTER instead of MUST
  std::shared_ptr<BooleanQuery::Builder> bq2 =
      std::make_shared<BooleanQuery::Builder>();
  bq2->add(should, Occur::SHOULD);
  bq2->add(must, Occur::FILTER);
  bq2->add(filter, Occur::FILTER);
  bq2->add(mustNot, Occur::MUST_NOT);

  TestUtil::assertEquals(Collections::emptySet(),
                         std::unordered_set<>(queryCache->cachedQueries()));
  searcher->search(bq->build(), 1);
  TestUtil::assertEquals(std::unordered_set<>(Arrays::asList(filter, mustNot)),
                         std::unordered_set<>(queryCache->cachedQueries()));

  queryCache->clear();
  TestUtil::assertEquals(Collections::emptySet(),
                         std::unordered_set<>(queryCache->cachedQueries()));
  searcher->search(std::make_shared<ConstantScoreQuery>(bq->build()), 1);
  TestUtil::assertEquals(std::unordered_set<>(Arrays::asList(
                             bq2->build(), should, must, filter, mustNot)),
                         std::unordered_set<>(queryCache->cachedQueries()));

  delete reader;
  delete dir;
}

private:
static Term randomTerm()
{
  const std::wstring term = RandomPicks::randomFrom(
      LuceneTestCase::random(), Arrays::asList(L"foo", L"bar", L"baz"));
  return std::make_shared<Term>(L"foo", term);
}

static Query buildRandomQuery(int level)
{
  if (level == 10) {
    // at most 10 levels
    return std::make_shared<MatchAllDocsQuery>();
  }
  switch (LuceneTestCase::random()->nextInt(6)) {
  case 0:
    return std::make_shared<TermQuery>(randomTerm());
  case 1: {
    std::shared_ptr<BooleanQuery::Builder> bq =
        std::make_shared<BooleanQuery::Builder>();
    constexpr int numClauses =
        TestUtil::nextInt(LuceneTestCase::random(), 1, 3);
    int numShould = 0;
    for (int i = 0; i < numClauses; ++i) {
      constexpr Occur occur =
          RandomPicks::randomFrom(LuceneTestCase::random(), Occur::values());
      bq->add(buildRandomQuery(level + 1), occur);
      if (occur == Occur::SHOULD) {
        numShould++;
      }
    }
    bq->setMinimumNumberShouldMatch(
        TestUtil::nextInt(LuceneTestCase::random(), 0, numShould));
    return bq->build();
  }
  case 2: {
    std::shared_ptr<Term> t1 = randomTerm();
    std::shared_ptr<Term> t2 = randomTerm();
    std::shared_ptr<PhraseQuery> pq =
        std::make_shared<PhraseQuery>(LuceneTestCase::random()->nextInt(2),
                                      t1->field(), t1->bytes(), t2->bytes());
    return pq;
  }
  case 3:
    return std::make_shared<MatchAllDocsQuery>();
  case 4:
    return std::make_shared<ConstantScoreQuery>(buildRandomQuery(level + 1));
  case 5: {
    std::deque<std::shared_ptr<Query>> disjuncts;
    constexpr int numQueries =
        TestUtil::nextInt(LuceneTestCase::random(), 1, 3);
    for (int i = 0; i < numQueries; ++i) {
      disjuncts.push_back(buildRandomQuery(level + 1));
    }
    return std::make_shared<DisjunctionMaxQuery>(
        disjuncts, LuceneTestCase::random()->nextFloat());
  }
  default:
    throw std::make_shared<AssertionError>();
  }
}

public:
void testRandom() throws IOException
{
  std::shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
  std::shared_ptr<RandomIndexWriter> *const w =
      std::make_shared<RandomIndexWriter>(LuceneTestCase::random(), dir);
  std::shared_ptr<Document> doc = std::make_shared<Document>();
  std::shared_ptr<TextField> f =
      std::make_shared<TextField>(L"foo", L"foo", Store::NO);
  doc->push_back(f);
  w->addDocument(doc);
  std::shared_ptr<IndexReader> reader = w->getReader();

  constexpr int maxSize;
  constexpr int64_t maxRamBytesUsed;
  constexpr int iters;

  if (LuceneTestCase::TEST_NIGHTLY) {
    maxSize = TestUtil::nextInt(LuceneTestCase::random(), 1, 10000);
    maxRamBytesUsed = TestUtil::nextLong(LuceneTestCase::random(), 1, 5000000);
    iters = LuceneTestCase::atLeast(20000);
  } else {
    maxSize = TestUtil::nextInt(LuceneTestCase::random(), 1, 1000);
    maxRamBytesUsed = TestUtil::nextLong(LuceneTestCase::random(), 1, 500000);
    iters = LuceneTestCase::atLeast(2000);
  }

  std::shared_ptr<LRUQueryCache> *const queryCache =
      std::make_shared<LRUQueryCache>(
          maxSize, maxRamBytesUsed,
          [&](std::any context) { LuceneTestCase::random()->nextBoolean(); });
  std::shared_ptr<IndexSearcher> uncachedSearcher = nullptr;
  std::shared_ptr<IndexSearcher> cachedSearcher = nullptr;

  for (int i = 0; i < iters; ++i) {
    if (i == 0 || LuceneTestCase::random()->nextInt(100) == 1) {
      delete reader;
      f->setStringValue(
          RandomPicks::randomFrom(LuceneTestCase::random(),
                                  Arrays::asList(L"foo", L"bar", L"bar baz")));
      w->addDocument(doc);
      if (LuceneTestCase::random()->nextBoolean()) {
        w->deleteDocuments(buildRandomQuery(0));
      }
      reader = w->getReader();
      uncachedSearcher = LuceneTestCase::newSearcher(reader);
      uncachedSearcher->setQueryCache(nullptr);
      cachedSearcher = LuceneTestCase::newSearcher(reader);
      cachedSearcher->setQueryCache(queryCache);
      cachedSearcher->setQueryCachingPolicy(QueryCachingPolicy::ALWAYS_CACHE);
    }
    std::shared_ptr<Query> *const q = buildRandomQuery(0);
    TestUtil::assertEquals(uncachedSearcher->count(q),
                           cachedSearcher->count(q));
    if (LuceneTestCase::rarely()) {
      queryCache->assertConsistent();
    }
  }
  queryCache->assertConsistent();
  delete w;
  delete reader;
  delete dir;
  queryCache->assertConsistent();
}

private:
static class BadQuery extends Query
{
  GET_CLASS_NAME(BadQuery)

  std::deque<int> i = {42}; // an array so that clone keeps the reference

public:
  Weight createWeight(IndexSearcher searcher, bool needsScores,
                      float boost) throws IOException
  {
    return std::make_shared<ConstantScoreWeightAnonymousInnerClass>(
        shared_from_this(), boost);
  }

  // C++ TODO: There is no native C++ equivalent to 'toString':
  std::wstring Term::toString(std::wstring field) { return L"BadQuery"; }

  int hashCode() { return classHash() ^ i[0]; }

  bool equals(std::any other)
  {
    return sameClassAs(other) &&
           i[0] == (std::static_pointer_cast<BadQuery>(other))->i[0];
  }
}

      void testDetectMutatedQueries() throws IOException
{
  LuceneTestCase::assumeFalse(
      L"LUCENE-7604: For some unknown reason the non-constant "
      L"BadQuery#hashCode() does not trigger ConcurrentModificationException "
      L"on Java 9 b150",
      Constants::JRE_IS_MINIMUM_JAVA9);
  std::shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
  std::shared_ptr<RandomIndexWriter> *const w =
      std::make_shared<RandomIndexWriter>(LuceneTestCase::random(), dir);
  w->addDocument(std::make_shared<Document>());
  std::shared_ptr<IndexReader> reader = w->getReader();

  // size of 1 so that 2nd query evicts from the cache
  std::shared_ptr<LRUQueryCache> *const queryCache =
      std::make_shared<LRUQueryCache>(1, 10000,
                                      [&](std::any context) { true; });
  std::shared_ptr<IndexSearcher> *const searcher =
      LuceneTestCase::newSearcher(reader);
  searcher->setQueryCache(queryCache);
  searcher->setQueryCachingPolicy(QueryCachingPolicy::ALWAYS_CACHE);

  std::shared_ptr<BadQuery> query = std::make_shared<BadQuery>();
  searcher->count(query);
  query->i[0] += 1; // change the hashCode!

  try {
    // trigger an eviction
    searcher->search(std::make_shared<MatchAllDocsQuery>(),
                     std::make_shared<TotalHitCountCollector>());
    fail();
  } catch (const ConcurrentModificationException &e) {
    // expected
  } catch (const std::runtime_error &e) {
    // expected: wrapped when executor is in use
    std::runtime_error cause = e.getCause();
    assertTrue(std::dynamic_pointer_cast<ExecutionException>(cause) != nullptr);
    assertTrue(std::dynamic_pointer_cast<ConcurrentModificationException>(
                   cause.getCause()) != nullptr);
  }

  IOUtils::close({w, reader, dir});
}

void testRefuseToCacheTooLargeEntries() throws IOException
{
  std::shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
  std::shared_ptr<RandomIndexWriter> *const w =
      std::make_shared<RandomIndexWriter>(LuceneTestCase::random(), dir);
  for (int i = 0; i < 100; ++i) {
    w->addDocument(std::make_shared<Document>());
  }
  std::shared_ptr<IndexReader> reader = w->getReader();

  // size of 1 byte
  std::shared_ptr<LRUQueryCache> *const queryCache =
      std::make_shared<LRUQueryCache>(1, 1, [&](std::any context) {
        LuceneTestCase::random()->nextBoolean();
      });
  std::shared_ptr<IndexSearcher> *const searcher =
      LuceneTestCase::newSearcher(reader);
  searcher->setQueryCache(queryCache);
  searcher->setQueryCachingPolicy(QueryCachingPolicy::ALWAYS_CACHE);

  searcher->count(std::make_shared<MatchAllDocsQuery>());
  TestUtil::assertEquals(0, queryCache->getCacheCount());
  TestUtil::assertEquals(0, queryCache->getEvictionCount());

  delete reader;
  delete w;
  delete dir;
}

/**
 * Tests CachingWrapperWeight.scorer() propagation of {@link
 * QueryCachingPolicy#onUse(Query)} when the first segment is skipped.
 *
 * #f:foo #f:bar causes all frequencies to increment
 * #f:bar #f:foo does not increment the frequency for f:foo
 */
void testOnUseWithRandomFirstSegmentSkipping() throws IOException
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (final org.apache.lucene.store.Directory
  // directory = newDirectory())
  {
    std::shared_ptr<org::apache::lucene::store::Directory> *const directory =
        LuceneTestCase::newDirectory();
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (final
    // org.apache.lucene.index.RandomIndexWriter indexWriter = new
    // org.apache.lucene.index.RandomIndexWriter(random(), directory,
    // newIndexWriterConfig().setMergePolicy(org.apache.lucene.index.NoMergePolicy.INSTANCE)))
    {
      std::shared_ptr<org::apache::lucene::index::RandomIndexWriter>
          *const indexWriter =
              std::make_shared<org.apache::lucene::index::RandomIndexWriter>(
                  LuceneTestCase::random(), directory,
                  LuceneTestCase::newIndexWriterConfig()->setMergePolicy(
                      org.apache::lucene::index::NoMergePolicy::INSTANCE));
      std::shared_ptr<Document> doc =
          std::make_shared<org.apache::lucene::document::Document>();
      doc->push_back(
          std::make_shared<org.apache::lucene::document::StringField>(
              L"f", L"bar", org.apache::lucene::document::Field::Store::NO));
      indexWriter->addDocument(doc);
      if (LuceneTestCase::random()->nextBoolean()) {
        indexWriter->getReader()->close();
      }
      doc = std::make_shared<org.apache::lucene::document::Document>();
      doc->push_back(
          std::make_shared<org.apache::lucene::document::StringField>(
              L"f", L"foo", org.apache::lucene::document::Field::Store::NO));
      doc->push_back(
          std::make_shared<org.apache::lucene::document::StringField>(
              L"f", L"bar", org.apache::lucene::document::Field::Store::NO));
      indexWriter->addDocument(doc);
      indexWriter->commit();
    }
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (final org.apache.lucene.index.IndexReader
    // indexReader = org.apache.lucene.index.DirectoryReader.open(directory))
    {
      std::shared_ptr<org::apache::lucene::index::IndexReader>
          *const indexReader =
              org.apache::lucene::index::DirectoryReader::open(directory);
      std::shared_ptr<FrequencyCountingPolicy> *const policy =
          std::make_shared<FrequencyCountingPolicy>();
      std::shared_ptr<IndexSearcher> *const indexSearcher =
          std::make_shared<IndexSearcher>(indexReader);
      indexSearcher->setQueryCache(
          std::make_shared<LRUQueryCache>(100, 10240, [&](std::any context) {
            LuceneTestCase::random()->nextBoolean();
          }));
      indexSearcher->setQueryCachingPolicy(policy);
      std::shared_ptr<Query> *const foo = std::make_shared<TermQuery>(
          std::make_shared<org.apache::lucene::index::Term>(L"f", L"foo"));
      std::shared_ptr<Query> *const bar = std::make_shared<TermQuery>(
          std::make_shared<org.apache::lucene::index::Term>(L"f", L"bar"));
      std::shared_ptr<BooleanQuery::Builder> *const query =
          std::make_shared<BooleanQuery::Builder>();
      if (LuceneTestCase::random()->nextBoolean()) {
        query->add(foo,
                   org.apache::lucene::search::BooleanClause::Occur::FILTER);
        query->add(bar,
                   org.apache::lucene::search::BooleanClause::Occur::FILTER);
      } else {
        query->add(bar,
                   org.apache::lucene::search::BooleanClause::Occur::FILTER);
        query->add(foo,
                   org.apache::lucene::search::BooleanClause::Occur::FILTER);
      }
      indexSearcher->count(query->build());
      TestUtil::assertEquals(1, policy->frequency(query->build()));
      TestUtil::assertEquals(1, policy->frequency(foo));
      TestUtil::assertEquals(1, policy->frequency(bar));
    }
  }
}

private:
static class FrequencyCountingPolicy implements QueryCachingPolicy
{
  GET_CLASS_NAME(FrequencyCountingPolicy)
  final
      std::unordered_map<std::shared_ptr<Query>, std::shared_ptr<AtomicInteger>>
          counts = std::unordered_map<>();

public:
  int frequency(final Query query)
  {
    std::shared_ptr<AtomicInteger> count;
    {
      std::lock_guard<std::mutex> lock(counts);
      count = counts->get(query);
    }
    return count != nullptr ? count->get() : 0;
  }

  void onUse(final Query query)
  {
    std::shared_ptr<AtomicInteger> count;
    {
      std::lock_guard<std::mutex> lock(counts);
      count = counts->get(query);
      if (count == nullptr) {
        count = std::make_shared<AtomicInteger>();
        counts::put(query, count);
      }
    }
    count->incrementAndGet();
  }

  bool shouldCache(Query query) throws IOException { return true; }
}

private : static class WeightWrapper extends FilterWeight
{
  GET_CLASS_NAME(WeightWrapper)

  final AtomicBoolean scorerCalled;
  final AtomicBoolean bulkScorerCalled;

protected:
  WeightWrapper(Weight in_, AtomicBoolean scorerCalled,
                AtomicBoolean bulkScorerCalled)
  {
    __super(in_);
    this->scorerCalled = scorerCalled;
    this->bulkScorerCalled = bulkScorerCalled;
  }

public:
  Scorer scorer(org.apache::lucene::index::LeafReaderContext context) throws
      IOException
  {
    scorerCalled::set(true);
    return in_::scorer(context);
  }

  BulkScorer bulkScorer(
      org.apache::lucene::index::LeafReaderContext context) throws IOException
  {
    bulkScorerCalled::set(true);
    return in_::bulkScorer(context);
  }
}

      void testPropagateBulkScorer() throws IOException
{
  std::shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
  std::shared_ptr<RandomIndexWriter> w =
      std::make_shared<org.apache::lucene::index::RandomIndexWriter>(
          LuceneTestCase::random(), dir);
  w->addDocument(std::make_shared<org.apache::lucene::document::Document>());
  std::shared_ptr<IndexReader> reader = w->getReader();
  delete w;
  std::shared_ptr<IndexSearcher> searcher = LuceneTestCase::newSearcher(reader);
  std::shared_ptr<LeafReaderContext> leaf =
      searcher->getIndexReader()->leaves()[0];
  std::shared_ptr<AtomicBoolean> scorerCalled =
      std::make_shared<AtomicBoolean>();
  std::shared_ptr<AtomicBoolean> bulkScorerCalled =
      std::make_shared<AtomicBoolean>();
  std::shared_ptr<LRUQueryCache> cache =
      std::make_shared<LRUQueryCache>(1, std::numeric_limits<int64_t>::max(),
                                      [&](std::any context) { true; });

  // test that the bulk scorer is propagated when a scorer should not be cached
  std::shared_ptr<Weight> weight =
      searcher->createWeight(std::make_shared<MatchAllDocsQuery>(), false, 1);
  weight =
      std::make_shared<WeightWrapper>(weight, scorerCalled, bulkScorerCalled);
  weight = cache->doCache(weight, NEVER_CACHE);
  weight->bulkScorer(leaf);
  TestUtil::assertEquals(true, bulkScorerCalled->get());
  TestUtil::assertEquals(false, scorerCalled->get());
  TestUtil::assertEquals(0, cache->getCacheCount());

  // test that the doc id set is computed using the bulk scorer
  bulkScorerCalled->set(false);
  weight =
      searcher->createWeight(std::make_shared<MatchAllDocsQuery>(), false, 1);
  weight =
      std::make_shared<WeightWrapper>(weight, scorerCalled, bulkScorerCalled);
  weight = cache->doCache(weight, QueryCachingPolicy::ALWAYS_CACHE);
  weight->scorer(leaf);
  TestUtil::assertEquals(true, bulkScorerCalled->get());
  TestUtil::assertEquals(false, scorerCalled->get());
  TestUtil::assertEquals(1, cache->getCacheCount());

  delete searcher->getIndexReader();
  delete dir;
}

void testEvictEmptySegmentCache() throws IOException
{
  std::shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
  std::shared_ptr<RandomIndexWriter> *const w =
      std::make_shared<org.apache::lucene::index::RandomIndexWriter>(
          LuceneTestCase::random(), dir);
  w->addDocument(std::make_shared<org.apache::lucene::document::Document>());
  std::shared_ptr<DirectoryReader> *const reader = w->getReader();
  std::shared_ptr<IndexSearcher> *const searcher =
      LuceneTestCase::newSearcher(reader);

protected:
  std::shared_ptr<LRUQueryCache> *const queryCache =
      std::make_shared<LRUQueryCache>(2, 100000,
                                      [&](std::any context) { true; })
  {
    @Override void onDocIdSetEviction(std::any readerCoreKey, int numEntries,
                                      int64_t sumRamBytesUsed)
    {
      __super::onDocIdSetEviction(readerCoreKey, numEntries, sumRamBytesUsed);
      assertTrue(numEntries > 0);
    }
  };

  searcher::setQueryCache(queryCache);
  searcher::setQueryCachingPolicy(QueryCachingPolicy::ALWAYS_CACHE);

  std::shared_ptr<Query> query = std::make_shared<DummyQuery>();
  searcher::count(query);
  TestUtil::assertEquals(Collections::singletonList(query),
                         queryCache::cachedQueries());
  queryCache::clearQuery(query);

  reader->close(); // make sure this does not trigger eviction of segment caches
                   // with no entries
  w->close();
  dir->close();
}

public:
void testMinSegmentSizePredicate() throws IOException
{
  std::shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
  std::shared_ptr<IndexWriterConfig> iwc =
      LuceneTestCase::newIndexWriterConfig()->setMergePolicy(
          org.apache::lucene::index::NoMergePolicy::INSTANCE);
  std::shared_ptr<RandomIndexWriter> w =
      std::make_shared<org.apache::lucene::index::RandomIndexWriter>(
          LuceneTestCase::random(), dir, iwc);
  w->addDocument(std::make_shared<org.apache::lucene::document::Document>());
  std::shared_ptr<DirectoryReader> reader = w->getReader();
  std::shared_ptr<IndexSearcher> searcher = LuceneTestCase::newSearcher(reader);
  searcher->setQueryCachingPolicy(QueryCachingPolicy::ALWAYS_CACHE);

  std::shared_ptr<LRUQueryCache> cache = std::make_shared<LRUQueryCache>(
      2, 10000,
      std::make_shared<LRUQueryCache::MinSegmentSizePredicate>(2, 0.0f));
  searcher->setQueryCache(cache);
  searcher->count(std::make_shared<DummyQuery>());
  TestUtil::assertEquals(0, cache->getCacheCount());

  cache = std::make_shared<LRUQueryCache>(
      2, 10000,
      std::make_shared<LRUQueryCache::MinSegmentSizePredicate>(1, 0.0f));
  searcher->setQueryCache(cache);
  searcher->count(std::make_shared<DummyQuery>());
  TestUtil::assertEquals(1, cache->getCacheCount());

  cache = std::make_shared<LRUQueryCache>(
      2, 10000,
      std::make_shared<LRUQueryCache::MinSegmentSizePredicate>(0, .6f));
  searcher->setQueryCache(cache);
  searcher->count(std::make_shared<DummyQuery>());
  TestUtil::assertEquals(1, cache->getCacheCount());

  w->addDocument(std::make_shared<org.apache::lucene::document::Document>());
  reader->close();
  reader = w->getReader();
  searcher = LuceneTestCase::newSearcher(reader);
  searcher->setQueryCachingPolicy(QueryCachingPolicy::ALWAYS_CACHE);
  cache = std::make_shared<LRUQueryCache>(
      2, 10000,
      std::make_shared<LRUQueryCache::MinSegmentSizePredicate>(0, .6f));
  searcher->setQueryCache(cache);
  searcher->count(std::make_shared<DummyQuery>());
  TestUtil::assertEquals(0, cache->getCacheCount());

  reader->close();
  delete w;
  delete dir;
}

// a reader whose sole purpose is to not be cacheable
private:
static class DummyDirectoryReader extends org.apache::lucene : public
    : index::FilterDirectoryReader
{
  GET_CLASS_NAME(DummyDirectoryReader)

public:
  DummyDirectoryReader(org.apache::lucene::index::DirectoryReader in_)
      throws IOException
  {
    __super(in_, std::make_shared<SubReaderWrapperAnonymousInnerClass>());
  }

protected:
  org.apache::lucene::index::DirectoryReader doWrapDirectoryReader(
      org.apache::lucene::index::DirectoryReader in_) throws IOException
  {
    return std::make_shared<DummyDirectoryReader>(in_);
  }

public:
  org.apache::lucene::index::IndexReader::CacheHelper getReaderCacheHelper()
  {
    return nullptr;
  }
}

void testReaderNotSuitedForCaching() throws IOException
{
  std::shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
  std::shared_ptr<IndexWriterConfig> iwc =
      LuceneTestCase::newIndexWriterConfig()->setMergePolicy(
          org.apache::lucene::index::NoMergePolicy::INSTANCE);
  std::shared_ptr<RandomIndexWriter> w =
      std::make_shared<org.apache::lucene::index::RandomIndexWriter>(
          LuceneTestCase::random(), dir, iwc);
  w->addDocument(std::make_shared<org.apache::lucene::document::Document>());
  std::shared_ptr<DirectoryReader> reader =
      std::make_shared<DummyDirectoryReader>(w->getReader());
  std::shared_ptr<IndexSearcher> searcher = LuceneTestCase::newSearcher(reader);
  searcher->setQueryCachingPolicy(QueryCachingPolicy::ALWAYS_CACHE);

  // don't cache if the reader does not expose a cache helper
  assertNull(reader->leaves()->get(0).reader().getCoreCacheHelper());
  std::shared_ptr<LRUQueryCache> cache = std::make_shared<LRUQueryCache>(
      2, 10000, [&](std::any context) { true; });
  searcher->setQueryCache(cache);
  TestUtil::assertEquals(0, searcher->count(std::make_shared<DummyQuery>()));
  TestUtil::assertEquals(0, cache->getCacheCount());
  reader->close();
  delete w;
  delete dir;
}

// A query that returns null from Weight.getCacheHelper
private:
static class NoCacheQuery extends Query
{
  GET_CLASS_NAME(NoCacheQuery)

public:
  Weight createWeight(IndexSearcher searcher, bool needsScores,
                      float boost) throws IOException
  {
    return std::make_shared<WeightAnonymousInnerClass>(shared_from_this());
  }

  // C++ TODO: There is no native C++ equivalent to 'toString':
  std::wstring Term::toString(std::wstring field) { return L"NoCacheQuery"; }

  bool equals(std::any obj) { return sameClassAs(obj); }

  int hashCode() { return 0; }
}

      void testQueryNotSuitedForCaching() throws IOException
{
  std::shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
  std::shared_ptr<IndexWriterConfig> iwc =
      LuceneTestCase::newIndexWriterConfig()->setMergePolicy(
          org.apache::lucene::index::NoMergePolicy::INSTANCE);
  std::shared_ptr<RandomIndexWriter> w =
      std::make_shared<org.apache::lucene::index::RandomIndexWriter>(
          LuceneTestCase::random(), dir, iwc);
  w->addDocument(std::make_shared<org.apache::lucene::document::Document>());
  std::shared_ptr<DirectoryReader> reader = w->getReader();
  std::shared_ptr<IndexSearcher> searcher = LuceneTestCase::newSearcher(reader);
  searcher->setQueryCachingPolicy(QueryCachingPolicy::ALWAYS_CACHE);

  std::shared_ptr<LRUQueryCache> cache = std::make_shared<LRUQueryCache>(
      2, 10000, [&](std::any context) { true; });
  searcher->setQueryCache(cache);

  TestUtil::assertEquals(0, searcher->count(std::make_shared<NoCacheQuery>()));
  TestUtil::assertEquals(0, cache->getCacheCount());

  // BooleanQuery wrapping an uncacheable query should also not be cached
  std::shared_ptr<BooleanQuery> bq =
      (std::make_shared<BooleanQuery::Builder>())
          ->add(std::make_shared<NoCacheQuery>(),
                org.apache::lucene::search::BooleanClause::Occur::MUST)
          ->add(std::make_shared<TermQuery>(
                    std::make_shared<org.apache::lucene::index::Term>(L"field",
                                                                      L"term")),
                org.apache::lucene::search::BooleanClause::Occur::MUST)
          ->build();
  TestUtil::assertEquals(0, searcher->count(bq));
  TestUtil::assertEquals(0, cache->getCacheCount());

  reader->close();
  delete w;
  delete dir;
}

private:
static class DummyQuery2 extends Query
{
  GET_CLASS_NAME(DummyQuery2)

  final AtomicBoolean scorerCreated;

  DummyQuery2(AtomicBoolean scorerCreated)
  {
    this->scorerCreated = scorerCreated;
  }

public:
  Weight createWeight(IndexSearcher searcher, bool needsScores,
                      float boost) throws IOException
  {
    return std::make_shared<ConstantScoreWeightAnonymousInnerClass2>(
        shared_from_this(), boost);
  }

  bool equals(std::any other) { return sameClassAs(other); }

  int hashCode() { return 0; }

  // C++ TODO: There is no native C++ equivalent to 'toString':
  std::wstring Term::toString(std::wstring field) { return L"DummyQuery2"; }

}

      void testPropagatesScorerSupplier() throws IOException
{
  std::shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
  std::shared_ptr<IndexWriterConfig> iwc =
      LuceneTestCase::newIndexWriterConfig()->setMergePolicy(
          org.apache::lucene::index::NoMergePolicy::INSTANCE);
  std::shared_ptr<RandomIndexWriter> w =
      std::make_shared<org.apache::lucene::index::RandomIndexWriter>(
          LuceneTestCase::random(), dir, iwc);
  w->addDocument(std::make_shared<org.apache::lucene::document::Document>());
  std::shared_ptr<DirectoryReader> reader = w->getReader();
  std::shared_ptr<IndexSearcher> searcher = LuceneTestCase::newSearcher(reader);
  searcher->setQueryCachingPolicy(NEVER_CACHE);

  std::shared_ptr<LRUQueryCache> cache =
      std::make_shared<LRUQueryCache>(1, 1000);
  searcher->setQueryCache(cache);

  std::shared_ptr<AtomicBoolean> scorerCreated =
      std::make_shared<AtomicBoolean>(false);
  std::shared_ptr<Query> query = std::make_shared<DummyQuery2>(scorerCreated);
  std::shared_ptr<Weight> weight =
      searcher->createWeight(searcher->rewrite(query), false, 1);
  std::shared_ptr<ScorerSupplier> supplier =
      weight->scorerSupplier(searcher->getIndexReader()->leaves()[0]);
  assertFalse(scorerCreated->get());
  supplier->get(LuceneTestCase::random()->nextLong() & 0x7FFFFFFFFFFFFFFFLL);
  assertTrue(scorerCreated->get());

  reader->close();
  delete w;
  delete dir;
}

static class DVCacheQuery extends Query
{
  GET_CLASS_NAME(DVCacheQuery)

  const std::wstring field;

  std::shared_ptr<AtomicInteger> scorerCreatedCount =
      std::make_shared<AtomicInteger>(0);

  DVCacheQuery(std::wstring field) { this->field = field; }

  // C++ TODO: There is no native C++ equivalent to 'toString':
  std::wstring Term::toString(std::wstring field) { return L"DVCacheQuery"; }

  bool equals(std::any obj) { return sameClassAs(obj); }

  int hashCode() { return 0; }

  Weight createWeight(IndexSearcher searcher, bool needsScores,
                      float boost) throws IOException
  {
    return std::make_shared<ConstantScoreWeightAnonymousInnerClass3>(
        shared_from_this(), field, scorerCreatedCount);
  }
}

      void testDocValuesUpdatesDontBreakCache() throws IOException
{
  std::shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
  std::shared_ptr<IndexWriterConfig> iwc =
      LuceneTestCase::newIndexWriterConfig()->setMergePolicy(
          org.apache::lucene::index::NoMergePolicy::INSTANCE);
  std::shared_ptr<IndexWriter> w =
      std::make_shared<org.apache::lucene::index::IndexWriter>(dir, iwc);
  w->addDocument(std::make_shared<org.apache::lucene::document::Document>());
  w->commit();
  std::shared_ptr<DirectoryReader> reader =
      org.apache::lucene::index::DirectoryReader::open(w);

  // IMPORTANT:
  // Don't use newSearcher(), because that will sometimes use an
  // ExecutorService, and we need to be single threaded to ensure that
  // LRUQueryCache doesn't skip the cache due to thread contention
  std::shared_ptr<IndexSearcher> searcher =
      std::make_shared<AssertingIndexSearcher>(LuceneTestCase::random(),
                                               reader);
  searcher->setQueryCachingPolicy(QueryCachingPolicy::ALWAYS_CACHE);

  std::shared_ptr<LRUQueryCache> cache = std::make_shared<LRUQueryCache>(
      1, 10000, [&](std::any context) { true; });
  searcher->setQueryCache(cache);

  std::shared_ptr<DVCacheQuery> query =
      std::make_shared<DVCacheQuery>(L"field");
  TestUtil::assertEquals(1, searcher->count(query));
  TestUtil::assertEquals(1, query->scorerCreatedCount->get());
  TestUtil::assertEquals(1, searcher->count(query));
  TestUtil::assertEquals(1,
                         query->scorerCreatedCount->get()); // should be cached

  std::shared_ptr<Document> doc =
      std::make_shared<org.apache::lucene::document::Document>();
  doc->push_back(
      std::make_shared<org.apache::lucene::document::NumericDocValuesField>(
          L"field", 1));
  doc->push_back(LuceneTestCase::newTextField(
      L"text", L"text", org.apache::lucene::document::Field::Store::NO));
  w->addDocument(doc);
  reader->close();
  reader = org.apache::lucene::index::DirectoryReader::open(w);
  searcher = std::make_shared<AssertingIndexSearcher>(
      LuceneTestCase::random(),
      reader); // no newSearcher(reader) - see comment above
  searcher->setQueryCachingPolicy(QueryCachingPolicy::ALWAYS_CACHE);
  searcher->setQueryCache(cache);

  TestUtil::assertEquals(2, searcher->count(query));
  TestUtil::assertEquals(
      2, query->scorerCreatedCount->get()); // first segment cached

  reader->close();
  reader = org.apache::lucene::index::DirectoryReader::open(w);
  searcher = std::make_shared<AssertingIndexSearcher>(
      LuceneTestCase::random(),
      reader); // no newSearcher(reader) - see comment above
  searcher->setQueryCachingPolicy(QueryCachingPolicy::ALWAYS_CACHE);
  searcher->setQueryCache(cache);

  TestUtil::assertEquals(2, searcher->count(query));
  TestUtil::assertEquals(
      2, query->scorerCreatedCount->get()); // both segments cached

  w->updateNumericDocValue(
      std::make_shared<org.apache::lucene::index::Term>(L"text", L"text"),
      L"field", 2LL);
  reader->close();
  reader = org.apache::lucene::index::DirectoryReader::open(w);
  searcher = std::make_shared<AssertingIndexSearcher>(
      LuceneTestCase::random(),
      reader); // no newSearcher(reader) - see comment above
  searcher->setQueryCachingPolicy(QueryCachingPolicy::ALWAYS_CACHE);
  searcher->setQueryCache(cache);

  TestUtil::assertEquals(2, searcher->count(query));
  TestUtil::assertEquals(
      3, query->scorerCreatedCount
             ->get()); // second segment no longer cached due to DV update

  TestUtil::assertEquals(2, searcher->count(query));
  TestUtil::assertEquals(4,
                         query->scorerCreatedCount->get()); // still no caching

  reader->close();
  delete w;
  delete dir;
}

void testQueryCacheSoftUpdate() throws IOException
{
  std::shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
  std::shared_ptr<IndexWriterConfig> iwc =
      LuceneTestCase::newIndexWriterConfig()->setSoftDeletesField(
          L"soft_delete");
  std::shared_ptr<IndexWriter> w =
      std::make_shared<org.apache::lucene::index::IndexWriter>(dir, iwc);
  std::shared_ptr<LRUQueryCache> queryCache = std::make_shared<LRUQueryCache>(
      10, 1000 * 1000, [&](std::any ctx) { true; });
  IndexSearcher::setDefaultQueryCache(queryCache);
  IndexSearcher::setDefaultQueryCachingPolicy(QueryCachingPolicy::ALWAYS_CACHE);

  std::shared_ptr<SearcherManager> sm =
      std::make_shared<SearcherManager>(w, std::make_shared<SearcherFactory>());

  std::shared_ptr<Document> doc =
      std::make_shared<org.apache::lucene::document::Document>();
  doc->push_back(std::make_shared<org.apache::lucene::document::StringField>(
      L"id", L"1", org.apache::lucene::document::Field::Store::YES));
  w->addDocument(doc);

  doc = std::make_shared<org.apache::lucene::document::Document>();
  doc->push_back(std::make_shared<org.apache::lucene::document::StringField>(
      L"id", L"2", org.apache::lucene::document::Field::Store::YES));
  w->addDocument(doc);

  sm->maybeRefreshBlocking();

  std::shared_ptr<IndexSearcher> searcher = sm->acquire();
  std::shared_ptr<Query> query =
      (std::make_shared<BooleanQuery::Builder>())
          ->add(std::make_shared<TermQuery>(
                    std::make_shared<org.apache::lucene::index::Term>(L"id",
                                                                      L"1")),
                BooleanClause::Occur::FILTER)
          ->build();
  TestUtil::assertEquals(1, searcher->count(query));
  TestUtil::assertEquals(1, queryCache->getCacheSize());
  TestUtil::assertEquals(0, queryCache->getEvictionCount());

  bool softDelete = true;
  if (softDelete) {
    std::shared_ptr<Document> tombstone =
        std::make_shared<org.apache::lucene::document::Document>();
    tombstone->push_back(
        std::make_shared<org.apache::lucene::document::NumericDocValuesField>(
            L"soft_delete", 1));
    w->softUpdateDocument(
        std::make_shared<org.apache::lucene::index::Term>(L"id", L"1"),
        tombstone,
        {std::make_shared<org.apache::lucene::document::NumericDocValuesField>(
            L"soft_delete", 1)});
    w->softUpdateDocument(
        std::make_shared<org.apache::lucene::index::Term>(L"id", L"2"),
        tombstone,
        {std::make_shared<org.apache::lucene::document::NumericDocValuesField>(
            L"soft_delete", 1)});
  } else {
    w->deleteDocuments(
        {std::make_shared<org.apache::lucene::index::Term>(L"id", L"1")});
    w->deleteDocuments(
        {std::make_shared<org.apache::lucene::index::Term>(L"id", L"2")});
  }
  sm->maybeRefreshBlocking();
  // All docs in the first segment are deleted - we should drop it with the
  // default merge policy.
  sm->release(searcher);
  TestUtil::assertEquals(0, queryCache->getCacheSize());
  TestUtil::assertEquals(1, queryCache->getEvictionCount());
  delete sm;
  delete w;
  delete dir;
}

void testBulkScorerLocking() throws std::runtime_error
{

  std::shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
  std::shared_ptr<IndexWriterConfig> iwc =
      LuceneTestCase::newIndexWriterConfig()
          ->setMergePolicy(org.apache::lucene::index::NoMergePolicy::INSTANCE)
          ->setMaxBufferedDocs(-1);
  std::shared_ptr<IndexWriter> w =
      std::make_shared<org.apache::lucene::index::IndexWriter>(dir, iwc);

  constexpr int numDocs = LuceneTestCase::atLeast(10);
  std::shared_ptr<Document> emptyDoc =
      std::make_shared<org.apache::lucene::document::Document>();
  for (int d = 0; d < numDocs; ++d) {
    for (int i = LuceneTestCase::random()->nextInt(5000); i >= 0; --i) {
      w->addDocument(emptyDoc);
    }
    std::shared_ptr<Document> doc =
        std::make_shared<org.apache::lucene::document::Document>();
    for (std::wstring value : Arrays::asList(L"foo", L"bar", L"baz")) {
      if (LuceneTestCase::random()->nextBoolean()) {
        doc->push_back(
            std::make_shared<org.apache::lucene::document::StringField>(
                L"field", value,
                org.apache::lucene::document::Field::Store::NO));
      }
    }
  }
  for (int i = org.apache::lucene::util::TestUtil::nextInt(
           LuceneTestCase::random(), 3000, 5000);
       i >= 0; --i) {
    w->addDocument(emptyDoc);
  }
  if (LuceneTestCase::random()->nextBoolean()) {
    w->forceMerge(1);
  }

  std::shared_ptr<DirectoryReader> reader =
      org.apache::lucene::index::DirectoryReader::open(w);
  std::shared_ptr<DirectoryReader> noCacheReader =
      std::make_shared<DummyDirectoryReader>(reader);

  std::shared_ptr<LRUQueryCache> cache = std::make_shared<LRUQueryCache>(
      1, 100000, [&](std::any context) { true; });
  std::shared_ptr<IndexSearcher> searcher =
      std::make_shared<AssertingIndexSearcher>(LuceneTestCase::random(),
                                               reader);
  searcher->setQueryCache(cache);
  searcher->setQueryCachingPolicy(QueryCachingPolicy::ALWAYS_CACHE);

  std::shared_ptr<Query> query = std::make_shared<ConstantScoreQuery>(
      std::make_shared<BooleanQuery::Builder>()
          .add(std::make_shared<BoostQuery>(
                   std::make_shared<TermQuery>(
                       std::make_shared<org.apache::lucene::index::Term>(
                           L"field", L"foo")),
                   3),
               org.apache::lucene::search::BooleanClause::Occur::SHOULD)
          ->add(std::make_shared<BoostQuery>(
                    std::make_shared<TermQuery>(
                        std::make_shared<org.apache::lucene::index::Term>(
                            L"field", L"bar")),
                    3),
                org.apache::lucene::search::BooleanClause::Occur::SHOULD)
          ->add(std::make_shared<BoostQuery>(
                    std::make_shared<TermQuery>(
                        std::make_shared<org.apache::lucene::index::Term>(
                            L"field", L"baz")),
                    3),
                org.apache::lucene::search::BooleanClause::Occur::SHOULD)
          .build());

  searcher->search(query, 1);

  std::shared_ptr<IndexSearcher> noCacheHelperSearcher =
      std::make_shared<AssertingIndexSearcher>(LuceneTestCase::random(),
                                               noCacheReader);
  noCacheHelperSearcher->setQueryCache(cache);
  noCacheHelperSearcher->setQueryCachingPolicy(
      QueryCachingPolicy::ALWAYS_CACHE);
  noCacheHelperSearcher->search(query, 1);

  std::shared_ptr<Thread> t = std::make_shared<Thread>([&]() {
    try {
      noCacheReader->close();
      delete w;
      dir->close();
    } catch (const std::runtime_error &e) {
      throw std::runtime_error(e);
    }
  });
  t->start();
  t->join();
}
}

private:
static class QueryCachingPolicyAnonymousInnerClass implements QueryCachingPolicy
{
  GET_CLASS_NAME(QueryCachingPolicyAnonymousInnerClass)
  MissingClass outerInstance;

  org.apache::lucene::search::Query expectedCacheKey;
  org.apache::lucene::search::BooleanQuery::Builder query;

public:
  QueryCachingPolicyAnonymousInnerClass(
      MissingClass outerInstance,
      org.apache::lucene::search::Query expectedCacheKey,
      org.apache::lucene::search::BooleanQuery::Builder query)
  {
    this->outerInstance = outerInstance;
    this->expectedCacheKey = expectedCacheKey;
    this->query = query;
  }

  bool shouldCache(Query query) throws IOException
  {
    TestUtil::assertEquals(expectedCacheKey, query);
    return true;
  }

  void onUse(Query query) { TestUtil::assertEquals(expectedCacheKey, query); }
}

private : static class ConstantScoreWeightAnonymousInnerClass extends
              ConstantScoreWeight
{
  GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass)
  MissingClass outerInstance;

public:
  ConstantScoreWeightAnonymousInnerClass(MissingClass outerInstance,
                                         UnknownType boost)
  {
    __super(outerInstance, boost);
    this->outerInstance = outerInstance;
  }

  Scorer scorer(org.apache::lucene::index::LeafReaderContext context) throws
      IOException
  {
    return nullptr;
  }

  bool isCacheable(org.apache::lucene::index::LeafReaderContext ctx)
  {
    return true;
  }
}

private : static class SubReaderWrapperAnonymousInnerClass extends
              org.apache::lucene : public
    : index::FilterDirectoryReader::SubReaderWrapper
{
  GET_CLASS_NAME(SubReaderWrapperAnonymousInnerClass)
public:
  SubReaderWrapperAnonymousInnerClass() {}

  org.apache::lucene::index::LeafReader wrap(
      org.apache::lucene::index::LeafReader reader)
  {
    return std::make_shared<FilterLeafReaderAnonymousInnerClass>(
        shared_from_this(), reader);
  }
}

private:
static class WeightAnonymousInnerClass extends Weight
{
  GET_CLASS_NAME(WeightAnonymousInnerClass)
  MissingClass outerInstance;

public:
  WeightAnonymousInnerClass(MissingClass outerInstance)
  {
    __super(outerInstance);
    this->outerInstance = outerInstance;
  }

  void extractTerms(Set<std::shared_ptr<Term>> terms) {}

  Explanation explain(org.apache::lucene::index::LeafReaderContext context,
                      int doc) throws IOException
  {
    return nullptr;
  }

  Scorer scorer(org.apache::lucene::index::LeafReaderContext context) throws
      IOException
  {
    return nullptr;
  }

  bool isCacheable(org.apache::lucene::index::LeafReaderContext ctx)
  {
    return false;
  }
}

private : static class ConstantScoreWeightAnonymousInnerClass2 extends
              ConstantScoreWeight
{
  GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass2)
  MissingClass outerInstance;

public:
  ConstantScoreWeightAnonymousInnerClass2(MissingClass outerInstance,
                                          UnknownType boost)
  {
    __super(outerInstance, boost);
    this->outerInstance = outerInstance;
  }

  Scorer scorer(org.apache::lucene::index::LeafReaderContext context) throws
      IOException
  {
    return scorerSupplier(context)->get(std::numeric_limits<int64_t>::max());
  }

  bool isCacheable(org.apache::lucene::index::LeafReaderContext ctx)
  {
    return true;
  }

  ScorerSupplier scorerSupplier(
      org.apache::lucene::index::LeafReaderContext context) throws IOException
  {
    std::shared_ptr<Weight> *const weight = shared_from_this();
    return std::make_shared<ScorerSupplierAnonymousInnerClass>(
        shared_from_this(), weight);
  }
}

private : static class ConstantScoreWeightAnonymousInnerClass3 extends
              ConstantScoreWeight
{
  GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass3)
  MissingClass outerInstance;

  std::wstring field;
  AtomicInteger scorerCreatedCount;

public:
  ConstantScoreWeightAnonymousInnerClass3(MissingClass outerInstance,
                                          std::wstring field,
                                          AtomicInteger scorerCreatedCount)
  {
    __super(outerInstance, 1);
    this->outerInstance = outerInstance;
    this->field = field;
    this->scorerCreatedCount = scorerCreatedCount;
  }

  Scorer scorer(org.apache::lucene::index::LeafReaderContext context) throws
      IOException
  {
    scorerCreatedCount::incrementAndGet();
    return std::make_shared<ConstantScoreScorer>(
        shared_from_this(), 1,
        DocIdSetIterator::all(context::reader().maxDoc()));
  }

  bool isCacheable(org.apache::lucene::index::LeafReaderContext ctx)
  {
    return org.apache::lucene::index::DocValues::isCacheable(ctx, {field});
  }

}

private : static class FilterLeafReaderAnonymousInnerClass extends
              org.apache::lucene : public : index::FilterLeafReader
{
  GET_CLASS_NAME(FilterLeafReaderAnonymousInnerClass)
  MissingClass outerInstance;

public:
  FilterLeafReaderAnonymousInnerClass(MissingClass outerInstance,
                                      UnknownType reader)
  {
    __super(reader);
    this->outerInstance = outerInstance;
  }

  org.apache::lucene::index::IndexReader::CacheHelper getCoreCacheHelper()
  {
    return nullptr;
  }
  org.apache::lucene::index::IndexReader::CacheHelper getReaderCacheHelper()
  {
    return nullptr;
  }
}

private:
static class ScorerSupplierAnonymousInnerClass extends ScorerSupplier
{
  GET_CLASS_NAME(ScorerSupplierAnonymousInnerClass)
  MissingClass outerInstance;

  org.apache::lucene::search::Weight weight;

public:
  ScorerSupplierAnonymousInnerClass(MissingClass outerInstance,
                                    org.apache::lucene::search::Weight weight)
  {
    this->outerInstance = outerInstance;
    this->weight = weight;
  }

  Scorer get(int64_t leadCost) throws IOException
  {
    scorerCreated::set(true);
    return std::make_shared<ConstantScoreScorer>(weight, boost,
                                                 DocIdSetIterator::all(1));
  }

  int64_t cost() { return 1; }
}
}
