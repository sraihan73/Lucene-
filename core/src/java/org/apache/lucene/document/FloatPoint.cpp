using namespace std;

#include "FloatPoint.h"

namespace org::apache::lucene::document
{
using PointValues = org::apache::lucene::index::PointValues;
using PointInSetQuery = org::apache::lucene::search::PointInSetQuery;
using PointRangeQuery = org::apache::lucene::search::PointRangeQuery;
using Query = org::apache::lucene::search::Query;
using BytesRef = org::apache::lucene::util::BytesRef;
using NumericUtils = org::apache::lucene::util::NumericUtils;

float FloatPoint::nextUp(float f)
{
  if (Float::floatToIntBits(f) == 0x8000'0000) { // -0f
    return +0.0f;
  }
  return Math::nextUp(f);
}

float FloatPoint::nextDown(float f)
{
  if (Float::floatToIntBits(f) == 0) { // +0f
    return -0.0f;
  }
  return Math::nextDown(f);
}

shared_ptr<FieldType> FloatPoint::getType(int numDims)
{
  shared_ptr<FieldType> type = make_shared<FieldType>();
  type->setDimensions(numDims, Float::BYTES);
  type->freeze();
  return type;
}

void FloatPoint::setFloatValue(float value) { setFloatValues({value}); }

void FloatPoint::setFloatValues(deque<float> &point)
{
  if (type->pointDimensionCount() != point->length) {
    throw invalid_argument(L"this field (name=" + name_ + L") uses " +
                           to_wstring(type->pointDimensionCount()) +
                           L" dimensions; cannot change to (incoming) " +
                           point->length + L" dimensions");
  }
  fieldsData = pack({point});
}

void FloatPoint::setBytesValue(shared_ptr<BytesRef> bytes)
{
  throw invalid_argument(L"cannot change value type from float to BytesRef");
}

shared_ptr<Number> FloatPoint::numericValue()
{
  if (type->pointDimensionCount() != 1) {
    throw make_shared<IllegalStateException>(
        L"this field (name=" + name_ + L") uses " +
        to_wstring(type->pointDimensionCount()) +
        L" dimensions; cannot convert to a single numeric value");
  }
  shared_ptr<BytesRef> bytes = any_cast<std::shared_ptr<BytesRef>>(fieldsData);
  assert(bytes->length == Float::BYTES);
  return decodeDimension(bytes->bytes, bytes->offset);
}

shared_ptr<BytesRef> FloatPoint::pack(deque<float> &point)
{
  if (point == nullptr) {
    throw invalid_argument(L"point must not be null");
  }
  if (point->length == 0) {
    throw invalid_argument(L"point must not be 0 dimensions");
  }
  std::deque<char> packed(point->length * Float::BYTES);

  for (int dim = 0; dim < point->length; dim++) {
    encodeDimension(point[dim], packed, dim * Float::BYTES);
  }

  return make_shared<BytesRef>(packed);
}

FloatPoint::FloatPoint(const wstring &name, deque<float> &point)
    : Field(name, pack(point), getType(point->length))
{
}

wstring FloatPoint::toString()
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
        decodeDimension(bytes->bytes, bytes->offset + dim * Float::BYTES));
  }

  result->append(L'>');
  return result->toString();
}

void FloatPoint::encodeDimension(float value, std::deque<char> &dest,
                                 int offset)
{
  NumericUtils::intToSortableBytes(NumericUtils::floatToSortableInt(value),
                                   dest, offset);
}

float FloatPoint::decodeDimension(std::deque<char> &value, int offset)
{
  return NumericUtils::sortableIntToFloat(
      NumericUtils::sortableBytesToInt(value, offset));
}

shared_ptr<Query> FloatPoint::newExactQuery(const wstring &field, float value)
{
  return newRangeQuery(field, value, value);
}

shared_ptr<Query> FloatPoint::newRangeQuery(const wstring &field,
                                            float lowerValue, float upperValue)
{
  return newRangeQuery(field, std::deque<float>{lowerValue},
                       std::deque<float>{upperValue});
}

shared_ptr<Query> FloatPoint::newRangeQuery(const wstring &field,
                                            std::deque<float> &lowerValue,
                                            std::deque<float> &upperValue)
{
  PointRangeQuery::checkArgs(field, lowerValue, upperValue);
  return make_shared<PointRangeQueryAnonymousInnerClass>(
      field, pack(lowerValue)->bytes, pack(upperValue)->bytes,
      lowerValue.size());
}

FloatPoint::PointRangeQueryAnonymousInnerClass::
    PointRangeQueryAnonymousInnerClass(const wstring &field,
                                       deque<char> &bytes, deque<char> &bytes,
                                       shared_ptr<UnknownType> length)
    : org::apache::lucene::search::PointRangeQuery(field, bytes, bytes, length)
{
}

wstring FloatPoint::PointRangeQueryAnonymousInnerClass::toString(
    int dimension, std::deque<char> &value)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Float::toString(decodeDimension(value, 0));
}

shared_ptr<Query> FloatPoint::newSetQuery(const wstring &field,
                                          deque<float> &values)
{

  // Don't unexpectedly change the user's incoming values array:
  std::deque<float> sortedValues = values::clone();
  Arrays::sort(sortedValues);

  shared_ptr<BytesRef> *const encoded =
      make_shared<BytesRef>(std::deque<char>(Float::BYTES));

  return make_shared<PointInSetQuery>(
      field, 1, Float::BYTES,
      make_shared<StreamAnonymousInnerClass>(sortedValues, encoded)){
    protected :
        wstring toString(char[] value){assert(value->length == Float::BYTES);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Float::toString(decodeDimension(value, 0));
}
}; // namespace org::apache::lucene::document
}

FloatPoint::StreamAnonymousInnerClass::StreamAnonymousInnerClass(
    deque<float> &sortedValues, shared_ptr<BytesRef> encoded)
{
  this->sortedValues = sortedValues;
  this->encoded = encoded;
}

shared_ptr<BytesRef> FloatPoint::StreamAnonymousInnerClass::next()
{
  if (upto == sortedValues.size()) {
    return nullptr;
  } else {
    encodeDimension(sortedValues[upto], encoded->bytes, 0);
    upto++;
    return encoded;
  }
}

shared_ptr<Query> FloatPoint::newSetQuery(const wstring &field,
                                          shared_ptr<deque<float>> values)
{
  std::deque<optional<float>> boxed =
      values->toArray(std::deque<optional<float>>(0));
  std::deque<float> unboxed(boxed.size());
  for (int i = 0; i < boxed.size(); i++) {
    unboxed[i] = boxed[i];
  }
  return newSetQuery(field, unboxed);
}
}