using namespace std;

#include "TestSpellChecker.h"

namespace org::apache::lucene::search::spell
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using English = org::apache::lucene::util::English;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using NamedThreadFactory = org::apache::lucene::util::NamedThreadFactory;

void TestSpellChecker::setUp() 
{
  LuceneTestCase::setUp();

  // create a user index
  userindex = newDirectory();
  analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      userindex, make_shared<IndexWriterConfig>(analyzer));

  for (int i = 0; i < 1000; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newTextField(L"field1", English::intToEnglish(i), Field::Store::YES));
    doc->push_back(newTextField(L"field2", English::intToEnglish(i + 1),
                                Field::Store::YES)); // + word thousand
    doc->push_back(newTextField(L"field3",
                                L"fvei" + (i % 2 == 0 ? L" five" : L""),
                                Field::Store::YES)); // + word thousand
    writer->addDocument(doc);
  }
  {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newTextField(L"field1", L"eight", Field::Store::YES)); // "eight" in
                                                               // the index
                                                               // twice
    writer->addDocument(doc);
  }
  {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newTextField(L"field1", L"twenty-one twenty-one",
                     Field::Store::YES)); // "twenty-one" in the index thrice
    writer->addDocument(doc);
  }
  {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newTextField(L"field1", L"twenty", Field::Store::YES)); // "twenty"
                                                                // in the
                                                                // index
                                                                // twice
    writer->addDocument(doc);
  }

  delete writer;
  searchers =
      Collections::synchronizedList(deque<std::shared_ptr<IndexSearcher>>());
  // create the spellChecker
  spellindex = newDirectory();
  spellChecker = make_shared<SpellCheckerMock>(shared_from_this(), spellindex);
}

void TestSpellChecker::tearDown() 
{
  delete userindex;
  if (!spellChecker->isClosed()) {
    delete spellChecker;
  }
  delete spellindex;
  delete analyzer;
  LuceneTestCase::tearDown();
}

void TestSpellChecker::testBuild() 
{
  shared_ptr<IndexReader> r = DirectoryReader::open(userindex);

  spellChecker->clearIndex();

  addwords(r, spellChecker, L"field1");
  int num_field1 = this->numdoc();

  addwords(r, spellChecker, L"field2");
  int num_field2 = this->numdoc();

  assertEquals(num_field2, num_field1 + 1);

  assertLastSearcherOpen(4);

  checkCommonSuggestions(r);
  checkLevenshteinSuggestions(r);

  spellChecker->setStringDistance(make_shared<JaroWinklerDistance>());
  spellChecker->setAccuracy(0.8f);
  checkCommonSuggestions(r);
  checkJaroWinklerSuggestions();
  // the accuracy is set to 0.8 by default, but the best result has a score of
  // 0.925
  std::deque<wstring> similar =
      spellChecker->suggestSimilar(L"fvie", 2, 0.93f);
  assertTrue(similar.empty());
  similar = spellChecker->suggestSimilar(L"fvie", 2, 0.92f);
  assertTrue(similar.size() == 1);

  similar = spellChecker->suggestSimilar(L"fiv", 2);
  assertTrue(similar.size() > 0);
  assertEquals(similar[0], L"five");

  spellChecker->setStringDistance(make_shared<NGramDistance>(2));
  spellChecker->setAccuracy(0.5f);
  checkCommonSuggestions(r);
  checkNGramSuggestions();

  delete r;
}

void TestSpellChecker::testComparator() 
{
  shared_ptr<IndexReader> r = DirectoryReader::open(userindex);
  shared_ptr<Directory> compIdx = newDirectory();
  shared_ptr<SpellChecker> compareSP = make_shared<SpellCheckerMock>(
      shared_from_this(), compIdx, make_shared<LevenshteinDistance>(),
      make_shared<SuggestWordFrequencyComparator>());
  addwords(r, compareSP, L"field3");

  std::deque<wstring> similar = compareSP->suggestSimilar(
      L"fvie", 2, r, L"field3", SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX);
  assertTrue(similar.size() == 2);
  // five and fvei have the same score, but different frequencies.
  assertEquals(L"fvei", similar[0]);
  assertEquals(L"five", similar[1]);
  delete r;
  if (!compareSP->isClosed()) {
    delete compareSP;
  }
  delete compIdx;
}

