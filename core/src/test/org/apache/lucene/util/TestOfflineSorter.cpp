using namespace std;

#include "TestOfflineSorter.h"

namespace org::apache::lucene::util
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using CorruptingIndexOutput = org::apache::lucene::store::CorruptingIndexOutput;
using Directory = org::apache::lucene::store::Directory;
using FilterDirectory = org::apache::lucene::store::FilterDirectory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using BufferSize = org::apache::lucene::util::OfflineSorter::BufferSize;
using ByteSequencesWriter =
    org::apache::lucene::util::OfflineSorter::ByteSequencesWriter;
using SortInfo = org::apache::lucene::util::OfflineSorter::SortInfo;

void TestOfflineSorter::setUp() 
{
  LuceneTestCase::setUp();
  tempDir = createTempDir(L"mergesort");
}

void TestOfflineSorter::tearDown() 
{
  if (tempDir != nullptr) {
    IOUtils::rm({tempDir});
  }
  LuceneTestCase::tearDown();
}

void TestOfflineSorter::testEmpty() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory())
  {
    org::apache::lucene::store::Directory dir = newDirectory();
    checkSort(dir, make_shared<OfflineSorter>(dir, L"foo"),
              std::deque<std::deque<char>>());
  }
}

void TestOfflineSorter::testSingleLine() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory())
  {
    org::apache::lucene::store::Directory dir = newDirectory();
    checkSort(
        dir, make_shared<OfflineSorter>(dir, L"foo"),
        std::deque<std::deque<char>>{
            (wstring(L"Single line only.")).getBytes(StandardCharsets::UTF_8)});
  }
}

shared_ptr<ExecutorService> TestOfflineSorter::randomExecutorServiceOrNull()
{
  if (random()->nextBoolean()) {
    return nullptr;
  } else {
    return make_shared<ThreadPoolExecutor>(
        1, TestUtil::nextInt(random(), 2, 6), numeric_limits<int64_t>::max(),
        TimeUnit::MILLISECONDS, make_shared<LinkedBlockingQueue<Runnable>>(),
        make_shared<NamedThreadFactory>(L"TestIndexSearcher"));
  }
}

void TestOfflineSorter::testIntermediateMerges() 
{
  // Sort 20 mb worth of data with 1mb buffer, binary merging.
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory())
  {
    org::apache::lucene::store::Directory dir = newDirectory();
    shared_ptr<ExecutorService> exec = randomExecutorServiceOrNull();
    shared_ptr<SortInfo> info =
        checkSort(dir,
                  make_shared<OfflineSorter>(
                      dir, L"foo", OfflineSorter::DEFAULT_COMPARATOR,
                      BufferSize::megabytes(1), 2, -1, exec,
                      TestUtil::nextInt(random(), 1, 4)),
                  generateRandom(static_cast<int>(OfflineSorter::MB) * 20));
    if (exec != nullptr) {
      exec->shutdownNow();
    }
    assertTrue(info->mergeRounds > 10);
  }
}

void TestOfflineSorter::testSmallRandom() 
{
  // Sort 20 mb worth of data with 1mb buffer.
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory())
  {
    org::apache::lucene::store::Directory dir = newDirectory();
    shared_ptr<ExecutorService> exec = randomExecutorServiceOrNull();
    shared_ptr<SortInfo> sortInfo =
        checkSort(dir,
                  make_shared<OfflineSorter>(
                      dir, L"foo", OfflineSorter::DEFAULT_COMPARATOR,
                      BufferSize::megabytes(1), OfflineSorter::MAX_TEMPFILES,
                      -1, exec, TestUtil::nextInt(random(), 1, 4)),
                  generateRandom(static_cast<int>(OfflineSorter::MB) * 20));
    if (exec != nullptr) {
      exec->shutdownNow();
    }
    assertEquals(3, sortInfo->mergeRounds);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testLargerRandom() throws Exception
void TestOfflineSorter::testLargerRandom() 
{
  // Sort 100MB worth of data with 15mb buffer.
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newFSDirectory(createTempDir()))
  {
    org::apache::lucene::store::Directory dir = newFSDirectory(createTempDir());
    shared_ptr<ExecutorService> exec = randomExecutorServiceOrNull();
    checkSort(dir,
              make_shared<OfflineSorter>(
                  dir, L"foo", OfflineSorter::DEFAULT_COMPARATOR,
                  BufferSize::megabytes(16), OfflineSorter::MAX_TEMPFILES, -1,
                  exec, TestUtil::nextInt(random(), 1, 4)),
              generateRandom(static_cast<int>(OfflineSorter::MB) * 100));
    if (exec != nullptr) {
      exec->shutdownNow();
    }
  }
}

