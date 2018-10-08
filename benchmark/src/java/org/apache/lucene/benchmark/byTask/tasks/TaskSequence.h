#pragma once
#include "PerfTask.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::benchmark::byTask::tasks
{
class PerfTask;
}

namespace org::apache::lucene::benchmark::byTask
{
class PerfRunData;
}

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::benchmark::byTask::tasks
{

using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;

/**
 * Sequence of parallel or sequential tasks.
 */
class TaskSequence : public PerfTask
{
  GET_CLASS_NAME(TaskSequence)
public:
  static int REPEAT_EXHAUST;

private:
  std::deque<std::shared_ptr<PerfTask>> tasks;
  int repetitions = 1;
  bool parallel = false;
  std::shared_ptr<TaskSequence> parent;
  bool letChildReport = true;
  int rate = 0;
  bool perMin = false; // rate, if set, is, by default, be sec.
  std::wstring seqName;
  bool exhausted = false;
  bool resetExhausted = false;
  std::deque<std::shared_ptr<PerfTask>> tasksArray;
  bool anyExhaustibleTasks = false;
  bool collapsable = false; // to not collapse external sequence named in alg.

  bool fixedTime = false; // true if we run for fixed time
  double runTimeSec = 0;  // how long to run for
  const int64_t logByTimeMsec;

public:
  TaskSequence(std::shared_ptr<PerfRunData> runData, const std::wstring &name,
               std::shared_ptr<TaskSequence> parent, bool parallel);

  virtual ~TaskSequence();

private:
  void initTasksArray();

  /**
   * @return Returns the parallel.
   */
public:
  virtual bool isParallel();

  /**
   * @return Returns the repetitions.
   */
  virtual int getRepetitions();

private:
  std::deque<int> countsByTime;

public:
  virtual void setRunTime(double sec) ;

  /**
   * @param repetitions The repetitions to set.
   */
  virtual void setRepetitions(int repetitions) ;

  /**
   * @return Returns the parent.
   */
  virtual std::shared_ptr<TaskSequence> getParent();

  /*
   * (non-Javadoc)
   * @see org.apache.lucene.benchmark.byTask.tasks.PerfTask#doLogic()
   */
  int doLogic()  override;

private:
  class RunBackgroundTask : public Thread
  {
    GET_CLASS_NAME(RunBackgroundTask)
  private:
    const std::shared_ptr<PerfTask> task;
    const bool letChildReport;
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: private volatile int count;
    int count = 0;

  public:
    RunBackgroundTask(std::shared_ptr<PerfTask> task, bool letChildReport);

    virtual void stopNow() ;

    virtual int getCount();

    void run() override;

  protected:
    std::shared_ptr<RunBackgroundTask> shared_from_this()
    {
      return std::static_pointer_cast<RunBackgroundTask>(
          Thread::shared_from_this());
    }
  };

private:
  int doSerialTasks() ;

  int doSerialTasksWithRate() ;

  // update state regarding exhaustion.
  void updateExhausted(std::shared_ptr<PerfTask> task);

private:
  class ParallelTask : public Thread
  {
    GET_CLASS_NAME(ParallelTask)
  private:
    std::shared_ptr<TaskSequence> outerInstance;

  public:
    int count = 0;
    const std::shared_ptr<PerfTask> task;

    ParallelTask(std::shared_ptr<TaskSequence> outerInstance,
                 std::shared_ptr<PerfTask> task);

    void run() override;

  protected:
    std::shared_ptr<ParallelTask> shared_from_this()
    {
      return std::static_pointer_cast<ParallelTask>(Thread::shared_from_this());
    }
  };

public:
  void stopNow() override;

  std::deque<std::shared_ptr<ParallelTask>> runningParallelTasks;

private:
  int doParallelTasks() ;

  // run threads
  void startThreads(std::deque<std::shared_ptr<ParallelTask>> &t) throw(
      InterruptedException);

  // run threads with rate
  void startlThreadsWithRate(std::deque<std::shared_ptr<ParallelTask>>
                                 &t) ;

public:
  virtual void addTask(std::shared_ptr<PerfTask> task);

  /* (non-Javadoc)
   * @see java.lang.Object#toString()
   */
  virtual std::wstring toString();

  /**
   * Execute child tasks in a way that they do not report their time separately.
   */
  virtual void setNoChildReport();

  /**
   * Returns the rate per minute: how many operations should be performed in a
   * minute. If 0 this has no effect.
   * @return the rate per min: how many operations should be performed in a
   * minute.
   */
  virtual int getRate();

  /**
   * @param rate The rate to set.
   */
  virtual void setRate(int rate, bool perMin);

private:
  void setSequenceName();

public:
  std::wstring getName() override;

  /**
   * @return Returns the tasks.
   */
  virtual std::deque<std::shared_ptr<PerfTask>> getTasks();

  /* (non-Javadoc)
   * @see java.lang.Object#clone()
   */
protected:
  std::shared_ptr<TaskSequence>
  clone()  override;

  /**
   * Return true if can be collapsed in case it is outermost sequence
   */
public:
  virtual bool isCollapsable();

protected:
  std::shared_ptr<TaskSequence> shared_from_this()
  {
    return std::static_pointer_cast<TaskSequence>(PerfTask::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask::tasks
