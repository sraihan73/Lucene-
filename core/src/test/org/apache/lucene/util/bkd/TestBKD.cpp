using namespace std;

#include "TestBKD.h"

namespace org::apache::lucene::util::bkd
{
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using MergeState = org::apache::lucene::index::MergeState;
using IntersectVisitor =
    org::apache::lucene::index::PointValues::IntersectVisitor;
using Relation = org::apache::lucene::index::PointValues::Relation;
using PointValues = org::apache::lucene::index::PointValues;
using CorruptingIndexOutput = org::apache::lucene::store::CorruptingIndexOutput;
using Directory = org::apache::lucene::store::Directory;
using FilterDirectory = org::apache::lucene::store::FilterDirectory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using NumericUtils = org::apache::lucene::util::NumericUtils;
using StringHelper = org::apache::lucene::util::StringHelper;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestBKD::testBasicInts1D() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // getDirectory(100))
  {
    org::apache::lucene::store::Directory dir = getDirectory(100);
    shared_ptr<BKDWriter> w =
        make_shared<BKDWriter>(100, dir, L"tmp", 1, 4, 2, 1.0f, 100, true);
    std::deque<char> scratch(4);
    for (int docID = 0; docID < 100; docID++) {
      NumericUtils::intToSortableBytes(docID, scratch, 0);
      w->add(scratch, docID);
    }

    int64_t indexFP;
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput out =
    // dir.createOutput("bkd", org.apache.lucene.store.IOContext.DEFAULT))
    {
      org::apache::lucene::store::IndexOutput out = dir->createOutput(
          L"bkd", org::apache::lucene::store::IOContext::DEFAULT);
      indexFP = w->finish(out);
    }

    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexInput in =
    // dir.openInput("bkd", org.apache.lucene.store.IOContext.DEFAULT))
    {
      org::apache::lucene::store::IndexInput in_ = dir->openInput(
          L"bkd", org::apache::lucene::store::IOContext::DEFAULT);
      in_->seek(indexFP);
      shared_ptr<BKDReader> r = make_shared<BKDReader>(in_);

      // Simple 1D range query:
      constexpr int queryMin = 42;
      constexpr int queryMax = 87;

      shared_ptr<BitSet> *const hits = make_shared<BitSet>();
      r->intersect(make_shared<IntersectVisitorAnonymousInnerClass>(
          shared_from_this(), queryMin, queryMax, hits));

      for (int docID = 0; docID < 100; docID++) {
        bool expected = docID >= queryMin && docID <= queryMax;
        bool actual = hits->get(docID);
        assertEquals(L"docID=" + to_wstring(docID), expected, actual);
      }
    }
  }
}

TestBKD::IntersectVisitorAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass(shared_ptr<TestBKD> outerInstance,
                                        int queryMin, int queryMax,
                                        shared_ptr<BitSet> hits)
{
  this->outerInstance = outerInstance;
  this->queryMin = queryMin;
  this->queryMax = queryMax;
  this->hits = hits;
}

void TestBKD::IntersectVisitorAnonymousInnerClass::visit(int docID)
{
  hits->set(docID);
  if (VERBOSE) {
    wcout << L"visit docID=" << docID << endl;
  }
}

void TestBKD::IntersectVisitorAnonymousInnerClass::visit(
    int docID, std::deque<char> &packedValue)
{
  int x = NumericUtils::sortableBytesToInt(packedValue, 0);
  if (VERBOSE) {
    wcout << L"visit docID=" << docID << L" x=" << x << endl;
  }
  if (x >= queryMin && x <= queryMax) {
    hits->set(docID);
  }
}

Relation TestBKD::IntersectVisitorAnonymousInnerClass::compare(
    std::deque<char> &minPacked, std::deque<char> &maxPacked)
{
  int min = NumericUtils::sortableBytesToInt(minPacked, 0);
  int max = NumericUtils::sortableBytesToInt(maxPacked, 0);
  assert(max >= min);
  if (VERBOSE) {
    wcout << L"compare: min=" << min << L" max=" << max << L" vs queryMin="
          << queryMin << L" queryMax=" << queryMax << endl;
  }

  if (max < queryMin || min > queryMax) {
    return Relation::CELL_OUTSIDE_QUERY;
  } else if (min >= queryMin && max <= queryMax) {
    return Relation::CELL_INSIDE_QUERY;
  } else {
    return Relation::CELL_CROSSES_QUERY;
  }
}

void TestBKD::testRandomIntsNDims() 
{
  int numDocs = atLeast(1000);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // getDirectory(numDocs))
  {
    org::apache::lucene::store::Directory dir = getDirectory(numDocs);
    int numDims = TestUtil::nextInt(random(), 1, 5);
    int maxPointsInLeafNode = TestUtil::nextInt(random(), 50, 100);
    float maxMB = static_cast<float>(3.0) + (3 * random()->nextFloat());
    shared_ptr<BKDWriter> w =
        make_shared<BKDWriter>(numDocs, dir, L"tmp", numDims, 4,
                               maxPointsInLeafNode, maxMB, numDocs, true);

    if (VERBOSE) {
      wcout << L"TEST: numDims=" << numDims << L" numDocs=" << numDocs << endl;
    }
    std::deque<std::deque<int>> docs(numDocs);
    std::deque<char> scratch(4 * numDims);
    std::deque<int> minValue(numDims);
    std::deque<int> maxValue(numDims);
    Arrays::fill(minValue, numeric_limits<int>::max());
    Arrays::fill(maxValue, numeric_limits<int>::min());
    for (int docID = 0; docID < numDocs; docID++) {
      std::deque<int> values(numDims);
      if (VERBOSE) {
        wcout << L"  docID=" << docID << endl;
      }
      for (int dim = 0; dim < numDims; dim++) {
        values[dim] = random()->nextInt();
        if (values[dim] < minValue[dim]) {
          minValue[dim] = values[dim];
        }
        if (values[dim] > maxValue[dim]) {
          maxValue[dim] = values[dim];
        }
        NumericUtils::intToSortableBytes(values[dim], scratch,
                                         dim * Integer::BYTES);
        if (VERBOSE) {
          wcout << L"    " << dim << L" -> " << values[dim] << endl;
        }
      }
      docs[docID] = values;
      w->add(scratch, docID);
    }

    int64_t indexFP;
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput out =
    // dir.createOutput("bkd", org.apache.lucene.store.IOContext.DEFAULT))
    {
      org::apache::lucene::store::IndexOutput out = dir->createOutput(
          L"bkd", org::apache::lucene::store::IOContext::DEFAULT);
      indexFP = w->finish(out);
    }

    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexInput in =
    // dir.openInput("bkd", org.apache.lucene.store.IOContext.DEFAULT))
    {
      org::apache::lucene::store::IndexInput in_ = dir->openInput(
          L"bkd", org::apache::lucene::store::IOContext::DEFAULT);
      in_->seek(indexFP);
      shared_ptr<BKDReader> r = make_shared<BKDReader>(in_);

      std::deque<char> minPackedValue = r->getMinPackedValue();
      std::deque<char> maxPackedValue = r->getMaxPackedValue();
      for (int dim = 0; dim < numDims; dim++) {
        TestUtil::assertEquals(minValue[dim],
                               NumericUtils::sortableBytesToInt(
                                   minPackedValue, dim * Integer::BYTES));
        TestUtil::assertEquals(maxValue[dim],
                               NumericUtils::sortableBytesToInt(
                                   maxPackedValue, dim * Integer::BYTES));
      }

      int iters = atLeast(100);
      for (int iter = 0; iter < iters; iter++) {
        if (VERBOSE) {
          wcout << L"\nTEST: iter=" << iter << endl;
        }

        // Random N dims rect query:
        std::deque<int> queryMin(numDims);
        std::deque<int> queryMax(numDims);
        for (int dim = 0; dim < numDims; dim++) {
          queryMin[dim] = random()->nextInt();
          queryMax[dim] = random()->nextInt();
          if (queryMin[dim] > queryMax[dim]) {
            int x = queryMin[dim];
            queryMin[dim] = queryMax[dim];
            queryMax[dim] = x;
          }
        }

        shared_ptr<BitSet> *const hits = make_shared<BitSet>();
        r->intersect(make_shared<IntersectVisitorAnonymousInnerClass2>(
            shared_from_this(), numDims, queryMin, queryMax, hits));

        for (int docID = 0; docID < numDocs; docID++) {
          std::deque<int> docValues = docs[docID];
          bool expected = true;
          for (int dim = 0; dim < numDims; dim++) {
            int x = docValues[dim];
            if (x < queryMin[dim] || x > queryMax[dim]) {
              expected = false;
              break;
            }
          }
          bool actual = hits->get(docID);
          assertEquals(L"docID=" + to_wstring(docID), expected, actual);
        }
      }
    }
  }
}

