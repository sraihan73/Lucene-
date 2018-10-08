using namespace std;

#include "PersistenceTest.h"

namespace org::apache::lucene::search::suggest
{
using LookupResult = org::apache::lucene::search::suggest::Lookup::LookupResult;
using FSTCompletionLookup =
    org::apache::lucene::search::suggest::fst::FSTCompletionLookup;
using JaspellLookup =
    org::apache::lucene::search::suggest::jaspell::JaspellLookup;
using TSTLookup = org::apache::lucene::search::suggest::tst::TSTLookup;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void PersistenceTest::testTSTPersistence() 
{
  runTest(TSTLookup::typeid, true);
}

void PersistenceTest::testJaspellPersistence() 
{
  runTest(JaspellLookup::typeid, true);
}

void PersistenceTest::testFSTPersistence() 
{
  runTest(FSTCompletionLookup::typeid, false);
}

shared_ptr<Directory> PersistenceTest::getDirectory() { return newDirectory(); }

void PersistenceTest::runTest(type_info lookupClass,
                              bool supportsExactWeights) 
{

  // Add all input keys.
  shared_ptr<Lookup> lookup;
  shared_ptr<Directory> tempDir = getDirectory();
  if (lookupClass == TSTLookup::typeid) {
    lookup = make_shared<TSTLookup>(tempDir, L"suggest");
  } else if (lookupClass == FSTCompletionLookup::typeid) {
    lookup = make_shared<FSTCompletionLookup>(tempDir, L"suggest");
  } else {
    lookup = lookupClass.newInstance();
  }
  std::deque<std::shared_ptr<Input>> keys(this->keys.size());
  for (int i = 0; i < keys.size(); i++) {
    keys[i] = make_shared<Input>(this->keys[i], i);
  }
  lookup->build(make_shared<InputArrayIterator>(keys));

  // Store the suggester.
  shared_ptr<Path> storeDir =
      createTempDir(LuceneTestCase::getTestClass().name());
  lookup->store(Files::newOutputStream(storeDir->resolve(L"lookup.dat")));

  // Re-read it from disk.
  lookup = lookupClass.newInstance();
  lookup->load(Files::newInputStream(storeDir->resolve(L"lookup.dat")));

  // Assert validity.
  shared_ptr<Random> random = PersistenceTest::random();
  int64_t previous = numeric_limits<int64_t>::min();
  for (auto k : keys) {
    deque<std::shared_ptr<LookupResult>> deque = lookup->lookup(
        TestUtil::bytesToCharSequence(k->term, random), false, 1);
    TestUtil::assertEquals(1, deque.size());
    shared_ptr<LookupResult> lookupResult = deque[0];
    assertNotNull(k->term->utf8ToString(), lookupResult->key);

    if (supportsExactWeights) {
      assertEquals(k->term->utf8ToString(), k->v, lookupResult->value);
    } else {
      assertTrue(to_wstring(lookupResult->value) + L">=" + to_wstring(previous),
                 lookupResult->value >= previous);
      previous = lookupResult->value;
    }
  }
  delete tempDir;
}
} // namespace org::apache::lucene::search::suggest