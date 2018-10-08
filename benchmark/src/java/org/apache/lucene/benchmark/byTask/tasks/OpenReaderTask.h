#pragma once
#include "PerfTask.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/benchmark/byTask/PerfRunData.h"

#include  "core/src/java/org/apache/lucene/index/IndexCommit.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"

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
using IndexCommit = org::apache::lucene::index::IndexCommit;
using Directory = org::apache::lucene::store::Directory;

/**
 * Open an index reader.
 * <br>Other side effects: index reader object in perfRunData is set.
 * <br> Optional params commitUserData eg. OpenReader(false,commit1)
 */
class OpenReaderTask : public PerfTask
{
  GET_CLASS_NAME(OpenReaderTask)
public:
  static const std::wstring USER_DATA;

private:
  std::wstring commitUserData = L"";

public:
  OpenReaderTask(std::shared_ptr<PerfRunData> runData);

  int doLogic()  override;

  void setParams(const std::wstring &params) override;

  bool supportsParams() override;

  static std::shared_ptr<IndexCommit>
  findIndexCommit(std::shared_ptr<Directory> dir,
                  const std::wstring &userData) ;

protected:
  std::shared_ptr<OpenReaderTask> shared_from_this()
  {
    return std::static_pointer_cast<OpenReaderTask>(
        PerfTask::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/tasks/