TestBKD::IntersectVisitorAnonymousInnerClass2::
    IntersectVisitorAnonymousInnerClass2(shared_ptr<TestBKD> outerInstance,
                                         int numDims, deque<int> &queryMin,
                                         deque<int> &queryMax,
                                         shared_ptr<BitSet> hits)
{
  this->outerInstance = outerInstance;
  this->numDims = numDims;
  this->queryMin = queryMin;
  this->queryMax = queryMax;
  this->hits = hits;
}

void TestBKD::IntersectVisitorAnonymousInnerClass2::visit(int docID)
{
  hits->set(docID);
  // System.out.println("visit docID=" + docID);
}

void TestBKD::IntersectVisitorAnonymousInnerClass2::visit(
    int docID, std::deque<char> &packedValue)
{
  // System.out.println("visit check docID=" + docID);
  for (int dim = 0; dim < numDims; dim++) {
    int x = NumericUtils::sortableBytesToInt(packedValue, dim * Integer::BYTES);
    if (x < queryMin[dim] || x > queryMax[dim]) {
      // System.out.println("  no");
      return;
    }
  }

  // System.out.println("  yes");
  hits->set(docID);
}

Relation TestBKD::IntersectVisitorAnonymousInnerClass2::compare(
    std::deque<char> &minPacked, std::deque<char> &maxPacked)
{
  bool crosses = false;
  for (int dim = 0; dim < numDims; dim++) {
    int min = NumericUtils::sortableBytesToInt(minPacked, dim * Integer::BYTES);
    int max = NumericUtils::sortableBytesToInt(maxPacked, dim * Integer::BYTES);
    assert(max >= min);

    if (max < queryMin[dim] || min > queryMax[dim]) {
      return Relation::CELL_OUTSIDE_QUERY;
    } else if (min < queryMin[dim] || max > queryMax[dim]) {
      crosses = true;
    }
  }

  if (crosses) {
    return Relation::CELL_CROSSES_QUERY;
  } else {
    return Relation::CELL_INSIDE_QUERY;
  }
}

void TestBKD::testBigIntNDims() 
{

  int numDocs = atLeast(1000);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // getDirectory(numDocs))
  {
    org::apache::lucene::store::Directory dir = getDirectory(numDocs);
    int numBytesPerDim = TestUtil::nextInt(random(), 2, 30);
    int numDims = TestUtil::nextInt(random(), 1, 5);
    int maxPointsInLeafNode = TestUtil::nextInt(random(), 50, 100);
    float maxMB = static_cast<float>(3.0) + (3 * random()->nextFloat());
    shared_ptr<BKDWriter> w =
        make_shared<BKDWriter>(numDocs, dir, L"tmp", numDims, numBytesPerDim,
                               maxPointsInLeafNode, maxMB, numDocs, true);
    std::deque<std::deque<std::shared_ptr<int64_t>>> docs(numDocs);

    std::deque<char> scratch(numBytesPerDim * numDims);
    for (int docID = 0; docID < numDocs; docID++) {
      std::deque<std::shared_ptr<int64_t>> values(numDims);
      if (VERBOSE) {
        wcout << L"  docID=" << docID << endl;
      }
      for (int dim = 0; dim < numDims; dim++) {
        values[dim] = randomBigInt(numBytesPerDim);
        NumericUtils::bigIntToSortableBytes(values[dim], numBytesPerDim,
                                            scratch, dim * numBytesPerDim);
        if (VERBOSE) {
          wcout << L"    " << dim << L" -> " << values[dim] << endl;
        }
      }
      docs[docID] = values;
      w->add(scratch, docID);
    }

    int64_t indexFP;
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput out =
    // dir.createOutput("bkd", org.apache.lucene.store.IOContext.DEFAULT))
    {
      org::apache::lucene::store::IndexOutput out = dir->createOutput(
          L"bkd", org::apache::lucene::store::IOContext::DEFAULT);
      indexFP = w->finish(out);
    }

    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexInput in =
    // dir.openInput("bkd", org.apache.lucene.store.IOContext.DEFAULT))
    {
      org::apache::lucene::store::IndexInput in_ = dir->openInput(
          L"bkd", org::apache::lucene::store::IOContext::DEFAULT);
      in_->seek(indexFP);
      shared_ptr<BKDReader> r = make_shared<BKDReader>(in_);

      int iters = atLeast(100);
      for (int iter = 0; iter < iters; iter++) {
        if (VERBOSE) {
          wcout << L"\nTEST: iter=" << iter << endl;
        }

        // Random N dims rect query:
        std::deque<std::shared_ptr<int64_t>> queryMin(numDims);
        std::deque<std::shared_ptr<int64_t>> queryMax(numDims);
        for (int dim = 0; dim < numDims; dim++) {
          queryMin[dim] = randomBigInt(numBytesPerDim);
          queryMax[dim] = randomBigInt(numBytesPerDim);
          if (queryMin[dim]->compareTo(queryMax[dim]) > 0) {
            shared_ptr<int64_t> x = queryMin[dim];
            queryMin[dim] = queryMax[dim];
            queryMax[dim] = x;
          }
        }

        shared_ptr<BitSet> *const hits = make_shared<BitSet>();
        r->intersect(make_shared<IntersectVisitorAnonymousInnerClass3>(
            shared_from_this(), numBytesPerDim, numDims, queryMin, queryMax,
            hits));

        for (int docID = 0; docID < numDocs; docID++) {
          std::deque<std::shared_ptr<int64_t>> docValues = docs[docID];
          bool expected = true;
          for (int dim = 0; dim < numDims; dim++) {
            shared_ptr<int64_t> x = docValues[dim];
            if (x->compareTo(queryMin[dim]) < 0 ||
                x->compareTo(queryMax[dim]) > 0) {
              expected = false;
              break;
            }
          }
          bool actual = hits->get(docID);
          assertEquals(L"docID=" + to_wstring(docID), expected, actual);
        }
      }
    }
  }
}

