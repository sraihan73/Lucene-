using namespace std;

#include "ForceMergeTask.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriter.h"
#include "../PerfRunData.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using IndexWriter = org::apache::lucene::index::IndexWriter;

ForceMergeTask::ForceMergeTask(shared_ptr<PerfRunData> runData)
    : PerfTask(runData)
{
}

int ForceMergeTask::doLogic() 
{
  if (maxNumSegments == -1) {
    throw make_shared<IllegalStateException>(
        L"required argument (maxNumSegments) was not specified");
  }
  shared_ptr<IndexWriter> iw = getRunData()->getIndexWriter();
  iw->forceMerge(maxNumSegments);
  // System.out.println("forceMerge called");
  return 1;
}

void ForceMergeTask::setParams(const wstring &params)
{
  PerfTask::setParams(params);
  maxNumSegments = static_cast<int>(stod(params));
}

bool ForceMergeTask::supportsParams() { return true; }
} // namespace org::apache::lucene::benchmark::byTask::tasks