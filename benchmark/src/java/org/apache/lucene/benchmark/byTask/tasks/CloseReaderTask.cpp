using namespace std;

#include "CloseReaderTask.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../PerfRunData.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using IndexReader = org::apache::lucene::index::IndexReader;

CloseReaderTask::CloseReaderTask(shared_ptr<PerfRunData> runData)
    : PerfTask(runData)
{
}

int CloseReaderTask::doLogic() 
{
  shared_ptr<IndexReader> reader = getRunData()->getIndexReader();
  getRunData()->setIndexReader(nullptr);
  if (reader->getRefCount() != 1) {
    wcout << L"WARNING: CloseReader: reference count is currently "
          << reader->getRefCount() << endl;
  }
  reader->decRef();
  return 1;
}
} // namespace org::apache::lucene::benchmark::byTask::tasks