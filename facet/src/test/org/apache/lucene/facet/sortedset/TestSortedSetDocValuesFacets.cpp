using namespace std;

#include "TestSortedSetDocValuesFacets.h"

namespace org::apache::lucene::facet::sortedset
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DrillDownQuery = org::apache::lucene::facet::DrillDownQuery;
using FacetResult = org::apache::lucene::facet::FacetResult;
using FacetTestCase = org::apache::lucene::facet::FacetTestCase;
using Facets = org::apache::lucene::facet::Facets;
using FacetsCollector = org::apache::lucene::facet::FacetsCollector;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using LabelAndValue = org::apache::lucene::facet::LabelAndValue;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using Accountable = org::apache::lucene::util::Accountable;
using IOUtils = org::apache::lucene::util::IOUtils;
using NamedThreadFactory = org::apache::lucene::util::NamedThreadFactory;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestSortedSetDocValuesFacets::testBasic() 
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();
  config->setMultiValued(L"a", true);
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesFacetField>(L"a", L"foo"));
  doc->push_back(make_shared<SortedSetDocValuesFacetField>(L"a", L"bar"));
  doc->push_back(make_shared<SortedSetDocValuesFacetField>(L"a", L"zoo"));
  doc->push_back(make_shared<SortedSetDocValuesFacetField>(L"b", L"baz"));
  writer->addDocument(config->build(doc));
  if (random()->nextBoolean()) {
    writer->commit();
  }

  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesFacetField>(L"a", L"foo"));
  writer->addDocument(config->build(doc));

  // NRT open
  shared_ptr<IndexSearcher> searcher = newSearcher(writer->getReader());

  // Per-top-reader state:
  shared_ptr<SortedSetDocValuesReaderState> state =
      make_shared<DefaultSortedSetDocValuesReaderState>(
          searcher->getIndexReader());

  shared_ptr<ExecutorService> exec = randomExecutorServiceOrNull();
  shared_ptr<Facets> facets = getAllFacets(searcher, state, exec);

  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"dim=a path=[] value=4 childCount=3\n  foo (2)\n  bar (1)\n  zoo (1)\n",
      facets->getTopChildren(10, L"a")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"dim=b path=[] value=1 childCount=1\n  baz (1)\n",
                         facets->getTopChildren(10, L"b")->toString());

  // DrillDown:
  shared_ptr<DrillDownQuery> q = make_shared<DrillDownQuery>(config);
  q->add(L"a", {L"foo"});
  q->add(L"b", {L"baz"});
  shared_ptr<TopDocs> hits = searcher->search(q, 1);
  TestUtil::assertEquals(1, hits->totalHits);

  if (exec != nullptr) {
    exec->shutdownNow();
  }
  delete writer;
  IOUtils::close({searcher->getIndexReader(), dir});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("unused") public void testStaleState()
// throws Exception
void TestSortedSetDocValuesFacets::testStaleState() 
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesFacetField>(L"a", L"foo"));
  writer->addDocument(config->build(doc));

  shared_ptr<IndexReader> r = writer->getReader();
  shared_ptr<SortedSetDocValuesReaderState> state =
      make_shared<DefaultSortedSetDocValuesReaderState>(r);

  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesFacetField>(L"a", L"bar"));
  writer->addDocument(config->build(doc));

  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesFacetField>(L"a", L"baz"));
  writer->addDocument(config->build(doc));

  shared_ptr<IndexSearcher> searcher = newSearcher(writer->getReader());

  shared_ptr<FacetsCollector> c = make_shared<FacetsCollector>();

  searcher->search(make_shared<MatchAllDocsQuery>(), c);

  expectThrows(IllegalStateException::typeid,
               [&]() { make_shared<SortedSetDocValuesFacetCounts>(state, c); });

  delete r;
  delete writer;
  delete searcher->getIndexReader();
  delete dir;
}

