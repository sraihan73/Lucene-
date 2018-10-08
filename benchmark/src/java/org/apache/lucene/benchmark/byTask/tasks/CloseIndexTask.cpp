using namespace std;

#include "CloseIndexTask.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/InfoStream.h"
#include "../PerfRunData.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using InfoStream = org::apache::lucene::util::InfoStream;

CloseIndexTask::CloseIndexTask(shared_ptr<PerfRunData> runData)
    : PerfTask(runData)
{
}

int CloseIndexTask::doLogic() 
{
  shared_ptr<IndexWriter> iw = getRunData()->getIndexWriter();
  if (iw != nullptr) {
    // If infoStream was set to output to a file, close it.
    shared_ptr<InfoStream> infoStream = iw->getConfig()->getInfoStream();
    if (infoStream != nullptr) {
      delete infoStream;
    }
    if (doWait == false) {
      iw->commit();
      iw->rollback();
    } else {
      delete iw;
    }
    getRunData()->setIndexWriter(nullptr);
  }
  return 1;
}

void CloseIndexTask::setParams(const wstring &params)
{
  PerfTask::setParams(params);
  doWait = StringHelper::fromString<bool>(params).booleanValue();
}

bool CloseIndexTask::supportsParams() { return true; }
} // namespace org::apache::lucene::benchmark::byTask::tasks