std::deque<std::deque<char>>
TestOfflineSorter::generateRandom(int howMuchDataInBytes)
{
  deque<std::deque<char>> data = deque<std::deque<char>>();
  while (howMuchDataInBytes > 0) {
    std::deque<char> current(random()->nextInt(256));
    random()->nextBytes(current);
    data.push_back(current);
    howMuchDataInBytes -= current.size();
  }
  std::deque<std::deque<char>> bytes =
      data.toArray(std::deque<std::deque<char>>(data.size()));
  return bytes;
}

std::deque<std::deque<char>>
TestOfflineSorter::generateFixed(int howMuchDataInBytes)
{
  deque<std::deque<char>> data = deque<std::deque<char>>();
  int length = 256;
  char counter = 0;
  while (howMuchDataInBytes > 0) {
    std::deque<char> current(length);
    for (int i = 0; i < current.size(); i++) {
      current[i] = counter;
      counter++;
    }
    data.push_back(current);
    howMuchDataInBytes -= current.size();

    length--;
    if (length <= 128) {
      length = 256;
    }
  }
  std::deque<std::deque<char>> bytes =
      data.toArray(std::deque<std::deque<char>>(data.size()));
  return bytes;
}

const shared_ptr<java::util::Comparator<std::deque<char>>>
    TestOfflineSorter::unsignedByteOrderComparator =
        make_shared<ComparatorAnonymousInnerClass>();

TestOfflineSorter::ComparatorAnonymousInnerClass::
    ComparatorAnonymousInnerClass()
{
}

int TestOfflineSorter::ComparatorAnonymousInnerClass::compare(
    std::deque<char> &left, std::deque<char> &right)
{
  constexpr int max = min(left.size(), right.size());
  for (int i = 0, j = 0; i < max; i++, j++) {
    int diff = (left[i] & 0xff) - (right[j] & 0xff);
    if (diff != 0) {
      return diff;
    }
  }
  return left.size() - right.size();
}

shared_ptr<SortInfo> TestOfflineSorter::checkSort(
    shared_ptr<Directory> dir, shared_ptr<OfflineSorter> sorter,
    std::deque<std::deque<char>> &data) 
{

  shared_ptr<IndexOutput> unsorted =
      dir->createTempOutput(L"unsorted", L"tmp", IOContext::DEFAULT);
  writeAll(unsorted, data);

  shared_ptr<IndexOutput> golden =
      dir->createTempOutput(L"golden", L"tmp", IOContext::DEFAULT);
  Arrays::sort(data, unsignedByteOrderComparator);
  writeAll(golden, data);

  wstring sorted = sorter->sort(unsorted->getName());
  // System.out.println("Input size [MB]: " + unsorted.length() / (1024 *
  // 1024)); System.out.println(sortInfo);
  assertFilesIdentical(dir, golden->getName(), sorted);

  return sorter->sortInfo;
}

void TestOfflineSorter::assertFilesIdentical(
    shared_ptr<Directory> dir, const wstring &golden,
    const wstring &sorted) 
{
  int64_t numBytes = dir->fileLength(golden);
  assertEquals(numBytes, dir->fileLength(sorted));

  std::deque<char> buf1(64 * 1024);
  std::deque<char> buf2(64 * 1024);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexInput in1 =
  // dir.openInput(golden, org.apache.lucene.store.IOContext.READONCE);
  // org.apache.lucene.store.IndexInput in2 = dir.openInput(sorted,
  // org.apache.lucene.store.IOContext.READONCE))
  {
    org::apache::lucene::store::IndexInput in1 =
        dir->openInput(golden, org::apache::lucene::store::IOContext::READONCE);
    org::apache::lucene::store::IndexInput in2 =
        dir->openInput(sorted, org::apache::lucene::store::IOContext::READONCE);
    int64_t left = numBytes;
    while (left > 0) {
      int chunk = static_cast<int>(min(buf1.size(), left));
      left -= chunk;
      in1->readBytes(buf1, 0, chunk);
      in2->readBytes(buf2, 0, chunk);
      for (int i = 0; i < chunk; i++) {
        assertEquals(buf1[i], buf2[i]);
      }
    }
  }
}

