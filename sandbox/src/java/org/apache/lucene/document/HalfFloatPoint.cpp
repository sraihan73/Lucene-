using namespace std;

#include "HalfFloatPoint.h"

namespace org::apache::lucene::document
{
using PointValues = org::apache::lucene::index::PointValues;
using PointInSetQuery = org::apache::lucene::search::PointInSetQuery;
using PointRangeQuery = org::apache::lucene::search::PointRangeQuery;
using Query = org::apache::lucene::search::Query;
using BytesRef = org::apache::lucene::util::BytesRef;

float HalfFloatPoint::nextUp(float v)
{
  if (isnan(v) || v == numeric_limits<float>::infinity()) {
    return v;
  }
  short s = halfFloatToSortableShort(v);
  // if the float does not represent a half float accurately then just
  // converting back might give us the value we are looking for
  float r = sortableShortToHalfFloat(s);
  if (r <= v) {
    r = sortableShortToHalfFloat(static_cast<short>(s + 1));
  }
  return r;
}

float HalfFloatPoint::nextDown(float v)
{
  if (isnan(v) || v == -numeric_limits<float>::infinity()) {
    return v;
  }
  short s = halfFloatToSortableShort(v);
  // if the float does not represent a half float accurately then just
  // converting back might give us the value we are looking for
  float r = sortableShortToHalfFloat(s);
  if (r >= v) {
    r = sortableShortToHalfFloat(static_cast<short>(s - 1));
  }
  return r;
}

short HalfFloatPoint::halfFloatToSortableShort(float v)
{
  return sortableShortBits(halfFloatToShortBits(v));
}

float HalfFloatPoint::sortableShortToHalfFloat(short bits)
{
  return shortBitsToHalfFloat(sortableShortBits(bits));
}

short HalfFloatPoint::sortableShortBits(short s)
{
  return static_cast<short>(s ^ (s >> 15) & 0x7fff);
}

short HalfFloatPoint::halfFloatToShortBits(float v)
{
  int floatBits = Float::floatToIntBits(v);
  int sign = static_cast<int>(static_cast<unsigned int>(floatBits) >> 31);
  int exp =
      (static_cast<int>(static_cast<unsigned int>(floatBits) >> 23)) & 0xff;
  int mantissa = floatBits & 0x7fffff;

  if (exp == 0xff) {
    // preserve NaN and Infinity
    exp = 0x1f;
    mantissa =
        static_cast<int>(static_cast<unsigned int>(mantissa) >> (23 - 10));
  } else if (exp == 0x00) {
    // denormal float rounded to zero since even the largest denormal float
    // cannot be represented as a half float
    mantissa = 0;
  } else {
    exp = exp - 127 + 15;
    if (exp >= 0x1f) {
      // too large, make it infinity
      exp = 0x1f;
      mantissa = 0;
    } else if (exp <= 0) {
      // we need to convert to a denormal representation
      int shift = 23 - 10 - exp + 1;
      if (shift >= 32) {
        // need a special case since shifts are mod 32...
        exp = 0;
        mantissa = 0;
      } else {
        // add the implicit bit
        mantissa |= 0x800000;
        mantissa = roundShift(mantissa, shift);
        exp = static_cast<int>(static_cast<unsigned int>(mantissa) >> 10);
        mantissa &= 0x3ff;
      }
    } else {
      mantissa = roundShift((exp << 23) | mantissa, 23 - 10);
      exp = static_cast<int>(static_cast<unsigned int>(mantissa) >> 10);
      mantissa &= 0x3ff;
    }
  }
  return static_cast<short>((sign << 15) | (exp << 10) | mantissa);
}

int HalfFloatPoint::roundShift(int i, int shift)
{
  assert(shift > 0);
  i +=
      1 << (shift - 1); // add 2^(shift-1) so that we round rather than truncate
  i -= (static_cast<int>(static_cast<unsigned int>(i) >> shift)) &
       1; // and subtract the shift-th bit so that we round to even in case of
          // tie
  return static_cast<int>(static_cast<unsigned int>(i) >> shift);
}

float HalfFloatPoint::shortBitsToHalfFloat(short s)
{
  int sign = static_cast<short>(static_cast<unsigned short>(s) >> 15);
  int exp = (static_cast<short>(static_cast<unsigned short>(s) >> 10)) & 0x1f;
  int mantissa = s & 0x3ff;
  if (exp == 0x1f) {
    // NaN or infinities
    exp = 0xff;
    mantissa <<= (23 - 10);
  } else if (mantissa == 0 && exp == 0) {
    // zero
  } else {
    if (exp == 0) {
      // denormal half float becomes a normal float
      int shift = Integer::numberOfLeadingZeros(mantissa) - (32 - 11);
      mantissa = (mantissa << shift) & 0x3ff; // clear the implicit bit
      exp = exp - shift + 1;
    }
    exp = exp + 127 - 15;
    mantissa <<= (23 - 10);
  }

  return Float::intBitsToFloat((sign << 31) | (exp << 23) | mantissa);
}

void HalfFloatPoint::shortToSortableBytes(short value,
                                          std::deque<char> &result, int offset)
{
  // Flip the sign bit, so negative shorts sort before positive shorts
  // correctly:
  value ^= 0x8000;
  result[offset] = static_cast<char>(value >> 8);
  result[offset + 1] = static_cast<char>(value);
}

short HalfFloatPoint::sortableBytesToShort(std::deque<char> &encoded,
                                           int offset)
{
  short x = static_cast<short>(((encoded[offset] & 0xFF) << 8) |
                               (encoded[offset + 1] & 0xFF));
  // Re-flip the sign bit to restore the original value:
  return static_cast<short>(x ^ 0x8000);
}

shared_ptr<FieldType> HalfFloatPoint::getType(int numDims)
{
  shared_ptr<FieldType> type = make_shared<FieldType>();
  type->setDimensions(numDims, BYTES);
  type->freeze();
  return type;
}

void HalfFloatPoint::setFloatValue(float value) { setFloatValues({value}); }

void HalfFloatPoint::setFloatValues(deque<float> &point)
{
  if (type->pointDimensionCount() != point->length) {
    throw invalid_argument(L"this field (name=" + name_ + L") uses " +
                           to_wstring(type->pointDimensionCount()) +
                           L" dimensions; cannot change to (incoming) " +
                           point->length + L" dimensions");
  }
  fieldsData = pack({point});
}

void HalfFloatPoint::setBytesValue(shared_ptr<BytesRef> bytes)
{
  throw invalid_argument(L"cannot change value type from float to BytesRef");
}

shared_ptr<Number> HalfFloatPoint::numericValue()
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

shared_ptr<BytesRef> HalfFloatPoint::pack(deque<float> &point)
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

HalfFloatPoint::HalfFloatPoint(const wstring &name, deque<float> &point)
    : Field(name, pack(point), getType(point->length))
{
}

wstring HalfFloatPoint::toString()
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

void HalfFloatPoint::encodeDimension(float value, std::deque<char> &dest,
                                     int offset)
{
  shortToSortableBytes(halfFloatToSortableShort(value), dest, offset);
}

float HalfFloatPoint::decodeDimension(std::deque<char> &value, int offset)
{
  return sortableShortToHalfFloat(sortableBytesToShort(value, offset));
}

shared_ptr<Query> HalfFloatPoint::newExactQuery(const wstring &field,
                                                float value)
{
  return newRangeQuery(field, value, value);
}

shared_ptr<Query> HalfFloatPoint::newRangeQuery(const wstring &field,
                                                float lowerValue,
                                                float upperValue)
{
  return newRangeQuery(field, std::deque<float>{lowerValue},
                       std::deque<float>{upperValue});
}

shared_ptr<Query> HalfFloatPoint::newRangeQuery(const wstring &field,
                                                std::deque<float> &lowerValue,
                                                std::deque<float> &upperValue)
{
  PointRangeQuery::checkArgs(field, lowerValue, upperValue);
  return make_shared<PointRangeQueryAnonymousInnerClass>(
      field, pack(lowerValue)->bytes, pack(upperValue)->bytes,
      lowerValue.size());
}

HalfFloatPoint::PointRangeQueryAnonymousInnerClass::
    PointRangeQueryAnonymousInnerClass(const wstring &field,
                                       deque<char> &bytes, deque<char> &bytes,
                                       shared_ptr<UnknownType> length)
    : org::apache::lucene::search::PointRangeQuery(field, bytes, bytes, length)
{
}

wstring HalfFloatPoint::PointRangeQueryAnonymousInnerClass::toString(
    int dimension, std::deque<char> &value)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Float::toString(decodeDimension(value, 0));
}

shared_ptr<Query> HalfFloatPoint::newSetQuery(const wstring &field,
                                              deque<float> &values)
{

  // Don't unexpectedly change the user's incoming values array:
  std::deque<float> sortedValues = values::clone();
  Arrays::sort(sortedValues);

  shared_ptr<BytesRef> *const encoded =
      make_shared<BytesRef>(std::deque<char>(BYTES));

  return make_shared<PointInSetQuery>(
      field, 1, BYTES,
      make_shared<StreamAnonymousInnerClass>(sortedValues, encoded)){
    protected : wstring toString(char[] value){assert(value->length == BYTES);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Float::toString(decodeDimension(value, 0));
}
}; // namespace org::apache::lucene::document
}

HalfFloatPoint::StreamAnonymousInnerClass::StreamAnonymousInnerClass(
    deque<float> &sortedValues, shared_ptr<BytesRef> encoded)
{
  this->sortedValues = sortedValues;
  this->encoded = encoded;
}

shared_ptr<BytesRef> HalfFloatPoint::StreamAnonymousInnerClass::next()
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
HalfFloatPoint::newSetQuery(const wstring &field,
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