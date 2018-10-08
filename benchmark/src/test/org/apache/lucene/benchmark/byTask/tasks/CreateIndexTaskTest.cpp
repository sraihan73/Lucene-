using namespace std;

#include "CreateIndexTaskTest.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/NoDeletionPolicy.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/NoMergePolicy.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/NoMergeScheduler.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/Version.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/PerfRunData.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/tasks/CloseIndexTask.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/tasks/CreateIndexTask.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/utils/Config.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using BenchmarkTestCase = org::apache::lucene::benchmark::BenchmarkTestCase;
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using NoDeletionPolicy = org::apache::lucene::index::NoDeletionPolicy;
using NoMergePolicy = org::apache::lucene::index::NoMergePolicy;
using NoMergeScheduler = org::apache::lucene::index::NoMergeScheduler;
using Version = org::apache::lucene::util::Version;

shared_ptr<PerfRunData> CreateIndexTaskTest::createPerfRunData(
    const wstring &infoStreamValue) 
{
  shared_ptr<Properties> props = make_shared<Properties>();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  props->setProperty(L"writer.version", Version::LATEST->toString());
  props->setProperty(L"print.props", L"false"); // don't print anything
  props->setProperty(L"directory", L"RAMDirectory");
  if (infoStreamValue != L"") {
    props->setProperty(L"writer.info.stream", infoStreamValue);
  }
  shared_ptr<Config> config = make_shared<Config>(props);
  return make_shared<PerfRunData>(config);
}

void CreateIndexTaskTest::testInfoStream_SystemOutErr() 
{

  shared_ptr<PrintStream> curOut = System::out;
  shared_ptr<ByteArrayOutputStream> baos = make_shared<ByteArrayOutputStream>();
  System::setOut(
      make_shared<PrintStream>(baos, false, Charset::defaultCharset().name()));
  try {
    shared_ptr<PerfRunData> runData = createPerfRunData(L"SystemOut");
    shared_ptr<CreateIndexTask> cit = make_shared<CreateIndexTask>(runData);
    cit->doLogic();
    (make_shared<CloseIndexTask>(runData))->doLogic();
    assertTrue(baos->size() > 0);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    System::setOut(curOut);
  }

  shared_ptr<PrintStream> curErr = System::err;
  baos->reset();
  System::setErr(
      make_shared<PrintStream>(baos, false, Charset::defaultCharset().name()));
  try {
    shared_ptr<PerfRunData> runData = createPerfRunData(L"SystemErr");
    shared_ptr<CreateIndexTask> cit = make_shared<CreateIndexTask>(runData);
    cit->doLogic();
    (make_shared<CloseIndexTask>(runData))->doLogic();
    assertTrue(baos->size() > 0);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    System::setErr(curErr);
  }
}

void CreateIndexTaskTest::testInfoStream_File() 
{

  shared_ptr<Path> outFile = getWorkDir()->resolve(L"infoStreamTest");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<PerfRunData> runData =
      createPerfRunData(outFile->toAbsolutePath()->toString());
  (make_shared<CreateIndexTask>(runData))->doLogic();
  (make_shared<CloseIndexTask>(runData))->doLogic();
  assertTrue(Files->size(outFile) > 0);
}

void CreateIndexTaskTest::testNoMergePolicy() 
{
  shared_ptr<PerfRunData> runData = createPerfRunData(L"");
  runData->getConfig()->set(L"merge.policy", NoMergePolicy::typeid->getName());
  (make_shared<CreateIndexTask>(runData))->doLogic();
  (make_shared<CloseIndexTask>(runData))->doLogic();
}

void CreateIndexTaskTest::testNoMergeScheduler() 
{
  shared_ptr<PerfRunData> runData = createPerfRunData(L"");
  runData->getConfig()->set(L"merge.scheduler",
                            NoMergeScheduler::typeid->getName());
  (make_shared<CreateIndexTask>(runData))->doLogic();
  (make_shared<CloseIndexTask>(runData))->doLogic();
}

void CreateIndexTaskTest::testNoDeletionPolicy() 
{
  shared_ptr<PerfRunData> runData = createPerfRunData(L"");
  runData->getConfig()->set(L"deletion.policy",
                            NoDeletionPolicy::typeid->getName());
  (make_shared<CreateIndexTask>(runData))->doLogic();
  (make_shared<CloseIndexTask>(runData))->doLogic();
}
} // namespace org::apache::lucene::benchmark::byTask::tasks