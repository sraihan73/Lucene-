#pragma once
#include "ReadTask.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/Sort.h"

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
using Sort = org::apache::lucene::search::Sort;

/**
 * Does sort search on specified field.
 *
 */
class SearchWithSortTask : public ReadTask
{
  GET_CLASS_NAME(SearchWithSortTask)

private:
  bool doScore = true;
  bool doMaxScore = true;
  std::shared_ptr<Sort> sort;

public:
  SearchWithSortTask(std::shared_ptr<PerfRunData> runData);

  /**
   * SortFields: field:type,field:type[,noscore][,nomaxscore]
   *
   * If noscore is present, then we turn off score tracking
   * in {@link org.apache.lucene.search.TopFieldCollector}.
   * If nomaxscore is present, then we turn off maxScore tracking
   * in {@link org.apache.lucene.search.TopFieldCollector}.
   *
   * name:string,page:int,subject:string
   *
   */
  void setParams(const std::wstring &sortField) override;

  bool supportsParams() override;

  std::shared_ptr<QueryMaker> getQueryMaker() override;

  bool withRetrieve() override;

  bool withSearch() override;

  bool withTraverse() override;

  bool withWarm() override;

  bool withScore() override;

  bool withMaxScore() override;

  std::shared_ptr<Sort> getSort() override;

protected:
  std::shared_ptr<SearchWithSortTask> shared_from_this()
  {
    return std::static_pointer_cast<SearchWithSortTask>(
        ReadTask::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/tasks/
