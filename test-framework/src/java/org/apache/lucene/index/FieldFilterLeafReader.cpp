using namespace std;

#include "FieldFilterLeafReader.h"

namespace org::apache::lucene::index
{
using FilterIterator = org::apache::lucene::util::FilterIterator;

FieldFilterLeafReader::FieldFilterLeafReader(shared_ptr<LeafReader> in_,
                                             shared_ptr<Set<wstring>> fields,
                                             bool negate)
    : FilterLeafReader(in_), fields(fields), negate(negate),
      fieldInfos(make_shared<FieldInfos>(filteredInfos::toArray(
          std::deque<std::shared_ptr<FieldInfo>>(filteredInfos->size()))))
{
  deque<std::shared_ptr<FieldInfo>> filteredInfos =
      deque<std::shared_ptr<FieldInfo>>();
  for (auto fi : in_->getFieldInfos()) {
    if (hasField(fi->name)) {
      filteredInfos.push_back(fi);
    }
  }
}

bool FieldFilterLeafReader::hasField(const wstring &field)
{
  return negate ^ fields->contains(field);
}

shared_ptr<FieldInfos> FieldFilterLeafReader::getFieldInfos()
{
  return fieldInfos;
}

shared_ptr<Fields>
FieldFilterLeafReader::getTermVectors(int docID) 
{
  shared_ptr<Fields> f = FilterLeafReader::getTermVectors(docID);
  if (f->empty()) {
    return nullptr;
  }
  f = make_shared<FieldFilterFields>(shared_from_this(), f);
  // we need to check for emptyness, so we can return
  // null:
  return f->begin()->hasNext() ? f : nullptr;
}

void FieldFilterLeafReader::document(
    int const docID, shared_ptr<StoredFieldVisitor> visitor) 
{
  FilterLeafReader::document(docID,
                             make_shared<StoredFieldVisitorAnonymousInnerClass>(
                                 shared_from_this(), visitor));
}

FieldFilterLeafReader::StoredFieldVisitorAnonymousInnerClass::
    StoredFieldVisitorAnonymousInnerClass(
        shared_ptr<FieldFilterLeafReader> outerInstance,
        shared_ptr<org::apache::lucene::index::StoredFieldVisitor> visitor)
{
  this->outerInstance = outerInstance;
  this->visitor = visitor;
}

void FieldFilterLeafReader::StoredFieldVisitorAnonymousInnerClass::binaryField(
    shared_ptr<FieldInfo> fieldInfo,
    std::deque<char> &value) 
{
  visitor->binaryField(fieldInfo, value);
}

void FieldFilterLeafReader::StoredFieldVisitorAnonymousInnerClass::stringField(
    shared_ptr<FieldInfo> fieldInfo,
    std::deque<char> &value) 
{
  visitor->stringField(fieldInfo, value);
}

void FieldFilterLeafReader::StoredFieldVisitorAnonymousInnerClass::intField(
    shared_ptr<FieldInfo> fieldInfo, int value) 
{
  visitor->intField(fieldInfo, value);
}

void FieldFilterLeafReader::StoredFieldVisitorAnonymousInnerClass::longField(
    shared_ptr<FieldInfo> fieldInfo, int64_t value) 
{
  visitor->longField(fieldInfo, value);
}

void FieldFilterLeafReader::StoredFieldVisitorAnonymousInnerClass::floatField(
    shared_ptr<FieldInfo> fieldInfo, float value) 
{
  visitor->floatField(fieldInfo, value);
}

void FieldFilterLeafReader::StoredFieldVisitorAnonymousInnerClass::doubleField(
    shared_ptr<FieldInfo> fieldInfo, double value) 
{
  visitor->doubleField(fieldInfo, value);
}

Status FieldFilterLeafReader::StoredFieldVisitorAnonymousInnerClass::needsField(
    shared_ptr<FieldInfo> fieldInfo) 
{
  return outerInstance->hasField(fieldInfo->name)
             ? visitor->needsField(fieldInfo)
             : Status::NO;
}

shared_ptr<Terms>
FieldFilterLeafReader::terms(const wstring &field) 
{
  return hasField(field) ? FilterLeafReader::terms(field) : nullptr;
}

shared_ptr<BinaryDocValues> FieldFilterLeafReader::getBinaryDocValues(
    const wstring &field) 
{
  return hasField(field) ? FilterLeafReader::getBinaryDocValues(field)
                         : nullptr;
}

shared_ptr<SortedDocValues> FieldFilterLeafReader::getSortedDocValues(
    const wstring &field) 
{
  return hasField(field) ? FilterLeafReader::getSortedDocValues(field)
                         : nullptr;
}

shared_ptr<SortedNumericDocValues>
FieldFilterLeafReader::getSortedNumericDocValues(const wstring &field) throw(
    IOException)
{
  return hasField(field) ? FilterLeafReader::getSortedNumericDocValues(field)
                         : nullptr;
}

shared_ptr<SortedSetDocValues> FieldFilterLeafReader::getSortedSetDocValues(
    const wstring &field) 
{
  return hasField(field) ? FilterLeafReader::getSortedSetDocValues(field)
                         : nullptr;
}

shared_ptr<NumericDocValues>
FieldFilterLeafReader::getNormValues(const wstring &field) 
{
  return hasField(field) ? FilterLeafReader::getNormValues(field) : nullptr;
}

wstring FieldFilterLeafReader::toString()
{
  shared_ptr<StringBuilder> *const sb =
      make_shared<StringBuilder>(L"FieldFilterLeafReader(reader=");
  sb->append(in_)->append(L", fields=");
  if (negate) {
    sb->append(L'!');
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return sb->append(fields)->append(L')')->toString();
}

FieldFilterLeafReader::FieldFilterFields::FieldFilterFields(
    shared_ptr<FieldFilterLeafReader> outerInstance, shared_ptr<Fields> in_)
    : FilterFields(in_), outerInstance(outerInstance)
{
}

int FieldFilterLeafReader::FieldFilterFields::size()
{
  // this information is not cheap, return -1 like MultiFields does:
  return -1;
}

shared_ptr<Iterator<wstring>>
FieldFilterLeafReader::FieldFilterFields::iterator()
{
  return make_shared<FilterIteratorAnonymousInnerClass>(shared_from_this(),
                                                        FilterFields::begin());
}

FieldFilterLeafReader::FieldFilterFields::FilterIteratorAnonymousInnerClass::
    FilterIteratorAnonymousInnerClass(
        shared_ptr<FieldFilterFields> outerInstance,
        shared_ptr<Iterator<wstring>> iterator)
    : org::apache::lucene::util::FilterIterator<std::wstring, std::wstring>(iterator)
{
  this->outerInstance = outerInstance;
}

bool FieldFilterLeafReader::FieldFilterFields::
    FilterIteratorAnonymousInnerClass::predicateFunction(const wstring &field)
{
  return outerInstance->outerInstance->hasField(field);
}

shared_ptr<Terms> FieldFilterLeafReader::FieldFilterFields::terms(
    const wstring &field) 
{
  return outerInstance->hasField(field) ? FilterFields::terms(field) : nullptr;
}

shared_ptr<CacheHelper> FieldFilterLeafReader::getCoreCacheHelper()
{
  return nullptr;
}

shared_ptr<CacheHelper> FieldFilterLeafReader::getReaderCacheHelper()
{
  return nullptr;
}
} // namespace org::apache::lucene::index