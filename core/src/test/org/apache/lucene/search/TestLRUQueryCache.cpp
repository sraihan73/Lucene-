using namespace std;

#include "TestLRUQueryCache.h"

namespace org::apache::lucene::search
{
using com::carrotsearch::randomizedtesting::generators::RandomPicks;
using Document = org::apache::lucene::document::Document;
using Store = org::apache::lucene::document::Field::Store;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using DocValues = org::apache::lucene::index::DocValues;
using FilterDirectoryReader = org::apache::lucene::index::FilterDirectoryReader;
using FilterLeafReader = org::apache::lucene::index::FilterLeafReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NoMergePolicy = org::apache::lucene::index::NoMergePolicy;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using SerialMergeScheduler = org::apache::lucene::index::SerialMergeScheduler;
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using Directory = org::apache::lucene::store::Directory;
using Constants = org::apache::lucene::util::Constants;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using RamUsageTester = org::apache::lucene::util::RamUsageTester;
using TestUtil = org::apache::lucene::util::TestUtil;
const shared_ptr<QueryCachingPolicy> TestLRUQueryCache::NEVER_CACHE =
    make_shared<QueryCachingPolicyAnonymousInnerClass>();

TestLRUQueryCache::QueryCachingPolicyAnonymousInnerClass::
    QueryCachingPolicyAnonymousInnerClass()
{
}

void TestLRUQueryCache::QueryCachingPolicyAnonymousInnerClass::onUse(
    shared_ptr<Query> query)
{
}

bool TestLRUQueryCache::QueryCachingPolicyAnonymousInnerClass::shouldCache(
    shared_ptr<Query> query) 
{
  return false;
}

void TestLRUQueryCache::testConcurrency() 
{
  shared_ptr<LRUQueryCache> *const queryCache = make_shared<LRUQueryCache>(
      1 + random()->nextInt(20), 1 + random()->nextInt(10000),
      [&](any context) { random()->nextBoolean(); });
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<SearcherFactory> *const searcherFactory =
      make_shared<SearcherFactoryAnonymousInnerClass>(shared_from_this(),
                                                      queryCache);
  constexpr bool applyDeletes = random()->nextBoolean();
  shared_ptr<SearcherManager> *const mgr =
      make_shared<SearcherManager>(w->w, applyDeletes, false, searcherFactory);
  shared_ptr<AtomicBoolean> *const indexing = make_shared<AtomicBoolean>(true);
  shared_ptr<AtomicReference<runtime_error>> *const error =
      make_shared<AtomicReference<runtime_error>>();
  constexpr int numDocs = atLeast(10000);
  std::deque<std::shared_ptr<Thread>> threads(3);
  threads[0] = make_shared<ThreadAnonymousInnerClass>(
      shared_from_this(), queryCache, w, mgr, indexing, error, numDocs);
  for (int i = 1; i < threads.size(); ++i) {
    threads[i] = make_shared<ThreadAnonymousInnerClass2>(shared_from_this(),
                                                         mgr, indexing, error);
  }

  for (auto thread : threads) {
    thread->start();
  }

  for (auto thread : threads) {
    thread->join();
  }

  if (error->get() != nullptr) {
    throw error->get();
  }
  queryCache->assertConsistent();
  delete mgr;
  delete w;
  delete dir;
  queryCache->assertConsistent();
}

TestLRUQueryCache::SearcherFactoryAnonymousInnerClass::
    SearcherFactoryAnonymousInnerClass(
        shared_ptr<TestLRUQueryCache> outerInstance,
        shared_ptr<org::apache::lucene::search::LRUQueryCache> queryCache)
{
  this->outerInstance = outerInstance;
  this->queryCache = queryCache;
}

shared_ptr<IndexSearcher>
TestLRUQueryCache::SearcherFactoryAnonymousInnerClass::LuceneTestCase::
    newSearcher(shared_ptr<IndexReader> reader,
                shared_ptr<IndexReader> previous) 
{
  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(reader);
  searcher->setQueryCachingPolicy(MAYBE_CACHE_POLICY);
  searcher->setQueryCache(queryCache);
  return searcher;
}

TestLRUQueryCache::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestLRUQueryCache> outerInstance,
    shared_ptr<org::apache::lucene::search::LRUQueryCache> queryCache,
    shared_ptr<RandomIndexWriter> w,
    shared_ptr<org::apache::lucene::search::SearcherManager> mgr,
    shared_ptr<AtomicBoolean> indexing,
    shared_ptr<AtomicReference<runtime_error>> error, int numDocs)
{
  this->outerInstance = outerInstance;
  this->queryCache = queryCache;
  this->w = w;
  this->mgr = mgr;
  this->indexing = indexing;
  this->error = error;
  this->numDocs = numDocs;
}

