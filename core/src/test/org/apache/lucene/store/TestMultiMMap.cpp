using namespace std;

#include "TestMultiMMap.h"

namespace org::apache::lucene::store
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using BytesRef = org::apache::lucene::util::BytesRef;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<Directory>
TestMultiMMap::getDirectory(shared_ptr<Path> path) 
{
  return make_shared<MMapDirectory>(path,
                                    1 << TestUtil::nextInt(random(), 10, 28));
}

void TestMultiMMap::setUp() 
{
  BaseDirectoryTestCase::setUp();
  assumeTrue(MMapDirectory::UNMAP_NOT_SUPPORTED_REASON,
             MMapDirectory::UNMAP_SUPPORTED);
}

void TestMultiMMap::testCloneSafety() 
{
  shared_ptr<MMapDirectory> mmapDir =
      make_shared<MMapDirectory>(createTempDir(L"testCloneSafety"));
  shared_ptr<IndexOutput> io =
      mmapDir->createOutput(L"bytes", newIOContext(random()));
  io->writeVInt(5);
  delete io;
  shared_ptr<IndexInput> one = mmapDir->openInput(L"bytes", IOContext::DEFAULT);
  shared_ptr<IndexInput> two = one->clone();
  shared_ptr<IndexInput> three = two->clone(); // clone of clone
  delete one;
  expectThrows(AlreadyClosedException::typeid, [&]() { one->readVInt(); });
  expectThrows(AlreadyClosedException::typeid, [&]() { two->readVInt(); });
  expectThrows(AlreadyClosedException::typeid, [&]() { three->readVInt(); });

  delete two;
  delete three;
  // test double close of master:
  delete one;
  delete mmapDir;
}

void TestMultiMMap::testCloneClose() 
{
  shared_ptr<MMapDirectory> mmapDir =
      make_shared<MMapDirectory>(createTempDir(L"testCloneClose"));
  shared_ptr<IndexOutput> io =
      mmapDir->createOutput(L"bytes", newIOContext(random()));
  io->writeVInt(5);
  delete io;
  shared_ptr<IndexInput> one = mmapDir->openInput(L"bytes", IOContext::DEFAULT);
  shared_ptr<IndexInput> two = one->clone();
  shared_ptr<IndexInput> three = two->clone(); // clone of clone
  delete two;
  TestUtil::assertEquals(5, one->readVInt());
  expectThrows(AlreadyClosedException::typeid, [&]() { two->readVInt(); });
  TestUtil::assertEquals(5, three->readVInt());
  delete one;
  delete three;
  delete mmapDir;
}

void TestMultiMMap::testCloneSliceSafety() 
{
  shared_ptr<MMapDirectory> mmapDir =
      make_shared<MMapDirectory>(createTempDir(L"testCloneSliceSafety"));
  shared_ptr<IndexOutput> io =
      mmapDir->createOutput(L"bytes", newIOContext(random()));
  io->writeInt(1);
  io->writeInt(2);
  delete io;
  shared_ptr<IndexInput> slicer =
      mmapDir->openInput(L"bytes", newIOContext(random()));
  shared_ptr<IndexInput> one = slicer->slice(L"first int", 0, 4);
  shared_ptr<IndexInput> two = slicer->slice(L"second int", 4, 4);
  shared_ptr<IndexInput> three = one->clone(); // clone of clone
  shared_ptr<IndexInput> four = two->clone();  // clone of clone
  delete slicer;
  expectThrows(AlreadyClosedException::typeid, [&]() { one->readInt(); });
  expectThrows(AlreadyClosedException::typeid, [&]() { two->readInt(); });
  expectThrows(AlreadyClosedException::typeid, [&]() { three->readInt(); });
  expectThrows(AlreadyClosedException::typeid, [&]() { four->readInt(); });

  delete one;
  delete two;
  delete three;
  delete four;
  // test double-close of slicer:
  delete slicer;
  delete mmapDir;
}

void TestMultiMMap::testCloneSliceClose() 
{
  shared_ptr<MMapDirectory> mmapDir =
      make_shared<MMapDirectory>(createTempDir(L"testCloneSliceClose"));
  shared_ptr<IndexOutput> io =
      mmapDir->createOutput(L"bytes", newIOContext(random()));
  io->writeInt(1);
  io->writeInt(2);
  delete io;
  shared_ptr<IndexInput> slicer =
      mmapDir->openInput(L"bytes", newIOContext(random()));
  shared_ptr<IndexInput> one = slicer->slice(L"first int", 0, 4);
  shared_ptr<IndexInput> two = slicer->slice(L"second int", 4, 4);
  delete one;
  expectThrows(AlreadyClosedException::typeid, [&]() { one->readInt(); });
  TestUtil::assertEquals(2, two->readInt());
  // reopen a new slice "another":
  shared_ptr<IndexInput> another = slicer->slice(L"first int", 0, 4);
  TestUtil::assertEquals(1, another->readInt());
  delete another;
  delete two;
  delete slicer;
  delete mmapDir;
}

