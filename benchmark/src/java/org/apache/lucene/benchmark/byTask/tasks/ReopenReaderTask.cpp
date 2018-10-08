using namespace std;

#include "ReopenReaderTask.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include "../PerfRunData.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;

ReopenReaderTask::ReopenReaderTask(shared_ptr<PerfRunData> runData)
    : PerfTask(runData)
{
}

int ReopenReaderTask::doLogic() 
{
  shared_ptr<DirectoryReader> r = getRunData()->getIndexReader();
  shared_ptr<DirectoryReader> nr = DirectoryReader::openIfChanged(r);
  if (nr != nullptr) {
    getRunData()->setIndexReader(nr);
    nr->decRef();
  }
  r->decRef();
  return 1;
}
} // namespace org::apache::lucene::benchmark::byTask::tasks