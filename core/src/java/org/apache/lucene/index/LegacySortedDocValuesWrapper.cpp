using namespace std;

#include "LegacySortedDocValuesWrapper.h"

namespace org::apache::lucene::index
{
using BytesRef = org::apache::lucene::util::BytesRef;

LegacySortedDocValuesWrapper::LegacySortedDocValuesWrapper(
    shared_ptr<LegacySortedDocValues> values, int maxDoc)
    : values(values), maxDoc(maxDoc)
{
}

int LegacySortedDocValuesWrapper::docID() { return docID_; }

int LegacySortedDocValuesWrapper::nextDoc()
{
  assert(docID_ != NO_MORE_DOCS);
  docID_++;
  while (docID_ < maxDoc) {
    ord = values->getOrd(docID_);
    if (ord != -1) {
      return docID_;
    }
    docID_++;
  }
  docID_ = NO_MORE_DOCS;
  return NO_MORE_DOCS;
}

int LegacySortedDocValuesWrapper::advance(int target)
{
  if (target < docID_) {
    throw invalid_argument(L"cannot advance backwards: docID=" +
                           to_wstring(docID_) + L" target=" +
                           to_wstring(target));
  }
  if (target >= maxDoc) {
    this->docID_ = NO_MORE_DOCS;
  } else {
    this->docID_ = target - 1;
    nextDoc();
  }
  return docID_;
}

bool LegacySortedDocValuesWrapper::advanceExact(int target) 
{
  docID_ = target;
  ord = values->getOrd(docID_);
  return ord != -1;
}

int64_t LegacySortedDocValuesWrapper::cost() { return 0; }

int LegacySortedDocValuesWrapper::ordValue() { return ord; }

shared_ptr<BytesRef> LegacySortedDocValuesWrapper::lookupOrd(int ord)
{
  return values->lookupOrd(ord);
}

int LegacySortedDocValuesWrapper::getValueCount()
{
  return values->getValueCount();
}
} // namespace org::apache::lucene::index