void TestSortedSetDocValuesFacets::testSparseFacets() 
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesFacetField>(L"a", L"foo1"));
  writer->addDocument(config->build(doc));

  if (random()->nextBoolean()) {
    writer->commit();
  }

  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesFacetField>(L"a", L"foo2"));
  doc->push_back(make_shared<SortedSetDocValuesFacetField>(L"b", L"bar1"));
  writer->addDocument(config->build(doc));

  if (random()->nextBoolean()) {
    writer->commit();
  }

  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesFacetField>(L"a", L"foo3"));
  doc->push_back(make_shared<SortedSetDocValuesFacetField>(L"b", L"bar2"));
  doc->push_back(make_shared<SortedSetDocValuesFacetField>(L"c", L"baz1"));
  writer->addDocument(config->build(doc));

  // NRT open
  shared_ptr<IndexSearcher> searcher = newSearcher(writer->getReader());
  delete writer;

  // Per-top-reader state:
  shared_ptr<SortedSetDocValuesReaderState> state =
      make_shared<DefaultSortedSetDocValuesReaderState>(
          searcher->getIndexReader());

  shared_ptr<ExecutorService> exec = randomExecutorServiceOrNull();
  shared_ptr<Facets> facets = getAllFacets(searcher, state, exec);

  // Ask for top 10 labels for any dims that have counts:
  deque<std::shared_ptr<FacetResult>> results = facets->getAllDims(10);

  TestUtil::assertEquals(3, results.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"dim=a path=[] value=3 childCount=3\n  foo1 (1)\n  "
                         L"foo2 (1)\n  foo3 (1)\n",
                         results[0]->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"dim=b path=[] value=2 childCount=2\n  bar1 (1)\n  bar2 (1)\n",
      results[1]->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"dim=c path=[] value=1 childCount=1\n  baz1 (1)\n",
                         results[2]->toString());

  shared_ptr<deque<std::shared_ptr<Accountable>>> resources =
      state->getChildResources();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(
      state->toString()->contains(FacetsConfig::DEFAULT_INDEX_FIELD_NAME));
  if (searcher->getIndexReader()->leaves().size() > 1) {
    assertTrue(state->ramBytesUsed() > 0);
    assertFalse(resources->isEmpty());
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertTrue(resources->toString()->contains(
        FacetsConfig::DEFAULT_INDEX_FIELD_NAME));
  } else {
    TestUtil::assertEquals(0, state->ramBytesUsed());
    assertTrue(resources->isEmpty());
  }

  if (exec != nullptr) {
    exec->shutdownNow();
  }
  delete searcher->getIndexReader();
  delete dir;
}

void TestSortedSetDocValuesFacets::testSomeSegmentsMissing() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesFacetField>(L"a", L"foo1"));
  writer->addDocument(config->build(doc));
  writer->commit();

  doc = make_shared<Document>();
  writer->addDocument(config->build(doc));
  writer->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesFacetField>(L"a", L"foo2"));
  writer->addDocument(config->build(doc));
  writer->commit();

  // NRT open
  shared_ptr<IndexSearcher> searcher = newSearcher(writer->getReader());
  delete writer;

  // Per-top-reader state:
  shared_ptr<SortedSetDocValuesReaderState> state =
      make_shared<DefaultSortedSetDocValuesReaderState>(
          searcher->getIndexReader());

  shared_ptr<ExecutorService> exec = randomExecutorServiceOrNull();
  shared_ptr<Facets> facets = getAllFacets(searcher, state, exec);

  // Ask for top 10 labels for any dims that have counts:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"dim=a path=[] value=2 childCount=2\n  foo1 (1)\n  foo2 (1)\n",
      facets->getTopChildren(10, L"a")->toString());

  if (exec != nullptr) {
    exec->shutdownNow();
  }
  delete searcher->getIndexReader();
  delete dir;
}

