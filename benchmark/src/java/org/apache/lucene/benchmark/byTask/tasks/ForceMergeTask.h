#pragma once
#include "PerfTask.h"
#include "exceptionhelper.h"
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
 * Runs forceMerge on the index.
 * <br>Other side effects: none.
 */
class ForceMergeTask : public PerfTask
{
  GET_CLASS_NAME(ForceMergeTask)

public:
  ForceMergeTask(std::shared_ptr<PerfRunData> runData);

  int maxNumSegments = -1;

  int doLogic()  override;

  void setParams(const std::wstring &params) override;

  bool supportsParams() override;

protected:
  std::shared_ptr<ForceMergeTask> shared_from_this()
  {
    return std::static_pointer_cast<ForceMergeTask>(
        PerfTask::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask::tasks