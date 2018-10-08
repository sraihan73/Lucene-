#pragma once
#include "../../../../../../../test-framework/src/java/org/apache/lucene/util/LuceneTestCase.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/benchmark/byTask/Benchmark.h"

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
namespace org::apache::lucene::benchmark
{

using Benchmark = org::apache::lucene::benchmark::byTask::Benchmark;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/** Base class for all Benchmark unit tests. */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressSysoutChecks(bugUrl = "very noisy") public abstract
// class BenchmarkTestCase extends org.apache.lucene.util.LuceneTestCase
class BenchmarkTestCase : public LuceneTestCase
{
private:
  static std::shared_ptr<Path> WORKDIR;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void
  // beforeClassBenchmarkTestCase()
  static void beforeClassBenchmarkTestCase();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void afterClassBenchmarkTestCase()
  static void afterClassBenchmarkTestCase();

  virtual std::shared_ptr<Path> getWorkDir();

  /** Copy a resource into the workdir */
  virtual void
  copyToWorkDir(const std::wstring &resourceName) ;

  /** Return a path, suitable for a .alg config file, for a resource in the
   * workdir */
  virtual std::wstring getWorkDirResourcePath(const std::wstring &resourceName);

  /** Return a path, suitable for a .alg config file, for the workdir */
  virtual std::wstring getWorkDirPath();

  // create the benchmark and execute it.
  virtual std::shared_ptr<Benchmark>
  execBenchmark(std::deque<std::wstring> &algLines) ;

  // properties in effect in all tests here
  std::deque<std::wstring> const propLines = {L"work.dir=" + getWorkDirPath(),
                                               L"directory=RAMDirectory",
                                               L"print.props=false"};

  static const std::wstring NEW_LINE;

  // catenate alg lines to make the alg text
private:
  std::wstring algLinesToText(std::deque<std::wstring> &algLines);

  static void logTstLogic(const std::wstring &txt);

protected:
  std::shared_ptr<BenchmarkTestCase> shared_from_this()
  {
    return std::static_pointer_cast<BenchmarkTestCase>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/benchmark/
