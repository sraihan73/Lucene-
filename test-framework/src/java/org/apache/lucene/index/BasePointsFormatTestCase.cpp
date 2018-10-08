using namespace std;

#include "BasePointsFormatTestCase.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Codec = org::apache::lucene::codecs::Codec;
using BinaryPoint = org::apache::lucene::document::BinaryPoint;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IntPoint = org::apache::lucene::document::IntPoint;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using IntersectVisitor =
    org::apache::lucene::index::PointValues::IntersectVisitor;
using Relation = org::apache::lucene::index::PointValues::Relation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Directory = org::apache::lucene::store::Directory;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using NumericUtils = org::apache::lucene::util::NumericUtils;
using Rethrow = org::apache::lucene::util::Rethrow;
using StringHelper = org::apache::lucene::util::StringHelper;
using TestUtil = org::apache::lucene::util::TestUtil;
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

void BasePointsFormatTestCase::addRandomFields(shared_ptr<Document> doc)
{
  constexpr int numValues = random()->nextInt(3);
  for (int i = 0; i < numValues; i++) {
    doc->push_back(make_shared<IntPoint>(L"f", random()->nextInt()));
  }
}

void BasePointsFormatTestCase::testBasic() 
{
  shared_ptr<Directory> dir = getDirectory(20);
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  std::deque<char> point(4);
  for (int i = 0; i < 20; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    NumericUtils::intToSortableBytes(i, point, 0);
    doc->push_back(make_shared<BinaryPoint>(L"dim", point));
    w->addDocument(doc);
  }
  w->forceMerge(1);
  delete w;

  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  shared_ptr<LeafReader> sub = getOnlyLeafReader(r);
  shared_ptr<PointValues> values = sub->getPointValues(L"dim");

  // Simple test: make sure intersect can visit every doc:
  shared_ptr<BitSet> seen = make_shared<BitSet>();
  values->intersect(make_shared<IntersectVisitorAnonymousInnerClass>(
      shared_from_this(), seen));
  TestUtil::assertEquals(20, seen->cardinality());
  IOUtils::close({r, dir});
}

BasePointsFormatTestCase::IntersectVisitorAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass(
        shared_ptr<BasePointsFormatTestCase> outerInstance,
        shared_ptr<BitSet> seen)
{
  this->outerInstance = outerInstance;
  this->seen = seen;
}

Relation BasePointsFormatTestCase::IntersectVisitorAnonymousInnerClass::compare(
    std::deque<char> &minPacked, std::deque<char> &maxPacked)
{
  return Relation::CELL_CROSSES_QUERY;
}

void BasePointsFormatTestCase::IntersectVisitorAnonymousInnerClass::visit(
    int docID)
{
  throw make_shared<IllegalStateException>();
}

void BasePointsFormatTestCase::IntersectVisitorAnonymousInnerClass::visit(
    int docID, std::deque<char> &packedValue)
{
  seen->set(docID);
  TestUtil::assertEquals(docID,
                         NumericUtils::sortableBytesToInt(packedValue, 0));
}

void BasePointsFormatTestCase::testMerge() 
{
  shared_ptr<Directory> dir = getDirectory(20);
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  std::deque<char> point(4);
  for (int i = 0; i < 20; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    NumericUtils::intToSortableBytes(i, point, 0);
    doc->push_back(make_shared<BinaryPoint>(L"dim", point));
    w->addDocument(doc);
    if (i == 10) {
      w->commit();
    }
  }
  w->forceMerge(1);
  delete w;

  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  shared_ptr<LeafReader> sub = getOnlyLeafReader(r);
  shared_ptr<PointValues> values = sub->getPointValues(L"dim");

  // Simple test: make sure intersect can visit every doc:
  shared_ptr<BitSet> seen = make_shared<BitSet>();
  values->intersect(make_shared<IntersectVisitorAnonymousInnerClass2>(
      shared_from_this(), seen));
  TestUtil::assertEquals(20, seen->cardinality());
  IOUtils::close({r, dir});
}

BasePointsFormatTestCase::IntersectVisitorAnonymousInnerClass2::
    IntersectVisitorAnonymousInnerClass2(
        shared_ptr<BasePointsFormatTestCase> outerInstance,
        shared_ptr<BitSet> seen)
{
  this->outerInstance = outerInstance;
  this->seen = seen;
}

Relation
BasePointsFormatTestCase::IntersectVisitorAnonymousInnerClass2::compare(
    std::deque<char> &minPacked, std::deque<char> &maxPacked)
{
  return Relation::CELL_CROSSES_QUERY;
}

