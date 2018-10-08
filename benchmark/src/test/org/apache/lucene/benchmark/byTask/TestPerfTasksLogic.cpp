using namespace std;

#include "TestPerfTasksLogic.h"
#include "../../../../../../../../analysis/common/src/java/org/apache/lucene/collation/CollationKeyAnalyzer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TermToBytesRefAttribute.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/Fields.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriterConfig.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/LogMergePolicy.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/MultiFields.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentInfos.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/Terms.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/TermsEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/DocIdSetIterator.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../facet/src/java/org/apache/lucene/facet/taxonomy/TaxonomyReader.h"
#include "../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockAnalyzer.h"
#include "../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../java/org/apache/lucene/benchmark/byTask/Benchmark.h"
#include "../../../../../../java/org/apache/lucene/benchmark/byTask/PerfRunData.h"
#include "../../../../../../java/org/apache/lucene/benchmark/byTask/feeds/DocMaker.h"
#include "../../../../../../java/org/apache/lucene/benchmark/byTask/stats/TaskStats.h"
#include "../../../../../../java/org/apache/lucene/benchmark/byTask/tasks/WriteLineDocTask.h"
#include "tasks/CountingSearchTestTask.h"

namespace org::apache::lucene::benchmark::byTask
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TermToBytesRefAttribute =
    org::apache::lucene::analysis::tokenattributes::TermToBytesRefAttribute;
using BenchmarkTestCase = org::apache::lucene::benchmark::BenchmarkTestCase;
using DocMaker = org::apache::lucene::benchmark::byTask::feeds::DocMaker;
using TaskStats = org::apache::lucene::benchmark::byTask::stats::TaskStats;
using CountingSearchTestTask =
    org::apache::lucene::benchmark::byTask::tasks::CountingSearchTestTask;
using WriteLineDocTask =
    org::apache::lucene::benchmark::byTask::tasks::WriteLineDocTask;
using CollationKeyAnalyzer =
    org::apache::lucene::collation::CollationKeyAnalyzer;
using TaxonomyReader = org::apache::lucene::facet::taxonomy::TaxonomyReader;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Fields = org::apache::lucene::index::Fields;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LogDocMergePolicy = org::apache::lucene::index::LogDocMergePolicy;
using LogMergePolicy = org::apache::lucene::index::LogMergePolicy;
using MultiFields = org::apache::lucene::index::MultiFields;
using SegmentInfos = org::apache::lucene::index::SegmentInfos;
using SerialMergeScheduler = org::apache::lucene::index::SerialMergeScheduler;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestPerfTasksLogic::setUp() 
{
  BenchmarkTestCase::setUp();
  copyToWorkDir(L"reuters.first20.lines.txt");
  copyToWorkDir(L"test-mapping-ISOLatin1Accent-partial.txt");
}

void TestPerfTasksLogic::testIndexAndSearchTasks() 
{
  // 1. alg definition (required in every "logic" test)
  std::deque<wstring> algLines = {L"ResetSystemErase",
                                   L"CreateIndex",
                                   L"{ AddDoc } : 1000",
                                   L"ForceMerge(1)",
                                   L"CloseIndex",
                                   L"OpenReader",
                                   L"{ CountingSearchTest } : 200",
                                   L"CloseReader",
                                   L"[ CountingSearchTest > : 70",
                                   L"[ CountingSearchTest > : 9"};

  // 2. we test this value later
  CountingSearchTestTask::numSearches = 0;

  // 3. execute the algorithm  (required in every "logic" test)
  shared_ptr<Benchmark> benchmark = execBenchmark(algLines);

  // 4. test specific checks after the benchmark run completed.
  assertEquals(L"TestSearchTask was supposed to be called!", 279,
               CountingSearchTestTask::numSearches);
  assertTrue(
      L"Index does not exist?...!",
      DirectoryReader::indexExists(benchmark->getRunData()->getDirectory()));
  // now we should be able to open the index for write.
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(
      benchmark->getRunData()->getDirectory(),
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()))
          .setOpenMode(OpenMode::APPEND));
  delete iw;
  shared_ptr<IndexReader> ir =
      DirectoryReader::open(benchmark->getRunData()->getDirectory());
  assertEquals(
      L"1000 docs were added to the index, this is what we expect to find!",
      1000, ir->numDocs());
  delete ir;
}

void TestPerfTasksLogic::testTimedSearchTask() 
{
  std::deque<wstring> algLines = {
      L"log.step=100000", L"ResetSystemErase",
      L"CreateIndex",     L"{ AddDoc } : 100",
      L"ForceMerge(1)",   L"CloseIndex",
      L"OpenReader",      L"{ CountingSearchTest } : .5s",
      L"CloseReader"};

  CountingSearchTestTask::numSearches = 0;
  execBenchmark(algLines);
  assertTrue(CountingSearchTestTask::numSearches > 0);
  int64_t elapsed = CountingSearchTestTask::prevLastMillis -
                      CountingSearchTestTask::startMillis;
  assertTrue(L"elapsed time was " + to_wstring(elapsed) + L" msec",
             elapsed <= 1500);
}

