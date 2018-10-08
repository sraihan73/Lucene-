using namespace std;

#include "SearchWithCollectorTask.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/Collector.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/TopScoreDocCollector.h"
#include "../PerfRunData.h"
#include "../feeds/QueryMaker.h"
#include "../utils/Config.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using QueryMaker = org::apache::lucene::benchmark::byTask::feeds::QueryMaker;
using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using Collector = org::apache::lucene::search::Collector;
using TopScoreDocCollector = org::apache::lucene::search::TopScoreDocCollector;

SearchWithCollectorTask::SearchWithCollectorTask(
    shared_ptr<PerfRunData> runData)
    : SearchTask(runData)
{
}

void SearchWithCollectorTask::setup() 
{
  SearchTask::setup();
  // check to make sure either the doc is being stored
  shared_ptr<PerfRunData> runData = getRunData();
  shared_ptr<Config> config = runData->getConfig();
  clnName = config->get(L"collector.class", L"");
}

bool SearchWithCollectorTask::withCollector() { return true; }

shared_ptr<Collector>
SearchWithCollectorTask::createCollector() 
{
  shared_ptr<Collector> collector = nullptr;
  // C++ TODO: The following Java case-insensitive std::wstring method call is not
  // converted:
  if (clnName.equalsIgnoreCase(L"topScoreDoc") == true) {
    collector = TopScoreDocCollector::create(numHits());
  } else if (clnName.length() > 0) {
    collector =
        type_info::forName(clnName).asSubclass(Collector::typeid).newInstance();

  } else {
    collector = SearchTask::createCollector();
  }
  return collector;
}

shared_ptr<QueryMaker> SearchWithCollectorTask::getQueryMaker()
{
  return getRunData()->getQueryMaker(shared_from_this());
}

bool SearchWithCollectorTask::withRetrieve() { return false; }

bool SearchWithCollectorTask::withSearch() { return true; }

bool SearchWithCollectorTask::withTraverse() { return false; }

bool SearchWithCollectorTask::withWarm() { return false; }
} // namespace org::apache::lucene::benchmark::byTask::tasks