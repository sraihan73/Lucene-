using namespace std;

#include "InetAddressPoint.h"

namespace org::apache::lucene::document
{
using PointValues = org::apache::lucene::index::PointValues;
using PointInSetQuery = org::apache::lucene::search::PointInSetQuery;
using PointRangeQuery = org::apache::lucene::search::PointRangeQuery;
using Query = org::apache::lucene::search::Query;
using BytesRef = org::apache::lucene::util::BytesRef;
using NumericUtils = org::apache::lucene::util::NumericUtils;
using StringHelper = org::apache::lucene::util::StringHelper;
std::deque<char> const InetAddressPoint::IPV4_PREFIX =
    std::deque<char>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1};
const shared_ptr<FieldType> InetAddressPoint::TYPE;

InetAddressPoint::StaticConstructor::StaticConstructor()
{
  TYPE = make_shared<FieldType>();
  TYPE->setDimensions(1, BYTES);
  TYPE->freeze();
  MIN_VALUE = decode(std::deque<char>(BYTES));
  std::deque<char> maxValueBytes(BYTES);
  Arrays::fill(maxValueBytes, static_cast<char>(0xFF));
  MAX_VALUE = decode(maxValueBytes);
}

InetAddressPoint::StaticConstructor InetAddressPoint::staticConstructor;
const shared_ptr<java::net::InetAddress> InetAddressPoint::MIN_VALUE;
const shared_ptr<java::net::InetAddress> InetAddressPoint::MAX_VALUE;

shared_ptr<InetAddress>
InetAddressPoint::nextUp(shared_ptr<InetAddress> address)
{
  if (address->equals(MAX_VALUE)) {
    throw make_shared<ArithmeticException>(
        L"Overflow: there is no greater InetAddress than " +
        address->getHostAddress());
  }
  std::deque<char> delta(BYTES);
  delta[BYTES - 1] = 1;
  std::deque<char> nextUpBytes(InetAddressPoint::BYTES);
  NumericUtils::add(InetAddressPoint::BYTES, 0, encode(address), delta,
                    nextUpBytes);
  return decode(nextUpBytes);
}

shared_ptr<InetAddress>
InetAddressPoint::nextDown(shared_ptr<InetAddress> address)
{
  if (address->equals(MIN_VALUE)) {
    throw make_shared<ArithmeticException>(
        L"Underflow: there is no smaller InetAddress than " +
        address->getHostAddress());
  }
  std::deque<char> delta(BYTES);
  delta[BYTES - 1] = 1;
  std::deque<char> nextDownBytes(InetAddressPoint::BYTES);
  NumericUtils::subtract(InetAddressPoint::BYTES, 0, encode(address), delta,
                         nextDownBytes);
  return decode(nextDownBytes);
}

void InetAddressPoint::setInetAddressValue(shared_ptr<InetAddress> value)
{
  if (value == nullptr) {
    throw invalid_argument(L"point must not be null");
  }
  fieldsData = make_shared<BytesRef>(encode(value));
}

void InetAddressPoint::setBytesValue(shared_ptr<BytesRef> bytes)
{
  throw invalid_argument(
      L"cannot change value type from InetAddress to BytesRef");
}

InetAddressPoint::InetAddressPoint(const wstring &name,
                                   shared_ptr<InetAddress> point)
    : Field(name, TYPE)
{
  setInetAddressValue(point);
}

wstring InetAddressPoint::toString()
{
  shared_ptr<StringBuilder> result = make_shared<StringBuilder>();
  result->append(getClass().getSimpleName());
  result->append(L" <");
  result->append(name_);
  result->append(L':');

  // IPv6 addresses are bracketed, to not cause confusion with historic
  // field:value representation
  shared_ptr<BytesRef> bytes = any_cast<std::shared_ptr<BytesRef>>(fieldsData);
  shared_ptr<InetAddress> address = decode(BytesRef::deepCopyOf(bytes)->bytes);
  if (address->getAddress()->length == 16) {
    result->append(L'[');
    result->append(address->getHostAddress());
    result->append(L']');
  } else {
    result->append(address->getHostAddress());
  }

  result->append(L'>');
  return result->toString();
}

std::deque<char> InetAddressPoint::encode(shared_ptr<InetAddress> value)
{
  std::deque<char> address = value->getAddress();
  if (address.size() == 4) {
    std::deque<char> mapped(16);
    System::arraycopy(IPV4_PREFIX, 0, mapped, 0, IPV4_PREFIX.size());
    System::arraycopy(address, 0, mapped, IPV4_PREFIX.size(), address.size());
    address = mapped;
  } else if (address.size() != 16) {
    // more of an assertion, how did you create such an InetAddress :)
    throw make_shared<UnsupportedOperationException>(
        L"Only IPv4 and IPv6 addresses are supported");
  }
  return address;
}