void BasePointsFormatTestCase::IntersectVisitorAnonymousInnerClass2::visit(
    int docID)
{
  throw make_shared<IllegalStateException>();
}

void BasePointsFormatTestCase::IntersectVisitorAnonymousInnerClass2::visit(
    int docID, std::deque<char> &packedValue)
{
  seen->set(docID);
  TestUtil::assertEquals(docID,
                         NumericUtils::sortableBytesToInt(packedValue, 0));
}

void BasePointsFormatTestCase::testAllPointDocsDeletedInSegment() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = getDirectory(20);
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  std::deque<char> point(4);
  for (int i = 0; i < 10; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    NumericUtils::intToSortableBytes(i, point, 0);
    doc->push_back(make_shared<BinaryPoint>(L"dim", point));
    doc->push_back(make_shared<NumericDocValuesField>(L"id", i));
    doc->push_back(newStringField(L"x", L"x", Field::Store::NO));
    w->addDocument(doc);
  }
  w->addDocument(make_shared<Document>());
  w->deleteDocuments({make_shared<Term>(L"x", L"x")});
  if (random()->nextBoolean()) {
    w->forceMerge(1);
  }
  delete w;
  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  TestUtil::assertEquals(1, r->numDocs());
  shared_ptr<Bits> liveDocs = MultiFields::getLiveDocs(r);

  for (shared_ptr<LeafReaderContext> ctx : r->leaves()) {
    shared_ptr<PointValues> values = ctx->reader()->getPointValues(L"dim");

    shared_ptr<NumericDocValues> idValues =
        ctx->reader()->getNumericDocValues(L"id");
    if (idValues == nullptr) {
      // this is (surprisingly) OK, because if the random IWC flushes all 10
      // docs before the 11th doc is added, and force merge runs, it will drop
      // the 100% deleted segments, and the "id" field never exists in the final
      // single doc segment
      continue;
    }
    std::deque<int> docIDToID(ctx->reader()->maxDoc());
    int docID;
    while ((docID = idValues->nextDoc()) != NO_MORE_DOCS) {
      docIDToID[docID] = static_cast<int>(idValues->longValue());
    }

    if (values != nullptr) {
      shared_ptr<BitSet> seen = make_shared<BitSet>();
      values->intersect(make_shared<IntersectVisitorAnonymousInnerClass3>(
          shared_from_this(), liveDocs, docIDToID, docID, seen));
      TestUtil::assertEquals(0, seen->cardinality());
    }
  }
  IOUtils::close({r, dir});
}

BasePointsFormatTestCase::IntersectVisitorAnonymousInnerClass3::
    IntersectVisitorAnonymousInnerClass3(
        shared_ptr<BasePointsFormatTestCase> outerInstance,
        shared_ptr<Bits> liveDocs, deque<int> &docIDToID, int docID,
        shared_ptr<BitSet> seen)
{
  this->outerInstance = outerInstance;
  this->liveDocs = liveDocs;
  this->docIDToID = docIDToID;
  this->docID = docID;
  this->seen = seen;
}

Relation
BasePointsFormatTestCase::IntersectVisitorAnonymousInnerClass3::compare(
    std::deque<char> &minPacked, std::deque<char> &maxPacked)
{
  return Relation::CELL_CROSSES_QUERY;
}

void BasePointsFormatTestCase::IntersectVisitorAnonymousInnerClass3::visit(
    int docID)
{
  throw make_shared<IllegalStateException>();
}

void BasePointsFormatTestCase::IntersectVisitorAnonymousInnerClass3::visit(
    int docID, std::deque<char> &packedValue)
{
  if (liveDocs->get(docID)) {
    seen->set(docID);
  }
  TestUtil::assertEquals(docIDToID[docID],
                         NumericUtils::sortableBytesToInt(packedValue, 0));
}

