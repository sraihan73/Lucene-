using namespace std;

#include "TestReaderPool.h"

namespace org::apache::lucene::index
{
using com::carrotsearch::randomizedtesting::generators::RandomPicks;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOSupplier = org::apache::lucene::util::IOSupplier;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using NullInfoStream = org::apache::lucene::util::NullInfoStream;

void TestReaderPool::testDrop() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<FieldInfos::FieldNumbers> fieldNumbers = buildIndex(directory);
  shared_ptr<StandardDirectoryReader> reader =
      std::static_pointer_cast<StandardDirectoryReader>(
          DirectoryReader::open(directory));
  shared_ptr<SegmentInfos> segmentInfos = reader->segmentInfos->clone();

  shared_ptr<ReaderPool> pool =
      make_shared<ReaderPool>(directory, directory, segmentInfos, fieldNumbers,
                              [&]() { 0LL; }, nullptr, nullptr, nullptr);
  shared_ptr<SegmentCommitInfo> commitInfo =
      RandomPicks::randomFrom(random(), segmentInfos->asList());
  shared_ptr<ReadersAndUpdates> readersAndUpdates = pool->get(commitInfo, true);
  assertSame(readersAndUpdates, pool->get(commitInfo, false));
  assertTrue(pool->drop(commitInfo));
  if (random()->nextBoolean()) {
    assertFalse(pool->drop(commitInfo));
  }
  assertNull(pool->get(commitInfo, false));
  pool->release(readersAndUpdates, random()->nextBoolean());
  IOUtils::close({pool, reader, directory});
}

void TestReaderPool::testPoolReaders() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<FieldInfos::FieldNumbers> fieldNumbers = buildIndex(directory);
  shared_ptr<StandardDirectoryReader> reader =
      std::static_pointer_cast<StandardDirectoryReader>(
          DirectoryReader::open(directory));
  shared_ptr<SegmentInfos> segmentInfos = reader->segmentInfos->clone();

  shared_ptr<ReaderPool> pool =
      make_shared<ReaderPool>(directory, directory, segmentInfos, fieldNumbers,
                              [&]() { 0LL; }, nullptr, nullptr, nullptr);
  shared_ptr<SegmentCommitInfo> commitInfo =
      RandomPicks::randomFrom(random(), segmentInfos->asList());
  assertFalse(pool->isReaderPoolingEnabled());
  pool->release(pool->get(commitInfo, true), random()->nextBoolean());
  assertNull(pool->get(commitInfo, false));
  // now start pooling
  pool->enableReaderPooling();
  assertTrue(pool->isReaderPoolingEnabled());
  pool->release(pool->get(commitInfo, true), random()->nextBoolean());
  assertNotNull(pool->get(commitInfo, false));
  assertSame(pool->get(commitInfo, false), pool->get(commitInfo, false));
  pool->drop(commitInfo);
  int64_t ramBytesUsed = 0;
  assertEquals(0, pool->ramBytesUsed());
  for (auto info : segmentInfos) {
    pool->release(pool->get(info, true), random()->nextBoolean());
    assertEquals(L" used: " + to_wstring(ramBytesUsed) + L" actual: " +
                     to_wstring(pool->ramBytesUsed()),
                 0, pool->ramBytesUsed());
    ramBytesUsed = pool->ramBytesUsed();
    assertSame(pool->get(info, false), pool->get(info, false));
  }
  assertNotSame(0, pool->ramBytesUsed());
  pool->dropAll();
  for (auto info : segmentInfos) {
    assertNull(pool->get(info, false));
  }
  assertEquals(0, pool->ramBytesUsed());
  IOUtils::close({pool, reader, directory});
}

