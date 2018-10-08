using namespace std;

#include "IntPoint.h"

namespace org::apache::lucene::document
{
using PointValues = org::apache::lucene::index::PointValues;
using PointInSetQuery = org::apache::lucene::search::PointInSetQuery;
using PointRangeQuery = org::apache::lucene::search::PointRangeQuery;
using Query = org::apache::lucene::search::Query;
using BytesRef = org::apache::lucene::util::BytesRef;
using NumericUtils = org::apache::lucene::util::NumericUtils;

shared_ptr<FieldType> IntPoint::getType(int numDims)
{
  shared_ptr<FieldType> type = make_shared<FieldType>();
  type->setDimensions(numDims, Integer::BYTES);
  type->freeze();
  return type;
}

void IntPoint::setIntValue(int value) { setIntValues({value}); }

void IntPoint::setIntValues(deque<int> &point)
{
  if (type->pointDimensionCount() != point->length) {
    throw invalid_argument(L"this field (name=" + name_ + L") uses " +
                           to_wstring(type->pointDimensionCount()) +
                           L" dimensions; cannot change to (incoming) " +
                           point->length + L" dimensions");
  }
  fieldsData = pack({point});
}

void IntPoint::setBytesValue(shared_ptr<BytesRef> bytes)
{
  throw invalid_argument(L"cannot change value type from int to BytesRef");
}

shared_ptr<Number> IntPoint::numericValue()
{
  if (type->pointDimensionCount() != 1) {
    throw make_shared<IllegalStateException>(
        L"this field (name=" + name_ + L") uses " +
        to_wstring(type->pointDimensionCount()) +
        L" dimensions; cannot convert to a single numeric value");
  }
  shared_ptr<BytesRef> bytes = any_cast<std::shared_ptr<BytesRef>>(fieldsData);
  assert(bytes->length == Integer::BYTES);
  return decodeDimension(bytes->bytes, bytes->offset);
}

shared_ptr<BytesRef> IntPoint::pack(deque<int> &point)
{
  if (point == nullptr) {
    throw invalid_argument(L"point must not be null");
  }
  if (point->length == 0) {
    throw invalid_argument(L"point must not be 0 dimensions");
  }
  std::deque<char> packed(point->length * Integer::BYTES);

  for (int dim = 0; dim < point->length; dim++) {
    encodeDimension(point[dim], packed, dim * Integer::BYTES);
  }

  return make_shared<BytesRef>(packed);
}

IntPoint::IntPoint(const wstring &name, deque<int> &point)
    : Field(name, pack(point), getType(point->length))
{
}

wstring IntPoint::toString()
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
        decodeDimension(bytes->bytes, bytes->offset + dim * Integer::BYTES));
  }

  result->append(L'>');
  return result->toString();
}

void IntPoint::encodeDimension(int value, std::deque<char> &dest, int offset)
{
  NumericUtils::intToSortableBytes(value, dest, offset);
}

int IntPoint::decodeDimension(std::deque<char> &value, int offset)
{
  return NumericUtils::sortableBytesToInt(value, offset);
}

shared_ptr<Query> IntPoint::newExactQuery(const wstring &field, int value)
{
  return newRangeQuery(field, value, value);
}

shared_ptr<Query> IntPoint::newRangeQuery(const wstring &field, int lowerValue,
                                          int upperValue)
{
  return newRangeQuery(field, std::deque<int>{lowerValue},
                       std::deque<int>{upperValue});
}

shared_ptr<Query> IntPoint::newRangeQuery(const wstring &field,
                                          std::deque<int> &lowerValue,
                                          std::deque<int> &upperValue)
{
  PointRangeQuery::checkArgs(field, lowerValue, upperValue);
  return make_shared<PointRangeQueryAnonymousInnerClass>(
      field, pack(lowerValue)->bytes, pack(upperValue)->bytes,
      lowerValue.size());
}

IntPoint::PointRangeQueryAnonymousInnerClass::
    PointRangeQueryAnonymousInnerClass(const wstring &field,
                                       deque<char> &bytes, deque<char> &bytes,
                                       shared_ptr<UnknownType> length)
    : org::apache::lucene::search::PointRangeQuery(field, bytes, bytes, length)
{
}

wstring
IntPoint::PointRangeQueryAnonymousInnerClass::toString(int dimension,
                                                       std::deque<char> &value)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Integer::toString(decodeDimension(value, 0));
}

shared_ptr<Query> IntPoint::newSetQuery(const wstring &field,
                                        deque<int> &values)
{

  // Don't unexpectedly change the user's incoming values array:
  std::deque<int> sortedValues = values::clone();
  Arrays::sort(sortedValues);

  shared_ptr<BytesRef> *const encoded =
      make_shared<BytesRef>(std::deque<char>(Integer::BYTES));

  return make_shared<PointInSetQuery>(
      field, 1, Integer::BYTES,
      make_shared<StreamAnonymousInnerClass>(sortedValues, encoded)){
    protected :
        wstring toString(char[] value){assert(value->length == Integer::BYTES);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Integer::toString(decodeDimension(value, 0));
}
}; // namespace org::apache::lucene::document
}

IntPoint::StreamAnonymousInnerClass::StreamAnonymousInnerClass(
    deque<int> &sortedValues, shared_ptr<BytesRef> encoded)
{
  this->sortedValues = sortedValues;
  this->encoded = encoded;
}

shared_ptr<BytesRef> IntPoint::StreamAnonymousInnerClass::next()
{
  if (upto == sortedValues.size()) {
    return nullptr;
  } else {
    encodeDimension(sortedValues[upto], encoded->bytes, 0);
    upto++;
    return encoded;
  }
}

shared_ptr<Query> IntPoint::newSetQuery(const wstring &field,
                                        shared_ptr<deque<int>> values)
{
  std::deque<optional<int>> boxed =
      values->toArray(std::deque<optional<int>>(0));
  std::deque<int> unboxed(boxed.size());
  for (int i = 0; i < boxed.size(); i++) {
    unboxed[i] = boxed[i];
  }
  return newSetQuery(field, unboxed);
}
}