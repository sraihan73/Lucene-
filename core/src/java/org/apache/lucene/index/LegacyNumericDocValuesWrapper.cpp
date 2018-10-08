using namespace std;

#include "LegacyNumericDocValuesWrapper.h"

namespace org::apache::lucene::index
{
using Bits = org::apache::lucene::util::Bits;

LegacyNumericDocValuesWrapper::LegacyNumericDocValuesWrapper(
    shared_ptr<Bits> docsWithField, shared_ptr<LegacyNumericDocValues> values)
    : docsWithField(docsWithField), values(values),
      maxDoc(docsWithField->length())
{
}

int LegacyNumericDocValuesWrapper::docID() { return docID_; }

int LegacyNumericDocValuesWrapper::nextDoc()
{
  docID_++;
  while (docID_ < maxDoc) {
    value = values->get(docID_);
    if (value != 0 || docsWithField->get(docID_)) {
      return docID_;
    }
    docID_++;
  }
  docID_ = NO_MORE_DOCS;
  return NO_MORE_DOCS;
}

int LegacyNumericDocValuesWrapper::advance(int target)
{
  assert((target >= docID_,
          L"target=" + to_wstring(target) + L" docID=" + to_wstring(docID_)));
  if (target == NO_MORE_DOCS) {
    this->docID_ = NO_MORE_DOCS;
  } else {
    this->docID_ = target - 1;
    nextDoc();
  }
  return docID_;
}

bool LegacyNumericDocValuesWrapper::advanceExact(int target) 
{
  docID_ = target;
  value = values->get(docID_);
  return value != 0 || docsWithField->get(docID_);
}

int64_t LegacyNumericDocValuesWrapper::cost()
{
  // TODO
  return 0;
}

int64_t LegacyNumericDocValuesWrapper::longValue() { return value; }

wstring LegacyNumericDocValuesWrapper::toString()
{
  return L"LegacyNumericDocValuesWrapper(" + values + L")";
}
} // namespace org::apache::lucene::index