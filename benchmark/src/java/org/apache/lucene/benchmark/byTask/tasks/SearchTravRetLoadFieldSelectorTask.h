#pragma once
#include "SearchTravTask.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_set>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::benchmark::byTask
{
class PerfRunData;
}

namespace org::apache::lucene::document
{
class Document;
}
namespace org::apache::lucene::index
{
class IndexReader;
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
using Document = org::apache::lucene::document::Document;
using IndexReader = org::apache::lucene::index::IndexReader;

/**
 * Search and Traverse and Retrieve docs task using a
 * FieldVisitor loading only the requested fields.
 *
 * <p>Note: This task reuses the reader if it is already open.
 * Otherwise a reader is opened at start and closed at the end.
 *
 * <p>Takes optional param: comma separated deque of Fields to load.</p>
 *
 * <p>Other side effects: counts additional 1 (record) for each traversed hit,
 * and 1 more for each retrieved (non null) document.</p>
 */
class SearchTravRetLoadFieldSelectorTask : public SearchTravTask
{
  GET_CLASS_NAME(SearchTravRetLoadFieldSelectorTask)

protected:
  std::shared_ptr<Set<std::wstring>> fieldsToLoad;

public:
  SearchTravRetLoadFieldSelectorTask(std::shared_ptr<PerfRunData> runData);

  bool withRetrieve() override;

protected:
  std::shared_ptr<Document> retrieveDoc(std::shared_ptr<IndexReader> ir,
                                        int id)  override;

public:
  void setParams(const std::wstring &params) override;

  /* (non-Javadoc)
   * @see org.apache.lucene.benchmark.byTask.tasks.PerfTask#supportsParams()
   */
  bool supportsParams() override;

protected:
  std::shared_ptr<SearchTravRetLoadFieldSelectorTask> shared_from_this()
  {
    return std::static_pointer_cast<SearchTravRetLoadFieldSelectorTask>(
        SearchTravTask::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask::tasks
