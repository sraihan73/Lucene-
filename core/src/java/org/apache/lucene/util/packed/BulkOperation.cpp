using namespace std;

#include "BulkOperation.h"

namespace org::apache::lucene::util::packed
{

std::deque<std::shared_ptr<BulkOperation>> const BulkOperation::packedBulkOps =
    std::deque<std::shared_ptr<BulkOperation>>{
        make_shared<BulkOperationPacked1>(),
        make_shared<BulkOperationPacked2>(),
        make_shared<BulkOperationPacked3>(),
        make_shared<BulkOperationPacked4>(),
        make_shared<BulkOperationPacked5>(),
        make_shared<BulkOperationPacked6>(),
        make_shared<BulkOperationPacked7>(),
        make_shared<BulkOperationPacked8>(),
        make_shared<BulkOperationPacked9>(),
        make_shared<BulkOperationPacked10>(),
        make_shared<BulkOperationPacked11>(),
        make_shared<BulkOperationPacked12>(),
        make_shared<BulkOperationPacked13>(),
        make_shared<BulkOperationPacked14>(),
        make_shared<BulkOperationPacked15>(),
        make_shared<BulkOperationPacked16>(),
        make_shared<BulkOperationPacked17>(),
        make_shared<BulkOperationPacked18>(),
        make_shared<BulkOperationPacked19>(),
        make_shared<BulkOperationPacked20>(),
        make_shared<BulkOperationPacked21>(),
        make_shared<BulkOperationPacked22>(),
        make_shared<BulkOperationPacked23>(),
        make_shared<BulkOperationPacked24>(),
        make_shared<BulkOperationPacked>(25),
        make_shared<BulkOperationPacked>(26),
        make_shared<BulkOperationPacked>(27),
        make_shared<BulkOperationPacked>(28),
        make_shared<BulkOperationPacked>(29),
        make_shared<BulkOperationPacked>(30),
        make_shared<BulkOperationPacked>(31),
        make_shared<BulkOperationPacked>(32),
        make_shared<BulkOperationPacked>(33),
        make_shared<BulkOperationPacked>(34),
        make_shared<BulkOperationPacked>(35),
        make_shared<BulkOperationPacked>(36),
        make_shared<BulkOperationPacked>(37),
        make_shared<BulkOperationPacked>(38),
        make_shared<BulkOperationPacked>(39),
        make_shared<BulkOperationPacked>(40),
        make_shared<BulkOperationPacked>(41),
        make_shared<BulkOperationPacked>(42),
        make_shared<BulkOperationPacked>(43),
        make_shared<BulkOperationPacked>(44),
        make_shared<BulkOperationPacked>(45),
        make_shared<BulkOperationPacked>(46),
        make_shared<BulkOperationPacked>(47),
        make_shared<BulkOperationPacked>(48),
        make_shared<BulkOperationPacked>(49),
        make_shared<BulkOperationPacked>(50),
        make_shared<BulkOperationPacked>(51),
        make_shared<BulkOperationPacked>(52),
        make_shared<BulkOperationPacked>(53),
        make_shared<BulkOperationPacked>(54),
        make_shared<BulkOperationPacked>(55),
        make_shared<BulkOperationPacked>(56),
        make_shared<BulkOperationPacked>(57),
        make_shared<BulkOperationPacked>(58),
        make_shared<BulkOperationPacked>(59),
        make_shared<BulkOperationPacked>(60),
        make_shared<BulkOperationPacked>(61),
        make_shared<BulkOperationPacked>(62),
        make_shared<BulkOperationPacked>(63),
        make_shared<BulkOperationPacked>(64)};
std::deque<std::shared_ptr<BulkOperation>> const
    BulkOperation::packedSingleBlockBulkOps =
        std::deque<std::shared_ptr<BulkOperation>>{
            make_shared<BulkOperationPackedSingleBlock>(1),
            make_shared<BulkOperationPackedSingleBlock>(2),
            make_shared<BulkOperationPackedSingleBlock>(3),
            make_shared<BulkOperationPackedSingleBlock>(4),
            make_shared<BulkOperationPackedSingleBlock>(5),
            make_shared<BulkOperationPackedSingleBlock>(6),
            make_shared<BulkOperationPackedSingleBlock>(7),
            make_shared<BulkOperationPackedSingleBlock>(8),
            make_shared<BulkOperationPackedSingleBlock>(9),
            make_shared<BulkOperationPackedSingleBlock>(10),
            nullptr,
            make_shared<BulkOperationPackedSingleBlock>(12),
            nullptr,
            nullptr,
            nullptr,
            make_shared<BulkOperationPackedSingleBlock>(16),
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            make_shared<BulkOperationPackedSingleBlock>(21),
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            make_shared<BulkOperationPackedSingleBlock>(32)};

shared_ptr<BulkOperation> BulkOperation::of(PackedInts::Format format,
                                            int bitsPerValue)
{
  switch (format) {
  case org::apache::lucene::util::packed::PackedInts::Format::PACKED:
    assert(packedBulkOps[bitsPerValue - 1] != nullptr);
    return packedBulkOps[bitsPerValue - 1];
  case org::apache::lucene::util::packed::PackedInts::Format::
      PACKED_SINGLE_BLOCK:
    assert(packedSingleBlockBulkOps[bitsPerValue - 1] != nullptr);
    return packedSingleBlockBulkOps[bitsPerValue - 1];
  default:
    throw make_shared<AssertionError>();
  }
}

int BulkOperation::writeLong(int64_t block, std::deque<char> &blocks,
                             int blocksOffset)
{
  for (int j = 1; j <= 8; ++j) {
    blocks[blocksOffset++] = static_cast<char>(static_cast<int64_t>(
        static_cast<uint64_t>(block) >> (64 - (j << 3))));
  }
  return blocksOffset;
}

int BulkOperation::computeIterations(int valueCount, int ramBudget)
{
  constexpr int iterations =
      ramBudget / (byteBlockCount() + 8 * byteValueCount());
  if (iterations == 0) {
    // at least 1
    return 1;
  } else if ((iterations - 1) * byteValueCount() >= valueCount) {
    // don't allocate for more than the size of the reader
    return static_cast<int>(
        ceil(static_cast<double>(valueCount) / byteValueCount()));
  } else {
    return iterations;
  }
}
} // namespace org::apache::lucene::util::packed