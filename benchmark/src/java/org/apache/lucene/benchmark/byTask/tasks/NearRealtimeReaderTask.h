#pragma once
#include "PerfTask.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

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
 * Spawns a BG thread that periodically (defaults to 3.0
 * seconds, but accepts param in seconds) wakes up and asks
 * IndexWriter for a near real-time reader.  Then runs a
 * single query (body: 1) sorted by docdate, and prints
 * time to reopen and time to run the search.
 *
 * @lucene.experimental It's also not generally usable, eg
 * you cannot change which query is executed.
 */
class NearRealtimeReaderTask : public PerfTask
{
  GET_CLASS_NAME(NearRealtimeReaderTask)

public:
  int64_t pauseMSec = 3000LL;

  int reopenCount = 0;
  std::deque<int> reopenTimes = std::deque<int>(1);

  NearRealtimeReaderTask(std::shared_ptr<PerfRunData> runData);

  int doLogic()  override;

  void setParams(const std::wstring &params) override;

  virtual ~NearRealtimeReaderTask();

  bool supportsParams() override;

protected:
  std::shared_ptr<NearRealtimeReaderTask> shared_from_this()
  {
    return std::static_pointer_cast<NearRealtimeReaderTask>(
        PerfTask::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask::tasks
