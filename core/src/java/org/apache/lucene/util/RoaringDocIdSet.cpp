using namespace std;

#include "RoaringDocIdSet.h"

namespace org::apache::lucene::util
{
using DocIdSet = org::apache::lucene::search::DocIdSet;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;

RoaringDocIdSet::Builder::Builder(int maxDoc)
    : maxDoc(maxDoc),
      sets(std::deque<std::shared_ptr<DocIdSet>>(static_cast<int>(
          static_cast<unsigned int>((maxDoc + (1 << 16) - 1)) >> 16))),
      buffer(std::deque<short>(MAX_ARRAY_LENGTH))
{
  lastDocId = -1;
  currentBlock = -1;
}

void RoaringDocIdSet::Builder::flush()
{
  assert(currentBlockCardinality <= BLOCK_SIZE);
  if (currentBlockCardinality <= MAX_ARRAY_LENGTH) {
    // Use sparse encoding
    assert(denseBuffer == nullptr);
    if (currentBlockCardinality > 0) {
      sets[currentBlock] = make_shared<ShortArrayDocIdSet>(
          Arrays::copyOf(buffer, currentBlockCardinality));
    }
  } else {
    assert(denseBuffer != nullptr);
    assert(denseBuffer->cardinality() == currentBlockCardinality);
    if (denseBuffer->length() == BLOCK_SIZE &&
        BLOCK_SIZE - currentBlockCardinality < MAX_ARRAY_LENGTH) {
      // Doc ids are very dense, inverse the encoding
      const std::deque<short> excludedDocs =
          std::deque<short>(BLOCK_SIZE - currentBlockCardinality);
      denseBuffer->flip(0, denseBuffer->length());
      int excludedDoc = -1;
      for (int i = 0; i < excludedDocs.size(); ++i) {
        excludedDoc = denseBuffer->nextSetBit(excludedDoc + 1);
        assert(excludedDoc != DocIdSetIterator::NO_MORE_DOCS);
        excludedDocs[i] = static_cast<short>(excludedDoc);
      }
      assert(excludedDoc + 1 == denseBuffer->length() ||
             denseBuffer->nextSetBit(excludedDoc + 1) ==
                 DocIdSetIterator::NO_MORE_DOCS);
      sets[currentBlock] = make_shared<NotDocIdSet>(
          BLOCK_SIZE, make_shared<ShortArrayDocIdSet>(excludedDocs));
    } else {
      // Neither sparse nor super dense, use a fixed bit set
      sets[currentBlock] =
          make_shared<BitDocIdSet>(denseBuffer, currentBlockCardinality);
    }
    denseBuffer.reset();
  }

  cardinality += currentBlockCardinality;
  denseBuffer.reset();
  currentBlockCardinality = 0;
}

shared_ptr<Builder> RoaringDocIdSet::Builder::add(int docId)
{
  if (docId <= lastDocId) {
    throw invalid_argument(L"Doc ids must be added in-order, got " +
                           to_wstring(docId) + L" which is <= lastDocID=" +
                           to_wstring(lastDocId));
  }
  constexpr int block =
      static_cast<int>(static_cast<unsigned int>(docId) >> 16);
  if (block != currentBlock) {
    // we went to a different block, let's flush what we buffered and start from
    // fresh
    flush();
    currentBlock = block;
  }

  if (currentBlockCardinality < MAX_ARRAY_LENGTH) {
    buffer[currentBlockCardinality] = static_cast<short>(docId);
  } else {
    if (denseBuffer == nullptr) {
      // the buffer is full, let's move to a fixed bit set
      constexpr int numBits = min(1 << 16, maxDoc - (block << 16));
      denseBuffer = make_shared<FixedBitSet>(numBits);
      for (auto doc : buffer) {
        denseBuffer->set(doc & 0xFFFF);
      }
    }
    denseBuffer->set(docId & 0xFFFF);
  }

  lastDocId = docId;
  currentBlockCardinality += 1;
  return shared_from_this();
}

shared_ptr<Builder> RoaringDocIdSet::Builder::add(
    shared_ptr<DocIdSetIterator> disi) 
{
  for (int doc = disi->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
       doc = disi->nextDoc()) {
    add(doc);
  }
  return shared_from_this();
}

shared_ptr<RoaringDocIdSet> RoaringDocIdSet::Builder::build()
{
  flush();
  return make_shared<RoaringDocIdSet>(sets, cardinality);
}

RoaringDocIdSet::ShortArrayDocIdSet::ShortArrayDocIdSet(
    std::deque<short> &docIDs)
    : docIDs(docIDs)
{
}

int64_t RoaringDocIdSet::ShortArrayDocIdSet::ramBytesUsed()
{
  return BASE_RAM_BYTES_USED + RamUsageEstimator::sizeOf(docIDs);
}

shared_ptr<DocIdSetIterator>
RoaringDocIdSet::ShortArrayDocIdSet::iterator() 
{
  return make_shared<DocIdSetIteratorAnonymousInnerClass>(shared_from_this());
}

RoaringDocIdSet::ShortArrayDocIdSet::DocIdSetIteratorAnonymousInnerClass::
    DocIdSetIteratorAnonymousInnerClass(
        shared_ptr<ShortArrayDocIdSet> outerInstance)
{
  this->outerInstance = outerInstance;
  i = -1;
  doc = -1;
}

int RoaringDocIdSet::ShortArrayDocIdSet::DocIdSetIteratorAnonymousInnerClass::
    docId(int i)
{
  return outerInstance->docIDs[i] & 0xFFFF;
}

int RoaringDocIdSet::ShortArrayDocIdSet::DocIdSetIteratorAnonymousInnerClass::
    nextDoc() 
{
  if (++i >= outerInstance->docIDs.size()) {
    return doc = DocIdSetIterator::NO_MORE_DOCS;
  }
  return doc = docId(i);
}

int RoaringDocIdSet::ShortArrayDocIdSet::DocIdSetIteratorAnonymousInnerClass::
    docID()
{
  return doc;
}

int64_t
RoaringDocIdSet::ShortArrayDocIdSet::DocIdSetIteratorAnonymousInnerClass::cost()
{
  return outerInstance->docIDs.size();
}

int RoaringDocIdSet::ShortArrayDocIdSet::DocIdSetIteratorAnonymousInnerClass::
    advance(int target) 
{
  // binary search
  int lo = i + 1;
  int hi = outerInstance->docIDs.size() - 1;
  while (lo <= hi) {
    constexpr int mid =
        static_cast<int>(static_cast<unsigned int>((lo + hi)) >> 1);
    constexpr int midDoc = docId(mid);
    if (midDoc < target) {
      lo = mid + 1;
    } else {
      hi = mid - 1;
    }
  }
  if (lo == outerInstance->docIDs.size()) {
    i = outerInstance->docIDs.size();
    return doc = DocIdSetIterator::NO_MORE_DOCS;
  } else {
    i = lo;
    return doc = docId(i);
  }
}

RoaringDocIdSet::RoaringDocIdSet(
    std::deque<std::shared_ptr<DocIdSet>> &docIdSets, int cardinality)
    : docIdSets(docIdSets), cardinality(cardinality),
      ramBytesUsed(ramBytesUsed_)
{
  int64_t ramBytesUsed =
      BASE_RAM_BYTES_USED + RamUsageEstimator::shallowSizeOf(docIdSets);
  for (auto set : this->docIdSets) {
    if (set != nullptr) {
      ramBytesUsed += set->ramBytesUsed();
    }
  }
}

int64_t RoaringDocIdSet::ramBytesUsed() { return ramBytesUsed_; }

shared_ptr<DocIdSetIterator> RoaringDocIdSet::iterator() 
{
  if (cardinality_ == 0) {
    return nullptr;
  }
  return make_shared<Iterator>(shared_from_this());
}

RoaringDocIdSet::Iterator::Iterator(
    shared_ptr<RoaringDocIdSet> outerInstance) 
    : outerInstance(outerInstance)
{
  doc = -1;
  block = -1;
  sub = DocIdSetIterator::empty();
}

int RoaringDocIdSet::Iterator::docID() { return doc; }

int RoaringDocIdSet::Iterator::nextDoc() 
{
  constexpr int subNext = sub->nextDoc();
  if (subNext == NO_MORE_DOCS) {
    return firstDocFromNextBlock();
  }
  return doc = (block << 16) | subNext;
}

int RoaringDocIdSet::Iterator::advance(int target) 
{
  constexpr int targetBlock =
      static_cast<int>(static_cast<unsigned int>(target) >> 16);
  if (targetBlock != block) {
    block = targetBlock;
    if (block >= outerInstance->docIdSets.size()) {
      sub.reset();
      return doc = NO_MORE_DOCS;
    }
    if (outerInstance->docIdSets[block] == nullptr) {
      return firstDocFromNextBlock();
    }
    sub = outerInstance->docIdSets[block]->begin();
  }
  constexpr int subNext = sub->advance(target & 0xFFFF);
  if (subNext == NO_MORE_DOCS) {
    return firstDocFromNextBlock();
  }
  return doc = (block << 16) | subNext;
}

int RoaringDocIdSet::Iterator::firstDocFromNextBlock() 
{
  while (true) {
    block += 1;
    if (block >= outerInstance->docIdSets.size()) {
      sub.reset();
      return doc = NO_MORE_DOCS;
    } else if (outerInstance->docIdSets[block] != nullptr) {
      sub = outerInstance->docIdSets[block]->begin();
      constexpr int subNext = sub->nextDoc();
      assert(subNext != NO_MORE_DOCS);
      return doc = (block << 16) | subNext;
    }
  }
}

int64_t RoaringDocIdSet::Iterator::cost()
{
  return outerInstance->cardinality_;
}

int RoaringDocIdSet::cardinality() { return cardinality_; }

wstring RoaringDocIdSet::toString()
{
  return L"RoaringDocIdSet(cardinality=" + to_wstring(cardinality_) + L")";
}
} // namespace org::apache::lucene::util