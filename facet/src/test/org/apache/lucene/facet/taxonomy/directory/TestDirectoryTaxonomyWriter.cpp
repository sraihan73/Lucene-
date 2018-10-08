using namespace std;

#include "TestDirectoryTaxonomyWriter.h"

namespace org::apache::lucene::facet::taxonomy::directory
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using DrillDownQuery = org::apache::lucene::facet::DrillDownQuery;
using FacetField = org::apache::lucene::facet::FacetField;
using FacetTestCase = org::apache::lucene::facet::FacetTestCase;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using FacetLabel = org::apache::lucene::facet::taxonomy::FacetLabel;
using TaxonomyReader = org::apache::lucene::facet::taxonomy::TaxonomyReader;
using MemoryOrdinalMap = org::apache::lucene::facet::taxonomy::directory::
    DirectoryTaxonomyWriter::MemoryOrdinalMap;
using LruTaxonomyWriterCache =
    org::apache::lucene::facet::taxonomy::writercache::LruTaxonomyWriterCache;
using TaxonomyWriterCache =
    org::apache::lucene::facet::taxonomy::writercache::TaxonomyWriterCache;
using UTF8TaxonomyWriterCache =
    org::apache::lucene::facet::taxonomy::writercache::UTF8TaxonomyWriterCache;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using SegmentInfos = org::apache::lucene::index::SegmentInfos;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::Test;
shared_ptr<
    org::apache::lucene::facet::taxonomy::writercache::TaxonomyWriterCache>
    TestDirectoryTaxonomyWriter::NO_OP_CACHE =
        make_shared<TaxonomyWriterCacheAnonymousInnerClass>();

TestDirectoryTaxonomyWriter::TaxonomyWriterCacheAnonymousInnerClass::
    TaxonomyWriterCacheAnonymousInnerClass()
{
}

TestDirectoryTaxonomyWriter::TaxonomyWriterCacheAnonymousInnerClass::
    ~TaxonomyWriterCacheAnonymousInnerClass()
{
}

int TestDirectoryTaxonomyWriter::TaxonomyWriterCacheAnonymousInnerClass::get(
    shared_ptr<FacetLabel> categoryPath)
{
  return -1;
}

bool TestDirectoryTaxonomyWriter::TaxonomyWriterCacheAnonymousInnerClass::put(
    shared_ptr<FacetLabel> categoryPath, int ordinal)
{
  return true;
}

bool TestDirectoryTaxonomyWriter::TaxonomyWriterCacheAnonymousInnerClass::
    isFull()
{
  return true;
}

void TestDirectoryTaxonomyWriter::TaxonomyWriterCacheAnonymousInnerClass::
    clear()
{
}

