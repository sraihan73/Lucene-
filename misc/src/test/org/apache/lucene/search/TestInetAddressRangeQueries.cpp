using namespace std;

#include "TestInetAddressRangeQueries.h"

namespace org::apache::lucene::search
{
using InetAddressPoint = org::apache::lucene::document::InetAddressPoint;
using InetAddressRange = org::apache::lucene::document::InetAddressRange;
using StringHelper = org::apache::lucene::util::StringHelper;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;
const wstring TestInetAddressRangeQueries::FIELD_NAME = L"ipRangeField";

shared_ptr<Range>
TestInetAddressRangeQueries::nextRange(int dimensions) 
{
  shared_ptr<InetAddress> min = nextInetaddress();
  std::deque<char> bMin = InetAddressPoint::encode(min);
  shared_ptr<InetAddress> max = nextInetaddress();
  std::deque<char> bMax = InetAddressPoint::encode(max);
  if (StringHelper::compare(bMin.size(), bMin, 0, bMax, 0) > 0) {
    return make_shared<IpRange>(shared_from_this(), max, min);
  }
  return make_shared<IpRange>(shared_from_this(), min, max);
}

shared_ptr<InetAddress>
TestInetAddressRangeQueries::nextInetaddress() 
{
  std::deque<char> b =
      random()->nextBoolean() ? std::deque<char>(4) : std::deque<char>(16);
  switch (random()->nextInt(5)) {
  case 0:
    return InetAddress::getByAddress(b);
  case 1:
    Arrays::fill(b, static_cast<char>(0xff));
    return InetAddress::getByAddress(b);
  case 2:
    Arrays::fill(b, static_cast<char>(42));
    return InetAddress::getByAddress(b);
  default:
    random()->nextBytes(b);
    return InetAddress::getByAddress(b);
  }
}

void TestInetAddressRangeQueries::testRandomTiny() 
{
  BaseRangeFieldQueryTestCase::testRandomTiny();
}

void TestInetAddressRangeQueries::testMultiValued() 
{
  BaseRangeFieldQueryTestCase::testRandomMedium();
}

void TestInetAddressRangeQueries::testRandomMedium() 
{
  BaseRangeFieldQueryTestCase::testMultiValued();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly @Override public void testRandomBig() throws
// Exception
void TestInetAddressRangeQueries::testRandomBig() 
{
  BaseRangeFieldQueryTestCase::testRandomBig();
}

shared_ptr<InetAddressRange>
TestInetAddressRangeQueries::newRangeField(shared_ptr<Range> r)
{
  return make_shared<InetAddressRange>(
      FIELD_NAME, (std::static_pointer_cast<IpRange>(r))->minAddress,
      (std::static_pointer_cast<IpRange>(r))->maxAddress);
}

shared_ptr<Query>
TestInetAddressRangeQueries::newIntersectsQuery(shared_ptr<Range> r)
{
  return InetAddressRange::newIntersectsQuery(
      FIELD_NAME, (std::static_pointer_cast<IpRange>(r))->minAddress,
      (std::static_pointer_cast<IpRange>(r))->maxAddress);
}

shared_ptr<Query>
TestInetAddressRangeQueries::newContainsQuery(shared_ptr<Range> r)
{
  return InetAddressRange::newContainsQuery(
      FIELD_NAME, (std::static_pointer_cast<IpRange>(r))->minAddress,
      (std::static_pointer_cast<IpRange>(r))->maxAddress);
}

shared_ptr<Query>
TestInetAddressRangeQueries::newWithinQuery(shared_ptr<Range> r)
{
  return InetAddressRange::newWithinQuery(
      FIELD_NAME, (std::static_pointer_cast<IpRange>(r))->minAddress,
      (std::static_pointer_cast<IpRange>(r))->maxAddress);
}

shared_ptr<Query>
TestInetAddressRangeQueries::newCrossesQuery(shared_ptr<Range> r)
{
  return InetAddressRange::newCrossesQuery(
      FIELD_NAME, (std::static_pointer_cast<IpRange>(r))->minAddress,
      (std::static_pointer_cast<IpRange>(r))->maxAddress);
}

TestInetAddressRangeQueries::IpRange::IpRange(
    shared_ptr<TestInetAddressRangeQueries> outerInstance,
    shared_ptr<InetAddress> min, shared_ptr<InetAddress> max)
    : outerInstance(outerInstance)
{
  this->minAddress = min;
  this->maxAddress = max;
  this->min = InetAddressPoint::encode(min);
  this->max = InetAddressPoint::encode(max);
}

int TestInetAddressRangeQueries::IpRange::numDimensions() { return 1; }

shared_ptr<InetAddress> TestInetAddressRangeQueries::IpRange::getMin(int dim)
{
  return minAddress;
}

void TestInetAddressRangeQueries::IpRange::setMin(int dim, any val)
{
  shared_ptr<InetAddress> v = any_cast<std::shared_ptr<InetAddress>>(val);
  std::deque<char> e = InetAddressPoint::encode(v);

  if (StringHelper::compare(e.size(), min, 0, e, 0) < 0) {
    max = e;
    maxAddress = v;
  } else {
    min = e;
    minAddress = v;
  }
}

shared_ptr<InetAddress> TestInetAddressRangeQueries::IpRange::getMax(int dim)
{
  return maxAddress;
}

void TestInetAddressRangeQueries::IpRange::setMax(int dim, any val)
{
  shared_ptr<InetAddress> v = any_cast<std::shared_ptr<InetAddress>>(val);
  std::deque<char> e = InetAddressPoint::encode(v);

  if (StringHelper::compare(e.size(), max, 0, e, 0) > 0) {
    min = e;
    minAddress = v;
  } else {
    max = e;
    maxAddress = v;
  }
}

bool TestInetAddressRangeQueries::IpRange::isEqual(shared_ptr<Range> o)
{
  shared_ptr<IpRange> other = std::static_pointer_cast<IpRange>(o);
  return Arrays::equals(min, other->min) && Arrays::equals(max, other->max);
}

bool TestInetAddressRangeQueries::IpRange::isDisjoint(shared_ptr<Range> o)
{
  shared_ptr<IpRange> other = std::static_pointer_cast<IpRange>(o);
  return StringHelper::compare(min.size(), min, 0, other->max, 0) > 0 ||
         StringHelper::compare(max.size(), max, 0, other->min, 0) < 0;
}

bool TestInetAddressRangeQueries::IpRange::isWithin(shared_ptr<Range> o)
{
  shared_ptr<IpRange> other = std::static_pointer_cast<IpRange>(o);
  return StringHelper::compare(min.size(), min, 0, other->min, 0) >= 0 &&
         StringHelper::compare(max.size(), max, 0, other->max, 0) <= 0;
}

bool TestInetAddressRangeQueries::IpRange::contains(shared_ptr<Range> o)
{
  shared_ptr<IpRange> other = std::static_pointer_cast<IpRange>(o);
  return StringHelper::compare(min.size(), min, 0, other->min, 0) <= 0 &&
         StringHelper::compare(max.size(), max, 0, other->max, 0) >= 0;
}

wstring TestInetAddressRangeQueries::IpRange::toString()
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  b->append(L"Box(");
  b->append(minAddress->getHostAddress());
  b->append(L" TO ");
  b->append(maxAddress->getHostAddress());
  b->append(L")");
  return b->toString();
}
} // namespace org::apache::lucene::search