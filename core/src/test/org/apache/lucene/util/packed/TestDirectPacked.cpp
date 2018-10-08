using namespace std;

#include "TestDirectPacked.h"

namespace org::apache::lucene::util::packed
{
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using LongValues = org::apache::lucene::util::LongValues;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using DirectReader = org::apache::lucene::util::packed::DirectReader;
using DirectWriter = org::apache::lucene::util::packed::DirectWriter;

void TestDirectPacked::testSimple() 
{
  shared_ptr<Directory> dir = newDirectory();
  int bitsPerValue = DirectWriter::bitsRequired(2);
  shared_ptr<IndexOutput> output =
      dir->createOutput(L"foo", IOContext::DEFAULT);
  shared_ptr<DirectWriter> writer =
      DirectWriter::getInstance(output, 5, bitsPerValue);
  writer->add(1);
  writer->add(0);
  writer->add(2);
  writer->add(1);
  writer->add(2);
  writer->finish();
  delete output;
  shared_ptr<IndexInput> input = dir->openInput(L"foo", IOContext::DEFAULT);
  shared_ptr<LongValues> reader = DirectReader::getInstance(
      input->randomAccessSlice(0, input->length()), bitsPerValue, 0);
  TestUtil::assertEquals(1, reader->get(0));
  TestUtil::assertEquals(0, reader->get(1));
  TestUtil::assertEquals(2, reader->get(2));
  TestUtil::assertEquals(1, reader->get(3));
  TestUtil::assertEquals(2, reader->get(4));
  delete input;
  delete dir;
}

void TestDirectPacked::testNotEnoughValues() 
{
  shared_ptr<Directory> dir = newDirectory();
  int bitsPerValue = DirectWriter::bitsRequired(2);
  shared_ptr<IndexOutput> output =
      dir->createOutput(L"foo", IOContext::DEFAULT);
  shared_ptr<DirectWriter> writer =
      DirectWriter::getInstance(output, 5, bitsPerValue);
  writer->add(1);
  writer->add(0);
  writer->add(2);
  writer->add(1);
  shared_ptr<IllegalStateException> expected =
      expectThrows(IllegalStateException::typeid, [&]() { writer->finish(); });
  assertTrue(
      expected->getMessage()->startsWith(L"Wrong number of values added"));

  delete output;
  delete dir;
}

void TestDirectPacked::testRandom() 
{
  shared_ptr<Directory> dir = newDirectory();
  for (int bpv = 1; bpv <= 64; bpv++) {
    doTestBpv(dir, bpv, 0);
  }
  delete dir;
}

void TestDirectPacked::testRandomWithOffset() 
{
  shared_ptr<Directory> dir = newDirectory();
  constexpr int offset = TestUtil::nextInt(random(), 1, 100);
  for (int bpv = 1; bpv <= 64; bpv++) {
    doTestBpv(dir, bpv, offset);
  }
  delete dir;
}

void TestDirectPacked::doTestBpv(shared_ptr<Directory> directory, int bpv,
                                 int64_t offset) 
{
  shared_ptr<MyRandom> random =
      make_shared<MyRandom>(TestDirectPacked::random()->nextLong());
  int numIters = TEST_NIGHTLY ? 100 : 10;
  for (int i = 0; i < numIters; i++) {
    std::deque<int64_t> original = randomLongs(random, bpv);
    int bitsRequired =
        bpv == 64 ? 64 : DirectWriter::bitsRequired(1LL << (bpv - 1));
    wstring name = L"bpv" + to_wstring(bpv) + L"_" + to_wstring(i);
    shared_ptr<IndexOutput> output =
        directory->createOutput(name, IOContext::DEFAULT);
    for (int64_t j = 0; j < offset; ++j) {
      output->writeByte(
          static_cast<char>(TestDirectPacked::random()->nextInt()));
    }
    shared_ptr<DirectWriter> writer =
        DirectWriter::getInstance(output, original.size(), bitsRequired);
    for (int j = 0; j < original.size(); j++) {
      writer->add(original[j]);
    }
    writer->finish();
    delete output;
    shared_ptr<IndexInput> input =
        directory->openInput(name, IOContext::DEFAULT);
    shared_ptr<LongValues> reader = DirectReader::getInstance(
        input->randomAccessSlice(0, input->length()), bitsRequired, offset);
    for (int j = 0; j < original.size(); j++) {
      assertEquals(L"bpv=" + to_wstring(bpv), original[j], reader->get(j));
    }
    delete input;
  }
}

std::deque<int64_t>
TestDirectPacked::randomLongs(shared_ptr<MyRandom> random, int bpv)
{
  int amount = random->nextInt(5000);
  std::deque<int64_t> longs(amount);
  for (int i = 0; i < longs.size(); i++) {
    longs[i] = random->nextLong(bpv);
  }
  return longs;
}

TestDirectPacked::MyRandom::MyRandom(int64_t seed) : java::util::Random(seed)
{
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t TestDirectPacked::MyRandom::nextLong(int bpv)
{
  nextBytes(buffer);
  input->reset(buffer);
  int64_t bits = input->readLong();
  return static_cast<int64_t>(static_cast<uint64_t>(bits) >>
                                (64 - bpv));
}
} // namespace org::apache::lucene::util::packed