void TestOfflineSorter::writeAll(
    shared_ptr<IndexOutput> out,
    std::deque<std::deque<char>> &data) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try
  // (org.apache.lucene.util.OfflineSorter.ByteSequencesWriter w = new
  // OfflineSorter.ByteSequencesWriter(out))
  {
    org::apache::lucene::util::OfflineSorter::ByteSequencesWriter w =
        OfflineSorter::ByteSequencesWriter(out);
    for (auto datum : data) {
      w->write(datum);
    }
    CodecUtil::writeFooter(out);
  }
}

void TestOfflineSorter::testRamBuffer()
{
  int numIters = atLeast(10000);
  for (int i = 0; i < numIters; i++) {
    BufferSize::megabytes(1 + random()->nextInt(2047));
  }
  BufferSize::megabytes(2047);
  BufferSize::megabytes(1);

  expectThrows(invalid_argument::typeid,
               [&]() { BufferSize::megabytes(2048); });

  expectThrows(invalid_argument::typeid, [&]() { BufferSize::megabytes(0); });

  expectThrows(invalid_argument::typeid, [&]() { BufferSize::megabytes(-1); });
}

void TestOfflineSorter::testThreadSafety() 
{
  std::deque<std::shared_ptr<Thread>> threads(
      TestUtil::nextInt(random(), 4, 10));
  shared_ptr<AtomicBoolean> *const failed = make_shared<AtomicBoolean>();
  constexpr int iters = atLeast(1000);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory())
  {
    org::apache::lucene::store::Directory dir = newDirectory();
    for (int i = 0; i < threads.size(); i++) {
      constexpr int threadID = i;
      threads[i] = make_shared<ThreadAnonymousInnerClass>(
          shared_from_this(), failed, iters, dir, threadID);
      threads[i]->start();
    }
    for (auto thread : threads) {
      thread->join();
    }
  }

  assertFalse(failed->get());
}

TestOfflineSorter::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestOfflineSorter> outerInstance,
    shared_ptr<AtomicBoolean> failed, int iters, shared_ptr<Directory> dir,
    int threadID)
{
  this->outerInstance = outerInstance;
  this->failed = failed;
  this->iters = iters;
  this->dir = dir;
  this->threadID = threadID;
}

void TestOfflineSorter::ThreadAnonymousInnerClass::run()
{
  try {
    for (int iter = 0; iter < iters && failed->get() == false; iter++) {
      outerInstance->checkSort(
          dir,
          make_shared<OfflineSorter>(dir, L"foo_" + to_wstring(threadID) +
                                              L"_" + to_wstring(iter)),
          outerInstance->generateRandom(1024));
    }
  } catch (const runtime_error &th) {
    failed->set(true);
    throw runtime_error(th);
  }
}

void TestOfflineSorter::testBitFlippedOnInput1() 
{

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir0 =
  // newMockDirectory())
  {
    org::apache::lucene::store::Directory dir0 = newMockDirectory();

    shared_ptr<Directory> dir = make_shared<FilterDirectoryAnonymousInnerClass>(
        shared_from_this(), dir0);

    shared_ptr<IndexOutput> unsorted =
        dir->createTempOutput(L"unsorted", L"tmp", IOContext::DEFAULT);
    writeAll(unsorted, generateFixed(10 * 1024));

    shared_ptr<CorruptIndexException> e =
        expectThrows(CorruptIndexException::typeid, [&]() {
          (make_shared<OfflineSorter>(dir, L"foo"))->sort(unsorted->getName());
        });
    assertTrue(
        e->getMessage()->contains(L"checksum failed (hardware problem?)"));
  }
}

TestOfflineSorter::FilterDirectoryAnonymousInnerClass::
    FilterDirectoryAnonymousInnerClass(
        shared_ptr<TestOfflineSorter> outerInstance, shared_ptr<Directory> dir0)
    : org::apache::lucene::store::FilterDirectory(dir0)
{
  this->outerInstance = outerInstance;
  this->dir0 = dir0;
}

shared_ptr<IndexOutput>
TestOfflineSorter::FilterDirectoryAnonymousInnerClass::createTempOutput(
    const wstring &prefix, const wstring &suffix,
    shared_ptr<IOContext> context) 
{
  shared_ptr<IndexOutput> out = in_::createTempOutput(prefix, suffix, context);
  if (prefix == L"unsorted") {
    return make_shared<CorruptingIndexOutput>(dir0, 22, out);
  } else {
    return out;
  }
}