void TestPerfTasksLogic::testBGSearchTaskThreads() 
{
  std::deque<wstring> algLines = {
      L"log.time.step.msec = 100",
      L"log.step=100000",
      L"ResetSystemErase",
      L"CreateIndex",
      L"{ AddDoc } : 1000",
      L"ForceMerge(1)",
      L"CloseIndex",
      L"OpenReader",
      L"{",
      L"  [ \"XSearch\" { CountingSearchTest > : * ] : 2 &-1",
      L"  Wait(0.5)",
      L"}",
      L"CloseReader",
      L"RepSumByPref X"};

  CountingSearchTestTask::numSearches = 0;
  execBenchmark(algLines);

  // NOTE: cannot assert this, because on a super-slow
  // system, it could be after waiting 0.5 seconds that
  // the search threads hadn't yet succeeded in starting
  // up and then they start up and do no searching:
  // assertTrue(CountingSearchTestTask.numSearches > 0);
}

void TestPerfTasksLogic::testExhaustContentSource() 
{
  // 1. alg definition (required in every "logic" test)
  std::deque<wstring> algLines = {L"# ----- properties ",
                                   L"content.source=org.apache.lucene."
                                   L"benchmark.byTask.feeds.SingleDocSource",
                                   L"content.source.log.step=1",
                                   L"doc.term.deque=false",
                                   L"content.source.forever=false",
                                   L"directory=RAMDirectory",
                                   L"doc.stored=false",
                                   L"doc.tokenized=false",
                                   L"# ----- alg ",
                                   L"CreateIndex",
                                   L"{ AddDoc } : * ",
                                   L"ForceMerge(1)",
                                   L"CloseIndex",
                                   L"OpenReader",
                                   L"{ CountingSearchTest } : 100",
                                   L"CloseReader",
                                   L"[ CountingSearchTest > : 30",
                                   L"[ CountingSearchTest > : 9"};

  // 2. we test this value later
  CountingSearchTestTask::numSearches = 0;

  // 3. execute the algorithm  (required in every "logic" test)
  shared_ptr<Benchmark> benchmark = execBenchmark(algLines);

  // 4. test specific checks after the benchmark run completed.
  assertEquals(L"TestSearchTask was supposed to be called!", 139,
               CountingSearchTestTask::numSearches);
  assertTrue(
      L"Index does not exist?...!",
      DirectoryReader::indexExists(benchmark->getRunData()->getDirectory()));
  // now we should be able to open the index for write.
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(
      benchmark->getRunData()->getDirectory(),
      (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
          ->setOpenMode(OpenMode::APPEND));
  delete iw;
  shared_ptr<IndexReader> ir =
      DirectoryReader::open(benchmark->getRunData()->getDirectory());
  assertEquals(
      L"1 docs were added to the index, this is what we expect to find!", 1,
      ir->numDocs());
  delete ir;
}

void TestPerfTasksLogic::testDocMakerThreadSafety() 
{
  // 1. alg definition (required in every "logic" test)
  std::deque<wstring> algLines = {
      L"# ----- properties ",
      L"content.source=org.apache.lucene.benchmark.byTask.feeds."
      L"SortableSingleDocSource",
      L"doc.term.deque=false",
      L"log.step.AddDoc=10000",
      L"content.source.forever=true",
      L"directory=RAMDirectory",
      L"doc.reuse.fields=false",
      L"doc.stored=true",
      L"doc.tokenized=false",
      L"doc.index.props=true",
      L"# ----- alg ",
      L"CreateIndex",
      L"[ { AddDoc > : 250 ] : 4",
      L"CloseIndex"};

  // 2. we test this value later
  CountingSearchTestTask::numSearches = 0;

  // 3. execute the algorithm  (required in every "logic" test)
  shared_ptr<Benchmark> benchmark = execBenchmark(algLines);

  shared_ptr<DirectoryReader> r =
      DirectoryReader::open(benchmark->getRunData()->getDirectory());

  constexpr int maxDoc = r->maxDoc();
  TestUtil::assertEquals(1000, maxDoc);
  for (int i = 0; i < 1000; i++) {
    assertNotNull(L"doc " + to_wstring(i) + L" has null country",
                  r->document(i).getField(L"country"));
  }
  r->close();
}

void TestPerfTasksLogic::testParallelDocMaker() 
{
  // 1. alg definition (required in every "logic" test)
  std::deque<wstring> algLines = {
      L"# ----- properties ",
      L"content.source=org.apache.lucene.benchmark.byTask.feeds.LineDocSource",
      L"docs.file=" + getReuters20LinesFile(),
      L"content.source.log.step=3",
      L"doc.term.deque=false",
      L"content.source.forever=false",
      L"directory=FSDirectory",
      L"doc.stored=false",
      L"doc.tokenized=false",
      L"# ----- alg ",
      L"CreateIndex",
      L"[ { AddDoc } : * ] : 4 ",
      L"CloseIndex"};

  // 2. execute the algorithm  (required in every "logic" test)
  shared_ptr<Benchmark> benchmark = execBenchmark(algLines);

  // 3. test number of docs in the index
  shared_ptr<IndexReader> ir =
      DirectoryReader::open(benchmark->getRunData()->getDirectory());
  int ndocsExpected = 20; // first 20 reuters docs.
  assertEquals(L"wrong number of docs in the index!", ndocsExpected,
               ir->numDocs());
  delete ir;
}

void TestPerfTasksLogic::testLineDocFile() 
{
  shared_ptr<Path> lineFile = createTempFile(L"test.reuters.lines", L".txt");

  // We will call WriteLineDocs this many times
  constexpr int NUM_TRY_DOCS = 50;

  // Creates a line file with first 50 docs from SingleDocSource
  // C++ TODO: There is no native C++ equivalent to 'toString':
  std::deque<wstring> algLines1 = {
      L"# ----- properties ",
      L"content.source=org.apache.lucene.benchmark.byTask.feeds."
      L"SingleDocSource",
      L"content.source.forever=true",
      L"line.file.out=" +
          lineFile->toAbsolutePath()->toString()->replace(L'\\', L'/'),
      L"# ----- alg ",
      L"{WriteLineDoc()}:" + to_wstring(NUM_TRY_DOCS)};

  // Run algo
  shared_ptr<Benchmark> benchmark = execBenchmark(algLines1);

  shared_ptr<BufferedReader> r =
      Files::newBufferedReader(lineFile, StandardCharsets::UTF_8);
  int numLines = 0;
  wstring line;
  while ((line = r->readLine()) != L"") {
    if (numLines == 0 && StringHelper::startsWith(
                             line, WriteLineDocTask::FIELDS_HEADER_INDICATOR)) {
      continue; // do not count the header line as a doc
    }
    numLines++;
  }
  r->close();
  assertEquals(L"did not see the right number of docs; should be " +
                   to_wstring(NUM_TRY_DOCS) + L" but was " +
                   to_wstring(numLines),
               NUM_TRY_DOCS, numLines);

  // Index the line docs
  // C++ TODO: There is no native C++ equivalent to 'toString':
  std::deque<wstring> algLines2 = {
      L"# ----- properties ",
      L"analyzer=org.apache.lucene.analysis.core.WhitespaceAnalyzer",
      L"content.source=org.apache.lucene.benchmark.byTask.feeds.LineDocSource",
      L"docs.file=" +
          lineFile->toAbsolutePath()->toString()->replace(L'\\', L'/'),
      L"content.source.forever=false",
      L"doc.reuse.fields=false",
      L"ram.flush.mb=4",
      L"# ----- alg ",
      L"ResetSystemErase",
      L"CreateIndex",
      L"{AddDoc}: *",
      L"CloseIndex"};

  // Run algo
  benchmark = execBenchmark(algLines2);

  // now we should be able to open the index for write.
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(
      benchmark->getRunData()->getDirectory(),
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()))
          .setOpenMode(OpenMode::APPEND));
  delete iw;

  shared_ptr<IndexReader> ir =
      DirectoryReader::open(benchmark->getRunData()->getDirectory());
  assertEquals(to_wstring(numLines) + L" lines were created but " +
                   to_wstring(ir->numDocs()) + L" docs are in the index",
               numLines, ir->numDocs());
  delete ir;
}

