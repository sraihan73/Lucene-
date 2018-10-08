using namespace std;

#include "TestFlushByRamOrCountsPolicy.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using ThreadState =
    org::apache::lucene::index::DocumentsWriterPerThreadPool::ThreadState;
using Directory = org::apache::lucene::store::Directory;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using LineFileDocs = org::apache::lucene::util::LineFileDocs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::AfterClass;
using org::junit::BeforeClass;
shared_ptr<org::apache::lucene::util::LineFileDocs>
    TestFlushByRamOrCountsPolicy::lineDocFile;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestFlushByRamOrCountsPolicy::beforeClass() 
{
  lineDocFile = make_shared<LineFileDocs>(random());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
void TestFlushByRamOrCountsPolicy::afterClass() 
{
  delete lineDocFile;
  lineDocFile.reset();
}

void TestFlushByRamOrCountsPolicy::testFlushByRam() throw(IOException,
                                                          InterruptedException)
{
  constexpr double ramBuffer =
      (TEST_NIGHTLY ? 1 : 10) + atLeast(2) + random()->nextDouble();
  runFlushByRam(1 + random()->nextInt(TEST_NIGHTLY ? 5 : 1), ramBuffer, false);
}

void TestFlushByRamOrCountsPolicy::testFlushByRamLargeBuffer() throw(
    IOException, InterruptedException)
{
  // with a 256 mb ram buffer we should never stall
  runFlushByRam(1 + random()->nextInt(TEST_NIGHTLY ? 5 : 1), 256.d, true);
}

void TestFlushByRamOrCountsPolicy::runFlushByRam(
    int numThreads, double maxRamMB,
    bool ensureNotStalled) 
{
  constexpr int numDocumentsToIndex = 10 + atLeast(30);
  shared_ptr<AtomicInteger> numDocs =
      make_shared<AtomicInteger>(numDocumentsToIndex);
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<MockDefaultFlushPolicy> flushPolicy =
      make_shared<MockDefaultFlushPolicy>();
  shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
  analyzer->setMaxTokenLength(
      TestUtil::nextInt(random(), 1, IndexWriter::MAX_TERM_LENGTH));

  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(analyzer)->setFlushPolicy(flushPolicy);
  shared_ptr<DocumentsWriterPerThreadPool> threadPool =
      make_shared<DocumentsWriterPerThreadPool>();
  iwc->setIndexerThreadPool(threadPool);
  iwc->setRAMBufferSizeMB(maxRamMB);
  iwc->setMaxBufferedDocs(IndexWriterConfig::DISABLE_AUTO_FLUSH);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, iwc);
  flushPolicy = std::static_pointer_cast<MockDefaultFlushPolicy>(
      writer->getConfig()->getFlushPolicy());
  assertFalse(flushPolicy->flushOnDocCount());
  assertTrue(flushPolicy->flushOnRAM());
  shared_ptr<DocumentsWriter> docsWriter = writer->getDocsWriter();
  assertNotNull(docsWriter);
  shared_ptr<DocumentsWriterFlushControl> flushControl =
      docsWriter->flushControl;
  assertEquals(L" bytes must be 0 after init", 0, flushControl->flushBytes());

  std::deque<std::shared_ptr<IndexThread>> threads(numThreads);
  for (int x = 0; x < threads.size(); x++) {
    threads[x] = make_shared<IndexThread>(numDocs, numThreads, writer,
                                          lineDocFile, false);
    threads[x]->start();
  }

  for (int x = 0; x < threads.size(); x++) {
    threads[x]->join();
  }
  constexpr int64_t maxRAMBytes =
      static_cast<int64_t>(iwc->getRAMBufferSizeMB() * 1024.0 * 1024.0);
  assertEquals(L" all flushes must be due numThreads=" + to_wstring(numThreads),
               0, flushControl->flushBytes());
  TestUtil::assertEquals(numDocumentsToIndex, writer->numDocs());
  TestUtil::assertEquals(numDocumentsToIndex, writer->maxDoc());
  assertTrue(L"peak bytes without flush exceeded watermark",
             flushPolicy->peakBytesWithoutFlush <= maxRAMBytes);
  assertActiveBytesAfter(flushControl);
  if (flushPolicy->hasMarkedPending) {
    assertTrue(maxRAMBytes < flushControl->peakActiveBytes);
  }
  if (ensureNotStalled) {
    assertFalse(docsWriter->flushControl->stallControl->wasStalled());
  }
  delete writer;
  TestUtil::assertEquals(0, flushControl->activeBytes());
  delete dir;
}

