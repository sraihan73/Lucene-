using namespace std;

#include "TestAddTaxonomy.h"

namespace org::apache::lucene::facet::taxonomy::directory
{
using FacetTestCase = org::apache::lucene::facet::FacetTestCase;
using FacetLabel = org::apache::lucene::facet::taxonomy::FacetLabel;
using DiskOrdinalMap = org::apache::lucene::facet::taxonomy::directory::
    DirectoryTaxonomyWriter::DiskOrdinalMap;
using MemoryOrdinalMap = org::apache::lucene::facet::taxonomy::directory::
    DirectoryTaxonomyWriter::MemoryOrdinalMap;
using OrdinalMap = org::apache::lucene::facet::taxonomy::directory::
    DirectoryTaxonomyWriter::OrdinalMap;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestAddTaxonomy::dotest(int ncats, int const range) 
{
  shared_ptr<AtomicInteger> *const numCats = make_shared<AtomicInteger>(ncats);
  std::deque<std::shared_ptr<Directory>> dirs(2);
  for (int i = 0; i < dirs.size(); i++) {
    dirs[i] = newDirectory();
    shared_ptr<DirectoryTaxonomyWriter> *const tw =
        make_shared<DirectoryTaxonomyWriter>(dirs[i]);
    std::deque<std::shared_ptr<Thread>> addThreads(4);
    for (int j = 0; j < addThreads.size(); j++) {
      addThreads[j] = make_shared<ThreadAnonymousInnerClass>(
          shared_from_this(), range, numCats, tw);
    }

    for (auto t : addThreads) {
      t->start();
    }
    for (auto t : addThreads) {
      t->join();
    }
    delete tw;
  }

  shared_ptr<DirectoryTaxonomyWriter> tw =
      make_shared<DirectoryTaxonomyWriter>(dirs[0]);
  shared_ptr<OrdinalMap> map_obj = randomOrdinalMap();
  tw->addTaxonomy(dirs[1], map_obj);
  delete tw;

  validate(dirs[0], dirs[1], map_obj);

  IOUtils::close(dirs);
}

TestAddTaxonomy::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestAddTaxonomy> outerInstance, int range,
    shared_ptr<AtomicInteger> numCats,
    shared_ptr<org::apache::lucene::facet::taxonomy::directory::
                   DirectoryTaxonomyWriter>
        tw)
{
  this->outerInstance = outerInstance;
  this->range = range;
  this->numCats = numCats;
  this->tw = tw;
}

void TestAddTaxonomy::ThreadAnonymousInnerClass::run()
{
  shared_ptr<Random> random = TestAddTaxonomy::random();
  while (numCats->decrementAndGet() > 0) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring cat = Integer::toString(random->nextInt(range));
    try {
      tw->addCategory(make_shared<FacetLabel>(L"a", cat));
    } catch (const IOException &e) {
      throw runtime_error(e);
    }
  }
}

shared_ptr<OrdinalMap> TestAddTaxonomy::randomOrdinalMap() 
{
  if (random()->nextBoolean()) {
    return make_shared<DiskOrdinalMap>(createTempFile(L"taxoMap", L""));
  } else {
    return make_shared<MemoryOrdinalMap>();
  }
}