void TestPerfTasksLogic::testReadTokens() 
{

  // We will call ReadTokens on this many docs
  constexpr int NUM_DOCS = 20;

  // Read tokens from first NUM_DOCS docs from Reuters and
  // then build index from the same docs
  std::deque<wstring> algLines1 = {
      L"# ----- properties ",
      L"analyzer=org.apache.lucene.analysis.core.WhitespaceAnalyzer",
      L"content.source=org.apache.lucene.benchmark.byTask.feeds.LineDocSource",
      L"docs.file=" + getReuters20LinesFile(),
      L"# ----- alg ",
      L"{ReadTokens}: " + to_wstring(NUM_DOCS),
      L"ResetSystemErase",
      L"CreateIndex",
      L"{AddDoc}: " + to_wstring(NUM_DOCS),
      L"CloseIndex"};

  // Run algo
  shared_ptr<Benchmark> benchmark = execBenchmark(algLines1);

  deque<std::shared_ptr<TaskStats>> stats =
      benchmark->getRunData()->getPoints()->taskStats();

  // Count how many tokens all ReadTokens saw
  int totalTokenCount1 = 0;
  for (auto stat : stats) {
    if (stat->getTask()->getName() == L"ReadTokens") {
      totalTokenCount1 += stat->getCount();
    }
  }

  // Separately count how many tokens are actually in the index:
  shared_ptr<IndexReader> reader =
      DirectoryReader::open(benchmark->getRunData()->getDirectory());
  TestUtil::assertEquals(NUM_DOCS, reader->numDocs());

  int totalTokenCount2 = 0;

  shared_ptr<Fields> fields = MultiFields::getFields(reader);

  for (auto fieldName : fields) {
    if (fieldName.equals(DocMaker::ID_FIELD) ||
        fieldName.equals(DocMaker::DATE_MSEC_FIELD) ||
        fieldName.equals(DocMaker::TIME_SEC_FIELD)) {
      continue;
    }
    shared_ptr<Terms> terms = fields->terms(fieldName);
    if (terms == nullptr) {
      continue;
    }
    shared_ptr<TermsEnum> termsEnum = terms->begin();
    shared_ptr<PostingsEnum> docs = nullptr;
    while (termsEnum->next() != nullptr) {
      docs = TestUtil::docs(random(), termsEnum, docs, PostingsEnum::FREQS);
      while (docs->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
        totalTokenCount2 += docs->freq();
      }
    }
  }
  delete reader;

  // Make sure they are the same
  TestUtil::assertEquals(totalTokenCount1, totalTokenCount2);
}

