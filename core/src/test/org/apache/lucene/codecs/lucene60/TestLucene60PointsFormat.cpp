using namespace std;

#include "TestLucene60PointsFormat.h"

namespace org::apache::lucene::codecs::lucene60
{
using Codec = org::apache::lucene::codecs::Codec;
using FilterCodec = org::apache::lucene::codecs::FilterCodec;
using PointsFormat = org::apache::lucene::codecs::PointsFormat;
using PointsReader = org::apache::lucene::codecs::PointsReader;
using PointsWriter = org::apache::lucene::codecs::PointsWriter;
using BinaryPoint = org::apache::lucene::document::BinaryPoint;
using Document = org::apache::lucene::document::Document;
using BasePointsFormatTestCase =
    org::apache::lucene::index::BasePointsFormatTestCase;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReader = org::apache::lucene::index::LeafReader;
using MockRandomMergePolicy = org::apache::lucene::index::MockRandomMergePolicy;
using PointValues = org::apache::lucene::index::PointValues;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using IntersectVisitor =
    org::apache::lucene::index::PointValues::IntersectVisitor;
using Relation = org::apache::lucene::index::PointValues::Relation;
using Directory = org::apache::lucene::store::Directory;
using StringHelper = org::apache::lucene::util::StringHelper;
using TestUtil = org::apache::lucene::util::TestUtil;
using BKDWriter = org::apache::lucene::util::bkd::BKDWriter;

TestLucene60PointsFormat::TestLucene60PointsFormat()
{
  // standard issue
  shared_ptr<Codec> defaultCodec = TestUtil::getDefaultCodec();
  if (random()->nextBoolean()) {
    // randomize parameters
    maxPointsInLeafNode = TestUtil::nextInt(random(), 50, 500);
    double maxMBSortInHeap = 3.0 + (3 * random()->nextDouble());
    if (VERBOSE) {
      wcout << L"TEST: using Lucene60PointsFormat with maxPointsInLeafNode="
            << maxPointsInLeafNode << L" and maxMBSortInHeap="
            << maxMBSortInHeap << endl;
    }

    // sneaky impersonation!
    // C++ TODO: You cannot use 'shared_from_this' in a constructor:
    codec = make_shared<FilterCodecAnonymousInnerClass>(
        shared_from_this(), defaultCodec->getName(), defaultCodec,
        maxMBSortInHeap);
  } else {
    // standard issue
    codec = defaultCodec;
    maxPointsInLeafNode = BKDWriter::DEFAULT_MAX_POINTS_IN_LEAF_NODE;
  }
}

TestLucene60PointsFormat::FilterCodecAnonymousInnerClass::
    FilterCodecAnonymousInnerClass(
        shared_ptr<TestLucene60PointsFormat> outerInstance,
        const wstring &getName, shared_ptr<Codec> defaultCodec,
        double maxMBSortInHeap)
    : org::apache::lucene::codecs::FilterCodec(getName, defaultCodec)
{
  this->outerInstance = outerInstance;
  this->maxMBSortInHeap = maxMBSortInHeap;
}

shared_ptr<PointsFormat>
TestLucene60PointsFormat::FilterCodecAnonymousInnerClass::pointsFormat()
{
  return make_shared<PointsFormatAnonymousInnerClass>(shared_from_this());
}

TestLucene60PointsFormat::FilterCodecAnonymousInnerClass::
    PointsFormatAnonymousInnerClass::PointsFormatAnonymousInnerClass(
        shared_ptr<FilterCodecAnonymousInnerClass> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<PointsWriter>
TestLucene60PointsFormat::FilterCodecAnonymousInnerClass::
    PointsFormatAnonymousInnerClass::fieldsWriter(
        shared_ptr<SegmentWriteState> writeState) 
{
  return make_shared<Lucene60PointsWriter>(
      writeState, outerInstance->outerInstance.maxPointsInLeafNode,
      outerInstance->maxMBSortInHeap);
}

shared_ptr<PointsReader>
TestLucene60PointsFormat::FilterCodecAnonymousInnerClass::
    PointsFormatAnonymousInnerClass::fieldsReader(
        shared_ptr<SegmentReadState> readState) 
{
  return make_shared<Lucene60PointsReader>(readState);
}

shared_ptr<Codec> TestLucene60PointsFormat::getCodec() { return codec; }

void TestLucene60PointsFormat::testMergeStability() 
{
  assumeFalse(L"TODO: mess with the parameters and test gets angry!",
              std::dynamic_pointer_cast<FilterCodec>(codec) != nullptr);
  BasePointsFormatTestCase::testMergeStability();
}

void TestLucene60PointsFormat::testEstimatePointCount() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  // Avoid mockRandomMP since it may cause non-optimal merges that make the
  // number of points per leaf hard to predict
  while (std::dynamic_pointer_cast<MockRandomMergePolicy>(
             iwc->getMergePolicy()) != nullptr) {
    iwc->setMergePolicy(newMergePolicy());
  }
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  std::deque<char> pointValue(3);
  std::deque<char> uniquePointValue(3);
  random()->nextBytes(uniquePointValue);
  constexpr int numDocs = atLeast(10000); // make sure we have several leaves
  for (int i = 0; i < numDocs; ++i) {
    shared_ptr<Document> doc = make_shared<Document>();
    if (i == numDocs / 2) {
      doc->push_back(make_shared<BinaryPoint>(L"f", uniquePointValue));
    } else {
      do {
        random()->nextBytes(pointValue);
      } while (Arrays::equals(pointValue, uniquePointValue));
      doc->push_back(make_shared<BinaryPoint>(L"f", pointValue));
    }
    w->addDocument(doc);
  }
  w->forceMerge(1);
  shared_ptr<IndexReader> *const r = DirectoryReader::open(w);
  delete w;
  shared_ptr<LeafReader> *const lr = getOnlyLeafReader(r);
  shared_ptr<PointValues> points = lr->getPointValues(L"f");

  // If all points match, then the point count is numLeaves *
  // maxPointsInLeafNode
  constexpr int numLeaves = static_cast<int>(
      ceil(static_cast<double>(numDocs) / maxPointsInLeafNode));
  TestUtil::assertEquals(numLeaves * maxPointsInLeafNode,
                         points->estimatePointCount(
                             make_shared<IntersectVisitorAnonymousInnerClass>(
                                 shared_from_this())));

  // Return 0 if no points match
  TestUtil::assertEquals(0,
                         points->estimatePointCount(
                             make_shared<IntersectVisitorAnonymousInnerClass2>(
                                 shared_from_this())));

  // If only one point matches, then the point count is (maxPointsInLeafNode +
  // 1) / 2 in general, or maybe 2x that if the point is a split value
  constexpr int64_t pointCount = points->estimatePointCount(
      make_shared<IntersectVisitorAnonymousInnerClass3>(shared_from_this(),
                                                        uniquePointValue));
  assertTrue(L"" + to_wstring(pointCount),
             pointCount == (maxPointsInLeafNode + 1) / 2 ||
                 pointCount == 2 * ((maxPointsInLeafNode + 1) /
                                    2)); // if the point is a split value

  delete r;
  delete dir;
}

TestLucene60PointsFormat::IntersectVisitorAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass(
        shared_ptr<TestLucene60PointsFormat> outerInstance)
{
  this->outerInstance = outerInstance;
}

void TestLucene60PointsFormat::IntersectVisitorAnonymousInnerClass::visit(
    int docID, std::deque<char> &packedValue) 
{
}

void TestLucene60PointsFormat::IntersectVisitorAnonymousInnerClass::visit(
    int docID) 
{
}

PointValues::Relation
TestLucene60PointsFormat::IntersectVisitorAnonymousInnerClass::compare(
    std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue)
{
  return PointValues::Relation::CELL_INSIDE_QUERY;
}

TestLucene60PointsFormat::IntersectVisitorAnonymousInnerClass2::
    IntersectVisitorAnonymousInnerClass2(
        shared_ptr<TestLucene60PointsFormat> outerInstance)
{
  this->outerInstance = outerInstance;
}

void TestLucene60PointsFormat::IntersectVisitorAnonymousInnerClass2::visit(
    int docID, std::deque<char> &packedValue) 
{
}

void TestLucene60PointsFormat::IntersectVisitorAnonymousInnerClass2::visit(
    int docID) 
{
}

PointValues::Relation
TestLucene60PointsFormat::IntersectVisitorAnonymousInnerClass2::compare(
    std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue)
{
  return PointValues::Relation::CELL_OUTSIDE_QUERY;
}

TestLucene60PointsFormat::IntersectVisitorAnonymousInnerClass3::
    IntersectVisitorAnonymousInnerClass3(
        shared_ptr<TestLucene60PointsFormat> outerInstance,
        deque<char> &uniquePointValue)
{
  this->outerInstance = outerInstance;
  this->uniquePointValue = uniquePointValue;
}

void TestLucene60PointsFormat::IntersectVisitorAnonymousInnerClass3::visit(
    int docID, std::deque<char> &packedValue) 
{
}

void TestLucene60PointsFormat::IntersectVisitorAnonymousInnerClass3::visit(
    int docID) 
{
}

PointValues::Relation
TestLucene60PointsFormat::IntersectVisitorAnonymousInnerClass3::compare(
    std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue)
{
  if (StringHelper::compare(3, uniquePointValue, 0, maxPackedValue, 0) > 0 ||
      StringHelper::compare(3, uniquePointValue, 0, minPackedValue, 0) < 0) {
    return PointValues::Relation::CELL_OUTSIDE_QUERY;
  }
  return PointValues::Relation::CELL_CROSSES_QUERY;
}

void TestLucene60PointsFormat::testEstimatePointCount2Dims() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, newIndexWriterConfig());
  std::deque<std::deque<char>> pointValue(2);
  pointValue[0] = std::deque<char>(3);
  pointValue[1] = std::deque<char>(3);
  std::deque<std::deque<char>> uniquePointValue(2);
  uniquePointValue[0] = std::deque<char>(3);
  uniquePointValue[1] = std::deque<char>(3);
  random()->nextBytes(uniquePointValue[0]);
  random()->nextBytes(uniquePointValue[1]);
  constexpr int numDocs = atLeast(10000); // make sure we have several leaves
  for (int i = 0; i < numDocs; ++i) {
    shared_ptr<Document> doc = make_shared<Document>();
    if (i == numDocs / 2) {
      doc->push_back(make_shared<BinaryPoint>(L"f", uniquePointValue));
    } else {
      do {
        random()->nextBytes(pointValue[0]);
        random()->nextBytes(pointValue[1]);
      } while (Arrays::equals(pointValue[0], uniquePointValue[0]) ||
               Arrays::equals(pointValue[1], uniquePointValue[1]));
      doc->push_back(make_shared<BinaryPoint>(L"f", pointValue));
    }
    w->addDocument(doc);
  }
  w->forceMerge(1);
  shared_ptr<IndexReader> *const r = DirectoryReader::open(w);
  delete w;
  shared_ptr<LeafReader> *const lr = getOnlyLeafReader(r);
  shared_ptr<PointValues> points = lr->getPointValues(L"f");

  // With >1 dims, the tree is balanced
  int actualMaxPointsInLeafNode = numDocs;
  while (actualMaxPointsInLeafNode > maxPointsInLeafNode) {
    actualMaxPointsInLeafNode = (actualMaxPointsInLeafNode + 1) / 2;
  }

  // If all points match, then the point count is numLeaves *
  // maxPointsInLeafNode
  constexpr int numLeaves =
      Integer::highestOneBit((numDocs - 1) / actualMaxPointsInLeafNode) << 1;
  TestUtil::assertEquals(numLeaves * actualMaxPointsInLeafNode,
                         points->estimatePointCount(
                             make_shared<IntersectVisitorAnonymousInnerClass4>(
                                 shared_from_this())));

  // Return 0 if no points match
  TestUtil::assertEquals(0,
                         points->estimatePointCount(
                             make_shared<IntersectVisitorAnonymousInnerClass5>(
                                 shared_from_this())));

  // If only one point matches, then the point count is
  // (actualMaxPointsInLeafNode + 1) / 2 in general, or maybe 2x that if the
  // point is a split value
  constexpr int64_t pointCount = points->estimatePointCount(
      make_shared<IntersectVisitorAnonymousInnerClass6>(shared_from_this(),
                                                        uniquePointValue));
  assertTrue(L"" + to_wstring(pointCount),
             pointCount == (actualMaxPointsInLeafNode + 1) / 2 ||
                 pointCount == 2 * ((actualMaxPointsInLeafNode + 1) /
                                    2)); // if the point is a split value

  delete r;
  delete dir;
}