int TestDirectoryTaxonomyWriter::TaxonomyWriterCacheAnonymousInnerClass::size()
{
  return 0;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testCommit() throws Exception
void TestDirectoryTaxonomyWriter::testCommit() 
{
  // Verifies that nothing is committed to the underlying Directory, if
  // commit() wasn't called.
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<DirectoryTaxonomyWriter> ltw =
      make_shared<DirectoryTaxonomyWriter>(dir, OpenMode::CREATE_OR_APPEND,
                                           NO_OP_CACHE);
  assertFalse(DirectoryReader::indexExists(dir));
  ltw->commit(); // first commit, so that an index will be created
  ltw->addCategory(make_shared<FacetLabel>(L"a"));

  shared_ptr<IndexReader> r = DirectoryReader::open(dir);
  assertEquals(
      L"No categories should have been committed to the underlying directory",
      1, r->numDocs());
  delete r;
  delete ltw;
  delete dir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testCommitUserData() throws Exception
void TestDirectoryTaxonomyWriter::testCommitUserData() 
{
  // Verifies taxonomy commit data
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(dir, OpenMode::CREATE_OR_APPEND,
                                           NO_OP_CACHE);
  taxoWriter->addCategory(make_shared<FacetLabel>(L"a"));
  taxoWriter->addCategory(make_shared<FacetLabel>(L"b"));
  unordered_map<wstring, wstring> userCommitData =
      unordered_map<wstring, wstring>();
  userCommitData.emplace(L"testing", L"1 2 3");
  taxoWriter->setLiveCommitData(userCommitData.entrySet());
  delete taxoWriter;
  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  assertEquals(L"2 categories plus root should have been committed to the "
               L"underlying directory",
               3, r->numDocs());
  unordered_map<wstring, wstring> readUserCommitData =
      r->getIndexCommit()->getUserData();
  assertTrue(L"wrong value extracted from commit data",
             L"1 2 3" == readUserCommitData[L"testing"]);
  assertNotNull(DirectoryTaxonomyWriter::INDEX_EPOCH +
                    L" not found in commitData",
                readUserCommitData[DirectoryTaxonomyWriter::INDEX_EPOCH]);
  r->close();

  // open DirTaxoWriter again and commit, INDEX_EPOCH should still exist
  // in the commit data, otherwise DirTaxoReader.refresh() might not detect
  // that the taxonomy index has been recreated.
  taxoWriter = make_shared<DirectoryTaxonomyWriter>(
      dir, OpenMode::CREATE_OR_APPEND, NO_OP_CACHE);
  taxoWriter->addCategory(make_shared<FacetLabel>(
      L"c")); // add a category so that commit will happen
  taxoWriter->setLiveCommitData(
      (make_shared<HashMapAnonymousInnerClass>(shared_from_this()))
          ->entrySet());
  taxoWriter->commit();

  // verify taxoWriter.getCommitData()
  unordered_map<wstring, wstring> data = unordered_map<wstring, wstring>();
  deque<unordered_map::Entry<wstring, wstring>> iter =
      taxoWriter->getLiveCommitData();
  if (iter.size() > 0) {
    for (auto ent : iter) {
      data.emplace(ent.first, ent.second);
    }
  }

  assertNotNull(DirectoryTaxonomyWriter::INDEX_EPOCH +
                    L" not found in taoxWriter.commitData",
                data[DirectoryTaxonomyWriter::INDEX_EPOCH]);
  delete taxoWriter;

  r = DirectoryReader::open(dir);
  readUserCommitData = r->getIndexCommit()->getUserData();
  assertNotNull(DirectoryTaxonomyWriter::INDEX_EPOCH +
                    L" not found in commitData",
                readUserCommitData[DirectoryTaxonomyWriter::INDEX_EPOCH]);
  r->close();

  delete dir;
}

TestDirectoryTaxonomyWriter::HashMapAnonymousInnerClass::
    HashMapAnonymousInnerClass(
        shared_ptr<TestDirectoryTaxonomyWriter> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(L"just", L"data");
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRollback() throws Exception
void TestDirectoryTaxonomyWriter::testRollback() 
{
  // Verifies that if rollback is called, DTW is closed.
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<DirectoryTaxonomyWriter> dtw =
      make_shared<DirectoryTaxonomyWriter>(dir);
  dtw->addCategory(make_shared<FacetLabel>(L"a"));
  dtw->rollback();
  // should not have succeeded to add a category following rollback.
  expectThrows(AlreadyClosedException::typeid,
               [&]() { dtw->addCategory(make_shared<FacetLabel>(L"a")); });

  delete dir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRecreateRollback() throws Exception
void TestDirectoryTaxonomyWriter::testRecreateRollback() 
{
  // Tests rollback with OpenMode.CREATE
  shared_ptr<Directory> dir = newDirectory();
  delete (make_shared<DirectoryTaxonomyWriter>(dir));
  TestUtil::assertEquals(1, getEpoch(dir));
  (make_shared<DirectoryTaxonomyWriter>(dir, OpenMode::CREATE))->rollback();
  TestUtil::assertEquals(1, getEpoch(dir));

  delete dir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testEnsureOpen() throws Exception
void TestDirectoryTaxonomyWriter::testEnsureOpen() 
{
  // verifies that an exception is thrown if DTW was closed
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<DirectoryTaxonomyWriter> dtw =
      make_shared<DirectoryTaxonomyWriter>(dir);
  delete dtw;
  // should not succeed to add a category following close.
  expectThrows(AlreadyClosedException::typeid,
               [&]() { dtw->addCategory(make_shared<FacetLabel>(L"a")); });

  delete dir;
}

void TestDirectoryTaxonomyWriter::touchTaxo(
    shared_ptr<DirectoryTaxonomyWriter> taxoWriter,
    shared_ptr<FacetLabel> cp) 
{
  taxoWriter->addCategory(cp);
  taxoWriter->setLiveCommitData(
      (make_shared<HashMapAnonymousInnerClass2>(shared_from_this()))
          ->entrySet());
  taxoWriter->commit();
}

TestDirectoryTaxonomyWriter::HashMapAnonymousInnerClass2::
    HashMapAnonymousInnerClass2(
        shared_ptr<TestDirectoryTaxonomyWriter> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(L"just", L"data");
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRecreateAndRefresh() throws Exception
void TestDirectoryTaxonomyWriter::testRecreateAndRefresh() 
{
  // DirTaxoWriter lost the INDEX_EPOCH property if it was opened in
  // CREATE_OR_APPEND (or commit(userData) called twice), which could lead to
  // DirTaxoReader succeeding to refresh().
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory())
  {
    org::apache::lucene::store::Directory dir = newDirectory();

    shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
        make_shared<DirectoryTaxonomyWriter>(dir, OpenMode::CREATE_OR_APPEND,
                                             NO_OP_CACHE);
    touchTaxo(taxoWriter, make_shared<FacetLabel>(L"a"));

    shared_ptr<TaxonomyReader> taxoReader =
        make_shared<DirectoryTaxonomyReader>(dir);

    touchTaxo(taxoWriter, make_shared<FacetLabel>(L"b"));

    shared_ptr<TaxonomyReader> newtr =
        TaxonomyReader::openIfChanged(taxoReader);
    delete taxoReader;
    taxoReader = newtr;
    TestUtil::assertEquals(
        1,
        stoi(taxoReader
                 ->getCommitUserData()[DirectoryTaxonomyWriter::INDEX_EPOCH]));

    // now recreate the taxonomy, and check that the epoch is preserved after
    // opening DirTW again.
    delete taxoWriter;

    taxoWriter = make_shared<DirectoryTaxonomyWriter>(dir, OpenMode::CREATE,
                                                      NO_OP_CACHE);
    touchTaxo(taxoWriter, make_shared<FacetLabel>(L"c"));
    delete taxoWriter;

    taxoWriter = make_shared<DirectoryTaxonomyWriter>(
        dir, OpenMode::CREATE_OR_APPEND, NO_OP_CACHE);
    touchTaxo(taxoWriter, make_shared<FacetLabel>(L"d"));
    delete taxoWriter;

    newtr = TaxonomyReader::openIfChanged(taxoReader);
    delete taxoReader;
    taxoReader = newtr;
    TestUtil::assertEquals(
        2,
        stoi(taxoReader
                 ->getCommitUserData()[DirectoryTaxonomyWriter::INDEX_EPOCH]));
    delete taxoReader;
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testBackwardsCompatibility() throws
// Exception
void TestDirectoryTaxonomyWriter::testBackwardsCompatibility() throw(
    runtime_error)
{
  // tests that if the taxonomy index doesn't have the INDEX_EPOCH
  // property (supports pre-3.6 indexes), all still works.
  shared_ptr<Directory> dir = newDirectory();

  // create an empty index first, so that DirTaxoWriter initializes indexEpoch
  // to 1.
  delete (
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr)));

  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(dir, OpenMode::CREATE_OR_APPEND,
                                           NO_OP_CACHE);
  delete taxoWriter;

  shared_ptr<DirectoryTaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(dir);
  TestUtil::assertEquals(
      1, stoi(taxoReader
                  ->getCommitUserData()[DirectoryTaxonomyWriter::INDEX_EPOCH]));
  assertNull(TaxonomyReader::openIfChanged(taxoReader));
  delete taxoReader;

  delete dir;
}

void TestDirectoryTaxonomyWriter::testConcurrency() 
{
  constexpr int ncats = atLeast(100000); // add many categories
  constexpr int range = ncats * 3;       // affects the categories selection
  shared_ptr<AtomicInteger> *const numCats = make_shared<AtomicInteger>(ncats);
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<ConcurrentHashMap<wstring, wstring>> *const values =
      make_shared<ConcurrentHashMap<wstring, wstring>>();
  constexpr double d = random()->nextDouble();
  shared_ptr<TaxonomyWriterCache> *const cache;
  if (d < 0.7) {
    // this is the fastest, yet most memory consuming
    cache = make_shared<UTF8TaxonomyWriterCache>();
  } else if (TEST_NIGHTLY && d > 0.98) {
    // this is the slowest, but tests the writer concurrency when no caching is
    // done. only pick it during NIGHTLY tests, and even then, with very low
    // chances.
    cache = NO_OP_CACHE;
  } else {
    // this is slower than UTF8, but less memory consuming, and exercises
    // finding categories on disk too.
    cache = make_shared<LruTaxonomyWriterCache>(ncats / 10);
  }
  if (VERBOSE) {
    wcout << L"TEST: use cache=" << cache << endl;
  }
  shared_ptr<DirectoryTaxonomyWriter> *const tw =
      make_shared<DirectoryTaxonomyWriter>(dir, OpenMode::CREATE, cache);
  std::deque<std::shared_ptr<Thread>> addThreads(atLeast(4));
  for (int z = 0; z < addThreads.size(); z++) {
    addThreads[z] = make_shared<ThreadAnonymousInnerClass>(
        shared_from_this(), range, numCats, values, tw);
  }

  for (auto t : addThreads) {
    t->start();
  }
  for (auto t : addThreads) {
    t->join();
  }
  delete tw;

  shared_ptr<DirectoryTaxonomyReader> dtr =
      make_shared<DirectoryTaxonomyReader>(dir);
  // +1 for root category
  if (values->size() + 1 != dtr->getSize()) {
    for (auto value : values) {
      shared_ptr<FacetLabel> label =
          make_shared<FacetLabel>(FacetsConfig::stringToPath(value.first));
      if (dtr->getOrdinal(label) == -1) {
        wcout << L"FAIL: path=" << label << L" not recognized" << endl;
      }
    }
    fail(L"mismatch number of categories");
  }

  std::deque<int> parents = dtr->getParallelTaxonomyArrays()->parents();
  for (auto cat : values) {
    shared_ptr<FacetLabel> cp =
        make_shared<FacetLabel>(FacetsConfig::stringToPath(cat.first));
    assertTrue(L"category not found " + cp, dtr->getOrdinal(cp) > 0);
    int level = cp->length;
    int parentOrd = 0; // for root, parent is always virtual ROOT (ord=0)
    shared_ptr<FacetLabel> path = make_shared<FacetLabel>();
    for (int i = 0; i < level; i++) {
      path = cp->subpath(i + 1);
      int ord = dtr->getOrdinal(path);
      assertEquals(L"invalid parent for cp=" + path, parentOrd, parents[ord]);
      parentOrd = ord; // next level should have this parent
    }
  }

  IOUtils::close({dtr, dir});
}

TestDirectoryTaxonomyWriter::ThreadAnonymousInnerClass::
    ThreadAnonymousInnerClass(
        shared_ptr<TestDirectoryTaxonomyWriter> outerInstance, int range,
        shared_ptr<AtomicInteger> numCats,
        shared_ptr<ConcurrentHashMap<wstring, wstring>> values,
        shared_ptr<org::apache::lucene::facet::taxonomy::directory::
                       DirectoryTaxonomyWriter>
            tw)
{
  this->outerInstance = outerInstance;
  this->range = range;
  this->numCats = numCats;
  this->values = values;
  this->tw = tw;
}

void TestDirectoryTaxonomyWriter::ThreadAnonymousInnerClass::run()
{
  shared_ptr<Random> random = TestDirectoryTaxonomyWriter::random();
  while (numCats->decrementAndGet() > 0) {
    try {
      int value = random->nextInt(range);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      shared_ptr<FacetLabel> cp = make_shared<FacetLabel>(
          Integer::toString(value / 1000), Integer::toString(value / 10000),
          Integer::toString(value / 100000), Integer::toString(value));
      int ord = tw->addCategory(cp);
      assertTrue(L"invalid parent for ordinal " + to_wstring(ord) +
                     L", category " + cp,
                 tw->getParent(ord) != -1);
      wstring l1 = FacetsConfig::pathToString(cp->components, 1);
      wstring l2 = FacetsConfig::pathToString(cp->components, 2);
      wstring l3 = FacetsConfig::pathToString(cp->components, 3);
      wstring l4 = FacetsConfig::pathToString(cp->components, 4);
      values->put(l1, l1);
      values->put(l2, l2);
      values->put(l3, l3);
      values->put(l4, l4);
    } catch (const IOException &e) {
      throw runtime_error(e);
    }
  }
}

int64_t TestDirectoryTaxonomyWriter::getEpoch(
    shared_ptr<Directory> taxoDir) 
{
  shared_ptr<SegmentInfos> infos = SegmentInfos::readLatestCommit(taxoDir);
  return StringHelper::fromString<int64_t>(
      infos->getUserData()[DirectoryTaxonomyWriter::INDEX_EPOCH]);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testReplaceTaxonomy() throws Exception
void TestDirectoryTaxonomyWriter::testReplaceTaxonomy() 
{
  shared_ptr<Directory> input = newDirectory();
  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(input);
  int ordA = taxoWriter->addCategory(make_shared<FacetLabel>(L"a"));
  delete taxoWriter;

  shared_ptr<Directory> dir = newDirectory();
  taxoWriter = make_shared<DirectoryTaxonomyWriter>(dir);
  int ordB = taxoWriter->addCategory(make_shared<FacetLabel>(L"b"));
  taxoWriter->addCategory(make_shared<FacetLabel>(L"c"));
  taxoWriter->commit();

  int64_t origEpoch = getEpoch(dir);

  // replace the taxonomy with the input one
  taxoWriter->replaceTaxonomy(input);

  // LUCENE-4633: make sure that category "a" is not added again in any case
  taxoWriter->addTaxonomy(input, make_shared<MemoryOrdinalMap>());
  assertEquals(L"no categories should have been added", 2,
               taxoWriter->getSize()); // root + 'a'
  assertEquals(L"category 'a' received new ordinal?", ordA,
               taxoWriter->addCategory(make_shared<FacetLabel>(L"a")));

  // add the same category again -- it should not receive the same ordinal !
  int newOrdB = taxoWriter->addCategory(make_shared<FacetLabel>(L"b"));
  assertNotSame(L"new ordinal cannot be the original ordinal", ordB, newOrdB);
  assertEquals(L"ordinal should have been 2 since only one category was added "
               L"by replaceTaxonomy",
               2, newOrdB);

  delete taxoWriter;

  int64_t newEpoch = getEpoch(dir);
  assertTrue(L"index epoch should have been updated after replaceTaxonomy",
             origEpoch < newEpoch);

  delete dir;
  delete input;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testReaderFreshness() throws Exception
void TestDirectoryTaxonomyWriter::testReaderFreshness() 
{
  // ensures that the internal index reader is always kept fresh. Previously,
  // this simple scenario failed, if the cache just evicted the category that
  // is being added.
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(dir, OpenMode::CREATE, NO_OP_CACHE);
  int o1 = taxoWriter->addCategory(make_shared<FacetLabel>(L"a"));
  int o2 = taxoWriter->addCategory(make_shared<FacetLabel>(L"a"));
  assertTrue(
      L"ordinal for same category that is added twice should be the same !",
      o1 == o2);
  delete taxoWriter;
  delete dir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testCommitNoEmptyCommits() throws Exception
void TestDirectoryTaxonomyWriter::testCommitNoEmptyCommits() throw(
    runtime_error)
{
  // LUCENE-4972: DTW used to create empty commits even if no changes were made
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(dir);
  taxoWriter->addCategory(make_shared<FacetLabel>(L"a"));
  taxoWriter->commit();

  int64_t gen1 = SegmentInfos::getLastCommitGeneration(dir);
  taxoWriter->commit();
  int64_t gen2 = SegmentInfos::getLastCommitGeneration(dir);
  assertEquals(L"empty commit should not have changed the index", gen1, gen2);

  delete taxoWriter;
  delete dir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testCloseNoEmptyCommits() throws Exception
void TestDirectoryTaxonomyWriter::testCloseNoEmptyCommits() 
{
  // LUCENE-4972: DTW used to create empty commits even if no changes were made
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(dir);
  taxoWriter->addCategory(make_shared<FacetLabel>(L"a"));
  taxoWriter->commit();

  int64_t gen1 = SegmentInfos::getLastCommitGeneration(dir);
  delete taxoWriter;
  int64_t gen2 = SegmentInfos::getLastCommitGeneration(dir);
  assertEquals(L"empty commit should not have changed the index", gen1, gen2);

  delete taxoWriter;
  delete dir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testPrepareCommitNoEmptyCommits() throws
// Exception
void TestDirectoryTaxonomyWriter::testPrepareCommitNoEmptyCommits() throw(
    runtime_error)
{
  // LUCENE-4972: DTW used to create empty commits even if no changes were made
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(dir);
  taxoWriter->addCategory(make_shared<FacetLabel>(L"a"));
  taxoWriter->prepareCommit();
  taxoWriter->commit();

  int64_t gen1 = SegmentInfos::getLastCommitGeneration(dir);
  taxoWriter->prepareCommit();
  taxoWriter->commit();
  int64_t gen2 = SegmentInfos::getLastCommitGeneration(dir);
  assertEquals(L"empty commit should not have changed the index", gen1, gen2);

  delete taxoWriter;
  delete dir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testHugeLabel() throws Exception
void TestDirectoryTaxonomyWriter::testHugeLabel() 
{
  shared_ptr<Directory> indexDir = newDirectory(), taxoDir = newDirectory();
  shared_ptr<IndexWriter> indexWriter = make_shared<IndexWriter>(
      indexDir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(
          taxoDir, OpenMode::CREATE, make_shared<UTF8TaxonomyWriterCache>());
  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();

  // Add one huge label:
  wstring bigs = L"";
  int ordinal = -1;

  int len = FacetLabel::MAX_CATEGORY_PATH_LENGTH -
            4; // for the dimension and separator
  bigs = TestUtil::randomSimpleString(random(), len, len);
  shared_ptr<FacetField> ff = make_shared<FacetField>(L"dim", bigs);
  shared_ptr<FacetLabel> cp = make_shared<FacetLabel>(L"dim", bigs);
  ordinal = taxoWriter->addCategory(cp);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(ff);
  indexWriter->addDocument(config->build(taxoWriter, doc));

  // Add tiny ones to cause a re-hash
  for (int i = 0; i < 3; i++) {
    wstring s = TestUtil::randomSimpleString(random(), 1, 10);
    taxoWriter->addCategory(make_shared<FacetLabel>(L"dim", s));
    doc = make_shared<Document>();
    doc->push_back(make_shared<FacetField>(L"dim", s));
    indexWriter->addDocument(config->build(taxoWriter, doc));
  }

  // when too large components were allowed to be added, this resulted in a new
  // added category
  TestUtil::assertEquals(ordinal, taxoWriter->addCategory(cp));

  delete indexWriter;
  IOUtils::close({taxoWriter});

  shared_ptr<DirectoryReader> indexReader = DirectoryReader::open(indexDir);
  shared_ptr<TaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoDir);
  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(indexReader);
  shared_ptr<DrillDownQuery> ddq =
      make_shared<DrillDownQuery>(make_shared<FacetsConfig>());
  ddq->add(L"dim", {bigs});
  TestUtil::assertEquals(1, searcher->search(ddq, 10)->totalHits);

  IOUtils::close({indexReader, taxoReader, indexDir, taxoDir});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testReplaceTaxoWithLargeTaxonomy() throws
// Exception
void TestDirectoryTaxonomyWriter::testReplaceTaxoWithLargeTaxonomy() throw(
    runtime_error)
{
  shared_ptr<Directory> srcTaxoDir = newDirectory(),
                        targetTaxoDir = newDirectory();

  // build source, large, taxonomy
  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(srcTaxoDir);
  int ord = taxoWriter->addCategory(
      make_shared<FacetLabel>(L"A", L"1", L"1", L"1", L"1", L"1", L"1"));
  delete taxoWriter;

  taxoWriter = make_shared<DirectoryTaxonomyWriter>(targetTaxoDir);
  int ordinal = taxoWriter->addCategory(make_shared<FacetLabel>(L"B", L"1"));
  TestUtil::assertEquals(
      1, taxoWriter->getParent(
             ordinal)); // call getParent to initialize taxoArrays
  taxoWriter->commit();

  taxoWriter->replaceTaxonomy(srcTaxoDir);
  TestUtil::assertEquals(ord - 1, taxoWriter->getParent(ord));
  delete taxoWriter;

  delete srcTaxoDir;
  delete targetTaxoDir;
}
} // namespace org::apache::lucene::facet::taxonomy::directory