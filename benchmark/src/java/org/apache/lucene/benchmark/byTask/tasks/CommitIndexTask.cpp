using namespace std;

#include "CommitIndexTask.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriter.h"
#include "../PerfRunData.h"
#include "OpenReaderTask.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using IndexWriter = org::apache::lucene::index::IndexWriter;

CommitIndexTask::CommitIndexTask(shared_ptr<PerfRunData> runData)
    : PerfTask(runData)
{
}

bool CommitIndexTask::supportsParams() { return true; }

void CommitIndexTask::setParams(const wstring &params)
{
  PerfTask::setParams(params);
  commitUserData = unordered_map<>();
  commitUserData.emplace(OpenReaderTask::USER_DATA, params);
}

int CommitIndexTask::doLogic() 
{
  shared_ptr<IndexWriter> iw = getRunData()->getIndexWriter();
  if (iw != nullptr) {
    if (commitUserData.size() > 0) {
      iw->setLiveCommitData(commitUserData.entrySet());
    }
    iw->commit();
  }

  return 1;
}
} // namespace org::apache::lucene::benchmark::byTask::tasks