void BasePointsFormatTestCase::testWithExceptions() 
{
  int numDocs = atLeast(10000);
  int numBytesPerDim =
      TestUtil::nextInt(random(), 2, PointValues::MAX_NUM_BYTES);
  int numDims = TestUtil::nextInt(random(), 1, PointValues::MAX_DIMENSIONS);

  std::deque<std::deque<std::deque<char>>> docValues(numDocs);

  for (int docID = 0; docID < numDocs; docID++) {
    std::deque<std::deque<char>> values(numDims);
    for (int dim = 0; dim < numDims; dim++) {
      values[dim] = std::deque<char>(numBytesPerDim);
      random()->nextBytes(values[dim]);
    }
    docValues[docID] = values;
  }

  // Keep retrying until we 1) we allow a big enough heap, and 2) we hit a
  // random IOExc from MDW:
  bool done = false;
  while (done == false) {
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try
    // (org.apache.lucene.store.MockDirectoryWrapper dir =
    // newMockFSDirectory(createTempDir()))
    {
      org::apache::lucene::store::MockDirectoryWrapper dir =
          newMockFSDirectory(createTempDir());
      try {
        dir->setRandomIOExceptionRate(0.05);
        dir->setRandomIOExceptionRateOnOpen(0.05);
        verify(dir, docValues, nullptr, numDims, numBytesPerDim, true);
      } catch (const IllegalStateException &ise) {
        done = handlePossiblyFakeException(ise);
      } catch (const AssertionError &ae) {
        if (ae->getMessage() != nullptr &&
            ae->getMessage()->contains(L"does not exist; files=")) {
          // OK: likely we threw the random IOExc when IW was asserting the
          // commit files exist
          done = true;
        } else {
          throw ae;
        }
      } catch (const invalid_argument &iae) {
        // This just means we got a too-small maxMB for the maxPointsInLeafNode;
        // just retry w/ more heap
        assertTrue(iae.what()->contains(L"either increase maxMBSortInHeap or "
                                        L"decrease maxPointsInLeafNode"));
      } catch (const IOException &ioe) {
        done = handlePossiblyFakeException(ioe);
      }
    }
  }
}

bool BasePointsFormatTestCase::handlePossiblyFakeException(runtime_error e)
{
  runtime_error ex = e;
  while (ex != nullptr) {
    wstring message = ex.what();
    if (message != L"" &&
        (message.find(L"a random IOException") != wstring::npos ||
         message.find(L"background merge hit exception") != wstring::npos)) {
      return true;
    }
    ex = ex.getCause();
  }
  Rethrow::rethrow(e);

  // dead code yet javac disagrees:
  return false;
}

