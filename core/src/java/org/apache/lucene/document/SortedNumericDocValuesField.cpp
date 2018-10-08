using namespace std;

#include "SortedNumericDocValuesField.h"

namespace org::apache::lucene::document
{
using DocValues = org::apache::lucene::index::DocValues;
using DocValuesType = org::apache::lucene::index::DocValuesType;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using LeafReader = org::apache::lucene::index::LeafReader;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using IndexOrDocValuesQuery =
    org::apache::lucene::search::IndexOrDocValuesQuery;
using Query = org::apache::lucene::search::Query;
const shared_ptr<FieldType> SortedNumericDocValuesField::TYPE =
    make_shared<FieldType>();

SortedNumericDocValuesField::StaticConstructor::StaticConstructor()
{
  TYPE->setDocValuesType(DocValuesType::SORTED_NUMERIC);
  TYPE->freeze();
}

SortedNumericDocValuesField::StaticConstructor
    SortedNumericDocValuesField::staticConstructor;

SortedNumericDocValuesField::SortedNumericDocValuesField(const wstring &name,
                                                         int64_t value)
    : Field(name, TYPE)
{
  fieldsData = static_cast<int64_t>(value);
}

shared_ptr<Query> SortedNumericDocValuesField::newSlowRangeQuery(
    const wstring &field, int64_t lowerValue, int64_t upperValue)
{
  return make_shared<SortedNumericDocValuesRangeQueryAnonymousInnerClass>(
      field, lowerValue, upperValue);
}

SortedNumericDocValuesField::
    SortedNumericDocValuesRangeQueryAnonymousInnerClass::
        SortedNumericDocValuesRangeQueryAnonymousInnerClass(
            const wstring &field, int64_t lowerValue, int64_t upperValue)
    : SortedNumericDocValuesRangeQuery(field, lowerValue, upperValue)
{
  this->field = field;
}

shared_ptr<SortedNumericDocValues> SortedNumericDocValuesField::
    SortedNumericDocValuesRangeQueryAnonymousInnerClass::getValues(
        shared_ptr<LeafReader> reader, const wstring &field) 
{
  shared_ptr<FieldInfo> info = reader->getFieldInfos()->fieldInfo(field);
  if (info == nullptr) {
    // Queries have some optimizations when one sub scorer returns null rather
    // than a scorer that does not match any documents
    return nullptr;
  }
  return DocValues::getSortedNumeric(reader, field);
}

shared_ptr<Query>
SortedNumericDocValuesField::newSlowExactQuery(const wstring &field,
                                               int64_t value)
{
  return newSlowRangeQuery(field, value, value);
}
} // namespace org::apache::lucene::document