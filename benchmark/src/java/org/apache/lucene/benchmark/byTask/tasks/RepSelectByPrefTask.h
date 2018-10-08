#pragma once
#include "RepSumByPrefTask.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/benchmark/byTask/PerfRunData.h"

#include  "core/src/java/org/apache/lucene/benchmark/byTask/stats/Report.h"
#include  "core/src/java/org/apache/lucene/benchmark/byTask/stats/TaskStats.h"

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
 * Report by-name-prefix statistics with no aggregations.
 * <br>Other side effects: None.
 */
class RepSelectByPrefTask : public RepSumByPrefTask
{
  GET_CLASS_NAME(RepSelectByPrefTask)

public:
  RepSelectByPrefTask(std::shared_ptr<PerfRunData> runData);

  int doLogic()  override;

protected:
  virtual std::shared_ptr<Report>
  reportSelectByPrefix(std::deque<std::shared_ptr<TaskStats>> &taskStats);

protected:
  std::shared_ptr<RepSelectByPrefTask> shared_from_this()
  {
    return std::static_pointer_cast<RepSelectByPrefTask>(
        RepSumByPrefTask::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/tasks/