void TestLRUQueryCache::ThreadAnonymousInnerClass::run()
{
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<StringField> f =
      make_shared<StringField>(L"color", L"", Store::NO);
  doc->push_back(f);
  for (int i = 0; indexing->get() && i < numDocs; ++i) {
    f->setStringValue(RandomPicks::randomFrom(
        LuceneTestCase::random(),
        std::deque<wstring>{L"blue", L"red", L"yellow"}));
    try {
      w->addDocument(doc);
      if ((i & 63) == 0) {
        mgr->maybeRefresh();
        if (LuceneTestCase::rarely()) {
          queryCache->clear();
        }
        if (LuceneTestCase::rarely()) {
          const wstring color = RandomPicks::randomFrom(
              LuceneTestCase::random(),
              std::deque<wstring>{L"blue", L"red", L"yellow"});
          w->deleteDocuments(make_shared<Term>(L"color", color));
        }
      }
    } catch (const runtime_error &t) {
      error->compareAndSet(nullptr, t);
      break;
    }
  }
  indexing->set(false);
}

TestLRUQueryCache::ThreadAnonymousInnerClass2::ThreadAnonymousInnerClass2(
    shared_ptr<TestLRUQueryCache> outerInstance,
    shared_ptr<org::apache::lucene::search::SearcherManager> mgr,
    shared_ptr<AtomicBoolean> indexing,
    shared_ptr<AtomicReference<runtime_error>> error)
{
  this->outerInstance = outerInstance;
  this->mgr = mgr;
  this->indexing = indexing;
  this->error = error;
}

void TestLRUQueryCache::ThreadAnonymousInnerClass2::run()
{
  while (indexing->get()) {
    try {
      shared_ptr<IndexSearcher> *const searcher = mgr->acquire();
      try {
        const wstring value = RandomPicks::randomFrom(
            LuceneTestCase::random(),
            std::deque<wstring>{L"blue", L"red", L"yellow", L"green"});
        shared_ptr<Query> *const q =
            make_shared<TermQuery>(make_shared<Term>(L"color", value));
        shared_ptr<TotalHitCountCollector> collector =
            make_shared<TotalHitCountCollector>();
        searcher->search(q, collector); // will use the cache
        constexpr int totalHits1 = collector->getTotalHits();
        constexpr int64_t totalHits2 =
            searcher->search(q, 1)
                ->totalHits; // will not use the cache because of scores
        TestUtil::assertEquals(totalHits2, totalHits1);
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        mgr->release(searcher);
      }
    } catch (const runtime_error &t) {
      error->compareAndSet(nullptr, t);
    }
  }
}

