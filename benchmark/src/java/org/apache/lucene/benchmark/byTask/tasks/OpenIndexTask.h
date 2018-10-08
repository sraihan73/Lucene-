#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriterConfig.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/LogMergePolicy.h"
#include "PerfTask.h"
#include "stringhelper.h"
#include <memory>
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
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LogMergePolicy = org::apache::lucene::index::LogMergePolicy;

/**
 * Open an index writer.
 * <br>Other side effects: index writer object in perfRunData is set.
 * <br>Relevant properties: <code>merge.factor, max.buffered,
 * max.field.length, ram.flush.mb [default 0]</code>.
 *
 * <p> Accepts a param specifying the commit point as
 * previously saved with CommitIndexTask.  If you specify
 * this, it rolls the index back to that commit on opening
 * the IndexWriter.
 */
class OpenIndexTask : public PerfTask
{
  GET_CLASS_NAME(OpenIndexTask)

public:
  static constexpr int DEFAULT_MAX_BUFFERED =
      IndexWriterConfig::DEFAULT_MAX_BUFFERED_DOCS;
  static constexpr int DEFAULT_MERGE_PFACTOR =
      LogMergePolicy::DEFAULT_MERGE_FACTOR;
  static const double DEFAULT_RAM_FLUSH_MB;

private:
  std::wstring commitUserData;

public:
  OpenIndexTask(std::shared_ptr<PerfRunData> runData);

  int doLogic()  override;

  void setParams(const std::wstring &params) override;

  bool supportsParams() override;

protected:
  std::shared_ptr<OpenIndexTask> shared_from_this()
  {
    return std::static_pointer_cast<OpenIndexTask>(
        PerfTask::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask::tasks
