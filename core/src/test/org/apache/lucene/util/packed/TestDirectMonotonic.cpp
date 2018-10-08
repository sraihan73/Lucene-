using namespace std;

#include "TestDirectMonotonic.h"

namespace org::apache::lucene::util::packed
{
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using LongValues = org::apache::lucene::util::LongValues;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestDirectMonotonic::testEmpty() 
{
  shared_ptr<Directory> dir = newDirectory();
  constexpr int blockShift =
      TestUtil::nextInt(random(), DirectMonotonicWriter::MIN_BLOCK_SHIFT,
                        DirectMonotonicWriter::MAX_BLOCK_SHIFT);

  constexpr int64_t dataLength;
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput metaOut
  // = dir.createOutput("meta", org.apache.lucene.store.IOContext.DEFAULT);
  // org.apache.lucene.store.IndexOutput dataOut = dir.createOutput("data",
  // org.apache.lucene.store.IOContext.DEFAULT))
  {
    org::apache::lucene::store::IndexOutput metaOut = dir->createOutput(
        L"meta", org::apache::lucene::store::IOContext::DEFAULT);
    org::apache::lucene::store::IndexOutput dataOut = dir->createOutput(
        L"data", org::apache::lucene::store::IOContext::DEFAULT);
    shared_ptr<DirectMonotonicWriter> w =
        DirectMonotonicWriter::getInstance(metaOut, dataOut, 0, blockShift);
    w->finish();
    dataLength = dataOut->getFilePointer();
  }

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexInput metaIn =
  // dir.openInput("meta", org.apache.lucene.store.IOContext.READONCE);
  // org.apache.lucene.store.IndexInput dataIn = dir.openInput("data",
  // org.apache.lucene.store.IOContext.DEFAULT))
  {
    org::apache::lucene::store::IndexInput metaIn = dir->openInput(
        L"meta", org::apache::lucene::store::IOContext::READONCE);
    org::apache::lucene::store::IndexInput dataIn =
        dir->openInput(L"data", org::apache::lucene::store::IOContext::DEFAULT);
    shared_ptr<DirectMonotonicReader::Meta> meta =
        DirectMonotonicReader::loadMeta(metaIn, 0, blockShift);
    DirectMonotonicReader::getInstance(
        meta, dataIn->randomAccessSlice(0, dataLength));
    // no exception
  }

  delete dir;
}

void TestDirectMonotonic::testSimple() 
{
  shared_ptr<Directory> dir = newDirectory();
  constexpr int blockShift = 2;

  deque<int64_t> actualValues =
      Arrays::asList(1LL, 2LL, 5LL, 7LL, 8LL, 100LL);
  constexpr int numValues = actualValues.size();

  constexpr int64_t dataLength;
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput metaOut
  // = dir.createOutput("meta", org.apache.lucene.store.IOContext.DEFAULT);
  // org.apache.lucene.store.IndexOutput dataOut = dir.createOutput("data",
  // org.apache.lucene.store.IOContext.DEFAULT))
  {
    org::apache::lucene::store::IndexOutput metaOut = dir->createOutput(
        L"meta", org::apache::lucene::store::IOContext::DEFAULT);
    org::apache::lucene::store::IndexOutput dataOut = dir->createOutput(
        L"data", org::apache::lucene::store::IOContext::DEFAULT);
    shared_ptr<DirectMonotonicWriter> w = DirectMonotonicWriter::getInstance(
        metaOut, dataOut, numValues, blockShift);
    for (auto v : actualValues) {
      w->add(v);
    }
    w->finish();
    dataLength = dataOut->getFilePointer();
  }

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexInput metaIn =
  // dir.openInput("meta", org.apache.lucene.store.IOContext.READONCE);
  // org.apache.lucene.store.IndexInput dataIn = dir.openInput("data",
  // org.apache.lucene.store.IOContext.DEFAULT))
  {
    org::apache::lucene::store::IndexInput metaIn = dir->openInput(
        L"meta", org::apache::lucene::store::IOContext::READONCE);
    org::apache::lucene::store::IndexInput dataIn =
        dir->openInput(L"data", org::apache::lucene::store::IOContext::DEFAULT);
    shared_ptr<DirectMonotonicReader::Meta> meta =
        DirectMonotonicReader::loadMeta(metaIn, numValues, blockShift);
    shared_ptr<LongValues> values = DirectMonotonicReader::getInstance(
        meta, dataIn->randomAccessSlice(0, dataLength));
    for (int i = 0; i < numValues; ++i) {
      constexpr int64_t v = values->get(i);
      TestUtil::assertEquals(actualValues[i], v);
    }
  }

  delete dir;
}

void TestDirectMonotonic::testConstantSlope() 
{
  shared_ptr<Directory> dir = newDirectory();
  constexpr int blockShift =
      TestUtil::nextInt(random(), DirectMonotonicWriter::MIN_BLOCK_SHIFT,
                        DirectMonotonicWriter::MAX_BLOCK_SHIFT);
  constexpr int numValues = TestUtil::nextInt(random(), 1, 1 << 20);
  constexpr int64_t min = random()->nextLong();
  constexpr int64_t inc = random()->nextInt(1 << random()->nextInt(20));

  deque<int64_t> actualValues = deque<int64_t>();
  for (int i = 0; i < numValues; ++i) {
    actualValues.push_back(min + inc * i);
  }

  constexpr int64_t dataLength;
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput metaOut
  // = dir.createOutput("meta", org.apache.lucene.store.IOContext.DEFAULT);
  // org.apache.lucene.store.IndexOutput dataOut = dir.createOutput("data",
  // org.apache.lucene.store.IOContext.DEFAULT))
  {
    org::apache::lucene::store::IndexOutput metaOut = dir->createOutput(
        L"meta", org::apache::lucene::store::IOContext::DEFAULT);
    org::apache::lucene::store::IndexOutput dataOut = dir->createOutput(
        L"data", org::apache::lucene::store::IOContext::DEFAULT);
    shared_ptr<DirectMonotonicWriter> w = DirectMonotonicWriter::getInstance(
        metaOut, dataOut, numValues, blockShift);
    for (auto v : actualValues) {
      w->add(v);
    }
    w->finish();
    dataLength = dataOut->getFilePointer();
  }

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexInput metaIn =
  // dir.openInput("meta", org.apache.lucene.store.IOContext.READONCE);
  // org.apache.lucene.store.IndexInput dataIn = dir.openInput("data",
  // org.apache.lucene.store.IOContext.DEFAULT))
  {
    org::apache::lucene::store::IndexInput metaIn = dir->openInput(
        L"meta", org::apache::lucene::store::IOContext::READONCE);
    org::apache::lucene::store::IndexInput dataIn =
        dir->openInput(L"data", org::apache::lucene::store::IOContext::DEFAULT);
    shared_ptr<DirectMonotonicReader::Meta> meta =
        DirectMonotonicReader::loadMeta(metaIn, numValues, blockShift);
    shared_ptr<LongValues> values = DirectMonotonicReader::getInstance(
        meta, dataIn->randomAccessSlice(0, dataLength));
    for (int i = 0; i < numValues; ++i) {
      TestUtil::assertEquals(actualValues[i], values->get(i));
    }
    TestUtil::assertEquals(0, dataIn->getFilePointer());
  }

  delete dir;
}

void TestDirectMonotonic::testRandom() 
{
  constexpr int iters = atLeast(3);
  for (int iter = 0; iter < iters; ++iter) {
    shared_ptr<Directory> dir = newDirectory();
    constexpr int blockShift =
        TestUtil::nextInt(random(), DirectMonotonicWriter::MIN_BLOCK_SHIFT,
                          DirectMonotonicWriter::MAX_BLOCK_SHIFT);
    constexpr int maxNumValues = 1 << 20;
    constexpr int numValues;
    if (random()->nextBoolean()) {
      // random number
      numValues = TestUtil::nextInt(random(), 1, maxNumValues);
    } else {
      // multiple of the block size
      constexpr int numBlocks = TestUtil::nextInt(
          random(), 0,
          static_cast<int>(static_cast<unsigned int>(maxNumValues) >>
                           blockShift));
      numValues = TestUtil::nextInt(random(), 0, numBlocks) << blockShift;
    }
    deque<int64_t> actualValues = deque<int64_t>();
    int64_t previous = random()->nextLong();
    if (numValues > 0) {
      actualValues.push_back(previous);
    }
    for (int i = 1; i < numValues; ++i) {
      previous += random()->nextInt(1 << random()->nextInt(20));
      actualValues.push_back(previous);
    }

    constexpr int64_t dataLength;
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput
    // metaOut = dir.createOutput("meta",
    // org.apache.lucene.store.IOContext.DEFAULT);
    // org.apache.lucene.store.IndexOutput dataOut = dir.createOutput("data",
    // org.apache.lucene.store.IOContext.DEFAULT))
    {
      org::apache::lucene::store::IndexOutput metaOut = dir->createOutput(
          L"meta", org::apache::lucene::store::IOContext::DEFAULT);
      org::apache::lucene::store::IndexOutput dataOut = dir->createOutput(
          L"data", org::apache::lucene::store::IOContext::DEFAULT);
      shared_ptr<DirectMonotonicWriter> w = DirectMonotonicWriter::getInstance(
          metaOut, dataOut, numValues, blockShift);
      for (auto v : actualValues) {
        w->add(v);
      }
      w->finish();
      dataLength = dataOut->getFilePointer();
    }

    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexInput metaIn
    // = dir.openInput("meta", org.apache.lucene.store.IOContext.READONCE);
    // org.apache.lucene.store.IndexInput dataIn = dir.openInput("data",
    // org.apache.lucene.store.IOContext.DEFAULT))
    {
      org::apache::lucene::store::IndexInput metaIn = dir->openInput(
          L"meta", org::apache::lucene::store::IOContext::READONCE);
      org::apache::lucene::store::IndexInput dataIn = dir->openInput(
          L"data", org::apache::lucene::store::IOContext::DEFAULT);
      shared_ptr<DirectMonotonicReader::Meta> meta =
          DirectMonotonicReader::loadMeta(metaIn, numValues, blockShift);
      shared_ptr<LongValues> values = DirectMonotonicReader::getInstance(
          meta, dataIn->randomAccessSlice(0, dataLength));
      for (int i = 0; i < numValues; ++i) {
        TestUtil::assertEquals(actualValues[i], values->get(i));
      }
    }

    delete dir;
  }
}
} // namespace org::apache::lucene::util::packed