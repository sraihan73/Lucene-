using namespace std;

#include "BytesRefArray.h"

namespace org::apache::lucene::util
{

BytesRefArray::BytesRefArray(shared_ptr<Counter> bytesUsed)
    : pool(make_shared<ByteBlockPool>(
          make_shared<ByteBlockPool::DirectTrackingAllocator>(bytesUsed))),
      bytesUsed(bytesUsed)
{
  pool->nextBuffer();
  bytesUsed->addAndGet(RamUsageEstimator::NUM_BYTES_ARRAY_HEADER *
                       Integer::BYTES);
}

void BytesRefArray::clear()
{
  lastElement = 0;
  currentOffset = 0;
  // TODO: it's trappy that this does not return storage held by int[] offsets
  // array!
  Arrays::fill(offsets, 0);
  pool->reset(false,
              true); // no need to 0 fill the buffers we control the allocator
}

int BytesRefArray::append(shared_ptr<BytesRef> bytes)
{
  if (lastElement >= offsets.size()) {
    int oldLen = offsets.size();
    offsets = ArrayUtil::grow(offsets, offsets.size() + 1);
    bytesUsed->addAndGet((offsets.size() - oldLen) * Integer::BYTES);
  }
  pool->append(bytes);
  offsets[lastElement++] = currentOffset;
  currentOffset += bytes->length;
  return lastElement - 1;
}

int BytesRefArray::size() { return lastElement; }

shared_ptr<BytesRef> BytesRefArray::get(shared_ptr<BytesRefBuilder> spare,
                                        int index)
{
  FutureObjects::checkIndex(index, lastElement);
  int offset = offsets[index];
  int length = index == lastElement - 1 ? currentOffset - offset
                                        : offsets[index + 1] - offset;
  spare->grow(length);
  spare->setLength(length);
  pool->readBytes(offset, spare->bytes(), 0, spare->length());
  return spare->get();
}

void BytesRefArray::setBytesRef(shared_ptr<BytesRefBuilder> spare,
                                shared_ptr<BytesRef> result, int index)
{
  FutureObjects::checkIndex(index, lastElement);
  int offset = offsets[index];
  int length;
  if (index == lastElement - 1) {
    length = currentOffset - offset;
  } else {
    length = offsets[index + 1] - offset;
  }
  pool->setBytesRef(spare, result, offset, length);
}

std::deque<int>
BytesRefArray::sort(shared_ptr<Comparator<std::shared_ptr<BytesRef>>> comp)
{
  const std::deque<int> orderedEntries = std::deque<int>(size());
  for (int i = 0; i < orderedEntries.size(); i++) {
    orderedEntries[i] = i;
  }
  make_shared<IntroSorterAnonymousInnerClass>(shared_from_this(), comp,
                                              orderedEntries)
      .sort(0, size());
  return orderedEntries;
}

BytesRefArray::IntroSorterAnonymousInnerClass::IntroSorterAnonymousInnerClass(
    shared_ptr<BytesRefArray> outerInstance,
    shared_ptr<Comparator<std::shared_ptr<BytesRef>>> comp,
    deque<int> &orderedEntries)
{
  this->outerInstance = outerInstance;
  this->comp = comp;
  this->orderedEntries = orderedEntries;
  pivot = make_shared<BytesRef>();
  scratchBytes1 = make_shared<BytesRef>();
  scratchBytes2 = make_shared<BytesRef>();
  pivotBuilder = make_shared<BytesRefBuilder>();
  scratch1 = make_shared<BytesRefBuilder>();
  scratch2 = make_shared<BytesRefBuilder>();
}

void BytesRefArray::IntroSorterAnonymousInnerClass::swap(int i, int j)
{
  constexpr int o = orderedEntries[i];
  orderedEntries[i] = orderedEntries[j];
  orderedEntries[j] = o;
}

int BytesRefArray::IntroSorterAnonymousInnerClass::compare(int i, int j)
{
  constexpr int idx1 = orderedEntries[i], idx2 = orderedEntries[j];
  outerInstance->setBytesRef(scratch1, scratchBytes1, idx1);
  outerInstance->setBytesRef(scratch2, scratchBytes2, idx2);
  return comp->compare(scratchBytes1, scratchBytes2);
}

void BytesRefArray::IntroSorterAnonymousInnerClass::setPivot(int i)
{
  constexpr int index = orderedEntries[i];
  outerInstance->setBytesRef(pivotBuilder, pivot, index);
}

int BytesRefArray::IntroSorterAnonymousInnerClass::comparePivot(int j)
{
  constexpr int index = orderedEntries[j];
  outerInstance->setBytesRef(scratch2, scratchBytes2, index);
  return comp->compare(pivot, scratchBytes2);
}

shared_ptr<BytesRefIterator> BytesRefArray::iterator()
{
  return iterator(nullptr);
}

shared_ptr<BytesRefIterator>
BytesRefArray::iterator(shared_ptr<Comparator<std::shared_ptr<BytesRef>>> comp)
{
  shared_ptr<BytesRefBuilder> *const spare = make_shared<BytesRefBuilder>();
  shared_ptr<BytesRef> *const result = make_shared<BytesRef>();
  constexpr int size = this->size();
  const std::deque<int> indices = comp == nullptr ? nullptr : sort(comp);
  return make_shared<BytesRefIteratorAnonymousInnerClass>(
      shared_from_this(), spare, result, size, indices);
}

BytesRefArray::BytesRefIteratorAnonymousInnerClass::
    BytesRefIteratorAnonymousInnerClass(
        shared_ptr<BytesRefArray> outerInstance,
        shared_ptr<org::apache::lucene::util::BytesRefBuilder> spare,
        shared_ptr<org::apache::lucene::util::BytesRef> result, int size,
        deque<int> &indices)
{
  this->outerInstance = outerInstance;
  this->spare = spare;
  this->result = result;
  this->size = size;
  this->indices = indices;
  pos = 0;
}

shared_ptr<BytesRef> BytesRefArray::BytesRefIteratorAnonymousInnerClass::next()
{
  if (pos < size) {
    outerInstance->setBytesRef(spare, result,
                               indices.empty() ? pos++ : indices[pos++]);
    return result;
  }
  return nullptr;
}
} // namespace org::apache::lucene::util