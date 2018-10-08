using namespace std;

#include "RepAllTask.h"
#include "../PerfRunData.h"
#include "../stats/Report.h"
#include "../stats/TaskStats.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using Report = org::apache::lucene::benchmark::byTask::stats::Report;
using TaskStats = org::apache::lucene::benchmark::byTask::stats::TaskStats;

RepAllTask::RepAllTask(shared_ptr<PerfRunData> runData) : ReportTask(runData) {}

int RepAllTask::doLogic() 
{
  shared_ptr<Report> rp = reportAll(getRunData()->getPoints()->taskStats());

  wcout << endl;
  wcout << L"------------> Report All (" << rp->getSize() << L" out of "
        << rp->getOutOf() << L")" << endl;
  wcout << rp->getText() << endl;
  wcout << endl;
  return 0;
}

shared_ptr<Report>
RepAllTask::reportAll(deque<std::shared_ptr<TaskStats>> &taskStats)
{
  wstring longestOp = this->longestOp(taskStats);
  bool first = true;
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(tableTitle(longestOp));
  sb->append(newline);
  int reported = 0;
  for (auto stat : taskStats) {
    if (stat->getElapsed() >= 0) { // consider only tasks that ended
      if (!first) {
        sb->append(newline);
      }
      first = false;
      wstring line = taskReportLine(longestOp, stat);
      reported++;
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