void TestSortedSetDocValuesFacets::testRandom() 
{
  std::deque<wstring> tokens = getRandomTokens(10);
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();

  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), indexDir);
  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();
  int numDocs = atLeast(1000);
  int numDims = TestUtil::nextInt(random(), 1, 7);
  deque<std::shared_ptr<FacetTestCase::TestDoc>> testDocs =
      getRandomDocs(tokens, numDocs, numDims);
  for (auto testDoc : testDocs) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newStringField(L"content", testDoc->content, Field::Store::NO));
    for (int j = 0; j < numDims; j++) {
      if (testDoc->dims[j] != L"") {
        doc->push_back(make_shared<SortedSetDocValuesFacetField>(
            L"dim" + to_wstring(j), testDoc->dims[j]));
      }
    }
    w->addDocument(config->build(doc));
  }

  // NRT open
  shared_ptr<IndexSearcher> searcher = newSearcher(w->getReader());

  // Per-top-reader state:
  shared_ptr<SortedSetDocValuesReaderState> state =
      make_shared<DefaultSortedSetDocValuesReaderState>(
          searcher->getIndexReader());
  shared_ptr<ExecutorService> exec = randomExecutorServiceOrNull();

  int iters = atLeast(100);
  for (int iter = 0; iter < iters; iter++) {
    wstring searchToken = tokens[random()->nextInt(tokens.size())];
    if (VERBOSE) {
      wcout << L"\nTEST: iter content=" << searchToken << endl;
    }
    shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>();
    FacetsCollector::search(
        searcher,
        make_shared<TermQuery>(make_shared<Term>(L"content", searchToken)), 10,
        fc);
    shared_ptr<Facets> facets;
    if (exec != nullptr) {
      facets =
          make_shared<ConcurrentSortedSetDocValuesFacetCounts>(state, fc, exec);
    } else {
      facets = make_shared<SortedSetDocValuesFacetCounts>(state, fc);
    }

    // Slow, yet hopefully bug-free, faceting:
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings({"rawtypes","unchecked"})
    // java.util.Map<std::wstring,int>[] expectedCounts = new
    // java.util.HashMap[numDims];
    std::deque<unordered_map<wstring, int>> expectedCounts =
        std::deque<unordered_map>(numDims);
    for (int i = 0; i < numDims; i++) {
      expectedCounts[i] = unordered_map<>();
    }

    for (auto doc : testDocs) {
      if (doc->content == searchToken) {
        for (int j = 0; j < numDims; j++) {
          if (doc->dims[j] != L"") {
            optional<int> v = expectedCounts[j][doc->dims[j]];
            if (!v) {
              expectedCounts[j].emplace(doc->dims[j], 1);
            } else {
              expectedCounts[j].emplace(doc->dims[j], v.value() + 1);
            }
          }
        }
      }
    }

    deque<std::shared_ptr<FacetResult>> expected =
        deque<std::shared_ptr<FacetResult>>();
    for (int i = 0; i < numDims; i++) {
      deque<std::shared_ptr<LabelAndValue>> labelValues =
          deque<std::shared_ptr<LabelAndValue>>();
      int totCount = 0;
      for (auto ent : expectedCounts[i]) {
        labelValues.push_back(
            make_shared<LabelAndValue>(ent.first, ent.second));
        totCount += ent.second;
      }
      sortLabelValues(labelValues);
      if (totCount > 0) {
        expected.push_back(make_shared<FacetResult>(
            L"dim" + to_wstring(i), std::deque<wstring>(0), totCount,
            labelValues.toArray(std::deque<std::shared_ptr<LabelAndValue>>(
                labelValues.size())),
            labelValues.size()));
      }
    }

    // Sort by highest value, tie break by value:
    sortFacetResults(expected);

    deque<std::shared_ptr<FacetResult>> actual = facets->getAllDims(10);

    // Messy: fixup ties
    // sortTies(actual);

    TestUtil::assertEquals(expected, actual);
  }

  if (exec != nullptr) {
    exec->shutdownNow();
  }
  delete w;
  IOUtils::close({searcher->getIndexReader(), indexDir, taxoDir});
}

shared_ptr<Facets> TestSortedSetDocValuesFacets::getAllFacets(
    shared_ptr<IndexSearcher> searcher,
    shared_ptr<SortedSetDocValuesReaderState> state,
    shared_ptr<ExecutorService> exec) 
{
  if (random()->nextBoolean()) {
    shared_ptr<FacetsCollector> c = make_shared<FacetsCollector>();
    searcher->search(make_shared<MatchAllDocsQuery>(), c);
    if (exec != nullptr) {
      return make_shared<ConcurrentSortedSetDocValuesFacetCounts>(state, c,
                                                                  exec);
    } else {
      return make_shared<SortedSetDocValuesFacetCounts>(state, c);
    }
  } else if (exec != nullptr) {
    return make_shared<ConcurrentSortedSetDocValuesFacetCounts>(state, exec);
  } else {
    return make_shared<SortedSetDocValuesFacetCounts>(state);
  }
}

shared_ptr<ExecutorService>
TestSortedSetDocValuesFacets::randomExecutorServiceOrNull()
{
  if (random()->nextBoolean()) {
    return nullptr;
  } else {
    return make_shared<ThreadPoolExecutor>(
        1, TestUtil::nextInt(random(), 2, 6), numeric_limits<int64_t>::max(),
        TimeUnit::MILLISECONDS, make_shared<LinkedBlockingQueue<Runnable>>(),
        make_shared<NamedThreadFactory>(L"TestIndexSearcher"));
  }
}
} // namespace org::apache::lucene::facet::sortedset