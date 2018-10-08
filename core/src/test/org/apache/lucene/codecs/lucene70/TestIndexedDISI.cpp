using namespace std;

#include "TestIndexedDISI.h"

namespace org::apache::lucene::codecs::lucene70
{
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using BitSetIterator = org::apache::lucene::util::BitSetIterator;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestIndexedDISI::testEmpty() 
{
  int maxDoc = TestUtil::nextInt(random(), 1, 100000);
  shared_ptr<FixedBitSet> set = make_shared<FixedBitSet>(maxDoc);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory())
  {
    org::apache::lucene::store::Directory dir = newDirectory();
    doTest(set, dir);
  }
}

void TestIndexedDISI::testOneDoc() 
{
  int maxDoc = TestUtil::nextInt(random(), 1, 100000);
  shared_ptr<FixedBitSet> set = make_shared<FixedBitSet>(maxDoc);
  set->set(random()->nextInt(maxDoc));
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory())
  {
    org::apache::lucene::store::Directory dir = newDirectory();
    doTest(set, dir);
  }
}

void TestIndexedDISI::testTwoDocs() 
{
  int maxDoc = TestUtil::nextInt(random(), 1, 100000);
  shared_ptr<FixedBitSet> set = make_shared<FixedBitSet>(maxDoc);
  set->set(random()->nextInt(maxDoc));
  set->set(random()->nextInt(maxDoc));
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory())
  {
    org::apache::lucene::store::Directory dir = newDirectory();
    doTest(set, dir);
  }
}

void TestIndexedDISI::testAllDocs() 
{
  int maxDoc = TestUtil::nextInt(random(), 1, 100000);
  shared_ptr<FixedBitSet> set = make_shared<FixedBitSet>(maxDoc);
  set->set(1, maxDoc);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory())
  {
    org::apache::lucene::store::Directory dir = newDirectory();
    doTest(set, dir);
  }
}

void TestIndexedDISI::testHalfFull() 
{
  int maxDoc = TestUtil::nextInt(random(), 1, 100000);
  shared_ptr<FixedBitSet> set = make_shared<FixedBitSet>(maxDoc);
  for (int i = random()->nextInt(2); i < maxDoc;
       i += TestUtil::nextInt(random(), 1, 3)) {
    set->set(i);
  }
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory())
  {
    org::apache::lucene::store::Directory dir = newDirectory();
    doTest(set, dir);
  }
}

void TestIndexedDISI::testDocRange() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory())
  {
    org::apache::lucene::store::Directory dir = newDirectory();
    for (int iter = 0; iter < 10; ++iter) {
      int maxDoc = TestUtil::nextInt(random(), 1, 1000000);
      shared_ptr<FixedBitSet> set = make_shared<FixedBitSet>(maxDoc);
      constexpr int start = random()->nextInt(maxDoc);
      constexpr int end = TestUtil::nextInt(random(), start + 1, maxDoc);
      set->set(start, end);
      doTest(set, dir);
    }
  }
}

