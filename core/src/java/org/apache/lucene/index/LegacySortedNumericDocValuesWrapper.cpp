using namespace std;

#include "LegacySortedNumericDocValuesWrapper.h"

namespace org::apache::lucene::index
{

LegacySortedNumericDocValuesWrapper::LegacySortedNumericDocValuesWrapper(
    shared_ptr<LegacySortedNumericDocValues> values, int maxDoc)
    : values(values), maxDoc(maxDoc)
{
}

int LegacySortedNumericDocValuesWrapper::docID() { return docID_; }

int LegacySortedNumericDocValuesWrapper::nextDoc()
{
  assert(docID_ != NO_MORE_DOCS);
  while (true) {
    docID_++;
    if (docID_ == maxDoc) {
      docID_ = NO_MORE_DOCS;
      break;
    }
    values->setDocument(docID_);
    if (values->count() != 0) {
      break;
    }
  }
  upto = 0;
  return docID_;
}

int LegacySortedNumericDocValuesWrapper::advance(int target)
{
  if (target < docID_) {
    throw invalid_argument(L"cannot advance backwards: docID=" +
                           to_wstring(docID_) + L" target=" +
                           to_wstring(target));
  }
  if (target >= maxDoc) {
    docID_ = NO_MORE_DOCS;
  } else {
    docID_ = target - 1;
    nextDoc();
  }
  return docID_;
}

bool LegacySortedNumericDocValuesWrapper::advanceExact(int target) throw(
    IOException)
{
  docID_ = target;
  values->setDocument(docID_);
  upto = 0;
  return values->count() != 0;
}

int64_t LegacySortedNumericDocValuesWrapper::cost() { return 0; }

int64_t LegacySortedNumericDocValuesWrapper::nextValue()
{
  return values->valueAt(upto++);
}

int LegacySortedNumericDocValuesWrapper::docValueCount()
{
  return values->count();
}
} // namespace org::apache::lucene::index