TestBKD::IntersectVisitorAnonymousInnerClass3::
    IntersectVisitorAnonymousInnerClass3(
        shared_ptr<TestBKD> outerInstance, int numBytesPerDim, int numDims,
        deque<std::shared_ptr<int64_t>> &queryMin,
        deque<std::shared_ptr<int64_t>> &queryMax, shared_ptr<BitSet> hits)
{
  this->outerInstance = outerInstance;
  this->numBytesPerDim = numBytesPerDim;
  this->numDims = numDims;
  this->queryMin = queryMin;
  this->queryMax = queryMax;
  this->hits = hits;
}

void TestBKD::IntersectVisitorAnonymousInnerClass3::visit(int docID)
{
  hits->set(docID);
  // System.out.println("visit docID=" + docID);
}

void TestBKD::IntersectVisitorAnonymousInnerClass3::visit(
    int docID, std::deque<char> &packedValue)
{
  // System.out.println("visit check docID=" + docID);
  for (int dim = 0; dim < numDims; dim++) {
    shared_ptr<int64_t> x = NumericUtils::sortableBytesToBigInt(
        packedValue, dim * numBytesPerDim, numBytesPerDim);
    if (x->compareTo(queryMin[dim]) < 0 || x->compareTo(queryMax[dim]) > 0) {
      // System.out.println("  no");
      return;
    }
  }

  // System.out.println("  yes");
  hits->set(docID);
}

Relation TestBKD::IntersectVisitorAnonymousInnerClass3::compare(
    std::deque<char> &minPacked, std::deque<char> &maxPacked)
{
  bool crosses = false;
  for (int dim = 0; dim < numDims; dim++) {
    shared_ptr<int64_t> min = NumericUtils::sortableBytesToBigInt(
        minPacked, dim * numBytesPerDim, numBytesPerDim);
    shared_ptr<int64_t> max = NumericUtils::sortableBytesToBigInt(
        maxPacked, dim * numBytesPerDim, numBytesPerDim);
    assert(max->compareTo(min) >= 0);

    if (max->compareTo(queryMin[dim]) < 0 ||
        min->compareTo(queryMax[dim]) > 0) {
      return Relation::CELL_OUTSIDE_QUERY;
    } else if (min->compareTo(queryMin[dim]) < 0 ||
               max->compareTo(queryMax[dim]) > 0) {
      crosses = true;
    }
  }

  if (crosses) {
    return Relation::CELL_CROSSES_QUERY;
  } else {
    return Relation::CELL_INSIDE_QUERY;
  }
}

void TestBKD::testWithExceptions() 
{
  int numDocs = atLeast(10000);
  int numBytesPerDim = TestUtil::nextInt(random(), 2, 30);
  int numDims = TestUtil::nextInt(random(), 1, 5);

  std::deque<std::deque<std::deque<char>>> docValues(numDocs);

  for (int docID = 0; docID < numDocs; docID++) {
    std::deque<std::deque<char>> values(numDims);
    for (int dim = 0; dim < numDims; dim++) {
      values[dim] = std::deque<char>(numBytesPerDim);
      random()->nextBytes(values[dim]);
    }
    docValues[docID] = values;
  }

  double maxMBHeap = 0.05;
  // Keep retrying until we 1) we allow a big enough heap, and 2) we hit a
  // random IOExc from MDW:
  bool done = false;
  while (done == false) {
    shared_ptr<MockDirectoryWrapper> dir = newMockFSDirectory(createTempDir());
    try {
      dir->setRandomIOExceptionRate(0.05);
      dir->setRandomIOExceptionRateOnOpen(0.05);
      verify(dir, docValues, nullptr, numDims, numBytesPerDim, 50, maxMBHeap);
    } catch (const invalid_argument &iae) {
      // This just means we got a too-small maxMB for the maxPointsInLeafNode;
      // just retry w/ more heap
      assertTrue(iae.what()->contains(
          L"either increase maxMBSortInHeap or decrease maxPointsInLeafNode"));
      maxMBHeap *= 1.25;
    } catch (const IOException &ioe) {
      if (ioe->getMessage()->contains(L"a random IOException")) {
        // BKDWriter should fully clean up after itself:
        done = true;
      } else {
        throw ioe;
      }
    }

    std::deque<wstring> files = dir->listAll();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertTrue(L"files=" + Arrays->toString(files),
               files.empty() ||
                   Arrays::equals(files, std::deque<wstring>{L"extra0"}));
    delete dir;
  }
}

void TestBKD::testRandomBinaryTiny() 
{
  doTestRandomBinary(10);
}

void TestBKD::testRandomBinaryMedium() 
{
  doTestRandomBinary(10000);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testRandomBinaryBig() throws Exception
void TestBKD::testRandomBinaryBig() 
{
  doTestRandomBinary(200000);
}

void TestBKD::testTooLittleHeap() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // getDirectory(0))
  {
    org::apache::lucene::store::Directory dir = getDirectory(0);
    invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
      make_shared<BKDWriter>(1, dir, L"bkd", 1, 16, 1000000, 0.001, 0, true);
    });
    assertTrue(expected.what()->contains(
        L"either increase maxMBSortInHeap or decrease maxPointsInLeafNode"));
  }
}

void TestBKD::doTestRandomBinary(int count) 
{
  int numDocs = TestUtil::nextInt(random(), count, count * 2);
  int numBytesPerDim = TestUtil::nextInt(random(), 2, 30);

  int numDims = TestUtil::nextInt(random(), 1, 5);

  std::deque<std::deque<std::deque<char>>> docValues(numDocs);

  for (int docID = 0; docID < numDocs; docID++) {
    std::deque<std::deque<char>> values(numDims);
    for (int dim = 0; dim < numDims; dim++) {
      values[dim] = std::deque<char>(numBytesPerDim);
      random()->nextBytes(values[dim]);
    }
    docValues[docID] = values;
  }

  verify(docValues, nullptr, numDims, numBytesPerDim);
}

