using namespace std;

#include "CommitIndexTaskTest.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/SegmentInfos.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/Version.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/PerfRunData.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/tasks/CloseIndexTask.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/tasks/CommitIndexTask.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/tasks/CreateIndexTask.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/tasks/OpenReaderTask.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/utils/Config.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using BenchmarkTestCase = org::apache::lucene::benchmark::BenchmarkTestCase;
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using SegmentInfos = org::apache::lucene::index::SegmentInfos;
using Version = org::apache::lucene::util::Version;

shared_ptr<PerfRunData>
CommitIndexTaskTest::createPerfRunData() 
{
  shared_ptr<Properties> props = make_shared<Properties>();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  props->setProperty(L"writer.version", Version::LATEST->toString());
  props->setProperty(L"print.props", L"false"); // don't print anything
  props->setProperty(L"directory", L"RAMDirectory");
  shared_ptr<Config> config = make_shared<Config>(props);
  return make_shared<PerfRunData>(config);
}

void CommitIndexTaskTest::testNoParams() 
{
  shared_ptr<PerfRunData> runData = createPerfRunData();
  (make_shared<CreateIndexTask>(runData))->doLogic();
  (make_shared<CommitIndexTask>(runData))->doLogic();
  (make_shared<CloseIndexTask>(runData))->doLogic();
}

void CommitIndexTaskTest::testCommitData() 
{
  shared_ptr<PerfRunData> runData = createPerfRunData();
  (make_shared<CreateIndexTask>(runData))->doLogic();
  shared_ptr<CommitIndexTask> task = make_shared<CommitIndexTask>(runData);
  task->setParams(L"params");
  task->doLogic();
  shared_ptr<SegmentInfos> infos =
      SegmentInfos::readLatestCommit(runData->getDirectory());
  assertEquals(L"params", infos->getUserData()[OpenReaderTask::USER_DATA]);
  (make_shared<CloseIndexTask>(runData))->doLogic();
}
} // namespace org::apache::lucene::benchmark::byTask::tasks