using namespace std;

#include "LegacyBinaryDocValuesWrapper.h"

namespace org::apache::lucene::index
{
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;

LegacyBinaryDocValuesWrapper::LegacyBinaryDocValuesWrapper(
    shared_ptr<Bits> docsWithField, shared_ptr<LegacyBinaryDocValues> values)
    : docsWithField(docsWithField), values(values),
      maxDoc(docsWithField->length())
{
}

int LegacyBinaryDocValuesWrapper::docID() { return docID_; }

int LegacyBinaryDocValuesWrapper::nextDoc()
{
  docID_++;
  while (docID_ < maxDoc) {
    if (docsWithField->get(docID_)) {
      return docID_;
    }
    docID_++;
  }
  docID_ = NO_MORE_DOCS;
  return NO_MORE_DOCS;
}

int LegacyBinaryDocValuesWrapper::advance(int target)
{
  if (target < docID_) {
    throw invalid_argument(L"cannot advance backwards: docID=" +
                           to_wstring(docID_) + L" target=" +
                           to_wstring(target));
  }
  if (target == NO_MORE_DOCS) {
    this->docID_ = NO_MORE_DOCS;
  } else {
    this->docID_ = target - 1;
    nextDoc();
  }
  return docID_;
}

bool LegacyBinaryDocValuesWrapper::advanceExact(int target) 
{
  docID_ = target;
  return docsWithField->get(target);
}

int64_t LegacyBinaryDocValuesWrapper::cost() { return 0; }

shared_ptr<BytesRef> LegacyBinaryDocValuesWrapper::binaryValue()
{
  return values->get(docID_);
}
} // namespace org::apache::lucene::index