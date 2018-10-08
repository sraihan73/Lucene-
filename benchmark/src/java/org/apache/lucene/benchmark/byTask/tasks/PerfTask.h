#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
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
 * An abstract task to be tested for performance. <br>
 * Every performance task extends this class, and provides its own
 * {@link #doLogic()} method, which performs the actual task. <br>
GET_CLASS_NAME(,)
 * Tasks performing some work that should be measured for the task, can override
 * {@link #setup()} and/or {@link #tearDown()} and place that work there. <br>
 * Relevant properties: <code>task.max.depth.log</code>.<br>
 * Also supports the following logging attributes:
 * <ul>
 * <li>log.step - specifies how often to log messages about the current running
 * task. Default is 1000 {@link #doLogic()} invocations. Set to -1 to disable
 * logging.
 * <li>log.step.[class Task Name] - specifies the same as 'log.step', only for a
 * particular task name. For example, log.step.AddDoc will be applied only for
 * {@link AddDocTask}. It's a way to control
 * per task logging settings. If you want to omit logging for any other task,
 * include log.step=-1. The syntax is "log.step." together with the Task's
 * 'short' name (i.e., without the 'Task' part).
 * </ul>
 */
class PerfTask : public std::enable_shared_from_this<PerfTask>, public Cloneable
{
  GET_CLASS_NAME(PerfTask)

public:
  static constexpr int DEFAULT_LOG_STEP = 1000;

private:
  std::shared_ptr<PerfRunData> runData;

  // propeties that all tasks have
  std::wstring name;
  int depth = 0;

protected:
  int logStep = 0;

private:
  int logStepCount = 0;
  int maxDepthLogStart = 0;
  bool disableCounting = false;

protected:
  std::wstring params = L"";

private:
  bool runInBackground = false;
  int deltaPri = 0;

  // The first line of this task's definition in the alg file
  int algLineNum = 0;

protected:
  static const std::wstring NEW_LINE;

  /** Should not be used externally */
private:
  PerfTask();

public:
  virtual void setRunInBackground(int deltaPri);

  virtual bool getRunInBackground();

  virtual int getBackgroundDeltaPriority();

protected:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: protected volatile bool stopNow;
  // C++ NOTE: Fields cannot have the same name as methods:
  bool stopNow_ = false;

public:
  virtual void stopNow();

  PerfTask(std::shared_ptr<PerfRunData> runData);

protected:
  std::shared_ptr<PerfTask> clone()  override;

public:
  virtual ~PerfTask();

  /**
   * Run the task, record statistics.
   * @return number of work items done by this task.
   */
  int runAndMaybeStats(bool reportStats) ;

  /**
   * Perform the task once (ignoring repetitions specification)
   * Return number of work items done by this task.
   * For indexing that can be number of docs added.
   * For warming that can be number of scanned items, etc.
   * @return number of work items done by this task.
   */
  virtual int doLogic() = 0;

  /**
   * @return Returns the name.
   */
  virtual std::wstring getName();

  /**
   * @param name The name to set.
   */
protected:
  virtual void setName(const std::wstring &name);

  /**
   * @return Returns the run data.
   */
public:
  virtual std::shared_ptr<PerfRunData> getRunData();

  /**
   * @return Returns the depth.
   */
  virtual int getDepth();

  /**
   * @param depth The depth to set.
   */
  virtual void setDepth(int depth);

  // compute a blank string padding for printing this task indented by its depth
  virtual std::wstring getPadding();

  virtual std::wstring toString();

  /**
   * @return Returns the maxDepthLogStart.
   */
  virtual int getMaxDepthLogStart();

protected:
  virtual std::wstring getLogMessage(int recsCount);

  /**
   * Tasks that should never log at start can override this.
   * @return true if this task should never log when it start.
   */
  virtual bool shouldNeverLogAtStart();

  /**
   * Tasks that should not record statistics can override this.
   * @return true if this task should never record its statistics.
   */
  virtual bool shouldNotRecordStats();

  /**
   * Task setup work that should not be measured for that specific task. By
   * default it does nothing, but tasks can implement this, moving work from
   * {@link #doLogic()} to this method. Only the work done in {@link #doLogic()}
   * is measured for this task. Notice that higher level (sequence) tasks
   * containing this task would then measure larger time than the sum of their
   * contained tasks.
   */
public:
  virtual void setup() ;

  /**
   * Task tearDown work that should not be measured for that specific task. By
   * default it does nothing, but tasks can implement this, moving work from
   * {@link #doLogic()} to this method. Only the work done in {@link #doLogic()}
   * is measured for this task. Notice that higher level (sequence) tasks
   * containing this task would then measure larger time than the sum of their
   * contained tasks.
   */
  virtual void tearDown() ;

  /**
   * Sub classes that support parameters must override this method to return
   * true.
   *
   * @return true iff this task supports command line params.
   */
  virtual bool supportsParams();

  /**
   * Set the params of this task.
   *
   * @exception UnsupportedOperationException
   *              for tasks supporting command line parameters.
   */
  virtual void setParams(const std::wstring &params);

  /**
   * @return Returns the Params.
   */
  virtual std::wstring getParams();

  /**
   * Return true if counting is disabled for this task.
   */
  virtual bool isDisableCounting();

  /**
   * See {@link #isDisableCounting()}
   */
  virtual void setDisableCounting(bool disableCounting);

  virtual void setAlgLineNum(int algLineNum);

  virtual int getAlgLineNum();
};

} // namespace org::apache::lucene::benchmark::byTask::tasks