void TestLRUQueryCache::testLRUEviction() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), dir);

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<StringField> f =
      make_shared<StringField>(L"color", L"blue", Store::NO);
  doc->push_back(f);
  w->addDocument(doc);
  f->setStringValue(L"red");
  w->addDocument(doc);
  f->setStringValue(L"green");
  w->addDocument(doc);
  shared_ptr<DirectoryReader> *const reader = w->getReader();
  shared_ptr<IndexSearcher> *const searcher = newSearcher(reader);
  shared_ptr<LRUQueryCache> *const queryCache =
      make_shared<LRUQueryCache>(2, 100000, [&](any context) { true; });

  shared_ptr<Query> *const blue =
      make_shared<TermQuery>(make_shared<Term>(L"color", L"blue"));
  shared_ptr<Query> *const red =
      make_shared<TermQuery>(make_shared<Term>(L"color", L"red"));
  shared_ptr<Query> *const green =
      make_shared<TermQuery>(make_shared<Term>(L"color", L"green"));

  TestUtil::assertEquals(Collections::emptyList(), queryCache->cachedQueries());

  searcher->setQueryCache(queryCache);
  // the filter is not cached on any segment: no changes
  searcher->setQueryCachingPolicy(NEVER_CACHE);
  searcher->search(make_shared<ConstantScoreQuery>(green), 1);
  TestUtil::assertEquals(Collections::emptyList(), queryCache->cachedQueries());

  searcher->setQueryCachingPolicy(QueryCachingPolicy::ALWAYS_CACHE);
  searcher->search(make_shared<ConstantScoreQuery>(red), 1);
  TestUtil::assertEquals(Collections::singletonList(red),
                         queryCache->cachedQueries());

  searcher->search(make_shared<ConstantScoreQuery>(green), 1);
  TestUtil::assertEquals(Arrays::asList(red, green),
                         queryCache->cachedQueries());

  searcher->search(make_shared<ConstantScoreQuery>(red), 1);
  TestUtil::assertEquals(Arrays::asList(green, red),
                         queryCache->cachedQueries());

  searcher->search(make_shared<ConstantScoreQuery>(blue), 1);
  TestUtil::assertEquals(Arrays::asList(red, blue),
                         queryCache->cachedQueries());

  searcher->search(make_shared<ConstantScoreQuery>(blue), 1);
  TestUtil::assertEquals(Arrays::asList(red, blue),
                         queryCache->cachedQueries());

  searcher->search(make_shared<ConstantScoreQuery>(green), 1);
  TestUtil::assertEquals(Arrays::asList(blue, green),
                         queryCache->cachedQueries());

  searcher->setQueryCachingPolicy(NEVER_CACHE);
  searcher->search(make_shared<ConstantScoreQuery>(red), 1);
  TestUtil::assertEquals(Arrays::asList(blue, green),
                         queryCache->cachedQueries());

  reader->close();
  delete w;
  delete dir;
}

void TestLRUQueryCache::testClearFilter() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), dir);

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<StringField> f =
      make_shared<StringField>(L"color", L"", Store::NO);
  doc->push_back(f);
  constexpr int numDocs = atLeast(10);
  for (int i = 0; i < numDocs; ++i) {
    f->setStringValue(random()->nextBoolean() ? L"red" : L"blue");
    w->addDocument(doc);
  }
  shared_ptr<DirectoryReader> *const reader = w->getReader();
  shared_ptr<IndexSearcher> *const searcher = newSearcher(reader);

  shared_ptr<Query> *const query1 =
      make_shared<TermQuery>(make_shared<Term>(L"color", L"blue"));
  // different instance yet equal
  shared_ptr<Query> *const query2 =
      make_shared<TermQuery>(make_shared<Term>(L"color", L"blue"));

  shared_ptr<LRUQueryCache> *const queryCache = make_shared<LRUQueryCache>(
      numeric_limits<int>::max(), numeric_limits<int64_t>::max(),
      [&](any context) { true; });
  searcher->setQueryCache(queryCache);
  searcher->setQueryCachingPolicy(QueryCachingPolicy::ALWAYS_CACHE);

  searcher->search(
      make_shared<BoostQuery>(make_shared<ConstantScoreQuery>(query1),
                              random()->nextFloat()),
      1);
  TestUtil::assertEquals(1, queryCache->cachedQueries().size());

  queryCache->clearQuery(query2);

  assertTrue(queryCache->cachedQueries().empty());
  queryCache->assertConsistent();

  reader->close();
  delete w;
  delete dir;
}

