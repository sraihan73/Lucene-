using namespace std;

#include "DocValues.h"

namespace org::apache::lucene::index
{
using BytesRef = org::apache::lucene::util::BytesRef;

DocValues::DocValues() {}

shared_ptr<BinaryDocValues> DocValues::emptyBinary()
{
  return make_shared<BinaryDocValuesAnonymousInnerClass>();
}

DocValues::BinaryDocValuesAnonymousInnerClass::
    BinaryDocValuesAnonymousInnerClass()
{
}

int DocValues::BinaryDocValuesAnonymousInnerClass::advance(int target)
{
  return doc = NO_MORE_DOCS;
}

bool DocValues::BinaryDocValuesAnonymousInnerClass::advanceExact(
    int target) 
{
  doc = target;
  return false;
}

int DocValues::BinaryDocValuesAnonymousInnerClass::docID() { return doc; }

int DocValues::BinaryDocValuesAnonymousInnerClass::nextDoc()
{
  return doc = NO_MORE_DOCS;
}

int64_t DocValues::BinaryDocValuesAnonymousInnerClass::cost() { return 0; }

shared_ptr<BytesRef>
DocValues::BinaryDocValuesAnonymousInnerClass::binaryValue()
{
  assert(false);
  return nullptr;
}

shared_ptr<NumericDocValues> DocValues::emptyNumeric()
{
  return make_shared<NumericDocValuesAnonymousInnerClass>();
}

DocValues::NumericDocValuesAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass()
{
}

int DocValues::NumericDocValuesAnonymousInnerClass::advance(int target)
{
  return doc = NO_MORE_DOCS;
}

bool DocValues::NumericDocValuesAnonymousInnerClass::advanceExact(
    int target) 
{
  doc = target;
  return false;
}

int DocValues::NumericDocValuesAnonymousInnerClass::docID() { return doc; }

int DocValues::NumericDocValuesAnonymousInnerClass::nextDoc()
{
  return doc = NO_MORE_DOCS;
}

int64_t DocValues::NumericDocValuesAnonymousInnerClass::cost() { return 0; }

int64_t DocValues::NumericDocValuesAnonymousInnerClass::longValue()
{
  assert(false);
  return 0;
}

shared_ptr<LegacySortedDocValues> DocValues::emptyLegacySorted()
{
  shared_ptr<BytesRef> *const empty = make_shared<BytesRef>();
  return make_shared<LegacySortedDocValuesAnonymousInnerClass>(empty);
}

DocValues::LegacySortedDocValuesAnonymousInnerClass::
    LegacySortedDocValuesAnonymousInnerClass(shared_ptr<BytesRef> empty)
{
  this->empty = empty;
}

int DocValues::LegacySortedDocValuesAnonymousInnerClass::getOrd(int docID)
{
  return -1;
}

shared_ptr<BytesRef>
DocValues::LegacySortedDocValuesAnonymousInnerClass::lookupOrd(int ord)
{
  return empty;
}

int DocValues::LegacySortedDocValuesAnonymousInnerClass::getValueCount()
{
  return 0;
}

shared_ptr<SortedDocValues> DocValues::emptySorted()
{
  shared_ptr<BytesRef> *const empty = make_shared<BytesRef>();
  return make_shared<SortedDocValuesAnonymousInnerClass>(empty);
}

DocValues::SortedDocValuesAnonymousInnerClass::
    SortedDocValuesAnonymousInnerClass(shared_ptr<BytesRef> empty)
{
  this->empty = empty;
}

int DocValues::SortedDocValuesAnonymousInnerClass::advance(int target)
{
  return doc = NO_MORE_DOCS;
}

bool DocValues::SortedDocValuesAnonymousInnerClass::advanceExact(
    int target) 
{
  doc = target;
  return false;
}

int DocValues::SortedDocValuesAnonymousInnerClass::docID() { return doc; }

int DocValues::SortedDocValuesAnonymousInnerClass::nextDoc()
{
  return doc = NO_MORE_DOCS;
}

int64_t DocValues::SortedDocValuesAnonymousInnerClass::cost() { return 0; }

int DocValues::SortedDocValuesAnonymousInnerClass::ordValue()
{
  assert(false);
  return -1;
}

shared_ptr<BytesRef>
DocValues::SortedDocValuesAnonymousInnerClass::lookupOrd(int ord)
{
  return empty;
}

int DocValues::SortedDocValuesAnonymousInnerClass::getValueCount() { return 0; }

shared_ptr<SortedNumericDocValues> DocValues::emptySortedNumeric(int maxDoc)
{
  return make_shared<SortedNumericDocValuesAnonymousInnerClass>();
}

DocValues::SortedNumericDocValuesAnonymousInnerClass::
    SortedNumericDocValuesAnonymousInnerClass()
{
}

int DocValues::SortedNumericDocValuesAnonymousInnerClass::advance(int target)
{
  return doc = NO_MORE_DOCS;
}

bool DocValues::SortedNumericDocValuesAnonymousInnerClass::advanceExact(
    int target) 
{
  doc = target;
  return false;
}

int DocValues::SortedNumericDocValuesAnonymousInnerClass::docID()
{
  return doc;
}

int DocValues::SortedNumericDocValuesAnonymousInnerClass::nextDoc()
{
  return doc = NO_MORE_DOCS;
}

int64_t DocValues::SortedNumericDocValuesAnonymousInnerClass::cost()
{
  return 0;
}

int DocValues::SortedNumericDocValuesAnonymousInnerClass::docValueCount()
{
  throw make_shared<IllegalStateException>();
}

int64_t DocValues::SortedNumericDocValuesAnonymousInnerClass::nextValue()
{
  throw make_shared<IllegalStateException>();
}

shared_ptr<SortedSetDocValues> DocValues::emptySortedSet()
{
  shared_ptr<BytesRef> *const empty = make_shared<BytesRef>();
  return make_shared<SortedSetDocValuesAnonymousInnerClass>(empty);
}

DocValues::SortedSetDocValuesAnonymousInnerClass::
    SortedSetDocValuesAnonymousInnerClass(shared_ptr<BytesRef> empty)
{
  this->empty = empty;
}

int DocValues::SortedSetDocValuesAnonymousInnerClass::advance(int target)
{
  return doc = NO_MORE_DOCS;
}

bool DocValues::SortedSetDocValuesAnonymousInnerClass::advanceExact(
    int target) 
{
  doc = target;
  return false;
}

int DocValues::SortedSetDocValuesAnonymousInnerClass::docID() { return doc; }

int DocValues::SortedSetDocValuesAnonymousInnerClass::nextDoc()
{
  return doc = NO_MORE_DOCS;
}

int64_t DocValues::SortedSetDocValuesAnonymousInnerClass::cost() { return 0; }

int64_t DocValues::SortedSetDocValuesAnonymousInnerClass::nextOrd()
{
  assert(false);
  return NO_MORE_ORDS;
}

shared_ptr<BytesRef>
DocValues::SortedSetDocValuesAnonymousInnerClass::lookupOrd(int64_t ord)
{
  return empty;
}

int64_t DocValues::SortedSetDocValuesAnonymousInnerClass::getValueCount()
{
  return 0;
}

shared_ptr<SortedSetDocValues>
DocValues::singleton(shared_ptr<SortedDocValues> dv)
{
  return make_shared<SingletonSortedSetDocValues>(dv);
}

shared_ptr<SortedDocValues>
DocValues::unwrapSingleton(shared_ptr<SortedSetDocValues> dv)
{
  if (std::dynamic_pointer_cast<SingletonSortedSetDocValues>(dv) != nullptr) {
    return (std::static_pointer_cast<SingletonSortedSetDocValues>(dv))
        ->getSortedDocValues();
  } else {
    return nullptr;
  }
}

shared_ptr<NumericDocValues>
DocValues::unwrapSingleton(shared_ptr<SortedNumericDocValues> dv)
{
  if (std::dynamic_pointer_cast<SingletonSortedNumericDocValues>(dv) !=
      nullptr) {
    return (std::static_pointer_cast<SingletonSortedNumericDocValues>(dv))
        ->getNumericDocValues();
  } else {
    return nullptr;
  }
}

shared_ptr<SortedNumericDocValues>
DocValues::singleton(shared_ptr<NumericDocValues> dv)
{
  return make_shared<SingletonSortedNumericDocValues>(dv);
}

void DocValues::checkField(shared_ptr<LeafReader> in_, const wstring &field,
                           deque<DocValuesType> &expected)
{
  shared_ptr<FieldInfo> fi = in_->getFieldInfos()->fieldInfo(field);
  if (fi != nullptr) {
    DocValuesType actual = fi->getDocValuesType();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    throw make_shared<IllegalStateException>(
        L"unexpected docvalues type " + actual + L" for field '" + field +
        L"' " +
        StringHelper::toString(expected->length == 1
                                   ? L"(expected=" + expected[0]
                                   : L"(expected one of " +
                                         Arrays->toString(expected)) +
        L"). " + L"Re-index with correct docvalues type.");
  }
}

shared_ptr<NumericDocValues>
DocValues::getNumeric(shared_ptr<LeafReader> reader,
                      const wstring &field) 
{
  shared_ptr<NumericDocValues> dv = reader->getNumericDocValues(field);
  if (dv == nullptr) {
    checkField(reader, field, {DocValuesType::NUMERIC});
    return emptyNumeric();
  } else {
    return dv;
  }
}

shared_ptr<BinaryDocValues>
DocValues::getBinary(shared_ptr<LeafReader> reader,
                     const wstring &field) 
{
  shared_ptr<BinaryDocValues> dv = reader->getBinaryDocValues(field);
  if (dv == nullptr) {
    dv = reader->getSortedDocValues(field);
    if (dv == nullptr) {
      checkField(reader, field, {DocValuesType::BINARY, DocValuesType::SORTED});
      return emptyBinary();
    }
  }
  return dv;
}

shared_ptr<SortedDocValues>
DocValues::getSorted(shared_ptr<LeafReader> reader,
                     const wstring &field) 
{
  shared_ptr<SortedDocValues> dv = reader->getSortedDocValues(field);
  if (dv == nullptr) {
    checkField(reader, field, {DocValuesType::SORTED});
    return emptySorted();
  } else {
    return dv;
  }
}

shared_ptr<SortedNumericDocValues>
DocValues::getSortedNumeric(shared_ptr<LeafReader> reader,
                            const wstring &field) 
{
  shared_ptr<SortedNumericDocValues> dv =
      reader->getSortedNumericDocValues(field);
  if (dv == nullptr) {
    shared_ptr<NumericDocValues> single = reader->getNumericDocValues(field);
    if (single == nullptr) {
      checkField(reader, field,
                 {DocValuesType::SORTED_NUMERIC, DocValuesType::NUMERIC});
      return emptySortedNumeric(reader->maxDoc());
    }
    return singleton(single);
  }
  return dv;
}

shared_ptr<SortedSetDocValues>
DocValues::getSortedSet(shared_ptr<LeafReader> reader,
                        const wstring &field) 
{
  shared_ptr<SortedSetDocValues> dv = reader->getSortedSetDocValues(field);
  if (dv == nullptr) {
    shared_ptr<SortedDocValues> sorted = reader->getSortedDocValues(field);
    if (sorted == nullptr) {
      checkField(reader, field,
                 {DocValuesType::SORTED, DocValuesType::SORTED_SET});
      return emptySortedSet();
    }
    dv = singleton(sorted);
  }
  return dv;
}

bool DocValues::isCacheable(shared_ptr<LeafReaderContext> ctx,
                            deque<wstring> &fields)
{
  for (wstring field : fields) {
    shared_ptr<FieldInfo> fi = ctx->reader()->getFieldInfos()->fieldInfo(field);
    if (fi != nullptr && fi->getDocValuesGen() > -1) {
      return false;
    }
  }
  return true;
}
} // namespace org::apache::lucene::index