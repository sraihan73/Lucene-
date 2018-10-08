using namespace std;

#include "TestCrashCausesCorruptIndex.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;
using FilterDirectory = org::apache::lucene::store::FilterDirectory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestCrashCausesCorruptIndex::testCrashCorruptsIndexing() throw(
    runtime_error)
{
  path = createTempDir(L"testCrashCorruptsIndexing");

  indexAndCrashOnCreateOutputSegments2();

  searchForFleas(2);

  indexAfterRestart();

  searchForFleas(3);
}

void TestCrashCausesCorruptIndex::indexAndCrashOnCreateOutputSegments2() throw(
    IOException)
{
  shared_ptr<Directory> realDirectory = FSDirectory::open(path);
  shared_ptr<CrashAfterCreateOutput> crashAfterCreateOutput =
      make_shared<CrashAfterCreateOutput>(realDirectory);

  // NOTE: cannot use RandomIndexWriter because it
  // sometimes commits:
  shared_ptr<IndexWriter> indexWriter = make_shared<IndexWriter>(
      crashAfterCreateOutput,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random())));

  indexWriter->addDocument(getDocument());
  // writes segments_1:
  indexWriter->commit();

  crashAfterCreateOutput->setCrashAfterCreateOutput(L"pending_segments_2");
  indexWriter->addDocument(getDocument());
  // tries to write segments_2 but hits fake exc:
  expectThrows(CrashingException::typeid, [&]() { indexWriter->commit(); });

  // writes segments_3
  delete indexWriter;
  assertFalse(slowFileExists(realDirectory, L"segments_2"));
  delete crashAfterCreateOutput;
}

void TestCrashCausesCorruptIndex::indexAfterRestart() 
{
  shared_ptr<Directory> realDirectory = newFSDirectory(path);

  // LUCENE-3627 (before the fix): this line fails because
  // it doesn't know what to do with the created but empty
  // segments_2 file
  shared_ptr<IndexWriter> indexWriter = make_shared<IndexWriter>(
      realDirectory, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));

  // currently the test fails above.
  // however, to test the fix, the following lines should pass as well.
  indexWriter->addDocument(getDocument());
  delete indexWriter;
  assertFalse(slowFileExists(realDirectory, L"segments_2"));
  delete realDirectory;
}

void TestCrashCausesCorruptIndex::searchForFleas(
    int const expectedTotalHits) 
{
  shared_ptr<Directory> realDirectory = newFSDirectory(path);
  shared_ptr<IndexReader> indexReader = DirectoryReader::open(realDirectory);
  shared_ptr<IndexSearcher> indexSearcher = newSearcher(indexReader);
  shared_ptr<TopDocs> topDocs = indexSearcher->search(
      make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"fleas")), 10);
  assertNotNull(topDocs);
  assertEquals(expectedTotalHits, topDocs->totalHits);
  delete indexReader;
  delete realDirectory;
}

const wstring TestCrashCausesCorruptIndex::TEXT_FIELD = L"text";

shared_ptr<Document> TestCrashCausesCorruptIndex::getDocument()
{
  shared_ptr<Document> document = make_shared<Document>();
  document->push_back(
      newTextField(TEXT_FIELD, L"my dog has fleas", Field::Store::NO));
  return document;
}

TestCrashCausesCorruptIndex::CrashingException::CrashingException(
    const wstring &msg)
    : RuntimeException(msg)
{
}

TestCrashCausesCorruptIndex::CrashAfterCreateOutput::CrashAfterCreateOutput(
    shared_ptr<Directory> realDirectory) 
    : org::apache::lucene::store::FilterDirectory(realDirectory)
{
}

void TestCrashCausesCorruptIndex::CrashAfterCreateOutput::
    setCrashAfterCreateOutput(const wstring &name)
{
  this->crashAfterCreateOutput = name;
}

shared_ptr<IndexOutput>
TestCrashCausesCorruptIndex::CrashAfterCreateOutput::createOutput(
    const wstring &name, shared_ptr<IOContext> cxt) 
{
  shared_ptr<IndexOutput> indexOutput = in_->createOutput(name, cxt);
  if (L"" != crashAfterCreateOutput && name == crashAfterCreateOutput) {
    // CRASH!
    delete indexOutput;
    if (VERBOSE) {
      wcout << L"TEST: now crash" << endl;
      // C++ TODO: This exception's constructor requires an argument:
      // ORIGINAL LINE: new Throwable().printStackTrace(System.out);
      (runtime_error())->printStackTrace(System::out);
    }
    throw make_shared<CrashingException>(L"crashAfterCreateOutput " +
                                         crashAfterCreateOutput);
  }
  return indexOutput;
}
} // namespace org::apache::lucene::index