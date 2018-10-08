using namespace std;

#include "BigIntegerPoint.h"

namespace org::apache::lucene::document
{
using PointValues = org::apache::lucene::index::PointValues;
using PointInSetQuery = org::apache::lucene::search::PointInSetQuery;
using PointRangeQuery = org::apache::lucene::search::PointRangeQuery;
using Query = org::apache::lucene::search::Query;
using BytesRef = org::apache::lucene::util::BytesRef;
using NumericUtils = org::apache::lucene::util::NumericUtils;
const shared_ptr<java::math::int64_t> BigIntegerPoint::MIN_VALUE =
    java::math::int64_t::ONE::shiftLeft(BYTES * 8 - 1).negate();
const shared_ptr<java::math::int64_t> BigIntegerPoint::MAX_VALUE =
    java::math::int64_t::ONE::shiftLeft(BYTES * 8 - 1)
        .subtract(java::math::int64_t::ONE);

shared_ptr<FieldType> BigIntegerPoint::getType(int numDims)
{
  shared_ptr<FieldType> type = make_shared<FieldType>();
  type->setDimensions(numDims, BYTES);
  type->freeze();
  return type;
}

void BigIntegerPoint::setBigIntegerValues(deque<int64_t> &point)
{
  if (type->pointDimensionCount() != point->length) {
    throw invalid_argument(L"this field (name=" + name_ + L") uses " +
                           to_wstring(type->pointDimensionCount()) +
                           L" dimensions; cannot change to (incoming) " +
                           point->length + L" dimensions");
  }
  fieldsData = pack({point});
}

void BigIntegerPoint::setBytesValue(shared_ptr<BytesRef> bytes)
{
  throw invalid_argument(
      L"cannot change value type from int64_t to BytesRef");
}

shared_ptr<Number> BigIntegerPoint::numericValue()
{
  if (type->pointDimensionCount() != 1) {
    throw make_shared<IllegalStateException>(
        L"this field (name=" + name_ + L") uses " +
        to_wstring(type->pointDimensionCount()) +
        L" dimensions; cannot convert to a single numeric value");
  }
  shared_ptr<BytesRef> bytes = any_cast<std::shared_ptr<BytesRef>>(fieldsData);
  assert(bytes->length == BYTES);
  return decodeDimension(bytes->bytes, bytes->offset);
}

shared_ptr<BytesRef> BigIntegerPoint::pack(deque<int64_t> &point)
{
  if (point == nullptr) {
    throw invalid_argument(L"point must not be null");
  }
  if (point->length == 0) {
    throw invalid_argument(L"point must not be 0 dimensions");
  }
  std::deque<char> packed(point->length * BYTES);

  for (int dim = 0; dim < point->length; dim++) {
    encodeDimension(point[dim], packed, dim * BYTES);
  }

  return make_shared<BytesRef>(packed);
}

BigIntegerPoint::BigIntegerPoint(const wstring &name, deque<int64_t> &point)
    : Field(name, pack(point), getType(point->length))
{
}

wstring BigIntegerPoint::toString()
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
    result->append(decodeDimension(bytes->bytes, bytes->offset + dim * BYTES));
  }

  result->append(L'>');
  return result->toString();
}

void BigIntegerPoint::encodeDimension(shared_ptr<int64_t> value,
                                      std::deque<char> &dest, int offset)
{
  NumericUtils::bigIntToSortableBytes(value, BYTES, dest, offset);
}

shared_ptr<int64_t>
BigIntegerPoint::decodeDimension(std::deque<char> &value, int offset)
{
  return NumericUtils::sortableBytesToBigInt(value, offset, BYTES);
}

shared_ptr<Query> BigIntegerPoint::newExactQuery(const wstring &field,
                                                 shared_ptr<int64_t> value)
{
  return newRangeQuery(field, value, value);
}

shared_ptr<Query>
BigIntegerPoint::newRangeQuery(const wstring &field,
                               shared_ptr<int64_t> lowerValue,
                               shared_ptr<int64_t> upperValue)
{
  PointRangeQuery::checkArgs(field, lowerValue, upperValue);
  return newRangeQuery(field,
                       std::deque<std::shared_ptr<int64_t>>{lowerValue},
                       std::deque<std::shared_ptr<int64_t>>{upperValue});
}

shared_ptr<Query> BigIntegerPoint::newRangeQuery(
    const wstring &field, std::deque<std::shared_ptr<int64_t>> &lowerValue,
    std::deque<std::shared_ptr<int64_t>> &upperValue)
{
  PointRangeQuery::checkArgs(field, lowerValue, upperValue);
  return make_shared<PointRangeQueryAnonymousInnerClass>(
      field, pack(lowerValue)->bytes, pack(upperValue)->bytes,
      lowerValue.size());
}

BigIntegerPoint::PointRangeQueryAnonymousInnerClass::
    PointRangeQueryAnonymousInnerClass(const wstring &field,
                                       deque<char> &bytes, deque<char> &bytes,
                                       shared_ptr<UnknownType> length)
    : org::apache::lucene::search::PointRangeQuery(field, bytes, bytes, length)
{
}

wstring BigIntegerPoint::PointRangeQueryAnonymousInnerClass::toString(
    int dimension, std::deque<char> &value)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return BigIntegerPoint::decodeDimension(value, 0)->toString();
}

shared_ptr<Query> BigIntegerPoint::newSetQuery(const wstring &field,
                                               deque<int64_t> &values)
{

  // Don't unexpectedly change the user's incoming values array:
  std::deque<std::shared_ptr<int64_t>> sortedValues = values::clone();
  Arrays::sort(sortedValues);

  shared_ptr<BytesRef> *const encoded =
      make_shared<BytesRef>(std::deque<char>(BYTES));

  return make_shared<PointInSetQuery>(
      field, 1, BYTES,
      make_shared<StreamAnonymousInnerClass>(sortedValues, encoded)){
    protected : wstring toString(char[] value){assert(value->length == BYTES);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return decodeDimension(value, 0)->toString();
}
}; // namespace org::apache::lucene::document
}

BigIntegerPoint::StreamAnonymousInnerClass::StreamAnonymousInnerClass(
    deque<std::shared_ptr<int64_t>> &sortedValues,
    shared_ptr<BytesRef> encoded)
{
  this->sortedValues = sortedValues;
  this->encoded = encoded;
}

shared_ptr<BytesRef> BigIntegerPoint::StreamAnonymousInnerClass::next()
{
  if (upto == sortedValues.size()) {
    return nullptr;
  } else {
    encodeDimension(sortedValues[upto], encoded->bytes, 0);
    upto++;
    return encoded;
  }
}
}