void TestMultiMMap::testSeekZero() 
{
  for (int i = 0; i < 31; i++) {
    shared_ptr<MMapDirectory> mmapDir =
        make_shared<MMapDirectory>(createTempDir(L"testSeekZero"), 1 << i);
    shared_ptr<IndexOutput> io =
        mmapDir->createOutput(L"zeroBytes", newIOContext(random()));
    delete io;
    shared_ptr<IndexInput> ii =
        mmapDir->openInput(L"zeroBytes", newIOContext(random()));
    ii->seek(0LL);
    delete ii;
    delete mmapDir;
  }
}

void TestMultiMMap::testSeekSliceZero() 
{
  for (int i = 0; i < 31; i++) {
    shared_ptr<MMapDirectory> mmapDir =
        make_shared<MMapDirectory>(createTempDir(L"testSeekSliceZero"), 1 << i);
    shared_ptr<IndexOutput> io =
        mmapDir->createOutput(L"zeroBytes", newIOContext(random()));
    delete io;
    shared_ptr<IndexInput> slicer =
        mmapDir->openInput(L"zeroBytes", newIOContext(random()));
    shared_ptr<IndexInput> ii = slicer->slice(L"zero-length slice", 0, 0);
    ii->seek(0LL);
    delete ii;
    delete slicer;
    delete mmapDir;
  }
}

void TestMultiMMap::testSeekEnd() 
{
  for (int i = 0; i < 17; i++) {
    shared_ptr<MMapDirectory> mmapDir =
        make_shared<MMapDirectory>(createTempDir(L"testSeekEnd"), 1 << i);
    shared_ptr<IndexOutput> io =
        mmapDir->createOutput(L"bytes", newIOContext(random()));
    std::deque<char> bytes(1 << i);
    random()->nextBytes(bytes);
    io->writeBytes(bytes, bytes.size());
    delete io;
    shared_ptr<IndexInput> ii =
        mmapDir->openInput(L"bytes", newIOContext(random()));
    std::deque<char> actual(1 << i);
    ii->readBytes(actual, 0, actual.size());
    TestUtil::assertEquals(make_shared<BytesRef>(bytes),
                           make_shared<BytesRef>(actual));
    ii->seek(1 << i);
    delete ii;
    delete mmapDir;
  }
}

void TestMultiMMap::testSeekSliceEnd() 
{
  for (int i = 0; i < 17; i++) {
    shared_ptr<MMapDirectory> mmapDir =
        make_shared<MMapDirectory>(createTempDir(L"testSeekSliceEnd"), 1 << i);
    shared_ptr<IndexOutput> io =
        mmapDir->createOutput(L"bytes", newIOContext(random()));
    std::deque<char> bytes(1 << i);
    random()->nextBytes(bytes);
    io->writeBytes(bytes, bytes.size());
    delete io;
    shared_ptr<IndexInput> slicer =
        mmapDir->openInput(L"bytes", newIOContext(random()));
    shared_ptr<IndexInput> ii = slicer->slice(L"full slice", 0, bytes.size());
    std::deque<char> actual(1 << i);
    ii->readBytes(actual, 0, actual.size());
    TestUtil::assertEquals(make_shared<BytesRef>(bytes),
                           make_shared<BytesRef>(actual));
    ii->seek(1 << i);
    delete ii;
    delete slicer;
    delete mmapDir;
  }
}

void TestMultiMMap::testSeeking() 
{
  int numIters = TEST_NIGHTLY ? 10 : 1;
  for (int i = 0; i < numIters; i++) {
    shared_ptr<MMapDirectory> mmapDir =
        make_shared<MMapDirectory>(createTempDir(L"testSeeking"), 1 << i);
    shared_ptr<IndexOutput> io =
        mmapDir->createOutput(L"bytes", newIOContext(random()));
    std::deque<char> bytes(1 << (i + 1)); // make sure we switch buffers
    random()->nextBytes(bytes);
    io->writeBytes(bytes, bytes.size());
    delete io;
    shared_ptr<IndexInput> ii =
        mmapDir->openInput(L"bytes", newIOContext(random()));
    std::deque<char> actual(1 << (i + 1)); // first read all bytes
    ii->readBytes(actual, 0, actual.size());
    TestUtil::assertEquals(make_shared<BytesRef>(bytes),
                           make_shared<BytesRef>(actual));
    for (int sliceStart = 0; sliceStart < bytes.size(); sliceStart++) {
      for (int sliceLength = 0; sliceLength < bytes.size() - sliceStart;
           sliceLength++) {
        std::deque<char> slice(sliceLength);
        ii->seek(sliceStart);
        ii->readBytes(slice, 0, slice.size());
        TestUtil::assertEquals(
            make_shared<BytesRef>(bytes, sliceStart, sliceLength),
            make_shared<BytesRef>(slice));
      }
    }
    delete ii;
    delete mmapDir;
  }
}

