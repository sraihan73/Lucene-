#pragma once
#include "PerfTask.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/benchmark/byTask/PerfRunData.h"

#include  "core/src/java/org/apache/lucene/store/FSDirectory.h"

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
using FSDirectory = org::apache::lucene::store::FSDirectory;

/**
 * Adds an input index to an existing index, using
 * {@link IndexWriter#addIndexes(Directory...)} or
 * {@link IndexWriter#addIndexes(CodecReader...)}. The location of the input
 * index is specified by the parameter {@link #ADDINDEXES_INPUT_DIR} and is
 * assumed to be a directory on the file system.
 * <p>
 * Takes optional parameter {@code useAddIndexesDir} which specifies which
 * addIndexes variant to use (defaults to true, to use addIndexes(Directory)).
 */
class AddIndexesTask : public PerfTask
{
  GET_CLASS_NAME(AddIndexesTask)

public:
  static const std::wstring ADDINDEXES_INPUT_DIR;

  AddIndexesTask(std::shared_ptr<PerfRunData> runData);

private:
  bool useAddIndexesDir = true;
  std::shared_ptr<FSDirectory> inputDir;

public:
  void setup()  override;

  int doLogic()  override;

  /**
   * Set the params (useAddIndexesDir only)
   *
   * @param params
   *          {@code useAddIndexesDir=true} for using
   *          {@link IndexWriter#addIndexes(Directory...)} or {@code false} for
   *          using {@link IndexWriter#addIndexes(CodecReader...)}. Defaults to
   *          {@code true}.
   */
  void setParams(const std::wstring &params) override;

  bool supportsParams() override;

  void tearDown()  override;

protected:
  std::shared_ptr<AddIndexesTask> shared_from_this()
  {
    return std::static_pointer_cast<AddIndexesTask>(
        PerfTask::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/tasks/
