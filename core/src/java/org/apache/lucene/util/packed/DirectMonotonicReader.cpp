using namespace std;

#include "DirectMonotonicReader.h"

namespace org::apache::lucene::util::packed
{
using IndexInput = org::apache::lucene::store::IndexInput;
using RandomAccessInput = org::apache::lucene::store::RandomAccessInput;
using Accountable = org::apache::lucene::util::Accountable;
using LongValues = org::apache::lucene::util::LongValues;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
const shared_ptr<org::apache::lucene::util::LongValues>
    DirectMonotonicReader::EMPTY = make_shared<LongValuesAnonymousInnerClass>();

DirectMonotonicReader::LongValuesAnonymousInnerClass::
    LongValuesAnonymousInnerClass()
{
}

int64_t
DirectMonotonicReader::LongValuesAnonymousInnerClass::get(int64_t index)
{
  return 0;
}

DirectMonotonicReader::Meta::Meta(int64_t numValues, int blockShift)
    : blockShift(blockShift), numBlocks(static_cast<int>(numBlocks)),
      mins(std::deque<int64_t>(this->numBlocks)),
      avgs(std::deque<float>(this->numBlocks)),
      bpvs(std::deque<char>(this->numBlocks)),
      offsets(std::deque<int64_t>(this->numBlocks))
{
  int64_t numBlocks = static_cast<int64_t>(
      static_cast<uint64_t>(numValues) >> blockShift);
  if ((numBlocks << blockShift) < numValues) {
    numBlocks += 1;
  }
}

int64_t DirectMonotonicReader::Meta::ramBytesUsed()
{
  return BASE_RAM_BYTES_USED + RamUsageEstimator::sizeOf(mins) +
         RamUsageEstimator::sizeOf(avgs) + RamUsageEstimator::sizeOf(bpvs) +
         RamUsageEstimator::sizeOf(offsets);
}

shared_ptr<Meta>
DirectMonotonicReader::loadMeta(shared_ptr<IndexInput> metaIn,
                                int64_t numValues,
                                int blockShift) 
{
  shared_ptr<Meta> meta = make_shared<Meta>(numValues, blockShift);
  for (int i = 0; i < meta->numBlocks; ++i) {
    meta->mins[i] = metaIn->readLong();
    meta->avgs[i] = Float::intBitsToFloat(metaIn->readInt());
    meta->offsets[i] = metaIn->readLong();
    meta->bpvs[i] = metaIn->readByte();
  }
  return meta;
}

shared_ptr<LongValues> DirectMonotonicReader::getInstance(
    shared_ptr<Meta> meta,
    shared_ptr<RandomAccessInput> data) 
{
  std::deque<std::shared_ptr<LongValues>> readers(meta->numBlocks);
  for (int i = 0; i < meta->mins.size(); ++i) {
    if (meta->bpvs[i] == 0) {
      readers[i] = EMPTY;
    } else {
      readers[i] =
          DirectReader::getInstance(data, meta->bpvs[i], meta->offsets[i]);
    }
  }
  constexpr int blockShift = meta->blockShift;

  const std::deque<int64_t> mins = meta->mins;
  const std::deque<float> avgs = meta->avgs;
  return make_shared<LongValuesAnonymousInnerClass>(readers, blockShift, mins,
                                                    avgs);
}

DirectMonotonicReader::LongValuesAnonymousInnerClass::
    LongValuesAnonymousInnerClass(deque<std::shared_ptr<LongValues>> &readers,
                                  int blockShift, deque<int64_t> &mins,
                                  deque<float> &avgs)
{
  this->readers = readers;
  this->blockShift = blockShift;
  this->mins = mins;
  this->avgs = avgs;
}

int64_t
DirectMonotonicReader::LongValuesAnonymousInnerClass::get(int64_t index)
{
  constexpr int block = static_cast<int>(static_cast<int64_t>(
      static_cast<uint64_t>(index) >> blockShift));
  constexpr int64_t blockIndex = index & ((1 << blockShift) - 1);
  constexpr int64_t delta = readers[block]->get(blockIndex);
  return mins[block] + static_cast<int64_t>(avgs[block] * blockIndex) + delta;
}
} // namespace org::apache::lucene::util::packed