void TestMultiMMap::testSlicedSeeking() 
{
  int numIters = TEST_NIGHTLY ? 10 : 1;
  for (int i = 0; i < numIters; i++) {
    shared_ptr<MMapDirectory> mmapDir =
        make_shared<MMapDirectory>(createTempDir(L"testSlicedSeeking"), 1 << i);
    shared_ptr<IndexOutput> io =
        mmapDir->createOutput(L"bytes", newIOContext(random()));
    std::deque<char> bytes(1 << (i + 1)); // make sure we switch buffers
    random()->nextBytes(bytes);
    io->writeBytes(bytes, bytes.size());
    delete io;
    shared_ptr<IndexInput> ii =
        mmapDir->openInput(L"bytes", newIOContext(random()));
    std::deque<char> actual(1 << (i + 1)); // first read all bytes
    ii->readBytes(actual, 0, actual.size());
    delete ii;
    TestUtil::assertEquals(make_shared<BytesRef>(bytes),
                           make_shared<BytesRef>(actual));
    shared_ptr<IndexInput> slicer =
        mmapDir->openInput(L"bytes", newIOContext(random()));
    for (int sliceStart = 0; sliceStart < bytes.size(); sliceStart++) {
      for (int sliceLength = 0; sliceLength < bytes.size() - sliceStart;
           sliceLength++) {
        assertSlice(bytes, slicer, 0, sliceStart, sliceLength);
      }
    }
    delete slicer;
    delete mmapDir;
  }
}

void TestMultiMMap::testSliceOfSlice() 
{
  for (int i = 0; i < 10; i++) {
    shared_ptr<MMapDirectory> mmapDir =
        make_shared<MMapDirectory>(createTempDir(L"testSliceOfSlice"), 1 << i);
    shared_ptr<IndexOutput> io =
        mmapDir->createOutput(L"bytes", newIOContext(random()));
    std::deque<char> bytes(1 << (i + 1)); // make sure we switch buffers
    random()->nextBytes(bytes);
    io->writeBytes(bytes, bytes.size());
    delete io;
    shared_ptr<IndexInput> ii =
        mmapDir->openInput(L"bytes", newIOContext(random()));
    std::deque<char> actual(1 << (i + 1)); // first read all bytes
    ii->readBytes(actual, 0, actual.size());
    delete ii;
    TestUtil::assertEquals(make_shared<BytesRef>(bytes),
                           make_shared<BytesRef>(actual));
    shared_ptr<IndexInput> outerSlicer =
        mmapDir->openInput(L"bytes", newIOContext(random()));
    constexpr int outerSliceStart = random()->nextInt(bytes.size() / 2);
    constexpr int outerSliceLength =
        random()->nextInt(bytes.size() - outerSliceStart);
    shared_ptr<IndexInput> innerSlicer = outerSlicer->slice(
        L"parentBytesSlice", outerSliceStart, outerSliceLength);
    for (int sliceStart = 0; sliceStart < outerSliceLength; sliceStart++) {
      for (int sliceLength = 0; sliceLength < outerSliceLength - sliceStart;
           sliceLength++) {
        assertSlice(bytes, innerSlicer, outerSliceStart, sliceStart,
                    sliceLength);
      }
    }
    delete innerSlicer;
    delete outerSlicer;
    delete mmapDir;
  }
}

void TestMultiMMap::assertSlice(std::deque<char> &bytes,
                                shared_ptr<IndexInput> slicer,
                                int outerSliceStart, int sliceStart,
                                int sliceLength) 
{
  std::deque<char> slice(sliceLength);
  shared_ptr<IndexInput> input =
      slicer->slice(L"bytesSlice", sliceStart, slice.size());
  input->readBytes(slice, 0, slice.size());
  delete input;
  TestUtil::assertEquals(
      make_shared<BytesRef>(bytes, outerSliceStart + sliceStart, sliceLength),
      make_shared<BytesRef>(slice));
}

