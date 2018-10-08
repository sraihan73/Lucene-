using namespace std;

#include "DocIdSetBuilder.h"

namespace org::apache::lucene::util
{
using PointValues = org::apache::lucene::index::PointValues;
using Terms = org::apache::lucene::index::Terms;
using DocIdSet = org::apache::lucene::search::DocIdSet;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using PackedInts = org::apache::lucene::util::packed::PackedInts;

DocIdSetBuilder::FixedBitSetAdder::FixedBitSetAdder(
    shared_ptr<FixedBitSet> bitSet)
    : bitSet(bitSet)
{
}

void DocIdSetBuilder::FixedBitSetAdder::add(int doc) { bitSet->set(doc); }

DocIdSetBuilder::Buffer::Buffer(int length)
{
  this->array_ = std::deque<int>(length);
  this->length = 0;
}

DocIdSetBuilder::Buffer::Buffer(std::deque<int> &array_, int length)
{
  this->array_ = array_;
  this->length = length;
}

DocIdSetBuilder::BufferAdder::BufferAdder(shared_ptr<Buffer> buffer)
    : buffer(buffer)
{
}

void DocIdSetBuilder::BufferAdder::add(int doc)
{
  buffer->array_[buffer->length++] = doc;
}

DocIdSetBuilder::DocIdSetBuilder(int maxDoc) : DocIdSetBuilder(maxDoc, -1, -1)
{
}

DocIdSetBuilder::DocIdSetBuilder(int maxDoc,
                                 shared_ptr<Terms> terms) 
    : DocIdSetBuilder(maxDoc, terms->getDocCount(), terms->getSumDocFreq())
{
}

DocIdSetBuilder::DocIdSetBuilder(int maxDoc, shared_ptr<PointValues> values,
                                 const wstring &field) 
    : DocIdSetBuilder(maxDoc, values->getDocCount(), values->size())
{
}

DocIdSetBuilder::DocIdSetBuilder(int maxDoc, int docCount, int64_t valueCount)
    : maxDoc(maxDoc),
      threshold(static_cast<int>(static_cast<unsigned int>(maxDoc) >> 7)),
      multivalued(docCount < 0 || docCount != valueCount)
{
  if (docCount <= 0 || valueCount < 0) {
    // assume one value per doc, this means the cost will be overestimated
    // if the docs are actually multi-valued
    this->numValuesPerDoc = 1;
  } else {
    // otherwise compute from index stats
    this->numValuesPerDoc = static_cast<double>(valueCount) / docCount;
  }

  assert((numValuesPerDoc >= 1, L"valueCount=" + to_wstring(valueCount) +
                                    L" docCount=" + to_wstring(docCount)));

  // For ridiculously small sets, we'll just use a sorted int[]
  // maxDoc >>> 7 is a good value if you want to save memory, lower values
  // such as maxDoc >>> 11 should provide faster building but at the expense
  // of using a full bitset even for quite sparse data

  this->bitSet.reset();
}

void DocIdSetBuilder::add(shared_ptr<DocIdSetIterator> iter) 
{
  if (bitSet != nullptr) {
    bitSet->or (iter);
    return;
  }
  int cost = static_cast<int>(min(numeric_limits<int>::max(), iter->cost()));
  shared_ptr<BulkAdder> adder = grow(cost);
  for (int i = 0; i < cost; ++i) {
    int doc = iter->nextDoc();
    if (doc == DocIdSetIterator::NO_MORE_DOCS) {
      return;
    }
    adder->add(doc);
  }
  for (int doc = iter->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
       doc = iter->nextDoc()) {
    grow(1)->add(doc);
  }
}

shared_ptr<BulkAdder> DocIdSetBuilder::grow(int numDocs)
{
  if (bitSet == nullptr) {
    if (static_cast<int64_t>(totalAllocated) + numDocs <= threshold) {
      ensureBufferCapacity(numDocs);
    } else {
      upgradeToBitSet();
      counter += numDocs;
    }
  } else {
    counter += numDocs;
  }
  return adder;
}

void DocIdSetBuilder::ensureBufferCapacity(int numDocs)
{
  if (buffers.empty()) {
    addBuffer(additionalCapacity(numDocs));
    return;
  }

  shared_ptr<Buffer> current = buffers[buffers.size() - 1];
  if (current->array_.size() - current->length >= numDocs) {
    // current buffer is large enough
    return;
  }
  if (current->length <
      current->array_.size() -
          (static_cast<int>(static_cast<unsigned int>(current->array_.size()) >>
                            3))) {
    // current buffer is less than 7/8 full, resize rather than waste space
    growBuffer(current, additionalCapacity(numDocs));
  } else {
    addBuffer(additionalCapacity(numDocs));
  }
}

int DocIdSetBuilder::additionalCapacity(int numDocs)
{
  // exponential growth: the new array has a size equal to the sum of what
  // has been allocated so far
  int c = totalAllocated;
  // but is also >= numDocs + 1 so that we can store the next batch of docs
  // (plus an empty slot so that we are more likely to reuse the array in
  // build())
  c = max(numDocs + 1, c);
  // avoid cold starts
  c = max(32, c);
  // do not go beyond the threshold
  c = min(threshold - totalAllocated, c);
  return c;
}

shared_ptr<Buffer> DocIdSetBuilder::addBuffer(int len)
{
  shared_ptr<Buffer> buffer = make_shared<Buffer>(len);
  buffers.push_back(buffer);
  adder = make_shared<BufferAdder>(buffer);
  totalAllocated += buffer->array_.size();
  return buffer;
}

void DocIdSetBuilder::growBuffer(shared_ptr<Buffer> buffer,
                                 int additionalCapacity)
{
  buffer->array_ = Arrays::copyOf(buffer->array_,
                                  buffer->array_.size() + additionalCapacity);
  totalAllocated += additionalCapacity;
}

void DocIdSetBuilder::upgradeToBitSet()
{
  assert(bitSet == nullptr);
  shared_ptr<FixedBitSet> bitSet = make_shared<FixedBitSet>(maxDoc);
  int64_t counter = 0;
  for (auto buffer : buffers) {
    std::deque<int> array_ = buffer->array_;
    int length = buffer->length;
    counter += length;
    for (int i = 0; i < length; ++i) {
      bitSet->set(array_[i]);
    }
  }
  this->bitSet = bitSet;
  this->counter = counter;
  this->buffers.clear();
  this->adder = make_shared<FixedBitSetAdder>(bitSet);
}

shared_ptr<DocIdSet> DocIdSetBuilder::build()
{
  try {
    if (bitSet != nullptr) {
      assert(counter >= 0);
      constexpr int64_t cost = round(counter / numValuesPerDoc);
      return make_shared<BitDocIdSet>(bitSet, cost);
    } else {
      shared_ptr<Buffer> concatenated = concat(buffers);
      shared_ptr<LSBRadixSorter> sorter = make_shared<LSBRadixSorter>();
      sorter->sort(PackedInts::bitsRequired(maxDoc - 1), concatenated->array_,
                   concatenated->length);
      constexpr int l;
      if (multivalued) {
        l = dedup(concatenated->array_, concatenated->length);
      } else {
        assert((noDups(concatenated->array_, concatenated->length)));
        l = concatenated->length;
      }
      assert(l <= concatenated->length);
      concatenated->array_[l] = DocIdSetIterator::NO_MORE_DOCS;
      return make_shared<IntArrayDocIdSet>(concatenated->array_, l);
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    this->buffers.clear();
    this->bitSet.reset();
  }
}

shared_ptr<Buffer>
DocIdSetBuilder::concat(deque<std::shared_ptr<Buffer>> &buffers)
{
  int totalLength = 0;
  shared_ptr<Buffer> largestBuffer = nullptr;
  for (auto buffer : buffers) {
    totalLength += buffer->length;
    if (largestBuffer == nullptr ||
        buffer->array_.size() > largestBuffer->array_.size()) {
      largestBuffer = buffer;
    }
  }
  if (largestBuffer == nullptr) {
    return make_shared<Buffer>(1);
  }
  std::deque<int> docs = largestBuffer->array_;
  if (docs.size() < totalLength + 1) {
    docs = Arrays::copyOf(docs, totalLength + 1);
  }
  totalLength = largestBuffer->length;
  for (auto buffer : buffers) {
    if (buffer != largestBuffer) {
      System::arraycopy(buffer->array_, 0, docs, totalLength, buffer->length);
      totalLength += buffer->length;
    }
  }
  return make_shared<Buffer>(docs, totalLength);
}

int DocIdSetBuilder::dedup(std::deque<int> &arr, int length)
{
  if (length == 0) {
    return 0;
  }
  int l = 1;
  int previous = arr[0];
  for (int i = 1; i < length; ++i) {
    constexpr int value = arr[i];
    assert(value >= previous);
    if (value != previous) {
      arr[l++] = value;
      previous = value;
    }
  }
  return l;
}

bool DocIdSetBuilder::noDups(std::deque<int> &a, int len)
{
  for (int i = 1; i < len; ++i) {
    assert(a[i - 1] < a[i]);
  }
  return true;
}
} // namespace org::apache::lucene::util