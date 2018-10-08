using namespace std;

#include "TaskSequence.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"
#include "../PerfRunData.h"
#include "../feeds/NoMoreDataException.h"
#include "../stats/TaskStats.h"
#include "ResetInputsTask.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using NoMoreDataException =
    org::apache::lucene::benchmark::byTask::feeds::NoMoreDataException;
using TaskStats = org::apache::lucene::benchmark::byTask::stats::TaskStats;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
int TaskSequence::REPEAT_EXHAUST = -2;

TaskSequence::TaskSequence(shared_ptr<PerfRunData> runData, const wstring &name,
                           shared_ptr<TaskSequence> parent, bool parallel)
    : PerfTask(runData),
      logByTimeMsec(runData->getConfig()->get(L"report.time.step.msec", 0))
{
  collapsable = (name == L"");
  name = (name != L"" ? name : (parallel ? L"Par" : L"Seq"));
  setName(name);
  setSequenceName();
  this->parent = parent;
  this->parallel = parallel;
  tasks = deque<std::shared_ptr<PerfTask>>();
}

TaskSequence::~TaskSequence()
{
  initTasksArray();
  for (int i = 0; i < tasksArray.size(); i++) {
    tasksArray[i]->close();
  }
  delete getRunData()->getDocMaker();
}

void TaskSequence::initTasksArray()
{
  if (tasksArray.empty()) {
    constexpr int numTasks = tasks.size();
    tasksArray = std::deque<std::shared_ptr<PerfTask>>(numTasks);
    for (int k = 0; k < numTasks; k++) {
      tasksArray[k] = tasks[k];
      anyExhaustibleTasks |=
          std::dynamic_pointer_cast<ResetInputsTask>(tasksArray[k]) != nullptr;
      anyExhaustibleTasks |=
          std::dynamic_pointer_cast<TaskSequence>(tasksArray[k]) != nullptr;
    }
  }
  if (!parallel && logByTimeMsec != 0 && !letChildReport) {
    countsByTime = std::deque<int>(1);
  }
}

bool TaskSequence::isParallel() { return parallel; }

int TaskSequence::getRepetitions() { return repetitions; }

void TaskSequence::setRunTime(double sec) 
{
  runTimeSec = sec;
  fixedTime = true;
}

void TaskSequence::setRepetitions(int repetitions) 
{
  fixedTime = false;
  this->repetitions = repetitions;
  if (repetitions == REPEAT_EXHAUST) {
    if (isParallel()) {
      throw runtime_error(L"REPEAT_EXHAUST is not allowed for parallel tasks");
    }
  }
  setSequenceName();
}

shared_ptr<TaskSequence> TaskSequence::getParent() { return parent; }

int TaskSequence::doLogic() 
{
  exhausted = resetExhausted = false;
  return (parallel ? doParallelTasks() : doSerialTasks());
}

TaskSequence::RunBackgroundTask::RunBackgroundTask(shared_ptr<PerfTask> task,
                                                   bool letChildReport)
    : task(task), letChildReport(letChildReport)
{
}

void TaskSequence::RunBackgroundTask::stopNow() 
{
  task->stopNow();
}

int TaskSequence::RunBackgroundTask::getCount() { return count; }

