using namespace std;

#include "TestSearcherTaxonomyManager.h"

namespace org::apache::lucene::facet::taxonomy
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using FacetField = org::apache::lucene::facet::FacetField;
using FacetResult = org::apache::lucene::facet::FacetResult;
using FacetTestCase = org::apache::lucene::facet::FacetTestCase;
using Facets = org::apache::lucene::facet::Facets;
using FacetsCollector = org::apache::lucene::facet::FacetsCollector;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using SearcherAndTaxonomy = org::apache::lucene::facet::taxonomy::
    SearcherTaxonomyManager::SearcherAndTaxonomy;
using DirectoryTaxonomyWriter =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyWriter;
using IndexNotFoundException =
    org::apache::lucene::index::IndexNotFoundException;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using SegmentInfos = org::apache::lucene::index::SegmentInfos;
using TieredMergePolicy = org::apache::lucene::index::TieredMergePolicy;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using ReferenceManager = org::apache::lucene::search::ReferenceManager;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using TestUtil = org::apache::lucene::util::TestUtil;

TestSearcherTaxonomyManager::IndexerThread::IndexerThread(
    shared_ptr<IndexWriter> w, shared_ptr<FacetsConfig> config,
    shared_ptr<TaxonomyWriter> tw,
    shared_ptr<ReferenceManager<std::shared_ptr<SearcherAndTaxonomy>>> mgr,
    int ordLimit, shared_ptr<AtomicBoolean> stop)
{
  this->w = w;
  this->config = config;
  this->tw = tw;
  this->mgr = mgr;
  this->ordLimit = ordLimit;
  this->stop = stop;
}

