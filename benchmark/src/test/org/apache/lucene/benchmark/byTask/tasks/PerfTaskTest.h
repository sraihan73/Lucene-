#pragma once
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/tasks/PerfTask.h"
#include "../../BenchmarkTestCase.h"
#include "stringhelper.h"
#include <limits>
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

using BenchmarkTestCase = org::apache::lucene::benchmark::BenchmarkTestCase;
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;

/** Tests the functionality of the abstract {@link PerfTask}. */
class PerfTaskTest : public BenchmarkTestCase
{
  GET_CLASS_NAME(PerfTaskTest)

private:
  class MyPerfTask final : public PerfTask
  {
    GET_CLASS_NAME(MyPerfTask)

  public:
    MyPerfTask(std::shared_ptr<PerfRunData> runData);

    int doLogic()  override;

    int getLogStep();

  protected:
    std::shared_ptr<MyPerfTask> shared_from_this()
    {
      return std::static_pointer_cast<MyPerfTask>(PerfTask::shared_from_this());
    }
  };

private:
  std::shared_ptr<PerfRunData>
  createPerfRunData(bool setLogStep, int logStepVal, bool setTaskLogStep,
                    int taskLogStepVal) ;

  void doLogStepTest(bool setLogStep, int logStepVal, bool setTaskLogStep,
                     int taskLogStepVal,
                     int expLogStepValue) ;

public:
  virtual void testLogStep() ;

protected:
  std::shared_ptr<PerfTaskTest> shared_from_this()
  {
    return std::static_pointer_cast<PerfTaskTest>(
        org.apache.lucene.benchmark.BenchmarkTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/tasks/
