using namespace std;

#include "TestIndexSearcher.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using Store = org::apache::lucene::document::Field::Store;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using IndexReader = org::apache::lucene::index::IndexReader;
using MultiReader = org::apache::lucene::index::MultiReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using NamedThreadFactory = org::apache::lucene::util::NamedThreadFactory;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::Test;

void TestIndexSearcher::setUp() 
{
  LuceneTestCase::setUp();
  dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  for (int i = 0; i < 100; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(
        newStringField(L"field", Integer::toString(i), Field::Store::NO));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(newStringField(L"field2", Boolean::toString(i % 2 == 0),
                                  Field::Store::NO));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<SortedDocValuesField>(
        L"field2", make_shared<BytesRef>(Boolean::toString(i % 2 == 0))));
    iw->addDocument(doc);
  }
  reader = iw->getReader();
  delete iw;
}

void TestIndexSearcher::tearDown() 
{
  LuceneTestCase::tearDown();
  delete reader;
  delete dir;
}

void TestIndexSearcher::testHugeN() 
{
  shared_ptr<ExecutorService> service = make_shared<ThreadPoolExecutor>(
      4, 4, 0LL, TimeUnit::MILLISECONDS,
      make_shared<LinkedBlockingQueue<Runnable>>(),
      make_shared<NamedThreadFactory>(L"TestIndexSearcher"));

  std::deque<std::shared_ptr<IndexSearcher>> searchers = {
      make_shared<IndexSearcher>(reader),
      make_shared<IndexSearcher>(reader, service)};
  std::deque<std::shared_ptr<Query>> queries = {
      make_shared<MatchAllDocsQuery>(),
      make_shared<TermQuery>(make_shared<Term>(L"field", L"1"))};
  std::deque<std::shared_ptr<Sort>> sorts = {
      nullptr, make_shared<Sort>(
                   make_shared<SortField>(L"field2", SortField::Type::STRING))};
  std::deque<std::shared_ptr<ScoreDoc>> afters = {
      nullptr, make_shared<FieldDoc>(
                   0, 0.0f, std::deque<any>{make_shared<BytesRef>(L"boo!")})};

  for (auto searcher : searchers) {
    for (auto after : afters) {
      for (auto query : queries) {
        for (auto sort : sorts) {
          searcher->search(query, numeric_limits<int>::max());
          searcher->searchAfter(after, query, numeric_limits<int>::max());
          if (sort != nullptr) {
            searcher->search(query, numeric_limits<int>::max(), sort);
            searcher->search(query, numeric_limits<int>::max(), sort, true,
                             true);
            searcher->search(query, numeric_limits<int>::max(), sort, true,
                             false);
            searcher->search(query, numeric_limits<int>::max(), sort, false,
                             true);
            searcher->search(query, numeric_limits<int>::max(), sort, false,
                             false);
            searcher->searchAfter(after, query, numeric_limits<int>::max(),
                                  sort);
            searcher->searchAfter(after, query, numeric_limits<int>::max(),
                                  sort, true, true);
            searcher->searchAfter(after, query, numeric_limits<int>::max(),
                                  sort, true, false);
            searcher->searchAfter(after, query, numeric_limits<int>::max(),
                                  sort, false, true);
            searcher->searchAfter(after, query, numeric_limits<int>::max(),
                                  sort, false, false);
          }
        }
      }
    }
  }

  TestUtil::shutdownExecutorService(service);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSearchAfterPassedMaxDoc() throws
// Exception
void TestIndexSearcher::testSearchAfterPassedMaxDoc() 
{
  // LUCENE-5128: ensure we get a meaningful message if searchAfter exceeds
  // maxDoc
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  w->addDocument(make_shared<Document>());
  shared_ptr<IndexReader> r = w->getReader();
  delete w;

  shared_ptr<IndexSearcher> s = make_shared<IndexSearcher>(r);
  expectThrows(invalid_argument::typeid, [&]() {
    s->searchAfter(make_shared<ScoreDoc>(r->maxDoc(), 0.54f),
                   make_shared<MatchAllDocsQuery>(), 10);
  });

  IOUtils::close({r, dir});
}

void TestIndexSearcher::testCount() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  constexpr int numDocs = atLeast(100);
  for (int i = 0; i < numDocs; ++i) {
    shared_ptr<Document> doc = make_shared<Document>();
    if (random()->nextBoolean()) {
      doc->push_back(
          make_shared<StringField>(L"foo", L"bar", Field::Store::NO));
    }
    if (random()->nextBoolean()) {
      doc->push_back(
          make_shared<StringField>(L"foo", L"baz", Field::Store::NO));
    }
    if (rarely()) {
      doc->push_back(
          make_shared<StringField>(L"delete", L"yes", Field::Store::NO));
    }
    w->addDocument(doc);
  }
  for (auto delete : std::deque<bool>{false, true}) {
    if (delete) {
      w->deleteDocuments(make_shared<Term>(L"delete", L"yes"));
    }
    shared_ptr<IndexReader> *const reader = w->getReader();
    shared_ptr<IndexSearcher> *const searcher = newSearcher(reader);
    // Test multiple queries, some of them are optimized by
    // IndexSearcher.count()
    for (shared_ptr<Query> query : Arrays::asList(
             make_shared<MatchAllDocsQuery>(), make_shared<MatchNoDocsQuery>(),
             make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
             make_shared<ConstantScoreQuery>(
                 make_shared<TermQuery>(make_shared<Term>(L"foo", L"baz"))),
             (make_shared<BooleanQuery::Builder>())
                 ->add(
                     make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                     Occur::SHOULD)
                 ->add(
                     make_shared<TermQuery>(make_shared<Term>(L"foo", L"baz")),
                     Occur::SHOULD)
                 ->build())) {
      TestUtil::assertEquals(searcher->count(query),
                             searcher->search(query, 1)->totalHits);
    }
    delete reader;
  }
  delete w;
  delete dir;
}