void TestLRUQueryCache::testRamBytesUsedAgreesWithRamUsageTester() throw(
    IOException)
{
  assumeFalse(L"LUCENE-7595: RamUsageTester does not work exact in Java 9 "
              L"(estimations for maps and lists)",
              Constants::JRE_IS_MINIMUM_JAVA9);

  shared_ptr<LRUQueryCache> *const queryCache = make_shared<LRUQueryCache>(
      1 + random()->nextInt(5), 1 + random()->nextInt(10000),
      [&](any context) { random()->nextBoolean(); });
  // an accumulator that only sums up memory usage of referenced filters and doc
  // id sets
  shared_ptr<RamUsageTester::Accumulator> *const acc =
      make_shared<AccumulatorAnonymousInnerClass>(shared_from_this());

  shared_ptr<Directory> dir = newDirectory();
  // serial merges so that segments do not get closed while we are measuring ram
  // usage with RamUsageTester
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig()->setMergeScheduler(
      make_shared<SerialMergeScheduler>());
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  const deque<wstring> colors =
      Arrays::asList(L"blue", L"red", L"green", L"yellow");

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<StringField> f =
      make_shared<StringField>(L"color", L"", Store::NO);
  doc->push_back(f);
  constexpr int iters = atLeast(5);
  for (int iter = 0; iter < iters; ++iter) {
    constexpr int numDocs = atLeast(10);
    for (int i = 0; i < numDocs; ++i) {
      f->setStringValue(RandomPicks::randomFrom(random(), colors));
      w->addDocument(doc);
    }
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (final
    // org.apache.lucene.index.DirectoryReader reader = w.getReader())
    {
      shared_ptr<org::apache::lucene::index::DirectoryReader> *const reader =
          w->getReader();
      shared_ptr<IndexSearcher> *const searcher = newSearcher(reader);
      searcher->setQueryCache(queryCache);
      searcher->setQueryCachingPolicy(MAYBE_CACHE_POLICY);
      for (int i = 0; i < 3; ++i) {
        shared_ptr<Query> *const query =
            make_shared<TermQuery>(make_shared<org.apache::lucene::index::Term>(
                L"color", RandomPicks::randomFrom(random(), colors)));
        searcher->search(make_shared<ConstantScoreQuery>(query), 1);
      }
    }
    queryCache->assertConsistent();
    TestUtil::assertEquals(
        org.apache::lucene::util::RamUsageTester::sizeOf(queryCache, acc),
        queryCache->ramBytesUsed());
  }

  delete w;
  delete dir;
}

TestLRUQueryCache::AccumulatorAnonymousInnerClass::
    AccumulatorAnonymousInnerClass(shared_ptr<TestLRUQueryCache> outerInstance)
{
  this->outerInstance = outerInstance;
}

int64_t TestLRUQueryCache::AccumulatorAnonymousInnerClass::accumulateObject(
    any o, int64_t shallowSize,
    unordered_map<std::shared_ptr<Field>, any> &fieldValues,
    shared_ptr<deque<any>> queue)
{
  if (std::dynamic_pointer_cast<DocIdSet>(o) != nullptr) {
    return (any_cast<std::shared_ptr<DocIdSet>>(o)).ramBytesUsed();
  }
  if (std::dynamic_pointer_cast<Query>(o) != nullptr) {
    return LRUQueryCache::QUERY_DEFAULT_RAM_BYTES_USED;
  }
  if (std::dynamic_pointer_cast<IndexReader>(o) != nullptr ||
      o.type().getSimpleName().equals(L"SegmentCoreReaders")) {
    // do not take readers or core cache keys into account
    return 0;
  }
  if (dynamic_cast<unordered_map>(o) != nullptr) {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: java.util.Map<?,?> map_obj = (java.util.Map<?,?>) o;
    unordered_map < ?, ? > map_obj = any_cast < unordered_map < ?, ? >> (o);
    queue->addAll(map_obj.keySet());
    queue->addAll(map_obj.values());
    constexpr int64_t sizePerEntry =
        std::dynamic_pointer_cast<LinkedHashMap>(o) != nullptr
            ? LRUQueryCache::LINKED_HASHTABLE_RAM_BYTES_PER_ENTRY
            : LRUQueryCache::HASHTABLE_RAM_BYTES_PER_ENTRY;
    return sizePerEntry * map_obj.size();
  }
  // follow links to other objects, but ignore their memory usage
  outerInstance->super->accumulateObject(o, shallowSize, fieldValues, queue);
  return 0;
}

int64_t TestLRUQueryCache::AccumulatorAnonymousInnerClass::accumulateArray(
    any array_, int64_t shallowSize, deque<any> &values,
    shared_ptr<deque<any>> queue)
{
  // follow links to other objects, but ignore their memory usage
  outerInstance->super->accumulateArray(array_, shallowSize, values, queue);
  return 0;
}

int TestLRUQueryCache::DummyQuery::COUNTER = 0;

TestLRUQueryCache::DummyQuery::DummyQuery() : id(COUNTER++) {}

shared_ptr<Weight>
TestLRUQueryCache::DummyQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                            bool needsScores,
                                            float boost) 
{
  return make_shared<ConstantScoreWeightAnonymousInnerClass>(shared_from_this(),
                                                             boost);
}

