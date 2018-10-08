using namespace std;

#include "LineDocSourceTest.h"
#include "../../../../../../../../../analysis/common/src/java/org/apache/lucene/analysis/core/WhitespaceAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/Term.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/TermQuery.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/TopDocs.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/PerfRunData.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/feeds/DocMaker.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/feeds/LineDocSource.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/tasks/AddDocTask.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/tasks/CloseIndexTask.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/tasks/CreateIndexTask.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/tasks/TaskSequence.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/tasks/WriteLineDocTask.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/utils/Config.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{
using org::apache::commons::compress::compressors::CompressorStreamFactory;
using WhitespaceAnalyzer =
    org::apache::lucene::analysis::core::WhitespaceAnalyzer;
using BenchmarkTestCase = org::apache::lucene::benchmark::BenchmarkTestCase;
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using HeaderLineParser = org::apache::lucene::benchmark::byTask::feeds::
    LineDocSource::HeaderLineParser;
using LineParser =
    org::apache::lucene::benchmark::byTask::feeds::LineDocSource::LineParser;
using AddDocTask = org::apache::lucene::benchmark::byTask::tasks::AddDocTask;
using CloseIndexTask =
    org::apache::lucene::benchmark::byTask::tasks::CloseIndexTask;
using CreateIndexTask =
    org::apache::lucene::benchmark::byTask::tasks::CreateIndexTask;
using TaskSequence =
    org::apache::lucene::benchmark::byTask::tasks::TaskSequence;
using WriteLineDocTask =
    org::apache::lucene::benchmark::byTask::tasks::WriteLineDocTask;
using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using IOUtils = org::apache::lucene::util::IOUtils;
const shared_ptr<
    org::apache::commons::compress::compressors::CompressorStreamFactory>
    LineDocSourceTest::csFactory = make_shared<
        org::apache::commons::compress::compressors::CompressorStreamFactory>();

void LineDocSourceTest::createBZ2LineFile(shared_ptr<Path> file,
                                          bool addHeader) 
{
  shared_ptr<OutputStream> out = Files::newOutputStream(file);
  out = csFactory->createCompressorOutputStream(L"bzip2", out);
  shared_ptr<BufferedWriter> writer = make_shared<BufferedWriter>(
      make_shared<OutputStreamWriter>(out, StandardCharsets::UTF_8));
  writeDocsToFile(writer, addHeader, nullptr);
  writer->close();
}

void LineDocSourceTest::writeDocsToFile(
    shared_ptr<BufferedWriter> writer, bool addHeader,
    shared_ptr<Properties> otherFields) 
{
  if (addHeader) {
    writer->write(WriteLineDocTask::FIELDS_HEADER_INDICATOR);
    writer->write(WriteLineDocTask::SEP);
    writer->write(DocMaker::TITLE_FIELD);
    writer->write(WriteLineDocTask::SEP);
    writer->write(DocMaker::DATE_FIELD);
    writer->write(WriteLineDocTask::SEP);
    writer->write(DocMaker::BODY_FIELD);
    if (otherFields != nullptr) {
      // additional field names in the header
      for (auto fn : otherFields) {
        writer->write(WriteLineDocTask::SEP);
        // C++ TODO: There is no native C++ equivalent to 'toString':
        writer->write(fn.first->toString());
      }
    }
    writer->newLine();
  }
  shared_ptr<StringBuilder> doc = make_shared<StringBuilder>();
  doc->append(L"title")
      ->append(WriteLineDocTask::SEP)
      ->append(L"date")
      ->append(WriteLineDocTask::SEP)
      ->append(DocMaker::BODY_FIELD);
  if (otherFields != nullptr) {
    // additional field values in the doc line
    for (auto fv : otherFields) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      doc->append(WriteLineDocTask::SEP)->append(fv.second->toString());
    }
  }
  writer->write(doc->toString());
  writer->newLine();
}

void LineDocSourceTest::createRegularLineFile(
    shared_ptr<Path> file, bool addHeader) 
{
  shared_ptr<OutputStream> out = Files::newOutputStream(file);
  shared_ptr<BufferedWriter> writer = make_shared<BufferedWriter>(
      make_shared<OutputStreamWriter>(out, StandardCharsets::UTF_8));
  writeDocsToFile(writer, addHeader, nullptr);
  writer->close();
}

void LineDocSourceTest::createRegularLineFileWithMoreFields(
    shared_ptr<Path> file, deque<wstring> &extraFields) 
{
  shared_ptr<OutputStream> out = Files::newOutputStream(file);
  shared_ptr<BufferedWriter> writer = make_shared<BufferedWriter>(
      make_shared<OutputStreamWriter>(out, StandardCharsets::UTF_8));
  shared_ptr<Properties> p = make_shared<Properties>();
  for (wstring f : extraFields) {
    p->setProperty(f, f);
  }
  writeDocsToFile(writer, true, p);
  writer->close();
}

void LineDocSourceTest::doIndexAndSearchTest(
    shared_ptr<Path> file, type_info lineParserClass,
    const wstring &storedField) 
{
  doIndexAndSearchTestWithRepeats(file, lineParserClass, 1,
                                  storedField); // no extra repetitions
  doIndexAndSearchTestWithRepeats(file, lineParserClass, 2,
                                  storedField); // 1 extra repetition
  doIndexAndSearchTestWithRepeats(file, lineParserClass, 4,
                                  storedField); // 3 extra repetitions
}

