using namespace std;

#include "TestDocIdsWriter.h"

namespace org::apache::lucene::util::bkd
{
using IntersectVisitor =
    org::apache::lucene::index::PointValues::IntersectVisitor;
using Relation = org::apache::lucene::index::PointValues::Relation;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestDocIdsWriter::testRandom() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory())
  {
    org::apache::lucene::store::Directory dir = newDirectory();
    for (int iter = 0; iter < 1000; ++iter) {
      std::deque<int> docIDs(random()->nextInt(5000));
      constexpr int bpv = TestUtil::nextInt(random(), 1, 32);
      for (int i = 0; i < docIDs.size(); ++i) {
        docIDs[i] = TestUtil::nextInt(random(), 0, (1 << bpv) - 1);
      }
      test(dir, docIDs);
    }
  }
}

void TestDocIdsWriter::testSorted() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory())
  {
    org::apache::lucene::store::Directory dir = newDirectory();
    for (int iter = 0; iter < 1000; ++iter) {
      std::deque<int> docIDs(random()->nextInt(5000));
      constexpr int bpv = TestUtil::nextInt(random(), 1, 32);
      for (int i = 0; i < docIDs.size(); ++i) {
        docIDs[i] = TestUtil::nextInt(random(), 0, (1 << bpv) - 1);
      }
      Arrays::sort(docIDs);
      test(dir, docIDs);
    }
  }
}

void TestDocIdsWriter::test(shared_ptr<Directory> dir,
                            std::deque<int> &ints) 
{
  constexpr int64_t len;
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try(org.apache.lucene.store.IndexOutput out =
  // dir.createOutput("tmp", org.apache.lucene.store.IOContext.DEFAULT))
  {
    org::apache::lucene::store::IndexOutput out = dir->createOutput(
        L"tmp", org::apache::lucene::store::IOContext::DEFAULT);
    DocIdsWriter::writeDocIds(ints, 0, ints.size(), out);
    len = out->getFilePointer();
    if (random()->nextBoolean()) {
      out->writeLong(0); // garbage
    }
  }
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexInput in =
  // dir.openInput("tmp", org.apache.lucene.store.IOContext.READONCE))
  {
    org::apache::lucene::store::IndexInput in_ =
        dir->openInput(L"tmp", org::apache::lucene::store::IOContext::READONCE);
    std::deque<int> read(ints.size());
    DocIdsWriter::readInts(in_, ints.size(), read);
    assertArrayEquals(ints, read);
    TestUtil::assertEquals(len, in_->getFilePointer());
  }
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexInput in =
  // dir.openInput("tmp", org.apache.lucene.store.IOContext.READONCE))
  {
    org::apache::lucene::store::IndexInput in_ =
        dir->openInput(L"tmp", org::apache::lucene::store::IOContext::READONCE);
    std::deque<int> read(ints.size());
    DocIdsWriter::readInts(in_, ints.size(),
                           make_shared<IntersectVisitorAnonymousInnerClass>(
                               shared_from_this(), read));
    assertArrayEquals(ints, read);
    TestUtil::assertEquals(len, in_->getFilePointer());
  }
  dir->deleteFile(L"tmp");
}

TestDocIdsWriter::IntersectVisitorAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass(
        shared_ptr<TestDocIdsWriter> outerInstance, deque<int> &read)
{
  this->outerInstance = outerInstance;
  this->read = read;
  i = 0;
}

void TestDocIdsWriter::IntersectVisitorAnonymousInnerClass::visit(
    int docID) 
{
  read[i++] = docID;
}

void TestDocIdsWriter::IntersectVisitorAnonymousInnerClass::visit(
    int docID, std::deque<char> &packedValue) 
{
  throw make_shared<UnsupportedOperationException>();
}

Relation TestDocIdsWriter::IntersectVisitorAnonymousInnerClass::compare(
    std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue)
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::util::bkd