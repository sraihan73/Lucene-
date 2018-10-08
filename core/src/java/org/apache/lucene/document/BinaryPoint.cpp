using namespace std;

#include "BinaryPoint.h"

namespace org::apache::lucene::document
{
using IndexableFieldType = org::apache::lucene::index::IndexableFieldType;
using PointValues = org::apache::lucene::index::PointValues;
using MatchNoDocsQuery = org::apache::lucene::search::MatchNoDocsQuery;
using PointInSetQuery = org::apache::lucene::search::PointInSetQuery;
using PointRangeQuery = org::apache::lucene::search::PointRangeQuery;
using Query = org::apache::lucene::search::Query;
using BytesRef = org::apache::lucene::util::BytesRef;
using FutureArrays = org::apache::lucene::util::FutureArrays;

shared_ptr<FieldType>
BinaryPoint::getType(std::deque<std::deque<char>> &point)
{
  if (point.empty()) {
    throw invalid_argument(L"point must not be null");
  }
  if (point.empty()) {
    throw invalid_argument(L"point must not be 0 dimensions");
  }
  int bytesPerDim = -1;
  for (int i = 0; i < point.size(); i++) {
    std::deque<char> oneDim = point[i];
    if (oneDim.empty()) {
      throw invalid_argument(L"point must not have null values");
    }
    if (oneDim.empty()) {
      throw invalid_argument(L"point must not have 0-length values");
    }
    if (bytesPerDim == -1) {
      bytesPerDim = oneDim.size();
    } else if (bytesPerDim != oneDim.size()) {
      throw invalid_argument(
          L"all dimensions must have same bytes length; got " +
          to_wstring(bytesPerDim) + L" and " + oneDim.size());
    }
  }
  return getType(point.size(), bytesPerDim);
}

shared_ptr<FieldType> BinaryPoint::getType(int numDims, int bytesPerDim)
{
  shared_ptr<FieldType> type = make_shared<FieldType>();
  type->setDimensions(numDims, bytesPerDim);
  type->freeze();
  return type;
}

shared_ptr<BytesRef> BinaryPoint::pack(deque<char> &point)
{
  if (point == nullptr) {
    throw invalid_argument(L"point must not be null");
  }
  if (point->length == 0) {
    throw invalid_argument(L"point must not be 0 dimensions");
  }
  if (point->length == 1) {
    return make_shared<BytesRef>(point[0]);
  }
  int bytesPerDim = -1;
  for (char[] dim : point) {
    if (dim.empty()) {
      throw invalid_argument(L"point must not have null values");
    }
    if (bytesPerDim == -1) {
      if (dim.empty()) {
        throw invalid_argument(L"point must not have 0-length values");
      }
      bytesPerDim = dim.size();
    } else if (dim.size() != bytesPerDim) {
      throw invalid_argument(
          L"all dimensions must have same bytes length; got " +
          to_wstring(bytesPerDim) + L" and " + dim.size());
    }
  }
  std::deque<char> packed(bytesPerDim * point->length);
  for (int i = 0; i < point->length; i++) {
    System::arraycopy(point[i], 0, packed, i * bytesPerDim, bytesPerDim);
  }
  return make_shared<BytesRef>(packed);
}

BinaryPoint::BinaryPoint(const wstring &name, deque<char> &point)
    : Field(name, pack(point), getType(point))
{
}

BinaryPoint::BinaryPoint(const wstring &name, std::deque<char> &packedPoint,
                         shared_ptr<IndexableFieldType> type)
    : Field(name, packedPoint, type)
{
  if (packedPoint.size() !=
      type->pointDimensionCount() * type->pointNumBytes()) {
    throw invalid_argument(L"packedPoint is length=" + packedPoint.size() +
                           L" but type.pointDimensionCount()=" +
                           to_wstring(type->pointDimensionCount()) +
                           L" and type.pointNumBytes()=" +
                           to_wstring(type->pointNumBytes()));
  }
}

shared_ptr<Query> BinaryPoint::newExactQuery(const wstring &field,
                                             std::deque<char> &value)
{
  return newRangeQuery(field, value, value);
}

shared_ptr<Query> BinaryPoint::newRangeQuery(const wstring &field,
                                             std::deque<char> &lowerValue,
                                             std::deque<char> &upperValue)
{
  PointRangeQuery::checkArgs(field, lowerValue, upperValue);
  return newRangeQuery(field, std::deque<std::deque<char>>{lowerValue},
                       std::deque<std::deque<char>>{upperValue});
}

shared_ptr<Query>
BinaryPoint::newRangeQuery(const wstring &field,
                           std::deque<std::deque<char>> &lowerValue,
                           std::deque<std::deque<char>> &upperValue)
{
  return make_shared<PointRangeQueryAnonymousInnerClass>(
      field, pack(lowerValue)->bytes, pack(upperValue)->bytes,
      lowerValue.size());
}

BinaryPoint::PointRangeQueryAnonymousInnerClass::
    PointRangeQueryAnonymousInnerClass(const wstring &field,
                                       deque<char> &bytes, deque<char> &bytes,
                                       shared_ptr<UnknownType> length)
    : org::apache::lucene::search::PointRangeQuery(field, bytes, bytes, length)
{
}

wstring BinaryPoint::PointRangeQueryAnonymousInnerClass::toString(
    int dimension, std::deque<char> &value)
{
  assert(value.size() > 0);
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L"binary(");
  for (int i = 0; i < value.size(); i++) {
    if (i > 0) {
      sb->append(L' ');
    }
    sb->append(Integer::toHexString(value[i] & 0xFF));
  }
  sb->append(L')');
  return sb->toString();
}

