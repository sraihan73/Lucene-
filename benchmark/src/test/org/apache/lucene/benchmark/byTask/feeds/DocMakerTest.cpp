using namespace std;

#include "DocMakerTest.h"
#include "../../../../../../../../../analysis/common/src/java/org/apache/lucene/analysis/core/WhitespaceAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/Term.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/TermQuery.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/TopDocs.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/PerfRunData.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/feeds/DocData.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/feeds/DocMaker.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/feeds/LineDocSource.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/feeds/NoMoreDataException.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/tasks/AddDocTask.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/tasks/CloseIndexTask.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/tasks/CreateIndexTask.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/tasks/TaskSequence.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/utils/Config.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{
using WhitespaceAnalyzer =
    org::apache::lucene::analysis::core::WhitespaceAnalyzer;
using BenchmarkTestCase = org::apache::lucene::benchmark::BenchmarkTestCase;
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using AddDocTask = org::apache::lucene::benchmark::byTask::tasks::AddDocTask;
using CloseIndexTask =
    org::apache::lucene::benchmark::byTask::tasks::CloseIndexTask;
using CreateIndexTask =
    org::apache::lucene::benchmark::byTask::tasks::CreateIndexTask;
using TaskSequence =
    org::apache::lucene::benchmark::byTask::tasks::TaskSequence;
using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using Document = org::apache::lucene::document::Document;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using IOUtils = org::apache::lucene::util::IOUtils;

DocMakerTest::OneDocSource::~OneDocSource() {}

shared_ptr<DocData> DocMakerTest::OneDocSource::getNextDocData(
    shared_ptr<DocData> docData) 
{
  if (finish) {
    throw make_shared<NoMoreDataException>();
  }

  docData->setBody(L"body");
  docData->setDate(L"date");
  docData->setTitle(L"title");
  shared_ptr<Properties> props = make_shared<Properties>();
  props->setProperty(L"key", L"value");
  docData->setProps(props);
  finish = true;

  return docData;
}

void DocMakerTest::doTestIndexProperties(
    bool setIndexProps, bool indexPropsVal,
    int numExpectedResults) 
{
  shared_ptr<Properties> props = make_shared<Properties>();

  // Indexing configuration.
  props->setProperty(L"analyzer", WhitespaceAnalyzer::typeid->getName());
  props->setProperty(L"content.source", OneDocSource::typeid->getName());
  props->setProperty(L"directory", L"RAMDirectory");
  if (setIndexProps) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    props->setProperty(L"doc.index.props", Boolean::toString(indexPropsVal));
  }

  // Create PerfRunData
  shared_ptr<Config> config = make_shared<Config>(props);
  shared_ptr<PerfRunData> runData = make_shared<PerfRunData>(config);

  shared_ptr<TaskSequence> tasks =
      make_shared<TaskSequence>(runData, getTestName(), nullptr, false);
  tasks->addTask(make_shared<CreateIndexTask>(runData));
  tasks->addTask(make_shared<AddDocTask>(runData));
  tasks->addTask(make_shared<CloseIndexTask>(runData));
  tasks->doLogic();

  shared_ptr<IndexReader> reader =
      DirectoryReader::open(runData->getDirectory());
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  shared_ptr<TopDocs> td = searcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"key", L"value")), 10);
  assertEquals(numExpectedResults, td->totalHits);
  delete reader;
}

shared_ptr<Document>
DocMakerTest::createTestNormsDocument(bool setNormsProp, bool normsPropVal,
                                      bool setBodyNormsProp,
                                      bool bodyNormsVal) 
{
  shared_ptr<Properties> props = make_shared<Properties>();

  // Indexing configuration.
  props->setProperty(L"analyzer", WhitespaceAnalyzer::typeid->getName());
  props->setProperty(L"directory", L"RAMDirectory");
  if (setNormsProp) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    props->setProperty(L"doc.tokenized.norms", Boolean::toString(normsPropVal));
  }
  if (setBodyNormsProp) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    props->setProperty(L"doc.body.tokenized.norms",
                       Boolean::toString(bodyNormsVal));
  }

  // Create PerfRunData
  shared_ptr<Config> config = make_shared<Config>(props);

  shared_ptr<DocMaker> dm = make_shared<DocMaker>();
  dm->setConfig(config, make_shared<OneDocSource>());
  return dm->makeDocument();
}

void DocMakerTest::testIndexProperties() 
{
  // default is to not index properties.
  doTestIndexProperties(false, false, 0);

  // set doc.index.props to false.
  doTestIndexProperties(true, false, 0);

  // set doc.index.props to true.
  doTestIndexProperties(true, true, 1);
}

void DocMakerTest::testNorms() 
{

  shared_ptr<Document> doc;

  // Don't set anything, use the defaults
  doc = createTestNormsDocument(false, false, false, false);
  assertTrue(doc->getField(DocMaker::TITLE_FIELD)->fieldType()->omitNorms());
  assertFalse(doc->getField(DocMaker::BODY_FIELD)->fieldType()->omitNorms());

  // Set norms to false
  doc = createTestNormsDocument(true, false, false, false);
  assertTrue(doc->getField(DocMaker::TITLE_FIELD)->fieldType()->omitNorms());
  assertFalse(doc->getField(DocMaker::BODY_FIELD)->fieldType()->omitNorms());

  // Set norms to true
  doc = createTestNormsDocument(true, true, false, false);
  assertFalse(doc->getField(DocMaker::TITLE_FIELD)->fieldType()->omitNorms());
  assertFalse(doc->getField(DocMaker::BODY_FIELD)->fieldType()->omitNorms());

  // Set body norms to false
  doc = createTestNormsDocument(false, false, true, false);
  assertTrue(doc->getField(DocMaker::TITLE_FIELD)->fieldType()->omitNorms());
  assertTrue(doc->getField(DocMaker::BODY_FIELD)->fieldType()->omitNorms());

  // Set body norms to true
  doc = createTestNormsDocument(false, false, true, true);
  assertTrue(doc->getField(DocMaker::TITLE_FIELD)->fieldType()->omitNorms());
  assertFalse(doc->getField(DocMaker::BODY_FIELD)->fieldType()->omitNorms());
}

void DocMakerTest::testDocMakerLeak() 
{
  // DocMaker did not close its ContentSource if resetInputs was called twice,
  // leading to a file handle leak.
  shared_ptr<Path> f = getWorkDir()->resolve(L"docMakerLeak.txt");
  shared_ptr<PrintStream> ps =
      make_shared<PrintStream>(Files::newOutputStream(f), true, IOUtils::UTF_8);
  ps->println(L"one title\t" + System::currentTimeMillis() + L"\tsome content");
  ps->close();

  shared_ptr<Properties> props = make_shared<Properties>();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  props->setProperty(L"docs.file", f->toAbsolutePath()->toString());
  props->setProperty(L"content.source.forever", L"false");
  shared_ptr<Config> config = make_shared<Config>(props);

  shared_ptr<ContentSource> source = make_shared<LineDocSource>();
  source->setConfig(config);

  shared_ptr<DocMaker> dm = make_shared<DocMaker>();
  dm->setConfig(config, source);
  dm->resetInputs();
  dm->resetInputs();
  delete dm;
}
} // namespace org::apache::lucene::benchmark::byTask::feeds