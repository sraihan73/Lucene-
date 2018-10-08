#pragma once
#include "PerfTask.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/benchmark/byTask/PerfRunData.h"

#include  "core/src/java/org/apache/lucene/benchmark/byTask/stats/TaskStats.h"
#include  "core/src/java/org/apache/lucene/benchmark/byTask/stats/Report.h"

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
using Report = org::apache::lucene::benchmark::byTask::stats::Report;
using TaskStats = org::apache::lucene::benchmark::byTask::stats::TaskStats;

/**
 * Report (abstract) task - all report tasks extend this task.
 */
class ReportTask : public PerfTask
{
  GET_CLASS_NAME(ReportTask)

public:
  ReportTask(std::shared_ptr<PerfRunData> runData);

  /* (non-Javadoc)
   * @see PerfTask#shouldNeverLogAtStart()
   */
protected:
  bool shouldNeverLogAtStart() override;

  /* (non-Javadoc)
   * @see PerfTask#shouldNotRecordStats()
   */
  bool shouldNotRecordStats() override;

  /*
   * From here start the code used to generate the reports.
   * Subclasses would use this part to generate reports.
   */

  static const std::wstring newline;

  /**
   * Get a textual summary of the benchmark results, average from all test runs.
   */
  static const std::wstring OP;
  static const std::wstring ROUND;
  static const std::wstring RUNCNT;
  static const std::wstring RECCNT;
  static const std::wstring RECSEC;
  static const std::wstring ELAPSED;
  static const std::wstring USEDMEM;
  static const std::wstring TOTMEM;
  static std::deque<std::wstring> const COLS;

  /**
   * Compute a title line for a report table
   * @param longestOp size of longest op name in the table
   * @return the table title line.
   */
  virtual std::wstring tableTitle(const std::wstring &longestOp);

  /**
   * find the longest op name out of completed tasks.
   * @param taskStats completed tasks to be considered.
   * @return the longest op name out of completed tasks.
   */
  virtual std::wstring
  longestOp(std::deque<std::shared_ptr<TaskStats>> &taskStats);

  /**
   * Compute a report line for the given task stat.
   * @param longestOp size of longest op name in the table.
   * @param stat task stat to be printed.
   * @return the report line.
   */
  virtual std::wstring taskReportLine(const std::wstring &longestOp,
                                      std::shared_ptr<TaskStats> stat);

  virtual std::shared_ptr<Report> genPartialReport(
      int reported,
      std::shared_ptr<LinkedHashMap<std::wstring, std::shared_ptr<TaskStats>>>
          partOfTasks,
      int totalSize);

protected:
  std::shared_ptr<ReportTask> shared_from_this()
  {
    return std::static_pointer_cast<ReportTask>(PerfTask::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/tasks/