void TestBKD::testAllEqual() 
{
  int numBytesPerDim = TestUtil::nextInt(random(), 2, 30);
  int numDims = TestUtil::nextInt(random(), 1, 5);

  int numDocs = atLeast(1000);
  std::deque<std::deque<std::deque<char>>> docValues(numDocs);

  for (int docID = 0; docID < numDocs; docID++) {
    if (docID == 0) {
      std::deque<std::deque<char>> values(numDims);
      for (int dim = 0; dim < numDims; dim++) {
        values[dim] = std::deque<char>(numBytesPerDim);
        random()->nextBytes(values[dim]);
      }
      docValues[docID] = values;
    } else {
      docValues[docID] = docValues[0];
    }
  }

  verify(docValues, nullptr, numDims, numBytesPerDim);
}

void TestBKD::testOneDimEqual() 
{
  int numBytesPerDim = TestUtil::nextInt(random(), 2, 30);
  int numDims = TestUtil::nextInt(random(), 1, 5);

  int numDocs = atLeast(1000);
  int theEqualDim = random()->nextInt(numDims);
  std::deque<std::deque<std::deque<char>>> docValues(numDocs);

  for (int docID = 0; docID < numDocs; docID++) {
    std::deque<std::deque<char>> values(numDims);
    for (int dim = 0; dim < numDims; dim++) {
      values[dim] = std::deque<char>(numBytesPerDim);
      random()->nextBytes(values[dim]);
    }
    docValues[docID] = values;
    if (docID > 0) {
      docValues[docID][theEqualDim] = docValues[0][theEqualDim];
    }
  }

  // Use a small number of points in leaf blocks to trigger a lot of splitting
  verify(docValues, nullptr, numDims, numBytesPerDim,
         TestUtil::nextInt(random(), 20, 50));
}

void TestBKD::testOneDimLowCard() 
{
  int numBytesPerDim = TestUtil::nextInt(random(), 2, 30);
  int numDims = TestUtil::nextInt(random(), 2, 5);

  int numDocs = atLeast(10000);
  int theLowCardDim = random()->nextInt(numDims);

  std::deque<char> value1(numBytesPerDim);
  random()->nextBytes(value1);
  std::deque<char> value2 = value1.clone();
  if (value2[numBytesPerDim - 1] == 0 || random()->nextBoolean()) {
    value2[numBytesPerDim - 1]++;
  } else {
    value2[numBytesPerDim - 1]--;
  }

  std::deque<std::deque<std::deque<char>>> docValues(numDocs);

  for (int docID = 0; docID < numDocs; docID++) {
    std::deque<std::deque<char>> values(numDims);
    for (int dim = 0; dim < numDims; dim++) {
      if (dim == theLowCardDim) {
        values[dim] = random()->nextBoolean() ? value1 : value2;
      } else {
        values[dim] = std::deque<char>(numBytesPerDim);
        random()->nextBytes(values[dim]);
      }
    }
    docValues[docID] = values;
  }

  // Use a small number of points in leaf blocks to trigger a lot of splitting
  verify(docValues, nullptr, numDims, numBytesPerDim,
         TestUtil::nextInt(random(), 20, 50));
}

void TestBKD::testOneDimTwoValues() 
{
  int numBytesPerDim = TestUtil::nextInt(random(), 2, 30);
  int numDims = TestUtil::nextInt(random(), 1, 5);

  int numDocs = atLeast(1000);
  int theDim = random()->nextInt(numDims);
  std::deque<char> value1(numBytesPerDim);
  random()->nextBytes(value1);
  std::deque<char> value2(numBytesPerDim);
  random()->nextBytes(value2);
  std::deque<std::deque<std::deque<char>>> docValues(numDocs);

  for (int docID = 0; docID < numDocs; docID++) {
    std::deque<std::deque<char>> values(numDims);
    for (int dim = 0; dim < numDims; dim++) {
      if (dim == theDim) {
        values[dim] = random()->nextBoolean() ? value1 : value2;
      } else {
        values[dim] = std::deque<char>(numBytesPerDim);
        random()->nextBytes(values[dim]);
      }
    }
    docValues[docID] = values;
  }

  verify(docValues, nullptr, numDims, numBytesPerDim);
}

void TestBKD::testMultiValued() 
{
  int numBytesPerDim = TestUtil::nextInt(random(), 2, 30);
  int numDims = TestUtil::nextInt(random(), 1, 5);

  int numDocs = atLeast(1000);
  deque<std::deque<std::deque<char>>> docValues =
      deque<std::deque<std::deque<char>>>();
  deque<int> docIDs = deque<int>();

  for (int docID = 0; docID < numDocs; docID++) {
    int numValuesInDoc = TestUtil::nextInt(random(), 1, 5);
    for (int ord = 0; ord < numValuesInDoc; ord++) {
      docIDs.push_back(docID);
      std::deque<std::deque<char>> values(numDims);
      for (int dim = 0; dim < numDims; dim++) {
        values[dim] = std::deque<char>(numBytesPerDim);
        random()->nextBytes(values[dim]);
      }
      docValues.push_back(values);
    }
  }

  std::deque<std::deque<std::deque<char>>> docValuesArray =
      docValues.toArray(
          std::deque<std::deque<std::deque<char>>>(docValues.size()));
  std::deque<int> docIDsArray(docIDs.size());
  for (int i = 0; i < docIDsArray.size(); i++) {
    docIDsArray[i] = docIDs[i];
  }

  verify(docValuesArray, docIDsArray, numDims, numBytesPerDim);
}

void TestBKD::verify(std::deque<std::deque<std::deque<char>>> &docValues,
                     std::deque<int> &docIDs, int numDims,
                     int numBytesPerDim) 
{
  verify(docValues, docIDs, numDims, numBytesPerDim,
         TestUtil::nextInt(random(), 50, 1000));
}

void TestBKD::verify(std::deque<std::deque<std::deque<char>>> &docValues,
                     std::deque<int> &docIDs, int numDims, int numBytesPerDim,
                     int maxPointsInLeafNode) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // getDirectory(docValues.length))
  {
    org::apache::lucene::store::Directory dir = getDirectory(docValues.size());
    double maxMB = static_cast<float>(3.0) + (3 * random()->nextDouble());
    verify(dir, docValues, docIDs, numDims, numBytesPerDim, maxPointsInLeafNode,
           maxMB);
  }
}