void TestIndexSearcher::testGetQueryCache() 
{
  shared_ptr<IndexSearcher> searcher =
      make_shared<IndexSearcher>(make_shared<MultiReader>());
  TestUtil::assertEquals(IndexSearcher::getDefaultQueryCache(),
                         searcher->getQueryCache());
  shared_ptr<QueryCache> dummyCache =
      make_shared<QueryCacheAnonymousInnerClass>(shared_from_this());
  searcher->setQueryCache(dummyCache);
  TestUtil::assertEquals(dummyCache, searcher->getQueryCache());

  IndexSearcher::setDefaultQueryCache(dummyCache);
  searcher = make_shared<IndexSearcher>(make_shared<MultiReader>());
  TestUtil::assertEquals(dummyCache, searcher->getQueryCache());

  searcher->setQueryCache(nullptr);
  assertNull(searcher->getQueryCache());

  IndexSearcher::setDefaultQueryCache(nullptr);
  searcher = make_shared<IndexSearcher>(make_shared<MultiReader>());
  assertNull(searcher->getQueryCache());
}

TestIndexSearcher::QueryCacheAnonymousInnerClass::QueryCacheAnonymousInnerClass(
    shared_ptr<TestIndexSearcher> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Weight> TestIndexSearcher::QueryCacheAnonymousInnerClass::doCache(
    shared_ptr<Weight> weight, shared_ptr<QueryCachingPolicy> policy)
{
  return weight;
}

void TestIndexSearcher::testGetQueryCachingPolicy() 
{
  shared_ptr<IndexSearcher> searcher =
      make_shared<IndexSearcher>(make_shared<MultiReader>());
  TestUtil::assertEquals(IndexSearcher::getDefaultQueryCachingPolicy(),
                         searcher->getQueryCachingPolicy());
  shared_ptr<QueryCachingPolicy> dummyPolicy =
      make_shared<QueryCachingPolicyAnonymousInnerClass>(shared_from_this());
  searcher->setQueryCachingPolicy(dummyPolicy);
  TestUtil::assertEquals(dummyPolicy, searcher->getQueryCachingPolicy());

  IndexSearcher::setDefaultQueryCachingPolicy(dummyPolicy);
  searcher = make_shared<IndexSearcher>(make_shared<MultiReader>());
  TestUtil::assertEquals(dummyPolicy, searcher->getQueryCachingPolicy());
}

TestIndexSearcher::QueryCachingPolicyAnonymousInnerClass::
    QueryCachingPolicyAnonymousInnerClass(
        shared_ptr<TestIndexSearcher> outerInstance)
{
  this->outerInstance = outerInstance;
}

bool TestIndexSearcher::QueryCachingPolicyAnonymousInnerClass::shouldCache(
    shared_ptr<Query> query) 
{
  return false;
}

void TestIndexSearcher::QueryCachingPolicyAnonymousInnerClass::onUse(
    shared_ptr<Query> query)
{
}
} // namespace org::apache::lucene::search