void TestPerfTasksLogic::testParallelExhausted() 
{
  // 1. alg definition (required in every "logic" test)
  std::deque<wstring> algLines = {
      L"# ----- properties ",
      L"content.source=org.apache.lucene.benchmark.byTask.feeds.LineDocSource",
      L"docs.file=" + getReuters20LinesFile(),
      L"content.source.log.step=3",
      L"doc.term.deque=false",
      L"content.source.forever=false",
      L"directory=RAMDirectory",
      L"doc.stored=false",
      L"doc.tokenized=false",
      L"task.max.depth.log=1",
      L"# ----- alg ",
      L"CreateIndex",
      L"{ [ AddDoc]: 4} : * ",
      L"ResetInputs ",
      L"{ [ AddDoc]: 4} : * ",
      L"CloseIndex"};

  // 2. execute the algorithm  (required in every "logic" test)
  shared_ptr<Benchmark> benchmark = execBenchmark(algLines);

  // 3. test number of docs in the index
  shared_ptr<IndexReader> ir =
      DirectoryReader::open(benchmark->getRunData()->getDirectory());
  int ndocsExpected = 2 * 20; // first 20 reuters docs.
  assertEquals(L"wrong number of docs in the index!", ndocsExpected,
               ir->numDocs());
  delete ir;
}

void TestPerfTasksLogic::testExhaustedLooped() 
{
  // 1. alg definition (required in every "logic" test)
  std::deque<wstring> algLines = {
      L"# ----- properties ",
      L"content.source=org.apache.lucene.benchmark.byTask.feeds.LineDocSource",
      L"docs.file=" + getReuters20LinesFile(),
      L"content.source.log.step=3",
      L"doc.term.deque=false",
      L"content.source.forever=false",
      L"directory=RAMDirectory",
      L"doc.stored=false",
      L"doc.tokenized=false",
      L"task.max.depth.log=1",
      L"# ----- alg ",
      L"{ \"Rounds\"",
      L"  ResetSystemErase",
      L"  CreateIndex",
      L"  { \"AddDocs\"  AddDoc > : * ",
      L"  CloseIndex",
      L"} : 2"};

  // 2. execute the algorithm  (required in every "logic" test)
  shared_ptr<Benchmark> benchmark = execBenchmark(algLines);

  // 3. test number of docs in the index
  shared_ptr<IndexReader> ir =
      DirectoryReader::open(benchmark->getRunData()->getDirectory());
  int ndocsExpected = 20; // first 20 reuters docs.
  assertEquals(L"wrong number of docs in the index!", ndocsExpected,
               ir->numDocs());
  delete ir;
}

void TestPerfTasksLogic::testCloseIndexFalse() 
{
  // 1. alg definition (required in every "logic" test)
  std::deque<wstring> algLines = {
      L"# ----- properties ",
      L"content.source=org.apache.lucene.benchmark.byTask.feeds.LineDocSource",
      L"docs.file=" + getReuters20LinesFile(),
      L"ram.flush.mb=-1",
      L"max.buffered=2",
      L"content.source.log.step=3",
      L"doc.term.deque=false",
      L"content.source.forever=false",
      L"directory=RAMDirectory",
      L"doc.stored=false",
      L"doc.tokenized=false",
      L"debug.level=1",
      L"# ----- alg ",
      L"{ \"Rounds\"",
      L"  ResetSystemErase",
      L"  CreateIndex",
      L"  { \"AddDocs\"  AddDoc > : * ",
      L"  CloseIndex(false)",
      L"} : 2"};

  // 2. execute the algorithm  (required in every "logic" test)
  shared_ptr<Benchmark> benchmark = execBenchmark(algLines);

  // 3. test number of docs in the index
  shared_ptr<IndexReader> ir =
      DirectoryReader::open(benchmark->getRunData()->getDirectory());
  int ndocsExpected = 20; // first 20 reuters docs.
  assertEquals(L"wrong number of docs in the index!", ndocsExpected,
               ir->numDocs());
  delete ir;
}

TestPerfTasksLogic::MyMergeScheduler::MyMergeScheduler()
    : org::apache::lucene::index::SerialMergeScheduler()
{
  called = true;
}

