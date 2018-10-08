using namespace std;

#include "TestForUtil.h"

namespace org::apache::lucene::codecs::lucene50
{
//    import static
//    org.apache.lucene.codecs.lucene50.Lucene50PostingsFormat.BLOCK_SIZE;
//    import static org.apache.lucene.codecs.lucene50.ForUtil.MAX_DATA_SIZE;
//    import static org.apache.lucene.codecs.lucene50.ForUtil.MAX_ENCODED_SIZE;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using PackedInts = org::apache::lucene::util::packed::PackedInts;
using com::carrotsearch::randomizedtesting::generators::RandomNumbers;

void TestForUtil::testEncodeDecode() 
{
  constexpr int iterations = RandomNumbers::randomIntBetween(random(), 1, 1000);
  constexpr float acceptableOverheadRatio = random()->nextFloat();
  const std::deque<int> values =
      std::deque<int>((iterations - 1) * BLOCK_SIZE + ForUtil::MAX_DATA_SIZE);
  for (int i = 0; i < iterations; ++i) {
    constexpr int bpv = random()->nextInt(32);
    if (bpv == 0) {
      constexpr int value = RandomNumbers::randomIntBetween(
          random(), 0, numeric_limits<int>::max());
      for (int j = 0; j < BLOCK_SIZE; ++j) {
        values[i * BLOCK_SIZE + j] = value;
      }
    } else {
      for (int j = 0; j < BLOCK_SIZE; ++j) {
        values[i * BLOCK_SIZE + j] = RandomNumbers::randomIntBetween(
            random(), 0, static_cast<int>(PackedInts::maxValue(bpv)));
      }
    }
  }

  shared_ptr<Directory> *const d = make_shared<RAMDirectory>();
  constexpr int64_t endPointer;

  {
    // encode
    shared_ptr<IndexOutput> out =
        d->createOutput(L"test.bin", IOContext::DEFAULT);
    shared_ptr<ForUtil> *const forUtil =
        make_shared<ForUtil>(acceptableOverheadRatio, out);

    for (int i = 0; i < iterations; ++i) {
      forUtil->writeBlock(
          Arrays::copyOfRange(values, i * BLOCK_SIZE, values.size()),
          std::deque<char>(MAX_ENCODED_SIZE), out);
    }
    endPointer = out->getFilePointer();
    delete out;
  }

  {
    // decode
    shared_ptr<IndexInput> in_ = d->openInput(L"test.bin", IOContext::READONCE);
    shared_ptr<ForUtil> *const forUtil = make_shared<ForUtil>(in_);
    for (int i = 0; i < iterations; ++i) {
      if (random()->nextBoolean()) {
        forUtil->skipBlock(in_);
        continue;
      }
      const std::deque<int> restored = std::deque<int>(MAX_DATA_SIZE);
      forUtil->readBlock(in_, std::deque<char>(MAX_ENCODED_SIZE), restored);
      assertArrayEquals(
          Arrays::copyOfRange(values, i * BLOCK_SIZE, (i + 1) * BLOCK_SIZE),
          Arrays::copyOf(restored, BLOCK_SIZE));
    }
    assertEquals(endPointer, in_->getFilePointer());
    delete in_;
  }

  delete d;
}
} // namespace org::apache::lucene::codecs::lucene50