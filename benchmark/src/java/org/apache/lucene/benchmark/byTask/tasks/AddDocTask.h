#pragma once
#include "PerfTask.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/benchmark/byTask/PerfRunData.h"

#include  "core/src/java/org/apache/lucene/document/Document.h"

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
 * Add a document, optionally of a certain size.
 * <br>Other side effects: none.
 * <br>Takes optional param: document size.
 */
class AddDocTask : public PerfTask
{
  GET_CLASS_NAME(AddDocTask)

public:
  AddDocTask(std::shared_ptr<PerfRunData> runData);

private:
  int docSize = 0;

  /**
   * volatile data passed between setup(), doLogic(), tearDown().
   * the doc is created at setup() and added at doLogic().
   */
protected:
  std::shared_ptr<Document> doc;

public:
  void setup()  override;

  void tearDown()  override;

protected:
  std::wstring getLogMessage(int recsCount) override;

public:
  int doLogic()  override;

  /**
   * Set the params (docSize only)
   * @param params docSize, or 0 for no limit.
   */
  void setParams(const std::wstring &params) override;

  /* (non-Javadoc)
   * @see org.apache.lucene.benchmark.byTask.tasks.PerfTask#supportsParams()
   */
  bool supportsParams() override;

protected:
  std::shared_ptr<AddDocTask> shared_from_this()
  {
    return std::static_pointer_cast<AddDocTask>(PerfTask::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/tasks/