void TestPerfTasksLogic::testMergeScheduler() 
{
  // 1. alg definition (required in every "logic" test)
  std::deque<wstring> algLines = {
      L"# ----- properties ",
      L"content.source=org.apache.lucene.benchmark.byTask.feeds.LineDocSource",
      L"docs.file=" + getReuters20LinesFile(),
      L"content.source.log.step=3",
      L"doc.term.deque=false",
      L"content.source.forever=false",
      L"directory=RAMDirectory",
      L"merge.scheduler=" + MyMergeScheduler::typeid->getName(),
      L"doc.stored=false",
      L"doc.tokenized=false",
      L"debug.level=1",
      L"# ----- alg ",
      L"{ \"Rounds\"",
      L"  ResetSystemErase",
      L"  CreateIndex",
      L"  { \"AddDocs\"  AddDoc > : * ",
      L"} : 2"};
  // 2. execute the algorithm  (required in every "logic" test)
  shared_ptr<Benchmark> benchmark = execBenchmark(algLines);

  assertTrue(
      L"did not use the specified MergeScheduler",
      (std::static_pointer_cast<MyMergeScheduler>(benchmark->getRunData()
                                                      ->getIndexWriter()
                                                      ->getConfig()
                                                      ->getMergeScheduler()))
          ->called);
  delete benchmark->getRunData()->getIndexWriter();

  // 3. test number of docs in the index
  shared_ptr<IndexReader> ir =
      DirectoryReader::open(benchmark->getRunData()->getDirectory());
  int ndocsExpected = 20; // first 20 reuters docs.
  assertEquals(L"wrong number of docs in the index!", ndocsExpected,
               ir->numDocs());
  delete ir;
}

TestPerfTasksLogic::MyMergePolicy::MyMergePolicy() { called = true; }

void TestPerfTasksLogic::testMergePolicy() 
{
  // 1. alg definition (required in every "logic" test)
  std::deque<wstring> algLines = {
      L"# ----- properties ",
      L"content.source=org.apache.lucene.benchmark.byTask.feeds.LineDocSource",
      L"docs.file=" + getReuters20LinesFile(),
      L"content.source.log.step=3",
      L"ram.flush.mb=-1",
      L"max.buffered=2",
      L"doc.term.deque=false",
      L"content.source.forever=false",
      L"directory=RAMDirectory",
      L"merge.policy=" + MyMergePolicy::typeid->getName(),
      L"doc.stored=false",
      L"doc.tokenized=false",
      L"debug.level=1",
      L"# ----- alg ",
      L"{ \"Rounds\"",
      L"  ResetSystemErase",
      L"  CreateIndex",
      L"  { \"AddDocs\"  AddDoc > : * ",
      L"} : 2"};

  // 2. execute the algorithm  (required in every "logic" test)
  shared_ptr<Benchmark> benchmark = execBenchmark(algLines);
  assertTrue(L"did not use the specified MergePolicy",
             (std::static_pointer_cast<MyMergePolicy>(benchmark->getRunData()
                                                          ->getIndexWriter()
                                                          ->getConfig()
                                                          ->getMergePolicy()))
                 ->called);
  delete benchmark->getRunData()->getIndexWriter();

  // 3. test number of docs in the index
  shared_ptr<IndexReader> ir =
      DirectoryReader::open(benchmark->getRunData()->getDirectory());
  int ndocsExpected = 20; // first 20 reuters docs.
  assertEquals(L"wrong number of docs in the index!", ndocsExpected,
               ir->numDocs());
  delete ir;
}

void TestPerfTasksLogic::testIndexWriterSettings() 
{
  // 1. alg definition (required in every "logic" test)
  std::deque<wstring> algLines = {
      L"# ----- properties ",
      L"content.source=org.apache.lucene.benchmark.byTask.feeds.LineDocSource",
      L"docs.file=" + getReuters20LinesFile(),
      L"content.source.log.step=3",
      L"ram.flush.mb=-1",
      L"max.buffered=2",
      L"compound=cmpnd:true:false",
      L"doc.term.deque=deque:false:true",
      L"content.source.forever=false",
      L"directory=RAMDirectory",
      L"doc.stored=false",
      L"merge.factor=3",
      L"doc.tokenized=false",
      L"debug.level=1",
      L"# ----- alg ",
      L"{ \"Rounds\"",
      L"  ResetSystemErase",
      L"  CreateIndex",
      L"  { \"AddDocs\"  AddDoc > : * ",
      L"  NewRound",
      L"} : 2"};

  // 2. execute the algorithm  (required in every "logic" test)
  shared_ptr<Benchmark> benchmark = execBenchmark(algLines);
  shared_ptr<IndexWriter> *const writer =
      benchmark->getRunData()->getIndexWriter();
  TestUtil::assertEquals(2, writer->getConfig()->getMaxBufferedDocs());
  TestUtil::assertEquals(
      IndexWriterConfig::DISABLE_AUTO_FLUSH,
      static_cast<int>(writer->getConfig()->getRAMBufferSizeMB()));
  TestUtil::assertEquals(3, (std::static_pointer_cast<LogMergePolicy>(
                                 writer->getConfig()->getMergePolicy()))
                                ->getMergeFactor());
  assertEquals(0.0, writer->getConfig()->getMergePolicy()->getNoCFSRatio(),
               0.0);
  delete writer;
  shared_ptr<Directory> dir = benchmark->getRunData()->getDirectory();
  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  shared_ptr<Fields> tfv = reader->getTermVectors(0);
  assertNotNull(tfv);
  assertTrue(tfv->size() > 0);
  delete reader;
}

