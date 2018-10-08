using namespace std;

#include "SortedDocValuesField.h"

namespace org::apache::lucene::document
{
using DocValues = org::apache::lucene::index::DocValues;
using DocValuesType = org::apache::lucene::index::DocValuesType;
using LeafReader = org::apache::lucene::index::LeafReader;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using IndexOrDocValuesQuery =
    org::apache::lucene::search::IndexOrDocValuesQuery;
using Query = org::apache::lucene::search::Query;
using BytesRef = org::apache::lucene::util::BytesRef;
const shared_ptr<FieldType> SortedDocValuesField::TYPE =
    make_shared<FieldType>();

SortedDocValuesField::StaticConstructor::StaticConstructor()
{
  TYPE->setDocValuesType(DocValuesType::SORTED);
  TYPE->freeze();
}

SortedDocValuesField::StaticConstructor SortedDocValuesField::staticConstructor;

SortedDocValuesField::SortedDocValuesField(const wstring &name,
                                           shared_ptr<BytesRef> bytes)
    : Field(name, TYPE)
{
  fieldsData = bytes;
}

shared_ptr<Query> SortedDocValuesField::newSlowRangeQuery(
    const wstring &field, shared_ptr<BytesRef> lowerValue,
    shared_ptr<BytesRef> upperValue, bool lowerInclusive, bool upperInclusive)
{
  return make_shared<SortedSetDocValuesRangeQueryAnonymousInnerClass>(
      field, lowerValue, upperValue, lowerInclusive, upperInclusive);
}

SortedDocValuesField::SortedSetDocValuesRangeQueryAnonymousInnerClass::
    SortedSetDocValuesRangeQueryAnonymousInnerClass(
        const wstring &field, shared_ptr<BytesRef> lowerValue,
        shared_ptr<BytesRef> upperValue, bool lowerInclusive,
        bool upperInclusive)
    : SortedSetDocValuesRangeQuery(field, lowerValue, upperValue,
                                   lowerInclusive, upperInclusive)
{
  this->field = field;
}

shared_ptr<SortedSetDocValues>
SortedDocValuesField::SortedSetDocValuesRangeQueryAnonymousInnerClass::
    getValues(shared_ptr<LeafReader> reader,
              const wstring &field) 
{
  return DocValues::singleton(DocValues::getSorted(reader, field));
}

shared_ptr<Query>
SortedDocValuesField::newSlowExactQuery(const wstring &field,
                                        shared_ptr<BytesRef> value)
{
  return newSlowRangeQuery(field, value, value, true, true);
}
} // namespace org::apache::lucene::document