using namespace std;

#include "TestDirectoryTaxonomyReader.h"

namespace org::apache::lucene::facet::taxonomy::directory
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using FacetTestCase = org::apache::lucene::facet::FacetTestCase;
using FacetLabel = org::apache::lucene::facet::taxonomy::FacetLabel;
using ChildrenIterator =
    org::apache::lucene::facet::taxonomy::TaxonomyReader::ChildrenIterator;
using TaxonomyReader = org::apache::lucene::facet::taxonomy::TaxonomyReader;
using TaxonomyWriter = org::apache::lucene::facet::taxonomy::TaxonomyWriter;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LogByteSizeMergePolicy =
    org::apache::lucene::index::LogByteSizeMergePolicy;
using LogMergePolicy = org::apache::lucene::index::LogMergePolicy;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using IOUtils = org::apache::lucene::util::IOUtils;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testCloseAfterIncRef() throws Exception
void TestDirectoryTaxonomyReader::testCloseAfterIncRef() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<DirectoryTaxonomyWriter> ltw =
      make_shared<DirectoryTaxonomyWriter>(dir);
  ltw->addCategory(make_shared<FacetLabel>(L"a"));
  delete ltw;

  shared_ptr<DirectoryTaxonomyReader> ltr =
      make_shared<DirectoryTaxonomyReader>(dir);
  ltr->incRef();
  delete ltr;

  // should not fail as we incRef() before close
  ltr->getSize();
  ltr->decRef();

  delete dir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testCloseTwice() throws Exception
void TestDirectoryTaxonomyReader::testCloseTwice() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<DirectoryTaxonomyWriter> ltw =
      make_shared<DirectoryTaxonomyWriter>(dir);
  ltw->addCategory(make_shared<FacetLabel>(L"a"));
  delete ltw;

  shared_ptr<DirectoryTaxonomyReader> ltr =
      make_shared<DirectoryTaxonomyReader>(dir);
  delete ltr;
  delete ltr; // no exception should be thrown

  delete dir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testOpenIfChangedResult() throws Exception
