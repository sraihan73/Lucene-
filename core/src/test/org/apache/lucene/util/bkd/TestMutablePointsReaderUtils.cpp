using namespace std;

#include "TestMutablePointsReaderUtils.h"

namespace org::apache::lucene::util::bkd
{
using MutablePointValues = org::apache::lucene::codecs::MutablePointValues;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using StringHelper = org::apache::lucene::util::StringHelper;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestMutablePointsReaderUtils::testSort()
{
  for (int iter = 0; iter < 5; ++iter) {
    doTestSort();
  }
}

void TestMutablePointsReaderUtils::doTestSort()
{
  constexpr int bytesPerDim = TestUtil::nextInt(random(), 1, 16);
  constexpr int maxDoc =
      TestUtil::nextInt(random(), 1, 1 << random()->nextInt(30));
  std::deque<std::shared_ptr<Point>> points =
      createRandomPoints(1, bytesPerDim, maxDoc);
  shared_ptr<DummyPointsReader> reader = make_shared<DummyPointsReader>(points);
  MutablePointsReaderUtils::sort(maxDoc, bytesPerDim, reader, 0, points.size());
  Arrays::sort(points,
               make_shared<ComparatorAnonymousInnerClass>(shared_from_this()));
  assertNotSame(points, reader->points);
  assertArrayEquals(points, reader->points);
}

TestMutablePointsReaderUtils::ComparatorAnonymousInnerClass::
    ComparatorAnonymousInnerClass(
        shared_ptr<TestMutablePointsReaderUtils> outerInstance)
{
  this->outerInstance = outerInstance;
}

int TestMutablePointsReaderUtils::ComparatorAnonymousInnerClass::compare(
    shared_ptr<Point> o1, shared_ptr<Point> o2)
{
  int cmp = o1->packedValue->compareTo(o2->packedValue);
  if (cmp == 0) {
    cmp = Integer::compare(o1->doc, o2->doc);
  }
  return cmp;
}

void TestMutablePointsReaderUtils::testSortByDim()
{
  for (int iter = 0; iter < 5; ++iter) {
    doTestSortByDim();
  }
}

void TestMutablePointsReaderUtils::doTestSortByDim()
{
  constexpr int numDims = TestUtil::nextInt(random(), 1, 8);
  constexpr int bytesPerDim = TestUtil::nextInt(random(), 1, 16);
  constexpr int maxDoc =
      TestUtil::nextInt(random(), 1, 1 << random()->nextInt(30));
  std::deque<std::shared_ptr<Point>> points =
      createRandomPoints(numDims, bytesPerDim, maxDoc);
  std::deque<int> commonPrefixLengths(numDims);
  for (int i = 0; i < commonPrefixLengths.size(); ++i) {
    commonPrefixLengths[i] = TestUtil::nextInt(random(), 0, bytesPerDim);
  }
  shared_ptr<BytesRef> firstValue = points[0]->packedValue;
  for (int i = 1; i < points.size(); ++i) {
    for (int dim = 0; dim < numDims; ++dim) {
      int offset = dim * bytesPerDim;
      shared_ptr<BytesRef> packedValue = points[i]->packedValue;
      System::arraycopy(firstValue->bytes, firstValue->offset + offset,
                        packedValue->bytes, packedValue->offset + offset,
                        commonPrefixLengths[dim]);
    }
  }
  shared_ptr<DummyPointsReader> reader = make_shared<DummyPointsReader>(points);
  constexpr int sortedDim = random()->nextInt(numDims);
  MutablePointsReaderUtils::sortByDim(
      sortedDim, bytesPerDim, commonPrefixLengths, reader, 0, points.size(),
      make_shared<BytesRef>(), make_shared<BytesRef>());
  for (int i = 1; i < points.size(); ++i) {
    constexpr int offset = sortedDim * bytesPerDim;
    shared_ptr<BytesRef> previousValue = reader->points[i - 1].packedValue;
    shared_ptr<BytesRef> currentValue = reader->points[i].packedValue;
    int cmp = StringHelper::compare(
        bytesPerDim, previousValue->bytes, previousValue->offset + offset,
        currentValue->bytes, currentValue->offset + offset);
    if (cmp == 0) {
      cmp = reader->points[i - 1].doc - reader->points[i].doc;
    }
    assertTrue(cmp <= 0);
  }
}

void TestMutablePointsReaderUtils::testPartition()
{
  for (int iter = 0; iter < 5; ++iter) {
    doTestPartition();
  }
}

void TestMutablePointsReaderUtils::doTestPartition()
{
  constexpr int numDims = TestUtil::nextInt(random(), 1, 8);
  constexpr int bytesPerDim = TestUtil::nextInt(random(), 1, 16);
  constexpr int maxDoc =
      TestUtil::nextInt(random(), 1, 1 << random()->nextInt(30));
  std::deque<std::shared_ptr<Point>> points =
      createRandomPoints(numDims, bytesPerDim, maxDoc);
  int commonPrefixLength = TestUtil::nextInt(random(), 0, bytesPerDim);
  constexpr int splitDim = random()->nextInt(numDims);
  shared_ptr<BytesRef> firstValue = points[0]->packedValue;
  for (int i = 1; i < points.size(); ++i) {
    shared_ptr<BytesRef> packedValue = points[i]->packedValue;
    int offset = splitDim * bytesPerDim;
    System::arraycopy(firstValue->bytes, firstValue->offset + offset,
                      packedValue->bytes, packedValue->offset + offset,
                      commonPrefixLength);
  }
  shared_ptr<DummyPointsReader> reader = make_shared<DummyPointsReader>(points);
  constexpr int pivot = TestUtil::nextInt(random(), 0, points.size() - 1);
  MutablePointsReaderUtils::partition(
      maxDoc, splitDim, bytesPerDim, commonPrefixLength, reader, 0,
      points.size(), pivot, make_shared<BytesRef>(), make_shared<BytesRef>());
  shared_ptr<BytesRef> pivotValue = reader->points[pivot].packedValue;
  int offset = splitDim * bytesPerDim;
  for (int i = 0; i < points.size(); ++i) {
    shared_ptr<BytesRef> value = reader->points[i].packedValue;
    int cmp =
        StringHelper::compare(bytesPerDim, value->bytes, value->offset + offset,
                              pivotValue->bytes, pivotValue->offset + offset);
    if (cmp == 0) {
      cmp = reader->points[i].doc - reader->points[pivot].doc;
    }
    if (i < pivot) {
      assertTrue(cmp <= 0);
    } else if (i > pivot) {
      assertTrue(cmp >= 0);
    } else {
      TestUtil::assertEquals(0, cmp);
    }
  }
}

std::deque<std::shared_ptr<Point>>
TestMutablePointsReaderUtils::createRandomPoints(int numDims, int bytesPerDim,
                                                 int maxDoc)
{
  constexpr int packedBytesLength = numDims * bytesPerDim;
  constexpr int numPoints = TestUtil::nextInt(random(), 1, 100000);
  std::deque<std::shared_ptr<Point>> points(numPoints);
  for (int i = 0; i < numPoints; ++i) {
    std::deque<char> value(packedBytesLength);
    random()->nextBytes(value);
    points[i] = make_shared<Point>(value, random()->nextInt(maxDoc));
  }
  return points;
}

TestMutablePointsReaderUtils::Point::Point(std::deque<char> &packedValue,
                                           int doc)
    : packedValue(make_shared<BytesRef>(packedValue.size() + 1)), doc(doc)
{
  // use a non-null offset to make sure MutablePointsReaderUtils does not ignore
  // it
  this->packedValue->bytes[0] =
      static_cast<char>(LuceneTestCase::random()->nextInt(256));
  this->packedValue->offset = 1;
  this->packedValue->length = packedValue.size();
}

bool TestMutablePointsReaderUtils::Point::equals(any obj)
{
  if (obj == nullptr ||
      std::dynamic_pointer_cast<Point>(obj) != nullptr == false) {
    return false;
  }
  shared_ptr<Point> that = any_cast<std::shared_ptr<Point>>(obj);
  return packedValue->equals(that->packedValue) && doc == that->doc;
}

int TestMutablePointsReaderUtils::Point::hashCode()
{
  return 31 * packedValue->hashCode() + doc;
}

wstring TestMutablePointsReaderUtils::Point::toString()
{
  return L"value=" + packedValue + L" doc=" + to_wstring(doc);
}

TestMutablePointsReaderUtils::DummyPointsReader::DummyPointsReader(
    std::deque<std::shared_ptr<Point>> &points)
    : points(points.clone())
{
}

void TestMutablePointsReaderUtils::DummyPointsReader::getValue(
    int i, shared_ptr<BytesRef> packedValue)
{
  packedValue->bytes = points[i]->packedValue.bytes;
  packedValue->offset = points[i]->packedValue.offset;
  packedValue->length = points[i]->packedValue->length;
}

char TestMutablePointsReaderUtils::DummyPointsReader::getByteAt(int i, int k)
{
  shared_ptr<BytesRef> packedValue = points[i]->packedValue;
  return packedValue->bytes[packedValue->offset + k];
}

int TestMutablePointsReaderUtils::DummyPointsReader::getDocID(int i)
{
  return points[i]->doc;
}

void TestMutablePointsReaderUtils::DummyPointsReader::swap(int i, int j)
{
  ArrayUtil::swap(points, i, j);
}

void TestMutablePointsReaderUtils::DummyPointsReader::intersect(
    shared_ptr<IntersectVisitor> visitor) 
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t TestMutablePointsReaderUtils::DummyPointsReader::estimatePointCount(
    shared_ptr<IntersectVisitor> visitor)
{
  throw make_shared<UnsupportedOperationException>();
}

std::deque<char>
TestMutablePointsReaderUtils::DummyPointsReader::getMinPackedValue() throw(
    IOException)
{
  throw make_shared<UnsupportedOperationException>();
}

std::deque<char>
TestMutablePointsReaderUtils::DummyPointsReader::getMaxPackedValue() throw(
    IOException)
{
  throw make_shared<UnsupportedOperationException>();
}

int TestMutablePointsReaderUtils::DummyPointsReader::getNumDimensions() throw(
    IOException)
{
  throw make_shared<UnsupportedOperationException>();
}

int TestMutablePointsReaderUtils::DummyPointsReader::
    getBytesPerDimension() 
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t TestMutablePointsReaderUtils::DummyPointsReader::size()
{
  throw make_shared<UnsupportedOperationException>();
}

int TestMutablePointsReaderUtils::DummyPointsReader::getDocCount()
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::util::bkd