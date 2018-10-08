#pragma once
#include "ResetInputsTask.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/benchmark/byTask/PerfRunData.h"

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
 * Reset all index and input data and call gc, does NOT erase index/dir, does
 * NOT clear statistics. This contains ResetInputs. <br>Other side effects:
 * writers/readers nullified, closed. Index is NOT erased. Directory is NOT
 * erased.
 */
class ResetSystemSoftTask : public ResetInputsTask
{
  GET_CLASS_NAME(ResetSystemSoftTask)

public:
  ResetSystemSoftTask(std::shared_ptr<PerfRunData> runData);

  int doLogic()  override;

protected:
  std::shared_ptr<ResetSystemSoftTask> shared_from_this()
  {
    return std::static_pointer_cast<ResetSystemSoftTask>(
        ResetInputsTask::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/tasks/
