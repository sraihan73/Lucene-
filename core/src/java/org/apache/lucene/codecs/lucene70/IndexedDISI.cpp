using namespace std;

#include "IndexedDISI.h"
#include "../../store/IndexInput.h"
#include "../../store/IndexOutput.h"
#include "../../util/BitSetIterator.h"
#include "../../util/FixedBitSet.h"

namespace org::apache::lucene::codecs::lucene70
{
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using BitSetIterator = org::apache::lucene::util::BitSetIterator;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using RoaringDocIdSet = org::apache::lucene::util::RoaringDocIdSet;

void IndexedDISI::flush(int block, shared_ptr<FixedBitSet> buffer,
                        int cardinality,
                        shared_ptr<IndexOutput> out) 
{
  assert(block >= 0 && block < 65536);
  out->writeShort(static_cast<short>(block));
  assert(cardinality > 0 && cardinality <= 65536);
  out->writeShort(static_cast<short>(cardinality - 1));
  if (cardinality > MAX_ARRAY_LENGTH) {
    if (cardinality != 65536) { // all docs are set
      for (auto word : buffer->getBits()) {
        out->writeLong(word);
      }
    }
  } else {
    shared_ptr<BitSetIterator> it =
        make_shared<BitSetIterator>(buffer, cardinality);
    for (int doc = it->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
         doc = it->nextDoc()) {
      out->writeShort(static_cast<short>(doc));
    }
  }
}

void IndexedDISI::writeBitSet(shared_ptr<DocIdSetIterator> it,
                              shared_ptr<IndexOutput> out) 
{
  int i = 0;
  shared_ptr<FixedBitSet> *const buffer = make_shared<FixedBitSet>(1 << 16);
  int prevBlock = -1;
  for (int doc = it->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
       doc = it->nextDoc()) {
    constexpr int block =
        static_cast<int>(static_cast<unsigned int>(doc) >> 16);
    if (prevBlock != -1 && block != prevBlock) {
      flush(prevBlock, buffer, i, out);
      buffer->clear(0, buffer->length());
      prevBlock = block;
      i = 0;
    }
    buffer->set(doc & 0xFFFF);
    i++;
    prevBlock = block;
  }
  if (i > 0) {
    flush(prevBlock, buffer, i, out);
    buffer->clear(0, buffer->length());
  }
  // NO_MORE_DOCS is stored explicitly
  buffer->set(DocIdSetIterator::NO_MORE_DOCS & 0xFFFF);
  flush(static_cast<int>(
            static_cast<unsigned int>(DocIdSetIterator::NO_MORE_DOCS) >> 16),
        buffer, 1, out);
}

IndexedDISI::IndexedDISI(shared_ptr<IndexInput> in_, int64_t offset,
                         int64_t length, int64_t cost) 
    : slice(in_->slice(L"docs", offset, length)), cost(cost)
{
}

int IndexedDISI::docID() { return doc; }

int IndexedDISI::advance(int target) 
{
  constexpr int targetBlock = target & 0xFFFF0000;
  if (block < targetBlock) {
    advanceBlock(targetBlock);
  }
  if (block == targetBlock) {
    if (method.advanceWithinBlock(shared_from_this(), target)) {
      return doc;
    }
    readBlockHeader();
  }
  bool found = method.advanceWithinBlock(shared_from_this(), block);
  assert(found);
  return doc;
}

bool IndexedDISI::advanceExact(int target) 
{
  constexpr int targetBlock = target & 0xFFFF0000;
  if (block < targetBlock) {
    advanceBlock(targetBlock);
  }
  bool found = block == targetBlock &&
               method.advanceExactWithinBlock(shared_from_this(), target);
  this->doc = target;
  return found;
}

void IndexedDISI::advanceBlock(int targetBlock) 
{
  do {
    slice->seek(blockEnd);
    readBlockHeader();
  } while (block < targetBlock);
}

void IndexedDISI::readBlockHeader() 
{
  block = Short::toUnsignedInt(slice->readShort()) << 16;
  assert(block >= 0);
  constexpr int numValues = 1 + Short::toUnsignedInt(slice->readShort());
  index_ = nextBlockIndex;
  nextBlockIndex = index_ + numValues;
  if (numValues <= MAX_ARRAY_LENGTH) {
    method = Method::SPARSE;
    blockEnd = slice->getFilePointer() + (numValues << 1);
  } else if (numValues == 65536) {
    method = Method::ALL;
    blockEnd = slice->getFilePointer();
    gap = block - index_ - 1;
  } else {
    method = Method::DENSE;
    blockEnd = slice->getFilePointer() + (1 << 13);
    wordIndex = -1;
    numberOfOnes = index_ + 1;
  }
}

int IndexedDISI::nextDoc()  { return advance(doc + 1); }

int IndexedDISI::index() { return index_; }

int64_t IndexedDISI::cost() { return cost_; }

deque<Method> Method::valueList;

Method::StaticConstructor::StaticConstructor() {}

Method::StaticConstructor Method::staticConstructor;
int Method::nextOrdinal = 0;
Method::Method(const wstring &name, InnerEnum innerEnum)
    : nameValue(name), ordinalValue(nextOrdinal++), innerEnumValue(innerEnum)
{
}

bool Method::operator==(const Method &other)
{
  return this->ordinalValue == other.ordinalValue;
}

bool Method::operator!=(const Method &other)
{
  return this->ordinalValue != other.ordinalValue;
}

deque<Method> Method::values() { return valueList; }

int Method::ordinal() { return ordinalValue; }

wstring Method::toString() { return nameValue; }

Method Method::valueOf(const wstring &name)
{
  for (auto enumInstance : Method::valueList) {
    if (enumInstance.nameValue == name) {
      return enumInstance;
    }
  }
}
} // namespace org::apache::lucene::codecs::lucene70