using namespace std;

#include "ConsumeContentSourceTask.h"
#include "../PerfRunData.h"
#include "../feeds/ContentSource.h"
#include "../feeds/DocData.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using ContentSource =
    org::apache::lucene::benchmark::byTask::feeds::ContentSource;
using DocData = org::apache::lucene::benchmark::byTask::feeds::DocData;

ConsumeContentSourceTask::ConsumeContentSourceTask(
    shared_ptr<PerfRunData> runData)
    : PerfTask(runData), source(runData->getContentSource())
{
}

wstring ConsumeContentSourceTask::getLogMessage(int recsCount)
{
  return L"read " + to_wstring(recsCount) +
         L" documents from the content source";
}

int ConsumeContentSourceTask::doLogic() 
{
  dd->set(source->getNextDocData(dd->get()));
  return 1;
}
} // namespace org::apache::lucene::benchmark::byTask::tasks