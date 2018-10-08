using namespace std;

#include "IntRange.h"

namespace org::apache::lucene::document
{
using QueryType = org::apache::lucene::document::RangeFieldQuery::QueryType;
using Query = org::apache::lucene::search::Query;
using BytesRef = org::apache::lucene::util::BytesRef;
using FutureObjects = org::apache::lucene::util::FutureObjects;
using NumericUtils = org::apache::lucene::util::NumericUtils;

IntRange::IntRange(const wstring &name, std::deque<int> &min,
                   std::deque<int> &max)
    : Field(name, getType(min.length))
{
  setRangeValues(min, max);
}

shared_ptr<FieldType> IntRange::getType(int dimensions)
{
  if (dimensions > 4) {
    throw invalid_argument(
        L"IntRange does not support greater than 4 dimensions");
  }

  shared_ptr<FieldType> ft = make_shared<FieldType>();
  // dimensions is set as 2*dimension size (min/max per dimension)
  ft->setDimensions(dimensions * 2, BYTES);
  ft->freeze();
  return ft;
}

void IntRange::setRangeValues(std::deque<int> &min, std::deque<int> &max)
{
  checkArgs(min, max);
  if (min.size() * 2 != type->pointDimensionCount() ||
      max.size() * 2 != type->pointDimensionCount()) {
    throw invalid_argument(L"field (name=" + name_ + L") uses " +
                           to_wstring(type->pointDimensionCount() / 2) +
                           L" dimensions; cannot change to (incoming) " +
                           min.size() + L" dimensions");
  }

  const std::deque<char> bytes;
  if (fieldsData == nullptr) {
    bytes = std::deque<char>(BYTES * 2 * min.size());
    fieldsData = make_shared<BytesRef>(bytes);
  } else {
    bytes = (any_cast<std::shared_ptr<BytesRef>>(fieldsData)).bytes;
  }
  verifyAndEncode(min, max, bytes);
}

void IntRange::checkArgs(std::deque<int> &min, std::deque<int> &max)
{
  if (min.empty() || max.empty() || min.empty() || max.empty()) {
    throw invalid_argument(L"min/max range values cannot be null or empty");
  }
  if (min.size() != max.size()) {
    throw invalid_argument(L"min/max ranges must agree");
  }
  if (min.size() > 4) {
    throw invalid_argument(
        L"IntRange does not support greater than 4 dimensions");
  }
}

std::deque<char> IntRange::encode(std::deque<int> &min, std::deque<int> &max)
{
  checkArgs(min, max);
  std::deque<char> b(BYTES * 2 * min.size());
  verifyAndEncode(min, max, b);
  return b;
}

void IntRange::verifyAndEncode(std::deque<int> &min, std::deque<int> &max,
                               std::deque<char> &bytes)
{
  for (int d = 0, i = 0, j = min.size() * BYTES; d < min.size();
       ++d, i += BYTES, j += BYTES) {
    if (isnan(min[d])) {
      throw invalid_argument(L"invalid min value (" + NAN + L")" +
                             L" in IntRange");
    }
    if (isnan(max[d])) {
      throw invalid_argument(L"invalid max value (" + NAN + L")" +
                             L" in IntRange");
    }
    if (min[d] > max[d]) {
      throw invalid_argument(L"min value (" + to_wstring(min[d]) +
                             L") is greater than max value (" +
                             to_wstring(max[d]) + L")");
    }
    encode(min[d], bytes, i);
    encode(max[d], bytes, j);
  }
}

void IntRange::encode(int val, std::deque<char> &bytes, int offset)
{
  NumericUtils::intToSortableBytes(val, bytes, offset);
}

int IntRange::getMin(int dimension)
{
  FutureObjects::checkIndex(dimension, type->pointDimensionCount() / 2);
  return decodeMin((any_cast<std::shared_ptr<BytesRef>>(fieldsData)).bytes,
                   dimension);
}

int IntRange::getMax(int dimension)
{
  FutureObjects::checkIndex(dimension, type->pointDimensionCount() / 2);
  return decodeMax((any_cast<std::shared_ptr<BytesRef>>(fieldsData)).bytes,
                   dimension);
}

int IntRange::decodeMin(std::deque<char> &b, int dimension)
{
  int offset = dimension * BYTES;
  return NumericUtils::sortableBytesToInt(b, offset);
}

int IntRange::decodeMax(std::deque<char> &b, int dimension)
{
  int offset = b.size() / 2 + dimension * BYTES;
  return NumericUtils::sortableBytesToInt(b, offset);
}

shared_ptr<Query> IntRange::newIntersectsQuery(const wstring &field,
                                               std::deque<int> &min,
                                               std::deque<int> &max)
{
  return newRelationQuery(field, min, max, QueryType::INTERSECTS);
}

shared_ptr<Query> IntRange::newContainsQuery(const wstring &field,
                                             std::deque<int> &min,
                                             std::deque<int> &max)
{
  return newRelationQuery(field, min, max, QueryType::CONTAINS);
}

shared_ptr<Query> IntRange::newWithinQuery(const wstring &field,
                                           std::deque<int> &min,
                                           std::deque<int> &max)
{
  return newRelationQuery(field, min, max, QueryType::WITHIN);
}

shared_ptr<Query> IntRange::newCrossesQuery(const wstring &field,
                                            std::deque<int> &min,
                                            std::deque<int> &max)
{
  return newRelationQuery(field, min, max, QueryType::CROSSES);
}

shared_ptr<Query> IntRange::newRelationQuery(const wstring &field,
                                             std::deque<int> &min,
                                             std::deque<int> &max,
                                             QueryType relation)
{
  checkArgs(min, max);
  return make_shared<RangeFieldQueryAnonymousInnerClass>(
      field, encode(min, max), min.size(), relation);
}

IntRange::RangeFieldQueryAnonymousInnerClass::
    RangeFieldQueryAnonymousInnerClass(const wstring &field,
                                       deque<char> &encode,
                                       shared_ptr<UnknownType> length,
                                       QueryType relation)
    : RangeFieldQuery(field, encode, length, relation)
{
}

wstring IntRange::RangeFieldQueryAnonymousInnerClass::toString(
    std::deque<char> &ranges, int dimension)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return IntRange::toString(ranges, dimension);
}

wstring IntRange::toString()
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(getClass().getSimpleName());
  sb->append(L" <");
  sb->append(name_);
  sb->append(L':');
  std::deque<char> b = (any_cast<std::shared_ptr<BytesRef>>(fieldsData)).bytes;
  toString(b, 0);
  for (int d = 0; d < type->pointDimensionCount() / 2; ++d) {
    sb->append(L' ');
    sb->append(toString(b, d));
  }
  sb->append(L'>');

  return sb->toString();
}

wstring IntRange::toString(std::deque<char> &ranges, int dimension)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"[" + Integer::toString(decodeMin(ranges, dimension)) +
         L" : "
         // C++ TODO: There is no native C++ equivalent to 'toString':
         + Integer::toString(decodeMax(ranges, dimension)) + L"]";
}
} // namespace org::apache::lucene::document