void TestAddTaxonomy::validate(
    shared_ptr<Directory> dest, shared_ptr<Directory> src,
    shared_ptr<OrdinalMap> ordMap) 
{
  shared_ptr<DirectoryTaxonomyReader> destTR =
      make_shared<DirectoryTaxonomyReader>(dest);
  try {
    constexpr int destSize = destTR->getSize();
    shared_ptr<DirectoryTaxonomyReader> srcTR =
        make_shared<DirectoryTaxonomyReader>(src);
    try {
      std::deque<int> map_obj = ordMap->getMap();

      // validate taxo sizes
      int srcSize = srcTR->getSize();
      assertTrue(
          L"destination taxonomy expected to be larger than source; dest=" +
              to_wstring(destSize) + L" src=" + to_wstring(srcSize),
          destSize >= srcSize);

      // validate that all source categories exist in destination, and their
      // ordinals are as expected.
      for (int j = 1; j < srcSize; j++) {
        shared_ptr<FacetLabel> cp = srcTR->getPath(j);
        int destOrdinal = destTR->getOrdinal(cp);
        assertTrue(cp + L" not found in destination", destOrdinal > 0);
        TestUtil::assertEquals(destOrdinal, map_obj[j]);
      }
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      delete srcTR;
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete destTR;
  }
}

void TestAddTaxonomy::testAddEmpty() 
{
  shared_ptr<Directory> dest = newDirectory();
  shared_ptr<DirectoryTaxonomyWriter> destTW =
      make_shared<DirectoryTaxonomyWriter>(dest);
  destTW->addCategory(make_shared<FacetLabel>(L"Author", L"Rob Pike"));
  destTW->addCategory(make_shared<FacetLabel>(L"Aardvarks", L"Bob"));
  destTW->commit();

  shared_ptr<Directory> src = newDirectory();
  delete (
      make_shared<DirectoryTaxonomyWriter>(src)); // create an empty taxonomy

  shared_ptr<OrdinalMap> map_obj = randomOrdinalMap();
  destTW->addTaxonomy(src, map_obj);
  delete destTW;

  validate(dest, src, map_obj);

  IOUtils::close({dest, src});
}

void TestAddTaxonomy::testAddToEmpty() 
{
  shared_ptr<Directory> dest = newDirectory();

  shared_ptr<Directory> src = newDirectory();
  shared_ptr<DirectoryTaxonomyWriter> srcTW =
      make_shared<DirectoryTaxonomyWriter>(src);
  srcTW->addCategory(make_shared<FacetLabel>(L"Author", L"Rob Pike"));
  srcTW->addCategory(make_shared<FacetLabel>(L"Aardvarks", L"Bob"));
  delete srcTW;

  shared_ptr<DirectoryTaxonomyWriter> destTW =
      make_shared<DirectoryTaxonomyWriter>(dest);
  shared_ptr<OrdinalMap> map_obj = randomOrdinalMap();
  destTW->addTaxonomy(src, map_obj);
  delete destTW;

  validate(dest, src, map_obj);

  IOUtils::close({dest, src});
}

void TestAddTaxonomy::testBig() 
{
  dotest(200, 10000);
  dotest(1000, 20000);
  dotest(400000, 1000000);
}

void TestAddTaxonomy::testMedium() 
{
  shared_ptr<Random> random = TestAddTaxonomy::random();
  int numTests = atLeast(3);
  for (int i = 0; i < numTests; i++) {
    dotest(TestUtil::nextInt(random, 2, 100),
           TestUtil::nextInt(random, 100, 1000));
  }
}

void TestAddTaxonomy::testSimple() 
{
  shared_ptr<Directory> dest = newDirectory();
  shared_ptr<DirectoryTaxonomyWriter> tw1 =
      make_shared<DirectoryTaxonomyWriter>(dest);
  tw1->addCategory(make_shared<FacetLabel>(L"Author", L"Mark Twain"));
  tw1->addCategory(make_shared<FacetLabel>(L"Animals", L"Dog"));
  tw1->addCategory(make_shared<FacetLabel>(L"Author", L"Rob Pike"));

  shared_ptr<Directory> src = newDirectory();
  shared_ptr<DirectoryTaxonomyWriter> tw2 =
      make_shared<DirectoryTaxonomyWriter>(src);
  tw2->addCategory(make_shared<FacetLabel>(L"Author", L"Rob Pike"));
  tw2->addCategory(make_shared<FacetLabel>(L"Aardvarks", L"Bob"));
  delete tw2;

  shared_ptr<OrdinalMap> map_obj = randomOrdinalMap();

  tw1->addTaxonomy(src, map_obj);
  delete tw1;

  validate(dest, src, map_obj);

  IOUtils::close({dest, src});
}

void TestAddTaxonomy::testConcurrency() 
{
  // tests that addTaxonomy and addCategory work in parallel
  constexpr int numCategories = atLeast(10000);

  // build an input taxonomy index
  shared_ptr<Directory> src = newDirectory();
  shared_ptr<DirectoryTaxonomyWriter> tw =
      make_shared<DirectoryTaxonomyWriter>(src);
  for (int i = 0; i < numCategories; i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    tw->addCategory(make_shared<FacetLabel>(L"a", Integer::toString(i)));
  }
  delete tw;

  // now add the taxonomy to an empty taxonomy, while adding the categories
  // again, in parallel -- in the end, no duplicate categories should exist.
  shared_ptr<Directory> dest = newDirectory();
  shared_ptr<DirectoryTaxonomyWriter> *const destTW =
      make_shared<DirectoryTaxonomyWriter>(dest);
  shared_ptr<Thread> t = make_shared<ThreadAnonymousInnerClass2>(
      shared_from_this(), numCategories, destTW);
  t->start();

  shared_ptr<OrdinalMap> map_obj = make_shared<MemoryOrdinalMap>();
  destTW->addTaxonomy(src, map_obj);
  t->join();
  delete destTW;

  // now validate

  shared_ptr<DirectoryTaxonomyReader> dtr =
      make_shared<DirectoryTaxonomyReader>(dest);
  // +2 to account for the root category + "a"
  TestUtil::assertEquals(numCategories + 2, dtr->getSize());
  unordered_set<std::shared_ptr<FacetLabel>> categories =
      unordered_set<std::shared_ptr<FacetLabel>>();
  for (int i = 1; i < dtr->getSize(); i++) {
    shared_ptr<FacetLabel> cat = dtr->getPath(i);
    assertTrue(L"category " + cat + L" already existed",
               categories.insert(cat));
  }
  delete dtr;

  IOUtils::close({src, dest});
}

TestAddTaxonomy::ThreadAnonymousInnerClass2::ThreadAnonymousInnerClass2(
    shared_ptr<TestAddTaxonomy> outerInstance, int numCategories,
    shared_ptr<org::apache::lucene::facet::taxonomy::directory::
                   DirectoryTaxonomyWriter>
        destTW)
{
  this->outerInstance = outerInstance;
  this->numCategories = numCategories;
  this->destTW = destTW;
}

void TestAddTaxonomy::ThreadAnonymousInnerClass2::run()
{
  for (int i = 0; i < numCategories; i++) {
    try {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      destTW->addCategory(make_shared<FacetLabel>(L"a", Integer::toString(i)));
    } catch (const IOException &e) {
      // shouldn't happen - if it does, let the test fail on uncaught exception.
      throw runtime_error(e);
    }
  }
}
} // namespace org::apache::lucene::facet::taxonomy::directory