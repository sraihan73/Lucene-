using namespace std;

#include "PerfTask.h"
#include "../PerfRunData.h"
#include "../stats/Points.h"
#include "../stats/TaskStats.h"
#include "../utils/Config.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using Points = org::apache::lucene::benchmark::byTask::stats::Points;
using TaskStats = org::apache::lucene::benchmark::byTask::stats::TaskStats;
using Config = org::apache::lucene::benchmark::byTask::utils::Config;
const wstring PerfTask::NEW_LINE = System::getProperty(L"line.separator");

PerfTask::PerfTask()
{
  name = getClass().getSimpleName();
  if (StringHelper::endsWith(name, L"Task")) {
    name = name.substr(0, name.length() - 4);
  }
}

void PerfTask::setRunInBackground(int deltaPri)
{
  runInBackground = true;
  this->deltaPri = deltaPri;
}

bool PerfTask::getRunInBackground() { return runInBackground; }

int PerfTask::getBackgroundDeltaPriority() { return deltaPri; }

void PerfTask::stopNow() { stopNow_ = true; }

PerfTask::PerfTask(shared_ptr<PerfRunData> runData) : PerfTask()
{
  this->runData = runData;
  shared_ptr<Config> config = runData->getConfig();
  this->maxDepthLogStart = config->get(L"task.max.depth.log", 0);

  wstring logStepAtt = L"log.step";
  wstring taskLogStepAtt = L"log.step." + name;
  if (config->get(taskLogStepAtt, L"") != L"") {
    logStepAtt = taskLogStepAtt;
  }

  // It's important to read this from Config, to support vals-by-round.
  logStep = config->get(logStepAtt, DEFAULT_LOG_STEP);
  // To avoid the check 'if (logStep > 0)' in tearDown(). This effectively
  // turns logging off.
  if (logStep <= 0) {
    logStep = numeric_limits<int>::max();
  }
}

shared_ptr<PerfTask> PerfTask::clone() 
{
  // tasks having non primitive data structures should override this.
  // otherwise parallel running of a task sequence might not run correctly.
  return std::static_pointer_cast<PerfTask>(__super::clone());
}

PerfTask::~PerfTask() {}

int PerfTask::runAndMaybeStats(bool reportStats) 
{
  if (!reportStats || shouldNotRecordStats()) {
    setup();
    int count = doLogic();
    count = disableCounting ? 0 : count;
    tearDown();
    return count;
  }
  if (reportStats && depth <= maxDepthLogStart && !shouldNeverLogAtStart()) {
    wcout << L"------------> starting task: " << getName() << endl;
  }
  setup();
  shared_ptr<Points> pnts = runData->getPoints();
  shared_ptr<TaskStats> ts = pnts->markTaskStart(
      shared_from_this(), runData->getConfig()->getRoundNumber());
  int count = doLogic();
  count = disableCounting ? 0 : count;
  pnts->markTaskEnd(ts, count);
  tearDown();
  return count;
}

wstring PerfTask::getName()
{
  if (params == L"") {
    return name;
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return (make_shared<StringBuilder>(name))
      ->append(L'(')
      ->append(params)
      ->append(L')')
      ->toString();
}

void PerfTask::setName(const wstring &name) { this->name = name; }

shared_ptr<PerfRunData> PerfTask::getRunData() { return runData; }

int PerfTask::getDepth() { return depth; }

void PerfTask::setDepth(int depth) { this->depth = depth; }

wstring PerfTask::getPadding()
{
  std::deque<wchar_t> c(4 * getDepth());
  for (int i = 0; i < c.size(); i++) {
    c[i] = L' ';
  }
  return wstring(c);
}

wstring PerfTask::toString()
{
  wstring padd = getPadding();
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>(padd);
  if (disableCounting) {
    sb->append(L'-');
  }
  sb->append(getName());
  if (getRunInBackground()) {
    sb->append(L" &");
    int x = getBackgroundDeltaPriority();
    if (x != 0) {
      sb->append(x);
    }
  }
  return sb->toString();
}

int PerfTask::getMaxDepthLogStart() { return maxDepthLogStart; }

wstring PerfTask::getLogMessage(int recsCount)
{
  return L"processed " + to_wstring(recsCount) + L" records";
}

bool PerfTask::shouldNeverLogAtStart() { return false; }

bool PerfTask::shouldNotRecordStats() { return false; }

void PerfTask::setup()  {}

void PerfTask::tearDown() 
{
  if (++logStepCount % logStep == 0) {
    double time =
        (System::currentTimeMillis() - runData->getStartTimeMillis()) / 1000.0;
    wcout << wstring::format(Locale::ROOT, L"%7.2f", time) << L" sec --> "
          << Thread::currentThread().getName() << L" "
          << getLogMessage(logStepCount) << endl;
  }
}

bool PerfTask::supportsParams() { return false; }

void PerfTask::setParams(const wstring &params)
{
  if (!supportsParams()) {
    throw make_shared<UnsupportedOperationException>(
        getName() + L" does not support command line parameters.");
  }
  this->params = params;
}

wstring PerfTask::getParams() { return params; }

bool PerfTask::isDisableCounting() { return disableCounting; }

void PerfTask::setDisableCounting(bool disableCounting)
{
  this->disableCounting = disableCounting;
}

void PerfTask::setAlgLineNum(int algLineNum) { this->algLineNum = algLineNum; }

int PerfTask::getAlgLineNum() { return algLineNum; }
} // namespace org::apache::lucene::benchmark::byTask::tasks