TestLRUQueryCache::DummyQuery::ConstantScoreWeightAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass(shared_ptr<DummyQuery> outerInstance,
                                           float boost)
    : ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Scorer>
TestLRUQueryCache::DummyQuery::ConstantScoreWeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  return nullptr;
}

bool TestLRUQueryCache::DummyQuery::ConstantScoreWeightAnonymousInnerClass::
    isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return true;
}

bool TestLRUQueryCache::DummyQuery::equals(any other)
{
  return sameClassAs(other) &&
         id == (any_cast<std::shared_ptr<DummyQuery>>(other)).id;
}

int TestLRUQueryCache::DummyQuery::hashCode() { return id; }

wstring TestLRUQueryCache::DummyQuery::toString(const wstring &field)
{
  return L"DummyQuery";
}

void TestLRUQueryCache::testRamBytesUsedConstantEntryOverhead() throw(
    IOException)
{
  assumeFalse(L"LUCENE-7595: RamUsageTester does not work exact in Java 9 "
              L"(estimations for maps and lists)",
              Constants::JRE_IS_MINIMUM_JAVA9);

  shared_ptr<LRUQueryCache> *const queryCache =
      make_shared<LRUQueryCache>(1000000, 10000000, [&](any context) { true; });

  shared_ptr<RamUsageTester::Accumulator> *const acc =
      make_shared<AccumulatorAnonymousInnerClass>(shared_from_this());

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  constexpr int numDocs = atLeast(100);
  for (int i = 0; i < numDocs; ++i) {
    w->addDocument(doc);
  }
  shared_ptr<DirectoryReader> *const reader = w->getReader();
  shared_ptr<IndexSearcher> *const searcher =
      make_shared<IndexSearcher>(reader);
  searcher->setQueryCache(queryCache);
  searcher->setQueryCachingPolicy(QueryCachingPolicy::ALWAYS_CACHE);

  constexpr int numQueries = atLeast(1000);
  for (int i = 0; i < numQueries; ++i) {
    shared_ptr<Query> *const query = make_shared<DummyQuery>();
    searcher->search(make_shared<ConstantScoreQuery>(query), 1);
  }
  assertTrue(queryCache->getCacheCount() > 0);

  constexpr int64_t actualRamBytesUsed =
      RamUsageTester::sizeOf(queryCache, acc);
  constexpr int64_t expectedRamBytesUsed = queryCache->ramBytesUsed();
  // error < 30%
  assertEquals(actualRamBytesUsed, expectedRamBytesUsed,
               30 * actualRamBytesUsed / 100);

  reader->close();
  delete w;
  delete dir;
}

TestLRUQueryCache::AccumulatorAnonymousInnerClass::
    AccumulatorAnonymousInnerClass(shared_ptr<TestLRUQueryCache> outerInstance)
{
  this->outerInstance = outerInstance;
}

int64_t TestLRUQueryCache::AccumulatorAnonymousInnerClass::accumulateObject(
    any o, int64_t shallowSize,
    unordered_map<std::shared_ptr<Field>, any> &fieldValues,
    shared_ptr<deque<any>> queue)
{
  if (std::dynamic_pointer_cast<DocIdSet>(o) != nullptr) {
    return (any_cast<std::shared_ptr<DocIdSet>>(o)).ramBytesUsed();
  }
  if (std::dynamic_pointer_cast<Query>(o) != nullptr) {
    return LRUQueryCache::QUERY_DEFAULT_RAM_BYTES_USED;
  }
  if (o.type().getSimpleName().equals(L"SegmentCoreReaders")) {
    // do not follow references to core cache keys
    return 0;
  }
  return outerInstance->super->accumulateObject(o, shallowSize, fieldValues,
                                                queue);
}