TestLucene60PointsFormat::IntersectVisitorAnonymousInnerClass4::
    IntersectVisitorAnonymousInnerClass4(
        shared_ptr<TestLucene60PointsFormat> outerInstance)
{
  this->outerInstance = outerInstance;
}

void TestLucene60PointsFormat::IntersectVisitorAnonymousInnerClass4::visit(
    int docID, std::deque<char> &packedValue) 
{
}

void TestLucene60PointsFormat::IntersectVisitorAnonymousInnerClass4::visit(
    int docID) 
{
}

PointValues::Relation
TestLucene60PointsFormat::IntersectVisitorAnonymousInnerClass4::compare(
    std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue)
{
  return PointValues::Relation::CELL_INSIDE_QUERY;
}

TestLucene60PointsFormat::IntersectVisitorAnonymousInnerClass5::
    IntersectVisitorAnonymousInnerClass5(
        shared_ptr<TestLucene60PointsFormat> outerInstance)
{
  this->outerInstance = outerInstance;
}

void TestLucene60PointsFormat::IntersectVisitorAnonymousInnerClass5::visit(
    int docID, std::deque<char> &packedValue) 
{
}

void TestLucene60PointsFormat::IntersectVisitorAnonymousInnerClass5::visit(
    int docID) 
{
}

