#pragma once
#include "../../../BenchmarkTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

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
namespace org::apache::lucene::benchmark::byTask::tasks::alt
{

using BenchmarkTestCase = org::apache::lucene::benchmark::BenchmarkTestCase;

/** Tests that tasks in alternate packages are found. */
class AltPackageTaskTest : public BenchmarkTestCase
{
  GET_CLASS_NAME(AltPackageTaskTest)

  /** Benchmark should fail loading the algorithm when alt is not specified */
public:
  virtual void testWithoutAlt() ;

  /** Benchmark should be able to load the algorithm when alt is specified */
  virtual void testWithAlt() ;

private:
  std::deque<std::wstring> altAlg(bool allowAlt);

protected:
  std::shared_ptr<AltPackageTaskTest> shared_from_this()
  {
    return std::static_pointer_cast<AltPackageTaskTest>(
        org.apache.lucene.benchmark.BenchmarkTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/tasks/alt/