void TestSpellChecker::testBogusField() 
{
  shared_ptr<IndexReader> r = DirectoryReader::open(userindex);
  shared_ptr<Directory> compIdx = newDirectory();
  shared_ptr<SpellChecker> compareSP = make_shared<SpellCheckerMock>(
      shared_from_this(), compIdx, make_shared<LevenshteinDistance>(),
      make_shared<SuggestWordFrequencyComparator>());
  addwords(r, compareSP, L"field3");

  std::deque<wstring> similar =
      compareSP->suggestSimilar(L"fvie", 2, r, L"bogusFieldBogusField",
                                SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX);
  assertEquals(0, similar.size());
  delete r;
  if (!compareSP->isClosed()) {
    delete compareSP;
  }
  delete compIdx;
}

void TestSpellChecker::testSuggestModes() 
{
  shared_ptr<IndexReader> r = DirectoryReader::open(userindex);
  spellChecker->clearIndex();
  addwords(r, spellChecker, L"field1");

  {
    std::deque<wstring> similar = spellChecker->suggestSimilar(
        L"eighty", 2, r, L"field1", SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX);
    assertEquals(1, similar.size());
    assertEquals(L"eighty", similar[0]);
  }

  {
    std::deque<wstring> similar = spellChecker->suggestSimilar(
        L"eight", 2, r, L"field1", SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX);
    assertEquals(1, similar.size());
    assertEquals(L"eight", similar[0]);
  }

  {
    std::deque<wstring> similar = spellChecker->suggestSimilar(
        L"eighty", 5, r, L"field1", SuggestMode::SUGGEST_MORE_POPULAR);
    assertEquals(5, similar.size());
    assertEquals(L"eight", similar[0]);
  }

  {
    std::deque<wstring> similar = spellChecker->suggestSimilar(
        L"twenty", 5, r, L"field1", SuggestMode::SUGGEST_MORE_POPULAR);
    assertEquals(1, similar.size());
    assertEquals(L"twenty-one", similar[0]);
  }

  {
    std::deque<wstring> similar = spellChecker->suggestSimilar(
        L"eight", 5, r, L"field1", SuggestMode::SUGGEST_MORE_POPULAR);
    assertEquals(0, similar.size());
  }

  {
    std::deque<wstring> similar = spellChecker->suggestSimilar(
        L"eighty", 5, r, L"field1", SuggestMode::SUGGEST_ALWAYS);
    assertEquals(5, similar.size());
    assertEquals(L"eight", similar[0]);
  }

  {
    std::deque<wstring> similar = spellChecker->suggestSimilar(
        L"eight", 5, r, L"field1", SuggestMode::SUGGEST_ALWAYS);
    assertEquals(5, similar.size());
    assertEquals(L"eighty", similar[0]);
  }
  delete r;
}

void TestSpellChecker::checkCommonSuggestions(shared_ptr<IndexReader> r) throw(
    IOException)
{
  std::deque<wstring> similar = spellChecker->suggestSimilar(L"fvie", 2);
  assertTrue(similar.size() > 0);
  assertEquals(similar[0], L"five");

  similar = spellChecker->suggestSimilar(L"five", 2);
  if (similar.size() > 0) {
    assertFalse(similar[0] == L"five"); // don't suggest a word for itself
  }

  similar = spellChecker->suggestSimilar(L"fiv", 2);
  assertTrue(similar.size() > 0);
  assertEquals(similar[0], L"five");

  similar = spellChecker->suggestSimilar(L"fives", 2);
  assertTrue(similar.size() > 0);
  assertEquals(similar[0], L"five");

  assertTrue(similar.size() > 0);
  similar = spellChecker->suggestSimilar(L"fie", 2);
  assertEquals(similar[0], L"five");

  //  test restraint to a field
  similar = spellChecker->suggestSimilar(
      L"tousand", 10, r, L"field1", SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX);
  assertEquals(
      0, similar.size()); // there isn't the term thousand in the field field1

  similar = spellChecker->suggestSimilar(
      L"tousand", 10, r, L"field2", SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX);
  assertEquals(
      1, similar.size()); // there is the term thousand in the field field2
}