void TestBKD::verify(shared_ptr<Directory> dir,
                     std::deque<std::deque<std::deque<char>>> &docValues,
                     std::deque<int> &docIDs, int numDims, int numBytesPerDim,
                     int maxPointsInLeafNode, double maxMB) 
{
  int numValues = docValues.size();
  if (VERBOSE) {
    wcout << L"TEST: numValues=" << numValues << L" numDims=" << numDims
          << L" numBytesPerDim=" << numBytesPerDim << L" maxPointsInLeafNode="
          << maxPointsInLeafNode << L" maxMB=" << maxMB << endl;
  }

  deque<int64_t> toMerge;
  deque<std::shared_ptr<MergeState::DocMap>> docMaps;
  int seg = 0;

  shared_ptr<BKDWriter> w = make_shared<BKDWriter>(
      numValues, dir, L"_" + to_wstring(seg), numDims, numBytesPerDim,
      maxPointsInLeafNode, maxMB, docValues.size(), false);
  shared_ptr<IndexOutput> out = dir->createOutput(L"bkd", IOContext::DEFAULT);
  shared_ptr<IndexInput> in_ = nullptr;

  bool success = false;

  try {

    std::deque<char> scratch(numBytesPerDim * numDims);
    int lastDocIDBase = 0;
    bool useMerge = numDims == 1 && numValues >= 10 && random()->nextBoolean();
    int valuesInThisSeg;
    if (useMerge) {
      // Sometimes we will call merge with a single segment:
      valuesInThisSeg = TestUtil::nextInt(random(), numValues / 10, numValues);
    } else {
      valuesInThisSeg = 0;
    }

    int segCount = 0;

    for (int ord = 0; ord < numValues; ord++) {
      int docID;
      if (docIDs.empty()) {
        docID = ord;
      } else {
        docID = docIDs[ord];
      }
      if (VERBOSE) {
        wcout << L"  ord=" << ord << L" docID=" << docID << L" lastDocIDBase="
              << lastDocIDBase << endl;
      }
      for (int dim = 0; dim < numDims; dim++) {
        if (VERBOSE) {
          wcout << L"    " << dim << L" -> "
                << make_shared<BytesRef>(docValues[ord][dim]) << endl;
        }
        System::arraycopy(docValues[ord][dim], 0, scratch, dim * numBytesPerDim,
                          numBytesPerDim);
      }
      w->add(scratch, docID - lastDocIDBase);

      segCount++;

      if (useMerge && segCount == valuesInThisSeg) {
        if (toMerge.empty()) {
          toMerge = deque<>();
          docMaps = deque<>();
        }
        constexpr int curDocIDBase = lastDocIDBase;
        docMaps.push_back(make_shared<DocMapAnonymousInnerClass>(
            shared_from_this(), docID, curDocIDBase));
        toMerge.push_back(w->finish(out));
        valuesInThisSeg =
            TestUtil::nextInt(random(), numValues / 10, numValues / 2);
        segCount = 0;

        seg++;
        maxPointsInLeafNode = TestUtil::nextInt(random(), 50, 1000);
        maxMB = static_cast<float>(3.0) + (3 * random()->nextDouble());
        w = make_shared<BKDWriter>(numValues, dir, L"_" + to_wstring(seg),
                                   numDims, numBytesPerDim, maxPointsInLeafNode,
                                   maxMB, docValues.size(), false);
        lastDocIDBase = docID;
      }
    }

    int64_t indexFP;

    if (toMerge.size() > 0) {
      if (segCount > 0) {
        toMerge.push_back(w->finish(out));
        constexpr int curDocIDBase = lastDocIDBase;
        docMaps.push_back(make_shared<DocMapAnonymousInnerClass2>(
            shared_from_this(), curDocIDBase));
      }
      delete out;
      in_ = dir->openInput(L"bkd", IOContext::DEFAULT);
      seg++;
      w = make_shared<BKDWriter>(numValues, dir, L"_" + to_wstring(seg),
                                 numDims, numBytesPerDim, maxPointsInLeafNode,
                                 maxMB, docValues.size(), false);
      deque<std::shared_ptr<BKDReader>> readers =
          deque<std::shared_ptr<BKDReader>>();
      for (auto fp : toMerge) {
        in_->seek(fp);
        readers.push_back(make_shared<BKDReader>(in_));
      }
      out = dir->createOutput(L"bkd2", IOContext::DEFAULT);
      indexFP = w->merge(out, docMaps, readers);
      delete out;
      delete in_;
      in_ = dir->openInput(L"bkd2", IOContext::DEFAULT);
    } else {
      indexFP = w->finish(out);
      delete out;
      in_ = dir->openInput(L"bkd", IOContext::DEFAULT);
    }

    in_->seek(indexFP);
    shared_ptr<BKDReader> r = make_shared<BKDReader>(in_);

    int iters = atLeast(100);
    for (int iter = 0; iter < iters; iter++) {
      if (VERBOSE) {
        wcout << L"\nTEST: iter=" << iter << endl;
      }

      // Random N dims rect query:
      std::deque<std::deque<char>> queryMin(numDims);
      std::deque<std::deque<char>> queryMax(numDims);
      for (int dim = 0; dim < numDims; dim++) {
        queryMin[dim] = std::deque<char>(numBytesPerDim);
        random()->nextBytes(queryMin[dim]);
        queryMax[dim] = std::deque<char>(numBytesPerDim);
        random()->nextBytes(queryMax[dim]);
        if (StringHelper::compare(numBytesPerDim, queryMin[dim], 0,
                                  queryMax[dim], 0) > 0) {
          std::deque<char> x = queryMin[dim];
          queryMin[dim] = queryMax[dim];
          queryMax[dim] = x;
        }
      }

      shared_ptr<BitSet> *const hits = make_shared<BitSet>();
      r->intersect(make_shared<IntersectVisitorAnonymousInnerClass4>(
          shared_from_this(), numDims, numBytesPerDim, queryMin, queryMax,
          hits));

      shared_ptr<BitSet> expected = make_shared<BitSet>();
      for (int ord = 0; ord < numValues; ord++) {
        bool matches = true;
        for (int dim = 0; dim < numDims; dim++) {
          std::deque<char> x = docValues[ord][dim];
          if (StringHelper::compare(numBytesPerDim, x, 0, queryMin[dim], 0) <
                  0 ||
              StringHelper::compare(numBytesPerDim, x, 0, queryMax[dim], 0) >
                  0) {
            matches = false;
            break;
          }
        }

        if (matches) {
          int docID;
          if (docIDs.empty()) {
            docID = ord;
          } else {
            docID = docIDs[ord];
          }
          expected->set(docID);
        }
      }

      int limit = max(expected->length(), hits->length());
      for (int docID = 0; docID < limit; docID++) {
        assertEquals(L"docID=" + to_wstring(docID), expected->get(docID),
                     hits->get(docID));
      }
    }
    delete in_;
    dir->deleteFile(L"bkd");
    if (toMerge.size() > 0) {
      dir->deleteFile(L"bkd2");
    }
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success == false) {
      IOUtils::closeWhileHandlingException({w, in_, out});
      IOUtils::deleteFilesIgnoringExceptions(dir, {L"bkd", L"bkd2"});
    }
  }
}

TestBKD::DocMapAnonymousInnerClass::DocMapAnonymousInnerClass(
    shared_ptr<TestBKD> outerInstance, int docID, int curDocIDBase)
{
  this->outerInstance = outerInstance;
  this->docID = docID;
  this->curDocIDBase = curDocIDBase;
}

int TestBKD::DocMapAnonymousInnerClass::get(int docID)
{
  return curDocIDBase + docID;
}