void TestFlushByRamOrCountsPolicy::testFlushDocCount() throw(
    IOException, InterruptedException)
{
  std::deque<int> numThreads = {2 + atLeast(1), 1};
  shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
  analyzer->setMaxTokenLength(
      TestUtil::nextInt(random(), 1, IndexWriter::MAX_TERM_LENGTH));
  for (int i = 0; i < numThreads.size(); i++) {

    constexpr int numDocumentsToIndex = 50 + atLeast(30);
    shared_ptr<AtomicInteger> numDocs =
        make_shared<AtomicInteger>(numDocumentsToIndex);
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<MockDefaultFlushPolicy> flushPolicy =
        make_shared<MockDefaultFlushPolicy>();
    shared_ptr<IndexWriterConfig> iwc =
        newIndexWriterConfig(analyzer)->setFlushPolicy(flushPolicy);

    shared_ptr<DocumentsWriterPerThreadPool> threadPool =
        make_shared<DocumentsWriterPerThreadPool>();
    iwc->setIndexerThreadPool(threadPool);
    iwc->setMaxBufferedDocs(2 + atLeast(10));
    iwc->setRAMBufferSizeMB(IndexWriterConfig::DISABLE_AUTO_FLUSH);
    shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, iwc);
    flushPolicy = std::static_pointer_cast<MockDefaultFlushPolicy>(
        writer->getConfig()->getFlushPolicy());
    assertTrue(flushPolicy->flushOnDocCount());
    assertFalse(flushPolicy->flushOnRAM());
    shared_ptr<DocumentsWriter> docsWriter = writer->getDocsWriter();
    assertNotNull(docsWriter);
    shared_ptr<DocumentsWriterFlushControl> flushControl =
        docsWriter->flushControl;
    assertEquals(L" bytes must be 0 after init", 0, flushControl->flushBytes());

    std::deque<std::shared_ptr<IndexThread>> threads(numThreads[i]);
    for (int x = 0; x < threads.size(); x++) {
      threads[x] = make_shared<IndexThread>(numDocs, numThreads[i], writer,
                                            lineDocFile, false);
      threads[x]->start();
    }

    for (int x = 0; x < threads.size(); x++) {
      threads[x]->join();
    }

    assertEquals(L" all flushes must be due numThreads=" +
                     to_wstring(numThreads[i]),
                 0, flushControl->flushBytes());
    TestUtil::assertEquals(numDocumentsToIndex, writer->numDocs());
    TestUtil::assertEquals(numDocumentsToIndex, writer->maxDoc());
    assertTrue(L"peak bytes without flush exceeded watermark",
               flushPolicy->peakDocCountWithoutFlush <=
                   iwc->getMaxBufferedDocs());
    assertActiveBytesAfter(flushControl);
    delete writer;
    TestUtil::assertEquals(0, flushControl->activeBytes());
    delete dir;
  }
}

