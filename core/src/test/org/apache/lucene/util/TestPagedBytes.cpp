using namespace std;

#include "TestPagedBytes.h"

namespace org::apache::lucene::util
{
using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using org::junit::Ignore;

void TestPagedBytes::testDataInputOutput() 
{
  shared_ptr<Random> random = TestPagedBytes::random();
  for (int iter = 0; iter < 5 * RANDOM_MULTIPLIER; iter++) {
    shared_ptr<BaseDirectoryWrapper> dir =
        newFSDirectory(createTempDir(L"testOverflow"));
    if (std::dynamic_pointer_cast<MockDirectoryWrapper>(dir) != nullptr) {
      (std::static_pointer_cast<MockDirectoryWrapper>(dir))
          ->setThrottling(MockDirectoryWrapper::Throttling::NEVER);
    }
    constexpr int blockBits = TestUtil::nextInt(random, 1, 20);
    constexpr int blockSize = 1 << blockBits;
    shared_ptr<PagedBytes> *const p = make_shared<PagedBytes>(blockBits);
    shared_ptr<IndexOutput> *const out =
        dir->createOutput(L"foo", IOContext::DEFAULT);
    constexpr int numBytes =
        TestUtil::nextInt(TestPagedBytes::random(), 2, 10000000);

    const std::deque<char> answer = std::deque<char>(numBytes);
    TestPagedBytes::random()->nextBytes(answer);
    int written = 0;
    while (written < numBytes) {
      if (TestPagedBytes::random()->nextInt(10) == 7) {
        out->writeByte(answer[written++]);
      } else {
        int chunk =
            min(TestPagedBytes::random()->nextInt(1000), numBytes - written);
        out->writeBytes(answer, written, chunk);
        written += chunk;
      }
    }

    delete out;
    shared_ptr<IndexInput> *const input =
        dir->openInput(L"foo", IOContext::DEFAULT);
    shared_ptr<DataInput> *const in_ = input->clone();

    p->copy(input, input->length());
    shared_ptr<PagedBytes::Reader> *const reader =
        p->freeze(random->nextBoolean());

    const std::deque<char> verify = std::deque<char>(numBytes);
    int read = 0;
    while (read < numBytes) {
      if (TestPagedBytes::random()->nextInt(10) == 7) {
        verify[read++] = in_->readByte();
      } else {
        int chunk =
            min(TestPagedBytes::random()->nextInt(1000), numBytes - read);
        in_->readBytes(verify, read, chunk);
        read += chunk;
      }
    }
    assertTrue(Arrays::equals(answer, verify));

    shared_ptr<BytesRef> *const slice = make_shared<BytesRef>();
    for (int iter2 = 0; iter2 < 100; iter2++) {
      constexpr int pos = random->nextInt(numBytes - 1);
      constexpr int len = random->nextInt(min(blockSize + 1, numBytes - pos));
      reader->fillSlice(slice, pos, len);
      for (int byteUpto = 0; byteUpto < len; byteUpto++) {
        assertEquals(answer[pos + byteUpto],
                     slice->bytes[slice->offset + byteUpto]);
      }
    }
    delete input;
    delete dir;
  }
}

void TestPagedBytes::testDataInputOutput2() 
{
  shared_ptr<Random> random = TestPagedBytes::random();
  for (int iter = 0; iter < 5 * RANDOM_MULTIPLIER; iter++) {
    constexpr int blockBits = TestUtil::nextInt(random, 1, 20);
    constexpr int blockSize = 1 << blockBits;
    shared_ptr<PagedBytes> *const p = make_shared<PagedBytes>(blockBits);
    shared_ptr<DataOutput> *const out = p->getDataOutput();
    constexpr int numBytes = TestPagedBytes::random()->nextInt(10000000);

    const std::deque<char> answer = std::deque<char>(numBytes);
    TestPagedBytes::random()->nextBytes(answer);
    int written = 0;
    while (written < numBytes) {
      if (TestPagedBytes::random()->nextInt(10) == 7) {
        out->writeByte(answer[written++]);
      } else {
        int chunk =
            min(TestPagedBytes::random()->nextInt(1000), numBytes - written);
        out->writeBytes(answer, written, chunk);
        written += chunk;
      }
    }

    shared_ptr<PagedBytes::Reader> *const reader =
        p->freeze(random->nextBoolean());

    shared_ptr<DataInput> *const in_ = p->getDataInput();

    const std::deque<char> verify = std::deque<char>(numBytes);
    int read = 0;
    while (read < numBytes) {
      if (TestPagedBytes::random()->nextInt(10) == 7) {
        verify[read++] = in_->readByte();
      } else {
        int chunk =
            min(TestPagedBytes::random()->nextInt(1000), numBytes - read);
        in_->readBytes(verify, read, chunk);
        read += chunk;
      }
    }
    assertTrue(Arrays::equals(answer, verify));

    shared_ptr<BytesRef> *const slice = make_shared<BytesRef>();
    for (int iter2 = 0; iter2 < 100; iter2++) {
      constexpr int pos = random->nextInt(numBytes - 1);
      constexpr int len = random->nextInt(min(blockSize + 1, numBytes - pos));
      reader->fillSlice(slice, pos, len);
      for (int byteUpto = 0; byteUpto < len; byteUpto++) {
        assertEquals(answer[pos + byteUpto],
                     slice->bytes[slice->offset + byteUpto]);
      }
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Ignore public void testOverflow() throws java.io.IOException
void TestPagedBytes::testOverflow() 
{
  shared_ptr<BaseDirectoryWrapper> dir =
      newFSDirectory(createTempDir(L"testOverflow"));
  if (std::dynamic_pointer_cast<MockDirectoryWrapper>(dir) != nullptr) {
    (std::static_pointer_cast<MockDirectoryWrapper>(dir))
        ->setThrottling(MockDirectoryWrapper::Throttling::NEVER);
  }
  constexpr int blockBits = TestUtil::nextInt(random(), 14, 28);
  constexpr int blockSize = 1 << blockBits;
  std::deque<char> arr(
      TestUtil::nextInt(random(), blockSize / 2, blockSize * 2));
  for (int i = 0; i < arr.size(); ++i) {
    arr[i] = static_cast<char>(i);
  }
  constexpr int64_t numBytes =
      (1LL << 31) + TestUtil::nextInt(random(), 1, blockSize * 3);
  shared_ptr<PagedBytes> *const p = make_shared<PagedBytes>(blockBits);
  shared_ptr<IndexOutput> *const out =
      dir->createOutput(L"foo", IOContext::DEFAULT);
  for (int64_t i = 0; i < numBytes;) {
    assertEquals(i, out->getFilePointer());
    constexpr int len = static_cast<int>(min(arr.size(), numBytes - i));
    out->writeBytes(arr, len);
    i += len;
  }
  assertEquals(numBytes, out->getFilePointer());
  delete out;
  shared_ptr<IndexInput> *const in_ =
      dir->openInput(L"foo", IOContext::DEFAULT);
  p->copy(in_, numBytes);
  shared_ptr<PagedBytes::Reader> *const reader =
      p->freeze(random()->nextBoolean());

  for (auto offset :
       std::deque<int64_t>{0LL, numeric_limits<int>::max(), numBytes - 1,
                              TestUtil::nextLong(random(), 1, numBytes - 2)}) {
    shared_ptr<BytesRef> b = make_shared<BytesRef>();
    reader->fillSlice(b, offset, 1);
    assertEquals(arr[static_cast<int>(offset % arr.size())],
                 b->bytes[b->offset]);
  }
  delete in_;
  delete dir;
}

void TestPagedBytes::testRamBytesUsed()
{
  constexpr int blockBits = TestUtil::nextInt(random(), 4, 22);
  shared_ptr<PagedBytes> b = make_shared<PagedBytes>(blockBits);
  constexpr int totalBytes = random()->nextInt(10000);
  for (int64_t pointer = 0; pointer < totalBytes;) {
    shared_ptr<BytesRef> bytes =
        make_shared<BytesRef>(TestUtil::randomSimpleString(random(), 10));
    pointer = b->copyUsingLengthPrefix(bytes);
  }
  assertEquals(RamUsageTester::sizeOf(b), b->ramBytesUsed());
  shared_ptr<PagedBytes::Reader> *const reader =
      b->freeze(random()->nextBoolean());
  assertEquals(RamUsageTester::sizeOf(b), b->ramBytesUsed());
  assertEquals(RamUsageTester::sizeOf(reader), reader->ramBytesUsed());
}
} // namespace org::apache::lucene::util