void TestSpellChecker::checkLevenshteinSuggestions(
    shared_ptr<IndexReader> r) 
{
  // test small word
  std::deque<wstring> similar = spellChecker->suggestSimilar(L"fvie", 2);
  assertEquals(1, similar.size());
  assertEquals(similar[0], L"five");

  similar = spellChecker->suggestSimilar(L"five", 2);
  assertEquals(1, similar.size());
  assertEquals(similar[0], L"nine"); // don't suggest a word for itself

  similar = spellChecker->suggestSimilar(L"fiv", 2);
  assertEquals(1, similar.size());
  assertEquals(similar[0], L"five");

  similar = spellChecker->suggestSimilar(L"ive", 2);
  assertEquals(2, similar.size());
  assertEquals(similar[0], L"five");
  assertEquals(similar[1], L"nine");

  similar = spellChecker->suggestSimilar(L"fives", 2);
  assertEquals(1, similar.size());
  assertEquals(similar[0], L"five");

  similar = spellChecker->suggestSimilar(L"fie", 2);
  assertEquals(2, similar.size());
  assertEquals(similar[0], L"five");
  assertEquals(similar[1], L"nine");

  similar = spellChecker->suggestSimilar(L"fi", 2);
  assertEquals(1, similar.size());
  assertEquals(similar[0], L"five");

  // test restraint to a field
  similar = spellChecker->suggestSimilar(
      L"tousand", 10, r, L"field1", SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX);
  assertEquals(
      0, similar.size()); // there isn't the term thousand in the field field1

  similar = spellChecker->suggestSimilar(
      L"tousand", 10, r, L"field2", SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX);
  assertEquals(
      1, similar.size()); // there is the term thousand in the field field2

  similar = spellChecker->suggestSimilar(L"onety", 2);
  assertEquals(2, similar.size());
  assertEquals(similar[0], L"ninety");
  assertEquals(similar[1], L"one");
  // should not throw exception
  spellChecker->suggestSimilar(L"tousand", 10, r, L"",
                               SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX);
}

void TestSpellChecker::checkJaroWinklerSuggestions() 
{
  std::deque<wstring> similar = spellChecker->suggestSimilar(L"onety", 2);
  assertEquals(2, similar.size());
  assertEquals(similar[0], L"one");
  assertEquals(similar[1], L"ninety");
}

void TestSpellChecker::checkNGramSuggestions() 
{
  std::deque<wstring> similar = spellChecker->suggestSimilar(L"onety", 2);
  assertEquals(2, similar.size());
  assertEquals(similar[0], L"one");
  assertEquals(similar[1], L"ninety");
}

void TestSpellChecker::addwords(shared_ptr<IndexReader> r,
                                shared_ptr<SpellChecker> sc,
                                const wstring &field) 
{
  int64_t time = System::currentTimeMillis();
  sc->indexDictionary(make_shared<LuceneDictionary>(r, field),
                      newIndexWriterConfig(nullptr), false);
  time = System::currentTimeMillis() - time;
  // System.out.println("time to build " + field + ": " + time);
}

int TestSpellChecker::numdoc() 
{
  shared_ptr<IndexReader> rs = DirectoryReader::open(spellindex);
  int num = rs->numDocs();
  assertTrue(num != 0);
  // System.out.println("num docs: " + num);
  delete rs;
  return num;
}

void TestSpellChecker::testClose() 
{
  shared_ptr<IndexReader> r = DirectoryReader::open(userindex);
  spellChecker->clearIndex();
  wstring field = L"field1";
  addwords(r, spellChecker, L"field1");
  int num_field1 = this->numdoc();
  addwords(r, spellChecker, L"field2");
  int num_field2 = this->numdoc();
  assertEquals(num_field2, num_field1 + 1);
  checkCommonSuggestions(r);
  assertLastSearcherOpen(4);
  delete spellChecker;
  assertSearchersClosed();

  expectThrows(AlreadyClosedException::typeid, [&]() { delete spellChecker; });

  expectThrows(AlreadyClosedException::typeid,
               [&]() { checkCommonSuggestions(r); });

  expectThrows(AlreadyClosedException::typeid,
               [&]() { spellChecker->clearIndex(); });

  expectThrows(AlreadyClosedException::typeid, [&]() {
    spellChecker->indexDictionary(make_shared<LuceneDictionary>(r, field),
                                  newIndexWriterConfig(nullptr), false);
  });

  expectThrows(AlreadyClosedException::typeid,
               [&]() { spellChecker->setSpellIndex(spellindex); });

  assertEquals(4, searchers.size());
  assertSearchersClosed();
  delete r;
}