void TestFlushByRamOrCountsPolicy::testRandom() throw(IOException,
                                                      InterruptedException)
{
  constexpr int numThreads = 1 + random()->nextInt(8);
  constexpr int numDocumentsToIndex = 50 + atLeast(70);
  shared_ptr<AtomicInteger> numDocs =
      make_shared<AtomicInteger>(numDocumentsToIndex);
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
  analyzer->setMaxTokenLength(
      TestUtil::nextInt(random(), 1, IndexWriter::MAX_TERM_LENGTH));
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(analyzer);
  shared_ptr<MockDefaultFlushPolicy> flushPolicy =
      make_shared<MockDefaultFlushPolicy>();
  iwc->setFlushPolicy(flushPolicy);

  shared_ptr<DocumentsWriterPerThreadPool> threadPool =
      make_shared<DocumentsWriterPerThreadPool>();
  iwc->setIndexerThreadPool(threadPool);

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, iwc);
  flushPolicy = std::static_pointer_cast<MockDefaultFlushPolicy>(
      writer->getConfig()->getFlushPolicy());
  shared_ptr<DocumentsWriter> docsWriter = writer->getDocsWriter();
  assertNotNull(docsWriter);
  shared_ptr<DocumentsWriterFlushControl> flushControl =
      docsWriter->flushControl;

  assertEquals(L" bytes must be 0 after init", 0, flushControl->flushBytes());

  std::deque<std::shared_ptr<IndexThread>> threads(numThreads);
  for (int x = 0; x < threads.size(); x++) {
    threads[x] = make_shared<IndexThread>(numDocs, numThreads, writer,
                                          lineDocFile, true);
    threads[x]->start();
  }

  for (int x = 0; x < threads.size(); x++) {
    threads[x]->join();
  }
  assertEquals(L" all flushes must be due", 0, flushControl->flushBytes());
  TestUtil::assertEquals(numDocumentsToIndex, writer->numDocs());
  TestUtil::assertEquals(numDocumentsToIndex, writer->maxDoc());
  if (flushPolicy->flushOnRAM() && !flushPolicy->flushOnDocCount()) {
    constexpr int64_t maxRAMBytes =
        static_cast<int64_t>(iwc->getRAMBufferSizeMB() * 1024.0 * 1024.0);
    assertTrue(L"peak bytes without flush exceeded watermark",
               flushPolicy->peakBytesWithoutFlush <= maxRAMBytes);
    if (flushPolicy->hasMarkedPending) {
      assertTrue(L"max: " + to_wstring(maxRAMBytes) + L" " +
                     to_wstring(flushControl->peakActiveBytes),
                 maxRAMBytes <= flushControl->peakActiveBytes);
    }
  }
  assertActiveBytesAfter(flushControl);
  writer->commit();
  TestUtil::assertEquals(0, flushControl->activeBytes());
  shared_ptr<IndexReader> r = DirectoryReader::open(dir);
  TestUtil::assertEquals(numDocumentsToIndex, r->numDocs());
  TestUtil::assertEquals(numDocumentsToIndex, r->maxDoc());
  if (!flushPolicy->flushOnRAM()) {
    assertFalse(L"never stall if we don't flush on RAM",
                docsWriter->flushControl->stallControl->wasStalled());
    assertFalse(L"never block if we don't flush on RAM",
                docsWriter->flushControl->stallControl->hasBlocked());
  }
  delete r;
  delete writer;
  delete dir;
}

