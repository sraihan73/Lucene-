using namespace std;

#include "LegacySortedSetDocValuesWrapper.h"

namespace org::apache::lucene::index
{
using BytesRef = org::apache::lucene::util::BytesRef;

LegacySortedSetDocValuesWrapper::LegacySortedSetDocValuesWrapper(
    shared_ptr<LegacySortedSetDocValues> values, int maxDoc)
    : values(values), maxDoc(maxDoc)
{
}

int LegacySortedSetDocValuesWrapper::docID() { return docID_; }

int LegacySortedSetDocValuesWrapper::nextDoc()
{
  assert(docID_ != NO_MORE_DOCS);
  docID_++;
  while (docID_ < maxDoc) {
    values->setDocument(docID_);
    ord = values->nextOrd();
    if (ord != NO_MORE_ORDS) {
      return docID_;
    }
    docID_++;
  }
  docID_ = NO_MORE_DOCS;
  return NO_MORE_DOCS;
}

int LegacySortedSetDocValuesWrapper::advance(int target)
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

bool LegacySortedSetDocValuesWrapper::advanceExact(int target) throw(
    IOException)
{
  docID_ = target;
  values->setDocument(docID_);
  ord = values->nextOrd();
  return ord != NO_MORE_ORDS;
}

int64_t LegacySortedSetDocValuesWrapper::cost() { return 0; }

int64_t LegacySortedSetDocValuesWrapper::nextOrd()
{
  int64_t result = ord;
  if (result != NO_MORE_ORDS) {
    ord = values->nextOrd();
  }
  return result;
}

shared_ptr<BytesRef> LegacySortedSetDocValuesWrapper::lookupOrd(int64_t ord)
{
  return values->lookupOrd(static_cast<int>(ord));
}

int64_t LegacySortedSetDocValuesWrapper::getValueCount()
{
  return values->getValueCount();
}

wstring LegacySortedSetDocValuesWrapper::toString()
{
  return L"LegacySortedSetDocValuesWrapper(" + values + L")";
}
} // namespace org::apache::lucene::index