void TestLRUQueryCache::testOnUse() 
{
  shared_ptr<LRUQueryCache> *const queryCache = make_shared<LRUQueryCache>(
      1 + random()->nextInt(5), 1 + random()->nextInt(1000),
      [&](any context) { random()->nextBoolean(); });

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), dir);

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<StringField> f =
      make_shared<StringField>(L"color", L"", Store::NO);
  doc->push_back(f);
  constexpr int numDocs = atLeast(10);
  for (int i = 0; i < numDocs; ++i) {
    f->setStringValue(RandomPicks::randomFrom(
        random(), Arrays::asList(L"red", L"blue", L"green", L"yellow")));
    w->addDocument(doc);
    if (random()->nextBoolean()) {
      w->getReader()->close();
    }
  }
  shared_ptr<DirectoryReader> *const reader = w->getReader();
  shared_ptr<IndexSearcher> *const searcher =
      make_shared<IndexSearcher>(reader);

  const unordered_map<std::shared_ptr<Query>, int> actualCounts =
      unordered_map<std::shared_ptr<Query>, int>();
  const unordered_map<std::shared_ptr<Query>, int> expectedCounts =
      unordered_map<std::shared_ptr<Query>, int>();

  shared_ptr<QueryCachingPolicy> *const countingPolicy =
      make_shared<QueryCachingPolicyAnonymousInnerClass>(shared_from_this(),
                                                         expectedCounts);

  std::deque<std::shared_ptr<Query>> queries(10 + random()->nextInt(10));
  for (int i = 0; i < queries.size(); ++i) {
    queries[i] = make_shared<BoostQuery>(
        make_shared<TermQuery>(make_shared<Term>(
            L"color", RandomPicks::randomFrom(
                          random(), Arrays::asList(L"red", L"blue", L"green",
                                                   L"yellow")))),
        random()->nextFloat());
  }

  searcher->setQueryCache(queryCache);
  searcher->setQueryCachingPolicy(countingPolicy);
  for (int i = 0; i < 20; ++i) {
    constexpr int idx = random()->nextInt(queries.size());
    searcher->search(make_shared<ConstantScoreQuery>(queries[idx]), 1);
    shared_ptr<Query> cacheKey = queries[idx];
    while (std::dynamic_pointer_cast<BoostQuery>(cacheKey) != nullptr) {
      cacheKey = (std::static_pointer_cast<BoostQuery>(cacheKey))->getQuery();
    }
    actualCounts.emplace(cacheKey, 1 + actualCounts.getOrDefault(cacheKey, 0));
  }

  TestUtil::assertEquals(actualCounts, expectedCounts);

  reader->close();
  delete w;
  delete dir;
}

TestLRUQueryCache::QueryCachingPolicyAnonymousInnerClass::
    QueryCachingPolicyAnonymousInnerClass(
        shared_ptr<TestLRUQueryCache> outerInstance,
        unordered_map<std::shared_ptr<Query>, int> &expectedCounts)
{
  this->outerInstance = outerInstance;
  this->expectedCounts = expectedCounts;
}

bool TestLRUQueryCache::QueryCachingPolicyAnonymousInnerClass::shouldCache(
    shared_ptr<Query> query) 
{
  return LuceneTestCase::random()->nextBoolean();
}

void TestLRUQueryCache::QueryCachingPolicyAnonymousInnerClass::onUse(
    shared_ptr<Query> query)
{
  expectedCounts.emplace(query, 1 + expectedCounts.getOrDefault(query, 0));
}