void BasePointsFormatTestCase::testMultiValued() 
{
  int numBytesPerDim =
      TestUtil::nextInt(random(), 2, PointValues::MAX_NUM_BYTES);
  int numDims = TestUtil::nextInt(random(), 1, PointValues::MAX_DIMENSIONS);

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

void BasePointsFormatTestCase::testAllEqual() 
{
  int numBytesPerDim =
      TestUtil::nextInt(random(), 2, PointValues::MAX_NUM_BYTES);
  int numDims = TestUtil::nextInt(random(), 1, PointValues::MAX_DIMENSIONS);

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

void BasePointsFormatTestCase::testOneDimEqual() 
{
  int numBytesPerDim =
      TestUtil::nextInt(random(), 2, PointValues::MAX_NUM_BYTES);
  int numDims = TestUtil::nextInt(random(), 1, PointValues::MAX_DIMENSIONS);

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

  verify(docValues, nullptr, numDims, numBytesPerDim);
}

void BasePointsFormatTestCase::testOneDimTwoValues() 
{
  int numBytesPerDim =
      TestUtil::nextInt(random(), 2, PointValues::MAX_NUM_BYTES);
  int numDims = TestUtil::nextInt(random(), 1, PointValues::MAX_DIMENSIONS);

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

void BasePointsFormatTestCase::testBigIntNDims() 
{

  int numDocs = atLeast(1000);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // getDirectory(numDocs))
  {
    org::apache::lucene::store::Directory dir = getDirectory(numDocs);
    int numBytesPerDim =
        TestUtil::nextInt(random(), 2, PointValues::MAX_NUM_BYTES);
    int numDims = TestUtil::nextInt(random(), 1, PointValues::MAX_DIMENSIONS);
    shared_ptr<IndexWriterConfig> iwc =
        newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
    // We rely on docIDs not changing:
    iwc->setMergePolicy(newLogMergePolicy());
    shared_ptr<RandomIndexWriter> w =
        make_shared<RandomIndexWriter>(random(), dir, iwc);
    std::deque<std::deque<std::shared_ptr<int64_t>>> docs(numDocs);

    for (int docID = 0; docID < numDocs; docID++) {
      std::deque<std::shared_ptr<int64_t>> values(numDims);
      if (VERBOSE) {
        wcout << L"  docID=" << docID << endl;
      }
      std::deque<std::deque<char>> bytes(numDims);
      for (int dim = 0; dim < numDims; dim++) {
        values[dim] = randomBigInt(numBytesPerDim);
        bytes[dim] = std::deque<char>(numBytesPerDim);
        NumericUtils::bigIntToSortableBytes(values[dim], numBytesPerDim,
                                            bytes[dim], 0);
        if (VERBOSE) {
          wcout << L"    " << dim << L" -> " << values[dim] << endl;
        }
      }
      docs[docID] = values;
      shared_ptr<Document> doc = make_shared<Document>();
      doc->push_back(make_shared<BinaryPoint>(L"field", bytes));
      w->addDocument(doc);
    }

    shared_ptr<DirectoryReader> r = w->getReader();
    delete w;

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
        if (VERBOSE) {
          wcout << L"  " << dim << L"\n    min=" << queryMin[dim]
                << L"\n    max=" << queryMax[dim] << endl;
        }
      }

      shared_ptr<BitSet> *const hits = make_shared<BitSet>();
      for (shared_ptr<LeafReaderContext> ctx : r->leaves()) {
        shared_ptr<PointValues> dimValues =
            ctx->reader()->getPointValues(L"field");
        if (dimValues == nullptr) {
          continue;
        }

        constexpr int docBase = ctx->docBase;

        dimValues->intersect(make_shared<IntersectVisitorAnonymousInnerClass4>(
            shared_from_this(), numBytesPerDim, numDims, queryMin, queryMax,
            hits, docBase));
      }

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
    r->close();
  }
}

BasePointsFormatTestCase::IntersectVisitorAnonymousInnerClass4::
    IntersectVisitorAnonymousInnerClass4(
        shared_ptr<BasePointsFormatTestCase> outerInstance, int numBytesPerDim,
        int numDims, deque<std::shared_ptr<int64_t>> &queryMin,
        deque<std::shared_ptr<int64_t>> &queryMax, shared_ptr<BitSet> hits,
        int docBase)
{
  this->outerInstance = outerInstance;
  this->numBytesPerDim = numBytesPerDim;
  this->numDims = numDims;
  this->queryMin = queryMin;
  this->queryMax = queryMax;
  this->hits = hits;
  this->docBase = docBase;
}

void BasePointsFormatTestCase::IntersectVisitorAnonymousInnerClass4::visit(
    int docID)
{
  hits->set(docBase + docID);
  // System.out.println("visit docID=" + docID);
}

void BasePointsFormatTestCase::IntersectVisitorAnonymousInnerClass4::visit(
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
  hits->set(docBase + docID);
}

Relation
BasePointsFormatTestCase::IntersectVisitorAnonymousInnerClass4::compare(
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

void BasePointsFormatTestCase::testRandomBinaryTiny() 
{
  doTestRandomBinary(10);
}

void BasePointsFormatTestCase::testRandomBinaryMedium() 
{
  doTestRandomBinary(10000);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testRandomBinaryBig() throws Exception
void BasePointsFormatTestCase::testRandomBinaryBig() 
{
  assumeFalse(L"too slow with SimpleText",
              Codec::getDefault()->getName() == L"SimpleText");
  doTestRandomBinary(200000);
}

void BasePointsFormatTestCase::doTestRandomBinary(int count) throw(
    runtime_error)
{
  int numDocs = TestUtil::nextInt(random(), count, count * 2);
  int numBytesPerDim =
      TestUtil::nextInt(random(), 2, PointValues::MAX_NUM_BYTES);
  int numDims = TestUtil::nextInt(random(), 1, PointValues::MAX_DIMENSIONS);

  std::deque<std::deque<std::deque<char>>> docValues(numDocs);

  for (int docID = 0; docID < numDocs; docID++) {
    std::deque<std::deque<char>> values(numDims);
    for (int dim = 0; dim < numDims; dim++) {
      values[dim] = std::deque<char>(numBytesPerDim);
      // TODO: sometimes test on a "small" volume too, so we test the high
      // density cases, higher chance of boundary, etc. cases:
      random()->nextBytes(values[dim]);
    }
    docValues[docID] = values;
  }

  verify(docValues, nullptr, numDims, numBytesPerDim);
}

void BasePointsFormatTestCase::verify(
    std::deque<std::deque<std::deque<char>>> &docValues,
    std::deque<int> &docIDs, int numDims,
    int numBytesPerDim) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // getDirectory(docValues.length))
  {
    org::apache::lucene::store::Directory dir = getDirectory(docValues.size());
    while (true) {
      try {
        verify(dir, docValues, docIDs, numDims, numBytesPerDim, false);
        return;
      } catch (const invalid_argument &iae) {
        // This just means we got a too-small maxMB for the maxPointsInLeafNode;
        // just retry
        assertTrue(iae.what()->contains(L"either increase maxMBSortInHeap or "
                                        L"decrease maxPointsInLeafNode"));
      }
    }
  }
}

void BasePointsFormatTestCase::verify(
    shared_ptr<Directory> dir,
    std::deque<std::deque<std::deque<char>>> &docValues,
    std::deque<int> &ids, int numDims, int numBytesPerDim,
    bool expectExceptions) 
{
  int numValues = docValues.size();
  if (VERBOSE) {
    wcout << L"TEST: numValues=" << numValues << L" numDims=" << numDims
          << L" numBytesPerDim=" << numBytesPerDim << endl;
  }

  // RandomIndexWriter is too slow:
  bool useRealWriter = docValues.size() > 10000;

  shared_ptr<IndexWriterConfig> iwc;
  if (useRealWriter) {
    iwc = make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  } else {
    iwc = newIndexWriterConfig();
  }

  if (expectExceptions) {
    shared_ptr<MergeScheduler> ms = iwc->getMergeScheduler();
    if (std::dynamic_pointer_cast<ConcurrentMergeScheduler>(ms) != nullptr) {
      (std::static_pointer_cast<ConcurrentMergeScheduler>(ms))
          ->setSuppressExceptions();
    }
  }
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  shared_ptr<DirectoryReader> r = nullptr;

  // Compute actual min/max values:
  std::deque<std::deque<char>> expectedMinValues(numDims);
  std::deque<std::deque<char>> expectedMaxValues(numDims);
  for (int ord = 0; ord < docValues.size(); ord++) {
    for (int dim = 0; dim < numDims; dim++) {
      if (ord == 0) {
        expectedMinValues[dim] = std::deque<char>(numBytesPerDim);
        System::arraycopy(docValues[ord][dim], 0, expectedMinValues[dim], 0,
                          numBytesPerDim);
        expectedMaxValues[dim] = std::deque<char>(numBytesPerDim);
        System::arraycopy(docValues[ord][dim], 0, expectedMaxValues[dim], 0,
                          numBytesPerDim);
      } else {
        // TODO: it's cheating that we use StringHelper.compare for "truth":
        // what if it's buggy?
        if (StringHelper::compare(numBytesPerDim, docValues[ord][dim], 0,
                                  expectedMinValues[dim], 0) < 0) {
          System::arraycopy(docValues[ord][dim], 0, expectedMinValues[dim], 0,
                            numBytesPerDim);
        }
        if (StringHelper::compare(numBytesPerDim, docValues[ord][dim], 0,
                                  expectedMaxValues[dim], 0) > 0) {
          System::arraycopy(docValues[ord][dim], 0, expectedMaxValues[dim], 0,
                            numBytesPerDim);
        }
      }
    }
  }

  // 20% of the time we add into a separate directory, then at some point use
  // addIndexes to bring the indexed point values to the main directory:
  shared_ptr<Directory> saveDir;
  shared_ptr<RandomIndexWriter> saveW;
  int addIndexesAt;
  if (random()->nextInt(5) == 1) {
    saveDir = dir;
    saveW = w;
    dir = getDirectory(numValues);
    if (useRealWriter) {
      iwc = make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
    } else {
      iwc = newIndexWriterConfig();
    }
    if (expectExceptions) {
      shared_ptr<MergeScheduler> ms = iwc->getMergeScheduler();
      if (std::dynamic_pointer_cast<ConcurrentMergeScheduler>(ms) != nullptr) {
        (std::static_pointer_cast<ConcurrentMergeScheduler>(ms))
            ->setSuppressExceptions();
      }
    }
    w = make_shared<RandomIndexWriter>(random(), dir, iwc);
    addIndexesAt = TestUtil::nextInt(random(), 1, numValues - 1);
  } else {
    saveW.reset();
    saveDir.reset();
    addIndexesAt = 0;
  }

  try {

    shared_ptr<Document> doc;
    int lastID = -1;
    for (int ord = 0; ord < numValues; ord++) {
      int id;
      if (ids.empty()) {
        id = ord;
      } else {
        id = ids[ord];
      }
      if (id != lastID) {
        if (doc->size() > 0) {
          if (useRealWriter) {
            w->w->addDocument(doc);
          } else {
            w->addDocument(doc);
          }
        }
        doc = make_shared<Document>();
        doc->push_back(make_shared<NumericDocValuesField>(L"id", id));
      }
      doc->push_back(make_shared<BinaryPoint>(L"field", docValues[ord]));
      lastID = id;

      if (random()->nextInt(30) == 17) {
        // randomly index some documents without this field
        if (useRealWriter) {
          w->w->addDocument(make_shared<Document>());
        } else {
          w->addDocument(make_shared<Document>());
        }
        if (VERBOSE) {
          wcout << L"add empty doc" << endl;
        }
      }

      if (random()->nextInt(30) == 17) {
        // randomly index some documents with this field, but we will delete
        // them:
        shared_ptr<Document> xdoc = make_shared<Document>();
        xdoc->push_back(make_shared<BinaryPoint>(L"field", docValues[ord]));
        xdoc->push_back(
            make_shared<StringField>(L"nukeme", L"yes", Field::Store::NO));
        if (useRealWriter) {
          w->w->addDocument(xdoc);
        } else {
          w->addDocument(xdoc);
        }
        if (VERBOSE) {
          wcout << L"add doc doc-to-delete" << endl;
        }

        if (random()->nextInt(5) == 1) {
          if (useRealWriter) {
            w->w->deleteDocuments({make_shared<Term>(L"nukeme", L"yes")});
          } else {
            w->deleteDocuments(make_shared<Term>(L"nukeme", L"yes"));
          }
        }
      }

      if (VERBOSE) {
        wcout << L"  ord=" << ord << L" id=" << id << endl;
        for (int dim = 0; dim < numDims; dim++) {
          wcout << L"    dim=" << dim << L" value="
                << make_shared<BytesRef>(docValues[ord][dim]) << endl;
        }
      }

      if (saveW != nullptr && ord >= addIndexesAt) {
        switchIndex(w, dir, saveW);
        w = saveW;
        dir = saveDir;
        saveW.reset();
        saveDir.reset();
      }
    }
    w->addDocument(doc);
    w->deleteDocuments(make_shared<Term>(L"nukeme", L"yes"));

    if (random()->nextBoolean()) {
      if (VERBOSE) {
        wcout << L"\nTEST: now force merge" << endl;
      }
      w->forceMerge(1);
    }

    r = w->getReader();
    delete w;

    if (VERBOSE) {
      wcout << L"TEST: reader=" << r << endl;
    }

    shared_ptr<NumericDocValues> idValues =
        MultiDocValues::getNumericValues(r, L"id");
    std::deque<int> docIDToID(r->maxDoc());
    {
      int docID;
      while ((docID = idValues->nextDoc()) != NO_MORE_DOCS) {
        docIDToID[docID] = static_cast<int>(idValues->longValue());
      }
    }

    shared_ptr<Bits> liveDocs = MultiFields::getLiveDocs(r);

    // Verify min/max values are correct:
    std::deque<char> minValues(numDims * numBytesPerDim);
    Arrays::fill(minValues, static_cast<char>(0xff));

    std::deque<char> maxValues(numDims * numBytesPerDim);

    for (shared_ptr<LeafReaderContext> ctx : r->leaves()) {
      shared_ptr<PointValues> dimValues =
          ctx->reader()->getPointValues(L"field");
      if (dimValues == nullptr) {
        continue;
      }

      std::deque<char> leafMinValues = dimValues->getMinPackedValue();
      std::deque<char> leafMaxValues = dimValues->getMaxPackedValue();
      for (int dim = 0; dim < numDims; dim++) {
        if (StringHelper::compare(numBytesPerDim, leafMinValues,
                                  dim * numBytesPerDim, minValues,
                                  dim * numBytesPerDim) < 0) {
          System::arraycopy(leafMinValues, dim * numBytesPerDim, minValues,
                            dim * numBytesPerDim, numBytesPerDim);
        }
        if (StringHelper::compare(numBytesPerDim, leafMaxValues,
                                  dim * numBytesPerDim, maxValues,
                                  dim * numBytesPerDim) > 0) {
          System::arraycopy(leafMaxValues, dim * numBytesPerDim, maxValues,
                            dim * numBytesPerDim, numBytesPerDim);
        }
      }
    }

    std::deque<char> scratch(numBytesPerDim);
    for (int dim = 0; dim < numDims; dim++) {
      System::arraycopy(minValues, dim * numBytesPerDim, scratch, 0,
                        numBytesPerDim);
      // System.out.println("dim=" + dim + " expectedMin=" + new
      // BytesRef(expectedMinValues[dim]) + " min=" + new BytesRef(scratch));
      assertTrue(Arrays::equals(expectedMinValues[dim], scratch));
      System::arraycopy(maxValues, dim * numBytesPerDim, scratch, 0,
                        numBytesPerDim);
      // System.out.println("dim=" + dim + " expectedMax=" + new
      // BytesRef(expectedMaxValues[dim]) + " max=" + new BytesRef(scratch));
      assertTrue(Arrays::equals(expectedMaxValues[dim], scratch));
    }

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

      if (VERBOSE) {
        for (int dim = 0; dim < numDims; dim++) {
          wcout << L"  dim=" << dim << L"\n    queryMin="
                << make_shared<BytesRef>(queryMin[dim]) << L"\n    queryMax="
                << make_shared<BytesRef>(queryMax[dim]) << endl;
        }
      }

      shared_ptr<BitSet> *const hits = make_shared<BitSet>();

      for (shared_ptr<LeafReaderContext> ctx : r->leaves()) {
        shared_ptr<PointValues> dimValues =
            ctx->reader()->getPointValues(L"field");
        if (dimValues == nullptr) {
          continue;
        }

        constexpr int docBase = ctx->docBase;

        dimValues->intersect(make_shared<IntersectVisitorAnonymousInnerClass>(
            shared_from_this(), numDims, numBytesPerDim, docIDToID, liveDocs,
            queryMin, queryMax, hits, docBase));
      }

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
          int id;
          if (ids.empty()) {
            id = ord;
          } else {
            id = ids[ord];
          }
          expected->set(id);
        }
      }

      int limit = max(expected->length(), hits->length());
      int failCount = 0;
      int successCount = 0;
      for (int id = 0; id < limit; id++) {
        if (expected->get(id) != hits->get(id)) {
          wcout << L"FAIL: id=" << id << endl;
          failCount++;
        } else {
          successCount++;
        }
      }

      if (failCount != 0) {
        for (int docID = 0; docID < r->maxDoc(); docID++) {
          wcout << L"  docID=" << docID << L" id=" << docIDToID[docID] << endl;
        }

        fail(to_wstring(failCount) + L" docs failed; " +
             to_wstring(successCount) + L" docs succeeded");
      }
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::closeWhileHandlingException(
        {r, w, saveW, saveDir == nullptr ? nullptr : dir});
  }
}

BasePointsFormatTestCase::IntersectVisitorAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass(
        shared_ptr<BasePointsFormatTestCase> outerInstance, int numDims,
        int numBytesPerDim, deque<int> &docIDToID, shared_ptr<Bits> liveDocs,
        deque<deque<char>> &queryMin, deque<deque<char>> &queryMax,
        shared_ptr<BitSet> hits, int docBase)
{
  this->outerInstance = outerInstance;
  this->numDims = numDims;
  this->numBytesPerDim = numBytesPerDim;
  this->docIDToID = docIDToID;
  this->liveDocs = liveDocs;
  this->queryMin = queryMin;
  this->queryMax = queryMax;
  this->hits = hits;
  this->docBase = docBase;
}

void BasePointsFormatTestCase::IntersectVisitorAnonymousInnerClass::visit(
    int docID)
{
  if (liveDocs == nullptr || liveDocs->get(docBase + docID)) {
    hits->set(docIDToID[docBase + docID]);
  }
  // System.out.println("visit docID=" + docID);
}

void BasePointsFormatTestCase::IntersectVisitorAnonymousInnerClass::visit(
    int docID, std::deque<char> &packedValue)
{
  if (liveDocs != nullptr && liveDocs->get(docBase + docID) == false) {
    return;
  }

  for (int dim = 0; dim < numDims; dim++) {
    // System.out.println("  dim=" + dim + " value=" + new BytesRef(packedValue,
    // dim*numBytesPerDim, numBytesPerDim));
    if (StringHelper::compare(numBytesPerDim, packedValue, dim * numBytesPerDim,
                              queryMin[dim], 0) < 0 ||
        StringHelper::compare(numBytesPerDim, packedValue, dim * numBytesPerDim,
                              queryMax[dim], 0) > 0) {
      // System.out.println("  no");
      return;
    }
  }

  // System.out.println("  yes");
  hits->set(docIDToID[docBase + docID]);
}

Relation BasePointsFormatTestCase::IntersectVisitorAnonymousInnerClass::compare(
    std::deque<char> &minPacked, std::deque<char> &maxPacked)
{
  bool crosses = false;
  // System.out.println("compare");
  for (int dim = 0; dim < numDims; dim++) {
    if (StringHelper::compare(numBytesPerDim, maxPacked, dim * numBytesPerDim,
                              queryMin[dim], 0) < 0 ||
        StringHelper::compare(numBytesPerDim, minPacked, dim * numBytesPerDim,
                              queryMax[dim], 0) > 0) {
      // System.out.println("  query_outside_cell");
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
    // System.out.println("  query_crosses_cell");
    return Relation::CELL_CROSSES_QUERY;
  } else {
    // System.out.println("  cell_inside_query");
    return Relation::CELL_INSIDE_QUERY;
  }
}

void BasePointsFormatTestCase::testAddIndexes() 
{
  shared_ptr<Directory> dir1 = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir1);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(L"int1", 17));
  w->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(L"int2", 42));
  w->addDocument(doc);
  delete w;

  // Different field number assigments:
  shared_ptr<Directory> dir2 = newDirectory();
  w = make_shared<RandomIndexWriter>(random(), dir2);
  doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(L"int2", 42));
  w->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(L"int1", 17));
  w->addDocument(doc);
  delete w;

  shared_ptr<Directory> dir = newDirectory();
  w = make_shared<RandomIndexWriter>(random(), dir);
  w->addIndexes(std::deque<std::shared_ptr<Directory>>{dir1, dir2});
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = w->getReader();
  shared_ptr<IndexSearcher> s = newSearcher(r, false);
  TestUtil::assertEquals(2, s->count(IntPoint::newExactQuery(L"int1", 17)));
  TestUtil::assertEquals(2, s->count(IntPoint::newExactQuery(L"int2", 42)));
  r->close();
  delete w;
  delete dir;
  delete dir1;
  delete dir2;
}

