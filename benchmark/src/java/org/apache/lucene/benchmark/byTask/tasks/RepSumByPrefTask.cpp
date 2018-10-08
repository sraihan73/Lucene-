using namespace std;

#include "RepSumByPrefTask.h"
#include "../PerfRunData.h"
#include "../stats/Report.h"
#include "../stats/TaskStats.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using Report = org::apache::lucene::benchmark::byTask::stats::Report;
using TaskStats = org::apache::lucene::benchmark::byTask::stats::TaskStats;

RepSumByPrefTask::RepSumByPrefTask(shared_ptr<PerfRunData> runData)
    : ReportTask(runData)
{
}

int RepSumByPrefTask::doLogic() 
{
  shared_ptr<Report> rp =
      reportSumByPrefix(getRunData()->getPoints()->taskStats());

  wcout << endl;
  wcout << L"------------> Report Sum By Prefix (" << prefix << L") ("
        << rp->getSize() << L" about " << rp->getReported() << L" out of "
        << rp->getOutOf() << L")" << endl;
  wcout << rp->getText() << endl;
  wcout << endl;

  return 0;
}

shared_ptr<Report> RepSumByPrefTask::reportSumByPrefix(
    deque<std::shared_ptr<TaskStats>> &taskStats)
{
  // aggregate by task name
  int reported = 0;
  shared_ptr<LinkedHashMap<wstring, std::shared_ptr<TaskStats>>> p2 =
      make_shared<LinkedHashMap<wstring, std::shared_ptr<TaskStats>>>();
  for (auto stat1 : taskStats) {
    if (stat1->getElapsed() >= 0 &&
        StringHelper::startsWith(stat1->getTask()->getName(),
                                 prefix)) { // only ended tasks with proper name
      reported++;
      wstring name = stat1->getTask()->getName();
      shared_ptr<TaskStats> stat2 = p2->get(name);
      if (stat2 == nullptr) {
        try {
          stat2 = stat1->clone();
        } catch (const CloneNotSupportedException &e) {
          throw runtime_error(e);
        }
        p2->put(name, stat2);
      } else {
        stat2->add(stat1);
      }
    }
  }
  // now generate report from secondary deque p2
  return genPartialReport(reported, p2, taskStats.size());
}

void RepSumByPrefTask::setPrefix(const wstring &prefix)
{
  this->prefix = prefix;
}

wstring RepSumByPrefTask::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return ReportTask::toString() + L" " + prefix;
}
} // namespace org::apache::lucene::benchmark::byTask::tasks