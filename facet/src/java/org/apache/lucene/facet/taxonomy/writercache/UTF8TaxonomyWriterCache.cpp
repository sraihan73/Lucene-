using namespace std;

#include "UTF8TaxonomyWriterCache.h"

namespace org::apache::lucene::facet::taxonomy::writercache
{
using FacetLabel = org::apache::lucene::facet::taxonomy::FacetLabel;
using TaxonomyWriterCache =
    org::apache::lucene::facet::taxonomy::writercache::TaxonomyWriterCache;
using Accountable = org::apache::lucene::util::Accountable;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using DirectTrackingAllocator =
    org::apache::lucene::util::ByteBlockPool::DirectTrackingAllocator;
using ByteBlockPool = org::apache::lucene::util::ByteBlockPool;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using BytesRefHash = org::apache::lucene::util::BytesRefHash;
using Counter = org::apache::lucene::util::Counter;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using UnicodeUtil = org::apache::lucene::util::UnicodeUtil;

UTF8TaxonomyWriterCache::ThreadLocalAnonymousInnerClass::
    ThreadLocalAnonymousInnerClass()
{
}

shared_ptr<BytesRefBuilder>
UTF8TaxonomyWriterCache::ThreadLocalAnonymousInnerClass::initialValue()
{
  return make_shared<BytesRefBuilder>();
}

UTF8TaxonomyWriterCache::UTF8TaxonomyWriterCache()
{
  ordinals = std::deque<std::deque<int>>(1);
  ordinals[0] = std::deque<int>(ORDINALS_PAGE_SIZE);
}

int UTF8TaxonomyWriterCache::get(shared_ptr<FacetLabel> label)
{
  shared_ptr<BytesRef> bytes = toBytes(label);
  int id;
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this()) { id = map_obj->find(bytes); }
  if (id == -1) {
    return LabelToOrdinal::INVALID_ORDINAL;
  }
  int page = id / ORDINALS_PAGE_SIZE;
  int offset = id % ORDINALS_PAGE_MASK;
  return ordinals[page][offset];
}

bool UTF8TaxonomyWriterCache::assertSameOrdinal(shared_ptr<FacetLabel> label,
                                                int id, int ord)
{
  id = -id - 1;
  int page = id / ORDINALS_PAGE_SIZE;
  int offset = id % ORDINALS_PAGE_MASK;
  int oldOrd = ordinals[page][offset];
  if (oldOrd != ord) {
    throw invalid_argument(
        L"label " + label + L" was already cached, with old ord=" +
        to_wstring(oldOrd) + L" versus new ord=" + to_wstring(ord));
  }
  return true;
}

bool UTF8TaxonomyWriterCache::put(shared_ptr<FacetLabel> label, int ord)
{
  shared_ptr<BytesRef> bytes = toBytes(label);
  int id;
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    id = map_obj->add(bytes);
    if (id < 0) {
      assert((assertSameOrdinal(label, id, ord)));
      return false;
    }
    assert(id == count);
    int page = id / ORDINALS_PAGE_SIZE;
    int offset = id % ORDINALS_PAGE_MASK;
    if (page == pageCount) {
      if (page == ordinals.size()) {
        std::deque<std::deque<int>> newOrdinals(ArrayUtil::oversize(
            page + 1, RamUsageEstimator::NUM_BYTES_OBJECT_REF));
        System::arraycopy(ordinals, 0, newOrdinals, 0, ordinals.size());
        ordinals = newOrdinals;
      }
      ordinals[page] = std::deque<int>(ORDINALS_PAGE_MASK);
      pageCount++;
    }
    ordinals[page][offset] = ord;
    count++;

    // we never prune from the cache
    return false;
  }
}

bool UTF8TaxonomyWriterCache::isFull()
{
  // we are never full
  return false;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void UTF8TaxonomyWriterCache::clear()
{
  map_obj->clear();
  map_obj->reinit();
  ordinals = std::deque<std::deque<int>>(1);
  ordinals[0] = std::deque<int>(ORDINALS_PAGE_SIZE);
  count = 0;
  pageCount = 0;
  assert(bytesUsed->get() == 0);
}

int UTF8TaxonomyWriterCache::size() { return count; }

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t UTF8TaxonomyWriterCache::ramBytesUsed()
{
  return bytesUsed->get() +
         pageCount * ORDINALS_PAGE_SIZE * RamUsageEstimator::NUM_BYTES_INT;
}

UTF8TaxonomyWriterCache::~UTF8TaxonomyWriterCache() {}

shared_ptr<BytesRef>
UTF8TaxonomyWriterCache::toBytes(shared_ptr<FacetLabel> label)
{
  shared_ptr<BytesRefBuilder> bytes = this->bytes->get();
  bytes->clear();
  for (int i = 0; i < label->length; i++) {
    wstring part = label->components[i];
    if (i > 0) {
      bytes->append(DELIM_CHAR);
    }
    bytes->grow(bytes->length() + UnicodeUtil::maxUTF8Length(part.length()));
    bytes->setLength(UnicodeUtil::UTF16toUTF8(part, 0, part.length(),
                                              bytes->bytes(), bytes->length()));
  }
  return bytes->get();
}
} // namespace org::apache::lucene::facet::taxonomy::writercache