void TestPerfTasksLogic::testIndexingWithFacets() 
{
  // 1. alg definition (required in every "logic" test)
  std::deque<wstring> algLines = {
      L"# ----- properties ",
      L"content.source=org.apache.lucene.benchmark.byTask.feeds.LineDocSource",
      L"docs.file=" + getReuters20LinesFile(),
      L"content.source.log.step=100",
      L"content.source.forever=false",
      L"directory=RAMDirectory",
      L"doc.stored=false",
      L"merge.factor=3",
      L"doc.tokenized=false",
      L"debug.level=1",
      L"# ----- alg ",
      L"ResetSystemErase",
      L"CreateIndex",
      L"CreateTaxonomyIndex",
      L"{ \"AddDocs\"  AddFacetedDoc > : * ",
      L"CloseIndex",
      L"CloseTaxonomyIndex",
      L"OpenTaxonomyReader"};

  // 2. execute the algorithm  (required in every "logic" test)
  shared_ptr<Benchmark> benchmark = execBenchmark(algLines);
  shared_ptr<PerfRunData> runData = benchmark->getRunData();
  assertNull(L"taxo writer was not properly closed",
             runData->getTaxonomyWriter());
  shared_ptr<TaxonomyReader> taxoReader = runData->getTaxonomyReader();
  assertNotNull(L"taxo reader was not opened", taxoReader);
  assertTrue(L"nothing was added to the taxnomy (expecting root and at least "
             L"one addtional category)",
             taxoReader->getSize() > 1);
  delete taxoReader;
}

void TestPerfTasksLogic::testForceMerge() 
{
  // 1. alg definition (required in every "logic" test)
  std::deque<wstring> algLines = {
      L"# ----- properties ",
      L"content.source=org.apache.lucene.benchmark.byTask.feeds.LineDocSource",
      L"docs.file=" + getReuters20LinesFile(),
      L"content.source.log.step=3",
      L"ram.flush.mb=-1",
      L"max.buffered=3",
      L"doc.term.deque=false",
      L"content.source.forever=false",
      L"directory=RAMDirectory",
      L"merge.policy=org.apache.lucene.index.LogDocMergePolicy",
      L"doc.stored=false",
      L"doc.tokenized=false",
      L"debug.level=1",
      L"# ----- alg ",
      L"{ \"Rounds\"",
      L"  ResetSystemErase",
      L"  CreateIndex",
      L"  { \"AddDocs\"  AddDoc > : * ",
      L"  ForceMerge(3)",
      L"  CloseIndex()",
      L"} : 2"};

  // 2. execute the algorithm  (required in every "logic" test)
  shared_ptr<Benchmark> benchmark = execBenchmark(algLines);

  // 3. test number of docs in the index
  shared_ptr<IndexReader> ir =
      DirectoryReader::open(benchmark->getRunData()->getDirectory());
  int ndocsExpected = 20; // first 20 reuters docs.
  assertEquals(L"wrong number of docs in the index!", ndocsExpected,
               ir->numDocs());
  delete ir;

  // Make sure we have 3 segments:
  shared_ptr<SegmentInfos> infos =
      SegmentInfos::readLatestCommit(benchmark->getRunData()->getDirectory());
  TestUtil::assertEquals(3, infos->size());
}

void TestPerfTasksLogic::testDisableCounting() 
{
  doTestDisableCounting(true);
  doTestDisableCounting(false);
}

void TestPerfTasksLogic::doTestDisableCounting(bool disable) throw(
    runtime_error)
{
  // 1. alg definition (required in every "logic" test)
  std::deque<wstring> algLines = disableCountingLines(disable);

  // 2. execute the algorithm  (required in every "logic" test)
  shared_ptr<Benchmark> benchmark = execBenchmark(algLines);

  // 3. test counters
  int n = disable ? 0 : 1;
  int nChecked = 0;
  for (auto stats : benchmark->getRunData()->getPoints()->taskStats()) {
    wstring taskName = stats->getTask()->getName();
    if (taskName == L"Rounds") {
      assertEquals(L"Wrong total count!", 20 + 2 * n, stats->getCount());
      nChecked++;
    } else if (taskName == L"CreateIndex") {
      assertEquals(L"Wrong count for CreateIndex!", n, stats->getCount());
      nChecked++;
    } else if (taskName == L"CloseIndex") {
      assertEquals(L"Wrong count for CloseIndex!", n, stats->getCount());
      nChecked++;
    }
  }
  assertEquals(L"Missing some tasks to check!", 3, nChecked);
}

std::deque<wstring> TestPerfTasksLogic::disableCountingLines(bool disable)
{
  wstring dis = disable ? L"-" : L"";
  return std::deque<wstring>{
      L"# ----- properties ",
      L"content.source=org.apache.lucene.benchmark.byTask.feeds.LineDocSource",
      L"docs.file=" + getReuters20LinesFile(),
      L"content.source.log.step=30",
      L"doc.term.deque=false",
      L"content.source.forever=false",
      L"directory=RAMDirectory",
      L"doc.stored=false",
      L"doc.tokenized=false",
      L"task.max.depth.log=1",
      L"# ----- alg ",
      L"{ \"Rounds\"",
      L"  ResetSystemErase",
      L"  " + dis + L"CreateIndex",
      L"  { \"AddDocs\"  AddDoc > : * ",
      L"  " + dis + L"  CloseIndex",
      L"}",
      L"RepSumByName"};
}

