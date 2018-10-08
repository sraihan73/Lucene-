using namespace std;

#include "LongPoint.h"

namespace org::apache::lucene::document
{
using PointValues = org::apache::lucene::index::PointValues;
using PointInSetQuery = org::apache::lucene::search::PointInSetQuery;
using PointRangeQuery = org::apache::lucene::search::PointRangeQuery;
using Query = org::apache::lucene::search::Query;
using BytesRef = org::apache::lucene::util::BytesRef;
using NumericUtils = org::apache::lucene::util::NumericUtils;

shared_ptr<FieldType> LongPoint::getType(int numDims)
{
  shared_ptr<FieldType> type = make_shared<FieldType>();
  type->setDimensions(numDims, Long::BYTES);
  type->freeze();
  return type;
}

void LongPoint::setLongValue(int64_t value) { setLongValues({value}); }

void LongPoint::setLongValues(deque<int64_t> &point)
{
  if (type->pointDimensionCount() != point->length) {
    throw invalid_argument(L"this field (name=" + name_ + L") uses " +
                           to_wstring(type->pointDimensionCount()) +
                           L" dimensions; cannot change to (incoming) " +
                           point->length + L" dimensions");
  }
  fieldsData = pack({point});
}

void LongPoint::setBytesValue(shared_ptr<BytesRef> bytes)
{
  throw invalid_argument(L"cannot change value type from long to BytesRef");
}

shared_ptr<Number> LongPoint::numericValue()
{
  if (type->pointDimensionCount() != 1) {
    throw make_shared<IllegalStateException>(
        L"this field (name=" + name_ + L") uses " +
        to_wstring(type->pointDimensionCount()) +
        L" dimensions; cannot convert to a single numeric value");
  }
  shared_ptr<BytesRef> bytes = any_cast<std::shared_ptr<BytesRef>>(fieldsData);
  assert(bytes->length == Long::BYTES);
  return decodeDimension(bytes->bytes, bytes->offset);
}

shared_ptr<BytesRef> LongPoint::pack(deque<int64_t> &point)
{
  if (point == nullptr) {
    throw invalid_argument(L"point must not be null");
  }
  if (point->length == 0) {
    throw invalid_argument(L"point must not be 0 dimensions");
  }
  std::deque<char> packed(point->length * Long::BYTES);

  for (int dim = 0; dim < point->length; dim++) {
    encodeDimension(point[dim], packed, dim * Long::BYTES);
  }

  return make_shared<BytesRef>(packed);
}

LongPoint::LongPoint(const wstring &name, deque<int64_t> &point)
    : Field(name, pack(point), getType(point->length))
{
}

wstring LongPoint::toString()
{
  shared_ptr<StringBuilder> result = make_shared<StringBuilder>();
  result->append(getClass().getSimpleName());
  result->append(L" <");
  result->append(name_);
  result->append(L':');

  shared_ptr<BytesRef> bytes = any_cast<std::shared_ptr<BytesRef>>(fieldsData);
  for (int dim = 0; dim < type->pointDimensionCount(); dim++) {
    if (dim > 0) {
      result->append(L',');
    }
    result->append(
        decodeDimension(bytes->bytes, bytes->offset + dim * Long::BYTES));
  }

  result->append(L'>');
  return result->toString();
}

void LongPoint::encodeDimension(int64_t value, std::deque<char> &dest,
                                int offset)
{
  NumericUtils::longToSortableBytes(value, dest, offset);
}

int64_t LongPoint::decodeDimension(std::deque<char> &value, int offset)
{
  return NumericUtils::sortableBytesToLong(value, offset);
}

shared_ptr<Query> LongPoint::newExactQuery(const wstring &field,
                                           int64_t value)
{
  return newRangeQuery(field, value, value);
}

shared_ptr<Query> LongPoint::newRangeQuery(const wstring &field,
                                           int64_t lowerValue,
                                           int64_t upperValue)
{
  return newRangeQuery(field, std::deque<int64_t>{lowerValue},
                       std::deque<int64_t>{upperValue});
}

shared_ptr<Query> LongPoint::newRangeQuery(const wstring &field,
                                           std::deque<int64_t> &lowerValue,
                                           std::deque<int64_t> &upperValue)
{
  PointRangeQuery::checkArgs(field, lowerValue, upperValue);
  return make_shared<PointRangeQueryAnonymousInnerClass>(
      field, pack(lowerValue)->bytes, pack(upperValue)->bytes,
      lowerValue.size());
}

LongPoint::PointRangeQueryAnonymousInnerClass::
    PointRangeQueryAnonymousInnerClass(const wstring &field,
                                       deque<char> &bytes, deque<char> &bytes,
                                       shared_ptr<UnknownType> length)
    : org::apache::lucene::search::PointRangeQuery(field, bytes, bytes, length)
{
}

wstring LongPoint::PointRangeQueryAnonymousInnerClass::toString(
    int dimension, std::deque<char> &value)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Long::toString(decodeDimension(value, 0));
}

shared_ptr<Query> LongPoint::newSetQuery(const wstring &field,
                                         deque<int64_t> &values)
{

  // Don't unexpectedly change the user's incoming values array:
  std::deque<int64_t> sortedValues = values::clone();
  Arrays::sort(sortedValues);

  shared_ptr<BytesRef> *const encoded =
      make_shared<BytesRef>(std::deque<char>(Long::BYTES));

  return make_shared<PointInSetQuery>(
      field, 1, Long::BYTES,
      make_shared<StreamAnonymousInnerClass>(sortedValues, encoded)){
    protected :
        wstring toString(char[] value){assert(value->length == Long::BYTES);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Long::toString(decodeDimension(value, 0));
}
}; // namespace org::apache::lucene::document
}

LongPoint::StreamAnonymousInnerClass::StreamAnonymousInnerClass(
    deque<int64_t> &sortedValues, shared_ptr<BytesRef> encoded)
{
  this->sortedValues = sortedValues;
  this->encoded = encoded;
}

shared_ptr<BytesRef> LongPoint::StreamAnonymousInnerClass::next()
{
  if (upto == sortedValues.size()) {
    return nullptr;
  } else {
    encodeDimension(sortedValues[upto], encoded->bytes, 0);
    upto++;
    return encoded;
  }
}

shared_ptr<Query>
LongPoint::newSetQuery(const wstring &field,
                       shared_ptr<deque<int64_t>> values)
{
  std::deque<optional<int64_t>> boxed =
      values->toArray(std::deque<optional<int64_t>>(0));
  std::deque<int64_t> unboxed(boxed.size());
  for (int i = 0; i < boxed.size(); i++) {
    unboxed[i] = boxed[i];
  }
  return newSetQuery(field, unboxed);
}
}