void TaskSequence::RunBackgroundTask::run()
{
  try {
    count = task->runAndMaybeStats(letChildReport);
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}

int TaskSequence::doSerialTasks() 
{
  if (rate > 0) {
    return doSerialTasksWithRate();
  }

  initTasksArray();
  int count = 0;

  constexpr int64_t runTime = static_cast<int64_t>(runTimeSec * 1000);
  deque<std::shared_ptr<RunBackgroundTask>> bgTasks;

  constexpr int64_t t0 = System::currentTimeMillis();
  for (int k = 0; fixedTime || (repetitions == REPEAT_EXHAUST && !exhausted) ||
                  k < repetitions;
       k++) {
    if (stopNow_) {
      break;
    }
    for (int l = 0; l < tasksArray.size(); l++) {
      shared_ptr<PerfTask> *const task = tasksArray[l];
      if (task->getRunInBackground()) {
        if (bgTasks.empty()) {
          bgTasks = deque<>();
        }
        shared_ptr<RunBackgroundTask> bgTask =
            make_shared<RunBackgroundTask>(task, letChildReport);
        bgTask->setPriority(task->getBackgroundDeltaPriority() +
                            Thread::currentThread().getPriority());
        bgTask->start();
        bgTasks.push_back(bgTask);
      } else {
        try {
          constexpr int inc = task->runAndMaybeStats(letChildReport);
          count += inc;
          if (countsByTime.size() > 0) {
            constexpr int slot = static_cast<int>(
                (System::currentTimeMillis() - t0) / logByTimeMsec);
            if (slot >= countsByTime.size()) {
              countsByTime = ArrayUtil::grow(countsByTime, 1 + slot);
            }
            countsByTime[slot] += inc;
          }
          if (anyExhaustibleTasks) {
            updateExhausted(task);
          }
        } catch (const NoMoreDataException &e) {
          exhausted = true;
        }
      }
    }
    if (fixedTime && System::currentTimeMillis() - t0 > runTime) {
      repetitions = k + 1;
      break;
    }
  }

  if (bgTasks.size() > 0) {
    for (auto bgTask : bgTasks) {
      bgTask->stopNow();
    }
    for (auto bgTask : bgTasks) {
      bgTask->join();
      count += bgTask->getCount();
    }
  }

  if (countsByTime.size() > 0) {
    getRunData()->getPoints()->getCurrentStats()->setCountsByTime(
        countsByTime, logByTimeMsec);
  }

  stopNow_ = false;

  return count;
}

int TaskSequence::doSerialTasksWithRate() 
{
  initTasksArray();
  int64_t delayStep = (perMin ? 60000 : 1000) / rate;
  int64_t nextStartTime = System::currentTimeMillis();
  int count = 0;
  constexpr int64_t t0 = System::currentTimeMillis();
  for (int k = 0;
       (repetitions == REPEAT_EXHAUST && !exhausted) || k < repetitions; k++) {
    if (stopNow_) {
      break;
    }
    for (int l = 0; l < tasksArray.size(); l++) {
      shared_ptr<PerfTask> *const task = tasksArray[l];
      while (!stopNow_) {
        int64_t waitMore = nextStartTime - System::currentTimeMillis();
        if (waitMore > 0) {
          // TODO: better to use condition to notify
          delay(1);
        } else {
          break;
        }
      }
      if (stopNow_) {
        break;
      }
      nextStartTime += delayStep; // this aims at avarage rate.
      try {
        constexpr int inc = task->runAndMaybeStats(letChildReport);
        count += inc;
        if (countsByTime.size() > 0) {
          constexpr int slot = static_cast<int>(
              (System::currentTimeMillis() - t0) / logByTimeMsec);
          if (slot >= countsByTime.size()) {
            countsByTime = ArrayUtil::grow(countsByTime, 1 + slot);
          }
          countsByTime[slot] += inc;
        }

        if (anyExhaustibleTasks) {
          updateExhausted(task);
        }
      } catch (const NoMoreDataException &e) {
        exhausted = true;
      }
    }
  }
  stopNow_ = false;
  return count;
}

void TaskSequence::updateExhausted(shared_ptr<PerfTask> task)
{
  if (std::dynamic_pointer_cast<ResetInputsTask>(task) != nullptr) {
    exhausted = false;
    resetExhausted = true;
  } else if (std::dynamic_pointer_cast<TaskSequence>(task) != nullptr) {
    shared_ptr<TaskSequence> t = std::static_pointer_cast<TaskSequence>(task);
    if (t->resetExhausted) {
      exhausted = false;
      resetExhausted = true;
      t->resetExhausted = false;
    } else {
      exhausted |= t->exhausted;
    }
  }
}

TaskSequence::ParallelTask::ParallelTask(shared_ptr<TaskSequence> outerInstance,
                                         shared_ptr<PerfTask> task)
    : task(task), outerInstance(outerInstance)
{
}

void TaskSequence::ParallelTask::run()
{
  try {
    int n = task->runAndMaybeStats(outerInstance->letChildReport);
    if (outerInstance->anyExhaustibleTasks) {
      outerInstance->updateExhausted(task);
    }
    count += n;
  } catch (const NoMoreDataException &e) {
    outerInstance->exhausted = true;
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}

void TaskSequence::stopNow()
{
  PerfTask::stopNow();
  // Forwards top request to children
  if (runningParallelTasks.size() > 0) {
    for (auto t : runningParallelTasks) {
      if (t != nullptr) {
        t->task->stopNow();
      }
    }
  }
}

int TaskSequence::doParallelTasks() 
{

  shared_ptr<TaskStats> *const stats =
      getRunData()->getPoints()->getCurrentStats();

  initTasksArray();
  std::deque<std::shared_ptr<ParallelTask>> t = runningParallelTasks =
      std::deque<std::shared_ptr<ParallelTask>>(repetitions * tasks.size());
  // prepare threads
  int index = 0;
  for (int k = 0; k < repetitions; k++) {
    for (int i = 0; i < tasksArray.size(); i++) {
      shared_ptr<PerfTask> *const task = tasksArray[i]->clone();
      t[index++] = make_shared<ParallelTask>(shared_from_this(), task);
    }
  }
  // run threads
  startThreads(t);

  if (stopNow_) {
    for (auto task : t) {
      task->task->stopNow();
    }
  }

  // wait for all threads to complete
  int count = 0;
  for (int i = 0; i < t.size(); i++) {
    t[i]->join();
    count += t[i]->count;
    if (std::dynamic_pointer_cast<TaskSequence>(t[i]->task) != nullptr) {
      shared_ptr<TaskSequence> sub =
          std::static_pointer_cast<TaskSequence>(t[i]->task);
      if (sub->countsByTime.size() > 0) {
        if (countsByTime.empty()) {
          countsByTime = std::deque<int>(sub->countsByTime.size());
        } else if (countsByTime.size() < sub->countsByTime.size()) {
          countsByTime =
              ArrayUtil::grow(countsByTime, sub->countsByTime.size());
        }
        for (int j = 0; j < sub->countsByTime.size(); j++) {
          countsByTime[j] += sub->countsByTime[j];
        }
      }
    }
  }

  if (countsByTime.size() > 0) {
    stats->setCountsByTime(countsByTime, logByTimeMsec);
  }

  // return total count
  return count;
}

void TaskSequence::startThreads(
    std::deque<std::shared_ptr<ParallelTask>> &t) 
{
  if (rate > 0) {
    startlThreadsWithRate(t);
    return;
  }
  for (int i = 0; i < t.size(); i++) {
    t[i]->start();
  }
}

void TaskSequence::startlThreadsWithRate(
    std::deque<std::shared_ptr<ParallelTask>> &t) 
{
  int64_t delayStep = (perMin ? 60000 : 1000) / rate;
  int64_t nextStartTime = System::currentTimeMillis();
  for (int i = 0; i < t.size(); i++) {
    int64_t waitMore = nextStartTime - System::currentTimeMillis();
    if (waitMore > 0) {
      delay(waitMore);
    }
    nextStartTime +=
        delayStep; // this aims at average rate of starting threads.
    t[i]->start();
  }
}

void TaskSequence::addTask(shared_ptr<PerfTask> task)
{
  tasks.push_back(task);
  task->setDepth(getDepth() + 1);
}

wstring TaskSequence::toString()
{
  wstring padd = getPadding();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<StringBuilder> sb =
      make_shared<StringBuilder>(PerfTask::toString());
  sb->append(parallel ? L" [" : L" {");
  sb->append(NEW_LINE);
  for (auto task : tasks) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    sb->append(task->toString());
    sb->append(NEW_LINE);
  }
  sb->append(padd);
  sb->append(!letChildReport ? L">" : (parallel ? L"]" : L"}"));
  if (fixedTime) {
    sb->append(
        L" " +
        NumberFormat::getNumberInstance(Locale::ROOT).format(runTimeSec) +
        L"s");
  } else if (repetitions > 1) {
    sb->append(L" * " + to_wstring(repetitions));
  } else if (repetitions == REPEAT_EXHAUST) {
    sb->append(L" * EXHAUST");
  }
  if (rate > 0) {
    sb->append(L",  rate: " + to_wstring(rate) + L"/" +
               (perMin ? L"min" : L"sec"));
  }
  if (getRunInBackground()) {
    sb->append(L" &");
    int x = getBackgroundDeltaPriority();
    if (x != 0) {
      sb->append(x);
    }
  }
  return sb->toString();
}

void TaskSequence::setNoChildReport()
{
  letChildReport = false;
  for (auto task : tasks) {
    if (std::dynamic_pointer_cast<TaskSequence>(task) != nullptr) {
      (std::static_pointer_cast<TaskSequence>(task))->setNoChildReport();
    }
  }
}

int TaskSequence::getRate() { return (perMin ? rate : 60 * rate); }

void TaskSequence::setRate(int rate, bool perMin)
{
  this->rate = rate;
  this->perMin = perMin;
  setSequenceName();
}

void TaskSequence::setSequenceName()
{
  seqName = PerfTask::getName();
  if (repetitions == REPEAT_EXHAUST) {
    seqName += L"_Exhaust";
  } else if (repetitions > 1) {
    seqName += L"_" + to_wstring(repetitions);
  }
  if (rate > 0) {
    seqName += L"_" + to_wstring(rate) + (perMin ? L"/min" : L"/sec");
  }
  if (parallel && seqName.toLowerCase(Locale::ROOT)->find(L"par") < 0) {
    seqName += L"_Par";
  }
}

wstring TaskSequence::getName()
{
  return seqName; // override to include more info
}

deque<std::shared_ptr<PerfTask>> TaskSequence::getTasks() { return tasks; }

shared_ptr<TaskSequence> TaskSequence::clone() 
{
  shared_ptr<TaskSequence> res =
      std::static_pointer_cast<TaskSequence>(PerfTask::clone());
  res->tasks = deque<std::shared_ptr<PerfTask>>();
  for (int i = 0; i < tasks.size(); i++) {
    res->tasks.push_back(tasks[i]->clone());
  }
  return res;
}

bool TaskSequence::isCollapsable() { return collapsable; }
} // namespace org::apache::lucene::benchmark::byTask::tasks