void TestIndexedDISI::testSparseDenseBoundary() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory())
  {
    org::apache::lucene::store::Directory dir = newDirectory();
    shared_ptr<FixedBitSet> set = make_shared<FixedBitSet>(200000);
    int start = 65536 + random()->nextInt(100);

    // we set MAX_ARRAY_LENGTH bits so the encoding will be sparse
    set->set(start, start + IndexedDISI::MAX_ARRAY_LENGTH);
    int64_t length;
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput out =
    // dir.createOutput("sparse", org.apache.lucene.store.IOContext.DEFAULT))
    {
      org::apache::lucene::store::IndexOutput out = dir->createOutput(
          L"sparse", org::apache::lucene::store::IOContext::DEFAULT);
      IndexedDISI::writeBitSet(
          make_shared<BitSetIterator>(set, IndexedDISI::MAX_ARRAY_LENGTH), out);
      length = out->getFilePointer();
    }
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexInput in =
    // dir.openInput("sparse", org.apache.lucene.store.IOContext.DEFAULT))
    {
      org::apache::lucene::store::IndexInput in_ = dir->openInput(
          L"sparse", org::apache::lucene::store::IOContext::DEFAULT);
      shared_ptr<IndexedDISI> disi = make_shared<IndexedDISI>(
          in_, 0LL, length, IndexedDISI::MAX_ARRAY_LENGTH);
      TestUtil::assertEquals(start, disi->nextDoc());
      TestUtil::assertEquals(IndexedDISI::Method::SPARSE, disi->method);
    }
    doTest(set, dir);

    // now we set one more bit so the encoding will be dense
    set->set(start + IndexedDISI::MAX_ARRAY_LENGTH + random()->nextInt(100));
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput out =
    // dir.createOutput("bar", org.apache.lucene.store.IOContext.DEFAULT))
    {
      org::apache::lucene::store::IndexOutput out = dir->createOutput(
          L"bar", org::apache::lucene::store::IOContext::DEFAULT);
      IndexedDISI::writeBitSet(
          make_shared<BitSetIterator>(set, IndexedDISI::MAX_ARRAY_LENGTH + 1),
          out);
      length = out->getFilePointer();
    }
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexInput in =
    // dir.openInput("bar", org.apache.lucene.store.IOContext.DEFAULT))
    {
      org::apache::lucene::store::IndexInput in_ = dir->openInput(
          L"bar", org::apache::lucene::store::IOContext::DEFAULT);
      shared_ptr<IndexedDISI> disi = make_shared<IndexedDISI>(
          in_, 0LL, length, IndexedDISI::MAX_ARRAY_LENGTH + 1);
      TestUtil::assertEquals(start, disi->nextDoc());
      TestUtil::assertEquals(IndexedDISI::Method::DENSE, disi->method);
    }
    doTest(set, dir);
  }
}

void TestIndexedDISI::testOneDocMissing() 
{
  int maxDoc = TestUtil::nextInt(random(), 1, 1000000);
  shared_ptr<FixedBitSet> set = make_shared<FixedBitSet>(maxDoc);
  set->set(0, maxDoc);
  set->clear(random()->nextInt(maxDoc));
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory())
  {
    org::apache::lucene::store::Directory dir = newDirectory();
    doTest(set, dir);
  }
}

void TestIndexedDISI::testFewMissingDocs() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory())
  {
    org::apache::lucene::store::Directory dir = newDirectory();
    for (int iter = 0; iter < 100; ++iter) {
      int maxDoc = TestUtil::nextInt(random(), 1, 100000);
      shared_ptr<FixedBitSet> set = make_shared<FixedBitSet>(maxDoc);
      set->set(0, maxDoc);
      constexpr int numMissingDocs = TestUtil::nextInt(random(), 2, 1000);
      for (int i = 0; i < numMissingDocs; ++i) {
        set->clear(random()->nextInt(maxDoc));
      }
      doTest(set, dir);
    }
  }
}

void TestIndexedDISI::testRandom() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory())
  {
    org::apache::lucene::store::Directory dir = newDirectory();
    for (int i = 0; i < 10; ++i) {
      doTestRandom(dir);
    }
  }
}

void TestIndexedDISI::doTestRandom(shared_ptr<Directory> dir) 
{
  deque<int> docs = deque<int>();
  constexpr int maxStep =
      TestUtil::nextInt(random(), 1, 1 << TestUtil::nextInt(random(), 2, 20));
  constexpr int numDocs = TestUtil::nextInt(
      random(), 1, min(100000, numeric_limits<int>::max() / maxStep));
  for (int doc = -1, i = 0; i < numDocs; ++i) {
    doc += TestUtil::nextInt(random(), 1, maxStep);
    docs.push_back(doc);
  }
  constexpr int maxDoc =
      docs[docs.size() - 1] + TestUtil::nextInt(random(), 1, 100);

  shared_ptr<FixedBitSet> set = make_shared<FixedBitSet>(maxDoc);
  for (auto doc : docs) {
    set->set(doc);
  }

  doTest(set, dir);
}

