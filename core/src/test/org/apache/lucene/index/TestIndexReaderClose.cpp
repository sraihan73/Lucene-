using namespace std;

#include "TestIndexReaderClose.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestIndexReaderClose::testCloseUnderException() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(random(), make_shared<MockAnalyzer>(random())));
  writer->addDocument(make_shared<Document>());
  writer->commit();
  delete writer;
  constexpr int iters = 1000 + 1 + random()->nextInt(20);
  for (int j = 0; j < iters; j++) {
    shared_ptr<DirectoryReader> open = DirectoryReader::open(dir);
    constexpr bool throwOnClose = !rarely();
    shared_ptr<LeafReader> leaf = getOnlyLeafReader(open);
    shared_ptr<FilterLeafReader> reader =
        make_shared<FilterLeafReaderAnonymousInnerClass>(shared_from_this(),
                                                         leaf, throwOnClose);
    int listenerCount = random()->nextInt(20);
    shared_ptr<AtomicInteger> count = make_shared<AtomicInteger>();
    bool faultySet = false;
    for (int i = 0; i < listenerCount; i++) {
      if (rarely()) {
        faultySet = true;
        reader->getReaderCacheHelper()->addClosedListener(
            make_shared<FaultyListener>());
      } else {
        count->incrementAndGet();
        reader->getReaderCacheHelper()->addClosedListener(
            make_shared<CountListener>(
                count, reader->getReaderCacheHelper()->getKey()));
      }
    }
    if (!faultySet && !throwOnClose) {
      reader->getReaderCacheHelper()->addClosedListener(
          make_shared<FaultyListener>());
    }

    shared_ptr<IllegalStateException> expected =
        expectThrows(IllegalStateException::typeid, [&]() { delete reader; });

    if (throwOnClose) {
      TestUtil::assertEquals(L"BOOM!", expected->getMessage());
    } else {
      TestUtil::assertEquals(L"GRRRRRRRRRRRR!", expected->getMessage());
    }

    expectThrows(AlreadyClosedException::typeid,
                 [&]() { reader->terms(L"someField"); });

    if (random()->nextBoolean()) {
      delete reader; // call it again
    }
    TestUtil::assertEquals(0, count->get());
  }
  delete dir;
}

TestIndexReaderClose::FilterLeafReaderAnonymousInnerClass::
    FilterLeafReaderAnonymousInnerClass(
        shared_ptr<TestIndexReaderClose> outerInstance,
        shared_ptr<org::apache::lucene::index::LeafReader> leaf,
        bool throwOnClose)
    : FilterLeafReader(leaf)
{
  this->outerInstance = outerInstance;
  this->throwOnClose = throwOnClose;
}

shared_ptr<CacheHelper>
TestIndexReaderClose::FilterLeafReaderAnonymousInnerClass::getCoreCacheHelper()
{
  return in_::getCoreCacheHelper();
}

shared_ptr<CacheHelper> TestIndexReaderClose::
    FilterLeafReaderAnonymousInnerClass::getReaderCacheHelper()
{
  return in_::getReaderCacheHelper();
}

void TestIndexReaderClose::FilterLeafReaderAnonymousInnerClass::doClose() throw(
    IOException)
{
  try {
    outerInstance->super->doClose();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (throwOnClose) {
      throw make_shared<IllegalStateException>(L"BOOM!");
    }
  }
}

void TestIndexReaderClose::
    testCoreListenerOnWrapperWithDifferentCacheKey() 
{
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), newDirectory());
  constexpr int numDocs = TestUtil::nextInt(random(), 1, 5);
  for (int i = 0; i < numDocs; ++i) {
    w->addDocument(make_shared<Document>());
    if (random()->nextBoolean()) {
      w->commit();
    }
  }
  w->forceMerge(1);
  w->commit();
  delete w;

  shared_ptr<IndexReader> *const reader =
      DirectoryReader::open(w->w->getDirectory());
  shared_ptr<LeafReader> *const leafReader =
      make_shared<AssertingLeafReader>(getOnlyLeafReader(reader));

  constexpr int numListeners = TestUtil::nextInt(random(), 1, 10);
  const deque<IndexReader::ClosedListener> listeners =
      deque<IndexReader::ClosedListener>();
  shared_ptr<AtomicInteger> counter = make_shared<AtomicInteger>(numListeners);

  for (int i = 0; i < numListeners; ++i) {
    shared_ptr<CountListener> listener = make_shared<CountListener>(
        counter, leafReader->getCoreCacheHelper()->getKey());
    listeners.push_back(listener);
    leafReader->getCoreCacheHelper()->addClosedListener(listener);
  }
  for (int i = 0; i < 100; ++i) {
    leafReader->getCoreCacheHelper()->addClosedListener(
        listeners[random()->nextInt(listeners.size())]);
  }
  TestUtil::assertEquals(numListeners, counter->get());
  // make sure listeners are registered on the wrapped reader and that closing
  // any of them has the same effect
  if (random()->nextBoolean()) {
    delete reader;
  } else {
    delete leafReader;
  }
  TestUtil::assertEquals(0, counter->get());
  delete w->w->getDirectory();
}

TestIndexReaderClose::CountListener::CountListener(
    shared_ptr<AtomicInteger> count, any coreCacheKey)
    : count(count), coreCacheKey(coreCacheKey)
{
}

void TestIndexReaderClose::CountListener::onClose(
    shared_ptr<IndexReader::CacheKey> coreCacheKey)
{
  assertSame(this->coreCacheKey, coreCacheKey);
  count->decrementAndGet();
}

void TestIndexReaderClose::FaultyListener::onClose(
    shared_ptr<IndexReader::CacheKey> cacheKey)
{
  throw make_shared<IllegalStateException>(L"GRRRRRRRRRRRR!");
}

void TestIndexReaderClose::testRegisterListenerOnClosedReader() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, newIndexWriterConfig());
  w->addDocument(make_shared<Document>());
  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  delete w;

  // The reader is open, everything should work
  r->getReaderCacheHelper().addClosedListener([&](any key) {});
  r->leaves()->get(0).reader().getReaderCacheHelper().addClosedListener(
      [&](any key) {});
  r->leaves()->get(0).reader().getCoreCacheHelper().addClosedListener(
      [&](any key) {});

  // But now we close
  r->close();
  expectThrows(AlreadyClosedException::typeid, [&]() {
    r->getReaderCacheHelper().addClosedListener([&](any key) {});
  });
  expectThrows(AlreadyClosedException::typeid, [&]() {
    r->leaves()->get(0).reader().getReaderCacheHelper().addClosedListener(
        [&](any key) {});
  });
  expectThrows(AlreadyClosedException::typeid, [&]() {
    r->leaves()->get(0).reader().getCoreCacheHelper().addClosedListener(
        [&](any key) {});
  });

  delete dir;
}
} // namespace org::apache::lucene::index