PointValues::Relation
TestLucene60PointsFormat::IntersectVisitorAnonymousInnerClass5::compare(
    std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue)
{
  return PointValues::Relation::CELL_OUTSIDE_QUERY;
}

TestLucene60PointsFormat::IntersectVisitorAnonymousInnerClass6::
    IntersectVisitorAnonymousInnerClass6(
        shared_ptr<TestLucene60PointsFormat> outerInstance,
        deque<deque<char>> &uniquePointValue)
{
  this->outerInstance = outerInstance;
  this->uniquePointValue = uniquePointValue;
}

void TestLucene60PointsFormat::IntersectVisitorAnonymousInnerClass6::visit(
    int docID, std::deque<char> &packedValue) 
{
}

void TestLucene60PointsFormat::IntersectVisitorAnonymousInnerClass6::visit(
    int docID) 
{
}

PointValues::Relation
TestLucene60PointsFormat::IntersectVisitorAnonymousInnerClass6::compare(
    std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue)
{
  for (int dim = 0; dim < 2; ++dim) {
    if (StringHelper::compare(3, uniquePointValue[dim], 0, maxPackedValue,
                              dim * 3) > 0 ||
        StringHelper::compare(3, uniquePointValue[dim], 0, minPackedValue,
                              dim * 3) < 0) {
      return PointValues::Relation::CELL_OUTSIDE_QUERY;
    }
  }
  return PointValues::Relation::CELL_CROSSES_QUERY;
}
} // namespace org::apache::lucene::codecs::lucene60