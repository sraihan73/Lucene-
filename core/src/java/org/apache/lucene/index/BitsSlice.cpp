using namespace std;

#include "BitsSlice.h"

namespace org::apache::lucene::index
{
using Bits = org::apache::lucene::util::Bits;
using FutureObjects = org::apache::lucene::util::FutureObjects;

BitsSlice::BitsSlice(shared_ptr<Bits> parent, shared_ptr<ReaderSlice> slice)
    : parent(parent), start(slice->start), length(slice->length)
{
  assert((length_ >= 0, L"length=" + to_wstring(length_)));
}

bool BitsSlice::get(int doc)
{
  FutureObjects::checkIndex(doc, length_);
  return parent->get(doc + start);
}

int BitsSlice::length() { return length_; }
} // namespace org::apache::lucene::index