using namespace std;

#include "DoublePoint.h"

namespace org::apache::lucene::document
{
using PointValues = org::apache::lucene::index::PointValues;
using PointInSetQuery = org::apache::lucene::search::PointInSetQuery;
using PointRangeQuery = org::apache::lucene::search::PointRangeQuery;
using Query = org::apache::lucene::search::Query;
using BytesRef = org::apache::lucene::util::BytesRef;
using NumericUtils = org::apache::lucene::util::NumericUtils;

double DoublePoint::nextUp(double d)
{
  if (Double::doubleToLongBits(d) == 0x8000'0000'0000'0000LL) { // -0d
    return +0;
  }
  return Math::nextUp(d);
}

double DoublePoint::nextDown(double d)
{
  if (Double::doubleToLongBits(d) == 0LL) { // +0d
    return -0.0f;
  }
  return Math::nextDown(d);
}

shared_ptr<FieldType> DoublePoint::getType(int numDims)
{
  shared_ptr<FieldType> type = make_shared<FieldType>();
  type->setDimensions(numDims, Double::BYTES);
  type->freeze();
  return type;
}

void DoublePoint::setDoubleValue(double value) { setDoubleValues({value}); }

void DoublePoint::setDoubleValues(deque<double> &point)
{
  if (type->pointDimensionCount() != point->length) {
    throw invalid_argument(L"this field (name=" + name_ + L") uses " +
                           to_wstring(type->pointDimensionCount()) +
                           L" dimensions; cannot change to (incoming) " +
                           point->length + L" dimensions");
  }
  fieldsData = pack({point});
}

void DoublePoint::setBytesValue(shared_ptr<BytesRef> bytes)
{
  throw invalid_argument(L"cannot change value type from double to BytesRef");
}

shared_ptr<Number> DoublePoint::numericValue()
{
  if (type->pointDimensionCount() != 1) {
    throw make_shared<IllegalStateException>(
        L"this field (name=" + name_ + L") uses " +
        to_wstring(type->pointDimensionCount()) +
        L" dimensions; cannot convert to a single numeric value");
  }
  shared_ptr<BytesRef> bytes = any_cast<std::shared_ptr<BytesRef>>(fieldsData);
  assert(bytes->length == Double::BYTES);
  return decodeDimension(bytes->bytes, bytes->offset);
}

shared_ptr<BytesRef> DoublePoint::pack(deque<double> &point)
{
  if (point == nullptr) {
    throw invalid_argument(L"point must not be null");
  }
  if (point->length == 0) {
    throw invalid_argument(L"point must not be 0 dimensions");
  }
  std::deque<char> packed(point->length * Double::BYTES);

  for (int dim = 0; dim < point->length; dim++) {
    encodeDimension(point[dim], packed, dim * Double::BYTES);
  }

  return make_shared<BytesRef>(packed);
}

DoublePoint::DoublePoint(const wstring &name, deque<double> &point)
    : Field(name, pack(point), getType(point->length))
{
}

wstring DoublePoint::toString()
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
        decodeDimension(bytes->bytes, bytes->offset + dim * Double::BYTES));
  }

  result->append(L'>');
  return result->toString();
}

void DoublePoint::encodeDimension(double value, std::deque<char> &dest,
                                  int offset)
{
  NumericUtils::longToSortableBytes(NumericUtils::doubleToSortableLong(value),
                                    dest, offset);
}

double DoublePoint::decodeDimension(std::deque<char> &value, int offset)
{
  return NumericUtils::sortableLongToDouble(
      NumericUtils::sortableBytesToLong(value, offset));
}

shared_ptr<Query> DoublePoint::newExactQuery(const wstring &field, double value)
{
  return newRangeQuery(field, value, value);
}

shared_ptr<Query> DoublePoint::newRangeQuery(const wstring &field,
                                             double lowerValue,
                                             double upperValue)
{
  return newRangeQuery(field, std::deque<double>{lowerValue},
                       std::deque<double>{upperValue});
}

shared_ptr<Query> DoublePoint::newRangeQuery(const wstring &field,
                                             std::deque<double> &lowerValue,
                                             std::deque<double> &upperValue)
{
  PointRangeQuery::checkArgs(field, lowerValue, upperValue);
  return make_shared<PointRangeQueryAnonymousInnerClass>(
      field, pack(lowerValue)->bytes, pack(upperValue)->bytes,
      lowerValue.size());
}

DoublePoint::PointRangeQueryAnonymousInnerClass::
    PointRangeQueryAnonymousInnerClass(const wstring &field,
                                       deque<char> &bytes, deque<char> &bytes,
                                       shared_ptr<UnknownType> length)
    : org::apache::lucene::search::PointRangeQuery(field, bytes, bytes, length)
{
}

wstring DoublePoint::PointRangeQueryAnonymousInnerClass::toString(
    int dimension, std::deque<char> &value)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Double::toString(decodeDimension(value, 0));
}

shared_ptr<Query> DoublePoint::newSetQuery(const wstring &field,
                                           deque<double> &values)
{

  // Don't unexpectedly change the user's incoming values array:
  std::deque<double> sortedValues = values::clone();
  Arrays::sort(sortedValues);

  shared_ptr<BytesRef> *const encoded =
      make_shared<BytesRef>(std::deque<char>(Double::BYTES));

  return make_shared<PointInSetQuery>(
      field, 1, Double::BYTES,
      make_shared<StreamAnonymousInnerClass>(sortedValues, encoded)){
    protected :
        wstring toString(char[] value){assert(value->length == Double::BYTES);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Double::toString(decodeDimension(value, 0));
}
}; // namespace org::apache::lucene::document
}

DoublePoint::StreamAnonymousInnerClass::StreamAnonymousInnerClass(
    deque<double> &sortedValues, shared_ptr<BytesRef> encoded)
{
  this->sortedValues = sortedValues;
  this->encoded = encoded;
}

shared_ptr<BytesRef> DoublePoint::StreamAnonymousInnerClass::next()
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
DoublePoint::newSetQuery(const wstring &field,
                         shared_ptr<deque<double>> values)
{
  std::deque<optional<double>> boxed =
      values->toArray(std::deque<optional<double>>(0));
  std::deque<double> unboxed(boxed.size());
  for (int i = 0; i < boxed.size(); i++) {
    unboxed[i] = boxed[i];
  }
  return newSetQuery(field, unboxed);
}
}