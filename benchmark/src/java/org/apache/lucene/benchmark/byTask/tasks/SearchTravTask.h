#pragma once
#include "ReadTask.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <string>

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
 * Search and Traverse task.
 *
 * <p>Note: This task reuses the reader if it is already open.
 * Otherwise a reader is opened at start and closed at the end.
 *
 * <p>Takes optional param: traversal size (otherwise all results are
 * traversed).</p>
 *
 * <p>Other side effects: counts additional 1 (record) for each traversed
 * hit.</p>
 */
class SearchTravTask : public ReadTask
{
  GET_CLASS_NAME(SearchTravTask)
protected:
  // C++ NOTE: Fields cannot have the same name as methods:
  int traversalSize_ = std::numeric_limits<int>::max();

public:
  SearchTravTask(std::shared_ptr<PerfRunData> runData);

  bool withRetrieve() override;

  bool withSearch() override;

  bool withTraverse() override;

  bool withWarm() override;

  std::shared_ptr<QueryMaker> getQueryMaker() override;

  int traversalSize() override;

  void setParams(const std::wstring &params) override;

  /* (non-Javadoc)
   * @see org.apache.lucene.benchmark.byTask.tasks.PerfTask#supportsParams()
   */
  bool supportsParams() override;

protected:
  std::shared_ptr<SearchTravTask> shared_from_this()
  {
    return std::static_pointer_cast<SearchTravTask>(
        ReadTask::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/tasks/