void TestIndexedDISI::doTest(shared_ptr<FixedBitSet> set,
                             shared_ptr<Directory> dir) 
{
  constexpr int cardinality = set->cardinality();
  int64_t length;
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput out =
  // dir.createOutput("foo", org.apache.lucene.store.IOContext.DEFAULT))
  {
    org::apache::lucene::store::IndexOutput out = dir->createOutput(
        L"foo", org::apache::lucene::store::IOContext::DEFAULT);
    IndexedDISI::writeBitSet(make_shared<BitSetIterator>(set, cardinality),
                             out);
    length = out->getFilePointer();
  }

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexInput in =
  // dir.openInput("foo", org.apache.lucene.store.IOContext.DEFAULT))
  {
    org::apache::lucene::store::IndexInput in_ =
        dir->openInput(L"foo", org::apache::lucene::store::IOContext::DEFAULT);
    shared_ptr<IndexedDISI> disi =
        make_shared<IndexedDISI>(in_, 0LL, length, cardinality);
    shared_ptr<BitSetIterator> disi2 =
        make_shared<BitSetIterator>(set, cardinality);
    int i = 0;
    for (int doc = disi2->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
         doc = disi2->nextDoc()) {
      TestUtil::assertEquals(doc, disi->nextDoc());
      TestUtil::assertEquals(i++, disi->index());
    }
    TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, disi->nextDoc());
  }

  for (auto step : std::deque<int>{1, 10, 100, 1000, 10000, 100000}) {
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexInput in =
    // dir.openInput("foo", org.apache.lucene.store.IOContext.DEFAULT))
    {
      org::apache::lucene::store::IndexInput in_ = dir->openInput(
          L"foo", org::apache::lucene::store::IOContext::DEFAULT);
      shared_ptr<IndexedDISI> disi =
          make_shared<IndexedDISI>(in_, 0LL, length, cardinality);
      shared_ptr<BitSetIterator> disi2 =
          make_shared<BitSetIterator>(set, cardinality);
      int index = -1;
      while (true) {
        int target = disi2->docID() + step;
        int doc;
        do {
          doc = disi2->nextDoc();
          index++;
        } while (doc < target);
        TestUtil::assertEquals(doc, disi->advance(target));
        if (doc == DocIdSetIterator::NO_MORE_DOCS) {
          break;
        }
        TestUtil::assertEquals(index, disi->index());
      }
    }
  }

  for (auto step : std::deque<int>{10, 100, 1000, 10000, 100000}) {
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexInput in =
    // dir.openInput("foo", org.apache.lucene.store.IOContext.DEFAULT))
    {
      org::apache::lucene::store::IndexInput in_ = dir->openInput(
          L"foo", org::apache::lucene::store::IOContext::DEFAULT);
      shared_ptr<IndexedDISI> disi =
          make_shared<IndexedDISI>(in_, 0LL, length, cardinality);
      shared_ptr<BitSetIterator> disi2 =
          make_shared<BitSetIterator>(set, cardinality);
      int index = -1;
      for (int target = 0; target < set->length();) {
        target += TestUtil::nextInt(random(), 0, step);
        int doc = disi2->docID();
        while (doc < target) {
          doc = disi2->nextDoc();
          index++;
        }

        bool exists = disi->advanceExact(target);
        TestUtil::assertEquals(doc == target, exists);
        if (exists) {
          TestUtil::assertEquals(index, disi->index());
        } else if (random()->nextBoolean()) {
          TestUtil::assertEquals(doc, disi->nextDoc());
          TestUtil::assertEquals(index, disi->index());
          target = doc;
        }
      }
    }
  }

  dir->deleteFile(L"foo");
}
} // namespace org::apache::lucene::codecs::lucene70