void TestMultiMMap::testRandomChunkSizes() 
{
  int num = TEST_NIGHTLY ? atLeast(10) : 3;
  for (int i = 0; i < num; i++) {
    assertChunking(random(), TestUtil::nextInt(random(), 20, 100));
  }
}

void TestMultiMMap::assertChunking(shared_ptr<Random> random,
                                   int chunkSize) 
{
  shared_ptr<Path> path = createTempDir(L"mmap" + to_wstring(chunkSize));
  shared_ptr<MMapDirectory> mmapDir =
      make_shared<MMapDirectory>(path, chunkSize);
  // we will map_obj a lot, try to turn on the unmap hack
  if (MMapDirectory::UNMAP_SUPPORTED) {
    mmapDir->setUseUnmap(true);
  }
  shared_ptr<MockDirectoryWrapper> dir =
      make_shared<MockDirectoryWrapper>(random, mmapDir);
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random, dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random))
          ->setMergePolicy(newLogMergePolicy()));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> docid = newStringField(L"docid", L"0", Field::Store::YES);
  shared_ptr<Field> junk = newStringField(L"junk", L"", Field::Store::YES);
  doc->push_back(docid);
  doc->push_back(junk);

  int numDocs = 100;
  for (int i = 0; i < numDocs; i++) {
    docid->setStringValue(L"" + to_wstring(i));
    junk->setStringValue(TestUtil::randomUnicodeString(random));
    writer->addDocument(doc);
  }
  shared_ptr<IndexReader> reader = writer->getReader();
  delete writer;

  int numAsserts = atLeast(100);
  for (int i = 0; i < numAsserts; i++) {
    int docID = random->nextInt(numDocs);
    TestUtil::assertEquals(L"" + to_wstring(docID),
                           reader->document(docID)[L"docid"]);
  }
  delete reader;
  delete dir;
}

void TestMultiMMap::testImplementations() 
{
  for (int i = 2; i < 12; i++) {
    constexpr int chunkSize = 1 << i;
    shared_ptr<MMapDirectory> mmapDir = make_shared<MMapDirectory>(
        createTempDir(L"testImplementations"), chunkSize);
    shared_ptr<IndexOutput> io =
        mmapDir->createOutput(L"bytes", newIOContext(random()));
    int size = random()->nextInt(chunkSize * 2) +
               3; // add some buffer of 3 for slice tests
    std::deque<char> bytes(size);
    random()->nextBytes(bytes);
    io->writeBytes(bytes, bytes.size());
    delete io;
    shared_ptr<IndexInput> ii =
        mmapDir->openInput(L"bytes", newIOContext(random()));
    std::deque<char> actual(size); // first read all bytes
    ii->readBytes(actual, 0, actual.size());
    TestUtil::assertEquals(make_shared<BytesRef>(bytes),
                           make_shared<BytesRef>(actual));
    // reinit:
    ii->seek(0LL);

    // check impl (we must check size < chunksize: currently, if
    // size==chunkSize, we get 2 buffers, the second one empty:
    assertTrue(
        (size < chunkSize)
            ? (std::dynamic_pointer_cast<
                   ByteBufferIndexInput::SingleBufferImpl>(ii) != nullptr)
            : (std::dynamic_pointer_cast<ByteBufferIndexInput::MultiBufferImpl>(
                   ii) != nullptr));

    // clone tests:
    assertSame(ii->getClass(), ii->clone()->getClass());

    // slice test (offset 0)
    int sliceSize = random()->nextInt(size);
    shared_ptr<IndexInput> slice = ii->slice(L"slice", 0, sliceSize);
    assertTrue(
        (sliceSize < chunkSize)
            ? (std::dynamic_pointer_cast<
                   ByteBufferIndexInput::SingleBufferImpl>(slice) != nullptr)
            : (std::dynamic_pointer_cast<ByteBufferIndexInput::MultiBufferImpl>(
                   slice) != nullptr));

    // slice test (offset > 0 )
    int offset = random()->nextInt(size - 1) + 1;
    sliceSize = random()->nextInt(size - offset + 1);
    slice = ii->slice(L"slice", offset, sliceSize);
    // System.out.println(offset + "/" + sliceSize + " chunkSize=" + chunkSize +
    // " " + slice.getClass());
    if (offset % chunkSize + sliceSize < chunkSize) {
      assertTrue(
          std::dynamic_pointer_cast<ByteBufferIndexInput::SingleBufferImpl>(
              slice) != nullptr);
    } else {
      assertTrue(
          std::dynamic_pointer_cast<ByteBufferIndexInput::MultiBufferImpl>(
              slice) != nullptr);
    }

    delete ii;
    delete mmapDir;
  }
}
} // namespace org::apache::lucene::store