void TestOfflineSorter::testBitFlippedOnInput2() 
{

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir0 =
  // newMockDirectory())
  {
    org::apache::lucene::store::Directory dir0 = newMockDirectory();

    shared_ptr<Directory> dir =
        make_shared<FilterDirectoryAnonymousInnerClass2>(shared_from_this(),
                                                         dir0);

    shared_ptr<IndexOutput> unsorted =
        dir->createTempOutput(L"unsorted", L"tmp", IOContext::DEFAULT);
    writeAll(unsorted, generateFixed(5 * 1024));

    // This corruption made OfflineSorter fail with its own exception, but we
    // verify it also went and added (as suppressed) that the checksum was
    // wrong:
    shared_ptr<EOFException> e = expectThrows(EOFException::typeid, [&]() {
      (make_shared<OfflineSorter>(dir, L"foo"))->sort(unsorted->getName());
    });
    assertEquals(1, e->getSuppressed()->length);
    assertTrue(std::dynamic_pointer_cast<CorruptIndexException>(
                   e->getSuppressed()[0]) != nullptr);
    assertTrue(e->getSuppressed()[0].getMessage()->contains(
        L"checksum failed (hardware problem?)"));
  }
}

TestOfflineSorter::FilterDirectoryAnonymousInnerClass2::
    FilterDirectoryAnonymousInnerClass2(
        shared_ptr<TestOfflineSorter> outerInstance, shared_ptr<Directory> dir0)
    : org::apache::lucene::store::FilterDirectory(dir0)
{
  this->outerInstance = outerInstance;
  this->dir0 = dir0;
}

shared_ptr<IndexOutput>
TestOfflineSorter::FilterDirectoryAnonymousInnerClass2::createTempOutput(
    const wstring &prefix, const wstring &suffix,
    shared_ptr<IOContext> context) 
{
  shared_ptr<IndexOutput> out = in_::createTempOutput(prefix, suffix, context);
  if (prefix == L"unsorted") {
    return make_shared<CorruptingIndexOutputAnonymousInnerClass>(
        shared_from_this(), dir0, out);
  } else {
    return out;
  }
}

TestOfflineSorter::FilterDirectoryAnonymousInnerClass2::
    CorruptingIndexOutputAnonymousInnerClass::
        CorruptingIndexOutputAnonymousInnerClass(
            shared_ptr<FilterDirectoryAnonymousInnerClass2> outerInstance,
            shared_ptr<Directory> dir0, shared_ptr<IndexOutput> out)
    : org::apache::lucene::store::CorruptingIndexOutput(dir0, 22, out)
{
  this->outerInstance = outerInstance;
  this->out = out;
}

