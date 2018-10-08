using namespace std;

#include "RepSumByNameRoundTask.h"
#include "../PerfRunData.h"
#include "../stats/Report.h"
#include "../stats/TaskStats.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using Report = org::apache::lucene::benchmark::byTask::stats::Report;
using TaskStats = org::apache::lucene::benchmark::byTask::stats::TaskStats;

RepSumByNameRoundTask::RepSumByNameRoundTask(shared_ptr<PerfRunData> runData)
    : ReportTask(runData)
{
}

int RepSumByNameRoundTask::doLogic() 
{
  shared_ptr<Report> rp =
      reportSumByNameRound(getRunData()->getPoints()->taskStats());

  wcout << endl;
  wcout << L"------------> Report Sum By (any) Name and Round ("
        << rp->getSize() << L" about " << rp->getReported() << L" out of "
        << rp->getOutOf() << L")" << endl;
  wcout << rp->getText() << endl;
  wcout << endl;

  return 0;
}

shared_ptr<Report> RepSumByNameRoundTask::reportSumByNameRound(
    deque<std::shared_ptr<TaskStats>> &taskStats)
{
  // aggregate by task name and round
  shared_ptr<LinkedHashMap<wstring, std::shared_ptr<TaskStats>>> p2 =
      make_shared<LinkedHashMap<wstring, std::shared_ptr<TaskStats>>>();
  int reported = 0;
  for (auto stat1 : taskStats) {
    if (stat1->getElapsed() >= 0) { // consider only tasks that ended
      reported++;
      wstring name = stat1->getTask()->getName();
      wstring rname =
          to_wstring(stat1->getRound()) + L"." + name; // group by round
      shared_ptr<TaskStats> stat2 = p2->get(rname);
      if (stat2 == nullptr) {
        try {
          stat2 = stat1->clone();
        } catch (const CloneNotSupportedException &e) {
          throw runtime_error(e);
        }
        p2->put(rname, stat2);
      } else {
        stat2->add(stat1);
      }
    }
  }
  // now generate report from secondary deque p2
  return genPartialReport(reported, p2, taskStats.size());
}
} // namespace org::apache::lucene::benchmark::byTask::tasks