void TestLRUQueryCache::testStats() 
{
  shared_ptr<LRUQueryCache> *const queryCache =
      make_shared<LRUQueryCache>(1, 10000000, [&](any context) { true; });

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), dir);

  const deque<wstring> colors =
      Arrays::asList(L"blue", L"red", L"green", L"yellow");

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<StringField> f =
      make_shared<StringField>(L"color", L"", Store::NO);
  doc->push_back(f);
  for (int i = 0; i < 10; ++i) {
    f->setStringValue(RandomPicks::randomFrom(random(), colors));
    w->addDocument(doc);
    if (random()->nextBoolean()) {
      w->getReader()->close();
    }
  }

  shared_ptr<DirectoryReader> *const reader = w->getReader();
  constexpr int segmentCount = reader->leaves()->size();
  shared_ptr<IndexSearcher> *const searcher =
      make_shared<IndexSearcher>(reader);
  shared_ptr<Query> *const query =
      make_shared<TermQuery>(make_shared<Term>(L"color", L"red"));
  shared_ptr<Query> *const query2 =
      make_shared<TermQuery>(make_shared<Term>(L"color", L"blue"));

  searcher->setQueryCache(queryCache);
  // first pass, lookups without caching that all miss
  searcher->setQueryCachingPolicy(NEVER_CACHE);
  for (int i = 0; i < 10; ++i) {
    searcher->search(make_shared<ConstantScoreQuery>(query), 1);
  }
  TestUtil::assertEquals(10 * segmentCount, queryCache->getTotalCount());
  TestUtil::assertEquals(0, queryCache->getHitCount());
  TestUtil::assertEquals(10 * segmentCount, queryCache->getMissCount());
  TestUtil::assertEquals(0, queryCache->getCacheCount());
  TestUtil::assertEquals(0, queryCache->getEvictionCount());
  TestUtil::assertEquals(0, queryCache->getCacheSize());

  // second pass, lookups + caching, only the first one is a miss
  searcher->setQueryCachingPolicy(QueryCachingPolicy::ALWAYS_CACHE);
  for (int i = 0; i < 10; ++i) {
    searcher->search(make_shared<ConstantScoreQuery>(query), 1);
  }
  TestUtil::assertEquals(20 * segmentCount, queryCache->getTotalCount());
  TestUtil::assertEquals(9 * segmentCount, queryCache->getHitCount());
  TestUtil::assertEquals(11 * segmentCount, queryCache->getMissCount());
  TestUtil::assertEquals(1 * segmentCount, queryCache->getCacheCount());
  TestUtil::assertEquals(0, queryCache->getEvictionCount());
  TestUtil::assertEquals(1 * segmentCount, queryCache->getCacheSize());

  // third pass lookups without caching, we only have hits
  searcher->setQueryCachingPolicy(NEVER_CACHE);
  for (int i = 0; i < 10; ++i) {
    searcher->search(make_shared<ConstantScoreQuery>(query), 1);
  }
  TestUtil::assertEquals(30 * segmentCount, queryCache->getTotalCount());
  TestUtil::assertEquals(19 * segmentCount, queryCache->getHitCount());
  TestUtil::assertEquals(11 * segmentCount, queryCache->getMissCount());
  TestUtil::assertEquals(1 * segmentCount, queryCache->getCacheCount());
  TestUtil::assertEquals(0, queryCache->getEvictionCount());
  TestUtil::assertEquals(1 * segmentCount, queryCache->getCacheSize());

  // fourth pass with a different filter which will trigger evictions since the
  // size is 1
  searcher->setQueryCachingPolicy(QueryCachingPolicy::ALWAYS_CACHE);
  for (int i = 0; i < 10; ++i) {
    searcher->search(make_shared<ConstantScoreQuery>(query2), 1);
  }
  TestUtil::assertEquals(40 * segmentCount, queryCache->getTotalCount());
  TestUtil::assertEquals(28 * segmentCount, queryCache->getHitCount());
  TestUtil::assertEquals(12 * segmentCount, queryCache->getMissCount());
  TestUtil::assertEquals(2 * segmentCount, queryCache->getCacheCount());
  TestUtil::assertEquals(1 * segmentCount, queryCache->getEvictionCount());
  TestUtil::assertEquals(1 * segmentCount, queryCache->getCacheSize());

  // now close, causing evictions due to the closing of segment cores
  reader->close();
  delete w;
  TestUtil::assertEquals(40 * segmentCount, queryCache->getTotalCount());
  TestUtil::assertEquals(28 * segmentCount, queryCache->getHitCount());
  TestUtil::assertEquals(12 * segmentCount, queryCache->getMissCount());
  TestUtil::assertEquals(2 * segmentCount, queryCache->getCacheCount());
  TestUtil::assertEquals(2 * segmentCount, queryCache->getEvictionCount());
  TestUtil::assertEquals(0, queryCache->getCacheSize());

  delete dir;
}