void TestReaderPool::testUpdate() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<FieldInfos::FieldNumbers> fieldNumbers = buildIndex(directory);
  shared_ptr<StandardDirectoryReader> reader =
      std::static_pointer_cast<StandardDirectoryReader>(
          DirectoryReader::open(directory));
  shared_ptr<SegmentInfos> segmentInfos = reader->segmentInfos->clone();
  shared_ptr<ReaderPool> pool = make_shared<ReaderPool>(
      directory, directory, segmentInfos, fieldNumbers, [&]() { 0LL; },
      make_shared<NullInfoStream>(), nullptr, nullptr);
  int id = random()->nextInt(10);
  if (random()->nextBoolean()) {
    pool->enableReaderPooling();
  }
  for (auto commitInfo : segmentInfos) {
    shared_ptr<ReadersAndUpdates> readersAndUpdates =
        pool->get(commitInfo, true);
    shared_ptr<SegmentReader> readOnlyClone =
        readersAndUpdates->getReadOnlyClone(IOContext::READ);
    shared_ptr<PostingsEnum> postings =
        readOnlyClone->postings(make_shared<Term>(L"id", L"" + to_wstring(id)));
    bool expectUpdate = false;
    int doc = -1;
    if (postings != nullptr &&
        postings->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
      shared_ptr<NumericDocValuesFieldUpdates> number =
          make_shared<NumericDocValuesFieldUpdates>(0, L"number",
                                                    commitInfo->info->maxDoc());
      number->add(doc = postings->docID(), 1000LL);
      number->finish();
      readersAndUpdates->addDVUpdate(number);
      expectUpdate = true;
      assertEquals(DocIdSetIterator::NO_MORE_DOCS, postings->nextDoc());
      assertTrue(pool->anyDocValuesChanges());
    } else {
      assertFalse(pool->anyDocValuesChanges());
    }
    delete readOnlyClone;
    bool writtenToDisk;
    if (pool->isReaderPoolingEnabled()) {
      if (random()->nextBoolean()) {
        writtenToDisk = pool->writeAllDocValuesUpdates();
        assertFalse(readersAndUpdates->isMerging());
      } else if (random()->nextBoolean()) {
        writtenToDisk = pool->commit(segmentInfos);
        assertFalse(readersAndUpdates->isMerging());
      } else {
        writtenToDisk = pool->writeDocValuesUpdatesForMerge(
            Collections::singletonList(commitInfo));
        assertTrue(readersAndUpdates->isMerging());
      }
      assertFalse(pool->release(readersAndUpdates, random()->nextBoolean()));
    } else {
      if (random()->nextBoolean()) {
        writtenToDisk =
            pool->release(readersAndUpdates, random()->nextBoolean());
        assertFalse(readersAndUpdates->isMerging());
      } else {
        writtenToDisk = pool->writeDocValuesUpdatesForMerge(
            Collections::singletonList(commitInfo));
        assertTrue(readersAndUpdates->isMerging());
        assertFalse(pool->release(readersAndUpdates, random()->nextBoolean()));
      }
    }
    assertFalse(pool->anyDocValuesChanges());
    assertEquals(expectUpdate, writtenToDisk);
    if (expectUpdate) {
      readersAndUpdates = pool->get(commitInfo, true);
      shared_ptr<SegmentReader> updatedReader =
          readersAndUpdates->getReadOnlyClone(IOContext::READ);
      assertNotSame(-1, doc);
      shared_ptr<NumericDocValues> number =
          updatedReader->getNumericDocValues(L"number");
      assertEquals(doc, number->advance(doc));
      assertEquals(1000LL, number->longValue());
      readersAndUpdates->release(updatedReader);
      assertFalse(pool->release(readersAndUpdates, random()->nextBoolean()));
    }
  }
  IOUtils::close({pool, reader, directory});
}