void TestDirectoryTaxonomyReader::testOpenIfChangedResult() 
{
  shared_ptr<Directory> dir = nullptr;
  shared_ptr<DirectoryTaxonomyWriter> ltw = nullptr;
  shared_ptr<DirectoryTaxonomyReader> ltr = nullptr;

  try {
    dir = newDirectory();
    ltw = make_shared<DirectoryTaxonomyWriter>(dir);

    ltw->addCategory(make_shared<FacetLabel>(L"a"));
    ltw->commit();

    ltr = make_shared<DirectoryTaxonomyReader>(dir);
    assertNull(L"Nothing has changed", TaxonomyReader::openIfChanged(ltr));

    ltw->addCategory(make_shared<FacetLabel>(L"b"));
    ltw->commit();

    shared_ptr<DirectoryTaxonomyReader> newtr =
        TaxonomyReader::openIfChanged(ltr);
    assertNotNull(L"changes were committed", newtr);
    assertNull(L"Nothing has changed", TaxonomyReader::openIfChanged(newtr));
    delete newtr;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::close({ltw, ltr, dir});
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testAlreadyClosed() throws Exception
void TestDirectoryTaxonomyReader::testAlreadyClosed() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<DirectoryTaxonomyWriter> ltw =
      make_shared<DirectoryTaxonomyWriter>(dir);
  ltw->addCategory(make_shared<FacetLabel>(L"a"));
  delete ltw;

  shared_ptr<DirectoryTaxonomyReader> ltr =
      make_shared<DirectoryTaxonomyReader>(dir);
  delete ltr;
  expectThrows(AlreadyClosedException::typeid, [&]() { ltr->getSize(); });

  delete dir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFreshReadRecreatedTaxonomy() throws
// Exception
void TestDirectoryTaxonomyReader::testFreshReadRecreatedTaxonomy() throw(
    runtime_error)
{
  doTestReadRecreatedTaxonomy(random(), true);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testOpenIfChangedReadRecreatedTaxonomy()
// throws Exception
void TestDirectoryTaxonomyReader::
    testOpenIfChangedReadRecreatedTaxonomy() 
{
  doTestReadRecreatedTaxonomy(random(), false);
}

void TestDirectoryTaxonomyReader::doTestReadRecreatedTaxonomy(
    shared_ptr<Random> random, bool closeReader) 
{
  shared_ptr<Directory> dir = nullptr;
  shared_ptr<TaxonomyWriter> tw = nullptr;
  shared_ptr<TaxonomyReader> tr = nullptr;

  // prepare a few categories
  int n = 10;
  std::deque<std::shared_ptr<FacetLabel>> cp(n);
  for (int i = 0; i < n; i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    cp[i] = make_shared<FacetLabel>(L"a", Integer::toString(i));
  }

  try {
    dir = newDirectory();

    tw = make_shared<DirectoryTaxonomyWriter>(dir);
    tw->addCategory(make_shared<FacetLabel>(L"a"));
    delete tw;

    tr = make_shared<DirectoryTaxonomyReader>(dir);
    int baseNumCategories = tr->getSize();

    for (int i = 0; i < n; i++) {
      int k = random->nextInt(n);
      tw = make_shared<DirectoryTaxonomyWriter>(dir, OpenMode::CREATE);
      for (int j = 0; j <= k; j++) {
        tw->addCategory(cp[j]);
      }
      delete tw;
      if (closeReader) {
        delete tr;
        tr = make_shared<DirectoryTaxonomyReader>(dir);
      } else {
        shared_ptr<TaxonomyReader> newtr = TaxonomyReader::openIfChanged(tr);
        assertNotNull(newtr);
        delete tr;
        tr = newtr;
      }
      assertEquals(L"Wrong #categories in taxonomy (i=" + to_wstring(i) +
                       L", k=" + to_wstring(k) + L")",
                   baseNumCategories + 1 + k, tr->getSize());
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::close({tr, tw, dir});
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testOpenIfChangedAndRefCount() throws
// Exception
void TestDirectoryTaxonomyReader::testOpenIfChangedAndRefCount() throw(
    runtime_error)
{
  shared_ptr<Directory> dir =
      make_shared<RAMDirectory>(); // no need for random directories here

  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(dir);
  taxoWriter->addCategory(make_shared<FacetLabel>(L"a"));
  taxoWriter->commit();

  shared_ptr<TaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(dir);
  assertEquals(L"wrong refCount", 1, taxoReader->getRefCount());

  taxoReader->incRef();
  assertEquals(L"wrong refCount", 2, taxoReader->getRefCount());

  taxoWriter->addCategory(make_shared<FacetLabel>(L"a", L"b"));
  taxoWriter->commit();
  shared_ptr<TaxonomyReader> newtr = TaxonomyReader::openIfChanged(taxoReader);
  assertNotNull(newtr);
  delete taxoReader;
  taxoReader = newtr;
  assertEquals(L"wrong refCount", 1, taxoReader->getRefCount());

  delete taxoWriter;
  delete taxoReader;
  delete dir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testOpenIfChangedManySegments() throws
// Exception
void TestDirectoryTaxonomyReader::testOpenIfChangedManySegments() throw(
    runtime_error)
{
  // test openIfChanged() when the taxonomy contains many segments
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<DirectoryTaxonomyWriter> writer =
      make_shared<DirectoryTaxonomyWriterAnonymousInnerClass>(
          shared_from_this(), dir);
  shared_ptr<TaxonomyReader> reader =
      make_shared<DirectoryTaxonomyReader>(writer);

  int numRounds = random()->nextInt(10) + 10;
  int numCategories = 1; // one for root
  for (int i = 0; i < numRounds; i++) {
    int numCats = random()->nextInt(4) + 1;
    for (int j = 0; j < numCats; j++) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      writer->addCategory(
          make_shared<FacetLabel>(Integer::toString(i), Integer::toString(j)));
    }
    numCategories += numCats + 1;
    shared_ptr<TaxonomyReader> newtr = TaxonomyReader::openIfChanged(reader);
    assertNotNull(newtr);
    delete reader;
    reader = newtr;

    // assert categories
    assertEquals(numCategories, reader->getSize());
    // C++ TODO: There is no native C++ equivalent to 'toString':
    int roundOrdinal =
        reader->getOrdinal(make_shared<FacetLabel>(Integer::toString(i)));
    std::deque<int> parents = reader->getParallelTaxonomyArrays()->parents();
    assertEquals(0, parents[roundOrdinal]); // round's parent is root
    for (int j = 0; j < numCats; j++) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      int ord = reader->getOrdinal(
          make_shared<FacetLabel>(Integer::toString(i), Integer::toString(j)));
      assertEquals(roundOrdinal, parents[ord]); // round's parent is root
    }
  }

  delete reader;
  delete writer;
  delete dir;
}

TestDirectoryTaxonomyReader::DirectoryTaxonomyWriterAnonymousInnerClass::
    DirectoryTaxonomyWriterAnonymousInnerClass(
        shared_ptr<TestDirectoryTaxonomyReader> outerInstance,
        shared_ptr<Directory> dir)
    : DirectoryTaxonomyWriter(dir)
{
  this->outerInstance = outerInstance;
}

shared_ptr<IndexWriterConfig>
TestDirectoryTaxonomyReader::DirectoryTaxonomyWriterAnonymousInnerClass::
    createIndexWriterConfig(OpenMode openMode)
{
  shared_ptr<IndexWriterConfig> conf =
      outerInstance->super->createIndexWriterConfig(openMode);
  shared_ptr<LogMergePolicy> lmp =
      std::static_pointer_cast<LogMergePolicy>(conf->getMergePolicy());
  lmp->setMergeFactor(2);
  return conf;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testOpenIfChangedMergedSegment() throws
// Exception
void TestDirectoryTaxonomyReader::testOpenIfChangedMergedSegment() throw(
    runtime_error)
{
  // test openIfChanged() when all index segments were merged - used to be
  // a bug in ParentArray, caught by testOpenIfChangedManySegments - only
  // this test is not random
  shared_ptr<Directory> dir = newDirectory();

  // hold onto IW to forceMerge
  // note how we don't close it, since DTW will close it.
  shared_ptr<IndexWriter> *const iw = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()))
               .setMergePolicy(make_shared<LogByteSizeMergePolicy>()));
  shared_ptr<DirectoryTaxonomyWriter> writer =
      make_shared<DirectoryTaxonomyWriterAnonymousInnerClass2>(
          shared_from_this(), dir, iw);

  shared_ptr<TaxonomyReader> reader =
      make_shared<DirectoryTaxonomyReader>(writer);
  assertEquals(1, reader->getSize());
  assertEquals(1, reader->getParallelTaxonomyArrays()->parents().size());

  // add category and call forceMerge -- this should flush IW and merge segments
  // down to 1 in ParentArray.initFromReader, this used to fail assuming there
  // are no parents.
  writer->addCategory(make_shared<FacetLabel>(L"1"));
  iw->forceMerge(1);

  // now calling openIfChanged should trip on the bug
  shared_ptr<TaxonomyReader> newtr = TaxonomyReader::openIfChanged(reader);
  assertNotNull(newtr);
  delete reader;
  reader = newtr;
  assertEquals(2, reader->getSize());
  assertEquals(2, reader->getParallelTaxonomyArrays()->parents().size());

  delete reader;
  delete writer;
  delete dir;
}

TestDirectoryTaxonomyReader::DirectoryTaxonomyWriterAnonymousInnerClass2::
    DirectoryTaxonomyWriterAnonymousInnerClass2(
        shared_ptr<TestDirectoryTaxonomyReader> outerInstance,
        shared_ptr<Directory> dir, shared_ptr<IndexWriter> iw)
    : DirectoryTaxonomyWriter(dir)
{
  this->outerInstance = outerInstance;
  this->iw = iw;
}

shared_ptr<IndexWriter>
TestDirectoryTaxonomyReader::DirectoryTaxonomyWriterAnonymousInnerClass2::
    openIndexWriter(shared_ptr<Directory> directory,
                    shared_ptr<IndexWriterConfig> config) 
{
  return iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testOpenIfChangedNoChangesButSegmentMerges()
// throws Exception
void TestDirectoryTaxonomyReader::
    testOpenIfChangedNoChangesButSegmentMerges() 
{
  // test openIfChanged() when the taxonomy hasn't really changed, but segments
  // were merged. The NRT reader will be reopened, and ParentArray used to
  // assert that the new reader contains more ordinals than were given from the
  // old TaxReader version
  shared_ptr<Directory> dir = newDirectory();

  // hold onto IW to forceMerge
  // note how we don't close it, since DTW will close it.
  shared_ptr<IndexWriter> *const iw = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()))
               .setMergePolicy(make_shared<LogByteSizeMergePolicy>()));
  shared_ptr<DirectoryTaxonomyWriter> writer =
      make_shared<DirectoryTaxonomyWriterAnonymousInnerClass3>(
          shared_from_this(), dir, iw);

  // add a category so that the following DTR open will cause a flush and
  // a new segment will be created
  writer->addCategory(make_shared<FacetLabel>(L"a"));

  shared_ptr<TaxonomyReader> reader =
      make_shared<DirectoryTaxonomyReader>(writer);
  assertEquals(2, reader->getSize());
  assertEquals(2, reader->getParallelTaxonomyArrays()->parents().size());

  // merge all the segments so that NRT reader thinks there's a change
  iw->forceMerge(1);

  // now calling openIfChanged should trip on the wrong assert in ParetArray's
  // ctor
  shared_ptr<TaxonomyReader> newtr = TaxonomyReader::openIfChanged(reader);
  assertNotNull(newtr);
  delete reader;
  reader = newtr;
  assertEquals(2, reader->getSize());
  assertEquals(2, reader->getParallelTaxonomyArrays()->parents().size());

  delete reader;
  delete writer;
  delete dir;
}

TestDirectoryTaxonomyReader::DirectoryTaxonomyWriterAnonymousInnerClass3::
    DirectoryTaxonomyWriterAnonymousInnerClass3(
        shared_ptr<TestDirectoryTaxonomyReader> outerInstance,
        shared_ptr<Directory> dir, shared_ptr<IndexWriter> iw)
    : DirectoryTaxonomyWriter(dir)
{
  this->outerInstance = outerInstance;
  this->iw = iw;
}

shared_ptr<IndexWriter>
TestDirectoryTaxonomyReader::DirectoryTaxonomyWriterAnonymousInnerClass3::
    openIndexWriter(shared_ptr<Directory> directory,
                    shared_ptr<IndexWriterConfig> config) 
{
  return iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testOpenIfChangedReuseAfterRecreate() throws
// Exception
void TestDirectoryTaxonomyReader::testOpenIfChangedReuseAfterRecreate() throw(
    runtime_error)
{
  // tests that if the taxonomy is recreated, no data is reused from the
  // previous taxonomy
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<DirectoryTaxonomyWriter> writer =
      make_shared<DirectoryTaxonomyWriter>(dir);
  shared_ptr<FacetLabel> cp_a = make_shared<FacetLabel>(L"a");
  writer->addCategory(cp_a);
  delete writer;

  shared_ptr<DirectoryTaxonomyReader> r1 =
      make_shared<DirectoryTaxonomyReader>(dir);
  // fill r1's caches
  assertEquals(1, r1->getOrdinal(cp_a));
  assertEquals(cp_a, r1->getPath(1));

  // now recreate, add a different category
  writer = make_shared<DirectoryTaxonomyWriter>(dir, OpenMode::CREATE);
  shared_ptr<FacetLabel> cp_b = make_shared<FacetLabel>(L"b");
  writer->addCategory(cp_b);
  delete writer;

  shared_ptr<DirectoryTaxonomyReader> r2 = TaxonomyReader::openIfChanged(r1);
  assertNotNull(r2);

  // fill r2's caches
  assertEquals(1, r2->getOrdinal(cp_b));
  assertEquals(cp_b, r2->getPath(1));

  // check that r1 doesn't see cp_b
  assertEquals(TaxonomyReader::INVALID_ORDINAL, r1->getOrdinal(cp_b));
  assertEquals(cp_a, r1->getPath(1));

  // check that r2 doesn't see cp_a
  assertEquals(TaxonomyReader::INVALID_ORDINAL, r2->getOrdinal(cp_a));
  assertEquals(cp_b, r2->getPath(1));

  delete r2;
  delete r1;
  delete dir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testOpenIfChangedReuse() throws Exception
void TestDirectoryTaxonomyReader::testOpenIfChangedReuse() 
{
  // test the reuse of data from the old DTR instance
  for (auto nrt : std::deque<bool>{false, true}) {
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<DirectoryTaxonomyWriter> writer =
        make_shared<DirectoryTaxonomyWriter>(dir);

    shared_ptr<FacetLabel> cp_a = make_shared<FacetLabel>(L"a");
    writer->addCategory(cp_a);
    if (!nrt) {
      writer->commit();
    }

    shared_ptr<DirectoryTaxonomyReader> r1 =
        nrt ? make_shared<DirectoryTaxonomyReader>(writer)
            : make_shared<DirectoryTaxonomyReader>(dir);
    // fill r1's caches
    assertEquals(1, r1->getOrdinal(cp_a));
    assertEquals(cp_a, r1->getPath(1));

    shared_ptr<FacetLabel> cp_b = make_shared<FacetLabel>(L"b");
    writer->addCategory(cp_b);
    if (!nrt) {
      writer->commit();
    }

    shared_ptr<DirectoryTaxonomyReader> r2 = TaxonomyReader::openIfChanged(r1);
    assertNotNull(r2);

    // add r2's categories to the caches
    assertEquals(2, r2->getOrdinal(cp_b));
    assertEquals(cp_b, r2->getPath(2));

    // check that r1 doesn't see cp_b
    assertEquals(TaxonomyReader::INVALID_ORDINAL, r1->getOrdinal(cp_b));
    assertNull(r1->getPath(2));

    delete r1;
    delete r2;
    delete writer;
    delete dir;
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testOpenIfChangedReplaceTaxonomy() throws
// Exception
void TestDirectoryTaxonomyReader::testOpenIfChangedReplaceTaxonomy() throw(
    runtime_error)
{
  // test openIfChanged when replaceTaxonomy is called, which is equivalent to
  // recreate only can work with NRT as well
  shared_ptr<Directory> src = newDirectory();
  shared_ptr<DirectoryTaxonomyWriter> w =
      make_shared<DirectoryTaxonomyWriter>(src);
  shared_ptr<FacetLabel> cp_b = make_shared<FacetLabel>(L"b");
  w->addCategory(cp_b);
  delete w;

  for (auto nrt : std::deque<bool>{false, true}) {
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<DirectoryTaxonomyWriter> writer =
        make_shared<DirectoryTaxonomyWriter>(dir);

    shared_ptr<FacetLabel> cp_a = make_shared<FacetLabel>(L"a");
    writer->addCategory(cp_a);
    if (!nrt) {
      writer->commit();
    }

    shared_ptr<DirectoryTaxonomyReader> r1 =
        nrt ? make_shared<DirectoryTaxonomyReader>(writer)
            : make_shared<DirectoryTaxonomyReader>(dir);
    // fill r1's caches
    assertEquals(1, r1->getOrdinal(cp_a));
    assertEquals(cp_a, r1->getPath(1));

    // now replace taxonomy
    writer->replaceTaxonomy(src);
    if (!nrt) {
      writer->commit();
    }

    shared_ptr<DirectoryTaxonomyReader> r2 = TaxonomyReader::openIfChanged(r1);
    assertNotNull(r2);

    // fill r2's caches
    assertEquals(1, r2->getOrdinal(cp_b));
    assertEquals(cp_b, r2->getPath(1));

    // check that r1 doesn't see cp_b
    assertEquals(TaxonomyReader::INVALID_ORDINAL, r1->getOrdinal(cp_b));
    assertEquals(cp_a, r1->getPath(1));

    // check that r2 doesn't see cp_a
    assertEquals(TaxonomyReader::INVALID_ORDINAL, r2->getOrdinal(cp_a));
    assertEquals(cp_b, r2->getPath(1));

    delete r2;
    delete r1;
    delete writer;
    delete dir;
  }

  delete src;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGetChildren() throws Exception
void TestDirectoryTaxonomyReader::testGetChildren() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(dir);
  int numCategories = atLeast(10);
  int numA = 0, numB = 0;
  shared_ptr<Random> random = TestDirectoryTaxonomyReader::random();
  // add the two categories for which we'll also add children (so asserts are
  // simpler)
  taxoWriter->addCategory(make_shared<FacetLabel>(L"a"));
  taxoWriter->addCategory(make_shared<FacetLabel>(L"b"));
  for (int i = 0; i < numCategories; i++) {
    if (random->nextBoolean()) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      taxoWriter->addCategory(
          make_shared<FacetLabel>(L"a", Integer::toString(i)));
      ++numA;
    } else {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      taxoWriter->addCategory(
          make_shared<FacetLabel>(L"b", Integer::toString(i)));
      ++numB;
    }
  }
  // add category with no children
  taxoWriter->addCategory(make_shared<FacetLabel>(L"c"));
  delete taxoWriter;

  shared_ptr<DirectoryTaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(dir);

  // non existing category
  shared_ptr<ChildrenIterator> it = taxoReader->getChildren(
      taxoReader->getOrdinal(make_shared<FacetLabel>(L"invalid")));
  assertEquals(TaxonomyReader::INVALID_ORDINAL, it->next());

  // a category with no children
  it = taxoReader->getChildren(
      taxoReader->getOrdinal(make_shared<FacetLabel>(L"c")));
  assertEquals(TaxonomyReader::INVALID_ORDINAL, it->next());

  // arbitrary negative ordinal
  it = taxoReader->getChildren(-2);
  assertEquals(TaxonomyReader::INVALID_ORDINAL, it->next());

  // root's children
  shared_ptr<Set<wstring>> roots =
      unordered_set<wstring>(Arrays::asList(L"a", L"b", L"c"));
  it = taxoReader->getChildren(TaxonomyReader::ROOT_ORDINAL);
  while (!roots->isEmpty()) {
    shared_ptr<FacetLabel> root = taxoReader->getPath(it->next());
    assertEquals(1, root->length);
    assertTrue(roots->remove(root->components[0]));
  }
  assertEquals(TaxonomyReader::INVALID_ORDINAL, it->next());

  for (int i = 0; i < 2; i++) {
    shared_ptr<FacetLabel> cp =
        i == 0 ? make_shared<FacetLabel>(L"a") : make_shared<FacetLabel>(L"b");
    int ordinal = taxoReader->getOrdinal(cp);
    it = taxoReader->getChildren(ordinal);
    int numChildren = 0;
    int child;
    while ((child = it->next()) != TaxonomyReader::INVALID_ORDINAL) {
      shared_ptr<FacetLabel> path = taxoReader->getPath(child);
      assertEquals(2, path->length);
      assertEquals(path->components[0], i == 0 ? L"a" : L"b");
      ++numChildren;
    }
    int expected = i == 0 ? numA : numB;
    assertEquals(L"invalid num children", expected, numChildren);
  }
  delete taxoReader;

  delete dir;
}

void TestDirectoryTaxonomyReader::testAccountable() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(dir);
  int numCategories = atLeast(10);
  int numA = 0, numB = 0;
  shared_ptr<Random> random = TestDirectoryTaxonomyReader::random();
  // add the two categories for which we'll also add children (so asserts are
  // simpler)
  taxoWriter->addCategory(make_shared<FacetLabel>(L"a"));
  taxoWriter->addCategory(make_shared<FacetLabel>(L"b"));
  for (int i = 0; i < numCategories; i++) {
    if (random->nextBoolean()) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      taxoWriter->addCategory(
          make_shared<FacetLabel>(L"a", Integer::toString(i)));
      ++numA;
    } else {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      taxoWriter->addCategory(
          make_shared<FacetLabel>(L"b", Integer::toString(i)));
      ++numB;
    }
  }
  // add category with no children
  taxoWriter->addCategory(make_shared<FacetLabel>(L"c"));
  delete taxoWriter;

  shared_ptr<DirectoryTaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(dir);
  assertTrue(taxoReader->ramBytesUsed() > 0);
  assertTrue(taxoReader->getChildResources()->size() > 0);
  delete taxoReader;
  delete dir;
}
} // namespace org::apache::lucene::facet::taxonomy::directory