using namespace std;

#include "FixedLengthBytesRefArray.h"

namespace org::apache::lucene::util
{

FixedLengthBytesRefArray::FixedLengthBytesRefArray(int valueLength)
    : valueLength(valueLength), valuesPerBlock(max(1, 32768 / valueLength))
{

  // ~32K per page, unless each value is > 32K:
  nextEntry = valuesPerBlock;
  blocks = std::deque<std::deque<char>>(0);
}

void FixedLengthBytesRefArray::clear()
{
  size_ = 0;
  blocks = std::deque<std::deque<char>>(0);
  currentBlock = -1;
  nextEntry = valuesPerBlock;
}

int FixedLengthBytesRefArray::append(shared_ptr<BytesRef> bytes)
{
  if (bytes->length != valueLength) {
    throw invalid_argument(L"value length is " + to_wstring(bytes->length) +
                           L" but is supposed to always be " +
                           to_wstring(valueLength));
  }
  if (nextEntry == valuesPerBlock) {
    currentBlock++;
    if (currentBlock == blocks.size()) {
      int size = ArrayUtil::oversize(currentBlock + 1,
                                     RamUsageEstimator::NUM_BYTES_OBJECT_REF);
      std::deque<std::deque<char>> next(size);
      System::arraycopy(blocks, 0, next, 0, blocks.size());
      blocks = next;
    }
    blocks[currentBlock] = std::deque<char>(valuesPerBlock * valueLength);
    nextEntry = 0;
  }

  System::arraycopy(bytes->bytes, bytes->offset, blocks[currentBlock],
                    nextEntry * valueLength, valueLength);
  nextEntry++;

  return size_++;
}

int FixedLengthBytesRefArray::size() { return size_; }

std::deque<int> FixedLengthBytesRefArray::sort(
    shared_ptr<Comparator<std::shared_ptr<BytesRef>>> comp)
{
  const std::deque<int> orderedEntries = std::deque<int>(size());
  for (int i = 0; i < orderedEntries.size(); i++) {
    orderedEntries[i] = i;
  }

  if (std::dynamic_pointer_cast<BytesRefComparator>(comp) != nullptr) {
    shared_ptr<BytesRefComparator> bComp =
        std::static_pointer_cast<BytesRefComparator>(comp);
    make_shared<MSBRadixSorterAnonymousInnerClass>(
        shared_from_this(), bComp->comparedBytesCount, orderedEntries, bComp)
        .sort(0, size());
    return orderedEntries;
  }

  shared_ptr<BytesRef> *const pivot = make_shared<BytesRef>();
  shared_ptr<BytesRef> *const scratch1 = make_shared<BytesRef>();
  shared_ptr<BytesRef> *const scratch2 = make_shared<BytesRef>();
  pivot->length = valueLength;
  scratch1->length = valueLength;
  scratch2->length = valueLength;

  make_shared<IntroSorterAnonymousInnerClass>(
      shared_from_this(), comp, orderedEntries, pivot, scratch1, scratch2)
      .sort(0, size());
  return orderedEntries;
}

FixedLengthBytesRefArray::MSBRadixSorterAnonymousInnerClass::
    MSBRadixSorterAnonymousInnerClass(
        shared_ptr<FixedLengthBytesRefArray> outerInstance,
        int comparedBytesCount, deque<int> &orderedEntries,
        shared_ptr<org::apache::lucene::util::BytesRefComparator> bComp)
    : MSBRadixSorter(comparedBytesCount)
{
  this->outerInstance = outerInstance;
  this->orderedEntries = orderedEntries;
  this->bComp = bComp;

  scratch = make_shared<BytesRef>();
  scratch->length = outerInstance->valueLength;
}

void FixedLengthBytesRefArray::MSBRadixSorterAnonymousInnerClass::swap(int i,
                                                                       int j)
{
  int o = orderedEntries[i];
  orderedEntries[i] = orderedEntries[j];
  orderedEntries[j] = o;
}

int FixedLengthBytesRefArray::MSBRadixSorterAnonymousInnerClass::byteAt(int i,
                                                                        int k)
{
  int index1 = orderedEntries[i];
  scratch->bytes =
      outerInstance->blocks[index1 / outerInstance->valuesPerBlock];
  scratch->offset =
      (index1 % outerInstance->valuesPerBlock) * outerInstance->valueLength;
  return bComp->byteAt(scratch, k);
}

FixedLengthBytesRefArray::IntroSorterAnonymousInnerClass::
    IntroSorterAnonymousInnerClass(
        shared_ptr<FixedLengthBytesRefArray> outerInstance,
        shared_ptr<Comparator<std::shared_ptr<BytesRef>>> comp,
        deque<int> &orderedEntries,
        shared_ptr<org::apache::lucene::util::BytesRef> pivot,
        shared_ptr<org::apache::lucene::util::BytesRef> scratch1,
        shared_ptr<org::apache::lucene::util::BytesRef> scratch2)
{
  this->outerInstance = outerInstance;
  this->comp = comp;
  this->orderedEntries = orderedEntries;
  this->pivot = pivot;
  this->scratch1 = scratch1;
  this->scratch2 = scratch2;
}

void FixedLengthBytesRefArray::IntroSorterAnonymousInnerClass::swap(int i,
                                                                    int j)
{
  int o = orderedEntries[i];
  orderedEntries[i] = orderedEntries[j];
  orderedEntries[j] = o;
}

int FixedLengthBytesRefArray::IntroSorterAnonymousInnerClass::compare(int i,
                                                                      int j)
{
  int index1 = orderedEntries[i];
  scratch1->bytes =
      outerInstance->blocks[index1 / outerInstance->valuesPerBlock];
  scratch1->offset =
      (index1 % outerInstance->valuesPerBlock) * outerInstance->valueLength;

  int index2 = orderedEntries[j];
  scratch2->bytes =
      outerInstance->blocks[index2 / outerInstance->valuesPerBlock];
  scratch2->offset =
      (index2 % outerInstance->valuesPerBlock) * outerInstance->valueLength;

  return comp->compare(scratch1, scratch2);
}

void FixedLengthBytesRefArray::IntroSorterAnonymousInnerClass::setPivot(int i)
{
  int index = orderedEntries[i];
  pivot->bytes = outerInstance->blocks[index / outerInstance->valuesPerBlock];
  pivot->offset =
      (index % outerInstance->valuesPerBlock) * outerInstance->valueLength;
}

int FixedLengthBytesRefArray::IntroSorterAnonymousInnerClass::comparePivot(
    int j)
{
  constexpr int index = orderedEntries[j];
  scratch2->bytes =
      outerInstance->blocks[index / outerInstance->valuesPerBlock];
  scratch2->offset =
      (index % outerInstance->valuesPerBlock) * outerInstance->valueLength;
  return comp->compare(pivot, scratch2);
}

shared_ptr<BytesRefIterator> FixedLengthBytesRefArray::iterator(
    shared_ptr<Comparator<std::shared_ptr<BytesRef>>> comp)
{
  shared_ptr<BytesRef> *const result = make_shared<BytesRef>();
  result->length = valueLength;
  constexpr int size = this->size();
  const std::deque<int> indices = sort(comp);
  return make_shared<BytesRefIteratorAnonymousInnerClass>(
      shared_from_this(), result, size, indices);
}

FixedLengthBytesRefArray::BytesRefIteratorAnonymousInnerClass::
    BytesRefIteratorAnonymousInnerClass(
        shared_ptr<FixedLengthBytesRefArray> outerInstance,
        shared_ptr<org::apache::lucene::util::BytesRef> result, int size,
        deque<int> &indices)
{
  this->outerInstance = outerInstance;
  this->result = result;
  this->size = size;
  this->indices = indices;
  pos = 0;
}

shared_ptr<BytesRef>
FixedLengthBytesRefArray::BytesRefIteratorAnonymousInnerClass::next()
{
  if (pos < size) {
    int index = indices[pos];
    pos++;
    result->bytes =
        outerInstance->blocks[index / outerInstance->valuesPerBlock];
    result->offset =
        (index % outerInstance->valuesPerBlock) * outerInstance->valueLength;
    return result;
  }
  return nullptr;
}
} // namespace org::apache::lucene::util