shared_ptr<Query> BinaryPoint::newSetQuery(const wstring &field,
                                           deque<char> &values)
{

  // Make sure all byte[] have the same length
  int bytesPerDim = -1;
  for (char[] value : values) {
    if (bytesPerDim == -1) {
      bytesPerDim = value.size();
    } else if (value.size() != bytesPerDim) {
      throw invalid_argument(L"all byte[] must be the same length, but saw " +
                             to_wstring(bytesPerDim) + L" and " + value.size());
    }
  }

  if (bytesPerDim == -1) {
    // There are no points, and we cannot guess the bytesPerDim here, so we
    // return an equivalent query:
    return make_shared<MatchNoDocsQuery>(L"empty BinaryPoint.newSetQuery");
  }

  // Don't unexpectedly change the user's incoming values array:
  std::deque<std::deque<char>> sortedValues = values::clone();
  Arrays::sort(sortedValues, make_shared<ComparatorAnonymousInnerClass>());

  shared_ptr<BytesRef> *const encoded =
      make_shared<BytesRef>(std::deque<char>(bytesPerDim));

  return make_shared<PointInSetQuery>(
      field, 1, bytesPerDim,
      make_shared<StreamAnonymousInnerClass>(sortedValues, encoded)){
    protected : wstring toString(char[] value){
        // C++ TODO: There is no native C++ equivalent to 'toString':
        return (make_shared<BytesRef>(value))->toString();
}
}; // namespace org::apache::lucene::document
}

BinaryPoint::ComparatorAnonymousInnerClass::ComparatorAnonymousInnerClass() {}

int BinaryPoint::ComparatorAnonymousInnerClass::compare(std::deque<char> &a,
                                                        std::deque<char> &b)
{
  return FutureArrays::compareUnsigned(a, 0, a.size(), b, 0, b.size());
}

BinaryPoint::StreamAnonymousInnerClass::StreamAnonymousInnerClass(
    deque<deque<char>> &sortedValues, shared_ptr<BytesRef> encoded)
{
  this->sortedValues = sortedValues;
  this->encoded = encoded;
}

shared_ptr<BytesRef> BinaryPoint::StreamAnonymousInnerClass::next()
{
  if (upto == sortedValues.size()) {
    return nullptr;
  } else {
    encoded->bytes = sortedValues[upto];
    upto++;
    return encoded;
  }
}
}