using namespace std;

#include "TestExitableDirectoryReader.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using ExitingReaderException =
    org::apache::lucene::index::ExitableDirectoryReader::ExitingReaderException;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using PrefixQuery = org::apache::lucene::search::PrefixQuery;
using Query = org::apache::lucene::search::Query;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Ignore;

TestExitableDirectoryReader::TestReader::TestTerms::TestTerms(
    shared_ptr<Terms> in_)
    : FilterTerms(in_)
{
}

shared_ptr<TermsEnum>
TestExitableDirectoryReader::TestReader::TestTerms::iterator() throw(
    IOException)
{
  return make_shared<TestTermsEnum>(FilterTerms::begin());
}

TestExitableDirectoryReader::TestReader::TestTermsEnum::TestTermsEnum(
    shared_ptr<TermsEnum> in_)
    : FilterTermsEnum(in_)
{
}

shared_ptr<BytesRef>
TestExitableDirectoryReader::TestReader::TestTermsEnum::next() throw(
    IOException)
{
  try {
    // Sleep for 100ms before each .next() call.
    delay(100);
  } catch (const InterruptedException &e) {
  }
  return in_->next();
}

TestExitableDirectoryReader::TestReader::TestReader(
    shared_ptr<LeafReader> reader) 
    : FilterLeafReader(reader)
{
}

shared_ptr<Terms> TestExitableDirectoryReader::TestReader::terms(
    const wstring &field) 
{
  shared_ptr<Terms> terms = FilterLeafReader::terms(field);
  return terms == nullptr ? nullptr : make_shared<TestTerms>(terms);
}

shared_ptr<CacheHelper>
TestExitableDirectoryReader::TestReader::getCoreCacheHelper()
{
  return in_->getCoreCacheHelper();
}

shared_ptr<CacheHelper>
TestExitableDirectoryReader::TestReader::getReaderCacheHelper()
{
  return in_->getReaderCacheHelper();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Ignore("this test relies on wall clock time and sometimes
// false fails") public void testExitableFilterIndexReader() throws Exception
void TestExitableDirectoryReader::testExitableFilterIndexReader() throw(
    runtime_error)
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      directory, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));

  shared_ptr<Document> d1 = make_shared<Document>();
  d1->push_back(newTextField(L"default", L"one two", Field::Store::YES));
  writer->addDocument(d1);

  shared_ptr<Document> d2 = make_shared<Document>();
  d2->push_back(newTextField(L"default", L"one three", Field::Store::YES));
  writer->addDocument(d2);

  shared_ptr<Document> d3 = make_shared<Document>();
  d3->push_back(newTextField(L"default", L"ones two four", Field::Store::YES));
  writer->addDocument(d3);
  writer->forceMerge(1);

  writer->commit();
  delete writer;

  shared_ptr<DirectoryReader> directoryReader;
  shared_ptr<DirectoryReader> exitableDirectoryReader;
  shared_ptr<IndexReader> reader;
  shared_ptr<IndexSearcher> searcher;

  shared_ptr<Query> query =
      make_shared<PrefixQuery>(make_shared<Term>(L"default", L"o"));

  // Set a fairly high timeout value (1 second) and expect the query to complete
  // in that time frame. Not checking the validity of the result, all we are
  // bothered about in this test is the timing out.
  directoryReader = DirectoryReader::open(directory);
  exitableDirectoryReader = make_shared<ExitableDirectoryReader>(
      directoryReader, make_shared<QueryTimeoutImpl>(1000));
  reader = make_shared<TestReader>(getOnlyLeafReader(exitableDirectoryReader));
  searcher = make_shared<IndexSearcher>(reader);
  searcher->search(query, 10);
  delete reader;

  // Set a really low timeout value (1 millisecond) and expect an Exception
  directoryReader = DirectoryReader::open(directory);
  exitableDirectoryReader = make_shared<ExitableDirectoryReader>(
      directoryReader, make_shared<QueryTimeoutImpl>(1));
  reader = make_shared<TestReader>(getOnlyLeafReader(exitableDirectoryReader));
  shared_ptr<IndexSearcher> slowSearcher = make_shared<IndexSearcher>(reader);
  expectThrows(ExitingReaderException::typeid,
               [&]() { slowSearcher->search(query, 10); });
  delete reader;

  // Set maximum time out and expect the query to complete.
  // Not checking the validity of the result, all we are bothered about in this
  // test is the timing out.
  directoryReader = DirectoryReader::open(directory);
  exitableDirectoryReader = make_shared<ExitableDirectoryReader>(
      directoryReader,
      make_shared<QueryTimeoutImpl>(numeric_limits<int64_t>::max()));
  reader = make_shared<TestReader>(getOnlyLeafReader(exitableDirectoryReader));
  searcher = make_shared<IndexSearcher>(reader);
  searcher->search(query, 10);
  delete reader;

  // Set a negative time allowed and expect the query to complete (should
  // disable timeouts) Not checking the validity of the result, all we are
  // bothered about in this test is the timing out.
  directoryReader = DirectoryReader::open(directory);
  exitableDirectoryReader = make_shared<ExitableDirectoryReader>(
      directoryReader, make_shared<QueryTimeoutImpl>(-189034LL));
  reader = make_shared<TestReader>(getOnlyLeafReader(exitableDirectoryReader));
  searcher = make_shared<IndexSearcher>(reader);
  searcher->search(query, 10);
  delete reader;

  delete directory;
}
} // namespace org::apache::lucene::index