shared_ptr<InetAddress> InetAddressPoint::decode(std::deque<char> &value)
{
  try {
    return InetAddress::getByAddress(value);
  } catch (const UnknownHostException &e) {
    // this only happens if value.length != 4 or 16, strange exception class
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new IllegalArgumentException("encoded bytes are of
    // incorrect length", e);
    throw invalid_argument(L"encoded bytes are of incorrect length");
  }
}

shared_ptr<Query> InetAddressPoint::newExactQuery(const wstring &field,
                                                  shared_ptr<InetAddress> value)
{
  return newRangeQuery(field, value, value);
}

shared_ptr<Query> InetAddressPoint::newPrefixQuery(
    const wstring &field, shared_ptr<InetAddress> value, int prefixLength)
{
  if (value == nullptr) {
    throw invalid_argument(L"InetAddress must not be null");
  }
  if (prefixLength < 0 || prefixLength > 8 * value->getAddress()->length) {
    throw invalid_argument(
        L"illegal prefixLength '" + to_wstring(prefixLength) +
        L"'. Must be 0-32 for IPv4 ranges, 0-128 for IPv6 ranges");
  }
  // create the lower value by zeroing out the host portion, upper value by
  // filling it with all ones.
  std::deque<char> lower = value->getAddress();
  std::deque<char> upper = value->getAddress();
  for (int i = prefixLength; i < 8 * lower.size(); i++) {
    int m = 1 << (7 - (i & 7));
    lower[i >> 3] &= ~m;
    upper[i >> 3] |= m;
  }
  try {
    return newRangeQuery(field, InetAddress::getByAddress(lower),
                         InetAddress::getByAddress(upper));
  } catch (const UnknownHostException &e) {
    throw make_shared<AssertionError>(e); // values are coming from InetAddress
  }
}

shared_ptr<Query>
InetAddressPoint::newRangeQuery(const wstring &field,
                                shared_ptr<InetAddress> lowerValue,
                                shared_ptr<InetAddress> upperValue)
{
  PointRangeQuery::checkArgs(field, lowerValue, upperValue);
  return make_shared<PointRangeQueryAnonymousInnerClass>(
      field, encode(lowerValue), encode(upperValue));
}

InetAddressPoint::PointRangeQueryAnonymousInnerClass::
    PointRangeQueryAnonymousInnerClass(const wstring &field,
                                       deque<char> &encode,
                                       deque<char> &encode)
    : org::apache::lucene::search::PointRangeQuery(field, encode, encode, 1)
{
}

wstring InetAddressPoint::PointRangeQueryAnonymousInnerClass::toString(
    int dimension, std::deque<char> &value)
{
  return decode(value)
      ->getHostAddress(); // for ranges, the range itself is already bracketed
}

shared_ptr<Query> InetAddressPoint::newSetQuery(const wstring &field,
                                                deque<InetAddress> &values)
{

  // We must compare the encoded form (InetAddress doesn't implement Comparable,
  // and even if it did, we do our own thing with ipv4 addresses):

  // NOTE: we could instead convert-per-comparison and save this extra array, at
  // cost of slower sort:
  std::deque<std::deque<char>> sortedValues(values->length);
  for (int i = 0; i < values->length; i++) {
    sortedValues[i] = encode(values[i]);
  }

  Arrays::sort(sortedValues, make_shared<ComparatorAnonymousInnerClass>());

  shared_ptr<BytesRef> *const encoded =
      make_shared<BytesRef>(std::deque<char>(BYTES));

  return make_shared<PointInSetQuery>(
      field, 1, BYTES,
      make_shared<StreamAnonymousInnerClass>(sortedValues, encoded)){
    protected : wstring toString(char[] value){assert(value->length == BYTES);
  return decode(value)->getHostAddress();
}
}; // namespace org::apache::lucene::document
}

InetAddressPoint::ComparatorAnonymousInnerClass::ComparatorAnonymousInnerClass()
{
}

int InetAddressPoint::ComparatorAnonymousInnerClass::compare(
    std::deque<char> &a, std::deque<char> &b)
{
  return StringHelper::compare(BYTES, a, 0, b, 0);
}

InetAddressPoint::StreamAnonymousInnerClass::StreamAnonymousInnerClass(
    deque<deque<char>> &sortedValues, shared_ptr<BytesRef> encoded)
{
  this->sortedValues = sortedValues;
  this->encoded = encoded;
}

shared_ptr<BytesRef> InetAddressPoint::StreamAnonymousInnerClass::next()
{
  if (upto == sortedValues.size()) {
    return nullptr;
  } else {
    encoded->bytes = sortedValues[upto];
    assert(encoded->bytes.size() == encoded->length);
    upto++;
    return encoded;
  }
}
}