void TestOfflineSorter::FilterDirectoryAnonymousInnerClass2::
    CorruptingIndexOutputAnonymousInnerClass::corruptFile() 
{
  wstring newTempName;
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try(org.apache.lucene.store.IndexOutput tmpOut =
  // dir0.createTempOutput("tmp", "tmp",
  // org.apache.lucene.store.IOContext.DEFAULT);
  // org.apache.lucene.store.IndexInput in = dir0.openInput(out.getName(),
  // org.apache.lucene.store.IOContext.DEFAULT))
  {
    org::apache::lucene::store::IndexOutput tmpOut =
        outerInstance->dir0.createTempOutput(
            L"tmp", L"tmp", org::apache::lucene::store::IOContext::DEFAULT);
    org::apache::lucene::store::IndexInput in_ = outerInstance->dir0.openInput(
        out->getName(), org::apache::lucene::store::IOContext::DEFAULT);
    newTempName = tmpOut->getName();
    // Replace length at the end with a too-long value:
    short v = in_->readShort();
    assertEquals(256, v);
    tmpOut->writeShort(numeric_limits<short>::max());
    tmpOut->copyBytes(in_, in_->length() - Short::BYTES);
  }

  // Delete original and copy corrupt version back:
  outerInstance->dir0.deleteFile(out->getName());
  outerInstance->dir0.copyFrom(outerInstance->dir0, newTempName, out->getName(),
                               IOContext::DEFAULT);
  outerInstance->dir0.deleteFile(newTempName);
}

void TestOfflineSorter::testBitFlippedOnPartition1() 
{

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir0 =
  // newMockDirectory())
  {
    org::apache::lucene::store::Directory dir0 = newMockDirectory();

    shared_ptr<Directory> dir =
        make_shared<FilterDirectoryAnonymousInnerClass3>(shared_from_this(),
                                                         dir0);

    shared_ptr<IndexOutput> unsorted =
        dir->createTempOutput(L"unsorted", L"tmp", IOContext::DEFAULT);
    writeAll(unsorted, generateFixed(static_cast<int>(OfflineSorter::MB * 3)));

    shared_ptr<CorruptIndexException> e =
        expectThrows(CorruptIndexException::typeid, [&]() {
          (make_shared<OfflineSorter>(
               dir, L"foo", OfflineSorter::DEFAULT_COMPARATOR,
               BufferSize::megabytes(1), 10, -1, nullptr, 0))
              ->sort(unsorted->getName());
        });
    assertTrue(
        e->getMessage()->contains(L"checksum failed (hardware problem?)"));
  }
}

TestOfflineSorter::FilterDirectoryAnonymousInnerClass3::
    FilterDirectoryAnonymousInnerClass3(
        shared_ptr<TestOfflineSorter> outerInstance, shared_ptr<Directory> dir0)
    : org::apache::lucene::store::FilterDirectory(dir0)
{
  this->outerInstance = outerInstance;
  this->dir0 = dir0;
}

shared_ptr<IndexOutput>
TestOfflineSorter::FilterDirectoryAnonymousInnerClass3::createTempOutput(
    const wstring &prefix, const wstring &suffix,
    shared_ptr<IOContext> context) 
{
  shared_ptr<IndexOutput> out = in_::createTempOutput(prefix, suffix, context);
  if (corrupted == false && suffix == L"sort") {
    corrupted = true;
    return make_shared<CorruptingIndexOutput>(dir0, 544677, out);
  } else {
    return out;
  }
}

void TestOfflineSorter::testBitFlippedOnPartition2() 
{

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir0 =
  // newMockDirectory())
  {
    org::apache::lucene::store::Directory dir0 = newMockDirectory();

    shared_ptr<Directory> dir =
        make_shared<FilterDirectoryAnonymousInnerClass4>(shared_from_this(),
                                                         dir0);

    shared_ptr<IndexOutput> unsorted =
        dir->createTempOutput(L"unsorted", L"tmp", IOContext::DEFAULT);
    writeAll(unsorted, generateFixed(static_cast<int>(OfflineSorter::MB * 3)));

    shared_ptr<EOFException> e = expectThrows(EOFException::typeid, [&]() {
      (make_shared<OfflineSorter>(dir, L"foo",
                                  OfflineSorter::DEFAULT_COMPARATOR,
                                  BufferSize::megabytes(1), 10, -1, nullptr, 0))
          ->sort(unsorted->getName());
    });
    assertEquals(1, e->getSuppressed()->length);
    assertTrue(std::dynamic_pointer_cast<CorruptIndexException>(
                   e->getSuppressed()[0]) != nullptr);
    assertTrue(e->getSuppressed()[0].getMessage()->contains(
        L"checksum failed (hardware problem?)"));
  }
}

TestOfflineSorter::FilterDirectoryAnonymousInnerClass4::
    FilterDirectoryAnonymousInnerClass4(
        shared_ptr<TestOfflineSorter> outerInstance, shared_ptr<Directory> dir0)
    : org::apache::lucene::store::FilterDirectory(dir0)
{
  this->outerInstance = outerInstance;
  this->dir0 = dir0;
}

shared_ptr<IndexOutput>
TestOfflineSorter::FilterDirectoryAnonymousInnerClass4::createTempOutput(
    const wstring &prefix, const wstring &suffix,
    shared_ptr<IOContext> context) 
{
  shared_ptr<IndexOutput> out = in_::createTempOutput(prefix, suffix, context);
  if (corrupted == false && suffix == L"sort") {
    corrupted = true;
    return make_shared<CorruptingIndexOutputAnonymousInnerClass2>(
        shared_from_this(), dir0, out);
  } else {
    return out;
  }
}

TestOfflineSorter::FilterDirectoryAnonymousInnerClass4::
    CorruptingIndexOutputAnonymousInnerClass2::
        CorruptingIndexOutputAnonymousInnerClass2(
            shared_ptr<FilterDirectoryAnonymousInnerClass4> outerInstance,
            shared_ptr<Directory> dir0, shared_ptr<IndexOutput> out)
    : org::apache::lucene::store::CorruptingIndexOutput(dir0, 544677, out)
{
  this->outerInstance = outerInstance;
  this->out = out;
}

void TestOfflineSorter::FilterDirectoryAnonymousInnerClass4::
    CorruptingIndexOutputAnonymousInnerClass2::corruptFile() 
{
  wstring newTempName;
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try(org.apache.lucene.store.IndexOutput tmpOut =
  // dir0.createTempOutput("tmp", "tmp",
  // org.apache.lucene.store.IOContext.DEFAULT);
  // org.apache.lucene.store.IndexInput in = dir0.openInput(out.getName(),
  // org.apache.lucene.store.IOContext.DEFAULT))
  {
    org::apache::lucene::store::IndexOutput tmpOut =
        outerInstance->dir0.createTempOutput(
            L"tmp", L"tmp", org::apache::lucene::store::IOContext::DEFAULT);
    org::apache::lucene::store::IndexInput in_ = outerInstance->dir0.openInput(
        out->getName(), org::apache::lucene::store::IOContext::DEFAULT);
    newTempName = tmpOut->getName();
    tmpOut->copyBytes(in_, 1025905);
    short v = in_->readShort();
    assertEquals(254, v);
    tmpOut->writeShort(numeric_limits<short>::max());
    tmpOut->copyBytes(in_, in_->length() - 1025905 - Short::BYTES);
  }

  // Delete original and copy corrupt version back:
  outerInstance->dir0.deleteFile(out->getName());
  outerInstance->dir0.copyFrom(outerInstance->dir0, newTempName, out->getName(),
                               IOContext::DEFAULT);
  outerInstance->dir0.deleteFile(newTempName);
}

void TestOfflineSorter::testFixedLengthHeap() 
{
  // Make sure the RAM accounting is correct, i.e. if we are sorting fixed width
  // ints (4 bytes) then the heap used is really only 4 bytes per value:
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexOutput> out =
      dir->createTempOutput(L"unsorted", L"tmp", IOContext::DEFAULT);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try
  // (org.apache.lucene.util.OfflineSorter.ByteSequencesWriter w = new
  // OfflineSorter.ByteSequencesWriter(out))
  {
    org::apache::lucene::util::OfflineSorter::ByteSequencesWriter w =
        OfflineSorter::ByteSequencesWriter(out);
    std::deque<char> bytes(Integer::BYTES);
    for (int i = 0; i < 1024 * 1024; i++) {
      random()->nextBytes(bytes);
      w->write(bytes);
    }
    CodecUtil::writeFooter(out);
  }

  shared_ptr<ExecutorService> exec = randomExecutorServiceOrNull();
  shared_ptr<OfflineSorter> sorter = make_shared<OfflineSorter>(
      dir, L"foo", OfflineSorter::DEFAULT_COMPARATOR, BufferSize::megabytes(4),
      OfflineSorter::MAX_TEMPFILES, Integer::BYTES, exec,
      TestUtil::nextInt(random(), 1, 4));
  sorter->sort(out->getName());
  if (exec != nullptr) {
    exec->shutdownNow();
  }
  // 1 MB of ints with 4 MH heap allowed should have been sorted in a single
  // heap partition:
  assertEquals(0, sorter->sortInfo->mergeRounds);
  delete dir;
}

void TestOfflineSorter::testFixedLengthLiesLiesLies() 
{
  // Make sure OfflineSorter catches me if I lie about the fixed value length:
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexOutput> out =
      dir->createTempOutput(L"unsorted", L"tmp", IOContext::DEFAULT);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try
  // (org.apache.lucene.util.OfflineSorter.ByteSequencesWriter w = new
  // OfflineSorter.ByteSequencesWriter(out))
  {
    org::apache::lucene::util::OfflineSorter::ByteSequencesWriter w =
        OfflineSorter::ByteSequencesWriter(out);
    std::deque<char> bytes(Integer::BYTES);
    random()->nextBytes(bytes);
    w->write(bytes);
    CodecUtil::writeFooter(out);
  }

  shared_ptr<OfflineSorter> sorter = make_shared<OfflineSorter>(
      dir, L"foo", OfflineSorter::DEFAULT_COMPARATOR, BufferSize::megabytes(4),
      OfflineSorter::MAX_TEMPFILES, Long::BYTES, nullptr, 0);
  invalid_argument e = expectThrows(invalid_argument::typeid,
                                    [&]() { sorter->sort(out->getName()); });
  assertEquals(L"value length is 4 but is supposed to always be 8", e.what());
  delete dir;
}

void TestOfflineSorter::testOverNexting() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexOutput> out =
      dir->createTempOutput(L"unsorted", L"tmp", IOContext::DEFAULT);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try
  // (org.apache.lucene.util.OfflineSorter.ByteSequencesWriter w = new
  // OfflineSorter.ByteSequencesWriter(out))
  {
    org::apache::lucene::util::OfflineSorter::ByteSequencesWriter w =
        OfflineSorter::ByteSequencesWriter(out);
    std::deque<char> bytes(Integer::BYTES);
    random()->nextBytes(bytes);
    w->write(bytes);
    CodecUtil::writeFooter(out);
  }

  make_shared<OfflineSorterAnonymousInnerClass>(
      shared_from_this(), dir, OfflineSorter::DEFAULT_COMPARATOR,
      BufferSize::megabytes(4), Integer::BYTES)
      .sort(out->getName());
  delete dir;
}

TestOfflineSorter::OfflineSorterAnonymousInnerClass::
    OfflineSorterAnonymousInnerClass(
        shared_ptr<TestOfflineSorter> outerInstance, shared_ptr<Directory> dir,
        shared_ptr<Comparator<std::shared_ptr<BytesRef>>> DEFAULT_COMPARATOR,
        shared_ptr<BufferSize> megabytes, shared_ptr<UnknownType> BYTES)
    : OfflineSorter(dir, L"foo", DEFAULT_COMPARATOR, megabytes,
                    OfflineSorter::MAX_TEMPFILES, BYTES, nullptr, 0)
{
  this->outerInstance = outerInstance;
}

shared_ptr<ByteSequencesReader>
TestOfflineSorter::OfflineSorterAnonymousInnerClass::getReader(
    shared_ptr<ChecksumIndexInput> in_, const wstring &name) 
{
  shared_ptr<ByteSequencesReader> other =
      outerInstance->super->getReader(in_, name);

  return make_shared<ByteSequencesReaderAnonymousInnerClass>(shared_from_this(),
                                                             in_, name, other);
}

TestOfflineSorter::OfflineSorterAnonymousInnerClass::
    ByteSequencesReaderAnonymousInnerClass::
        ByteSequencesReaderAnonymousInnerClass(
            shared_ptr<OfflineSorterAnonymousInnerClass> outerInstance,
            shared_ptr<ChecksumIndexInput> in_, const wstring &name,
            shared_ptr<
                org::apache::lucene::util::OfflineSorter::ByteSequencesReader>
                other)
    : ByteSequencesReader(in_, name)
{
  this->outerInstance = outerInstance;
  this->other = other;
}

shared_ptr<BytesRef> TestOfflineSorter::OfflineSorterAnonymousInnerClass::
    ByteSequencesReaderAnonymousInnerClass::next() 
{
  // if we returned null already, OfflineSorter should not call next() again
  assertFalse(alreadyEnded);
  shared_ptr<BytesRef> result = other->next();
  if (result == nullptr) {
    alreadyEnded = true;
  }
  return result;
}

TestOfflineSorter::OfflineSorterAnonymousInnerClass::
    ByteSequencesReaderAnonymousInnerClass::
        ~ByteSequencesReaderAnonymousInnerClass()
{
  delete other;
}

void TestOfflineSorter::testInvalidFixedLength() 
{
  invalid_argument e;
  e = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<OfflineSorter>(
        nullptr, L"foo", OfflineSorter::DEFAULT_COMPARATOR,
        BufferSize::megabytes(1), OfflineSorter::MAX_TEMPFILES, 0, nullptr, 0);
  });
  assertEquals(L"valueLength must be 1 .. 32767; got: 0", e.what());
  e = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<OfflineSorter>(
        nullptr, L"foo", OfflineSorter::DEFAULT_COMPARATOR,
        BufferSize::megabytes(1), OfflineSorter::MAX_TEMPFILES,
        numeric_limits<int>::max(), nullptr, 0);
  });
  assertEquals(L"valueLength must be 1 .. 32767; got: 2147483647", e.what());
}
} // namespace org::apache::lucene::util