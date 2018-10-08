#pragma once
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/tasks/SearchTask.h"
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
 * Test Search task which counts number of searches.
 */
class CountingSearchTestTask : public SearchTask
{
  GET_CLASS_NAME(CountingSearchTestTask)

public:
  static int numSearches;
  static int64_t startMillis;
  static int64_t lastMillis;
  static int64_t prevLastMillis;

  CountingSearchTestTask(std::shared_ptr<PerfRunData> runData);

  int doLogic()  override;

private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  static void incrNumSearches();

public:
  virtual int64_t getElapsedMillis();

protected:
  std::shared_ptr<CountingSearchTestTask> shared_from_this()
  {
    return std::static_pointer_cast<CountingSearchTestTask>(
        SearchTask::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/tasks/
