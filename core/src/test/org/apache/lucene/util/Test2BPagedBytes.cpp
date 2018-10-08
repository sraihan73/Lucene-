using namespace std;

#include "Test2BPagedBytes.h"

namespace org::apache::lucene::util
{
using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using org::apache::lucene::util::LuceneTestCase::Monster;

void Test2BPagedBytes::test() 
{
  shared_ptr<BaseDirectoryWrapper> dir =
      newFSDirectory(createTempDir(L"test2BPagedBytes"));
  if (std::dynamic_pointer_cast<MockDirectoryWrapper>(dir) != nullptr) {
    (std::static_pointer_cast<MockDirectoryWrapper>(dir))
        ->setThrottling(MockDirectoryWrapper::Throttling::NEVER);
  }
  shared_ptr<PagedBytes> pb = make_shared<PagedBytes>(15);
  shared_ptr<IndexOutput> dataOutput =
      dir->createOutput(L"foo", IOContext::DEFAULT);
  int64_t netBytes = 0;
  int64_t seed = random()->nextLong();
  int64_t lastFP = 0;
  shared_ptr<Random> r2 = make_shared<Random>(seed);
  while (netBytes < 1.1 * numeric_limits<int>::max()) {
    int numBytes = TestUtil::nextInt(r2, 1, 32768);
    std::deque<char> bytes(numBytes);
    r2->nextBytes(bytes);
    dataOutput->writeBytes(bytes, bytes.size());
    int64_t fp = dataOutput->getFilePointer();
    assert(fp == lastFP + numBytes);
    lastFP = fp;
    netBytes += numBytes;
  }
  delete dataOutput;
  shared_ptr<IndexInput> input = dir->openInput(L"foo", IOContext::DEFAULT);
  pb->copy(input, input->length());
  delete input;
  shared_ptr<PagedBytes::Reader> reader = pb->freeze(true);

  r2 = make_shared<Random>(seed);
  netBytes = 0;
  while (netBytes < 1.1 * numeric_limits<int>::max()) {
    int numBytes = TestUtil::nextInt(r2, 1, 32768);
    std::deque<char> bytes(numBytes);
    r2->nextBytes(bytes);
    shared_ptr<BytesRef> expected = make_shared<BytesRef>(bytes);

    shared_ptr<BytesRef> actual = make_shared<BytesRef>();
    reader->fillSlice(actual, netBytes, numBytes);
    assertEquals(expected, actual);

    netBytes += numBytes;
  }
  delete dir;
}
} // namespace org::apache::lucene::util