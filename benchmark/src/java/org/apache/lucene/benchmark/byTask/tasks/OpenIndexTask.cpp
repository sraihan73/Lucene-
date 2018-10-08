using namespace std;

#include "OpenIndexTask.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexCommit.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriter.h"
#include "../PerfRunData.h"
#include "../utils/Config.h"
#include "CreateIndexTask.h"
#include "OpenReaderTask.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexCommit = org::apache::lucene::index::IndexCommit;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LogMergePolicy = org::apache::lucene::index::LogMergePolicy;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
const double OpenIndexTask::DEFAULT_RAM_FLUSH_MB = static_cast<int>(
    org::apache::lucene::index::IndexWriterConfig::DEFAULT_RAM_BUFFER_SIZE_MB);

OpenIndexTask::OpenIndexTask(shared_ptr<PerfRunData> runData)
    : PerfTask(runData)
{
}

int OpenIndexTask::doLogic() 
{
  shared_ptr<PerfRunData> runData = getRunData();
  shared_ptr<Config> config = runData->getConfig();
  shared_ptr<IndexCommit> *const ic;
  if (commitUserData != L"") {
    ic = OpenReaderTask::findIndexCommit(runData->getDirectory(),
                                         commitUserData);
  } else {
    ic.reset();
  }

  shared_ptr<IndexWriter> *const writer = CreateIndexTask::configureWriter(
      config, runData, IndexWriterConfig::OpenMode::APPEND, ic);
  runData->setIndexWriter(writer);
  return 1;
}

void OpenIndexTask::setParams(const wstring &params)
{
  PerfTask::setParams(params);
  if (params != L"") {
    // specifies which commit point to open
    commitUserData = params;
  }
}

bool OpenIndexTask::supportsParams() { return true; }
} // namespace org::apache::lucene::benchmark::byTask::tasks