void TestReaderPool::testDeletes() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<FieldInfos::FieldNumbers> fieldNumbers = buildIndex(directory);
  shared_ptr<StandardDirectoryReader> reader =
      std::static_pointer_cast<StandardDirectoryReader>(
          DirectoryReader::open(directory));
  shared_ptr<SegmentInfos> segmentInfos = reader->segmentInfos->clone();
  shared_ptr<ReaderPool> pool = make_shared<ReaderPool>(
      directory, directory, segmentInfos, fieldNumbers, [&]() { 0LL; },
      make_shared<NullInfoStream>(), nullptr, nullptr);
  int id = random()->nextInt(10);
  if (random()->nextBoolean()) {
    pool->enableReaderPooling();
  }
  for (auto commitInfo : segmentInfos) {
    shared_ptr<ReadersAndUpdates> readersAndUpdates =
        pool->get(commitInfo, true);
    shared_ptr<SegmentReader> readOnlyClone =
        readersAndUpdates->getReadOnlyClone(IOContext::READ);
    shared_ptr<PostingsEnum> postings =
        readOnlyClone->postings(make_shared<Term>(L"id", L"" + to_wstring(id)));
    bool expectUpdate = false;
    int doc = -1;
    if (postings != nullptr &&
        postings->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
      assertTrue(readersAndUpdates->delete (doc = postings->docID()));
      expectUpdate = true;
      assertEquals(DocIdSetIterator::NO_MORE_DOCS, postings->nextDoc());
    }
    assertFalse(pool->anyDocValuesChanges()); // deletes are not accounted here
    delete readOnlyClone;
    bool writtenToDisk;
    if (pool->isReaderPoolingEnabled()) {
      writtenToDisk = pool->commit(segmentInfos);
      assertFalse(pool->release(readersAndUpdates, random()->nextBoolean()));
    } else {
      writtenToDisk = pool->release(readersAndUpdates, random()->nextBoolean());
    }
    assertFalse(pool->anyDocValuesChanges());
    assertEquals(expectUpdate, writtenToDisk);
    if (expectUpdate) {
      readersAndUpdates = pool->get(commitInfo, true);
      shared_ptr<SegmentReader> updatedReader =
          readersAndUpdates->getReadOnlyClone(IOContext::READ);
      assertNotSame(-1, doc);
      assertFalse(updatedReader->getLiveDocs()->get(doc));
      readersAndUpdates->release(updatedReader);
      assertFalse(pool->release(readersAndUpdates, random()->nextBoolean()));
    }
  }
  IOUtils::close({pool, reader, directory});
}

void TestReaderPool::testPassReaderToMergePolicyConcurrently() throw(
    runtime_error)
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<FieldInfos::FieldNumbers> fieldNumbers = buildIndex(directory);
  shared_ptr<StandardDirectoryReader> reader =
      std::static_pointer_cast<StandardDirectoryReader>(
          DirectoryReader::open(directory));
  shared_ptr<SegmentInfos> segmentInfos = reader->segmentInfos->clone();
  shared_ptr<ReaderPool> pool = make_shared<ReaderPool>(
      directory, directory, segmentInfos, fieldNumbers, [&]() { 0LL; },
      make_shared<NullInfoStream>(), nullptr, nullptr);
  if (random()->nextBoolean()) {
    pool->enableReaderPooling();
  }
  shared_ptr<AtomicBoolean> isDone = make_shared<AtomicBoolean>();
  shared_ptr<CountDownLatch> latch = make_shared<CountDownLatch>(1);
  shared_ptr<Thread> refresher = make_shared<Thread>([&]() {
    try {
      latch->countDown();
      while (isDone->get() == false) {
        for (auto commitInfo : segmentInfos) {
          shared_ptr<ReadersAndUpdates> readersAndUpdates =
              pool->get(commitInfo, true);
          shared_ptr<SegmentReader> segmentReader =
              readersAndUpdates->getReader(IOContext::READ);
          readersAndUpdates->release(segmentReader);
          pool->release(readersAndUpdates, random()->nextBoolean());
        }
      }
    } catch (const runtime_error &ex) {
      throw make_shared<AssertionError>(ex);
    }
  });
  refresher->start();
  shared_ptr<MergePolicy> mergePolicy =
      make_shared<FilterMergePolicyAnonymousInnerClass>(
          shared_from_this(), newMergePolicy(), reader);
  latch->await();
  for (int i = 0; i < reader->maxDoc(); i++) {
    for (auto commitInfo : segmentInfos) {
      shared_ptr<ReadersAndUpdates> readersAndUpdates =
          pool->get(commitInfo, true);
      shared_ptr<SegmentReader> sr =
          readersAndUpdates->getReadOnlyClone(IOContext::READ);
      shared_ptr<PostingsEnum> postings =
          sr->postings(make_shared<Term>(L"id", L"" + to_wstring(i)));
      sr->decRef();
      if (postings != nullptr) {
        for (int docId = postings->nextDoc();
             docId != DocIdSetIterator::NO_MORE_DOCS;
             docId = postings->nextDoc()) {
          readersAndUpdates->delete (docId);
          assertTrue(readersAndUpdates->keepFullyDeletedSegment(mergePolicy));
        }
      }
      assertTrue(readersAndUpdates->keepFullyDeletedSegment(mergePolicy));
      pool->release(readersAndUpdates, random()->nextBoolean());
    }
  }
  isDone->set(true);
  refresher->join();
  IOUtils::close({pool, reader, directory});
}

