using namespace std;

#include "TestReaderClosed.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TermRangeQuery = org::apache::lucene::search::TermRangeQuery;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestReaderClosed::setUp() 
{
  LuceneTestCase::setUp();
  dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(
          make_shared<MockAnalyzer>(random(), MockTokenizer::KEYWORD, false))
          ->setMaxBufferedDocs(TestUtil::nextInt(random(), 50, 1000)));

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> field = newStringField(L"field", L"", Field::Store::NO);
  doc->push_back(field);

  // we generate aweful prefixes: good for testing.
  // but for preflex codec, the test can be very slow, so use less iterations.
  int num = atLeast(10);
  for (int i = 0; i < num; i++) {
    field->setStringValue(TestUtil::randomUnicodeString(random(), 10));
    writer->addDocument(doc);
  }
  writer->forceMerge(1);
  reader = writer->getReader();
  delete writer;
}

void TestReaderClosed::test() 
{
  assertTrue(reader->getRefCount() > 0);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  shared_ptr<TermRangeQuery> query =
      TermRangeQuery::newStringRange(L"field", L"a", L"z", true, true);
  searcher->search(query, 5);
  reader->close();
  try {
    searcher->search(query, 5);
  } catch (const AlreadyClosedException &ace) {
    // expected
  } catch (const RejectedExecutionException &ree) {
    // expected if the searcher has been created with threads since
    // LuceneTestCase closes the thread-pool in a reader close listener
  }
}

void TestReaderClosed::testReaderChaining() 
{
  assertTrue(reader->getRefCount() > 0);
  shared_ptr<LeafReader> wrappedReader =
      make_shared<ParallelLeafReader>(getOnlyLeafReader(reader));

  // We wrap with a OwnCacheKeyMultiReader so that closing the underlying reader
  // does not terminate the threadpool (if that index searcher uses one)
  shared_ptr<IndexSearcher> searcher =
      newSearcher(make_shared<OwnCacheKeyMultiReader>(wrappedReader));

  shared_ptr<TermRangeQuery> query =
      TermRangeQuery::newStringRange(L"field", L"a", L"z", true, true);
  searcher->search(query, 5);
  reader->close(); // close original child reader
  try {
    searcher->search(query, 5);
  } catch (const runtime_error &e) {
    shared_ptr<AlreadyClosedException> ace = nullptr;
    for (runtime_error t = e; t != nullptr; t = t.getCause()) {
      if (std::dynamic_pointer_cast<AlreadyClosedException>(t) != nullptr) {
        ace = std::static_pointer_cast<AlreadyClosedException>(t);
      }
    }
    if (ace == nullptr) {
      throw make_shared<AssertionError>(
          L"Query failed, but not due to an AlreadyClosedException", e);
    }
    TestUtil::assertEquals(L"this IndexReader cannot be used anymore as one of "
                           L"its child readers was closed",
                           ace->getMessage());
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    // close executor: in case of wrap-wrap-wrapping
    delete searcher->getIndexReader();
  }
}

void TestReaderClosed::tearDown() 
{
  delete dir;
  LuceneTestCase::tearDown();
}
} // namespace org::apache::lucene::index