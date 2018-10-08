using namespace std;

#include "TestPerfTasksParse.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/Query.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/RAMDirectory.h"
#include "../../../../../../java/org/apache/lucene/benchmark/byTask/Benchmark.h"
#include "../../../../../../java/org/apache/lucene/benchmark/byTask/PerfRunData.h"
#include "../../../../../../java/org/apache/lucene/benchmark/byTask/feeds/DocData.h"
#include "../../../../../../java/org/apache/lucene/benchmark/byTask/feeds/NoMoreDataException.h"
#include "../../../../../../java/org/apache/lucene/benchmark/byTask/tasks/PerfTask.h"
#include "../../../../../../java/org/apache/lucene/benchmark/byTask/tasks/TaskSequence.h"
#include "../../../../../../java/org/apache/lucene/benchmark/byTask/utils/Algorithm.h"
#include "../../../../../../java/org/apache/lucene/benchmark/byTask/utils/Config.h"

namespace org::apache::lucene::benchmark::byTask
{
using AbstractQueryMaker =
    org::apache::lucene::benchmark::byTask::feeds::AbstractQueryMaker;
using ContentSource =
    org::apache::lucene::benchmark::byTask::feeds::ContentSource;
using DocData = org::apache::lucene::benchmark::byTask::feeds::DocData;
using NoMoreDataException =
    org::apache::lucene::benchmark::byTask::feeds::NoMoreDataException;
using PerfTask = org::apache::lucene::benchmark::byTask::tasks::PerfTask;
using TaskSequence =
    org::apache::lucene::benchmark::byTask::tasks::TaskSequence;
using Algorithm = org::apache::lucene::benchmark::byTask::utils::Algorithm;
using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using Query = org::apache::lucene::search::Query;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::apache::lucene::util::LuceneTestCase::SuppressSysoutChecks;
const wstring TestPerfTasksParse::NEW_LINE =
    System::getProperty(L"line.separator");
const wstring TestPerfTasksParse::INDENT = L"  ";
const wstring TestPerfTasksParse::propPart =
    INDENT + L"directory=RAMDirectory" + NEW_LINE + INDENT +
    L"print.props=false" + NEW_LINE;

void TestPerfTasksParse::testParseParallelTaskSequenceRepetition() throw(
    runtime_error)
{
  wstring taskStr = L"AddDoc";
  wstring parsedTasks = L"[ " + taskStr + L" ] : 1000";
  shared_ptr<Benchmark> benchmark =
      make_shared<Benchmark>(make_shared<StringReader>(propPart + parsedTasks));
  shared_ptr<Algorithm> alg = benchmark->getAlgorithm();
  deque<std::shared_ptr<PerfTask>> algTasks = alg->extractTasks();
  bool foundAdd = false;
  for (auto task : algTasks) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    if (task->toString()->find(taskStr) != wstring::npos) {
      foundAdd = true;
    }
    if (std::dynamic_pointer_cast<TaskSequence>(task) != nullptr) {
      assertEquals(
          L"repetions should be 1000 for " + parsedTasks, 1000,
          (std::static_pointer_cast<TaskSequence>(task))->getRepetitions());
      assertTrue(L"sequence for " + parsedTasks + L" should be parallel!",
                 (std::static_pointer_cast<TaskSequence>(task))->isParallel());
    }
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertTrue(L"Task " + taskStr + L" was not found in " + alg->toString(),
               foundAdd);
  }
}

void TestPerfTasksParse::testParseTaskSequenceRepetition() 
{
  wstring taskStr = L"AddDoc";
  wstring parsedTasks = L"{ " + taskStr + L" } : 1000";
  shared_ptr<Benchmark> benchmark =
      make_shared<Benchmark>(make_shared<StringReader>(propPart + parsedTasks));
  shared_ptr<Algorithm> alg = benchmark->getAlgorithm();
  deque<std::shared_ptr<PerfTask>> algTasks = alg->extractTasks();
  bool foundAdd = false;
  for (auto task : algTasks) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    if (task->toString()->find(taskStr) != wstring::npos) {
      foundAdd = true;
    }
    if (std::dynamic_pointer_cast<TaskSequence>(task) != nullptr) {
      assertEquals(
          L"repetions should be 1000 for " + parsedTasks, 1000,
          (std::static_pointer_cast<TaskSequence>(task))->getRepetitions());
      assertFalse(L"sequence for " + parsedTasks + L" should be sequential!",
                  (std::static_pointer_cast<TaskSequence>(task))->isParallel());
    }
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertTrue(L"Task " + taskStr + L" was not found in " + alg->toString(),
               foundAdd);
  }
}

shared_ptr<DocData> TestPerfTasksParse::MockContentSource::getNextDocData(
    shared_ptr<DocData> docData) 
{
  return docData;
}

TestPerfTasksParse::MockContentSource::~MockContentSource() {}

std::deque<std::shared_ptr<Query>>
TestPerfTasksParse::MockQueryMaker::prepareQueries() 
{
  return std::deque<std::shared_ptr<Query>>(0);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("try") public void testParseExamples()
// throws Exception
void TestPerfTasksParse::testParseExamples() 
{
  // hackedy-hack-hack
  bool foundFiles = false;
  shared_ptr<Path> *const examplesDir =
      Paths->get(getClass().getResource(L"/conf").toURI());
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try
  // (java.nio.file.DirectoryStream<java.nio.file.Path> stream =
  // java.nio.file.Files.newDirectoryStream(examplesDir, "*.alg"))
  {
    shared_ptr<java::nio::file::DirectoryStream<
        std::shared_ptr<java::nio::file::Path>>>
        stream =
            java::nio::file::Files::newDirectoryStream(examplesDir, L"*.alg");
    for (auto path : stream) {
      shared_ptr<Config> config = make_shared<Config>(
          Files::newBufferedReader(path, StandardCharsets::UTF_8));
      wstring contentSource = config->get(L"content.source", L"");
      if (contentSource != L"") {
        type_info::forName(contentSource);
      }
      // C++ TODO: There is no native C++ equivalent to 'toString':
      config->set(L"work.dir",
                  createTempDir(LuceneTestCase::getTestClass().name())
                      ->toAbsolutePath()
                      ->toString());
      config->set(L"content.source", MockContentSource::typeid->getName());
      wstring dir = config->get(L"content.source", L"");
      if (dir != L"") {
        type_info::forName(dir);
      }
      config->set(L"directory", RAMDirectory::typeid->getName());
      if (config->get(L"line.file.out", L"") != L"") {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        config->set(
            L"line.file.out",
            createTempFile(L"linefile", L".txt")->toAbsolutePath()->toString());
      }
      if (config->get(L"query.maker", L"") != L"") {
        type_info::forName(config->get(L"query.maker", L""));
        config->set(L"query.maker", MockQueryMaker::typeid->getName());
      }
      shared_ptr<PerfRunData> data = make_shared<PerfRunData>(config);
      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try
      // (org.apache.lucene.benchmark.byTask.utils.Algorithm algo = new
      // org.apache.lucene.benchmark.byTask.utils.Algorithm(data))
      {
        org::apache::lucene::benchmark::byTask::utils::Algorithm algo =
            org::apache::lucene::benchmark::byTask::utils::Algorithm(data);
      }
      foundFiles = true;
    }
  }
  if (!foundFiles) {
    fail(L"could not find any .alg files!");
  }
}
} // namespace org::apache::lucene::benchmark::byTask