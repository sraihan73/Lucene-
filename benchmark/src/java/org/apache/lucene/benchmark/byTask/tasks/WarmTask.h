#pragma once
#include "ReadTask.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/benchmark/byTask/PerfRunData.h"

#include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/QueryMaker.h"

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
using QueryMaker = org::apache::lucene::benchmark::byTask::feeds::QueryMaker;

/**
 * Warm reader task: retrieve all reader documents.
 *
 * <p>Note: This task reuses the reader if it is already open.
 * Otherwise a reader is opened at start and closed at the end.
 * </p>
 *
 * <p>Other side effects: counts additional 1 (record) for each
 * retrieved (non null) document.</p>
 */
class WarmTask : public ReadTask
{
  GET_CLASS_NAME(WarmTask)

public:
  WarmTask(std::shared_ptr<PerfRunData> runData);

  bool withRetrieve() override;

  bool withSearch() override;

  bool withTraverse() override;

  bool withWarm() override;

  std::shared_ptr<QueryMaker> getQueryMaker() override;

protected:
  std::shared_ptr<WarmTask> shared_from_this()
  {
    return std::static_pointer_cast<WarmTask>(ReadTask::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/tasks/