void BasePointsFormatTestCase::switchIndex(
    shared_ptr<RandomIndexWriter> w, shared_ptr<Directory> dir,
    shared_ptr<RandomIndexWriter> saveW) 
{
  if (random()->nextBoolean()) {
    // Add via readers:
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (DirectoryReader r = w.getReader())
    {
      DirectoryReader r = w->getReader();
      if (random()->nextBoolean()) {
        // Add via CodecReaders:
        deque<std::shared_ptr<CodecReader>> subs =
            deque<std::shared_ptr<CodecReader>>();
        for (shared_ptr<LeafReaderContext> context : r.leaves()) {
          subs.push_back(
              std::static_pointer_cast<CodecReader>(context->reader()));
        }
        if (VERBOSE) {
          wcout << L"TEST: now use addIndexes(CodecReader[]) to switch writers"
                << endl;
        }
        saveW->addIndexes({subs.toArray(
            std::deque<std::shared_ptr<CodecReader>>(subs.size()))});
      } else {
        if (VERBOSE) {
          wcout
              << L"TEST: now use TestUtil.addIndexesSlowly(DirectoryReader[]) "
                 L"to switch writers"
              << endl;
        }
        TestUtil::addIndexesSlowly(saveW->w, {r});
      }
    }
  } else {
    // Add via directory:
    if (VERBOSE) {
      wcout << L"TEST: now use addIndexes(Directory[]) to switch writers"
            << endl;
    }
    delete w;
    saveW->addIndexes(std::deque<std::shared_ptr<Directory>>{dir});
  }
  delete w;
  delete dir;
}

