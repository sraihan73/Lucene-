using namespace std;

#include "SearchTravTask.h"
#include "../PerfRunData.h"
#include "../feeds/QueryMaker.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using QueryMaker = org::apache::lucene::benchmark::byTask::feeds::QueryMaker;

SearchTravTask::SearchTravTask(shared_ptr<PerfRunData> runData)
    : ReadTask(runData)
{
}

bool SearchTravTask::withRetrieve() { return false; }

bool SearchTravTask::withSearch() { return true; }

bool SearchTravTask::withTraverse() { return true; }

bool SearchTravTask::withWarm() { return false; }

shared_ptr<QueryMaker> SearchTravTask::getQueryMaker()
{
  return getRunData()->getQueryMaker(shared_from_this());
}

int SearchTravTask::traversalSize() { return traversalSize_; }

void SearchTravTask::setParams(const wstring &params)
{
  ReadTask::setParams(params);
  traversalSize_ = static_cast<int>(stof(params));
}

bool SearchTravTask::supportsParams() { return true; }
} // namespace org::apache::lucene::benchmark::byTask::tasks