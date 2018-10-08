using namespace std;

#include "BitSetPostingsEnum.h"
#include "../../search/DocIdSetIterator.h"
#include "../../util/BitSet.h"
#include "../../util/BitSetIterator.h"
#include "../../util/BytesRef.h"

namespace org::apache::lucene::codecs::blocktree
{
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using BitSet = org::apache::lucene::util::BitSet;
using BitSetIterator = org::apache::lucene::util::BitSetIterator;
using BytesRef = org::apache::lucene::util::BytesRef;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;

BitSetPostingsEnum::BitSetPostingsEnum(shared_ptr<BitSet> bits) : bits(bits)
{
  reset();
}

int BitSetPostingsEnum::freq()  { return 1; }

int BitSetPostingsEnum::docID()
{
  if (in_ == nullptr) {
    return -1;
  } else {
    return in_->docID();
  }
}

int BitSetPostingsEnum::nextDoc() 
{
  if (in_ == nullptr) {
    in_ = make_shared<BitSetIterator>(bits, 0);
  }
  return in_->nextDoc();
}

int BitSetPostingsEnum::advance(int target) 
{
  return in_->advance(target);
}

int64_t BitSetPostingsEnum::cost() { return in_->cost(); }

void BitSetPostingsEnum::reset() { in_.reset(); }

shared_ptr<BytesRef> BitSetPostingsEnum::getPayload() { return nullptr; }

int BitSetPostingsEnum::nextPosition()
{
  throw make_shared<UnsupportedOperationException>();
}

int BitSetPostingsEnum::startOffset()
{
  throw make_shared<UnsupportedOperationException>();
}

int BitSetPostingsEnum::endOffset()
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::codecs::blocktree