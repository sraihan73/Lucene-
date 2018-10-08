using namespace std;

#include "PagedMutable.h"

namespace org::apache::lucene::util::packed
{
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using Mutable = org::apache::lucene::util::packed::PackedInts::Mutable;

PagedMutable::PagedMutable(int64_t size, int pageSize, int bitsPerValue,
                           float acceptableOverheadRatio)
    : PagedMutable(size, pageSize,
                   PackedInts::fastestFormatAndBits(pageSize, bitsPerValue,
                                                    acceptableOverheadRatio))
{
  fillPages();
}

PagedMutable::PagedMutable(int64_t size, int pageSize,
                           shared_ptr<PackedInts::FormatAndBits> formatAndBits)
    : PagedMutable(size, pageSize, formatAndBits->bitsPerValue,
                   formatAndBits->format)
{
}

PagedMutable::PagedMutable(int64_t size, int pageSize, int bitsPerValue,
                           PackedInts::Format format)
    : AbstractPagedMutable<PagedMutable>(bitsPerValue, size, pageSize),
      format(format)
{
}

shared_ptr<Mutable> PagedMutable::newMutable(int valueCount, int bitsPerValue)
{
  assert(this->bitsPerValue >= bitsPerValue);
  return PackedInts::getMutable(valueCount, this->bitsPerValue, format);
}

shared_ptr<PagedMutable> PagedMutable::newUnfilledCopy(int64_t newSize)
{
  return make_shared<PagedMutable>(newSize, pageSize(), bitsPerValue, format);
}

int64_t PagedMutable::baseRamBytesUsed()
{
  return AbstractPagedMutable<PagedMutable>::baseRamBytesUsed() +
         RamUsageEstimator::NUM_BYTES_OBJECT_REF;
}
} // namespace org::apache::lucene::util::packed