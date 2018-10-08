#pragma once
#include "../BenchmarkTestCase.h"
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
namespace org::apache::lucene::benchmark::quality
{

using BenchmarkTestCase = org::apache::lucene::benchmark::BenchmarkTestCase;

/**
 * Test that quality run does its job.
 * <p>
 * NOTE: if the default scoring or StandardAnalyzer is changed, then
 * this test will not work correctly, as it does not dynamically
 * generate its test trec topics/qrels!
 */
class TestQualityRun : public BenchmarkTestCase
{
  GET_CLASS_NAME(TestQualityRun)

public:
  void setUp()  override;

  virtual void testTrecQuality() ;

  virtual void testTrecTopicsReader() ;

  // use benchmark logic to create the mini Reuters index
private:
  void createReutersIndex() ;

protected:
  std::shared_ptr<TestQualityRun> shared_from_this()
  {
    return std::static_pointer_cast<TestQualityRun>(
        org.apache.lucene.benchmark.BenchmarkTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::quality
