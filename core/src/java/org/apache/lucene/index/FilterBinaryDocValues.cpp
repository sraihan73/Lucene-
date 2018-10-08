using namespace std;

#include "FilterBinaryDocValues.h"

namespace org::apache::lucene::index
{
using BytesRef = org::apache::lucene::util::BytesRef;

FilterBinaryDocValues::FilterBinaryDocValues(shared_ptr<BinaryDocValues> in_)
    : in_(in_)
{
}

int FilterBinaryDocValues::docID() { return in_->docID(); }

int FilterBinaryDocValues::nextDoc() 
{
  return in_->nextDoc();
}

int FilterBinaryDocValues::advance(int target) 
{
  return in_->advance(target);
}

bool FilterBinaryDocValues::advanceExact(int target) 
{
  return in_->advanceExact(target);
}

int64_t FilterBinaryDocValues::cost() { return in_->cost(); }

shared_ptr<BytesRef> FilterBinaryDocValues::binaryValue() 
{
  return in_->binaryValue();
}
} // namespace org::apache::lucene::index