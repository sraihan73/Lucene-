#pragma once
#include "../../BenchmarkTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

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

using BenchmarkTestCase = org::apache::lucene::benchmark::BenchmarkTestCase;
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;

/** Tests the functionality of {@link AddIndexesTask}. */
class AddIndexesTaskTest : public BenchmarkTestCase
{
  GET_CLASS_NAME(AddIndexesTaskTest)

private:
  static std::shared_ptr<Path> testDir, inputDir;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void
  // beforeClassAddIndexesTaskTest() throws Exception
  static void beforeClassAddIndexesTaskTest() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void
  // afterClassAddIndexesTaskTest()
  static void afterClassAddIndexesTaskTest();

private:
  std::shared_ptr<PerfRunData> createPerfRunData() ;

  void
  assertIndex(std::shared_ptr<PerfRunData> runData) ;

public:
  virtual void testAddIndexesDefault() ;

  virtual void testAddIndexesDir() ;

  virtual void testAddIndexesReader() ;

protected:
  std::shared_ptr<AddIndexesTaskTest> shared_from_this()
  {
    return std::static_pointer_cast<AddIndexesTaskTest>(
        org.apache.lucene.benchmark.BenchmarkTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask::tasks