void TestFlushByRamOrCountsPolicy::testStallControl() throw(
    InterruptedException, IOException)
{

  std::deque<int> numThreads = {4 + random()->nextInt(8), 1};
  constexpr int numDocumentsToIndex = 50 + random()->nextInt(50);
  shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
  analyzer->setMaxTokenLength(
      TestUtil::nextInt(random(), 1, IndexWriter::MAX_TERM_LENGTH));
  for (int i = 0; i < numThreads.size(); i++) {
    shared_ptr<AtomicInteger> numDocs =
        make_shared<AtomicInteger>(numDocumentsToIndex);
    shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
    // mock a very slow harddisk sometimes here so that flushing is very slow
    dir->setThrottling(MockDirectoryWrapper::Throttling::SOMETIMES);
    shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(analyzer);
    iwc->setMaxBufferedDocs(IndexWriterConfig::DISABLE_AUTO_FLUSH);
    shared_ptr<FlushPolicy> flushPolicy =
        make_shared<FlushByRamOrCountsPolicy>();
    iwc->setFlushPolicy(flushPolicy);

    shared_ptr<DocumentsWriterPerThreadPool> threadPool =
        make_shared<DocumentsWriterPerThreadPool>();
    iwc->setIndexerThreadPool(threadPool);
    // with such a small ram buffer we should be stalled quite quickly
    iwc->setRAMBufferSizeMB(0.25);
    shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, iwc);
    std::deque<std::shared_ptr<IndexThread>> threads(numThreads[i]);
    for (int x = 0; x < threads.size(); x++) {
      threads[x] = make_shared<IndexThread>(numDocs, numThreads[i], writer,
                                            lineDocFile, false);
      threads[x]->start();
    }

    for (int x = 0; x < threads.size(); x++) {
      threads[x]->join();
    }
    shared_ptr<DocumentsWriter> docsWriter = writer->getDocsWriter();
    assertNotNull(docsWriter);
    shared_ptr<DocumentsWriterFlushControl> flushControl =
        docsWriter->flushControl;
    assertEquals(L" all flushes must be due", 0, flushControl->flushBytes());
    TestUtil::assertEquals(numDocumentsToIndex, writer->numDocs());
    TestUtil::assertEquals(numDocumentsToIndex, writer->maxDoc());
    if (numThreads[i] == 1) {
      assertFalse(L"single thread must not block numThreads: " +
                      to_wstring(numThreads[i]),
                  docsWriter->flushControl->stallControl->hasBlocked());
    }
    if (docsWriter->flushControl->peakNetBytes >
        (2.d * iwc->getRAMBufferSizeMB() * 1024.d * 1024.d)) {
      assertTrue(docsWriter->flushControl->stallControl->wasStalled());
    }
    assertActiveBytesAfter(flushControl);
    delete writer;
    delete dir;
  }
}

void TestFlushByRamOrCountsPolicy::assertActiveBytesAfter(
    shared_ptr<DocumentsWriterFlushControl> flushControl)
{
  Iterator<std::shared_ptr<ThreadState>> allActiveThreads =
      flushControl->allActiveThreadStates();
  int64_t bytesUsed = 0;
  while (allActiveThreads->hasNext()) {
    shared_ptr<ThreadState> next = allActiveThreads->next();
    if (next->dwpt != nullptr) {
      bytesUsed += next->dwpt->bytesUsed();
    }
    allActiveThreads++;
  }
  TestUtil::assertEquals(bytesUsed, flushControl->activeBytes());
}

TestFlushByRamOrCountsPolicy::IndexThread::IndexThread(
    shared_ptr<AtomicInteger> pendingDocs, int numThreads,
    shared_ptr<IndexWriter> writer, shared_ptr<LineFileDocs> docs,
    bool doRandomCommit)
    : doRandomCommit(doRandomCommit)
{
  this->pendingDocs = pendingDocs;
  this->writer = writer;
  iwc = writer->getConfig();
  this->docs = docs;
}

void TestFlushByRamOrCountsPolicy::IndexThread::run()
{
  try {
    int64_t ramSize = 0;
    while (pendingDocs->decrementAndGet() > -1) {
      shared_ptr<Document> doc = docs->nextDoc();
      writer->addDocument(doc);
      int64_t newRamSize = writer->ramBytesUsed();
      if (newRamSize != ramSize) {
        ramSize = newRamSize;
      }
      if (doRandomCommit) {
        if (LuceneTestCase::rarely()) {
          writer->commit();
        }
      }
    }
    writer->commit();
  } catch (const runtime_error &ex) {
    wcout << L"FAILED exc:" << endl;
    ex.printStackTrace(System::out);
    throw runtime_error(ex);
  }
}

