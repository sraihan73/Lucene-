using namespace std;

#include "Test2BBKDPoints.h"

namespace org::apache::lucene::util::bkd
{
using CheckIndex = org::apache::lucene::index::CheckIndex;
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using org::apache::lucene::util::LuceneTestCase::Monster;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using NumericUtils = org::apache::lucene::util::NumericUtils;
using TimeUnits = org::apache::lucene::util::TimeUnits;
using com::carrotsearch::randomizedtesting::annotations::TimeoutSuite;

void Test2BBKDPoints::test1D() 
{
  shared_ptr<Directory> dir =
      FSDirectory::open(createTempDir(L"2BBKDPoints1D"));

  constexpr int numDocs = (numeric_limits<int>::max() / 26) + 100;

  shared_ptr<BKDWriter> w = make_shared<BKDWriter>(
      numDocs, dir, L"_0", 1, Long::BYTES,
      BKDWriter::DEFAULT_MAX_POINTS_IN_LEAF_NODE,
      BKDWriter::DEFAULT_MAX_MB_SORT_IN_HEAP, 26LL * numDocs, false);
  int counter = 0;
  std::deque<char> packedBytes(Long::BYTES);
  for (int docID = 0; docID < numDocs; docID++) {
    for (int j = 0; j < 26; j++) {
      // first a random int:
      NumericUtils::intToSortableBytes(random()->nextInt(), packedBytes, 0);
      // then our counter, which will overflow a bit in the end:
      NumericUtils::intToSortableBytes(counter, packedBytes, Integer::BYTES);
      w->add(packedBytes, docID);
      counter++;
    }
    if (VERBOSE && docID % 100000 == 0) {
      wcout << docID << L" of " << numDocs << L"..." << endl;
    }
  }
  shared_ptr<IndexOutput> out =
      dir->createOutput(L"1d.bkd", IOContext::DEFAULT);
  int64_t indexFP = w->finish(out);
  delete out;

  shared_ptr<IndexInput> in_ = dir->openInput(L"1d.bkd", IOContext::DEFAULT);
  in_->seek(indexFP);
  shared_ptr<BKDReader> r = make_shared<BKDReader>(in_);
  shared_ptr<CheckIndex::VerifyPointsVisitor> visitor =
      make_shared<CheckIndex::VerifyPointsVisitor>(L"1d", numDocs, r);
  r->intersect(visitor);
  assertEquals(r->size(), visitor->getPointCountSeen());
  assertEquals(r->getDocCount(), visitor->getDocCountSeen());
  delete in_;
  delete dir;
}

void Test2BBKDPoints::test2D() 
{
  shared_ptr<Directory> dir =
      FSDirectory::open(createTempDir(L"2BBKDPoints2D"));

  constexpr int numDocs = (numeric_limits<int>::max() / 26) + 100;

  shared_ptr<BKDWriter> w = make_shared<BKDWriter>(
      numDocs, dir, L"_0", 2, Long::BYTES,
      BKDWriter::DEFAULT_MAX_POINTS_IN_LEAF_NODE,
      BKDWriter::DEFAULT_MAX_MB_SORT_IN_HEAP, 26LL * numDocs, false);
  int counter = 0;
  std::deque<char> packedBytes(2 * Long::BYTES);
  for (int docID = 0; docID < numDocs; docID++) {
    for (int j = 0; j < 26; j++) {
      // first a random int:
      NumericUtils::intToSortableBytes(random()->nextInt(), packedBytes, 0);
      // then our counter, which will overflow a bit in the end:
      NumericUtils::intToSortableBytes(counter, packedBytes, Integer::BYTES);
      // then two random ints for the 2nd dimension:
      NumericUtils::intToSortableBytes(random()->nextInt(), packedBytes,
                                       Long::BYTES);
      NumericUtils::intToSortableBytes(random()->nextInt(), packedBytes,
                                       Long::BYTES + Integer::BYTES);
      w->add(packedBytes, docID);
      counter++;
    }
    if (VERBOSE && docID % 100000 == 0) {
      wcout << docID << L" of " << numDocs << L"..." << endl;
    }
  }
  shared_ptr<IndexOutput> out =
      dir->createOutput(L"2d.bkd", IOContext::DEFAULT);
  int64_t indexFP = w->finish(out);
  delete out;

  shared_ptr<IndexInput> in_ = dir->openInput(L"2d.bkd", IOContext::DEFAULT);
  in_->seek(indexFP);
  shared_ptr<BKDReader> r = make_shared<BKDReader>(in_);
  shared_ptr<CheckIndex::VerifyPointsVisitor> visitor =
      make_shared<CheckIndex::VerifyPointsVisitor>(L"2d", numDocs, r);
  r->intersect(visitor);
  assertEquals(r->size(), visitor->getPointCountSeen());
  assertEquals(r->getDocCount(), visitor->getDocCountSeen());
  delete in_;
  delete dir;
}
} // namespace org::apache::lucene::util::bkd