void TestLRUQueryCache::testFineGrainedStats() 
{
  shared_ptr<Directory> dir1 = newDirectory();
  shared_ptr<RandomIndexWriter> *const w1 =
      make_shared<RandomIndexWriter>(random(), dir1);
  shared_ptr<Directory> dir2 = newDirectory();
  shared_ptr<RandomIndexWriter> *const w2 =
      make_shared<RandomIndexWriter>(random(), dir2);

  const deque<wstring> colors =
      Arrays::asList(L"blue", L"red", L"green", L"yellow");

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<StringField> f =
      make_shared<StringField>(L"color", L"", Store::NO);
  doc->push_back(f);
  for (shared_ptr<RandomIndexWriter> w : Arrays::asList(w1, w2)) {
    for (int i = 0; i < 10; ++i) {
      f->setStringValue(RandomPicks::randomFrom(random(), colors));
      w->addDocument(doc);
      if (random()->nextBoolean()) {
        w->getReader()->close();
      }
    }
  }

  shared_ptr<DirectoryReader> *const reader1 = w1->getReader();
  constexpr int segmentCount1 = reader1->leaves()->size();
  shared_ptr<IndexSearcher> *const searcher1 =
      make_shared<IndexSearcher>(reader1);

  shared_ptr<DirectoryReader> *const reader2 = w2->getReader();
  constexpr int segmentCount2 = reader2->leaves()->size();
  shared_ptr<IndexSearcher> *const searcher2 =
      make_shared<IndexSearcher>(reader2);

  const unordered_map<std::shared_ptr<IndexReader::CacheKey>, int> indexId =
      unordered_map<std::shared_ptr<IndexReader::CacheKey>, int>();
  for (shared_ptr<LeafReaderContext> ctx : reader1->leaves()) {
    indexId.emplace(ctx->reader()->getCoreCacheHelper()->getKey(), 1);
  }
  for (shared_ptr<LeafReaderContext> ctx : reader2->leaves()) {
    indexId.emplace(ctx->reader()->getCoreCacheHelper()->getKey(), 2);
  }

  shared_ptr<AtomicLong> *const hitCount1 = make_shared<AtomicLong>();
  shared_ptr<AtomicLong> *const hitCount2 = make_shared<AtomicLong>();
  shared_ptr<AtomicLong> *const missCount1 = make_shared<AtomicLong>();
  shared_ptr<AtomicLong> *const missCount2 = make_shared<AtomicLong>();

  shared_ptr<AtomicLong> *const ramBytesUsage = make_shared<AtomicLong>();
  shared_ptr<AtomicLong> *const cacheSize = make_shared<AtomicLong>();

protected:
  shared_ptr<LRUQueryCache> *const queryCache =
      make_shared<LRUQueryCache>(2, 10000000, [&](any context) { true; })
  {
    @Override void onHit(any readerCoreKey, Query query)
    {
      LuceneTestCase::onHit(readerCoreKey, query);
      switch (indexId[readerCoreKey]) {
      case 1:
        hitCount1->incrementAndGet();
        break;
      case 2:
        hitCount2->incrementAndGet();
        break;
      default:
        throw make_shared<AssertionError>();
      }
    }

    void TestLRUQueryCache::onMiss(any readerCoreKey, shared_ptr<Query> query)
    {
      LuceneTestCase::onMiss(readerCoreKey, query);
      switch (indexId->get(readerCoreKey).intValue()) {
      case 1:
        missCount1::incrementAndGet();
        break;
      case 2:
        missCount2::incrementAndGet();
        break;
      default:
        throw make_shared<AssertionError>();
      }
    }

    void TestLRUQueryCache::onQueryCache(shared_ptr<Query> query,
                                         int64_t ramBytesUsed)
    {
      LuceneTestCase::onQueryCache(query, ramBytesUsed);
      assertNotNull(L"cached query is null", query);
      ramBytesUsage::addAndGet(ramBytesUsed);
    }

    void TestLRUQueryCache::onQueryEviction(shared_ptr<Query> query,
                                            int64_t ramBytesUsed)
    {
      LuceneTestCase::onQueryEviction(query, ramBytesUsed);
      assertNotNull(L"evicted query is null", query);
      ramBytesUsage::addAndGet(-ramBytesUsed);
    }

    void TestLRUQueryCache::onDocIdSetCache(any readerCoreKey,
                                            int64_t ramBytesUsed)
    {
      LuceneTestCase::onDocIdSetCache(readerCoreKey, ramBytesUsed);
      ramBytesUsage::addAndGet(ramBytesUsed);
      cacheSize::incrementAndGet();
    }

    void TestLRUQueryCache::onDocIdSetEviction(
        any readerCoreKey, int numEntries, int64_t sumRamBytesUsed)
    {
      LuceneTestCase::onDocIdSetEviction(readerCoreKey, numEntries,
                                         sumRamBytesUsed);
      ramBytesUsage::addAndGet(-sumRamBytesUsed);
      cacheSize::addAndGet(-numEntries);
    }

    void TestLRUQueryCache::onClear()
    {
      LuceneTestCase::onClear();
      ramBytesUsage::set(0);
      cacheSize::set(0);
    }
  }