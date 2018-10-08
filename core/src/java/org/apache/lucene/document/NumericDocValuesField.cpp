using namespace std;

#include "NumericDocValuesField.h"

namespace org::apache::lucene::document
{
using DocValues = org::apache::lucene::index::DocValues;
using DocValuesType = org::apache::lucene::index::DocValuesType;
using LeafReader = org::apache::lucene::index::LeafReader;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using IndexOrDocValuesQuery =
    org::apache::lucene::search::IndexOrDocValuesQuery;
using Query = org::apache::lucene::search::Query;
const shared_ptr<FieldType> NumericDocValuesField::TYPE =
    make_shared<FieldType>();

NumericDocValuesField::StaticConstructor::StaticConstructor()
{
  TYPE->setDocValuesType(DocValuesType::NUMERIC);
  TYPE->freeze();
}

NumericDocValuesField::StaticConstructor
    NumericDocValuesField::staticConstructor;

NumericDocValuesField::NumericDocValuesField(const wstring &name,
                                             int64_t value)
    : NumericDocValuesField(name, Long::valueOf(value))
{
}

NumericDocValuesField::NumericDocValuesField(const wstring &name,
                                             optional<int64_t> &value)
    : Field(name, TYPE)
{
  fieldsData = value;
}

shared_ptr<Query> NumericDocValuesField::newSlowRangeQuery(const wstring &field,
                                                           int64_t lowerValue,
                                                           int64_t upperValue)
{
  return make_shared<SortedNumericDocValuesRangeQueryAnonymousInnerClass>(
      field, lowerValue, upperValue);
}

NumericDocValuesField::SortedNumericDocValuesRangeQueryAnonymousInnerClass::
    SortedNumericDocValuesRangeQueryAnonymousInnerClass(const wstring &field,
                                                        int64_t lowerValue,
                                                        int64_t upperValue)
    : SortedNumericDocValuesRangeQuery(field, lowerValue, upperValue)
{
  this->field = field;
}

shared_ptr<SortedNumericDocValues>
NumericDocValuesField::SortedNumericDocValuesRangeQueryAnonymousInnerClass::
    getValues(shared_ptr<LeafReader> reader,
              const wstring &field) 
{
  shared_ptr<NumericDocValues> values = reader->getNumericDocValues(field);
  if (values == nullptr) {
    return nullptr;
  }
  return DocValues::singleton(values);
}

shared_ptr<Query> NumericDocValuesField::newSlowExactQuery(const wstring &field,
                                                           int64_t value)
{
  return newSlowRangeQuery(field, value, value);
}
} // namespace org::apache::lucene::document