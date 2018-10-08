using namespace std;

#include "InetAddressRange.h"

namespace org::apache::lucene::document
{
using QueryType = org::apache::lucene::document::RangeFieldQuery::QueryType;
using Query = org::apache::lucene::search::Query;
using BytesRef = org::apache::lucene::util::BytesRef;
using StringHelper = org::apache::lucene::util::StringHelper;
const shared_ptr<FieldType> InetAddressRange::TYPE;

InetAddressRange::StaticConstructor::StaticConstructor()
{
  TYPE = make_shared<FieldType>();
  TYPE->setDimensions(2, BYTES);
  TYPE->freeze();
}

InetAddressRange::StaticConstructor InetAddressRange::staticConstructor;

InetAddressRange::InetAddressRange(const wstring &name,
                                   shared_ptr<InetAddress> min,
                                   shared_ptr<InetAddress> max)
    : Field(name, TYPE)
{
  setRangeValues(min, max);
}

void InetAddressRange::setRangeValues(shared_ptr<InetAddress> min,
                                      shared_ptr<InetAddress> max)
{
  const std::deque<char> bytes;
  if (fieldsData == nullptr) {
    bytes = std::deque<char>(BYTES * 2);
    fieldsData = make_shared<BytesRef>(bytes);
  } else {
    bytes = (any_cast<std::shared_ptr<BytesRef>>(fieldsData)).bytes;
  }
  encode(min, max, bytes);
}

void InetAddressRange::encode(shared_ptr<InetAddress> min,
                              shared_ptr<InetAddress> max,
                              std::deque<char> &bytes)
{
  // encode min and max value (consistent w/ InetAddressPoint encoding)
  const std::deque<char> minEncoded = InetAddressPoint::encode(min);
  const std::deque<char> maxEncoded = InetAddressPoint::encode(max);
  // ensure min is lt max
  if (StringHelper::compare(BYTES, minEncoded, 0, maxEncoded, 0) > 0) {
    throw invalid_argument(L"min value cannot be greater than max value for "
                           L"InetAddressRange field");
  }
  System::arraycopy(minEncoded, 0, bytes, 0, BYTES);
  System::arraycopy(maxEncoded, 0, bytes, BYTES, BYTES);
}

std::deque<char> InetAddressRange::encode(shared_ptr<InetAddress> min,
                                           shared_ptr<InetAddress> max)
{
  std::deque<char> b(BYTES * 2);
  encode(min, max, b);
  return b;
}

shared_ptr<Query>
InetAddressRange::newIntersectsQuery(const wstring &field,
                                     shared_ptr<InetAddress> min,
                                     shared_ptr<InetAddress> max)
{
  return newRelationQuery(field, min, max, QueryType::INTERSECTS);
}

shared_ptr<Query>
InetAddressRange::newContainsQuery(const wstring &field,
                                   shared_ptr<InetAddress> min,
                                   shared_ptr<InetAddress> max)
{
  return newRelationQuery(field, min, max, QueryType::CONTAINS);
}

shared_ptr<Query> InetAddressRange::newWithinQuery(const wstring &field,
                                                   shared_ptr<InetAddress> min,
                                                   shared_ptr<InetAddress> max)
{
  return newRelationQuery(field, min, max, QueryType::WITHIN);
}

shared_ptr<Query> InetAddressRange::newCrossesQuery(const wstring &field,
                                                    shared_ptr<InetAddress> min,
                                                    shared_ptr<InetAddress> max)
{
  return newRelationQuery(field, min, max, QueryType::CROSSES);
}

shared_ptr<Query> InetAddressRange::newRelationQuery(
    const wstring &field, shared_ptr<InetAddress> min,
    shared_ptr<InetAddress> max, QueryType relation)
{
  return make_shared<RangeFieldQueryAnonymousInnerClass>(
      field, encode(min, max), relation);
}

InetAddressRange::RangeFieldQueryAnonymousInnerClass::
    RangeFieldQueryAnonymousInnerClass(const wstring &field,
                                       deque<char> &encode, QueryType relation)
    : RangeFieldQuery(field, encode, 1, relation)
{
}

wstring InetAddressRange::RangeFieldQueryAnonymousInnerClass::toString(
    std::deque<char> &ranges, int dimension)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return InetAddressRange::toString(ranges, dimension);
}

wstring InetAddressRange::toString(std::deque<char> &ranges, int dimension)
{
  std::deque<char> min(BYTES);
  System::arraycopy(ranges, 0, min, 0, BYTES);
  std::deque<char> max(BYTES);
  System::arraycopy(ranges, BYTES, max, 0, BYTES);
  return L"[" + InetAddressPoint::decode(min) + L" : " +
         InetAddressPoint::decode(max) + L"]";
}
} // namespace org::apache::lucene::document