TestReaderPool::FilterMergePolicyAnonymousInnerClass::
    FilterMergePolicyAnonymousInnerClass(
        shared_ptr<TestReaderPool> outerInstance,
        shared_ptr<org::apache::lucene::index::MergePolicy> newMergePolicy,
        shared_ptr<org::apache::lucene::index::StandardDirectoryReader> reader)
    : FilterMergePolicy(newMergePolicy)
{
  this->outerInstance = outerInstance;
  this->reader = reader;
}

bool TestReaderPool::FilterMergePolicyAnonymousInnerClass::
    keepFullyDeletedSegment(IOSupplier<std::shared_ptr<CodecReader>>
                                readerIOSupplier) 
{
  shared_ptr<CodecReader> reader = readerIOSupplier();
  assert(reader->maxDoc() > 0); // just try to access the reader
  return true;
}

shared_ptr<FieldInfos::FieldNumbers>
TestReaderPool::buildIndex(shared_ptr<Directory> directory) 
{
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(directory, newIndexWriterConfig());
  for (int i = 0; i < 10; i++) {
    shared_ptr<Document> document = make_shared<Document>();
    document->push_back(make_shared<StringField>(L"id", L"" + to_wstring(i),
                                                 Field::Store::YES));
    document->push_back(make_shared<NumericDocValuesField>(L"number", i));
    writer->addDocument(document);
    if (random()->nextBoolean()) {
      writer->flush();
    }
  }
  writer->commit();
  delete writer;
  return writer->globalFieldNumberMap;
}

void TestReaderPool::testGetReaderByRam() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<FieldInfos::FieldNumbers> fieldNumbers = buildIndex(directory);
  shared_ptr<StandardDirectoryReader> reader =
      std::static_pointer_cast<StandardDirectoryReader>(
          DirectoryReader::open(directory));
  shared_ptr<SegmentInfos> segmentInfos = reader->segmentInfos->clone();
  shared_ptr<ReaderPool> pool = make_shared<ReaderPool>(
      directory, directory, segmentInfos, fieldNumbers, [&]() { 0LL; },
      make_shared<NullInfoStream>(), nullptr, nullptr);
  assertEquals(0, pool->getReadersByRam().size());

  int ord = 0;
  for (auto commitInfo : segmentInfos) {
    shared_ptr<ReadersAndUpdates> readersAndUpdates =
        pool->get(commitInfo, true);
    shared_ptr<BinaryDocValuesFieldUpdates> test =
        make_shared<BinaryDocValuesFieldUpdates>(0, L"test",
                                                 commitInfo->info->maxDoc());
    test->add(0, make_shared<BytesRef>(std::deque<char>(ord++)));
    test->finish();
    readersAndUpdates->addDVUpdate(test);
  }

  deque<std::shared_ptr<ReadersAndUpdates>> readersByRam =
      pool->getReadersByRam();
  assertEquals(segmentInfos->size(), readersByRam.size());
  int64_t previousRam = numeric_limits<int64_t>::max();
  for (auto rld : readersByRam) {
    assertTrue(L"previous: " + to_wstring(previousRam) + L" now: " +
                   rld->ramBytesUsed->get(),
               previousRam >= rld->ramBytesUsed->get());
    previousRam = rld->ramBytesUsed->get();
    rld->dropChanges();
    pool->drop(rld->info);
  }
  IOUtils::close({pool, reader, directory});
}
} // namespace org::apache::lucene::index