void TestPerfTasksLogic::testLocale() 
{
  // empty Locale: clear it (null)
  shared_ptr<Benchmark> benchmark = execBenchmark(getLocaleConfig(L""));
  assertNull(benchmark->getRunData()->getLocale());

  // ROOT locale
  benchmark = execBenchmark(getLocaleConfig(L"ROOT"));
  TestUtil::assertEquals(make_shared<Locale>(L""),
                         benchmark->getRunData()->getLocale());

  // specify just a language
  benchmark = execBenchmark(getLocaleConfig(L"de"));
  TestUtil::assertEquals(make_shared<Locale>(L"de"),
                         benchmark->getRunData()->getLocale());

  // specify language + country
  benchmark = execBenchmark(getLocaleConfig(L"en,US"));
  TestUtil::assertEquals(make_shared<Locale>(L"en", L"US"),
                         benchmark->getRunData()->getLocale());

  // specify language + country + variant
  benchmark = execBenchmark(getLocaleConfig(L"no,NO,NY"));
  TestUtil::assertEquals(make_shared<Locale>(L"no", L"NO", L"NY"),
                         benchmark->getRunData()->getLocale());
}

std::deque<wstring>
TestPerfTasksLogic::getLocaleConfig(const wstring &localeParam)
{
  std::deque<wstring> algLines = {
      L"# ----- properties ",
      L"content.source=org.apache.lucene.benchmark.byTask.feeds.LineDocSource",
      L"docs.file=" + getReuters20LinesFile(),
      L"content.source.log.step=3",
      L"content.source.forever=false",
      L"directory=RAMDirectory",
      L"# ----- alg ",
      L"{ \"Rounds\"",
      L"  ResetSystemErase",
      L"  NewLocale(" + localeParam + L")",
      L"  CreateIndex",
      L"  { \"AddDocs\"  AddDoc > : * ",
      L"  NewRound",
      L"} : 1"};
  return algLines;
}

void TestPerfTasksLogic::testCollator() 
{
  // ROOT locale
  shared_ptr<Benchmark> benchmark =
      execBenchmark(getCollatorConfig(L"ROOT", L"impl:jdk"));
  shared_ptr<CollationKeyAnalyzer> expected = make_shared<CollationKeyAnalyzer>(
      Collator::getInstance(make_shared<Locale>(L"")));
  assertEqualCollation(expected, benchmark->getRunData()->getAnalyzer(),
                       L"foobar");

  // specify just a language
  benchmark = execBenchmark(getCollatorConfig(L"de", L"impl:jdk"));
  expected = make_shared<CollationKeyAnalyzer>(
      Collator::getInstance(make_shared<Locale>(L"de")));
  assertEqualCollation(expected, benchmark->getRunData()->getAnalyzer(),
                       L"foobar");

  // specify language + country
  benchmark = execBenchmark(getCollatorConfig(L"en,US", L"impl:jdk"));
  expected = make_shared<CollationKeyAnalyzer>(
      Collator::getInstance(make_shared<Locale>(L"en", L"US")));
  assertEqualCollation(expected, benchmark->getRunData()->getAnalyzer(),
                       L"foobar");

  // specify language + country + variant
  benchmark = execBenchmark(getCollatorConfig(L"no,NO,NY", L"impl:jdk"));
  expected = make_shared<CollationKeyAnalyzer>(
      Collator::getInstance(make_shared<Locale>(L"no", L"NO", L"NY")));
  assertEqualCollation(expected, benchmark->getRunData()->getAnalyzer(),
                       L"foobar");
}

void TestPerfTasksLogic::assertEqualCollation(
    shared_ptr<Analyzer> a1, shared_ptr<Analyzer> a2,
    const wstring &text) 
{
  shared_ptr<TokenStream> ts1 = a1->tokenStream(L"bogus", text);
  shared_ptr<TokenStream> ts2 = a2->tokenStream(L"bogus", text);
  ts1->reset();
  ts2->reset();
  shared_ptr<TermToBytesRefAttribute> termAtt1 =
      ts1->addAttribute(TermToBytesRefAttribute::typeid);
  shared_ptr<TermToBytesRefAttribute> termAtt2 =
      ts2->addAttribute(TermToBytesRefAttribute::typeid);
  assertTrue(ts1->incrementToken());
  assertTrue(ts2->incrementToken());
  shared_ptr<BytesRef> bytes1 = termAtt1->getBytesRef();
  shared_ptr<BytesRef> bytes2 = termAtt2->getBytesRef();
  TestUtil::assertEquals(bytes1, bytes2);
  assertFalse(ts1->incrementToken());
  assertFalse(ts2->incrementToken());
  delete ts1;
  delete ts2;
}

std::deque<wstring>
TestPerfTasksLogic::getCollatorConfig(const wstring &localeParam,
                                      const wstring &collationParam)
{
  std::deque<wstring> algLines = {
      L"# ----- properties ",
      L"content.source=org.apache.lucene.benchmark.byTask.feeds.LineDocSource",
      L"docs.file=" + getReuters20LinesFile(),
      L"content.source.log.step=3",
      L"content.source.forever=false",
      L"directory=RAMDirectory",
      L"# ----- alg ",
      L"{ \"Rounds\"",
      L"  ResetSystemErase",
      L"  NewLocale(" + localeParam + L")",
      L"  NewCollationAnalyzer(" + collationParam + L")",
      L"  CreateIndex",
      L"  { \"AddDocs\"  AddDoc > : * ",
      L"  NewRound",
      L"} : 1"};
  return algLines;
}

