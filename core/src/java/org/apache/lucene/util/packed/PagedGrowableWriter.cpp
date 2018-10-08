using namespace std;

#include "PagedGrowableWriter.h"

namespace org::apache::lucene::util::packed
{
using Mutable = org::apache::lucene::util::packed::PackedInts::Mutable;

PagedGrowableWriter::PagedGrowableWriter(int64_t size, int pageSize,
                                         int startBitsPerValue,
                                         float acceptableOverheadRatio)
    : PagedGrowableWriter(size, pageSize, startBitsPerValue,
                          acceptableOverheadRatio, true)
{
}

PagedGrowableWriter::PagedGrowableWriter(int64_t size, int pageSize,
                                         int startBitsPerValue,
                                         float acceptableOverheadRatio,
                                         bool fillPages)
    : AbstractPagedMutable<PagedGrowableWriter>(startBitsPerValue, size,
                                                pageSize),
      acceptableOverheadRatio(acceptableOverheadRatio)
{
  if (fillPages) {
    this->fillPages();
  }
}

shared_ptr<Mutable> PagedGrowableWriter::newMutable(int valueCount,
                                                    int bitsPerValue)
{
  return make_shared<GrowableWriter>(bitsPerValue, valueCount,
                                     acceptableOverheadRatio);
}

shared_ptr<PagedGrowableWriter>
PagedGrowableWriter::newUnfilledCopy(int64_t newSize)
{
  return make_shared<PagedGrowableWriter>(newSize, pageSize(), bitsPerValue,
                                          acceptableOverheadRatio, false);
}

int64_t PagedGrowableWriter::baseRamBytesUsed()
{
  return AbstractPagedMutable<PagedGrowableWriter>::baseRamBytesUsed() +
         Float::BYTES;
}
} // namespace org::apache::lucene::util::packed