TestBKD::DocMapAnonymousInnerClass2::DocMapAnonymousInnerClass2(
    shared_ptr<TestBKD> outerInstance, int curDocIDBase)
{
  this->outerInstance = outerInstance;
  this->curDocIDBase = curDocIDBase;
}

int TestBKD::DocMapAnonymousInnerClass2::get(int docID)
{
  return curDocIDBase + docID;
}

TestBKD::IntersectVisitorAnonymousInnerClass4::
    IntersectVisitorAnonymousInnerClass4(shared_ptr<TestBKD> outerInstance,
                                         int numDims, int numBytesPerDim,
                                         deque<deque<char>> &queryMin,
                                         deque<deque<char>> &queryMax,
                                         shared_ptr<BitSet> hits)
{
  this->outerInstance = outerInstance;
  this->numDims = numDims;
  this->numBytesPerDim = numBytesPerDim;
  this->queryMin = queryMin;
  this->queryMax = queryMax;
  this->hits = hits;
}

void TestBKD::IntersectVisitorAnonymousInnerClass4::visit(int docID)
{
  hits->set(docID);
  // System.out.println("visit docID=" + docID);
}

void TestBKD::IntersectVisitorAnonymousInnerClass4::visit(
    int docID, std::deque<char> &packedValue)
{
  // System.out.println("visit check docID=" + docID);
  for (int dim = 0; dim < numDims; dim++) {
    if (StringHelper::compare(numBytesPerDim, packedValue, dim * numBytesPerDim,
                              queryMin[dim], 0) < 0 ||
        StringHelper::compare(numBytesPerDim, packedValue, dim * numBytesPerDim,
                              queryMax[dim], 0) > 0) {
      // System.out.println("  no");
      return;
    }
  }

  // System.out.println("  yes");
  hits->set(docID);
}

Relation TestBKD::IntersectVisitorAnonymousInnerClass4::compare(
    std::deque<char> &minPacked, std::deque<char> &maxPacked)
{
  bool crosses = false;
  for (int dim = 0; dim < numDims; dim++) {
    if (StringHelper::compare(numBytesPerDim, maxPacked, dim * numBytesPerDim,
                              queryMin[dim], 0) < 0 ||
        StringHelper::compare(numBytesPerDim, minPacked, dim * numBytesPerDim,
                              queryMax[dim], 0) > 0) {
      return Relation::CELL_OUTSIDE_QUERY;
    } else if (StringHelper::compare(numBytesPerDim, minPacked,
                                     dim * numBytesPerDim, queryMin[dim],
                                     0) < 0 ||
               StringHelper::compare(numBytesPerDim, maxPacked,
                                     dim * numBytesPerDim, queryMax[dim],
                                     0) > 0) {
      crosses = true;
    }
  }

  if (crosses) {
    return Relation::CELL_CROSSES_QUERY;
  } else {
    return Relation::CELL_INSIDE_QUERY;
  }
}

shared_ptr<int64_t> TestBKD::randomBigInt(int numBytes)
{
  shared_ptr<int64_t> x =
      make_shared<int64_t>(numBytes * 8 - 1, random());
  if (random()->nextBoolean()) {
    x = x->negate();
  }
  return x;
}

shared_ptr<Directory> TestBKD::getDirectory(int numPoints)
{
  shared_ptr<Directory> dir;
  if (numPoints > 100000) {
    dir = newFSDirectory(createTempDir(L"TestBKDTree"));
  } else {
    dir = newDirectory();
  }
  return dir;
}

void TestBKD::testBitFlippedOnPartition1() 
{

  // Generate fixed data set:
  int numDocs = atLeast(10000);
  int numBytesPerDim = 4;
  int numDims = 3;

  std::deque<std::deque<std::deque<char>>> docValues(numDocs);
  char counter = 0;

  for (int docID = 0; docID < numDocs; docID++) {
    std::deque<std::deque<char>> values(numDims);
    for (int dim = 0; dim < numDims; dim++) {
      values[dim] = std::deque<char>(numBytesPerDim);
      for (int i = 0; i < values[dim].size(); i++) {
        values[dim][i] = counter;
        counter++;
      }
    }
    docValues[docID] = values;
  }

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir0 =
  // newMockDirectory())
  {
    org::apache::lucene::store::Directory dir0 = newMockDirectory();

    shared_ptr<Directory> dir = make_shared<FilterDirectoryAnonymousInnerClass>(
        shared_from_this(), dir0);

    shared_ptr<CorruptIndexException> e =
        expectThrows(CorruptIndexException::typeid, [&]() {
          verify(dir, docValues, nullptr, numDims, numBytesPerDim, 50, 0.1);
        });
    assertTrue(
        e->getMessage()->contains(L"checksum failed (hardware problem?)"));
  }
}

TestBKD::FilterDirectoryAnonymousInnerClass::FilterDirectoryAnonymousInnerClass(
    shared_ptr<TestBKD> outerInstance, shared_ptr<Directory> dir0)
    : org::apache::lucene::store::FilterDirectory(dir0)
{
  this->outerInstance = outerInstance;
  this->dir0 = dir0;
}

shared_ptr<IndexOutput>
TestBKD::FilterDirectoryAnonymousInnerClass::createTempOutput(
    const wstring &prefix, const wstring &suffix,
    shared_ptr<IOContext> context) 
{
  shared_ptr<IndexOutput> out = in_::createTempOutput(prefix, suffix, context);
  if (corrupted == false && prefix == L"_0_bkd1" && suffix == L"sort") {
    corrupted = true;
    return make_shared<CorruptingIndexOutput>(dir0, 22, out);
  } else {
    return out;
  }
}

void TestBKD::testBitFlippedOnPartition2() 
{

  // Generate fixed data set:
  int numDocs = atLeast(10000);
  int numBytesPerDim = 4;
  int numDims = 3;

  std::deque<std::deque<std::deque<char>>> docValues(numDocs);
  char counter = 0;

  for (int docID = 0; docID < numDocs; docID++) {
    std::deque<std::deque<char>> values(numDims);
    for (int dim = 0; dim < numDims; dim++) {
      values[dim] = std::deque<char>(numBytesPerDim);
      for (int i = 0; i < values[dim].size(); i++) {
        values[dim][i] = counter;
        counter++;
      }
    }
    docValues[docID] = values;
  }

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir0 =
  // newMockDirectory())
  {
    org::apache::lucene::store::Directory dir0 = newMockDirectory();

    shared_ptr<Directory> dir =
        make_shared<FilterDirectoryAnonymousInnerClass2>(shared_from_this(),
                                                         dir0);

    runtime_error t = expectThrows(CorruptIndexException::typeid, [&]() {
      verify(dir, docValues, nullptr, numDims, numBytesPerDim, 50, 0.1);
    });
    assertCorruptionDetected(t);
  }
}

TestBKD::FilterDirectoryAnonymousInnerClass2::
    FilterDirectoryAnonymousInnerClass2(shared_ptr<TestBKD> outerInstance,
                                        shared_ptr<Directory> dir0)
    : org::apache::lucene::store::FilterDirectory(dir0)
{
  this->outerInstance = outerInstance;
  this->dir0 = dir0;
}

