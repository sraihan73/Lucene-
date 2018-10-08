#pragma once
#include "PerfTask.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

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
 * Simply waits for the specified (via the parameter) amount
 * of time.  For example Wait(30s) waits for 30 seconds.
 * This is useful with background tasks to control how long
 * the tasks run.
 *
 *<p>You can specify h, m, or s (hours, minutes, seconds) as
 *the trailing time unit.  No unit is interpreted as
 *seconds.</p>
 */
class WaitTask : public PerfTask
{
  GET_CLASS_NAME(WaitTask)

private:
  double waitTimeSec = 0;

public:
  WaitTask(std::shared_ptr<PerfRunData> runData);

  void setParams(const std::wstring &params) override;

  int doLogic()  override;

  bool supportsParams() override;

protected:
  std::shared_ptr<WaitTask> shared_from_this()
  {
    return std::static_pointer_cast<WaitTask>(PerfTask::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask::tasks
