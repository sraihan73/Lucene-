#pragma once
#include "WriteLineDocTask.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::benchmark::byTask
{
class PerfRunData;
}

namespace org::apache::lucene::document
{
class Document;
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

/**
 * A {@link WriteLineDocTask} which for Wikipedia input, will write category
 * pages to  another file, while remaining pages will be written to the original
 * file. The categories file is derived from the original file, by adding a
 * prefix "categories-".
 */
class WriteEnwikiLineDocTask : public WriteLineDocTask
{
  GET_CLASS_NAME(WriteEnwikiLineDocTask)

private:
  const std::shared_ptr<PrintWriter> categoryLineFileOut;

public:
  WriteEnwikiLineDocTask(std::shared_ptr<PerfRunData> runData) throw(
      std::runtime_error);

  /** Compose categories line file out of original line file */
  static std::shared_ptr<Path> categoriesLineFile(std::shared_ptr<Path> f);

  virtual ~WriteEnwikiLineDocTask();

protected:
  std::shared_ptr<PrintWriter>
  lineFileOut(std::shared_ptr<Document> doc) override;

protected:
  std::shared_ptr<WriteEnwikiLineDocTask> shared_from_this()
  {
    return std::static_pointer_cast<WriteEnwikiLineDocTask>(
        WriteLineDocTask::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask::tasks