void TestFlushByRamOrCountsPolicy::MockDefaultFlushPolicy::onDelete(
    shared_ptr<DocumentsWriterFlushControl> control,
    shared_ptr<ThreadState> state)
{
  const deque<std::shared_ptr<ThreadState>> pending =
      deque<std::shared_ptr<ThreadState>>();
  const deque<std::shared_ptr<ThreadState>> notPending =
      deque<std::shared_ptr<ThreadState>>();
  findPending(control, pending, notPending);
  constexpr bool flushCurrent = state->flushPending;
  shared_ptr<ThreadState> *const toFlush;
  if (state->flushPending) {
    toFlush = state;
  } else {
    toFlush.reset();
  }
  FlushByRamOrCountsPolicy::onDelete(control, state);
  if (toFlush != nullptr) {
    if (flushCurrent) {
      // C++ TODO: The Java deque 'remove(Object)' method is not converted:
      assertTrue(pending.remove(toFlush));
    } else {
      // C++ TODO: The Java deque 'remove(Object)' method is not converted:
      assertTrue(notPending.remove(toFlush));
    }
    assertTrue(toFlush->flushPending);
    hasMarkedPending = true;
  }

  for (auto threadState : notPending) {
    assertFalse(threadState->flushPending);
  }
}

void TestFlushByRamOrCountsPolicy::MockDefaultFlushPolicy::onInsert(
    shared_ptr<DocumentsWriterFlushControl> control,
    shared_ptr<ThreadState> state)
{
  const deque<std::shared_ptr<ThreadState>> pending =
      deque<std::shared_ptr<ThreadState>>();
  const deque<std::shared_ptr<ThreadState>> notPending =
      deque<std::shared_ptr<ThreadState>>();
  findPending(control, pending, notPending);
  constexpr bool flushCurrent = state->flushPending;
  int64_t activeBytes = control->activeBytes();
  shared_ptr<ThreadState> *const toFlush;
  if (state->flushPending) {
    toFlush = state;
  } else if (flushOnDocCount() && state->dwpt->getNumDocsInRAM() >=
                                      indexWriterConfig->getMaxBufferedDocs()) {
    toFlush = state;
  } else if (flushOnRAM() &&
             activeBytes >= static_cast<int64_t>(
                                indexWriterConfig->getRAMBufferSizeMB() *
                                1024.0 * 1024.0)) {
    toFlush = findLargestNonPendingWriter(control, state);
    assertFalse(toFlush->flushPending);
  } else {
    toFlush.reset();
  }
  FlushByRamOrCountsPolicy::onInsert(control, state);
  if (toFlush != nullptr) {
    if (flushCurrent) {
      // C++ TODO: The Java deque 'remove(Object)' method is not converted:
      assertTrue(pending.remove(toFlush));
    } else {
      // C++ TODO: The Java deque 'remove(Object)' method is not converted:
      assertTrue(notPending.remove(toFlush));
    }
    assertTrue(toFlush->flushPending);
    hasMarkedPending = true;
  } else {
    peakBytesWithoutFlush = max(activeBytes, peakBytesWithoutFlush);
    peakDocCountWithoutFlush =
        max(state->dwpt->getNumDocsInRAM(), peakDocCountWithoutFlush);
  }

  for (auto threadState : notPending) {
    assertFalse(threadState->flushPending);
  }
}

void TestFlushByRamOrCountsPolicy::findPending(
    shared_ptr<DocumentsWriterFlushControl> flushControl,
    deque<std::shared_ptr<ThreadState>> &pending,
    deque<std::shared_ptr<ThreadState>> &notPending)
{
  Iterator<std::shared_ptr<ThreadState>> allActiveThreads =
      flushControl->allActiveThreadStates();
  while (allActiveThreads->hasNext()) {
    shared_ptr<ThreadState> next = allActiveThreads->next();
    if (next->flushPending) {
      pending.push_back(next);
    } else {
      notPending.push_back(next);
    }
    allActiveThreads++;
  }
}
} // namespace org::apache::lucene::index