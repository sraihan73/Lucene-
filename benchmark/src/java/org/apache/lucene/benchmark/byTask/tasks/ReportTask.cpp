using namespace std;

#include "ReportTask.h"
#include "../PerfRunData.h"
#include "../stats/Report.h"
#include "../stats/TaskStats.h"
#include "../utils/Format.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using Report = org::apache::lucene::benchmark::byTask::stats::Report;
using TaskStats = org::apache::lucene::benchmark::byTask::stats::TaskStats;
using Format = org::apache::lucene::benchmark::byTask::utils::Format;

ReportTask::ReportTask(shared_ptr<PerfRunData> runData) : PerfTask(runData) {}

bool ReportTask::shouldNeverLogAtStart() { return true; }

bool ReportTask::shouldNotRecordStats() { return true; }

const wstring ReportTask::newline = System::getProperty(L"line.separator");
const wstring ReportTask::OP = L"Operation  ";
const wstring ReportTask::ROUND = L" round";
const wstring ReportTask::RUNCNT = L"   runCnt";
const wstring ReportTask::RECCNT = L"   recsPerRun";
const wstring ReportTask::RECSEC = L"        rec/s";
const wstring ReportTask::ELAPSED = L"  elapsedSec";
const wstring ReportTask::USEDMEM = L"    avgUsedMem";
const wstring ReportTask::TOTMEM = L"    avgTotalMem";
std::deque<wstring> const ReportTask::COLS = {RUNCNT,  RECCNT,  RECSEC,
                                               ELAPSED, USEDMEM, TOTMEM};

wstring ReportTask::tableTitle(const wstring &longestOp)
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(Format::format(OP, longestOp));
  sb->append(ROUND);
  sb->append(getRunData()->getConfig()->getColsNamesForValsByRound());
  for (int i = 0; i < COLS.size(); i++) {
    sb->append(COLS[i]);
  }
  return sb->toString();
}

wstring ReportTask::longestOp(deque<std::shared_ptr<TaskStats>> &taskStats)
{
  wstring longest = OP;
  for (auto stat : taskStats) {
    if (stat->getElapsed() >= 0) { // consider only tasks that ended
      wstring name = stat->getTask()->getName();
      if (name.length() > longest.length()) {
        longest = name;
      }
    }
  }
  return longest;
}

wstring ReportTask::taskReportLine(const wstring &longestOp,
                                   shared_ptr<TaskStats> stat)
{
  shared_ptr<PerfTask> task = stat->getTask();
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(Format::format(task->getName(), longestOp));
  wstring round =
      (stat->getRound() >= 0 ? L"" + to_wstring(stat->getRound()) : L"-");
  sb->append(Format::formatPaddLeft(round, ROUND));
  sb->append(
      getRunData()->getConfig()->getColsValuesForValsByRound(stat->getRound()));
  sb->append(Format::format(stat->getNumRuns(), RUNCNT));
  sb->append(Format::format(stat->getCount() / stat->getNumRuns(), RECCNT));
  int64_t elapsed =
      (stat->getElapsed() > 0 ? stat->getElapsed() : 1); // assume at least 1ms
  sb->append(Format::format(
      2, static_cast<float>(stat->getCount() * 1000.0 / elapsed), RECSEC));
  sb->append(Format::format(2, static_cast<float>(stat->getElapsed()) / 1000,
                            ELAPSED));
  sb->append(Format::format(
      0, static_cast<float>(stat->getMaxUsedMem()) / stat->getNumRuns(),
      USEDMEM));
  sb->append(Format::format(
      0, static_cast<float>(stat->getMaxTotMem()) / stat->getNumRuns(),
      TOTMEM));
  return sb->toString();
}

shared_ptr<Report> ReportTask::genPartialReport(
    int reported,
    shared_ptr<LinkedHashMap<wstring, std::shared_ptr<TaskStats>>> partOfTasks,
    int totalSize)
{
  wstring longetOp = longestOp(partOfTasks->values());
  bool first = true;
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(tableTitle(longetOp));
  sb->append(newline);
  int lineNum = 0;
  for (auto stat : partOfTasks) {
    if (!first) {
      sb->append(newline);
    }
    first = false;
    wstring line = taskReportLine(longetOp, stat->second);
    lineNum++;
    if (partOfTasks->size() > 2 && lineNum % 2 == 0) {
      line = line.replaceAll(L"   ", L" - ");
    }
    sb->append(line);
    std::deque<int> byTime = stat->second.getCountsByTime();
    if (byTime.size() > 0) {
      sb->append(newline);
      int end = -1;
      for (int i = byTime.size() - 1; i >= 0; i--) {
        if (byTime[i] != 0) {
          end = i;
          break;
        }
      }
      if (end != -1) {
        sb->append(L"  by time:");
        for (int i = 0; i < end; i++) {
          sb->append(L' ')->append(byTime[i]);
        }
      }
    }
  }

  wstring reptxt =
      (reported == 0 ? L"No Matching Entries Were Found!" : sb->toString());
  return make_shared<Report>(reptxt, partOfTasks->size(), reported, totalSize);
}
} // namespace org::apache::lucene::benchmark::byTask::tasks