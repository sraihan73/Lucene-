#pragma once
#include "PerfTask.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::benchmark::byTask::feeds
{
class ContentSource;
}

namespace org::apache::lucene::benchmark::byTask::feeds
{
class DocData;
}
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
using ContentSource =
    org::apache::lucene::benchmark::byTask::feeds::ContentSource;
using DocData = org::apache::lucene::benchmark::byTask::feeds::DocData;

/** Consumes a {@link org.apache.lucene.benchmark.byTask.feeds.ContentSource}.
 */
class ConsumeContentSourceTask : public PerfTask
{
  GET_CLASS_NAME(ConsumeContentSourceTask)

private:
  const std::shared_ptr<ContentSource> source;
  std::shared_ptr<ThreadLocal<std::shared_ptr<DocData>>> dd =
      std::make_shared<ThreadLocal<std::shared_ptr<DocData>>>();

public:
  ConsumeContentSourceTask(std::shared_ptr<PerfRunData> runData);

protected:
  std::wstring getLogMessage(int recsCount) override;

public:
  int doLogic()  override;

protected:
  std::shared_ptr<ConsumeContentSourceTask> shared_from_this()
  {
    return std::static_pointer_cast<ConsumeContentSourceTask>(
        PerfTask::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask::tasks
