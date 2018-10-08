#pragma once
#include "PerfTask.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>

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
 * Close index reader.
 * <br>Other side effects: index reader in perfRunData is nullified.
 * <br>This would cause read related tasks to reopen their own reader.
 */
class CloseReaderTask : public PerfTask
{
  GET_CLASS_NAME(CloseReaderTask)

public:
  CloseReaderTask(std::shared_ptr<PerfRunData> runData);

  int doLogic()  override;

protected:
  std::shared_ptr<CloseReaderTask> shared_from_this()
  {
    return std::static_pointer_cast<CloseReaderTask>(
        PerfTask::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/tasks/
