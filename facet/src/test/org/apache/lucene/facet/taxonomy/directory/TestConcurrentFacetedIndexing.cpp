using namespace std;

#include "TestConcurrentFacetedIndexing.h"

namespace org::apache::lucene::facet::taxonomy::directory
{
using Document = org::apache::lucene::document::Document;
using FacetField = org::apache::lucene::facet::FacetField;
using FacetTestCase = org::apache::lucene::facet::FacetTestCase;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using FacetLabel = org::apache::lucene::facet::taxonomy::FacetLabel;
using LruTaxonomyWriterCache =
    org::apache::lucene::facet::taxonomy::writercache::LruTaxonomyWriterCache;
using TaxonomyWriterCache =
    org::apache::lucene::facet::taxonomy::writercache::TaxonomyWriterCache;
using UTF8TaxonomyWriterCache =
    org::apache::lucene::facet::taxonomy::writercache::UTF8TaxonomyWriterCache;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
shared_ptr<
    org::apache::lucene::facet::taxonomy::writercache::TaxonomyWriterCache>
    TestConcurrentFacetedIndexing::NO_OP_CACHE =
        make_shared<TaxonomyWriterCacheAnonymousInnerClass>();

TestConcurrentFacetedIndexing::TaxonomyWriterCacheAnonymousInnerClass::
    TaxonomyWriterCacheAnonymousInnerClass()
{
}

TestConcurrentFacetedIndexing::TaxonomyWriterCacheAnonymousInnerClass::
    ~TaxonomyWriterCacheAnonymousInnerClass()
{
}

int TestConcurrentFacetedIndexing::TaxonomyWriterCacheAnonymousInnerClass::get(
    shared_ptr<FacetLabel> categoryPath)
{
  return -1;
}

bool TestConcurrentFacetedIndexing::TaxonomyWriterCacheAnonymousInnerClass::put(
    shared_ptr<FacetLabel> categoryPath, int ordinal)
{
  return true;
}

bool TestConcurrentFacetedIndexing::TaxonomyWriterCacheAnonymousInnerClass::
    isFull()
{
  return true;
}

void TestConcurrentFacetedIndexing::TaxonomyWriterCacheAnonymousInnerClass::
    clear()
{
}

int TestConcurrentFacetedIndexing::TaxonomyWriterCacheAnonymousInnerClass::
    size()
{
  return 0;
}

shared_ptr<FacetField> TestConcurrentFacetedIndexing::newCategory()
{
  shared_ptr<Random> r = random();
  wstring l1 = L"l1." + r->nextInt(10);  // l1.0-l1.9 (10 categories)
  wstring l2 = L"l2." + r->nextInt(30);  // l2.0-l2.29 (30 categories)
  wstring l3 = L"l3." + r->nextInt(100); // l3.0-l3.99 (100 categories)
  return make_shared<FacetField>(l1, l2, l3);
}

shared_ptr<TaxonomyWriterCache>
TestConcurrentFacetedIndexing::newTaxoWriterCache(int ndocs)
{
  constexpr double d = random()->nextDouble();
  if (d < 0.7) {
    // this is the fastest, yet most memory consuming
    return make_shared<UTF8TaxonomyWriterCache>();
  } else if (TEST_NIGHTLY && d > 0.98) {
    // this is the slowest, but tests the writer concurrency when no caching is
    // done. only pick it during NIGHTLY tests, and even then, with very low
    // chances.
    return NO_OP_CACHE;
  } else {
    // this is slower than UTF8, but less memory consuming, and exercises
    // finding categories on disk too.
    return make_shared<LruTaxonomyWriterCache>(ndocs / 10);
  }
}

void TestConcurrentFacetedIndexing::testConcurrency() 
{
  shared_ptr<AtomicInteger> *const numDocs =
      make_shared<AtomicInteger>(atLeast(10000));
  shared_ptr<Directory> *const indexDir = newDirectory();
  shared_ptr<Directory> *const taxoDir = newDirectory();
  shared_ptr<ConcurrentHashMap<wstring, wstring>> *const values =
      make_shared<ConcurrentHashMap<wstring, wstring>>();
  shared_ptr<IndexWriter> *const iw =
      make_shared<IndexWriter>(indexDir, newIndexWriterConfig(nullptr));
  shared_ptr<DirectoryTaxonomyWriter> *const tw =
      make_shared<DirectoryTaxonomyWriter>(taxoDir, OpenMode::CREATE,
                                           newTaxoWriterCache(numDocs->get()));
  std::deque<std::shared_ptr<Thread>> indexThreads(atLeast(4));
  shared_ptr<FacetsConfig> *const config = make_shared<FacetsConfig>();
  for (int i = 0; i < 10; i++) {
    config->setHierarchical(L"l1." + to_wstring(i), true);
    config->setMultiValued(L"l1." + to_wstring(i), true);
  }

  for (int i = 0; i < indexThreads.size(); i++) {
    indexThreads[i] = make_shared<ThreadAnonymousInnerClass>(
        shared_from_this(), numDocs, values, iw, tw, config);
  }

  for (auto t : indexThreads) {
    t->start();
  }
  for (auto t : indexThreads) {
    t->join();
  }

  shared_ptr<DirectoryTaxonomyReader> tr =
      make_shared<DirectoryTaxonomyReader>(tw);
  // +1 for root category
  if (values->size() + 1 != tr->getSize()) {
    for (auto value : values) {
      shared_ptr<FacetLabel> label =
          make_shared<FacetLabel>(FacetsConfig::stringToPath(value.first));
      if (tr->getOrdinal(label) == -1) {
        wcout << L"FAIL: path=" << label << L" not recognized" << endl;
      }
    }
    fail(L"mismatch number of categories");
  }
  std::deque<int> parents = tr->getParallelTaxonomyArrays()->parents();
  for (auto cat : values) {
    shared_ptr<FacetLabel> cp =
        make_shared<FacetLabel>(FacetsConfig::stringToPath(cat.first));
    assertTrue(L"category not found " + cp, tr->getOrdinal(cp) > 0);
    int level = cp->length;
    int parentOrd = 0; // for root, parent is always virtual ROOT (ord=0)
    shared_ptr<FacetLabel> path = nullptr;
    for (int i = 0; i < level; i++) {
      path = cp->subpath(i + 1);
      int ord = tr->getOrdinal(path);
      assertEquals(L"invalid parent for cp=" + path, parentOrd, parents[ord]);
      parentOrd = ord; // next level should have this parent
    }
  }

  delete iw;
  IOUtils::close({tw, tr, taxoDir, indexDir});
}

TestConcurrentFacetedIndexing::ThreadAnonymousInnerClass::
    ThreadAnonymousInnerClass(
        shared_ptr<TestConcurrentFacetedIndexing> outerInstance,
        shared_ptr<AtomicInteger> numDocs,
        shared_ptr<ConcurrentHashMap<wstring, wstring>> values,
        shared_ptr<IndexWriter> iw,
        shared_ptr<org::apache::lucene::facet::taxonomy::directory::
                       DirectoryTaxonomyWriter>
            tw,
        shared_ptr<FacetsConfig> config)
{
  this->outerInstance = outerInstance;
  this->numDocs = numDocs;
  this->values = values;
  this->iw = iw;
  this->tw = tw;
  this->config = config;
}

void TestConcurrentFacetedIndexing::ThreadAnonymousInnerClass::run()
{
  shared_ptr<Random> random = TestConcurrentFacetedIndexing::random();
  while (numDocs->decrementAndGet() > 0) {
    try {
      shared_ptr<Document> doc = make_shared<Document>();
      int numCats = random->nextInt(3) + 1; // 1-3
      while (numCats-- > 0) {
        shared_ptr<FacetField> ff = newCategory();
        doc->push_back(ff);

        shared_ptr<FacetLabel> label =
            make_shared<FacetLabel>(ff->dim, ff->path);
        // add all prefixes to values
        int level = label->length;
        while (level > 0) {
          wstring s = FacetsConfig::pathToString(label->components, level);
          values->put(s, s);
          --level;
        }
      }
      iw->addDocument(config->build(tw, doc));
    } catch (const IOException &e) {
      throw runtime_error(e);
    }
  }
}
} // namespace org::apache::lucene::facet::taxonomy::directory