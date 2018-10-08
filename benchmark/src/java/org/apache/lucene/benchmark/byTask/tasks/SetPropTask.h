#pragma once
#include "PerfTask.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
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

/**
 * Set a performance test configuration property.
 * A property may have a single value, or a sequence of values, separated by
 * ":". If a sequence of values is specified, each time a new round starts, the
 * next (cyclic) value is taken. <br>Other side effects: none. <br>Takes
 * mandatory param: "name,value" pair.
 * @see org.apache.lucene.benchmark.byTask.tasks.NewRoundTask
 */
class SetPropTask : public PerfTask
{
  GET_CLASS_NAME(SetPropTask)

public:
  SetPropTask(std::shared_ptr<PerfRunData> runData);

private:
  std::wstring name;
  std::wstring value;

public:
  int doLogic()  override;

  /**
   * Set the params (property name and value).
   * @param params property name and value separated by ','.
   */
  void setParams(const std::wstring &params) override;

  /* (non-Javadoc)
   * @see org.apache.lucene.benchmark.byTask.tasks.PerfTask#supportsParams()
   */
  bool supportsParams() override;

protected:
  std::shared_ptr<SetPropTask> shared_from_this()
  {
    return std::static_pointer_cast<SetPropTask>(PerfTask::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask::tasks