void LineDocSourceTest::doIndexAndSearchTestWithRepeats(
    shared_ptr<Path> file, type_info lineParserClass, int numAdds,
    const wstring &storedField) 
{

  shared_ptr<IndexReader> reader = nullptr;
  shared_ptr<IndexSearcher> searcher = nullptr;
  shared_ptr<PerfRunData> runData = nullptr;
  try {
    shared_ptr<Properties> props = make_shared<Properties>();

    // LineDocSource specific settings.
    // C++ TODO: There is no native C++ equivalent to 'toString':
    props->setProperty(L"docs.file", file->toAbsolutePath()->toString());
    if (lineParserClass != nullptr) {
      props->setProperty(L"line.parser", lineParserClass.getName());
    }

    // Indexing configuration.
    props->setProperty(L"analyzer", WhitespaceAnalyzer::typeid->getName());
    props->setProperty(L"content.source", LineDocSource::typeid->getName());
    props->setProperty(L"directory", L"RAMDirectory");
    props->setProperty(L"doc.stored", L"true");
    props->setProperty(L"doc.index.props", L"true");

    // Create PerfRunData
    shared_ptr<Config> config = make_shared<Config>(props);
    runData = make_shared<PerfRunData>(config);

    shared_ptr<TaskSequence> tasks =
        make_shared<TaskSequence>(runData, L"testBzip2", nullptr, false);
    tasks->addTask(make_shared<CreateIndexTask>(runData));
    for (int i = 0; i < numAdds; i++) {
      tasks->addTask(make_shared<AddDocTask>(runData));
    }
    tasks->addTask(make_shared<CloseIndexTask>(runData));
    try {
      tasks->doLogic();
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      tasks->close();
    }

    reader = DirectoryReader::open(runData->getDirectory());
    searcher = newSearcher(reader);
    shared_ptr<TopDocs> td = searcher->search(
        make_shared<TermQuery>(make_shared<Term>(L"body", L"body")), 10);
    assertEquals(numAdds, td->totalHits);
    assertNotNull(td->scoreDocs[0]);

    if (storedField == L"") {
      storedField = DocMaker::BODY_FIELD; // added to all docs and satisfies
                                          // field-name == value
    }
    assertEquals(L"Wrong field value", storedField,
                 searcher->doc(0)[storedField]);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::close({reader, runData});
  }
}

void LineDocSourceTest::testBZip2() 
{
  shared_ptr<Path> file = getWorkDir()->resolve(L"one-line.bz2");
  createBZ2LineFile(file, true);
  doIndexAndSearchTest(file, nullptr, L"");
}

void LineDocSourceTest::testBZip2NoHeaderLine() 
{
  shared_ptr<Path> file = getWorkDir()->resolve(L"one-line.bz2");
  createBZ2LineFile(file, false);
  doIndexAndSearchTest(file, nullptr, L"");
}

void LineDocSourceTest::testRegularFile() 
{
  shared_ptr<Path> file = getWorkDir()->resolve(L"one-line");
  createRegularLineFile(file, true);
  doIndexAndSearchTest(file, nullptr, L"");
}

void LineDocSourceTest::testRegularFileSpecialHeader() 
{
  shared_ptr<Path> file = getWorkDir()->resolve(L"one-line");
  createRegularLineFile(file, true);
  doIndexAndSearchTest(file, HeaderLineParser::typeid, L"");
}

void LineDocSourceTest::testRegularFileNoHeaderLine() 
{
  shared_ptr<Path> file = getWorkDir()->resolve(L"one-line");
  createRegularLineFile(file, false);
  doIndexAndSearchTest(file, nullptr, L"");
}

void LineDocSourceTest::testInvalidFormat() 
{
  std::deque<wstring> testCases = {
      L"", L"title", L"title" + StringHelper::toString(WriteLineDocTask::SEP),
      L"title" + StringHelper::toString(WriteLineDocTask::SEP) + L"body"};

  for (int i = 0; i < testCases.size(); i++) {
    shared_ptr<Path> file = getWorkDir()->resolve(L"one-line");
    shared_ptr<BufferedWriter> writer =
        Files::newBufferedWriter(file, StandardCharsets::UTF_8);
    writer->write(testCases[i]);
    writer->newLine();
    writer->close();
    expectThrows(runtime_error::typeid,
                 [&]() { doIndexAndSearchTest(file, nullptr, L""); });
  }
}

void LineDocSourceTest::testWithDocsName() 
{
  shared_ptr<Path> file = getWorkDir()->resolve(L"one-line");
  createRegularLineFileWithMoreFields(file, {DocMaker::NAME_FIELD});
  doIndexAndSearchTest(file, nullptr, DocMaker::NAME_FIELD);
}

void LineDocSourceTest::testWithProperties() 
{
  shared_ptr<Path> file = getWorkDir()->resolve(L"one-line");
  wstring specialField = L"mySpecialField";
  createRegularLineFileWithMoreFields(file, {specialField});
  doIndexAndSearchTest(file, nullptr, specialField);
}
} // namespace org::apache::lucene::benchmark::byTask::feeds