using namespace std;

#include "RollbackIndexTask.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/InfoStream.h"
#include "../PerfRunData.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using InfoStream = org::apache::lucene::util::InfoStream;

RollbackIndexTask::RollbackIndexTask(shared_ptr<PerfRunData> runData)
    : PerfTask(runData)
{
}

int RollbackIndexTask::doLogic() 
{
  shared_ptr<IndexWriter> iw = getRunData()->getIndexWriter();
  if (iw != nullptr) {
    // If infoStream was set to output to a file, close it.
    shared_ptr<InfoStream> infoStream = iw->getConfig()->getInfoStream();
    if (infoStream != nullptr) {
      delete infoStream;
    }
    iw->rollback();
    getRunData()->setIndexWriter(nullptr);
  }
  return 1;
}
} // namespace org::apache::lucene::benchmark::byTask::tasks