void TestSearcherTaxonomyManager::IndexerThread::run()
{
  try {
    shared_ptr<Set<wstring>> seen = unordered_set<wstring>();
    deque<wstring> paths = deque<wstring>();
    while (true) {
      shared_ptr<Document> doc = make_shared<Document>();
      int numPaths = TestUtil::nextInt(random(), 1, 5);
      for (int i = 0; i < numPaths; i++) {
        wstring path;
        if (!paths.empty() && random()->nextInt(5) != 4) {
          // Use previous path
          path = paths[random()->nextInt(paths.size())];
        } else {
          // Create new path
          path = L"";
          while (true) {
            path = TestUtil::randomRealisticUnicodeString(random());
            if (path.length() != 0 && !seen->contains(path)) {
              seen->add(path);
              paths.push_back(path);
              break;
            }
          }
        }
        doc->push_back(make_shared<FacetField>(L"field", path));
      }
      try {
        w->addDocument(config->build(tw, doc));
        if (mgr != nullptr && random()->nextDouble() < 0.02) {
          w->commit();
          tw->commit();
          mgr->maybeRefresh();
        }
      } catch (const IOException &ioe) {
        throw runtime_error(ioe);
      }

      if (VERBOSE) {
        wcout << L"TW size=" << tw->getSize() << L" vs " << ordLimit << endl;
      }

      if (tw->getSize() >= ordLimit) {
        break;
      }
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    stop->set(true);
  }
}

void TestSearcherTaxonomyManager::testNRT() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  // Don't allow tiny maxBufferedDocs; it can make this
  // test too slow:
  iwc->setMaxBufferedDocs(max(500, iwc->getMaxBufferedDocs()));

  // MockRandom/AlcololicMergePolicy are too slow:
  shared_ptr<TieredMergePolicy> tmp = make_shared<TieredMergePolicy>();
  tmp->setFloorSegmentMB(.001);
  iwc->setMergePolicy(tmp);
  shared_ptr<IndexWriter> *const w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<DirectoryTaxonomyWriter> *const tw =
      make_shared<DirectoryTaxonomyWriter>(taxoDir);
  shared_ptr<FacetsConfig> *const config = make_shared<FacetsConfig>();
  config->setMultiValued(L"field", true);
  shared_ptr<AtomicBoolean> *const stop = make_shared<AtomicBoolean>();

  // How many unique facets to index before stopping:
  constexpr int ordLimit = TEST_NIGHTLY ? 100000 : 6000;

  shared_ptr<Thread> indexer =
      make_shared<IndexerThread>(w, config, tw, nullptr, ordLimit, stop);

  shared_ptr<SearcherTaxonomyManager> *const mgr =
      make_shared<SearcherTaxonomyManager>(w, true, nullptr, tw);

  shared_ptr<Thread> reopener =
      make_shared<ThreadAnonymousInnerClass>(shared_from_this(), stop, mgr);

  reopener->setName(L"reopener");
  reopener->start();

  indexer->setName(L"indexer");
  indexer->start();

  try {
    while (!stop->get()) {
      shared_ptr<SearcherAndTaxonomy> pair = mgr->acquire();
      try {
        // System.out.println("search maxOrd=" + pair.taxonomyReader.getSize());
        shared_ptr<FacetsCollector> sfc = make_shared<FacetsCollector>();
        pair->searcher->search(make_shared<MatchAllDocsQuery>(), sfc);
        shared_ptr<Facets> facets =
            getTaxonomyFacetCounts(pair->taxonomyReader, config, sfc);
        shared_ptr<FacetResult> result = facets->getTopChildren(10, L"field");
        if (pair->searcher->getIndexReader()->numDocs() > 0) {
          // System.out.println(pair.taxonomyReader.getSize());
          assertTrue(result->childCount > 0);
          assertTrue(result->labelValues.size() > 0);
        }

        // if (VERBOSE) {
        // System.out.println("TEST: facets=" +
        // FacetTestUtils.toString(results.get(0)));
        //}
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        mgr->release(pair);
      }
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    indexer->join();
    reopener->join();
  }

  if (VERBOSE) {
    wcout << L"TEST: now stop" << endl;
  }

  delete w;
  IOUtils::close({mgr, tw, taxoDir, dir});
}

TestSearcherTaxonomyManager::ThreadAnonymousInnerClass::
    ThreadAnonymousInnerClass(
        shared_ptr<TestSearcherTaxonomyManager> outerInstance,
        shared_ptr<AtomicBoolean> stop,
        shared_ptr<
            org::apache::lucene::facet::taxonomy::SearcherTaxonomyManager>
            mgr)
{
  this->outerInstance = outerInstance;
  this->stop = stop;
  this->mgr = mgr;
}

void TestSearcherTaxonomyManager::ThreadAnonymousInnerClass::run()
{
  while (!stop->get()) {
    try {
      // Sleep for up to 20 msec:
      delay(random()->nextInt(20));

      if (VERBOSE) {
        wcout << L"TEST: reopen" << endl;
      }

      mgr->maybeRefresh();

      if (VERBOSE) {
        wcout << L"TEST: reopen done" << endl;
      }
    } catch (const runtime_error &ioe) {
      throw runtime_error(ioe);
    }
  }
}

void TestSearcherTaxonomyManager::testDirectory() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();
  shared_ptr<IndexWriter> *const w = make_shared<IndexWriter>(
      indexDir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<DirectoryTaxonomyWriter> *const tw =
      make_shared<DirectoryTaxonomyWriter>(taxoDir);
  // first empty commit
  w->commit();
  tw->commit();
  shared_ptr<SearcherTaxonomyManager> *const mgr =
      make_shared<SearcherTaxonomyManager>(indexDir, taxoDir, nullptr);
  shared_ptr<FacetsConfig> *const config = make_shared<FacetsConfig>();
  config->setMultiValued(L"field", true);
  shared_ptr<AtomicBoolean> *const stop = make_shared<AtomicBoolean>();

  // How many unique facets to index before stopping:
  constexpr int ordLimit = TEST_NIGHTLY ? 100000 : 6000;

  shared_ptr<Thread> indexer =
      make_shared<IndexerThread>(w, config, tw, mgr, ordLimit, stop);
  indexer->start();

  try {
    while (!stop->get()) {
      shared_ptr<SearcherAndTaxonomy> pair = mgr->acquire();
      try {
        // System.out.println("search maxOrd=" + pair.taxonomyReader.getSize());
        shared_ptr<FacetsCollector> sfc = make_shared<FacetsCollector>();
        pair->searcher->search(make_shared<MatchAllDocsQuery>(), sfc);
        shared_ptr<Facets> facets =
            getTaxonomyFacetCounts(pair->taxonomyReader, config, sfc);
        shared_ptr<FacetResult> result = facets->getTopChildren(10, L"field");
        if (pair->searcher->getIndexReader()->numDocs() > 0) {
          // System.out.println(pair.taxonomyReader.getSize());
          assertTrue(result->childCount > 0);
          assertTrue(result->labelValues.size() > 0);
        }

        // if (VERBOSE) {
        // System.out.println("TEST: facets=" +
        // FacetTestUtils.toString(results.get(0)));
        //}
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        mgr->release(pair);
      }
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    indexer->join();
  }

  if (VERBOSE) {
    wcout << L"TEST: now stop" << endl;
  }

  delete w;
  IOUtils::close({mgr, tw, taxoDir, indexDir});
}

void TestSearcherTaxonomyManager::testReplaceTaxonomyNRT() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<DirectoryTaxonomyWriter> tw =
      make_shared<DirectoryTaxonomyWriter>(taxoDir);

  shared_ptr<Directory> taxoDir2 = newDirectory();
  shared_ptr<DirectoryTaxonomyWriter> tw2 =
      make_shared<DirectoryTaxonomyWriter>(taxoDir2);
  delete tw2;

  shared_ptr<SearcherTaxonomyManager> mgr =
      make_shared<SearcherTaxonomyManager>(w, true, nullptr, tw);
  w->addDocument(make_shared<Document>());
  tw->replaceTaxonomy(taxoDir2);
  delete taxoDir2;

  expectThrows(IllegalStateException::typeid, [&]() { mgr->maybeRefresh(); });

  delete w;
  IOUtils::close({mgr, tw, taxoDir, dir});
}

void TestSearcherTaxonomyManager::testReplaceTaxonomyDirectory() throw(
    runtime_error)
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      indexDir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<DirectoryTaxonomyWriter> tw =
      make_shared<DirectoryTaxonomyWriter>(taxoDir);
  w->commit();
  tw->commit();

  shared_ptr<Directory> taxoDir2 = newDirectory();
  shared_ptr<DirectoryTaxonomyWriter> tw2 =
      make_shared<DirectoryTaxonomyWriter>(taxoDir2);
  tw2->addCategory(make_shared<FacetLabel>(L"a", L"b"));
  delete tw2;

  shared_ptr<SearcherTaxonomyManager> mgr =
      make_shared<SearcherTaxonomyManager>(indexDir, taxoDir, nullptr);
  shared_ptr<SearcherAndTaxonomy> pair = mgr->acquire();
  try {
    TestUtil::assertEquals(1, pair->taxonomyReader->getSize());
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    mgr->release(pair);
  }

  w->addDocument(make_shared<Document>());
  tw->replaceTaxonomy(taxoDir2);
  delete taxoDir2;
  w->commit();
  tw->commit();

  mgr->maybeRefresh();
  pair = mgr->acquire();
  try {
    TestUtil::assertEquals(3, pair->taxonomyReader->getSize());
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    mgr->release(pair);
  }

  delete w;
  IOUtils::close({mgr, tw, taxoDir, indexDir});
}

void TestSearcherTaxonomyManager::testExceptionDuringRefresh() throw(
    runtime_error)
{

  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();

  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      indexDir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<DirectoryTaxonomyWriter> tw =
      make_shared<DirectoryTaxonomyWriter>(taxoDir);
  w->commit();
  tw->commit();

  shared_ptr<SearcherTaxonomyManager> mgr =
      make_shared<SearcherTaxonomyManager>(indexDir, taxoDir, nullptr);

  tw->addCategory(make_shared<FacetLabel>(L"a", L"b"));
  w->addDocument(make_shared<Document>());

  tw->commit();
  w->commit();

  // intentionally corrupt the taxo index:
  shared_ptr<SegmentInfos> infos = SegmentInfos::readLatestCommit(taxoDir);
  taxoDir->deleteFile(infos->getSegmentsFileName());
  expectThrows(IndexNotFoundException::typeid, mgr::maybeRefreshBlocking);
  IOUtils::close({w, tw, mgr, indexDir, taxoDir});
}
} // namespace org::apache::lucene::facet::taxonomy