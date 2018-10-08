#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::benchmark::byTask
{
class PerfRunData;
}

namespace org::apache::lucene::benchmark::byTask::utils
{
class Algorithm;
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
namespace org::apache::lucene::benchmark::byTask
{

using Algorithm = org::apache::lucene::benchmark::byTask::utils::Algorithm;

/**
 * Run the benchmark algorithm.
 * <p>Usage: java Benchmark  algorithm-file
 * <ol>
 * <li>Read algorithm.</li>
 * <li> Run the algorithm.</li>
 * </ol>
 * Things to be added/fixed in "Benchmarking by tasks":
 * <ol>
 * <li>TODO - report into Excel and/or graphed view.</li>
 * <li>TODO - perf comparison between Lucene releases over the years.</li>
 * <li>TODO - perf report adequate to include in Lucene nightly build site? (so
 * we can easily track performance changes.)</li> <li>TODO - add overall time
 * control for repeated execution (vs. current by-count only).</li> <li>TODO -
 * query maker that is based on index statistics.</li>
 * </ol>
 */
class Benchmark : public std::enable_shared_from_this<Benchmark>
{
  GET_CLASS_NAME(Benchmark)

private:
  std::shared_ptr<PerfRunData> runData;
  std::shared_ptr<Algorithm> algorithm;
  bool executed = false;

public:
  Benchmark(std::shared_ptr<Reader> algReader) ;

  /**
   * Execute this benchmark
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void execute() ;

  /**
   * Run the benchmark algorithm.
   * @param args benchmark config and algorithm files
   */
  static void main(std::deque<std::wstring> &args);

  /**
   * Utility: execute benchmark from command line
   * @param args single argument is expected: algorithm-file
   */
  static void exec(std::deque<std::wstring> &args);

  /**
   * @return Returns the algorithm.
   */
  virtual std::shared_ptr<Algorithm> getAlgorithm();

  /**
   * @return Returns the runData.
   */
  virtual std::shared_ptr<PerfRunData> getRunData();
};

} // namespace org::apache::lucene::benchmark::byTask