shared_ptr<int64_t> BasePointsFormatTestCase::randomBigInt(int numBytes)
{
  shared_ptr<int64_t> x =
      make_shared<int64_t>(numBytes * 8 - 1, random());
  if (random()->nextBoolean()) {
    x = x->negate();
  }
  return x;
}

shared_ptr<Directory>
BasePointsFormatTestCase::getDirectory(int numPoints) 
{
  shared_ptr<Directory> dir;
  if (numPoints > 100000) {
    dir = newFSDirectory(createTempDir(L"TestBKDTree"));
  } else {
    dir = newDirectory();
  }
  // dir = FSDirectory.open(createTempDir());
  return dir;
}

bool BasePointsFormatTestCase::mergeIsStable()
{
  // suppress this test from base class: merges for BKD trees are not stable
  // because the tree created by merge will have a different structure than the
  // tree created by adding points separately
  return false;
}

void BasePointsFormatTestCase::testMixedSchema() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  iwc->setMaxBufferedDocs(2);
  for (int i = 0; i < 2; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<StringField>(L"id", Integer::toString(i),
                                            Field::Store::NO));
    doc->push_back(make_shared<IntPoint>(L"int", i));
    w->addDocument(doc);
  }
  // index has 1 segment now (with 2 docs) and that segment does have points,
  // but the "id" field in particular does NOT

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(L"id", 0));
  w->addDocument(doc);
  // now we write another segment where the id field does have points:

  w->forceMerge(1);
  IOUtils::close({w, dir});
}
} // namespace org::apache::lucene::index