void TestSpellChecker::testConcurrentAccess() throw(IOException,
                                                    InterruptedException)
{
  assertEquals(1, searchers.size());
  shared_ptr<IndexReader> *const r = DirectoryReader::open(userindex);
  spellChecker->clearIndex();
  assertEquals(2, searchers.size());
  addwords(r, spellChecker, L"field1");
  assertEquals(3, searchers.size());
  int num_field1 = this->numdoc();
  addwords(r, spellChecker, L"field2");
  assertEquals(4, searchers.size());
  int num_field2 = this->numdoc();
  assertEquals(num_field2, num_field1 + 1);
  int numThreads = 5 + random()->nextInt(5);
  shared_ptr<ExecutorService> executor = Executors::newFixedThreadPool(
      numThreads, make_shared<NamedThreadFactory>(L"testConcurrentAccess"));
  std::deque<std::shared_ptr<SpellCheckWorker>> workers(numThreads);
  for (int i = 0; i < numThreads; i++) {
    shared_ptr<SpellCheckWorker> spellCheckWorker =
        make_shared<SpellCheckWorker>(shared_from_this(), r);
    executor->execute(spellCheckWorker);
    workers[i] = spellCheckWorker;
  }
  int iterations = 5 + random()->nextInt(5);
  for (int i = 0; i < iterations; i++) {
    delay(100);
    // concurrently reset the spell index
    spellChecker->setSpellIndex(this->spellindex);
    // for debug - prints the internal open searchers
    // showSearchersOpen();
  }

  delete spellChecker;
  executor->shutdown();
  // wait for 60 seconds - usually this is very fast but coverage runs could
  // take quite long
  executor->awaitTermination(60LL, TimeUnit::SECONDS);

  for (int i = 0; i < workers.size(); i++) {
    assertFalse(wstring::format(Locale::ROOT, L"worker thread %d failed", i),
                workers[i]->failed);
    assertTrue(
        wstring::format(
            Locale::ROOT,
            L"worker thread %d is still running but should be terminated", i),
        workers[i]->terminated);
  }
  // 4 searchers more than iterations
  // 1. at creation
  // 2. clearIndex()
  // 2. and 3. during addwords
  assertEquals(iterations + 4, searchers.size());
  assertSearchersClosed();
  delete r;
}

void TestSpellChecker::assertLastSearcherOpen(int numSearchers)
{
  assertEquals(numSearchers, searchers.size());
  std::deque<std::shared_ptr<IndexSearcher>> searcherArray =
      searchers.toArray(std::deque<std::shared_ptr<IndexSearcher>>(0));
  for (int i = 0; i < searcherArray.size(); i++) {
    if (i == searcherArray.size() - 1) {
      assertTrue(L"expected last searcher open but was closed",
                 searcherArray[i]->getIndexReader().getRefCount() > 0);
    } else {
      assertFalse(L"expected closed searcher but was open - Index: " +
                      to_wstring(i),
                  searcherArray[i]->getIndexReader().getRefCount() > 0);
    }
  }
}

void TestSpellChecker::assertSearchersClosed()
{
  for (auto searcher : searchers) {
    assertEquals(0, searcher->getIndexReader()->getRefCount());
  }
}

// C++ TODO: No base class can be determined:
TestSpellChecker::SpellCheckWorker::SpellCheckWorker(
    shared_ptr<TestSpellChecker> outerInstance, shared_ptr<IndexReader> reader)
    : reader(reader), outerInstance(outerInstance)
{
}

void TestSpellChecker::SpellCheckWorker::run()
{
  try {
    while (true) {
      try {
        outerInstance->checkCommonSuggestions(reader);
      } catch (const AlreadyClosedException &e) {

        return;
      } catch (const runtime_error &e) {

        e.printStackTrace();
        failed = true;
        return;
      }
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    terminated = true;
  }
}

TestSpellChecker::SpellCheckerMock::SpellCheckerMock(
    shared_ptr<TestSpellChecker> outerInstance,
    shared_ptr<Directory> spellIndex) 
    : SpellChecker(spellIndex), outerInstance(outerInstance)
{
}

TestSpellChecker::SpellCheckerMock::SpellCheckerMock(
    shared_ptr<TestSpellChecker> outerInstance,
    shared_ptr<Directory> spellIndex,
    shared_ptr<StringDistance> sd) 
    : SpellChecker(spellIndex, sd), outerInstance(outerInstance)
{
}

TestSpellChecker::SpellCheckerMock::SpellCheckerMock(
    shared_ptr<TestSpellChecker> outerInstance,
    shared_ptr<Directory> spellIndex, shared_ptr<StringDistance> sd,
    shared_ptr<Comparator<std::shared_ptr<SuggestWord>>>
        comparator) 
    : SpellChecker(spellIndex, sd, comparator), outerInstance(outerInstance)
{
}

shared_ptr<IndexSearcher> TestSpellChecker::SpellCheckerMock::createSearcher(
    shared_ptr<Directory> dir) 
{
  shared_ptr<IndexSearcher> searcher = SpellChecker::createSearcher(dir);
  outerInstance->searchers.add(searcher);
  return searcher;
}
} // namespace org::apache::lucene::search::spell