shared_ptr<IndexOutput>
TestBKD::FilterDirectoryAnonymousInnerClass2::createTempOutput(
    const wstring &prefix, const wstring &suffix,
    shared_ptr<IOContext> context) 
{
  shared_ptr<IndexOutput> out = in_::createTempOutput(prefix, suffix, context);
  // System.out.println("prefix=" + prefix + " suffix=" + suffix);
  if (corrupted == false && suffix == L"bkd_left1") {
    // System.out.println("now corrupt byte=" + x + " prefix=" + prefix + "
    // suffix=" + suffix);
    corrupted = true;
    return make_shared<CorruptingIndexOutput>(dir0, 22072, out);
  } else {
    return out;
  }
}

void TestBKD::assertCorruptionDetected(runtime_error t)
{
  if (std::dynamic_pointer_cast<CorruptIndexException>(t) != nullptr) {
    if (t.what()->contains(L"checksum failed (hardware problem?)")) {
      return;
    }
  }

  for (runtime_error suppressed : t.getSuppressed()) {
    if (std::dynamic_pointer_cast<CorruptIndexException>(suppressed) !=
        nullptr) {
      if (suppressed.what()->contains(L"checksum failed (hardware problem?)")) {
        return;
      }
    }
  }
  fail(L"did not see a suppressed CorruptIndexException");
}

void TestBKD::testTieBreakOrder() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory())
  {
    org::apache::lucene::store::Directory dir = newDirectory();
    int numDocs = 10000;
    shared_ptr<BKDWriter> w = make_shared<BKDWriter>(
        numDocs + 1, dir, L"tmp", 1, Integer::BYTES, 2, 0.01f, numDocs, true);
    for (int i = 0; i < numDocs; i++) {
      w->add(std::deque<char>(Integer::BYTES), i);
    }

    shared_ptr<IndexOutput> out = dir->createOutput(L"bkd", IOContext::DEFAULT);
    int64_t fp = w->finish(out);
    delete out;

    shared_ptr<IndexInput> in_ = dir->openInput(L"bkd", IOContext::DEFAULT);
    in_->seek(fp);
    shared_ptr<BKDReader> r = make_shared<BKDReader>(in_);
    r->intersect(
        make_shared<IntersectVisitorAnonymousInnerClass5>(shared_from_this()));
    delete in_;
  }
}

TestBKD::IntersectVisitorAnonymousInnerClass5::
    IntersectVisitorAnonymousInnerClass5(shared_ptr<TestBKD> outerInstance)
{
  this->outerInstance = outerInstance;
  lastDocID = -1;
}

void TestBKD::IntersectVisitorAnonymousInnerClass5::visit(int docID)
{
  assertTrue(L"lastDocID=" + lastDocID + L" docID=" + to_wstring(docID),
             docID > lastDocID);
  lastDocID = docID;
}

void TestBKD::IntersectVisitorAnonymousInnerClass5::visit(
    int docID, std::deque<char> &packedValue)
{
  visit(docID);
}

Relation TestBKD::IntersectVisitorAnonymousInnerClass5::compare(
    std::deque<char> &minPacked, std::deque<char> &maxPacked)
{
  return Relation::CELL_CROSSES_QUERY;
}

void TestBKD::test2DLongOrdsOffline() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory())
  {
    org::apache::lucene::store::Directory dir = newDirectory();
    int numDocs = 100000;
    bool singleValuePerDoc = false;
    bool longOrds = true;
    int offlineSorterMaxTempFiles = TestUtil::nextInt(random(), 2, 20);
    shared_ptr<BKDWriter> w = make_shared<BKDWriter>(
        numDocs + 1, dir, L"tmp", 2, Integer::BYTES, 2, 0.01f, numDocs,
        singleValuePerDoc, longOrds, 1, offlineSorterMaxTempFiles);
    std::deque<char> buffer(2 * Integer::BYTES);
    for (int i = 0; i < numDocs; i++) {
      random()->nextBytes(buffer);
      w->add(buffer, i);
    }

    shared_ptr<IndexOutput> out = dir->createOutput(L"bkd", IOContext::DEFAULT);
    int64_t fp = w->finish(out);
    delete out;

    shared_ptr<IndexInput> in_ = dir->openInput(L"bkd", IOContext::DEFAULT);
    in_->seek(fp);
    shared_ptr<BKDReader> r = make_shared<BKDReader>(in_);
    std::deque<int> count(1);
    r->intersect(make_shared<IntersectVisitorAnonymousInnerClass6>(
        shared_from_this(), count));
    TestUtil::assertEquals(numDocs, count[0]);
    delete in_;
  }
}

TestBKD::IntersectVisitorAnonymousInnerClass6::
    IntersectVisitorAnonymousInnerClass6(shared_ptr<TestBKD> outerInstance,
                                         deque<int> &count)
{
  this->outerInstance = outerInstance;
  this->count = count;
}

void TestBKD::IntersectVisitorAnonymousInnerClass6::visit(int docID)
{
  count[0]++;
}

void TestBKD::IntersectVisitorAnonymousInnerClass6::visit(
    int docID, std::deque<char> &packedValue)
{
  visit(docID);
}

Relation TestBKD::IntersectVisitorAnonymousInnerClass6::compare(
    std::deque<char> &minPacked, std::deque<char> &maxPacked)
{
  if (LuceneTestCase::random()->nextInt(7) == 1) {
    return Relation::CELL_CROSSES_QUERY;
  } else {
    return Relation::CELL_INSIDE_QUERY;
  }
}

void TestBKD::testWastedLeadingBytes() 
{
  int numDims = TestUtil::nextInt(random(), 1, PointValues::MAX_DIMENSIONS);
  int bytesPerDim = PointValues::MAX_NUM_BYTES;
  int bytesUsed = TestUtil::nextInt(random(), 1, 3);

  shared_ptr<Directory> dir = newFSDirectory(createTempDir());
  int numDocs = 100000;
  shared_ptr<BKDWriter> w = make_shared<BKDWriter>(
      numDocs + 1, dir, L"tmp", numDims, bytesPerDim, 32, 1.0f, numDocs, true);
  std::deque<char> tmp(bytesUsed);
  std::deque<char> buffer(numDims * bytesPerDim);
  for (int i = 0; i < numDocs; i++) {
    for (int dim = 0; dim < numDims; dim++) {
      random()->nextBytes(tmp);
      System::arraycopy(tmp, 0, buffer,
                        dim * bytesPerDim + (bytesPerDim - bytesUsed),
                        tmp.size());
    }
    w->add(buffer, i);
  }

  shared_ptr<IndexOutput> out = dir->createOutput(L"bkd", IOContext::DEFAULT);
  int64_t fp = w->finish(out);
  delete out;

  shared_ptr<IndexInput> in_ = dir->openInput(L"bkd", IOContext::DEFAULT);
  in_->seek(fp);
  shared_ptr<BKDReader> r = make_shared<BKDReader>(in_);
  std::deque<int> count(1);
  r->intersect(make_shared<IntersectVisitorAnonymousInnerClass7>(
      shared_from_this(), count));
  TestUtil::assertEquals(numDocs, count[0]);
  delete in_;
  delete dir;
}