void TestPerfTasksLogic::testShingleAnalyzer() 
{
  wstring text = L"one,two,three, four five six";

  // StandardTokenizer, maxShingleSize, and outputUnigrams
  shared_ptr<Benchmark> benchmark = execBenchmark(getAnalyzerFactoryConfig(
      L"shingle-analyzer", L"StandardTokenizer,ShingleFilter"));
  delete benchmark->getRunData()->getAnalyzer()->tokenStream(L"bogus", text);
  BaseTokenStreamTestCase::assertAnalyzesTo(
      benchmark->getRunData()->getAnalyzer(), text,
      std::deque<wstring>{L"one", L"one two", L"two", L"two three", L"three",
                           L"three four", L"four", L"four five", L"five",
                           L"five six", L"six"});
  // StandardTokenizer, maxShingleSize = 3, and outputUnigrams = false
  benchmark = execBenchmark(getAnalyzerFactoryConfig(
      L"shingle-analyzer", L"StandardTokenizer,ShingleFilter(maxShingleSize:3,"
                           L"outputUnigrams:false)"));
  BaseTokenStreamTestCase::assertAnalyzesTo(
      benchmark->getRunData()->getAnalyzer(), text,
      std::deque<wstring>{L"one two", L"one two three", L"two three",
                           L"two three four", L"three four", L"three four five",
                           L"four five", L"four five six", L"five six"});
  // WhitespaceTokenizer, default maxShingleSize and outputUnigrams
  benchmark = execBenchmark(getAnalyzerFactoryConfig(
      L"shingle-analyzer", L"WhitespaceTokenizer,ShingleFilter"));
  BaseTokenStreamTestCase::assertAnalyzesTo(
      benchmark->getRunData()->getAnalyzer(), text,
      std::deque<wstring>{L"one,two,three,", L"one,two,three, four", L"four",
                           L"four five", L"five", L"five six", L"six"});

  // WhitespaceTokenizer, maxShingleSize=3 and outputUnigrams=false
  benchmark = execBenchmark(getAnalyzerFactoryConfig(
      L"shingle-factory", L"WhitespaceTokenizer,ShingleFilter(outputUnigrams:"
                          L"false,maxShingleSize:3)"));
  BaseTokenStreamTestCase::assertAnalyzesTo(
      benchmark->getRunData()->getAnalyzer(), text,
      std::deque<wstring>{L"one,two,three, four", L"one,two,three, four five",
                           L"four five", L"four five six", L"five six"});
}

std::deque<wstring>
TestPerfTasksLogic::getAnalyzerFactoryConfig(const wstring &name,
                                             const wstring &params)
{
  const wstring singleQuoteEscapedName = name.replaceAll(L"'", L"\\\\'");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  std::deque<wstring> algLines = {
      L"content.source=org.apache.lucene.benchmark.byTask.feeds.LineDocSource",
      L"docs.file=" + getReuters20LinesFile(),
      L"work.dir=" +
          getWorkDir()->toAbsolutePath()->toString()->replaceAll(L"\\\\", L"/"),
      L"content.source.forever=false",
      L"directory=RAMDirectory",
      L"AnalyzerFactory(name:'" + singleQuoteEscapedName + L"', " + params +
          L")",
      L"NewAnalyzer('" + singleQuoteEscapedName + L"')",
      L"CreateIndex",
      L"{ \"AddDocs\"  AddDoc > : * "};
  return algLines;
}

void TestPerfTasksLogic::testAnalyzerFactory() 
{
  wstring text = L"Fortieth, Quarantième, Cuadragésimo";
  shared_ptr<Benchmark> benchmark = execBenchmark(getAnalyzerFactoryConfig(
      L"ascii folded, pattern replaced, standard tokenized, downcased, "
      L"bigrammed.'analyzer'",
      wstring(L"positionIncrementGap:100,offsetGap:1111,") +
          L"MappingCharFilter(mapping:'test-mapping-ISOLatin1Accent-partial."
          L"txt')," +
          L"PatternReplaceCharFilterFactory(pattern:'e(\\\\\\\\S*)m',"
          L"replacement:\"$1xxx$1\")," +
          L"StandardTokenizer,LowerCaseFilter,NGramTokenFilter(minGramSize:2,"
          L"maxGramSize:2)"));
  BaseTokenStreamTestCase::assertAnalyzesTo(
      benchmark->getRunData()->getAnalyzer(), text,
      std::deque<wstring>{L"fo", L"or", L"rt", L"ti", L"ie", L"et", L"th",
                           L"qu", L"ua", L"ar", L"ra", L"an", L"nt", L"ti",
                           L"ix", L"xx", L"xx", L"xe", L"cu", L"ua", L"ad",
                           L"dr", L"ra", L"ag", L"gs", L"si", L"ix", L"xx",
                           L"xx", L"xs", L"si", L"io"});
}

wstring TestPerfTasksLogic::getReuters20LinesFile()
{
  return getWorkDirResourcePath(L"reuters.first20.lines.txt");
}
} // namespace org::apache::lucene::benchmark::byTask