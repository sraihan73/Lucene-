using namespace std;

#include "AbstractBlockPackedWriter.h"

namespace org::apache::lucene::util::packed
{
//    import static org.apache.lucene.util.packed.PackedInts.checkBlockSize;
using DataOutput = org::apache::lucene::store::DataOutput;

void AbstractBlockPackedWriter::writeVLong(shared_ptr<DataOutput> out,
                                           int64_t i) 
{
  int k = 0;
  while ((i & ~0x7FLL) != 0LL && k++ < 8) {
    out->writeByte(static_cast<char>((i & 0x7FLL) | 0x80LL));
    i = static_cast<int64_t>(static_cast<uint64_t>(i) >> 7);
  }
  out->writeByte(static_cast<char>(i));
}

AbstractBlockPackedWriter::AbstractBlockPackedWriter(shared_ptr<DataOutput> out,
                                                     int blockSize)
    : values(std::deque<int64_t>(blockSize))
{
  checkBlockSize(blockSize, MIN_BLOCK_SIZE, MAX_BLOCK_SIZE);
  reset(out);
}

void AbstractBlockPackedWriter::reset(shared_ptr<DataOutput> out)
{
  assert(out != nullptr);
  this->out = out;
  off = 0;
  ord_ = 0LL;
  finished = false;
}

void AbstractBlockPackedWriter::checkNotFinished()
{
  if (finished) {
    throw make_shared<IllegalStateException>(L"Already finished");
  }
}

void AbstractBlockPackedWriter::add(int64_t l) 
{
  checkNotFinished();
  if (off == values.size()) {
    flush();
  }
  values[off++] = l;
  ++ord_;
}

void AbstractBlockPackedWriter::addBlockOfZeros() 
{
  checkNotFinished();
  if (off != 0 && off != values.size()) {
    throw make_shared<IllegalStateException>(L"" + to_wstring(off));
  }
  if (off == values.size()) {
    flush();
  }
  Arrays::fill(values, 0);
  off = values.size();
  ord_ += values.size();
}

void AbstractBlockPackedWriter::finish() 
{
  checkNotFinished();
  if (off > 0) {
    flush();
  }
  finished = true;
}

int64_t AbstractBlockPackedWriter::ord() { return ord_; }

void AbstractBlockPackedWriter::writeValues(int bitsRequired) 
{
  shared_ptr<PackedInts::Encoder> *const encoder = PackedInts::getEncoder(
      PackedInts::Format::PACKED, PackedInts::VERSION_CURRENT, bitsRequired);
  constexpr int iterations = values.size() / encoder->byteValueCount();
  constexpr int blockSize = encoder->byteBlockCount() * iterations;
  if (blocks.empty() || blocks.size() < blockSize) {
    blocks = std::deque<char>(blockSize);
  }
  if (off < values.size()) {
    Arrays::fill(values, off, values.size(), 0LL);
  }
  encoder->encode(values, 0, blocks, 0, iterations);
  constexpr int blockCount =
      static_cast<int>(PackedInts::Format::PACKED.byteCount(
          PackedInts::VERSION_CURRENT, off, bitsRequired));
  out->writeBytes(blocks, blockCount);
}
} // namespace org::apache::lucene::util::packed