TestBKD::IntersectVisitorAnonymousInnerClass7::
    IntersectVisitorAnonymousInnerClass7(shared_ptr<TestBKD> outerInstance,
                                         deque<int> &count)
{
  this->outerInstance = outerInstance;
  this->count = count;
}

void TestBKD::IntersectVisitorAnonymousInnerClass7::visit(int docID)
{
  count[0]++;
}

void TestBKD::IntersectVisitorAnonymousInnerClass7::visit(
    int docID, std::deque<char> &packedValue)
{
  visit(docID);
}

Relation TestBKD::IntersectVisitorAnonymousInnerClass7::compare(
    std::deque<char> &minPacked, std::deque<char> &maxPacked)
{
  if (LuceneTestCase::random()->nextInt(7) == 1) {
    return Relation::CELL_CROSSES_QUERY;
  } else {
    return Relation::CELL_INSIDE_QUERY;
  }
}

void TestBKD::testEstimatePointCount() 
{
  shared_ptr<Directory> dir = newDirectory();
  constexpr int numValues = atLeast(10000); // make sure to have multiple leaves
  constexpr int maxPointsInLeafNode = TestUtil::nextInt(random(), 50, 500);
  constexpr int numBytesPerDim = TestUtil::nextInt(random(), 1, 4);
  const std::deque<char> pointValue = std::deque<char>(numBytesPerDim);
  const std::deque<char> uniquePointValue = std::deque<char>(numBytesPerDim);
  random()->nextBytes(uniquePointValue);

  shared_ptr<BKDWriter> w = make_shared<BKDWriter>(
      numValues, dir, L"_temp", 1, numBytesPerDim, maxPointsInLeafNode,
      BKDWriter::DEFAULT_MAX_MB_SORT_IN_HEAP, numValues, true);
  for (int i = 0; i < numValues; ++i) {
    if (i == numValues / 2) {
      w->add(uniquePointValue, i);
    } else {
      do {
        random()->nextBytes(pointValue);
      } while (Arrays::equals(pointValue, uniquePointValue));
      w->add(pointValue, i);
    }
  }
  constexpr int64_t indexFP;
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput out =
  // dir.createOutput("bkd", org.apache.lucene.store.IOContext.DEFAULT))
  {
    org::apache::lucene::store::IndexOutput out = dir->createOutput(
        L"bkd", org::apache::lucene::store::IOContext::DEFAULT);
    indexFP = w->finish(out);
    delete w;
  }

  shared_ptr<IndexInput> pointsIn = dir->openInput(L"bkd", IOContext::DEFAULT);
  pointsIn->seek(indexFP);
  shared_ptr<BKDReader> points = make_shared<BKDReader>(pointsIn);

  int actualMaxPointsInLeafNode = numValues;
  while (actualMaxPointsInLeafNode > maxPointsInLeafNode) {
    actualMaxPointsInLeafNode = (actualMaxPointsInLeafNode + 1) / 2;
  }

  // If all points match, then the point count is numLeaves *
  // maxPointsInLeafNode
  constexpr int numLeaves =
      Integer::highestOneBit((numValues - 1) / actualMaxPointsInLeafNode) << 1;
  TestUtil::assertEquals(numLeaves * actualMaxPointsInLeafNode,
                         points->estimatePointCount(
                             make_shared<IntersectVisitorAnonymousInnerClass8>(
                                 shared_from_this())));

  // Return 0 if no points match
  TestUtil::assertEquals(0,
                         points->estimatePointCount(
                             make_shared<IntersectVisitorAnonymousInnerClass9>(
                                 shared_from_this())));

  // If only one point matches, then the point count is
  // (actualMaxPointsInLeafNode + 1) / 2 in general, or maybe 2x that if the
  // point is a split value
  constexpr int64_t pointCount = points->estimatePointCount(
      make_shared<IntersectVisitorAnonymousInnerClass10>(
          shared_from_this(), numBytesPerDim, uniquePointValue));
  assertTrue(L"" + to_wstring(pointCount),
             pointCount == (actualMaxPointsInLeafNode + 1) / 2 ||
                 pointCount == 2 * ((actualMaxPointsInLeafNode + 1) /
                                    2)); // if the point is a split value

  delete pointsIn;
  delete dir;
}

TestBKD::IntersectVisitorAnonymousInnerClass8::
    IntersectVisitorAnonymousInnerClass8(shared_ptr<TestBKD> outerInstance)
{
  this->outerInstance = outerInstance;
}

void TestBKD::IntersectVisitorAnonymousInnerClass8::visit(
    int docID, std::deque<char> &packedValue) 
{
}

void TestBKD::IntersectVisitorAnonymousInnerClass8::visit(int docID) throw(
    IOException)
{
}

Relation TestBKD::IntersectVisitorAnonymousInnerClass8::compare(
    std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue)
{
  return Relation::CELL_INSIDE_QUERY;
}

TestBKD::IntersectVisitorAnonymousInnerClass9::
    IntersectVisitorAnonymousInnerClass9(shared_ptr<TestBKD> outerInstance)
{
  this->outerInstance = outerInstance;
}

void TestBKD::IntersectVisitorAnonymousInnerClass9::visit(
    int docID, std::deque<char> &packedValue) 
{
}

void TestBKD::IntersectVisitorAnonymousInnerClass9::visit(int docID) throw(
    IOException)
{
}

Relation TestBKD::IntersectVisitorAnonymousInnerClass9::compare(
    std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue)
{
  return Relation::CELL_OUTSIDE_QUERY;
}

TestBKD::IntersectVisitorAnonymousInnerClass10::
    IntersectVisitorAnonymousInnerClass10(shared_ptr<TestBKD> outerInstance,
                                          int numBytesPerDim,
                                          deque<char> &uniquePointValue)
{
  this->outerInstance = outerInstance;
  this->numBytesPerDim = numBytesPerDim;
  this->uniquePointValue = uniquePointValue;
}

void TestBKD::IntersectVisitorAnonymousInnerClass10::visit(
    int docID, std::deque<char> &packedValue) 
{
}

void TestBKD::IntersectVisitorAnonymousInnerClass10::visit(int docID) throw(
    IOException)
{
}

Relation TestBKD::IntersectVisitorAnonymousInnerClass10::compare(
    std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue)
{
  if (StringHelper::compare(numBytesPerDim, uniquePointValue, 0, maxPackedValue,
                            0) > 0 ||
      StringHelper::compare(numBytesPerDim, uniquePointValue, 0, minPackedValue,
                            0) < 0) {
    return Relation::CELL_OUTSIDE_QUERY;
  }
  return Relation::CELL_CROSSES_QUERY;
}
} // namespace org::apache::lucene::util::bkd