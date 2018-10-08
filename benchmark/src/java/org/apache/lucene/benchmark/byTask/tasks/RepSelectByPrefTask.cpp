using namespace std;

#include "RepSelectByPrefTask.h"
#include "../PerfRunData.h"
#include "../stats/Report.h"
#include "../stats/TaskStats.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using Report = org::apache::lucene::benchmark::byTask::stats::Report;
using TaskStats = org::apache::lucene::benchmark::byTask::stats::TaskStats;

RepSelectByPrefTask::RepSelectByPrefTask(shared_ptr<PerfRunData> runData)
    : RepSumByPrefTask(runData)
{
}

int RepSelectByPrefTask::doLogic() 
{
  shared_ptr<Report> rp =
      reportSelectByPrefix(getRunData()->getPoints()->taskStats());

  wcout << endl;
  wcout << L"------------> Report Select By Prefix (" << prefix << L") ("
        << rp->getSize() << L" about " << rp->getReported() << L" out of "
        << rp->getOutOf() << L")" << endl;
  wcout << rp->getText() << endl;
  wcout << endl;

  return 0;
}

shared_ptr<Report> RepSelectByPrefTask::reportSelectByPrefix(
    deque<std::shared_ptr<TaskStats>> &taskStats)
{
  wstring longestOp = this->longestOp(taskStats);
  bool first = true;
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(tableTitle(longestOp));
  sb->append(newline);
  int reported = 0;
  for (auto stat : taskStats) {
    if (stat->getElapsed() >= 0 &&
        StringHelper::startsWith(stat->getTask()->getName(),
                                 prefix)) { // only ended tasks with proper name
      reported++;
      if (!first) {
        sb->append(newline);
      }
      first = false;
      wstring line = taskReportLine(longestOp, stat);
      if (taskStats.size() > 2 && reported % 2 == 0) {
        line = line.replaceAll(L"   ", L" - ");
      }
      sb->append(line);
    }
  }
  wstring reptxt =
      (reported == 0 ? L"No Matching Entries Were Found!" : sb->toString());
  return make_shared<Report>(reptxt, reported, reported, taskStats.size());
}
} // namespace org::apache::lucene::benchmark::byTask::tasks